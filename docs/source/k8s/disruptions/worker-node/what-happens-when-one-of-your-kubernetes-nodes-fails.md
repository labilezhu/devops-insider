# What happens when one of your Kubernetes nodes fails?

> [https://www.upnxtblog.com/index.php/2021/06/22/what-happens-when-one-of-your-kubernetes-nodes-fails/amp/](https://www.upnxtblog.com/index.php/2021/06/22/what-happens-when-one-of-your-kubernetes-nodes-fails/amp/)


This section details what happens during a node failure and what is expected during the recovery.

1. Post node failure, in about **1 minute**, `kubectl get nodes` will report `NotReady` state.
2. In about **5 minutes**, the states of all the pods running on the `NotReady` node will change to either `Unknown` or `NodeLost`.This is based on [pod eviction timeout](https://kubernetes.io/docs/concepts/architecture/nodes/#condition) settings, the default duration is **five minutes**.
3. Irrespective of deployments _(StatefuleSet or Deployment)_, Kubernetes will automatically evict the pod on the failed node and then try to recreate a new one with old volumes.
4. If the node is back online within 5 – 6 minutes of the failure, Kubernetes will restart pods, unmount, and re-mount volumes.
5. If incase if evicted pod gets stuck in `Terminating` state and the attached volumes cannot be released/reused, the newly created pod(s) will get stuck in `ContainerCreating` state. There are 2 options now:
    1. Either to forcefully delete the stuck pods manually (or)
    2. Kubernetes will take about another [**6 minutes**](https://github.com/kubernetes/kubernetes/blob/5e31799701123c50025567b8534e1a62dbc0e9f6/pkg/controller/volume/attachdetach/attach_detach_controller.go#L95) to delete the VolumeAttachment objects associated with the Pod and then finally detach the volume from the lost Node and allow it to be used by the new pod(s).

In summary, if the failed node is recovered later, Kubernetes will restart those terminating pods, detach the volumes, wait for the old VolumeAttachment cleanup, and reuse (re-attach & re-mount) the volumes. Typically these steps would take about 1 ~ 7 minutes.


