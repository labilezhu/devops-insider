

> https://medium.com/@laura.derohan/compiling-c-files-with-gcc-step-by-step-8e78318052


![](./compiling-c-files.assets/gcc.png)


# The steps

## 1. The preprocessor

- it gets rid of all the _comments_ in the source file(s)
- it includes the code of the _header file(s)_, which is a file with extension .h which contains C function declarations and macro definitions
- it replaces all of the _macros_ (fragments of code which have been given a name) by their values

```bash
gcc -E ./small.c
```


## 2. The compiler

```bash
gcc -S ./small.c
```

## 3. The assembler

```bash
gcc -c ./small.c
```

## 4. The linker

```bash
gcc ./small.c -o ./small
```
