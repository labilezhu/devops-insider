# Read memory



Reading memory:

```c
(gdb) x 0x20001234
0x20001234:     0xabababab
```



> https://stackoverflow.com/questions/29528732/gdb-print-all-values-in-char-array



```c
(gdb) list
1   int main(void) {
2       char * p = "hello\0world\0hahaha";
3       return 0;
4   }
5   
(gdb) b 3
Breakpoint 1 at 0x4004b8: file str.c, line 3.
(gdb) run
Starting program: /home/paul/src/sandbox/str 

Breakpoint 1, main () at str.c:3
3       return 0;
(gdb) print p
$1 = 0x40056c "hello"
(gdb) x/19bc p
0x40056c:   104 'h' 101 'e' 108 'l' 108 'l' 111 'o' 0 '\000'    119 'w' 111 'o'
0x400574:   114 'r' 108 'l' 100 'd' 0 '\000'    104 'h' 97 'a'  104 'h' 97 'a'
0x40057c:   104 'h' 97 'a'  0 '\000'
```





> https://stackoverflow.com/questions/29528732/gdb-print-all-values-in-char-array



With gdb, you can achieve to print the elements of your array using the following command:

```c
(gdb) print *array@size
```



> https://jvns.ca/blog/2014/02/10/three-steps-to-learning-gdb/

From looking at that assembly above, it seems like `0x40060c` might be the address of the string we’re printing. Let’s check!

```
(gdb) x/s 0x40060c
0x40060c:        "Hi!"
```

It is! Neat! Look at that. The `/s` part of `x/s` means “show it to me like it’s a string”. I could also have said “show me 10 characters” like this:

```
(gdb) x/10c 0x40060c
0x40060c:       72 'H'  105 'i' 33 '!'  0 '\000'        1 '\001'        27 '\033'       3 '\003'        59 ';'
0x400614:       52 '4'  0 '\000'
```

You can see that the first four characters are ‘H’, ‘i’, and ‘!’, and ‘\0’ and then after that there’s more unrelated stuff.









