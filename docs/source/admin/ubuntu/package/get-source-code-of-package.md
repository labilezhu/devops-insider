# get source code of package

> https://www.cyberciti.biz/faq/how-to-get-source-code-of-package-using-the-apt-command-on-debian-or-ubuntu/


```conf
sudo vi /etc/apt/sources.list

#uncomment:
#deb-src http://us.archive.ubuntu.com/ubuntu/ jammy main restricted


:%s~http://us.archive.ubuntu.com/ubuntu/~https://mirrors.ustc.edu.cn/ubuntu/~g
```

```bash
sudo apt update

sudo apt-get source openjdk-17

cd /home/labile/ubunut-jdk
apt-get source openjdk-17


labile@labile-hp ➜ ubunut-jdk $ ll
total 60M
drwxrwxr-x 10 labile labile 4.0K Sep 15 14:45 openjdk-17-17.0.8.1+1~us1
-rw-r--r--  1 labile labile 481K Aug 30 17:58 openjdk-17_17.0.8.1+1~us1-0ubuntu1~22.04.debian.tar.xz
-rw-r--r--  1 labile labile 4.5K Aug 30 17:58 openjdk-17_17.0.8.1+1~us1-0ubuntu1~22.04.dsc
-rw-r--r--  1 labile labile  60M Aug 30 17:58 openjdk-17_17.0.8.1+1~us1.orig.tar.xz
```

- openjdk-17_17.0.8.1+1~us1.orig.tar.xz : upstream source
- openjdk-17_17.0.8.1+1~us1-0ubuntu1~22.04.dsc : A description file with .dsc ending contains the name of the package, both, in its filename as well as content (after the Source: keyword)
- openjdk-17_17.0.8.1+1~us1-0ubuntu1~22.04.debian.tar.xz : A tarball, with any changes made to upstream source, plus all the files created for the Debian package


It is also possible to build packages:
```bash
apt-get --build source openjdk-17
```

## source package info

- https://packages.ubuntu.com/source/jammy/openjdk-17

Source:
https://salsa.debian.org/openjdk-team/openjdk/tree/openjdk-17

Changelog:
http://changelogs.ubuntu.com/changelogs/pool/universe/o/openjdk-17/openjdk-17_17.0.8.1+1~us1-0ubuntu1~22.04/changelog



## Ref.

- https://askubuntu.com/questions/1299684/which-openjdk-build-does-ubuntu-use
- https://access.redhat.com/documentation/en-us/openjdk/8/html/installing_and_using_openjdk_8_for_rhel/installing-and-configuring-debug-symbols