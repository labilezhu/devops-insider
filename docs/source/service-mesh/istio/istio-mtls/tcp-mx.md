# TCP MX

`TCP MX` or `TCP Metadata Exchange` or `istio-peer-exchange`。即是在两个 sidecar 间，在正式开始 TCP 流量前，均需要先用 TCP 数据流量交换自己的 Metadata 给对方。

## metadata exchange plugin

> [https://techblog.cisco.com/blog/istio-mixerless-telemetry](https://techblog.cisco.com/blog/istio-mixerless-telemetry)

### Metadata exchange plugin

The first problem that had to be solved was how to make client/server metadata about the two sides of a connection available in the proxies.

- For HTTP-based traffic this is accomplished via custom HTTP headers (`envoy.wasm.metadata_exchange.upstream`, `envoy.wasm.metadata_exchange.downstream`) in the request/response that contains the metadata attributes of the other side.

- For generic TCP traffic the metadata exchange uses ALPN-based tunneling and a prefix based protocol. A new protocol `istio-peer-exchange` is defined, which is advertised and prioritized by the client and the server sidecars in the mesh. ALPN negotiation resolves the protocol to istio-peer-exchange for connections between Istio enabled proxies, but not between an Istio enabled proxy and any client. 


## TCP MX 设计文档

> [https://docs.google.com/document/d/1s6ou__qRL4UiWY1amyk5p8YzOMQvBhfR15o1U_ZLIow/edit#](https://docs.google.com/document/d/1s6ou__qRL4UiWY1amyk5p8YzOMQvBhfR15o1U_ZLIow/edit#)

