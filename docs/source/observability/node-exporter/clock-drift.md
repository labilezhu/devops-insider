# Time metric - 发现时钟漂移

> [https://www.robustperception.io/time-metric-from-the-node-exporter/](https://www.robustperception.io/time-metric-from-the-node-exporter/)

The node exporter exposes the current machine time.

The time module is even simpler than [conntrack](https://www.robustperception.io/conntrack-metrics-from-the-node-exporter), exposing only a single metric:
```
# HELP node_time_seconds System time in seconds since epoch (1970).
# TYPE node_time_seconds gauge
node_time_seconds 1.5935998188567045e+09
```

On its own this may not seem of much use, as Prometheus already has the `time()` function to provide the evaluation time. Nor can you really compare `time()` to `node_time_seconds` to detect clock drift, as the difference is going to be as much as the scrape interval - and by the time you have 15-60s of clock drift it's probably already causing quite a few problems.

There is however the `timestamp()` function, which will give the timestamp of a sample. For samples from a scrape, this is when Prometheus begins the scrape. So as long as the time between when the scrape starts and the time module is collected is low (which it usually will be) then `timestamp(node_time_seconds) - node_time_seconds` will allow you to spot significant clock drift reasonably easily. It won't be accurate enough to spot being 10ms off, but 1s should be perfectly doable without having to dig into more the intricate timex and NTP metrics that the node exporter can provide.

This could also be used to see how much clock drift there was historically. If you were comparing logs across machines where at least one of them had time sync issues, it could help you figure out the true ordering of events.
