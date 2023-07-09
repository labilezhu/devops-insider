

Unsafe.java 

```java
Unsafe.allocateMemory
```

hotspot/src/share/vm/prims/unsafe.cpp
```cpp
UNSAFE_ENTRY(jlong, Unsafe_AllocateMemory(JNIEnv *env, jobject unsafe, jlong size))
  UnsafeWrapper("Unsafe_AllocateMemory");
  size_t sz = (size_t)size;
  if (sz != (julong)size || size < 0) {
    THROW_0(vmSymbols::java_lang_IllegalArgumentException());
  }
  if (sz == 0) {
    return 0;
  }
  sz = round_to(sz, HeapWordSize);
  void* x = os::malloc(sz, mtInternal);
  if (x == NULL) {
    THROW_0(vmSymbols::java_lang_OutOfMemoryError());
  }
  //Copy::fill_to_words((HeapWord*)x, sz / HeapWordSize);
  return addr_to_java(x);
UNSAFE_END
```

hotspot/src/share/vm/runtime/os.cpp
```cpp
void* os::malloc(size_t size, MEMFLAGS flags) {
  return os::malloc(size, flags, CALLER_PC);
}

void* os::malloc(size_t size, MEMFLAGS memflags, const NativeCallStack& stack) {
  NOT_PRODUCT(inc_stat_counter(&num_mallocs, 1));
  NOT_PRODUCT(inc_stat_counter(&alloc_bytes, size));

  // Since os::malloc can be called when the libjvm.{dll,so} is
  // first loaded and we don't have a thread yet we must accept NULL also here.
  assert(!os::ThreadCrashProtection::is_crash_protected(ThreadLocalStorage::thread()),
         "malloc() not allowed when crash protection is set");

  if (size == 0) {
    // return a valid pointer if size is zero
    // if NULL is returned the calling functions assume out of memory.
    size = 1;
  }

  // NMT support
  NMT_TrackingLevel level = MemTracker::tracking_level();
  size_t            nmt_header_size = MemTracker::malloc_header_size(level);

#ifndef ASSERT
  const size_t alloc_size = size + nmt_header_size;
#else
  const size_t alloc_size = GuardedMemory::get_total_size(size + nmt_header_size);
  if (size + nmt_header_size > alloc_size) { // Check for rollover.
    return NULL;
  }
#endif

  NOT_PRODUCT(if (MallocVerifyInterval > 0) check_heap());

  u_char* ptr;
  if (MallocMaxTestWords > 0) {
    ptr = testMalloc(alloc_size);
  } else {
    ptr = (u_char*)::malloc(alloc_size);
  }

#ifdef ASSERT
  if (ptr == NULL) {
    return NULL;
  }
  // Wrap memory with guard
  GuardedMemory guarded(ptr, size + nmt_header_size);
  ptr = guarded.get_user_ptr();
#endif
  if ((intptr_t)ptr == (intptr_t)MallocCatchPtr) {
    tty->print_cr("os::malloc caught, " SIZE_FORMAT " bytes --> " PTR_FORMAT, size, ptr);
    breakpoint();
  }
  debug_only(if (paranoid) verify_memory(ptr));
  if (PrintMalloc && tty != NULL) {
    tty->print_cr("os::malloc " SIZE_FORMAT " bytes --> " PTR_FORMAT, size, ptr);
  }

  // we do not track guard memory
  return MemTracker::record_malloc((address)ptr, size, memflags, stack, level);
}
```
