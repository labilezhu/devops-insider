https://fabianlee.org/2017/02/20/monitoring-java-jmx-exploration-from-the-console-using-jmxterm/


### jmxterm
https://docs.cyclopsgroup.org/jmxterm
https://docs.cyclopsgroup.org/jmxterm/user-manual
https://github.com/jiaqi/jmxterm

Use case:
https://fabianlee.org/2017/02/20/monitoring-java-jmx-exploration-from-the-console-using-jmxterm/


```bash
java -jar ~/Downloads/jmxterm-1.0.2-uber.jar -l $PID

bean -d java.lang java.lang:name=CodeHeap\\ 'non-nmethods',type=MemoryPool


echo "get -d java.lang -b java.lang:name=CodeHeap\\ 'non-nmethods',type=MemoryPool Usage " | java -jar ./jmxterm-1.0.2-uber.jar -n -l $PID
Usage = { 
  committed = 2555904;
  init = 2555904;
  max = 5836800;
  used = 1492864;
 };

```

```
➜  bin java -jar ~/Downloads/jmxterm-1.0.2-uber.jar --help   
[USAGE]
  jmxterm <OPTIONS>
[DESCRIPTION]
  Main executable of JMX terminal CLI tool
[OPTIONS]
  -a --appendtooutput           With this flag, the outputfile is preserved and content is appended to it
  -e --exitonfailure            With this flag, terminal exits for any Exception
  -h --help                     Show usage of this command line
  -i --input          <value>   Input script file. There can only be one input file. "stdin" is the default value which means console input
  -n --noninteract              Non interactive mode. Use this mode if input doesn't come from human or jmxterm is embedded
  -o --output         <value>   Output file, stdout or stderr. Default value is stdout
  -p --password       <value>   Password for user/password authentication
  -s --sslrmiregistry           Whether the server's RMI registry is protected with SSL/TLS
  -l --url            <value>   Location of MBean service. It can be <host>:<port> or full service URL.
  -u --user           <value>   User name for user/password authentication
  -v --verbose        <value>   Verbose level, could be silent|brief|verbose. Default value is brief
[NOTE]
  Without any option, this command opens an interactive command line based console. With a given input file, commands in file will be executed and process ends after file is processed

```