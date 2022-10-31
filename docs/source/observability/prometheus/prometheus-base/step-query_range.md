# Step and query_range - 步长与查询范围

> [https://www.robustperception.io/step-and-query_range/](https://www.robustperception.io/step-and-query_range/)

Graphs from Prometheus use the `query_range` endpoint, and there's a non-trivial amount of confusion that it's more magic than it actually is.

The [query range](https://prometheus.io/docs/prometheus/2.11/querying/api/#range-queries) endpoint isn't magic, in fact it is quite dumb. There's a query, a `start time`, an `end time`, and a `step`.

The provided query is evaluated at the `start time`, as if using the [query endpoint](https://prometheus.io/docs/prometheus/2.11/querying/api/#instant-queries). Then it's evaluated at the start time plus one `step`. Then the `start time` plus two `steps`, and so on stopping before the evaluation time would be after the end time. The results from all the evaluations are combined into time series, so if say samples for `series A` were present in the 1st and 3rd evaluations then both those samples would be returned in the same time series.

That's it. The query range endpoint is just syntactic sugar on top of the query endpoint*. Functions like `rate` don't know whether they're being called as part of a `range query`, nor do they know what the `step` is. `topk` is across each step, [not the entire graph](https://www.robustperception.io/graph-top-n-time-series-in-grafana).

One consequence(结果) of this is that you must take a little care when choosing the `range` for functions like `rate` or `avg_over_time`, as if it's smaller then the step then you'll undersample(欠采样) and skip over some data. If using Grafana, you can use `$__interval` to choose an appropriate value such as `rate(a_metric_total[$__interval])`.

 

**This <mark>was</mark> the case until Prometheus 2.3.0, where I made significant performance improvements to PromQL. Query is now a special case of query range, however conceptually and semantically it's all still the same.**


## Grafana global-variables


> [https://grafana.com/docs/grafana/latest/dashboards/variables/add-template-variables/#global-variables](https://grafana.com/docs/grafana/latest/dashboards/variables/add-template-variables/#global-variables)



### $__interval

You can use the `$__interval` variable as a parameter to group by time (for InfluxDB, MySQL, Postgres, MSSQL), Date histogram interval (for Elasticsearch), or as a _summarize_ function parameter (for Graphite).

Grafana automatically calculates an interval that can be used to group by time in queries. When there are more data points than can be shown on a graph, then queries can be made more efficient by grouping by a larger interval. It is more efficient to group by 1 day than by 10s when looking at 3 months of data and the graph will look the same and the query will be faster. The `$__interval` is calculated using the `time range` and the `width of the graph` (the number of pixels).

Approximate Calculation: `(to - from) / resolution`

For example, when the time range is 1 hour and the graph is full screen, then the interval might be calculated to `2m` - points are grouped in 2 minute intervals. If the time range is 6 months and the graph is full screen, then the interval might be `1d` (1 day) - points are grouped by day.

In the InfluxDB data source, the legacy variable `$interval` is the same variable. `$__interval` should be used instead.

The InfluxDB and Elasticsearch data sources have `Group by time interval` fields that are used to hard code the interval or to set the minimum limit for the `$__interval` variable (by using the `>` syntax -> `>10m`).

### $__interval_ms
This variable is the `$__interval` variable in milliseconds, not a time interval formatted string. For example, if the `$__interval` is `20m` then the `$__interval_ms` is `1200000`.

### $__range

Currently only supported for Prometheus and Loki data sources. This variable represents the range for the current dashboard. It is calculated by `to - from`. It has a millisecond and a second representation called `$__range_ms` and `$__range_s`.

### $__rate_interval

Currently only supported for Prometheus data sources. The `$__rate_interval` variable is meant to be used in the rate function. Refer to [Prometheus query variables](https://grafana.com/docs/grafana/latest/datasources/prometheus/#using-__rate_interval) for details.

## Using interval and range variables

> [https://grafana.com/docs/grafana/latest/datasources/prometheus/](https://grafana.com/docs/grafana/latest/datasources/prometheus/#using-__rate_interval:~:text=Using%20interval%20and%20range%20variables)


#### Using interval and range variables

> Support for `$__range`, `$__range_s` and `$__range_ms` only available from Grafana v5.3

You can use some global built-in variables in query variables, for example, `$__interval`, `$__interval_ms`, `$__range`, `$__range_s` and `$__range_ms`. See [Global built-in variables](https://grafana.com/docs/grafana/latest/dashboards/variables/add-template-variables/#global-variables) for more information. They are convenient to use in conjunction with the `query_result` function when you need to filter variable queries since the `label_values` function doesn’t support queries.

Make sure to set the variable’s `refresh` trigger to be `On Time Range Change` to get the correct instances when changing the time range on the dashboard.

**Example usage:**

Populate a variable with the busiest 5 request instances based on average QPS over the time range shown in the dashboard:

```
Query: query_result(topk(5, sum(rate(http_requests_total[$__range])) by (instance)))
Regex: /"([^"]+)"/
```

Populate a variable with the instances having a certain state over the time range shown in the dashboard, using `$__range_s`:

```
Query: query_result(max_over_time(<metric>[${__range_s}s]) != <state>)
Regex:
```

### Using `$__rate_interval`

> **Note:** Available in Grafana 7.2 and above

`$__rate_interval` is the recommended interval to use in the `rate` and `increase` functions. It will “just work” in most cases, avoiding most of the pitfalls that can occur when using a fixed interval or `$__interval`.

```
OK:       rate(http_requests_total[5m])
Better:   rate(http_requests_total[$__rate_interval])
```

Details: `$__rate_interval` is defined as max(`$__interval` + _Scrape interval_, 4 \* _Scrape interval_), where _Scrape interval_ is the Min step setting (AKA query_interval, a setting per PromQL query) if any is set. Otherwise, the Scrape interval setting in the Prometheus data source is used. (The Min interval setting in the panel is modified by the resolution setting and therefore doesn’t have any effect on \_Scrape interval_.) [This article](https://grafana.com/blog/2020/09/28/new-in-grafana-7.2-__rate_interval-for-prometheus-rate-queries-that-just-work/) contains additional details.

## New in Grafana 7.2: $__rate_interval for Prometheus rate queries that just work
> [https://grafana.com/blog/2020/09/28/new-in-grafana-7.2-__rate_interval-for-prometheus-rate-queries-that-just-work/](https://grafana.com/blog/2020/09/28/new-in-grafana-7.2-__rate_interval-for-prometheus-rate-queries-that-just-work/)

