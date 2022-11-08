# Metadata Exchange

## HTTP
[Proxy Metadata Exchange](https://docs.google.com/document/d/1bWQAsrBZguk5HCmBVDEgEMVGS91r9uh3SIr7D7ELZBk/edit#heading=h.pqxgidauhivq)

### Goal
In mixer-less istio, proxies host the adapters performing various mesh functions. These adapters need to consume rich environmental data (e.g. which labels are applied to a pod), that is not exposed directly at the network protocol level. This proposal is about a mechanism for proxies to obtain metadata about the peering proxies.




### Design Proposed Work


We propose to add an Envoy plugin written against WASM API to exchange Envoy node metadata and store it in the request context for consumption of other Envoy plugins.

*Example: Envoy Node Metadata*

```json
    "node": {
     "id": "sidecar~10.44.2.15~productpage-v1-6db7564db8-pvsnd.default~default.svc.cluster.local",
     "cluster": "productpage.default",
     "metadata": {
      "INTERCEPTION_MODE": "REDIRECT",
      "CONFIG_NAMESPACE": "default",
      "ISTIO_VERSION": "1.0-dev",
      "kubernetes.io/limit-ranger": "LimitRanger plugin set: cpu request for container productpage",
      "ISTIO_META_INSTANCE_IPS": "10.44.2.15,10.44.2.15,fe80::8475:dcff:fea1:6c67",
      "POD_NAME": "productpage-v1-6db7564db8-pvsnd",
      "istio": "sidecar",
      "ISTIO_PROXY_VERSION": "1.1.3",
      "ISTIO_PROXY_SHA": "istio-proxy:ecbd1731cedc5d373766ea6e2f1c2e58623b0e28",
      “istio.io/metadata”: {
        “deployment_name”: “productpage-v1”,
        “namespace”: “default”,
        “pod_ip”: “10.44.2.15”,
        “pod_name”: “productpage-v1-6db7564db8-pvsnd”,
        “pod_labels”: {
           “app”: “productpage”,
           “version”: “v1”,
           “pod-template-hash”: “6db7564db8”,
        },
        “ports_to_containers”: {
           “9080”: “productpage”,
        },
        “replicaset_name”: “productpage-v1-6db7564db8”,
        “service_account_name”: “default”,
      },
     },
```

This plugin populates two keys in the `request context` (currently implemented using per-request Dynamic Metadata):
 - `envoy.wasm.metadata_exchange.downstream`
 - `envoy.wasm.metadata_exchange.upstream`

The metadata key is chosen to accommodate native Envoy filters. These filters should be able to utilize Istio metadata without opting-in into Istio extensibility framework based on WASM extensions. 

The values of the keys are extracted from a well-known node metadata key, e.g. node.metadata[“istio.io/metadata”] in the example above. Downstream refers to the preceding hop client proxy, and upstream refers to the subsequent hop server proxy. The value is a generic `google.protobuf.Struct` value. Middle proxies might have both keys populated, while:
 - client-side proxies only have upstream metadata, 
 - and server-side proxies only have downstream metadata.

### Stateless approach

Each proxy injects two HTTP headers on every request and response:
 - `x-istio-peer-metadata` that contains `proto-marshalled base64 encoded metadata value`
 - `x-istio-peer-id `that contains the node key (typically node ID or node cluster)

If the headers are already present then the corresponding metadata key is emitted. The key is the downstream for the request headers and the upstream for the response headers.

#### HTTP header size considerations

There is no specified limit on HTTP headers, but in practice proxies enforce 8k limit on the total size of the HTTP headers. Given the 75% base64 compression ratio, it should be safe to use HTTP headers for up to several KB of metadata content.


## TCP
```{toctree}
tcp-mx.md
```


## Ref
```{toctree}
metadata-exchange-plugin.md
```