
```
sudo gdb -p `pgrep -x -f /usr/bin/containerd`


handle SIGPIPE ignore
Signal        Stop      Print   Pass to program Description
SIGPIPE       Yes       Yes     No              Broken pipe

handle all nostop pass
set detach-on-fork off
set follow-fork-mode child

catch syscall execve
condition 1 '$_streq((char*)$rdi, "/usr/sbin/runc")'

catch syscall unshare
catch syscall pivot_root

# catch exec
c&

detach inferior 2

detach 
inferior 1
c&

c&
inferior 1


interrupt

break open if strcmp($rdi,"/usr/sbin/runc") == 0

(gdb) catch syscall access
Catchpoint 1 (syscall 'access' [21])
(gdb) condition 1 $_streq((char *)$rdi, "/usr/sbin/runc")


(gdb) info proc exe
process 33988
exe = '/usr/bin/ls'


b __libc_start_main

watch -d  "ps -ef --forest| grep -A15 49357"


```

https://sourceware.org/gdb/onlinedocs/gdb/Convenience-Funs.html


```yaml
kubectl delete StatefulSet netshoot-worknode5

kubectl apply -f - <<"EOF"

apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: netshoot-worknode5
  labels:
    app: netshoot-worknode5
spec:
  replicas: 1
  selector:
    matchLabels:
      app: netshoot-worknode5
  template:
    metadata:
      labels:
          app.kubernetes.io/name: netshoot-worknode5
          app: netshoot-worknode5
      annotations:
        sidecar.istio.io/inject: "false"
    spec:
      restartPolicy: Always
      imagePullSecrets:
      - name: docker-registry-key
      containers:
      - name: main-app
        command:
        - /bin/sleep
        image: docker.io/nicolaka/netshoot:latest
        imagePullPolicy: IfNotPresent
        args: ["10d"]
        ports:
        - containerPort: 8080
          protocol: TCP
          name: http      
        - containerPort: 8070
          protocol: TCP
          name: http-m   
        - containerPort: 8079
          protocol: TCP
          name: grpc  
          
      affinity:
        nodeAffinity:
          requiredDuringSchedulingIgnoredDuringExecution:
            nodeSelectorTerms:
            - matchExpressions:
              - key: "kubernetes.io/hostname"
                operator: In
                values:
                - "worknode5" 

EOF
```

## Ref
 - https://developers.redhat.com/articles/2022/12/27/debugging-binaries-invoked-scripts-gdb#debugging_a_binary_run_from_a_wrapper_script_via_exec

