

## Doc

> https://docs.oracle.com/javase/8/docs/api/java/net/doc-files/net-properties.html
There are a few standard system properties used to alter the mechanisms and behavior of the various classes of the `java.net` package.

* http.keepalive (default: true)
Indicates if persistent connections should be supported. They improve performance by allowing the underlying socket connection to be reused for multiple http requests. If this is set to true then persistent connections will be requested with HTTP 1.1 servers.

* http.maxConnections (default: 5)
If HTTP keepalive is enabled (see above) this value determines the maximum number of idle connections that will be simultaneously kept alive, per destination.


> https://docs.oracle.com/javase/8/docs/technotes/guides/net/http-keepalive.html

* http.maxConnections=<int>  default: 5
Indicates the maximum number of connections per destination to **be kept alive at any given time**

## JDK java source


![](java-call-hierarchy.png)

##### sun.net.www.http.KeepAliveCache

```java
package sun.net.www.http;
public class KeepAliveCache {


    static final int LIFETIME = 5000;
    private Thread keepAliveTimer = null;


    /* maximum # keep-alive connections to maintain at once
     * This should be 2 by the HTTP spec, but because we don't support pipe-lining
     * a larger value is more appropriate. So we now set a default of 5, and the value
     * refers to the number of idle connections per destination (in the cache) only.
     * It can be reset by setting system property "http.maxConnections".
     */
    static final int MAX_CONNECTIONS = 5;
    static int getMaxConnections() {
        if (result == -1) {
            result = java.security.AccessController.doPrivileged(
                new sun.security.action.GetIntegerAction("http.maxConnections",
                                                         MAX_CONNECTIONS))
                .intValue();
            if (result <= 0)
                result = MAX_CONNECTIONS;
        }
            return result;
    }


    /* return a still valid, unused HttpClient */
    synchronized void put(HttpClient h) {
        if (size() >= KeepAliveCache.getMaxConnections()) {
            h.closeServer(); // otherwise the connection remains in limbo
        } else {
            push(new KeepAliveEntry(h, System.currentTimeMillis()));
        }
    }


    /**
     * Register this URL and HttpClient (that supports keep-alive) with the cache
     * @param url  The URL contains info about the host and port
     * @param http The HttpClient to be cached
     */
    public synchronized void put(final URL url, Object obj, HttpClient http) {
        boolean startThread = (keepAliveTimer == null);
        if (!startThread) {
            if (!keepAliveTimer.isAlive()) {
                startThread = true;
            }
        }
        if (startThread) {
          ...
                    this.keepAliveTimer = new Thread(grp, cache, "Keep-Alive-Timer");
                    keepAliveTimer.setDaemon(true);
                    keepAliveTimer.setPriority(Thread.MAX_PRIORITY - 2);
                    // Set the context class loader to null in order to avoid
                    // keeping a strong reference to an application classloader.
                    keepAliveTimer.setContextClassLoader(null);
                    keepAliveTimer.start();
          ...
        }

        KeepAliveKey key = new KeepAliveKey(url, obj);
        ClientVector v = super.get(key);

        if (v == null) {
            int keepAliveTimeout = http.getKeepAliveTimeout();
            v = new ClientVector(keepAliveTimeout > 0?
                                 keepAliveTimeout*1000 : LIFETIME);
            v.put(http);
            super.put(key, v);
        } else {
            v.put(http);
        }        

    }

    class KeepAliveKey {
        private String      protocol = null;
        private String      host = null;
        private int         port = 0;
        private Object      obj = null; // additional key, such as socketfactory

        /**
        * Constructor
        *
        * @param url the URL containing the protocol, host and port information
        */
        public KeepAliveKey(URL url, Object obj) {
            this.protocol = url.getProtocol();
            this.host = url.getHost();
            this.port = url.getPort();
            this.obj = obj;
        }
    }


}

```


##### sun.net.www.http.HttpClient

```java
public class HttpClient extends NetworkClient {
      // whether this httpclient comes from the cache
    protected boolean cachedHttpClient = false;

    // target host, port for the URL
    protected String host;
    protected int port;    

    /* where we cache currently open, persistent connections */
    protected static KeepAliveCache kac = new KeepAliveCache();

    volatile boolean keepingAlive = false;     /* this is a keep-alive connection */
    volatile boolean disableKeepAlive;/* keep-alive has been disabled for this
                                         connection - this will be used when
                                         recomputing the value of keepingAlive */

    int keepAliveConnections = -1;    /* number of keep-alives left */

    /**Idle timeout value, in milliseconds. Zero means infinity,
     * iff keepingAlive=true.
     * Unfortunately, we can't always believe this one.  If I'm connected
     * through a Netscape proxy to a server that sent me a keep-alive
     * time of 15 sec, the proxy unilaterally terminates my connection
     * after 5 sec.  So we have to hard code our effective timeout to
     * 4 sec for the case where we're using a proxy. *SIGH*
     */
    int keepAliveTimeout = 0;

    private HttpClient(URL url, String proxyHost, int proxyPort,
                       boolean proxyDisabled)
        throws IOException {
        this(url, proxyDisabled ? Proxy.NO_PROXY :
             newHttpProxy(proxyHost, proxyPort, "http"), -1);
    }    
    

    /* return it to the cache as still usable, if:
     * 1) It's keeping alive, AND
     * 2) It still has some connections left, AND
     * 3) It hasn't had a error (PrintStream.checkError())
     * 4) It hasn't timed out
     *
     * If this client is not keepingAlive, it should have been
     * removed from the cache in the parseHeaders() method.
     */

    public void finished() {
        if (reuse) /* will be reused */
            return;
        keepAliveConnections--;
        poster = null;
        if (keepAliveConnections > 0 && isKeepingAlive() &&
               !(serverOutput.checkError())) {
            /* This connection is keepingAlive && still valid.
             * Return it to the cache.
             */
            putInKeepAliveCache();
        } else {
            closeServer();
        }
    }    

    private boolean parseHTTPHeader(MessageHeader responses, ProgressSource pi, HttpURLConnection httpuc) {
        keepAliveConnections = -1;
        keepAliveTimeout = 0;

      ...
                    keep = responses.findValue("Connection");
      ...
        String te = responses.findValue("Transfer-Encoding");
        if (te != null && te.equalsIgnoreCase("chunked")) {
          ...
          this.keepingAlive = !disableKeepAlive;//true
          ...
        } else {
          ...
                this.keepingAlive = !disableKeepAlive;//true
          ...
        }
        
      ...
      //如果有 Keep-Alive header
                        HeaderParser p = new HeaderParser(
                            responses.findValue("Keep-Alive"));
                        /* default should be larger in case of proxy */
                        this.keepAliveConnections = p.findInt("max", usingProxy?50:5);
                        this.keepAliveTimeout = p.findInt("timeout", usingProxy?60:5);
      //如果有无 Keep-Alive header 且是 HTTP/1.1 或以上
                    /*
                     * We're talking 1.1 or later. Keep persistent until
                     * the server says to close.
                     */
                    if (keep != null || disableKeepAlive) {
                        /*
                         * The only Connection token we understand is close.
                         * Paranoia: if there is any Connection header then
                         * treat as non-persistent.
                         */
                        keepAliveConnections = 1;
                    } else {
                        keepAliveConnections = 5;
                    }

      ...
            if (useKeepAliveStream)   {
                // Wrap KeepAliveStream if keep alive is enabled.
                logFinest("KeepAlive stream used: " + url);
                serverInput = new KeepAliveStream(serverInput, pi, cl, this);
                failedOnce = false;
            }
            else        {
                serverInput = new MeteredStream(serverInput, pi, cl);
            }      

    }

    protected synchronized void putInKeepAliveCache() {
        if (inCache) {
            assert false : "Duplicate put to keep alive cache";
            return;
        }
        inCache = true;
        kac.put(url, null, this);
    }

    /* return it to the cache as still usable, if:
     * 1) It's keeping alive, AND
     * 2) It still has some connections left, AND
     * 3) It hasn't had a error (PrintStream.checkError())
     * 4) It hasn't timed out
     *
     * If this client is not keepingAlive, it should have been
     * removed from the cache in the parseHeaders() method.
     */

    public void finished() {
        if (reuse) /* will be reused */
            return;
        keepAliveConnections--;
        poster = null;
        if (keepAliveConnections > 0 && isKeepingAlive() &&
               !(serverOutput.checkError())) {
            /* This connection is keepingAlive && still valid.
             * Return it to the cache.
             */
            putInKeepAliveCache();
        } else {
            closeServer();
        }
    }

    // HttpClient 复用入口


}
```

### Info
> http://www.itersblog.com/archives/3.html


1、如果在HttpURLConnection的header中加入Connection: close，则此连接不会启用keepAlive

2、想要启用keepAlive，程序请求完毕后，必须调用HttpURLConnection.getInputStream().close()（表示归还长连接给缓存，以供下次同host:port的请求重用底层socket连接），而不能调用HttpURLConnection.disconnect()（表示关闭底层socket连接，不会启用keepAlive）

3、keepAliveTimeout首先从http response header中获取，如果没有取到，则默认为5秒，sun.net.www.http.KeepAliveCache.java中有一个线程，每5秒执行一次，检查缓存的连接的空闲时间是否超过keepAliveTimeout，如果超过则关闭连接。从KeepAliveCache中获取缓存的连接时也会检查获取到的连接的空闲时间是否超过keepAliveTimeout，如果超过则关闭连接，并且获取下一个连接，再执行以上检查，直达获取到空闲时间在keepAliveTimeout以内的缓存连接为此。

## Protocol
### HTTP 1.1
> https://en.wikipedia.org/wiki/HTTP_persistent_connection

In HTTP 1.1, **all connections are considered persistent unless declared otherwise**.[2] The HTTP persistent connections do not use separate keepalive messages, they just allow multiple requests to use a single connection. However, the default connection timeout of Apache httpd 1.3 and 2.0 is as little as 15 seconds[3][4] and just 5 seconds for Apache httpd 2.2 and above.[5][6] The advantage of a short timeout is the ability to deliver multiple components of a web page quickly while not consuming resources to run multiple server processes or threads for too long.[7]


#### Keepalive header


##### Connection:

> https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Connection
> https://www.w3.org/Protocols/rfc2616/rfc2616-sec8.html

```http
HTTP/1.1 200 OK
Connection: Keep-Alive
Content-Encoding: gzip
Content-Type: text/html; charset=utf-8
Date: Thu, 11 Aug 2016 15:23:13 GMT
Keep-Alive: timeout=5, max=1000
Last-Modified: Mon, 25 Jul 2016 04:32:39 GMT
Server: Apache
(body)
```


```
Connection: close
```
* `close`
    Indicates that either the client or the server would like to close the connection. This is the default on HTTP/1.0 requests.

##### Keep-Alive:
> https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Keep-Alive
> https://datatracker.ietf.org/doc/html/draft-thomson-hybi-http-timeout-03#section-2

```http
HTTP/1.1 200 OK
Connection: Keep-Alive
Content-Encoding: gzip
Content-Type: text/html; charset=utf-8
Date: Thu, 11 Aug 2016 15:23:13 GMT
Keep-Alive: timeout=5, max=1000
Last-Modified: Mon, 25 Jul 2016 04:32:39 GMT
Server: Apache
(body)
```


`timeout`: An integer representing the time in seconds that the host will allow an idle connection to remain open before it is closed. A connection is idle if no data is sent or received by a host. A host may keep an idle connection open for longer than timeout seconds,  **but the host should attempt to retain a connection for at least timeout seconds** .

`max`: indicating the maximum number of requests that can be sent on this connection before closing it. This parameter is deprecated.  Any limit on requests can be enforced by sending "Connection: close" and closing the connection. Unless 0, this value is ignored for non-pipelined connections as another request will be sent in the next response. An HTTP pipeline can use it to limit the pipelining.




##### pec
> https://www.w3.org/Protocols/rfc2616/rfc2616-sec8.html

 An HTTP/1.1 server MAY assume that a HTTP/1.1 client intends to maintain a persistent connection unless a Connection header including the connection-token "close" was sent in the request. If the server chooses to close the connection immediately after sending the response, it SHOULD send a Connection header including the connection-token close.

An HTTP/1.1 client MAY expect a connection to remain open, but would decide to keep it open based on whether the response from a server contains a Connection header with the connection-token close. In case the client does not want to maintain a connection for more than that request, it SHOULD send a Connection header including the connection-token close. 

 If either the client or the server sends the close token in the Connection header, that request becomes the last one for the connection. 


## Ref
https://docs.oracle.com/javase/8/docs/api/java/net/doc-files/net-properties.html
https://docs.oracle.com/javase/8/docs/technotes/guides/net/http-keepalive.html
https://stackoverflow.com/questions/53720957/java-exact-meaning-http-maxconnections
https://segmentfault.com/a/1190000020370495
https://www.mocklab.io/blog/which-java-http-client-should-i-use-in-2020/