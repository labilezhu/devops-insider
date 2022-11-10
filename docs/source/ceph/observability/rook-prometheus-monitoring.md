# Rook prometheus monitoring

> [https://rook.io/docs/rook/v1.9/ceph-monitoring.html](https://rook.io/docs/rook/v1.9/ceph-monitoring.html)



Each Rook Ceph cluster has some built in metrics collectors/exporters for monitoring with [Prometheus](https://prometheus.io/).

If you do not have Prometheus running, follow the steps below to enable monitoring of Rook. If your cluster already contains a Prometheus instance, it will automatically discover Rook’s scrape endpoint using the standard `prometheus.io/scrape` and `prometheus.io/port` annotations.

> **NOTE**: This assumes that the Prometheus instances is searching all your Kubernetes namespaces for Pods with these annotations. If prometheus is already installed in a cluster, it may not be configured to watch for third-party service monitors such as for Rook. Normally you should be able to add the prometheus annotations “prometheus.io/scrape=true” and prometheus.io/port={port} and prometheus would automatically configure the scrape points and start gathering metrics. If prometheus isn’t configured to do this, see the [prometheus operator docs](https://github.com/prometheus-community/helm-charts/tree/main/charts/kube-prometheus-stack#prometheusioscrape).



## Grafana Dashboards

The dashboards have been created by [@galexrt](https://github.com/galexrt). For feedback on the dashboards please reach out to him on the [Rook.io Slack](https://slack.rook.io/).

> **NOTE**: The dashboards are only compatible with Grafana 7.2.0 or higher.
>
> Also note that the dashboards are updated from time to time, to fix issues and improve them.

The following Grafana dashboards are available:

- [Ceph - Cluster](https://grafana.com/dashboards/2842)
- [Ceph - OSD (Single)](https://grafana.com/dashboards/5336)
- [Ceph - Pools](https://grafana.com/dashboards/5342)

## Special Cases

### CSI Liveness

To integrate CSI liveness and grpc into ceph monitoring we will need to deploy a service and service monitor.

```
kubectl create -f csi-metrics-service-monitor.yaml
```

This will create the service monitor to have promethues monitor CSI

### Collecting RBD per-image IO statistics

RBD per-image IO statistics collection is disabled by default. This can be enabled by setting `enableRBDStats: true` in the CephBlockPool spec. Prometheus does not need to be restarted after enabling it.

### Using custom label selectors in Prometheus

If Prometheus needs to select specific resources, we can do so by injecting labels into these objects and using it as label selector.

```YAML
apiVersion: ceph.rook.io/v1
kind: CephCluster
metadata:
  name: rook-ceph
  namespace: rook-ceph
  [...]
spec:
  [...]
  labels:
    monitoring:
      prometheus: k8s
  [...]
```

### Horizontal Pod Scaling using Kubernetes Event-driven Autoscaling (KEDA)

Using metrics exported from the Prometheus service, the horizontal pod scaling can use the custom metrics other than CPU and memory consumption. It can be done with help of Prometheus Scaler provided by the [KEDA](https://keda.sh/docs/2.5/scalers/prometheus/). See the [KEDA deployment guide](https://keda.sh/docs/2.5/deploy/) for details.

Following is an example to autoscale RGW:

```YAML
apiVersion: keda.sh/v1alpha1
kind: ScaledObject
metadata:
 name: rgw-scale
 namespace: rook-ceph
spec:
 scaleTargetRef:
   kind: Deployment
   name: rook-ceph-rgw-my-store-a # deployment for the autoscaling
 minReplicaCount: 1
 maxReplicaCount: 5
 triggers:
 - type: prometheus
   metadata:
     serverAddress: http://rook-prometheus.rook-ceph.svc:9090
     metricName: collecting_ceph_rgw_put
     query: |
       sum(rate(ceph_rgw_put[2m])) # promethues query used for autoscaling
     threshold: "90"
```
