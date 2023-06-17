

## Namespaces in operation, part 3: PID namespaces

> https://lwn.net/Articles/531419/
> https://lwn.net/Articles/532748/


> https://man7.org/linux/man-pages/man5/proc.5.html
```
       /proc/[pid]/root
              UNIX and Linux support the idea of a per-process root of
              the filesystem, set by the chroot(2) system call.  This
              file is a symbolic link that points to the process's root
              directory, and behaves in the same way as exe, and fd/*.

              Note however that this file is not merely a symbolic link.
              It provides the same view of the filesystem (including
              namespaces and the set of per-process mounts) as the
              process itself.  An example illustrates this point.  In
              one terminal, we start a shell in new user and mount
              namespaces, and in that shell we create some new mounts:

                  $ PS1='sh1# ' unshare -Urnm
                  sh1# mount -t tmpfs tmpfs /etc  # Mount empty tmpfs at /etc
                  sh1# mount --bind /usr /dev     # Mount /usr at /dev
                  sh1# echo $$
                  27123

              In a second terminal window, in the initial mount
              namespace, we look at the contents of the corresponding
              mounts in the initial and new namespaces:

                  $ PS1='sh2# ' sudo sh
                  sh2# ls /etc | wc -l                  # In initial NS
                  309
                  sh2# ls /proc/27123/root/etc | wc -l  # /etc in other NS
                  0                                     # The empty tmpfs dir
                  sh2# ls /dev | wc -l                  # In initial NS
                  205
                  sh2# ls /proc/27123/root/dev | wc -l  # /dev in other NS
                  11                                    # Actually bind
                                                        # mounted to /usr
                  sh2# ls /usr | wc -l                  # /usr in initial NS
                  11

              In a multithreaded process, the contents of the
              /proc/[pid]/root symbolic link are not available if the
              main thread has already terminated (typically by calling
              pthread_exit(3)).

              Permission to dereference or read (readlink(2)) this
              symbolic link is governed by a ptrace access mode
              PTRACE_MODE_READ_FSCREDS check; see ptrace(2).
```