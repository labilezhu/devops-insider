---
typora-root-url: ../../..
---

# containerd

> https://github.com/containerd/containerd




## containerd CRI plugin e.g

> https://github.com/containerd/containerd/blob/main/docs/cri/architecture.md



This document describes the architecture of the `cri` plugin for `containerd`.

This plugin is an implementation of Kubernetes [container runtime interface (CRI)](https://github.com/kubernetes/kubernetes/blob/master/staging/src/k8s.io/cri-api/pkg/apis/runtime/v1/api.proto). Containerd operates on the same node as the [Kubelet](https://kubernetes.io/docs/reference/command-line-tools-reference/kubelet/). The `cri` plugin inside containerd handles all CRI service requests from the Kubelet and uses containerd internals to manage containers and container images.

The `cri` plugin uses containerd to manage the full container lifecycle and all container images. As also shown below, `cri` manages pod networking via [CNI](https://github.com/containernetworking/cni) (another CNCF project).

[![architecture](/k8s/container/cri/cri-intro.assets/architecture.png)](https://github.com/containerd/containerd/blob/main/docs/cri/architecture.png)

Let's use an example to demonstrate how the `cri` plugin works for the case when Kubelet creates a single-container pod:

- Kubelet calls the `cri` plugin, via the CRI runtime service API, to create a pod;
- `cri` creates the pod’s network namespace, and then configures it using CNI;
- `cri` uses containerd internal to create and start a special [pause container](https://www.ianlewis.org/en/almighty-pause-container) (the sandbox container) and put that container inside the pod’s cgroups and namespace (steps omitted for brevity);
- Kubelet subsequently calls the `cri` plugin, via the CRI image service API, to pull the application container image;
- `cri` further uses containerd to pull the image if the image is not present on the node;
- Kubelet then calls `cri`, via the CRI runtime service API, to create and start the application container inside the pod using the pulled container image;
- `cri` finally uses containerd internal to create the application container, put it inside the pod’s cgroups and namespace, then to start the pod’s new application container. After these steps, a pod and its corresponding application container is created and running.

## Container Lifecycle

> https://github.com/containerd/containerd/blob/main/docs/historical/design/lifecycle.md



## Container Lifecycle

While containerd is a daemon that provides API to manage multiple containers, the containers themselves are not tied to the lifecycle of containerd. Each container has a shim that acts as the direct parent for the container's processes as well as reporting the exit status and holding onto the STDIO of the container. This also allows containerd to crash and restore all functionality to containers.

### containerd

The daemon provides an API to manage multiple containers. It can handle locking in process where needed to coordinate tasks between subsystems. While the daemon does fork off the needed processes to run containers, the shim and runc, these are re-parented to the system's init.

### shim

Each container has its own shim that acts as the direct parent of the container's processes. The shim is responsible for keeping the IO and/or pty master of the container open, writing the container's exit status for containerd, and reaping the container's processes when they exit. Since the shim owns the container's pty master, it provides an API for resizing.

Overall, a container's lifecycle is not tied to the containerd daemon. The daemon is a management API for multiple container whose lifecycle is tied to one shim per container.


```{toctree}
containerd-shim-runc-v2/containerd-shim-runc-v2.md
```