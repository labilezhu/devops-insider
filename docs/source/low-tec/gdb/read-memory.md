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











