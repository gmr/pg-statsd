CREATE FUNCTION statsd.add_timing(text, int4, text, int4)
returns void as 'statsd', 'statsd_add_timing' language C immutable;

COMMENT ON FUNCTION statsd.add_timing(text, int4, text, int4) IS
'Add a timing value in milliseconds to statsd for the given metric name (host, port, metric_name, value)';

CREATE FUNCTION statsd.increment_counter(text, int4, text)
returns void as 'statsd', 'statsd_increment_counter' language C immutable;

COMMENT ON FUNCTION statsd.increment_counter(text, int4, text) IS
'Increment a counter by one (host, port, metric_name)';

CREATE FUNCTION statsd.increment_counter(text, int4, text, int4)
returns void as 'statsd', 'statsd_increment_counter_with_value' language C immutable;

COMMENT ON FUNCTION statsd.increment_counter(text, int4, text, int4) IS
'Increment a counter by the specified value (host, port, metric_name, value)';

CREATE FUNCTION statsd.increment_counter(text, int4, text, int4, float8)
returns void as 'statsd', 'statsd_increment_sampled_counter' language C immutable;

COMMENT ON FUNCTION statsd.increment_counter(text, int4, text, int4, float8) IS
'Increment a counter by the specified value with a sample rate (host, port, metric_name, value, sample_rate)';

CREATE FUNCTION statsd.set_gauge(text, int4, text, float8)
returns void as 'statsd', 'statsd_set_gauge_float8' language C immutable;

COMMENT ON FUNCTION statsd.set_gauge(text, int4, text, float8) IS
'Sets a gauge value (host, port, metric_name, value)';

CREATE FUNCTION statsd.set_gauge(text, int4, text, int4)
returns void as 'statsd', 'statsd_set_gauge_int32' language C immutable;

COMMENT ON FUNCTION statsd.set_gauge(text, int4, text, int4) IS
'Sets a gauge value (host, port, metric_name, value)';
