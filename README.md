pg_statsd
=========
pg_statsd is a set of PostgreSQL user-defined functions that provide
and interface to statsd.

Requirements
------------
PostgreSQL

Installation
------------
pg_statsd is available for installation from pgxn: http://pgxn.org

Building
--------
To build pg_statsd, simply:

	make
	make install

Loading
-------

Once statsd is installed, you can add it to a database. Add this line to your
postgresql config

    shared_preload_libraries = 'pg_statsd.so'

Then, If you're running PostgreSQL 9.1.0 or greater, loading statsd is as simple
as connecting to a database as a super user and running:

    CREATE EXTENSION statsd;

If you've upgraded your cluster to PostgreSQL 9.1 and already had statsd
installed, you can upgrade it to a properly packaged extension with:

    CREATE EXTENSION statsd FROM unpackaged;

For versions of PostgreSQL less than 9.1.0, you'll need to run the
installation script:

    psql -d mydb -f /path/to/pgsql/share/contrib/statsd.sql

Functions
---------
The following functions are available for use.

	# Add a timing value for the specified metric in milliseconds
	statsd.add_timing(hostname::text, port::int4, metric::text, value::int4)

	# Increment the specified metric by one
	statsd.increment_counter(hostname::text, port::int4, metric::text)

	# Increment the specified metric by a value
	statsd.increment_counter(hostname::text, port::int4, metric::text, value::int4)

	# Increment the specified metric by a value and include a sample rate
	statsd.increment_counter(hostname::text, port::int4, metric::text, value::int4, sample::float8)

	# Set a gauge value for the specified metric
	statsd.set_gauge(hostname::text, port::int4, metric::text, value::float8)
	statsd.set_gauge(hostname::text, port::int4, metric::text, value::int4)

Examples
--------

	postgres=# SELECT statsd.add_timing('localhost', 8125, 'test-timing1', 70);
	 add_timing 
	------------
	 t
	(1 row)

	postgres=# SELECT statsd.increment_counter('localhost', 8125, 'test-counter-1');
	 increment_counter 
	-------------------
	 t
	(1 row)

	postgres=# SELECT statsd.increment_counter('localhost', 8125, 'test-counter-1', 5);
	 increment_counter 
	-------------------
	 t
	(1 row)

	postgres=# SELECT statsd.increment_counter('localhost', 8125, 'test-counter-1', 5, 0.25);
	 increment_counter 
	-------------------
	 t
	(1 row)

	postgres=# SELECT statsd.set_gauge('localhost', 8125, 'temperature', 98.7);
	 set_gauge 
	-----------
	 t
	(1 row)

All methods return bool indicating if the statsd command was able to be sent or not.

License
-------
Copyright (c) 2014, AWeber Communications.
All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
 
* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the following
  disclaimer in the documentation and/or other materials provided
  with the distribution.
* Neither the name pg_statsd nor the names of its contributors may 
  be used to endorse or promote products derived from this software 
  without specific prior written
  permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.