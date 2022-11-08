# Istio ALPN Issues



- [http alpn override applied to external services breaks connections #24619](https://github.com/istio/istio/issues/24619)
- [Set standard alpns as well in outbound traffic #29529](https://github.com/istio/istio/pull/29529)
- https://github.com/istio/istio/blob/1.5.4/pilot/pkg/networking/core/v1alpha3/listener.go#L1802
- [Significance of setting AlpnProtocols in the client's UpstreamTlsContext #11909](https://github.com/envoyproxy/envoy/issues/11909)
- [ALPN filter incorrectly applies to non-Istio TLS traffic #40680](https://github.com/istio/istio/issues/40680)
- ["TLS handshake error" issue after Istio update 1.13.7 -> 1.14.4 #41138](https://github.com/istio/istio/issues/41138)

