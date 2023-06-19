# containerd-shim-runc-v2

> https://github.com/containerd/containerd/blob/main/runtime/v2/README.md



## Flow

The following sequence diagram shows the flow of actions when `ctr run` command executed.


```mermaid
sequenceDiagram
    participant ctr
    participant containerd
    participant shim

    autonumber

    ctr->>containerd: Create container
    Note right of containerd: Save container metadata
    containerd-->>ctr: Container ID

    ctr->>containerd: Create task

    %% Start shim
    containerd-->shim: Prepare bundle
    containerd->>shim: Execute binary: containerd-shim-runc-v2 start
    shim->shim: Start TTRPC server
    shim-->>containerd: Respond with address: unix://containerd/container.sock

    containerd-->>shim: Create TTRPC client

    %% Schedule task

    Note right of containerd: Schedule new task

    containerd->>shim: TaskService.CreateTaskRequest
    shim-->>containerd: Task PID

    containerd-->>ctr: Task ID

    %% Start task

    ctr->>containerd: Start task

    containerd->>shim: TaskService.StartRequest
    shim-->>containerd: OK

    %% Wait task

    ctr->>containerd: Wait task

    containerd->>shim: TaskService.WaitRequest
    Note right of shim: Block until task exits
    shim-->>containerd: Exit status

    containerd-->>ctr: OK

    Note over ctr,shim: Other task requests (Kill, Pause, Resume, CloseIO, Exec, etc)

    %% Kill signal

    opt Kill task

    ctr->>containerd: Kill task

    containerd->>shim: TaskService.KillRequest
    shim-->>containerd: OK

    containerd-->>ctr: OK

    end

    %% Delete task

    ctr->>containerd: Task Delete

    containerd->>shim: TaskService.DeleteRequest
    shim-->>containerd: Exit information

    containerd->>shim: TaskService.ShutdownRequest
    shim-->>containerd: OK

    containerd-->shim: Close client
    containerd->>shim: Execute binary: containerd-shim-runc-v2 delete
    containerd-->shim: Delete bundle

    containerd-->>ctr: Exit code
```





