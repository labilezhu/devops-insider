https://www.pongasoft.com/blog/yan/entry/connecting_to_a_local_vm

https://docs.oracle.com/javase/8/docs/technotes/guides/management/agent.html

Setting up Monitoring and Management Programmatically
-----------------------------------------------------

As stated previously, in the Java SE platform version 6, you can create a JMX client that uses the [Attach API](https://docs.oracle.com/javase/8/docs/technotes/guides/attach/index.html) to enable out-of-the-box monitoring and management of any applications that are started on the Java SE 6 platform, without having to configure the applications for monitoring when you launch them. **The Attach API provides a way for tools to attach to and start agents in the target application**. Once an agent is running, JMX clients (and other tools) are able to obtain the JMX connector address for that agent via a property list that is maintained by the Java VM on behalf of the agents. The properties in the list are accessible from tools that use the Attach API. So, if an agent is started in an application, and if the agent creates a property to represent a piece of configuration information, then that configuration information is available to tools that attach to the application.


# Attach API
> https://docs.oracle.com/javase/8/docs/technotes/guides/attach/index.html

The Attach API is an extension that provides a mechanism to attach to a Java virtual machine. A tool written in the Java Language, uses this API to attach to a target virtual machine and load its tool agent into that virtual machine. For example, a management console might have a management agent which it uses to obtain management information from instrumented objects in a virtual machine. If the management console is required to manage an application that is running in a virtual machine that does not include the management agent, then this API can be used to attach to the target virtual machine and load the agent.


##### e.g. Start management agent
> https://docs.oracle.com/javase/8/docs/jdk/api/attach/spec/com/sun/tools/attach/VirtualMachine.html

```java
      // attach to target VM
      VirtualMachine vm = VirtualMachine.attach("2177");

      // start management agent
      Properties props = new Properties();
      props.put("com.sun.management.jmxremote.port", "5000");
      vm.startManagementAgent(props);

      // detach
      vm.detach();
```

```
public abstract void startManagementAgent(Properties agentProperties)
                                   throws IOException
Starts the JMX management agent in the target virtual machine.
The configuration properties are the same as those specified on the command line when starting the JMX management agent. In the same way as on the command line, you need to specify at least the com.sun.management.jmxremote.port property.

See the online documentation for Monitoring and Management Using JMX Technology for further details.

Parameters:
  agentProperties - A Properties object containing the configuration properties for the agent.

```

> https://www.pongasoft.com/blog/yan/entry/connecting_to_a_local_vm

```java
private static final String CONNECTOR_ADDRESS =
  "com.sun.management.jmxremote.localConnectorAddress";

private JMXServiceURL extractJMXServiceURL(pid)
{
  // attach to the target application
  com.sun.tools.attach.VirtualMachine vm = 
    com.sun.tools.attach.VirtualMachine.attach(pid.toString());

  try
  {
    // get the connector address
    String connectorAddress = 
      vm.getAgentProperties().getProperty(CONNECTOR_ADDRESS);

    // no connector address, so we start the JMX agent
    if (connectorAddress == null) {
      String agent = vm.getSystemProperties().getProperty("java.home") +
                     File.separator + "lib" + File.separator + 
                     "management-agent.jar";
      vm.loadAgent(agent);

      // agent is started, get the connector address
      connectorAddress = 
        vm.getAgentProperties().getProperty(CONNECTOR_ADDRESS);
    }

    // establish connection to connector server
    return new JMXServiceURL(connectorAddress);
  }
  finally
  {
    vm.detach()
  }
}
```

#### How jconsole local
> https://docs.oracle.com/javase/8/docs/technotes/guides/management/agent.html

com.sun.management.jmxremote

| Property | Description | Values |
| --- |  --- |  --- |
| `com.sun.management.jmxremote` | Enables the JMX remote agent and local monitoring via a JMX connector published on a private interface used by JConsole and any other local JMX clients that use the Attach API. **JConsole can use this connector if it is started by the same user as the user that started the agent. No password or access files are checked for requests coming via this connector.** | `true` / `false`. Default is `true`. |
| `com.sun.management.jmxremote. port` | Enables the JMX remote agent and creates a remote JMX connector to listen through the specified port. By default, the SSL, password, and access file properties are used for this connector. It also enables local monitoring as described for the `com.sun.management.jmxremote` property. | Port number. No default. |
| `com.sun.management.jmxremote. registry.ssl` | Binds the RMI connector stub to an RMI registry protected by SSL. | `true` / `false`. Default is `false`. |
| `com.sun.management.jmxremote. ssl` | Enables secure monitoring via SSL. If `false`, then SSL is not used. | `true` / `false`. Default is `true`. |
| `com.sun.management.jmxremote. ssl.enabled.protocols` | A comma-delimited list of SSL/TLS protocol versions to enable. Used in conjunction with `com.sun.management.jmxremote.ssl`. | Default SSL/TLS protocol version. |
| `com.sun.management.jmxremote. ssl.enabled.cipher.suites` | A comma-delimited list of SSL/TLS cipher suites to enable. Used in conjunction with `com.sun.management.jmxremote.ssl`. | Default SSL/TLS cipher suites. |
| `com.sun.management.jmxremote. ssl.need.client.auth` | If this property is `true` and the property `com.sun.management.jmxremote.ssl` is also `true`, then client authentication will be performed.It is recommended that you set this property to `true`. | `true` / `false`. Default is `false`. |
| `com.sun.management.jmxremote. authenticate` | If this property is `false` then JMX does not use passwords or access files: all users are allowed all access. | `true` / `false`. Default is `true`. |
| `com.sun.management.jmxremote. password.file` | Specifies location for password file. If `com.sun.management.jmxremote.authenticate` is `false`, then this property and the password and access files are ignored. Otherwise, the password file must exist and be in the valid format. If the password file is empty or nonexistent, then no access is allowed. | `*JRE_HOME*/lib/management/ jmxremote.password` |
| `com.sun.management.jmxremote. access.file` | Specifies location for the access file. If `com.sun.management.jmxremote.authenticate` is false, then this property and the password and access files are ignored. Otherwise, the access file must exist and be in the valid format. If the access file is empty or nonexistent, then no access is allowed. | `*JRE_HOME*/lib/management/ jmxremote.access` |
| <a id="loginconfig" name="loginconfig"></a>`com.sun.management.jmxremote.login.config` | Specifies the name of a Java Authentication and Authorization Service (JAAS) login configuration entry to use when the JMX agent authenticates users. When using this property to override the default login configuration, the named configuration entry must be in a file that is loaded by JAAS. In addition, the login modules specified in the configuration should use the name and password callbacks to acquire the user's credentials. For more information, see the API documentation for `javax.security.auth.callback.NameCallback` and `javax.security.auth.callback.PasswordCallback`. | Default login configuration is a file-based password authentication. |




> [通过Heap dump排查Java JMX连接不上的问题 ](http://hengyunabc.github.io/jmx-local-connect-problem/)