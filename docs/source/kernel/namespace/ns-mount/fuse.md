

> https://lwn.net/Articles/133848/
> Jamie Lokier [noticed](https://lwn.net/Articles/133853/) that each process's root directory is accessible via `/proc/*pid*/root`. A new process can be put into another process's namespace simply by setting its root with `chroot()`. If all works as it seems it should, a user-space solution can be envisioned: write a privileged daemon process which can create namespaces and, using file descriptor passing, hand them to interested processes. Those processes can then `chroot()` into that namespace. `chroot()` is a privileged operation, but the code to handle the user side of this operation could be hidden within a PAM module and made completely invisible.
