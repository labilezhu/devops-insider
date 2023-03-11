# 图解 eBPF 基础库 libbpf 示例流程

如果你学习 eBPF。那么你不久就会发现，几乎所有 eBPF 的架构，包括 `BCC`/`bpftrace` 在内，都在用户态使用了 `libbpf` 这个 helper lib。



![image-20230311151420617](libbpf-bootstrap-study-1-minimal.assets/image-20230311151420617.png)

*from [BPF Performance Tools] - Brendan Gregg*



在  `BCC`/`bpftrace`  刚兴起之时， libbpf 还是个小 BB，API 抽象不足，只能用作底层库，为 BCC/bpftrace 做脚手架。但当小 BB 发展到 libbpf 1.0 时，情况有了变化。API 界面的抽象和友好程度增加了，加上人们开始对轻量化 BPF 运行条件，一次编译到处运行([BPF CO-RE](https://nakryiko.com/posts/bpf-portability-and-co-re/)) 的期待越来越高。 libbpf 开始备受关注。



而我个人想研究 libbpf 的目的是想了解内核加载与运行 BPF 程序的接口和抽象概念。要了解内核如何抽象 BPF 设计和概念，当然可以直接看源码。但我认为看 BPF 用户状与内核态的 API 设计，足以应对大部分需求。也比较好控制方向不迷失。



同理，学习 libbpf 当然可以直接看原码。但时候，看设计者如何使用（调用）自己的源码，比直接看源码来得直观与快捷。一个例子胜过 N 个道理。

libbpf 提供了一个很好的，轻度使用 libbpf 的示例

https://github.com/libbpf/libbpf-bootstrap





Test BPF call:

```c
libc.so.6!syscall() (syscall.S:38)
sys_bpf(unsigned int size, union bpf_attr * attr, enum bpf_cmd cmd) (/home/labile/opensource/libbpf-bootstrap/libbpf/src/bpf.c:75)
sys_bpf_fd(unsigned int size, union bpf_attr * attr, enum bpf_cmd cmd) (/home/labile/opensource/libbpf-bootstrap/libbpf/src/bpf.c:83)
bpf_map_create(enum bpf_map_type map_type, const char * map_name, __u32 key_size, __u32 value_size, __u32 max_entries, const struct bpf_map_create_opts * opts) (/home/labile/opensource/libbpf-bootstrap/libbpf/src/bpf.c:201)
probe_kern_array_mmap() (/home/labile/opensource/libbpf-bootstrap/libbpf/src/libbpf.c:4674)
kernel_supports(const struct bpf_object * obj, enum kern_feature_id feat_id) (/home/labile/opensource/libbpf-bootstrap/libbpf/src/libbpf.c:4909)
kernel_supports(const struct bpf_object * obj, enum kern_feature_id feat_id) (/home/labile/opensource/libbpf-bootstrap/libbpf/src/libbpf.c:4897)
bpf_object__sanitize_maps(struct bpf_object * obj) (/home/labile/opensource/libbpf-bootstrap/libbpf/src/libbpf.c:7356)
bpf_object_load(struct bpf_object * obj) (/home/labile/opensource/libbpf-bootstrap/libbpf/src/libbpf.c:7735)
bpf_object__load(struct bpf_object * obj) (/home/labile/opensource/libbpf-bootstrap/libbpf/src/libbpf.c:7786)
bpf_object__load_skeleton(struct bpf_object_skeleton * s) (/home/labile/opensource/libbpf-bootstrap/libbpf/src/libbpf.c:12375)
minimal_bpf__load(struct minimal_bpf * obj) (/home/labile/opensource/libbpf-bootstrap/examples/c/.output/minimal.skel.h:90)
main(int argc, char ** argv) (/home/labile/opensource/libbpf-bootstrap/examples/c/minimal.c:34)
```

