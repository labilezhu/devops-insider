# PID1 in containers

> [https://www.marcusfolkesson.se/blog/pid1-in-containers/](https://www.marcusfolkesson.se/blog/pid1-in-containers/)

## What is PID 1

The top-most process in a UNIX system has PID (Process ID) 1 and is usually the _init process_. The Init process is the first userspace application started on a system and is started by the kernel at boottime. The kernel is looking in a few predefined paths (and the _init_ kernel parameter). If no such application is found, the system will panic().

See init/main.c:kernel\_init

```c
if (!try_to_run_init_process("/sbin/init") ||
    !try_to_run_init_process("/etc/init") ||
    !try_to_run_init_process("/bin/init") ||
    !try_to_run_init_process("/bin/sh"))
    return 0;
panic("No working init found.  Try passing init= option to kernel. "
      "See Linux Documentation/admin-guide/init.rst for guidance.");
```

All processes in UNIX has a parent/child relationship which builds up a big relationship-tree. Some resources and permissions are inherited from parent to child such as UID and cgroup restrictions.

As in the real world, with parenthood comes obligations. For example: What is usually the last line of your main()-function? Hopfully something like

```c
return EXIT_SUCCESS;
```

All processes exits with an exit code that tells us if the operation was sucessful or not. Who is interested in this exit code anyway? In the real world, the parents are interested in their children's result, and so even here. The parent is responsible to wait(2) on their children to terminate just to fetch its exit code. But what if the parent died before the child?

Lets go back to the init process. The init process has several tasks, and one is to adopt "orphaned" (called zombie) child processes. Why? Because all processes will return an exit code and will not terminate completely until someone is listen for what they have to say. The init process is simply wait(2):ing on the exit code, throw it away and let the child die. Sad but true, but the child may not rest i peace otherwise. The operating system expects the init process to reap adopted children. Otherwise the children will exist in the system as a zombie and taking up some kernel resources and consume a slot in the kernel process table.

## PID 1 in containers

Containers is a concept that isolate processes in different namespaces. Example of such namespaces are PID, users, networking and filesystem. To create a container is quite simple, just create a new process with clone(2) and provide relevant flags to create new namespaces for the process.

The flags related to namespaces are listed in *include/uapi/linux/sched.h*:

```c
#define CLONE_NEWPID                0x20000000      // New pid namespace
#define CLONE_NEWCGROUP             0x02000000      // New cgroup namespace
#define CLONE_NEWUTS                0x04000000      // New utsname namespace
#define CLONE_NEWIPC                0x08000000      // New ipc namespace
#define CLONE_NEWUSER               0x10000000      // New user namespace
#define CLONE_NEWPID                0x20000000      // New pid namespace
#define CLONE_NEWNET                0x40000000      // New network namespace
```

All processes is running in a "container-context" because the processes allways executes in a namespace. On a system "without containers", all processes still have **one** common namespace that all processes is using.

When using CLONE_NEWPID, the kernel will create a new PID namespace and let the newly created process has the PID 1. As we already know, the PID 1 process has a very special task, namely to kill all orphaned children. This PID 1 process could be any application (make, bash, nginx, ftp-server or whatever) that is missing this essential adopt-and-slay-mechanism. If the reaping is not handled, it will result in zombie-processes. This was a real problem not long time ago for Docker containers (google Docker and zombies to see what I mean). Nowadays we have the *--init* flag on *docker run* to tell the container to use *tini* (https://github.com/krallin/tini), a zombie-reaping init process to run with PID 1.

## When PID 1 dies

This is the reason to why I'm writing this post. I was wondering who is killing PID 1 in a container since we learned that a PID 1 may not die under any circumstances. PID 1 in cointainers is obviosly an exception from this golden rule, but how does the kernel differentiate between init processes in different PID namespaces?

Lets follow a process to its very last breath.

The call chain we will look at is the following: do_exit()->exit_notify()->forget_original_parent()->find_child_reaper().

### do_exit()

*kernel/exit.c:do_exit()* is called when a process is going to be cleaned up from the system after it has exited or being terminated. The function is collecting the exit code, delete timers, free up resources and so on. Here is an extract of the function:

```c
......

<<<<< Collect exit code >>>>>
tsk->exit_code = code;
taskstats_exit(tsk, group_dead);


exit_mm(tsk);

if (group_dead)
    acct_process();
trace_sched_process_exit(tsk);

<<<<< Free up resources >>>>>
exit_sem(tsk);
exit_shm(tsk);
exit_files(tsk);
exit_fs(tsk);
if (group_dead)
    disassociate_ctty(1);
exit_task_namespaces(tsk);
exit_task_work(tsk);
exit_thread(tsk);

perf_event_exit_task(tsk);

sched_autogroup_exit_task(tsk);
cgroup_exit(tsk);

<<<<< Notify tasks in the same group >>>>>
exit_notify(tsk, group_dead);

.........
```



exit_notify() is to notifing our "dead group" that we are going down. One important thing to notice is that almost all resources are freed at this point. Even if the process is going into a zombie state, the footprint is relative small, but still, the zombie consumes a slot in the process table.

The size of the process table in Linux and defined by PID_MAX_LIMIT in *include/linux/threads.h*:

```c
\*\*
\* A maximum of 4 million PIDs should be enough for a while.
\* [NOTE: PID/TIDs are limited to 2^29 ~= 500+ million, see futex.h.]
\*/
#define PID_MAX_LIMIT (CONFIG_BASE_SMALL ? PAGE_SIZE * 8 : \
(sizeof(long) > 4 ? 4 * 1024 * 1024 : PID_MAX_DEFAULT))
```

The process table is indeed quite big. But if you are running for example a webserver as PID 1 that is fork(2):ing on each HTTP request. All these forks will result in a zombie and the number will escalate quite fast.

### exit_notify()

*kernel/exit.c:exit_notify()* is sending signals to all the closest relatives so that they know to properly mourn this process. In the beginning of this function, a call is made to *forget_original_parent()*:

```c
static void exit_notify(struct task_struct *tsk, int group_dead)
{
    bool autoreap;
    struct task_struct *p, *n;
    LIST_HEAD(dead);

    write_lock_irq(&tasklist_lock);
  >>>>>  forget_original_parent(tsk, &dead);
```

### forget_original_parent()

This function simply does two things

1. Make init (PID 1) inherit all the child processes
2. Check to see if any process groups have become orphaned as a result of our exiting, and if they have any stopped jobs, send them a SIGHUP and then a SIGCONT.

find_child_reaper() will help us find a proper reaper:

```c
>>>>> reaper = find_child_reaper(father);
if (list_empty(&father->children))
    return;
```

### find_child_reaper()

*kernel/exit.c:find_child_reaper()* is looking if a father is available. If a father (or other relative) is not available at all, we must be the PID 1 process.

This is the interesting part:

```c
if (unlikely(pid_ns == &init_pid_ns)) {
    panic("Attempted to kill init! exitcode=0x%08x\n",
        father->signal->group_exit_code ?: father->exit_code);
}
zap_pid_ns_processes(pid_ns);
```

*init_pid_ns* refers (declared in *kernel/pid.c*) to our **real** init process. If the real init process exits, panic the whole system since it cannot continue without an init process. If it is not, call zap_pid_ns_processes(), here we have our PID1-cannot-be-killed-exception we are looking for! We contiue following the call chain down to *zap_pid_ns_processes()*.

### zap_pid_ns_processes()

zap_pid_ns_processes function is part of the PID namespace and is located in *kernel/pid_namespace.c* The function iterates through all tasks in the same group and send signal SIGKILL to each of them.

```c
nr = next_pidmap(pid_ns, 1);
while (nr > 0) {

    rcu_read_lock();

    task = pid_task(find_vpid(nr), PIDTYPE_PID);
    if (task && !__fatal_signal_pending(task))
    >>>>> send_sig_info(SIGKILL, SEND_SIG_FORCED, task);

    rcu_read_unlock();

    nr = next_pidmap(pid_ns, nr);

}
```

## Conclusion

The PID 1 in containers is handled in a seperate way than the **real** init process. This is obvious, but now we know where the codeflow differ for PID 1 in different namespaces.

We also see that if the PID1 in a PID namespace dies, all the subprocesses will be terminated with SIGKILL. This behavior reflects the fact that the init process is essential for the correct operation of any PID namespace.
