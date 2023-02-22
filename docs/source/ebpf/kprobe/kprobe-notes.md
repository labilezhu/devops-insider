

## Install kernel debug info

```
echo "deb http://ddebs.ubuntu.com $(lsb_release -cs) main restricted universe multiverse
deb http://ddebs.ubuntu.com $(lsb_release -cs)-updates main restricted universe multiverse
deb http://ddebs.ubuntu.com $(lsb_release -cs)-proposed main restricted universe multiverse" | \
sudo tee -a /etc/apt/sources.list.d/ddebs.list

sudo apt install ubuntu-dbgsym-keyring

sudo apt-get update

sudo apt-get install linux-image-$(uname -r)-dbgsym
```

Ref. https://wiki.ubuntu.com/Debug%20Symbol%20Packages

