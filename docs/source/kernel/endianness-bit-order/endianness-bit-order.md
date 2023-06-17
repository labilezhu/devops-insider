---
title: "Endianness-字节序"
date: 2021-03-01T15:12:15+09:00
draft: true
weight: 1
enableToc: true
enableBlogBreadcrumb: false
tocLevels: ["h2", "h3", "h4"]
tags:
- kernel
- kernel-mem
- kernel-mem-address
---

## Endianness-字节序

> https://helpful.knobs-dials.com/index.php/Endianness

- Little-endian (LSB) means we start with the least significant part in the lowest address.

- Big-endian (MSB) means we start with the most significant part.

For example, 16-bit integer `0x1234` would be stored in bytes as 
- 0x12 0x34 (LSB) 
- 0x34 0x12 (MSB).

## Little-endian, Big-endian, LSB, MSB

**Little-endian**: lowest value first, or leftmost/lowest in memory, increasing numeric significance with increasing memory addresses (or, in networking, time)

- Little-endian architectures store the least significant part first (in the lowest memory location)
- They include the `x86` line of processors (x86, AMD64 a.k.a. x86-64)

- In byte architectures, little-endian is also known as **LSB**, referring to the Least Significant Byte coming first.


Examples: consider a 32-bit integer

- 12345 = hex:0x12345 would, shown in hexidecimal, be 0x39 0x30 0x00 0x00
- 287454020 would be 0x44 0x33 0x22 0x11



**Big-endian**: highest value first, decreasing significance

- Big-endian architectures store the most significant part first (in the lowest memory location).
- Includes the Motorola 68000 line of processors (e.g. pre-Intel Macintosh), PowerPC G5 a.k.a. PowerPC 970

- In byte architectures, big-endian is also known as **MSB**, referring to the Most Significant Byte coming first.


Examples: consider a 32-bit integer

- 12345 would, shown in hexidecimal, be 0x00 0x00 0x30 0x39
- 287454020 would be 0x11 0x22 0x33 0x44



**Network byte order** is a term used by various RFCs, and refers to big-endian (with a few exceptions?[[1\]](http://en.wikipedia.org/wiki/Network_byte_order#Endianness_in_networking))

