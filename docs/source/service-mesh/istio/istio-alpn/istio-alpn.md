# Istio ALPN

## Protocol sniffing

> [Better Default Networking -- Protocol sniffing](https://docs.google.com/document/d/1l0oVAneaLLp9KjVOQSb3bwnJJpjyxU_xthpMKFM_l7o/edit#heading=h.edsodfixs1x7)



### **Background**



Istio 1.3 releases the protocol sniffing for outbound traffic. There are two remaining work:

1. Enable protocol sniffing for inbound listener.
2. Enable protocol sniffing for HTTP 2 traffic



The document includes some decisions that used for better networking.

Current inbound listener filter working flow, considering both TLS inspector and HTTP inspector are used.

| **Client Sidecar used** | **Application Protocols** | **Traffic over mTLS** | **TLS Inspector Output**               | **HTTP Inspector Output** | **Output**                             |
| ----------------------- | ------------------------- | --------------------- | -------------------------------------- | ------------------------- | -------------------------------------- |
| Yes                     | TCP                       | No                    | N/A                                    | ALPN: []                  | ALPN: []                               |
| Yes                     | TCP                       | Yes                   | ALPN: [“istio”]Transport Protocol: tls | N/A                       | ALPN: [“istio”]Transport Protocol: tls |
| Yes                     | HTTP 1.x                  | No                    | N/A                                    | ALPN: [“HTTP 1.x”]        | ALPN: [“HTTP 1.x”]                     |
| Yes                     | HTTP 1.x                  | Yes                   | ALPN: [“istio”]Transport Protocol: tls | N/A                       | ALPN: [“istio”]Transport Protocol: tls |
| No                      | TCP                       | No                    | N/A                                    | ALPN: []                  | ALPN: []                               |
| No                      | TCP                       | Yes                   | ALPN: []Transport Protocol: tls        | N/A                       | ALPN: []Transport Protocol: tls        |
| No                      | HTTP 1.x                  | No                    | N/A                                    | ALPN: [“HTTP 1.x”]        | ALPN: [“HTTP 1.x”]                     |
| No                      | HTTP 1.x                  | Yes                   | ALPN: []Transport Protocol: tls        | N/A                       | ALPN: []Transport Protocol: tls        |



When traffic is over TLS and TLS inspector is used by the listener, HTTP inspector will be skipped. HTTP inspector cannot be used to detect ALPN (HTTP vs TCP) from the incoming packet and cannot select filter chain. If client doesn’t provide enough ALPN for server to select filter chain, the TLS inspector cannot select the correct filter chain.



### **Objective**

The doc is going to

1. Describe several alternatives for resolving protocol sniffing when traffic is over TLS and TLS inspector is used.
2. Describe the solution for using protocol sniffing for h2.

### **Overview**

#### **Protocol sniffing for inbound listener**

##### **Filter chains**

In order to support sniffing on inbound, we need 4 inbound filter chains.



|      | Transport Protocol | Application Protocol           | Remarks                                                      |
| ---- | ------------------ | ------------------------------ | ------------------------------------------------------------ |
| FCM1 | tls                | [“istio-http/1.1”, “istio-h2”] | “istio-http/1.1” is for client uses sidecar + http/1.1“Istio-h2” is for client uses sidecar + h2 |
| FCM2 | empty              | [“http/1.1”, “h2c”]            |                                                              |
| FCM3 | tls                | [“istio”]                      | This is for traffic over TLS from client with sidecar        |
| FCM4 | empty              | []                             |                                                              |
| FCM5 | tls                | []                             | This branch includes the case that the client doesn’t use sidecar and sends HTTP traffic over TLS. |



##### **Get ALPN**

Make Envoy client inject the ALPN into the TLS context according to the different upstream connections. E.g., for http/1.1 connection over TLS, the Envoy will inject *istio-http/1.1* in the TLS context and the server will inspect the *istio-http/1.1* and select the http filter chain over TLS. If the client doesn’t have Envoy sidecar, the inspector relies on whether the client set ALPN or not. If the client doesn’t set ALPN, the traffic will be downgraded to TCP.



The ALPN filter will provide the context like:



```yaml

alpn_override: 

 - upstream_protocol: HTTP10
   alpn_override: ["istio-http/1.0", "istio"]

 - upstream_protocol: HTTP11
   alpn_override: ["istio-http/1.1", "istio"]

 - upstream_protocol: HTTP2
   alpn_override: ["istio-h2", "istio"]
```

The reason for including “istio” in the alpn_override is for backward compatible.   

##### **Alternatives Considered**

Inspect ALPN using HTTP inspector when TLS is enabled before creating the filter chain. An alternative to this solution would be creating two listeners, the first listener for terminating TLS connection and use the second listener to do ALPN sniffing.

| **Solution**                     | **Pros**                                                     | **Cons**                                                     |
| -------------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| Inject ALPN in the client side   | Easy to implement in Envoy side compared with inspecting ALPN using HTTP inspector | Need extra work to handle client without sidecar. (The server could use TCP proxy for traffic from client without sidecar). Perhaps we can rely on client to set ALPN itself. |
| Inspect ALPN after TLS inspector | Handle traffic from all clients                              | With current framework, it seems not possible to inspect ALPN with http inspector over TLS.  Need lots of effort to work on Envoy |
| Use two listeners                | No extra effort on Envoy                                     | Performance degradation                                      |



## Other

```{toctree}
istio-alpn-upstream.md
istio-alpn-issues.md
```
