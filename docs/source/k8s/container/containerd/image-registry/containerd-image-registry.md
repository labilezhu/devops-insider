

# Registry Configuration





- [Pull request: Add support for using a host registry dir in cri #4978, After containerd 1.5 2021-3-16 ](https://github.com/containerd/containerd/pull/4978)
  - Exposes the hosts dir config to CRI. This will be used instead of the cri registry config in the main config toml.



# Registry Configuration - Introduction

> https://github.com/containerd/containerd/blob/main/docs/hosts.md



New and additional registry hosts config support has been implemented in containerd v1.5 for the `ctr` client (the containerd tool for admins/developers), containerd image service clients, and CRI clients such as `kubectl` and `crictl`.

Configuring registries, for these clients, will be done by specifying (optionally) a `hosts.toml` file for each desired registry host in a configuration directory. **Note**: Updates under this directory do not require restarting the containerd daemon.



### Specifying the Configuration Directory



*The old CRI config pattern for specifying registry.mirrors and registry.configs has been **DEPRECATED**.* You should now point your registry `config_path` to the path where your `hosts.toml` files are located.

Modify your `config.toml` (default location: `/etc/containerd/config.toml`) as follows:

```
version = 2

[plugins."io.containerd.grpc.v1.cri".registry]
   config_path = "/etc/containerd/certs.d"
```



cat /etc/containerd/certs.d/docker.io/hosts.toml 

```toml
server = "https://docker.io"

[host."https://registry-1.docker.io"]
  capabilities = ["pull", "resolve"]
```



### CRI

Although we have deprecated the old CRI config pattern for specifying registry.mirrors and registry.configs you can still specify your credentials via [CRI config](https://github.com/containerd/containerd/blob/main/docs/cri/registry.md#configure-registry-credentials).



## Registry Configuration - Examples



### Setup a Local Mirror for Docker

```toml
server = "https://registry-1.docker.io"    # Exclude this to not use upstream

[host."https://public-mirror.example.com"]
  capabilities = ["pull"]                  # Requires less trust, won't resolve tag to digest from this host
[host."https://docker-mirror.internal"]
  capabilities = ["pull", "resolve"]
  ca = "docker-mirror.crt"                 # Or absolute path /etc/containerd/certs.d/docker.io/docker-mirror.crt
```

### Bypass TLS Verification Example

To bypass the TLS verification for a private registry at `192.168.31.250:5000`

Create a path and `hosts.toml` text at the path "/etc/containerd/certs.d/docker.io/hosts.toml" with following or similar contents:

```toml
server = "https://registry-1.docker.io"

[host."http://192.168.31.250:5000"]
  capabilities = ["pull", "resolve", "push"]
  skip_verify = true
```









