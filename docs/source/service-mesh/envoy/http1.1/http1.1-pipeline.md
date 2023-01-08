# HTTP 1.1 Pipelining is not supported with Envoy


> [https://support.f5.com/csp/article/K43429428](https://support.f5.com/csp/article/K43429428)



Description

With HTTP 1.1 protocol, there are several ways through which connection management is done: short-lived connections, persistent connections, and HTTP pipelining. In this article, we will be discussing the pipelining model. Instead of opening a new TCP connection between client and server for each request, HTTP pipelining sends several successive requests without even waiting for an answer on single TCP connection, reducing much of the latency.

- HTTP/1.1 without pipelining: Each HTTP request over the TCP connection must be responded to before the next request can be made.
- HTTP/1.1 with pipelining: Each HTTP request over the TCP connection may be made immediately without waiting for the previous request's response to return. The responses will come back in the same order.
- HTTP/2 multiplexing: Each HTTP request over the TCP connection may be made immediately without waiting for the previous response to come back. The responses may come back in any order.

Environment

- Aspen Mesh: 1.11.x

Cause

The following procedure was performed in lab to confirm that HTTP 1.1 Pipelining is not supported with Envoy:

1. A httpbin server was configured and two consecutive GET requests were sent via telnet with the following command:

    ```
  (echo -en "GET /delay/5 HTTP/1.1\nHost: httpbin:8000\nConnection: keep-alive\n\nGET /delay/2 HTTP/1.1\nHost: httpbin:8000\n\n"; sleep 10) | telnet httpbin 8000
    ```
    
2. Envoy Sidecar proxy on will accept/queue all pipelined requests sent by the Client App, then:

    - Forward 1st request to upstream server. i.e. (HTTPBIN)
    - Wait for the response.
    - Forward the response to Client App.
    - Forward second request in the same TCP connection.
    - Wait for the response.
    - Forward the response to Client App.

3. Looking through the packet capture, we see that instead of pipelining, they are serializing all the requests and process them sequentially. 

Recommended Actions

None

Additional Information

None

#### Related Content

- [Connection pooling](https://www.envoyproxy.io/docs/envoy/latest/intro/arch_overview/upstream/connection_pooling)
- [Envoy service proxy processes HTTP 1.1 requests with increasing delay on a persistent connection](https://github.com/envoyproxy/envoy/issues/13376)

F5 Support engineers who work directly with customers to resolve issues create this content. Support Solution articles give you fast access to mitigation, workaround, or troubleshooting suggestions.
