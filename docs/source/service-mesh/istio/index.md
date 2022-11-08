# Istio

见我的另一本开源书：[《Istio & Envoy 内幕》](https://istio-insider.mygraphql.com/)

```{toctree}
istio-mtls/istio-mtls.md
metadata-exchange/metadata-exchange.md
istio-alpn/istio-alpn.md
```

## Design Doc Links
> [Design Doc Links](https://github.com/istio/istio/wiki/Design-Doc-Links)

This doc tracks the design doc links in order to facilitate Istio design docs discovery. To avoid confusion, only approved docs or the docs with wide impact in Istio's roadmap can be listed here.

It's each developer's own responsibility to add doc links here.

### Nebulous Future

- [Mesh TroubleShooting RFC](https://docs.google.com/document/d/11P6OXlITExJvFqzsWfFu9WhFLjY6uqOcCUKqPVgSSoc/edit#) introduced a trouble shooting API to allow us make sidecar out of the pod.
- [Kernel Offloads for Istio and Envoy](https://docs.google.com/document/d/1y4k35xr38y1Hp98bu3zh7B_rZZf8ItVG86o9Oc5L7LM)
- [Istio Authentication rev2, JWT](https://docs.google.com/document/d/1f1ZVCUa7ktuoFKKFATCXfu9qFFFXX-7UMAJQtmMPuS4)
- [Istio Transport Security](https://docs.google.com/document/d/1ubUG78rNQbwwkqpvYcr7KgM14kEHwitSsuorCZjR6qY/edit?ts=5db9c907)
- [Istio Authentication rev2, mTLS](https://docs.google.com/document/d/1RAsCl4n_F75ANAXrbI9tllA8QmE4etQ3HBHXNjxLPTo)
- [Simpler Istio, istiod](https://docs.google.com/document/d/1v8BxI07u-mby5f5rCruwF7odSXgb9G8-C9W5hQtSIAg)

### Release 1.4

- [Istio Auto mTLS](https://docs.google.com/document/d/1yEMDRO2FZCyZnDK1AzNmjQtbtQqE7wN8YoM65FAH7uA), addresses the common UX pain for configuring `DestinationRule.TLSSettings` in order to opt-in Istio mutual TLS.
- [Isito Authorization v2 Beta policy](https://docs.google.com/document/d/1diwa9oYVwmLtarXb-kfWPq-Ul3O2Ic3b8Y21Z3R8DzQ), is focused on evolving authorization policy to workload based selector, rather service based model, with other UX improvements.
- [Better Default Networking, Protocol Sniffing](https://docs.google.com/document/d/1l0oVAneaLLp9KjVOQSb3bwnJJpjyxU_xthpMKFM_l7o/edit#), enables protocol sniffing for inbound listeners and HTTP2.
- [Istio Operator Architecture](https://github.com/istio/operator/blob/release-1.4/ARCHITECTURE.md), explains how the new Istio operator works, in replace of the Helm. The original design doc can be found [here](https://docs.google.com/document/d/11j9ZtYWNWnxQYnZy8ayZav1FMwTH6F6z6fkDYZ7V298/edit#heading=h.qex63c29z2to)
- [Istio Metadata Exchange for MixerV2 under mTLS](https://docs.google.com/document/d/1bWQAsrBZguk5HCmBVDEgEMVGS91r9uh3SIr7D7ELZBk/edit#), implementation [notes](https://docs.google.com/document/d/15jKV_muOZKX4CccnA1w-X73SkqGt8cV3a9TBXxzs0TA/edit#)

### Release 1.3

- [Better Default Networks](https://docs.google.com/document/d/12Z_oFCFn3_rTWbQsuR1GmjU5_WZVkBTJwFpFMx802Ns/edit), explains the mechanism to how to reduce the UX overhead of declaring service port name explicitly.

### Release 1.1

- Sidecar, TODO: link 

## Observability

```{toctree}
observability/metrics/tcp-metrics/tcp-metrics.md
observability/log/access-log.md
```