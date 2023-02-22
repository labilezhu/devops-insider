
> https://tenzir.com/blog/production-debugging-bpftrace-uprobes/


to trace:
```cpp
void scheduled_actor::enqueue(mailbox_element_ptr ptr, execution_unit* eu);
```


Every C++ object with virtual functions has a corresponding function table embedded in the binary that is used to determine where to jump for any virtual function calls, and how to perform a dynamic_cast<> between objects. It looks like this:

```c
struct vtable {
  long long int parent_offset;
  void* typeinfo;
  // void* thunks[nfuncs];
  char* anchor; // aka thunks[0], this is where the vtbl* in the class points to
};
```

```cpp
class mailbox_element : public intrusive::singly_linked<mailbox_element>,
                        public memory_managed,
                        public message_view { /* ... */ };
```


In terms of bpftrace it looks like this:


```c
struct mailbox_element {
  char *primary_vtbl;   // vtbl for caf::memory_managed and caf::mailbox_element
  void* next;           // instance of caf::single_linked
  char* secondary_vtbl; // vtbl for caf::message
  // ---
  char data_members[40]; // sender, message id, ...
};
```





























```c
struct vtable {
  long long int parent_offset;
  void* typeinfo;
  // void* thunks[nfuncs];
  char* anchor; // aka thunks[0], this is where the vtbl* in the class points to
};

struct vector {
  void* first;
  void* last;
  void* end_of_storage;
};

// -- caf-specific types

struct mailbox_element {
  char *primary_vtbl;   // vtbl for caf::memory_managed and caf::mailbox_element
  void* next;           // instance of caf::single_linked
  char* secondary_vtbl; // vtbl for caf::message
  // ---
  char* sender;
  char data_members[32]; // sizeof(caf::mailbox_element) == 64
};

struct mailbox_element_ptr {
  struct mailbox_element* ptr;
  void* dtor;
};

struct message_data {
  char* vtbl;
  char data_members[16]; // sizeof(caf::message_data) == 24
};

struct message {
  char* vtbl; // vtbl for caf::message and caf::type_erased_tuple
  struct message_data* data_ptr;
};

struct mailbox_element_wrapper {
  struct mailbox_element parent;
  struct message msg;
};

// typeinfo for caf::(anonymous namespace)::mailbox_element_wrapper
#define _ZTIN3caf12_GLOBAL__N_123mailbox_element_wrapperE 0x14c03a8

uprobe:/usr/bin/vast:_ZN3caf15scheduled_actor7enqueueESt10unique_ptrINS_15mailbox_elementENS_6detail8disposerEEPNS_14execution_unitE {
  $receiver = reg("di"); // "this" pointer
  $element_ptr = (struct mailbox_element_ptr*) reg("si");
  $element = $element_ptr->ptr;
  // 0x40 is the offset between the actor control block and the `this`-pointer of the actor
  $sender = $element->sender + 0x40;
  $anchor = $element->primary_vtbl;
  $vtable = (struct vtable*) ($anchor - 16);
  $typeinfo = $vtable->typeinfo;
  // caf::mailbox_element_wrapper
  if ($vtable->typeinfo == _ZTIN3caf12_GLOBAL__N_123mailbox_element_wrapperE) {
    $wrapper = (struct mailbox_element_wrapper*)$element;
    $msg_typeinfo = (struct vtable*)($wrapper->msg.vtbl - 16);
    $msg_anchor = $wrapper->msg.vtbl;
    $mdata = $wrapper->msg.data_ptr;
    $data_anchor = $mdata->vtbl;
    $data_typeinfo = ((struct vtable*)($mdata->vtbl - 16))->typeinfo;
    $mvt = $mdata->vtbl;
    // Most likely this will be caf::detail::tuple_vals<Ts...>, so we can get the
    // message types by just decoding the typeinfo symbol.
    printf("%llx -> %llx (message) 0x%llx\n", $sender, $receiver, $data_typeinfo);
  } else {
    printf("%llx -> %llx (mailbox element) 0x%llx\n", $sender, $receiver, $vtable->typeinfo);
  }
}
```