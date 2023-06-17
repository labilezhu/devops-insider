---
date: 2022-04-07T16:12:15+09:00
draft: true
weight: 1
enableToc: true
enableBlogBreadcrumb: false
tocLevels: ["h2", "h3", "h4"]
---

> https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_atomic_host/7/html/container_security_guide/linux_capabilities_and_seccomp

Secure Computing Mode (seccomp) is a kernel feature that allows you to filter system calls to the kernel from a container. The combination of restricted and allowed calls are arranged in profiles, and you can pass different profiles to different containers. Seccomp provides more fine-grained control than capabilities, giving an attacker a limited number of syscalls from the container.

The default seccomp profile for docker is a JSON file and can be viewed here: <https://github.com/docker/docker/blob/master/profiles/seccomp/default.json>. It blocks 44 system calls out of more than 300 available.Making the list stricter would be a trade-off with application compatibility. A table with a significant part of the blocked calls and the reasoning for blocking can be found here: <https://docs.docker.com/engine/security/seccomp/>.

Seccomp uses the Berkeley Packet Filter (BPF) system, which is programmable on the fly so you can make a custom filter. You can also limit a certain syscall by also customizing the conditions on how or when it should be limited. A seccomp filter replaces the syscall with a pointer to a BPF program, which will execute that program instead of the syscall. All children to a process with this filter will inherit the filter as well. The docker option which is used to operate with seccomp is `--security-opt`. To explicitly use the default policy for a container, the command will be:

```
# docker run --security-opt seccomp=/path/to/default/profile.json <container>
```

If you want to specify your own policy, point the option to your custom file:

```
# docker run --security-opt seccomp=/path/to/custom/profile.json <container>
```

