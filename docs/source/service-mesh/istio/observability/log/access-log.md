# Access Log


Pod Level enable access log:

```yaml
apiVersion: networking.istio.io/v1alpha3
kind: EnvoyFilter
metadata:
  name: envoy-scheduler-access-log-my-pod
spec:
  workloadSelector:
    labels:
      "app.kubernetes.io/name": "my-pod"
  configPatches:
  - applyTo: NETWORK_FILTER
    match:
      context: SIDECAR_INBOUND # If we only want to log input traffic of app
      proxy:
        metadata:
          #the POD name here:
          "NAME": "my-pod-6d879c7bd5-xhjk5" #the POD name here
      listener:
        filterChain:
          filter:
            name: "envoy.http_connection_manager"
    patch:
      operation: MERGE
      value: 
        typed_config:
          "@type": "type.googleapis.com/envoy.config.filter.network.http_connection_manager.v2.HttpConnectionManager"
          access_log:
            - name: envoy.my_access_log.file
              typed_config:
                "@type": type.googleapis.com/envoy.extensions.access_loggers.file.v3.FileAccessLog
                path: "/dev/stdout"
```