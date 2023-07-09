

## Code
java.net.InetAddress
> https://docs.oracle.com/javase/8/docs/api/java/net/InetAddress.html

#### Host Name Resolution

Host name-to-IP address *resolution* is accomplished through the use of a combination of local machine configuration information and network naming services such as the Domain Name System (DNS) and Network Information Service(NIS). The particular naming services(s) being used is by default the local machine configured one. For any host name, its corresponding IP address is returned.

*Reverse name resolution* means that for any IP address, the host associated with the IP address is returned.

The InetAddress class provides methods to resolve host names to their IP addresses and vice versa.

#### InetAddress Caching

The InetAddress class has a cache to store successful as well as unsuccessful host name resolutions.

By default, when a security manager is installed, in order to protect against DNS spoofing attacks, the result of positive host name resolutions are cached forever. When a security manager is not installed, the default behavior is to cache entries for a finite (implementation dependent) period of time. The result of unsuccessful host name resolution is cached for a very short period of time (10 seconds) to improve performance.

If the default behavior is not desired, then a Java security property can be set to a different Time-to-live (TTL) value for positive caching. Likewise, a system admin can configure a different negative caching TTL value when needed.

Two Java security properties control the TTL values used for positive and negative host name resolution caching:

>
> **networkaddress.cache.ttl**
>
> Indicates the caching policy for successful name lookups from the name service. The value is specified as as integer to indicate the number of seconds to cache the successful lookup. The default setting is to cache for an implementation specific period of time.
>
> A value of -1 indicates "cache forever".
>
>
> **networkaddress.cache.negative.ttl** (default: 10)
>
> Indicates the caching policy for un-successful name lookups from the name service. The value is specified as as integer to indicate the number of seconds to cache the failure for un-successful lookups.
>
> A value of 0 indicates "never cache". A value of -1 indicates "cache forever".
>


## Doc

> https://docs.oracle.com/javase/8/docs/api/java/net/doc-files/net-properties.html

Address Cache
-------------

The java.net package, when doing name resolution, uses an address cache for both security and performance reasons. Any address resolution attempt, be it forward (name to IP address) or reverse (IP address to name), will have its result cached, whether it was successful or not, so that subsequent identical requests will not have to access the naming service. These properties allow for some tuning on how the cache is operating.

-   **networkaddress.cache.ttl** (default: see below)
    Value is an integer corresponding to the number of seconds successful name lookups will be kept in the cache. A value of -1, or any other negative value for that matter, indicates a "cache forever" policy, while a value of 0 (zero) means no caching. The default value is -1 (forever) if a security manager is installed, and implementation specific when no security manager is installed.

-   **networkaddress.cache.negative.ttl** (default: 10)
    Value is an integer corresponding to the number of seconds an unsuccessful name lookup will be kept in the cache. A value of -1, or any negative value, means "cache forever", while a value of 0 (zero) means no caching.

Since these 2 properties are part of the security policy, they are not set by either the -D option or the System.setProperty() API, instead they are set as security properties.


## /opt/jdk1.8.0_261/jre/lib/security/java.security

```bash
bash-4.4$ cat /opt/jdk1.8.0_261/jre/lib/security/java.security

#
# The Java-level namelookup cache policy for successful lookups:
#
# any negative value: caching forever
# any positive value: the number of seconds to cache an address for
# zero: do not cache
#
# default value is forever (FOREVER). For security reasons, this
# caching is made forever when a security manager is set. When a security
# manager is not set, the default behavior in this implementation
# is to cache for 30 seconds.
#
# NOTE: setting this to anything other than the default value can have
#       serious security implications. Do not set it unless
#       you are sure you are not exposed to DNS spoofing attack.
#
#networkaddress.cache.ttl=-1

# The Java-level namelookup cache policy for failed lookups:
#
# any negative value: cache forever
# any positive value: the number of seconds to cache negative lookup results
# zero: do not cache
#
# In some Microsoft Windows networking environments that employ
# the WINS name service in addition to DNS, name service lookups
# that fail may take a noticeably long time to return (approx. 5 seconds).
# For this reason the default caching policy is to maintain these
# results for 10 seconds.
#
#
networkaddress.cache.negative.ttl=10

```


## Dump
> https://stackoverflow.com/questions/1835421/java-dns-cache-viewer