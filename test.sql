SELECT statsd.add_timing('localhost', 8125, 'test-timing1', 70);
SELECT statsd.add_timing('localhost', 8125, 'test-timing1', 50);
SELECT statsd.increment_counter('localhost', 8125, 'test-counter-1');
SELECT statsd.increment_counter('localhost', 8125, 'test-counter-1');
SELECT statsd.increment_counter('localhost', 8125, 'test-counter-1');
SELECT statsd.increment_counter('localhost', 8125, 'test-counter-1');
SELECT statsd.increment_counter('localhost', 8125, 'test-counter-1');
SELECT statsd.increment_counter('localhost', 8125, 'test-counter-1');
SELECT statsd.increment_counter('localhost', 8125, 'test-counter-1');
SELECT statsd.set_gauge('localhost', 8125, 'test-gauge', 98);
