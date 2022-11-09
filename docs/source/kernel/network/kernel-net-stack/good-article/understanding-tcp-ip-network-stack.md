# Understanding TCP/IP Network Stack & Writing Network Apps

> [https://www.cubrid.org/blog/3826497](https://www.cubrid.org/blog/3826497)



## Data Receiving

Now, let's take a look at how data is received. Data receiving is a procedure for how the network stack handles a packet coming in. **Figure 3** shows how the network stack handles a packet received.

![operation_process_by_each_layer_of_tcp_ip_for_data_received.png](understanding-tcp-ip-network-stack.assets/5d7322505c1ea20bb741d7b3904ef8f9-16680017300371.png)

**Figure 3: Operation Process by Each Layer of TCP/IP Network Stack for Handling Data Received.**

First, the NIC writes the packet onto its memory. It checks whether the packet is valid by performing the CRC check and then sends the packet to the memory buffer of the host. This buffer is a memory that has already been requested by the driver to the kernel and allocated for receiving packets. After the buffer has been allocated, the driver tells the memory address and size to the NIC. When there is no host memory buffer allocated by the driver even though the NIC receives a packet, the NIC may drop the packet.

After sending the packet to the host memory buffer, the NIC sends an interrupt to the host OS.

Then, the driver checks whether it can handle the new packet or not. So far, the driver-NIC communication protocol defined by the manufacturer is used.

When the driver should send a packet to the upper layer, the packet must be wrapped in a packet structure that the OS uses for the OS to understand the packet. For example, **sk\_buff** of Linux, **mbuf** of BSD-series kernel, and **NET\_BUFFER\_LIST** of Microsoft Windows are the packet structures of the corresponding OS. The driver sends the wrapped packets to the upper layer.

The Ethernet layer checks whether the packet is valid and then de-multiplexes the upper protocol (network protocol). At this time, it uses the ethertype value of the Ethernet header. The IPv4 ethertype value is **0x0800**. It removes the Ethernet header and then sends the packet to the IP layer.

The IP layer also checks whether the packet is valid. In other words, it checks the IP header checksum. It logically determines whether it should perform IP routing and make the local system handle the packet, or send the packet to the other system. If the packet must be handled by the local system, the IP layer de-multiplexes the upper protocol (transport protocol) by referring to the proto value of the IP header. The TCP proto value is 6. It removes the IP header and then sends the packet to the TCP layer.

Like the lower layer, the TCP layer checks whether the packet is valid. It also checks the TCP checksum. As mentioned before, since the current network stack uses the checksum offload, the TCP checksum is computed by NIC, not by the kernel.

Then it searches the `TCP control block` where the packet is connected. At this time, `<source IP, source port, target IP, target port>` of the packet is used as an identifier. After searching the connection, it performs the protocol to handle the packet. If it has received new data, it adds the data to the receive socket buffer. According to the TCP state, it can send a new TCP packet (for example, an ACK packet). Now TCP/IP receiving packet handling has completed.

The size of the receive socket buffer is the TCP receive window. To a certain point, the TCP throughput increases when the receive window is large. In the past, the socket buffer size had been adjusted on the application or the OS configuration. The latest network stack has a function to adjust the receive socket buffer size, i.e., the receive window, automatically.

When the application calls the read system call, the area is changed to the kernel area and the data in the socket buffer is copied to the memory in the user area. The copied data is removed from the socket buffer. And then the TCP is called. The TCP increases the receive window because there is new space in the socket buffer. And it sends a packet according to the protocol status. If no packet is transferred, the system call is terminated.



## Control Flow in the Stack

Now, we will take a more detailed look at the internal flow of the Linux network stack. Like a subsystem which is not a network stack, a network stack basically runs as the event-driven way that reacts when the event occurs. Therefore, there is no separated thread to execute the stack. **Figure 1** and **Figure 3** showed the simplified diagrams of control flow. **Figure 4** below illustrates more exact control flow.

![control_flow_in_stack.png](understanding-tcp-ip-network-stack.assets/bf6bff327d8abd33c8e44258c98ccce6-16680018104554.png)

**Figure 4: Control Flow in the Stack.**

- At `Flow (1)` in Figure 4, an application calls a system call to execute (use) the TCP. For example, calls the read system call and the write system call and then executes TCP. However, there is no packet transmission.

- `Flow (2)` is same as `Flow (1)` if it requires packet transmission after executing TCP. It creates a packet and sends down the packet to the driver. A queue is in front of the driver. The packet comes into the queue first, and then the queue implementation structure decides the time to send the packet to the driver. This is queue discipline (**qdisc**) of Linux. The function of Linux traffic control is to manipulate the `qdisc`. The default qdisc is a simple First-In-First-Out (FIFO) queue. By using another qdisc, operators can achieve various effects such as artificial packet loss, packet delay, transmission rate limit, etc. At `Flow (1)` and `Flow (2)`, the process thread of the application also executes the driver.

- `Flow (3)` shows the case in which the timer used by the TCP has expired. For example, when the **TIME\_WAIT** timer has expired, the TCP is called to delete the connection.

- Like `Flow (3)`, `Flow (4)` is the case in which the timer used by the TCP has expired and the TCP execution result packet should be transmitted. For example, when the retransmit timer has expired, the packet of which ACK has not been received is transmitted.

`Flow (3)` and `Flow (4)` show the procedure of executing the timer softirq that has processed the `timer interrupt`.

When the NIC driver receives an interrupt, it frees the transmitted packet. In most cases, execution of the driver is terminated here. `Flow (5)` is the case of packet accumulation in the transmit queue. The driver requests softirq and the softirq handler executes the transmit queue to send the accumulated packet to the driver.

When the NIC driver receives an interrupt and finds a newly received packet, it requests softirq. The softirq that processes the received packet calls the driver and transmits the received packet to the upper layer. In Linux, processing the received packet as shown above is called New API (NAPI). It is similar to polling because the driver does not directly transmit the packet to the upper layer, but the upper layer directly gets the packet. The actual code is called NAPI poll or poll.

`Flow (6)` shows the case that completes execution of TCP, and `Flow (7)` shows the case that requires additional packet transmission. All of `Flow (5)`, `(6)`, and `(7)` are executed by the softirq which has processed the NIC interrupt.





