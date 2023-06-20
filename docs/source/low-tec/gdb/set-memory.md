# set memory

> https://stackoverflow.com/questions/26064612/in-gdb-can-you-set-memory-as-if-it-were-a-char-array



6


There's alternative of writing char array in one command, without standard functions like strcpy().

```c
set *(char [CHAR_ARRAY_SIZE] *) <WRITE_ADDRESS> = "YOUR_CHAR_ARRAY"
```

where `CHAR_ARRAY_SIZE` is the size of `YOUR_CHAR_ARRAY`, plus extra NULL byte (null-terminated string).

e.g.

```c
set *(char [15] *) 0x20018000 = "Write a string"
```



> https://stackoverflow.com/questions/19503057/in-gdb-how-can-i-write-a-string-to-memory



```c
set {char [4]} 0x08040000 = "Ace"
```





> https://stackoverflow.com/questions/3305164/how-to-modify-memory-contents-using-gdb



Writing memory:

```c
(gdb) set *0x20001234 = 0xABABABAB
```

```
(gdb) set {int}0x08040000 = 42
(gdb) set {int}0x08040000 = 0xffffffff
```