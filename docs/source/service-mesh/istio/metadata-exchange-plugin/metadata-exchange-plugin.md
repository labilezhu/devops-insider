# metadata exchange plugin

> [https://techblog.cisco.com/blog/istio-mixerless-telemetry](https://techblog.cisco.com/blog/istio-mixerless-telemetry)

## Metadata exchange plugin

The first problem that had to be solved was how to make client/server metadata about the two sides of a connection available in the proxies.

- For HTTP-based traffic this is accomplished via custom HTTP headers (`envoy.wasm.metadata_exchange.upstream`, `envoy.wasm.metadata_exchange.downstream`) in the request/response that contains the metadata attributes of the other side.

- For generic TCP traffic the metadata exchange uses ALPN-based tunneling and a prefix based protocol. A new protocol `istio-peer-exchange` is defined, which is advertised and prioritized by the client and the server sidecars in the mesh. ALPN negotiation resolves the protocol to istio-peer-exchange for connections between Istio enabled proxies, but not between an Istio enabled proxy and any client.

### Stats plugin

The stats plugin records incoming and outgoing traffic metrics into the Envoy statistics subsystem and makes them available for Prometheus to scrape.

**The following are the standard service level metrics exported by default.**

#### For HTTP, HTTP/2, and GRPC traffic the proxy generates the following metrics

| Name                                | Description                                                  |
| ----------------------------------- | ------------------------------------------------------------ |
| istio_requests_total                | This is a COUNTER incremented for every request handled by an Istio proxy. |
| istio_request_duration_milliseconds | This is a DISTRIBUTION which measures the duration of requests. |
| istio_request_bytes                 | This is a DISTRIBUTION which measures HTTP request body sizes. |
| istio_response_bytes                | This is a DISTRIBUTION which measures HTTP response body sizes. |

#### For TCP traffic the proxy generates the following metrics

| Name                               | Description                                                  |
| ---------------------------------- | ------------------------------------------------------------ |
| istio_tcp_sent_bytes_total         | This is a COUNTER which measures the size of total bytes sent during response in case of a TCP connection. |
| istio_tcp_received_bytes_total     | This is a COUNTER which measures the size of total bytes received during request in case of a TCP connection. |
| istio_tcp_connections_opened_total | This is a COUNTER incremented for every opened connection.   |
| istio_tcp_connections_closed_total | This is a COUNTER incremented for every closed connection.   |

**The following are the default labels on service level metrics.**

```bash
reporter: conditional((context.reporter.kind | "inbound") == "outbound", "source", "destination")
source_workload: source.workload.name | "unknown"
source_workload_namespace: source.workload.namespace | "unknown"
source_principal: source.principal | "unknown"
source_app: source.labels["app"] | "unknown"
source_version: source.labels["version"] | "unknown"
destination_workload: destination.workload.name | "unknown"
destination_workload_namespace: destination.workload.namespace | "unknown"
destination_principal: destination.principal | "unknown"
destination_app: destination.labels["app"] | "unknown"
destination_version: destination.labels["version"] | "unknown"
destination_service: destination.service.host | "unknown"
destination_service_name: destination.service.name | "unknown"
destination_service_namespace: destination.service.namespace | "unknown"
request_protocol: api.protocol | context.protocol | "unknown"
response_code: response.code | 200
connection_security_policy: conditional((context.reporter.kind | "inbound") == "outbound", "unknown", conditional(connection.mtls | false, "mutual_tls", "none"))
response_flags: context.proxy_error_code | "-"
source_canonical_service
source_canonical_revision
destination_canonical_service
destination_canonical_revision
```

> You can find more info about the labels in [Istio docs](https://istio.io/docs/reference/config/telemetry/metrics/#labels)

The stats plugin in Istio 1.5 not only includes standard metrics, but [experimental support](https://github.com/istio/istio/wiki/Configurable-V2-Metrics) for modifying them. Be aware that **the API to configure the metrics will be changed in Istio 1.6, due to the new extensions API design**.

## Feature gaps between Mixer-based telemetry and Telemetry V2

Although Mixer has been deprecated with the 1.5 release; it is a highly configurable component and provides a lot of features. There are *significant* feature gaps between the telemetry provided by Mixer and what V2 provides today.

- Out of mesh telemetry is not fully supported: some metrics are missing (the traffic source or destination is not injected by the sidecar).
- Egress gateway telemetry is not supported.
- TCP telemetry is only supported with mTLS. (就是如果你的 TCP 流量因不小心的配置，没走 mTLS，那么就不会有 metrics 收集到)
- Black Hole telemetry for TCP and HTTP protocols is not supported.
- Histogram buckets are significantly different from the ones based on Mixer.
- Custom metrics support is experimental and limited.
