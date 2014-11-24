/*
 * Copyright (c) 2014, AWeber Communications.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name pg-statsd nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "postgres.h"
#include "fmgr.h"
#include "funcapi.h"
#include "utils/builtins.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

Datum statsd_add_timing(PG_FUNCTION_ARGS);
Datum statsd_increment_counter(PG_FUNCTION_ARGS);
Datum statsd_increment_sampled_counter(PG_FUNCTION_ARGS);
Datum statsd_increment_counter_with_value(PG_FUNCTION_ARGS);
Datum statsd_set_gauge_float8(PG_FUNCTION_ARGS);
Datum statsd_set_gauge_int32(PG_FUNCTION_ARGS);

/**
 * Create a socket and connect to the statsd server returning the fd
 *
 * @param FunctionCallInfoData *fcinfo
 * @param char *output
 * @return int
 */
static int local_send_metric(FunctionCallInfoData *fcinfo, char *output) {
    struct addrinfo *addr, *addrs;
    int bytes, err, sockfd;
    char ipstr[128], servname[10];

    // Get host and port
    char *host = DatumGetCString(DirectFunctionCall1(textout,
                                                     PointerGetDatum(PG_GETARG_TEXT_P(0))));
    snprintf(servname, sizeof(servname), "%d", PG_GETARG_INT32(1));

    // Get possible addresses to send to
    if ((err = getaddrinfo(host, servname,
                           &(struct addrinfo){.ai_family = AF_UNSPEC,
                                              .ai_socktype = SOCK_DGRAM,
                                              .ai_protocol = 0},
                           &addrs)) != 0)
    {
        ereport(WARNING,
                (errcode(ERRCODE_CONNECTION_FAILURE),
                 errmsg("getaddrinfo(%s): %s", host, gai_strerror(err))));
        return -1;
    }

    // Iterate through the addresses returned by getaddrinfo
    for (addr = addrs; addr != NULL; addr = addr->ai_next) {

        // Create the socket
        sockfd = socket(addr->ai_family, addr->ai_socktype, 0);
        if (sockfd == -1) {
            ereport(WARNING,
                    (errcode(ERRCODE_CONNECTION_FAILURE),
                     errmsg("Error: %s", strerror(errno))));
            continue;
        }

        // Send the packet without connecting
        bytes = sendto(sockfd, output, strlen(output), 0, addr->ai_addr, addr->ai_addrlen);
        if (bytes > 0) {
            return bytes;
        }

        // Get the IP address for debug
        switch(addr->ai_family) {
            case AF_INET:
                inet_ntop(AF_INET, &(((struct sockaddr_in *)addr)->sin_addr), ipstr, sizeof(ipstr));
                break;

            case AF_INET6:
                inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)addr)->sin6_addr), ipstr, sizeof(ipstr));
                break;
        }
        ereport(INFO,
                (errcode(ERRCODE_CONNECTION_FAILURE),
                 errmsg("%s Error: %s", ipstr, strerror(errno))));
    }

    ereport(WARNING,
            (errcode(ERRCODE_CONNECTION_FAILURE),
             errmsg("Could not connect to %s:%s - %s", host, servname, strerror(errno))));
    return -1;
}

/**
 * Add a timing value in milliseconds to statsd for the given metric
 *
 * @param text hostname
 * @param int4 port
 * @param text metric
 * @param int value
 */
PG_FUNCTION_INFO_V1(statsd_add_timing);
Datum
statsd_add_timing(PG_FUNCTION_ARGS){
    if (!PG_ARGISNULL(0)) {
        // Get the metric name
        char *metric = DatumGetCString(DirectFunctionCall1(textout,
                                                           PointerGetDatum(PG_GETARG_TEXT_P(2))));

        // Build the statsd command to send
        int size = strlen(metric) + 12;
        char *output = (char *) palloc(size);
        snprintf(output, size, "%s:%i|ms", metric, PG_GETARG_INT32(3));

        // Write to statsd and free the output
        local_send_metric(fcinfo, output);
        pfree(output);
    }
    PG_RETURN_VOID();
}

/**
 * Increment a counter by one
 *
 * @param text hostname
 * @param int4 port
 * @param text metric
 */
PG_FUNCTION_INFO_V1(statsd_increment_counter);
Datum
statsd_increment_counter(PG_FUNCTION_ARGS){
    if (!PG_ARGISNULL(0)) {
        // Get the metric name
        char *metric = DatumGetCString(DirectFunctionCall1(textout,
                                                           PointerGetDatum(PG_GETARG_TEXT_P(2))));

        // Build the statsd command to send
        int size = strlen(metric) + 5;
        char *output = (char *) palloc(size);
        snprintf(output, size, "%s:1|c", metric);

        // Write to statsd and free the output
        local_send_metric(fcinfo, output);
        pfree(output);
    }
    PG_RETURN_VOID();
}

/**
 * Increment a counter with a specified value
 *
 * @param text hostname
 * @param int4 port
 * @param text metric
 * @param int4 value
 */
PG_FUNCTION_INFO_V1(statsd_increment_counter_with_value);
Datum
statsd_increment_counter_with_value(PG_FUNCTION_ARGS){
    if (!PG_ARGISNULL(0)) {
        // Get the metric name
        char *metric = DatumGetCString(DirectFunctionCall1(textout,
                                                           PointerGetDatum(PG_GETARG_TEXT_P(2))));

        // Build the statsd command to send
        int32 size = strlen(metric) + 128;
        char *output = (char *) palloc(size);
        snprintf(output, size, "%s:%i|c", metric, PG_GETARG_INT32(3));

        // Write to statsd and free the output
        local_send_metric(fcinfo, output);
        pfree(output);
    }
    PG_RETURN_VOID();
}

/**
 * Increment a sampled counter with a value
 *
 * @param text hostname
 * @param int4 port
 * @param text metric
 * @param int4 value
 * @param float8 sample
 */
PG_FUNCTION_INFO_V1(statsd_increment_sampled_counter);
Datum
statsd_increment_sampled_counter(PG_FUNCTION_ARGS){
    if (!PG_ARGISNULL(0)) {
        // Get the metric name
        char *metric = DatumGetCString(DirectFunctionCall1(textout,
                                                           PointerGetDatum(PG_GETARG_TEXT_P(2))));

        // Build the statsd command to send
        int32 size = strlen(metric) + 256;
        char *output = (char *) palloc(size);
        snprintf(output, size, "%s:%d|c|@%f", metric, PG_GETARG_INT32(3), PG_GETARG_FLOAT8(4));

        // Write to statsd and free the output
        local_send_metric(fcinfo, output);
        pfree(output);
    }
    PG_RETURN_VOID();
}

/**
 * Set a float gauge value on a statsd server
 *
 * @param text hostname
 * @param int4 port
 * @param text metric
 * @param float8 value
 */
PG_FUNCTION_INFO_V1(statsd_set_gauge_float8);
Datum
statsd_set_gauge_float8(PG_FUNCTION_ARGS){
    if (!PG_ARGISNULL(0)) {
        // Get the metric name
        char *metric = DatumGetCString(DirectFunctionCall1(textout,
                                                           PointerGetDatum(PG_GETARG_TEXT_P(2))));

        // Build the statsd command to send
        int32 size = strlen(metric) + 128;
        char *output = (char *) palloc(size);
        snprintf(output, size, "%s:%f|g", metric, PG_GETARG_FLOAT8(3));

        // Write to statsd and free the output
        local_send_metric(fcinfo, output);
        pfree(output);
    }
    PG_RETURN_VOID();
}

/**
 * Set an int32 gauge value on a statsd server
 *
 * @param text hostname
 * @param int4 port
 * @param text metric
 * @param int4 value
 */
PG_FUNCTION_INFO_V1(statsd_set_gauge_int32);
Datum
statsd_set_gauge_int32(PG_FUNCTION_ARGS){
    if (!PG_ARGISNULL(0)) {
        // Get the metric name
        char *metric = DatumGetCString(DirectFunctionCall1(textout,
                                                           PointerGetDatum(PG_GETARG_TEXT_P(2))));

        // Build the statsd command to send
        int size = strlen(metric) + 12;
        char *output = (char *) palloc(size);
        snprintf(output, size, "%s:%i|g", metric, PG_GETARG_INT32(3));

        // Write to statsd and free the output
        local_send_metric(fcinfo, output);
        pfree(output);
    }
    PG_RETURN_VOID();
}
