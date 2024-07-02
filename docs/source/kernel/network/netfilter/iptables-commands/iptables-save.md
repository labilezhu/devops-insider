# iptables-save iptables-restore 

## iptables-save iptables-restore 文件格式



> https://unix.stackexchange.com/questions/400163/netfilter-iptables-restore-file-format-documentation



I found my answer, sort-of...
As best as I can tell, there is no document. However, in reading the source code I've uncovered how it works.

Lines starting with `#` are for comments and are not parsed.
Blank lines are ignored.
`*` marks the *table* name.
`:` marks the *chain*, followed by the default *policy* and optionally the packet and byte counters.
byte counters can precede a rule.
Rules are exactly as given on the command line less the table name.
Each table section must end with `COMMIT`.

The good news is that the syntax for the actual rules is just as it says in `man iptables`.

```
# iptables-restore format
*<table>
:<chain> <policy> [<packets_count>:<bytes_count>]
<optional_counter><rule>
... more rules ...
COMMIT
```

\-

```
# iptables-restore example
*filter
:INPUT DROP [0:0]
-A INPUT -s 127.0.0.1 -p tcp -m tcp --dport 9000 -J ACCEPT
-A INPUT -p tcp -m tcp --dport 9000 -j REJECT --reject-with icmp-port-unreachable
COMMIT
```



> https://serverfault.com/questions/373871/dont-understand-00-iptable-syntax/373875#373875



The [0:0] or [1280:144299] or whatever are the count of [`Packets`:`Bytes`] that have been trough the chain . They are saved when you run an `iptables-save` command and are used by the `iptables-restore` command to initialise the counters.

------

The `Packets` and `bytes` values can be useful *for some statistical purposes. Issuing an iptables-save command with the -c argument would then make it possible for us to reboot without breaking our statistical and accounting routines.* (Quoted from *Iptables Tutorial 1.2.2* - by Oskar Andreasson) Conclusively, restoring the `iptables` rules with `Packets` and `bytes` specified will not affect the rule behavior, just will keep a "stateful" track of `Packets` respectively `bytes` that match the rule.



