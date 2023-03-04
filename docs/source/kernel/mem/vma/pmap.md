##### 进程的内存

一般意义的进程是指可执行文件运行实例。进程的内存结构可能大致划分为：

![image-20220124225349623](pmap.assets/image-20220124225349623.png)

<p align = "center">Process virtual address
space.<br /> From [Computer Systems - A Programmer’s Perspective]</p>


其中的 `Memory-mapped region for shared libraries` 是二进制计算机指令部分，可先简单认为是直接 copy 或映射自可执行文件的 `.text section（区域）` (虽然这不完全准确)。


> https://www.labcorner.de/cheat-sheet-understanding-the-pmap1-output/


![](pmap.assets/read-pmap.png)


> https://techtalk.intersec.com/2013/07/memory-part-2-understanding-process-memory/

![](pmap.assets/vma-cata.png)

