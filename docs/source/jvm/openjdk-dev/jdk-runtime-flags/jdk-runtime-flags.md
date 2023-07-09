


hotspot/src/share/vm/runtime/globals.hpp
```cpp

struct Flag {
  enum Flags {
    // value origin
    DEFAULT          = 0,
    COMMAND_LINE     = 1,
    ENVIRON_VAR      = 2,
    CONFIG_FILE      = 3,
    MANAGEMENT       = 4,
    ERGONOMIC        = 5,
    ATTACH_ON_DEMAND = 6,
    INTERNAL         = 7,

    LAST_VALUE_ORIGIN = INTERNAL,
    VALUE_ORIGIN_BITS = 4,
    VALUE_ORIGIN_MASK = right_n_bits(VALUE_ORIGIN_BITS),

    // flag kind
    KIND_PRODUCT            = 1 << 4,
    KIND_MANAGEABLE         = 1 << 5,
    KIND_DIAGNOSTIC         = 1 << 6,
    KIND_EXPERIMENTAL       = 1 << 7,
    KIND_NOT_PRODUCT        = 1 << 8,
    KIND_DEVELOP            = 1 << 9,
    KIND_PLATFORM_DEPENDENT = 1 << 10,
    KIND_READ_WRITE         = 1 << 11,
    KIND_C1                 = 1 << 12,
    KIND_C2                 = 1 << 13,
    KIND_ARCH               = 1 << 14,
    KIND_SHARK              = 1 << 15,
    KIND_LP64_PRODUCT       = 1 << 16,
    KIND_COMMERCIAL         = 1 << 17,

    KIND_MASK = ~VALUE_ORIGIN_MASK
  };

  const char* _type;
  const char* _name;
  void* _addr;
  NOT_PRODUCT(const char* _doc;)
  Flags _flags;

  // points to all Flags static array
  static Flag* flags;

  // number of flags
  static size_t numFlags;

  static Flag* find_flag(const char* name, size_t length, bool allow_locked = false, bool return_flag = false);
  static Flag* fuzzy_match(const char* name, size_t length, bool allow_locked = false);

  void check_writable();

  bool is_bool() const;
  bool get_bool() const;
  void set_bool(bool value);

  bool is_intx() const;
  intx get_intx() const;
  void set_intx(intx value);

  bool is_uintx() const;
  uintx get_uintx() const;
  void set_uintx(uintx value);

  bool is_uint64_t() const;
  uint64_t get_uint64_t() const;
  void set_uint64_t(uint64_t value);

  bool is_double() const;
  double get_double() const;
  void set_double(double value);

  bool is_ccstr() const;
  bool ccstr_accumulates() const;
  ccstr get_ccstr() const;
  void set_ccstr(ccstr value);

  Flags get_origin();
  void set_origin(Flags origin);

  bool is_default();
  bool is_ergonomic();
  bool is_command_line();

  bool is_product() const;
  bool is_manageable() const;
  bool is_diagnostic() const;
  bool is_experimental() const;
  bool is_notproduct() const;
  bool is_develop() const;
  bool is_read_write() const;
  bool is_commercial() const;

  bool is_constant_in_binary() const;

  bool is_unlocker() const;
  bool is_unlocked() const;
  bool is_writeable() const;
  bool is_external() const;

  bool is_unlocker_ext() const;
  bool is_unlocked_ext() const;
  bool is_writeable_ext() const;
  bool is_external_ext() const;

  void unlock_diagnostic();

  void get_locked_message(char*, int) const;
  void get_locked_message_ext(char*, int) const;

  void print_on(outputStream* st, bool withComments = false );
  void print_kind(outputStream* st);
  void print_as_flag(outputStream* st);
};

#define RUNTIME_FLAGS(develop, develop_pd, product, product_pd, diagnostic, experimental, notproduct, manageable, product_rw, lp64_product) \
                                                                            \
  lp64_product(bool, UseCompressedOops, false,                              \
          "Use 32-bit object references in 64-bit VM. "                     \
          "lp64_product means flag is always constant in 32 bit VM")        \

...
  /* gc */                                                                  \
                                                                            \
  product(bool, UseSerialGC, false,                                         \
          "Use the Serial garbage collector")                               \
                                                                            \
  product(bool, UseG1GC, false,                                             \
          "Use the Garbage-First garbage collector")                        \
...
#define DECLARE_PRODUCT_FLAG(type, name, value, doc)      extern "C" type name;
...
RUNTIME_FLAGS(DECLARE_DEVELOPER_FLAG, DECLARE_PD_DEVELOPER_FLAG, DECLARE_PRODUCT_FLAG, DECLARE_PD_PRODUCT_FLAG, DECLARE_DIAGNOSTIC_FLAG, DECLARE_EXPERIMENTAL_FLAG, DECLARE_NOTPRODUCT_FLAG, DECLARE_MANAGEABLE_FLAG, DECLARE_PRODUCT_RW_FLAG, DECLARE_LP64_PRODUCT_FLAG)

```



hotspot/src/share/vm/runtime/globals.cpp
```cpp
#define RUNTIME_PRODUCT_FLAG_STRUCT(     type, name, value, doc) { #type, XSTR(name), &name,      NOT_PRODUCT_ARG(doc) Flag::Flags(Flag::DEFAULT | Flag::KIND_PRODUCT) },
...


static Flag flagTable[] = {
 RUNTIME_FLAGS(RUNTIME_DEVELOP_FLAG_STRUCT, RUNTIME_PD_DEVELOP_FLAG_STRUCT, RUNTIME_PRODUCT_FLAG_STRUCT, RUNTIME_PD_PRODUCT_FLAG_STRUCT, RUNTIME_DIAGNOSTIC_FLAG_STRUCT, RUNTIME_EXPERIMENTAL_FLAG_STRUCT, RUNTIME_NOTPRODUCT_FLAG_STRUCT, RUNTIME_MANAGEABLE_FLAG_STRUCT, RUNTIME_PRODUCT_RW_FLAG_STRUCT, RUNTIME_LP64_PRODUCT_FLAG_STRUCT)
 RUNTIME_OS_FLAGS(RUNTIME_DEVELOP_FLAG_STRUCT, RUNTIME_PD_DEVELOP_FLAG_STRUCT, RUNTIME_PRODUCT_FLAG_STRUCT, RUNTIME_PD_PRODUCT_FLAG_STRUCT, RUNTIME_DIAGNOSTIC_FLAG_STRUCT, RUNTIME_NOTPRODUCT_FLAG_STRUCT)
#if INCLUDE_ALL_GCS
 G1_FLAGS(RUNTIME_DEVELOP_FLAG_STRUCT, RUNTIME_PD_DEVELOP_FLAG_STRUCT, RUNTIME_PRODUCT_FLAG_STRUCT, RUNTIME_PD_PRODUCT_FLAG_STRUCT, RUNTIME_DIAGNOSTIC_FLAG_STRUCT, RUNTIME_EXPERIMENTAL_FLAG_STRUCT, RUNTIME_NOTPRODUCT_FLAG_STRUCT, RUNTIME_MANAGEABLE_FLAG_STRUCT, RUNTIME_PRODUCT_RW_FLAG_STRUCT)
#endif // INCLUDE_ALL_GCS
#ifdef COMPILER1
 C1_FLAGS(C1_DEVELOP_FLAG_STRUCT, C1_PD_DEVELOP_FLAG_STRUCT, C1_PRODUCT_FLAG_STRUCT, C1_PD_PRODUCT_FLAG_STRUCT, C1_DIAGNOSTIC_FLAG_STRUCT, C1_NOTPRODUCT_FLAG_STRUCT)
#endif
#ifdef COMPILER2
 C2_FLAGS(C2_DEVELOP_FLAG_STRUCT, C2_PD_DEVELOP_FLAG_STRUCT, C2_PRODUCT_FLAG_STRUCT, C2_PD_PRODUCT_FLAG_STRUCT, C2_DIAGNOSTIC_FLAG_STRUCT, C2_EXPERIMENTAL_FLAG_STRUCT, C2_NOTPRODUCT_FLAG_STRUCT)
#endif
};

Flag* Flag::flags = flagTable;
size_t Flag::numFlags = (sizeof(flagTable) / sizeof(Flag));

```


