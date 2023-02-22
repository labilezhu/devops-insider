# Kernel Git Skill

## clone stable only
```
git clone git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git linux-stable

git clone https://github.com/gregkh/linux.git linux-stable

git clone --depth 1 --branch v5.15  https://github.com/gregkh/linux.git linux-stable


```


## clone single branch

If you do not want to download whole kernel commit history (which is well above 1 GiB), you can download only such part of the kernel Git repo that leads to your desired branch. E.g. to locally checkout the Ubuntu kernel in version 4.5, you'd do:
```bash
git clone --depth 1 --branch v4.18 \
  git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
```