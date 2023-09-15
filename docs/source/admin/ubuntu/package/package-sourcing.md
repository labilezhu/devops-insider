
## Ubuntu 包溯源


#### 文件来自什么样包 - finding the package a file belongs
```bash
labile@worknode1:/etc/apt/sources.list.d$ dpkg -S /usr/bin/crictl
cri-tools: /usr/bin/crictl
```
#### 包来自什么源 - How do I find out which repository a package comes from

```bash
apt-cache policy cri-tools

cri-tools:
  Installed: 1.19.0-00
  Candidate: 1.19.0-00
  Version table:
 *** 1.19.0-00 500
        500 https://apt.kubernetes.io kubernetes-xenial/main amd64 Packages
        100 /var/lib/dpkg/status
     1.13.0-01 500
        500 https://apt.kubernetes.io kubernetes-xenial/main amd64 Packages


```