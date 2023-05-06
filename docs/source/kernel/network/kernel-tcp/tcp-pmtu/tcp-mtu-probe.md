# TCP的MTU探测功能


> [TCP的MTU探测功能](https://blog.csdn.net/sinat_20184565/article/details/89163458)

Linux内核默认情况下未开启TCP的MTU探测功能。

$ cat /proc/sys/net/ipv4/tcp_mtu_probing
0

当TCP客户端发起连接建立请求时，在函数tcp_connect_init中调用TCP的MTU探测初始化函数tcp_mtup_init。如上所述默认情况下enabled为零，使用MSS最大限制值mss_clamp加上TCP头部长度和网络层头部长度作为MTU探测的上限值，下限值由函数tcp_mss_to_mtu通过基础MSS值计算得到。

void tcp_mtup_init(struct sock *sk)
{
    struct tcp_sock *tp = tcp_sk(sk);
    struct inet_connection_sock *icsk = inet_csk(sk);

    icsk->icsk_mtup.enabled = net->ipv4.sysctl_tcp_mtu_probing > 1;
    icsk->icsk_mtup.search_high = tp->rx_opt.mss_clamp + sizeof(struct tcphdr) + icsk->icsk_af_ops->net_header_len;
    icsk->icsk_mtup.search_low = tcp_mss_to_mtu(sk, net->ipv4.sysctl_tcp_base_mss);
    icsk->icsk_mtup.probe_size = 0;
    if (icsk->icsk_mtup.enabled)
        icsk->icsk_mtup.probe_timestamp = tcp_jiffies32;
}
TCP的MTU探测的基础MSS默认初始化为1024，见宏定义TCP_BASE_MSS，可通过PROC文件tcp_base_mss修改其值。

$ cat /proc/sys/net/ipv4/tcp_base_mss
1024
$ cat /proc/sys/net/ipv4/tcp_probe_threshold
8
$ cat /proc/sys/net/ipv4/tcp_probe_interval
600

内核定义值如下：

#define TCP_BASE_MSS        1024
#define TCP_PROBE_INTERVAL  600
#define TCP_PROBE_THRESHOLD 8

static int __net_init tcp_sk_init(struct net *net)
{
    net->ipv4.sysctl_tcp_base_mss = TCP_BASE_MSS;
    net->ipv4.sysctl_tcp_probe_threshold = TCP_PROBE_THRESHOLD;
    net->ipv4.sysctl_tcp_probe_interval = TCP_PROBE_INTERVAL;
}
MTU到MSS推算

基础函数__tcp_mtu_to_mss如下。首先，路径MTU减去网路层头部和TCP标准头部的长度得到一个MSS的长度。其次，对于IPv6而言，需要在减去一个分片头部的长度；再次，MSS值不能够超过协商的限定值mss_clamp（其不包括TCP选项长度）；之后减去扩展头部长度，例如IP选项的长度；最终得到的MSS值不能小于48，否则使用48，即全部TCP选项的长度40加上8字节的数据。需要注意的是__tcp_mtu_to_mss函数在计算过程中并没有考虑TCP选项的长度。

static inline int __tcp_mtu_to_mss(struct sock *sk, int pmtu)
{
    /* Calculate base mss without TCP options: It is MMS_S - sizeof(tcphdr) of rfc1122 */
    mss_now = pmtu - icsk->icsk_af_ops->net_header_len - sizeof(struct tcphdr);

    /* IPv6 adds a frag_hdr in case RTAX_FEATURE_ALLFRAG is set */
    if (icsk->icsk_af_ops->net_frag_header_len) {
        const struct dst_entry *dst = __sk_dst_get(sk);
        if (dst && dst_allfrag(dst))
            mss_now -= icsk->icsk_af_ops->net_frag_header_len;
    }
    
    if (mss_now > tp->rx_opt.mss_clamp)
        mss_now = tp->rx_opt.mss_clamp;
    mss_now -= icsk->icsk_ext_hdr_len;
    if (mss_now < 48)
        mss_now = 48;
    return mss_now;
}

函数tcp_mtu_to_mss为对以上函数的封装，将函数__tcp_mtu_to_mss的返回结果值减去了选项的长度，即其考虑了TCP大部分选项的长度，但是并没有将SACK的选项考虑在内。

int tcp_mtu_to_mss(struct sock *sk, int pmtu)
{
    /* Subtract TCP options size, not including SACKs */
    return __tcp_mtu_to_mss(sk, pmtu) - (tcp_sk(sk)->tcp_header_len - sizeof(struct tcphdr));
}
另外一个与MTU到MSS转换相关的函数为tcp_bound_to_half_wnd。如果当前最大的接收窗口大于TCP_MSS_DEFAULT（536），将发送MSS限制在最大接收窗口的一半内；否则，对于小于536的小窗口，发送MSS的值不应超出整个窗口的值。

static inline int tcp_bound_to_half_wnd(struct tcp_sock *tp, int pktsize)
{
    /* When peer uses tiny windows, there is no use in packetizing are enough packets in the pipe for fast recovery.
     * On the other hand, for extremely large MSS devices, handling smaller than MSS windows in this way does make sense. */
    if (tp->max_window > TCP_MSS_DEFAULT)
        cutoff = (tp->max_window >> 1);
    else
        cutoff = tp->max_window;

    if (cutoff && pktsize > cutoff)
        return max_t(int, cutoff, 68U - tp->tcp_header_len);
    else
        return pktsize;
}
最终由函数tcp_sync_mss负责更新当前TCP发送使用的MSS值mss_cache，其包括除SACK选项之外的所有其它选项的长度，参见函数tcp_mtu_to_mss。并且使用tcp_bound_to_half_wnd控制发送MSS与对端接收窗口的比例关系，得到mss_cache的值，同时也更新当前连接的路径PMTU值icsk_pmtu_cookie。需要注意的是，如果启用了TCP的MTU探测功能，最后的发送mss_cache的值取当前值和以search_low计算得到的mss值两者之间的较小值。

unsigned int tcp_sync_mss(struct sock *sk, u32 pmtu)
{
    if (icsk->icsk_mtup.search_high > pmtu)
        icsk->icsk_mtup.search_high = pmtu;

    mss_now = tcp_mtu_to_mss(sk, pmtu);
    mss_now = tcp_bound_to_half_wnd(tp, mss_now);
    
    icsk->icsk_pmtu_cookie = pmtu;
    if (icsk->icsk_mtup.enabled)
        mss_now = min(mss_now, tcp_mtu_to_mss(sk, icsk->icsk_mtup.search_low));
    tp->mss_cache = mss_now;
    
    return mss_now;
}

当前发送MSS的计算有函数tcp_current_mss实现，其更进一步的考虑了TCP的SACK选项数据长度，最后得到TCP发送路径使用的MSS值。

unsigned int tcp_current_mss(struct sock *sk)
{
    const struct dst_entry *dst = __sk_dst_get(sk);
    mss_now = tp->mss_cache;
    if (dst) {
        u32 mtu = dst_mtu(dst);
        if (mtu != inet_csk(sk)->icsk_pmtu_cookie)
            mss_now = tcp_sync_mss(sk, mtu);
    }
    header_len = tcp_established_options(sk, NULL, &opts, &md5) + sizeof(struct tcphdr);
    if (header_len != tp->tcp_header_len) {
        int delta = (int) header_len - tp->tcp_header_len;
        mss_now -= delta;
    }
    return mss_now;
}
MTU探测

在TCP发送路径中，如果由于TCP_CORK选项累计了数据包，或者合并了小微数据包，在数据发送函数tcp_write_xmit中，内核调用tcp_mtu_probe发送MTU探测报文。首要条件是，没有正在运行的探测、拥塞状态在初始态、拥塞窗口大于11，并且没有SACK，以上条件只要有一个不满足，就不能进行MTU探测。

static int tcp_mtu_probe(struct sock *sk)
{
    if (likely(!icsk->icsk_mtup.enabled || icsk->icsk_mtup.probe_size || inet_csk(sk)->icsk_ca_state != TCP_CA_Open ||
           tp->snd_cwnd < 11 || tp->rx_opt.num_sacks || tp->rx_opt.dsack))
        return -1;
选取的MTU探测值probe_size等于下限值search_low加上其与上限值search_high之差的1/2，即search_low+1/2*（search_high-search_low）转换得到的MSS值，作为新的MTU探测值。但是如果新选取的值大于search_high对应的MSS值，或者上限值与下限值小于设定的探测阈值tcp_probe_threshold（8），返回失败。

    mss_now = tcp_current_mss(sk);
    probe_size = tcp_mtu_to_mss(sk, (icsk->icsk_mtup.search_high + icsk->icsk_mtup.search_low) >> 1);
    size_needed = probe_size + (tp->reordering + 1) * tp->mss_cache;
    interval = icsk->icsk_mtup.search_high - icsk->icsk_mtup.search_low;
    
    if (probe_size > tcp_mtu_to_mss(sk, icsk->icsk_mtup.search_high) || interval < net->ipv4.sysctl_tcp_probe_threshold) {
        /* Check whether enough time has elaplased for another round of probing. */
        tcp_mtu_check_reprobe(sk);
        return -1;
    }
在判断可发送之后，内核将开始组建数据长度为probe_size值的探测报文，新分配一个nskb，将发送队列sk_write_queue前端的数据包拷贝probe_size的数据到新的nskb中，释放拷贝过的数据包。

    nskb = sk_stream_alloc_skb(sk, probe_size, GFP_ATOMIC, false);
    skb = tcp_send_head(sk);
    
    TCP_SKB_CB(nskb)->seq = TCP_SKB_CB(skb)->seq;
    TCP_SKB_CB(nskb)->end_seq = TCP_SKB_CB(skb)->seq + probe_size;
    TCP_SKB_CB(nskb)->tcp_flags = TCPHDR_ACK;
    
    tcp_insert_write_queue_before(nskb, skb, sk);
    tcp_highest_sack_replace(sk, skb, nskb);
    tcp_for_write_queue_from_safe(skb, next, sk) {
    
    }
    tcp_init_tso_segs(nskb, nskb->len);
最后，调用TCP传输函数发送此数据包。

    /* We're ready to send.  If this fails, the probe will be resegmented into mss-sized pieces by tcp_write_xmit().*/
    if (!tcp_transmit_skb(sk, nskb, 1, GFP_ATOMIC)) {
        /* Decrement cwnd here because we are sending effectively two packets. */
        tp->snd_cwnd--;
        tcp_event_new_data_sent(sk, nskb);
    
    icsk->icsk_mtup.probe_size = tcp_mss_to_mtu(sk, nskb->len);
        tp->mtu_probe.probe_seq_start = TCP_SKB_CB(nskb)->seq;
        tp->mtu_probe.probe_seq_end = TCP_SKB_CB(nskb)->end_seq;
        return 1;
    }
    return -1;
}
以上tcp_mtu_probe函数中，如果遇到新的探测值probe_size大于search_high对应的MSS值，或者上限值与下限值小于设定的探测阈值tcp_probe_threshold（8），在返回错误之前，内核调用tcp_mtu_check_reprobe重新安排一次探测。前提是本次探测与上一次探测的时间间隔不小于设定的间隔值tcp_probe_interval（600），即10分钟。

static inline void tcp_mtu_check_reprobe(struct sock *sk)
{
    interval = net->ipv4.sysctl_tcp_probe_interval;
    delta = tcp_jiffies32 - icsk->icsk_mtup.probe_timestamp;
    if (unlikely(delta >= interval * HZ)) {
        int mss = tcp_current_mss(sk);

    /* Update current search range */
        icsk->icsk_mtup.probe_size = 0;
        icsk->icsk_mtup.search_high = tp->rx_opt.mss_clamp + sizeof(struct tcphdr) + icsk->icsk_af_ops->net_header_len;
        icsk->icsk_mtup.search_low = tcp_mss_to_mtu(sk, mss);
    
    /* Update probe time stamp */
        icsk->icsk_mtup.probe_timestamp = tcp_jiffies32;
    }
}

在tcp_ack函数中，如果接收到的时旧的ACK或者重复的SACK报文等非正常ACK报文，将调用tcp_fastretrans_alert处理。其中拥塞状态为非TCP_CA_Recovery，即处于TCP_CA_Loss或者其它，并且未确认的序号等于探测报文的开始序号，内核判断探测失败。

static void tcp_fastretrans_alert(struct sock *sk, const u32 prior_snd_una, bool is_dupack, int *ack_flag, int *rexmit)
{
    switch (icsk->icsk_ca_state) {
    case TCP_CA_Recovery:
        break;
    case TCP_CA_Loss:
    default:
        /* MTU probe failure: don't reduce cwnd */
        if (icsk->icsk_ca_state < TCP_CA_CWR && icsk->icsk_mtup.probe_size && tp->snd_una == tp->mtu_probe.probe_seq_start) {
            tcp_mtup_probe_failed(sk);
            /* Restores the reduction we did in tcp_mtup_probe() */
            tp->snd_cwnd++;
            tcp_simple_retransmit(sk);
            return;
        }
    }
}

探测失败处理函数tcp_mtup_probe_failed如下，如果探测失败的话，表明探测的MTU值过大，将探测值减去1赋值给探测上限值search_high。

static void tcp_mtup_probe_failed(struct sock *sk)
{
    struct inet_connection_sock *icsk = inet_csk(sk);

    icsk->icsk_mtup.search_high = icsk->icsk_mtup.probe_size - 1;
    icsk->icsk_mtup.probe_size = 0;
    NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPMTUPFAIL);
}
TCP接收到的ACK报文处理tcp_ack函数，检查重传队列tcp_clean_rtx_queue。如果发送了MTU探测报文（probe_size有值），并且探测报文的结束序号已被对端确认，意味值探测成功，由函数tcp_mtup_probe_success进行处理。

static int tcp_clean_rtx_queue(struct sock *sk, u32 prior_fack, u32 prior_snd_una, struct tcp_sacktag_state *sack)
{
    if (flag & FLAG_ACKED) {
        flag |= FLAG_SET_XMIT_TIMER;  /* set TLP or RTO timer */
        if (unlikely(icsk->icsk_mtup.probe_size && !after(tp->mtu_probe.probe_seq_end, tp->snd_una))) {
            tcp_mtup_probe_success(sk);
        }
    }
}
函数tcp_mtup_probe_success的调用表明探测成功，意味着连接的MTU值已增加，随即将探测值probe_size赋予MTU探测的下限值，复位probe_size。由函数tcp_sync_mss同步TCP的MSS值。

static void tcp_mtup_probe_success(struct sock *sk)
{
    tp->prior_ssthresh = tcp_current_ssthresh(sk);
    tp->snd_cwnd = tp->snd_cwnd * tcp_mss_to_mtu(sk, tp->mss_cache) / icsk->icsk_mtup.probe_size;
    tp->snd_cwnd_cnt = 0;
    tp->snd_cwnd_stamp = tcp_jiffies32;
    tp->snd_ssthresh = tcp_current_ssthresh(sk);

    icsk->icsk_mtup.search_low = icsk->icsk_mtup.probe_size;
    icsk->icsk_mtup.probe_size = 0;
    tcp_sync_mss(sk, icsk->icsk_pmtu_cookie);
}
路径黑洞探测
如果TCP的重传次数超过了tcp_retries1限定的值（默认为3），表明网络可能存在一定的问题，但是此时内核并不会结束此连接（直到超过tcp_retries2的值），此时TCP重传处理函数，将发起MTU探测。

static int tcp_write_timeout(struct sock *sk)
{
    if ((1 << sk->sk_state) & (TCPF_SYN_SENT | TCPF_SYN_RECV)) {
    } else {
        if (retransmits_timed_out(sk, net->ipv4.sysctl_tcp_retries1, 0)) {
            /* Black hole detection */
            tcp_mtu_probing(icsk, sk);
            dst_negative_advice(sk);
        }
    }
}
如下函数tcp_mtu_probing，如果tcp_mtu_probing等于零表示为开启，直接返回。如果未使能，此处使能并且更新探测开始时间戳。否则，内核在此时将降低探测所使用的MTU值，首先将探测的下限值search_low减低一半，但是不能低于规定的最小值tcp_base_mss，还要至少大于TCP头部最大长度加上8个TCP数据长度的结果减去TCP实际头部长度的值。

static void tcp_mtu_probing(struct inet_connection_sock *icsk, struct sock *sk)
{
    /* Black hole detection */
    if (!net->ipv4.sysctl_tcp_mtu_probing)
        return;

    if (!icsk->icsk_mtup.enabled) {
        icsk->icsk_mtup.enabled = 1;
        icsk->icsk_mtup.probe_timestamp = tcp_jiffies32;
    } else {
        mss = tcp_mtu_to_mss(sk, icsk->icsk_mtup.search_low) >> 1;
        mss = min(net->ipv4.sysctl_tcp_base_mss, mss);
        mss = max(mss, 68 - tcp_sk(sk)->tcp_header_len);
        icsk->icsk_mtup.search_low = tcp_mss_to_mtu(sk, mss);
    }
    tcp_sync_mss(sk, icsk->icsk_pmtu_cookie);
}

内核版本 4.15
————————————————
版权声明：本文为CSDN博主「redwingz」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/sinat_20184565/article/details/89163458
