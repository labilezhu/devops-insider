# TCP Proxy



> [https://www.envoyproxy.io/docs/envoy/latest/intro/arch_overview/listeners/tcp_proxy#arch-overview-tcp-proxy](https://www.envoyproxy.io/docs/envoy/latest/intro/arch_overview/listeners/tcp_proxy#arch-overview-tcp-proxy)

Since Envoy is fundamentally written as a L3/L4 server, basic L3/L4 proxy is easily implemented. The TCP proxy filter performs basic 1:1 network connection proxy between downstream clients and upstream clusters. It can be used by itself as an stunnel replacement, or in conjunction with other filters such as the [MongoDB filter](https://www.envoyproxy.io/docs/envoy/latest/intro/arch_overview/other_protocols/mongo#arch-overview-mongo) or the [rate limit](https://www.envoyproxy.io/docs/envoy/latest/configuration/listeners/network_filters/rate_limit_filter#config-network-filters-rate-limit) filter.

The TCP proxy filter will respect the [connection limits](https://www.envoyproxy.io/docs/envoy/latest/api-v3/config/cluster/v3/circuit_breaker.proto#envoy-v3-api-field-config-cluster-v3-circuitbreakers-thresholds-max-connections) imposed by each upstream cluster’s global resource manager. The TCP proxy filter checks with the upstream cluster’s resource manager if it can create a connection without going over that cluster’s maximum number of connections, if it can’t the TCP proxy will not make the connection.

TCP proxy filter [configuration reference](https://www.envoyproxy.io/docs/envoy/latest/configuration/listeners/network_filters/tcp_proxy_filter#config-network-filters-tcp-proxy).

## TCP Proxy Configuration

> [https://www.envoyproxy.io/docs/envoy/latest/configuration/listeners/network_filters/tcp_proxy_filter#config-network-filters-tcp-proxy](https://www.envoyproxy.io/docs/envoy/latest/configuration/listeners/network_filters/tcp_proxy_filter#config-network-filters-tcp-proxy)

- TCP proxy [architecture overview](https://www.envoyproxy.io/docs/envoy/latest/intro/arch_overview/listeners/tcp_proxy#arch-overview-tcp-proxy)
- This filter should be configured with the type URL `type.googleapis.com/envoy.extensions.filters.network.tcp_proxy.v3.TcpProxy`.
- [v3 API reference](https://www.envoyproxy.io/docs/envoy/latest/api-v3/extensions/filters/network/tcp_proxy/v3/tcp_proxy.proto#envoy-v3-api-msg-extensions-filters-network-tcp-proxy-v3-tcpproxy)



### Dynamic cluster selection

The upstream cluster used by the TCP proxy filter can be dynamically set by other network filters on a per-connection basis by setting a per-connection state object under the key `envoy.tcp_proxy.cluster`. See the implementation for the details.



### Routing to a subset of hosts

TCP proxy can be configured to route to a subset of hosts within an upstream cluster.

To define metadata that a suitable upstream host must match, use one of the following fields:

1. Use [TcpProxy.metadata_match](https://www.envoyproxy.io/docs/envoy/latest/api-v3/extensions/filters/network/tcp_proxy/v3/tcp_proxy.proto#envoy-v3-api-field-extensions-filters-network-tcp-proxy-v3-tcpproxy-metadata-match) to define required metadata for a single upstream cluster.
2. Use [ClusterWeight.metadata_match](https://www.envoyproxy.io/docs/envoy/latest/api-v3/extensions/filters/network/tcp_proxy/v3/tcp_proxy.proto#envoy-v3-api-field-extensions-filters-network-tcp-proxy-v3-tcpproxy-weightedcluster-clusterweight-metadata-match) to define required metadata for a weighted upstream cluster.
3. Use combination of [TcpProxy.metadata_match](https://www.envoyproxy.io/docs/envoy/latest/api-v3/extensions/filters/network/tcp_proxy/v3/tcp_proxy.proto#envoy-v3-api-field-extensions-filters-network-tcp-proxy-v3-tcpproxy-metadata-match) and [ClusterWeight.metadata_match](https://www.envoyproxy.io/docs/envoy/latest/api-v3/extensions/filters/network/tcp_proxy/v3/tcp_proxy.proto#envoy-v3-api-field-extensions-filters-network-tcp-proxy-v3-tcpproxy-weightedcluster-clusterweight-metadata-match) to define required metadata for a weighted upstream cluster (metadata from the latter will be merged on top of the former).

In addition, dynamic metadata can be set by earlier network filters on the `StreamInfo`. Setting the dynamic metadata must happen before `onNewConnection()` is called on the `TcpProxy` filter to affect load balancing.



### Statistics

The TCP proxy filter emits both its own downstream statistics, [access logs](https://www.envoyproxy.io/docs/envoy/latest/configuration/observability/access_log/usage#config-access-log) for upstream and downstream connections, as well as many of the [cluster upstream statistics](https://www.envoyproxy.io/docs/envoy/latest/configuration/upstream/cluster_manager/cluster_stats#config-cluster-manager-cluster-stats) where applicable. The downstream statistics are rooted at *tcp.<stat_prefix>.* with the following statistics:

| Name                                          | Type    | Description                                                  |
| --------------------------------------------- | ------- | ------------------------------------------------------------ |
| downstream_cx_total                           | Counter | Total number of connections handled by the filter            |
| downstream_cx_no_route                        | Counter | Number of connections for which no matching route was found or the cluster for the route was not found |
| downstream_cx_tx_bytes_total                  | Counter | Total bytes written to the downstream connection             |
| downstream_cx_tx_bytes_buffered               | Gauge   | Total bytes currently buffered to the downstream connection  |
| downstream_cx_rx_bytes_total                  | Counter | Total bytes read from the downstream connection              |
| downstream_cx_rx_bytes_buffered               | Gauge   | Total bytes currently buffered from the downstream connection |
| downstream_flow_control_paused_reading_total  | Counter | Total number of times flow control paused reading from downstream |
| downstream_flow_control_resumed_reading_total | Counter | Total number of times flow control resumed reading from downstream |
| idle_timeout                                  | Counter | Total number of connections closed due to idle timeout       |
| max_downstream_connection_duration            | Counter | Total number of connections closed due to max_downstream_connection_duration timeout |
| on_demand_cluster_attempt                     | Counter | Total number of connections that requested on demand cluster |
| on_demand_cluster_missing                     | Counter | Total number of connections closed due to on demand cluster is missing |
| on_demand_cluster_success                     | Counter | Total number of connections that requested and received on demand cluster |
| on_demand_cluster_timeout                     | Counter | Total number of connections closed due to on demand cluster lookup timeout |
| upstream_flush_total                          | Counter | Total number of connections that continued to flush upstream data after the downstream connection was closed |
| upstream_flush_active                         | Gauge   | Total connections currently continuing to flush upstream data after the downstream connection was closed |



### TCP Proxy(proto)
> [https://www.envoyproxy.io/docs/envoy/latest/api-v3/extensions/filters/network/tcp_proxy/v3/tcp_proxy.proto#envoy-v3-api-msg-extensions-filters-network-tcp-proxy-v3-tcpproxy-ondemand](https://www.envoyproxy.io/docs/envoy/latest/api-v3/extensions/filters/network/tcp_proxy/v3/tcp_proxy.proto#envoy-v3-api-msg-extensions-filters-network-tcp-proxy-v3-tcpproxy-ondemand)