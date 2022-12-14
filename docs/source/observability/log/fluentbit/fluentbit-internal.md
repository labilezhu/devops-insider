# 图解 Fluent Bit 内部设计

## 互动图片

点击 “*用 Draw.io 打开*” 后，可以进入互动图片状态。图中很多元素提供链接到相关源码或文档。可以做交叉参考，也是图可信性取证。

## Record 概念

可以简化认为，日志文件中的一行，就是一个 `Record`。内部以 json 树形式来记录一个 `Record`。

为提高内存中的 `Record` 数据密度，同时加速 json 结构树的访问。Fluent Bit 内部使用了 [`MessagePack`](https://msgpack.org/index.html) 格式在内存与硬盘中保存数据。所以，请注意不是用我们日常见的明文 json 格式。可能如果要比较精细评估 Fluent Bit 内存使用时，需要考虑这一点。



## Chunk 概念

为提高处理性能，Fluent Bit 每次以小批量的 `Record`  为单位处理数据。每个批叫 `Chunk`。他是 `Record` 的集合。

数据在由 Input Plugin 加载入内存时，就已经是以批(`Chunk`) 的形式了。加载后，经由 pipeline、最后再到 Output，均以 Chunk 为粒度去处理（这里我未完全肯定）。



下面说明一下代码与存储的结构：



:::{figure-md} 图：Chunk 定义
:class: full-width

<img src="fluentbit-chunk.drawio.svg" alt="Chunk 定义">

*图：Chunk 定义*  
:::
*[用 Draw.io 打开](https://app.diagrams.net/?ui=sketch#Uhttps%3A%2F%2Fdevops-insider.mygraphql.com%2Fzh_CN%2Flatest%2F_images%2Ffluentbit-chunk.drawio.svg)*

## Pipeline/Engine 概念

:::{figure-md} 图：Engine 概念
:class: full-width

<img src="fluentbit-pipeline.drawio.svg" alt="Engine 概念">

*图：Engine 概念*  
:::
*[用 Draw.io 打开](https://app.diagrams.net/?ui=sketch#Uhttps%3A%2F%2Fdevops-insider.mygraphql.com%2Fzh_CN%2Flatest%2F_images%2Ffluentbit-pipeline.drawio.svg)*


## Input

### Tail Input

#### Tail Input 概述

:::{figure-md} 图：Tail Input 概述
:class: full-width

<img src="fluentbit-tail-input.drawio.svg" alt="Tail Input 概述">

*图：Tail Input 概述*  
:::
*[用 Draw.io 打开](https://app.diagrams.net/?ui=sketch#Uhttps%3A%2F%2Fdevops-insider.mygraphql.com%2Fzh_CN%2Flatest%2F_images%2Ffluentbit-tail-input.drawio.svg)*

#### Tail Input 内部设计

:::{figure-md} 图：Tail Input 内部设计
:class: full-width

<img src="fluentbit-tail-internal.drawio.svg" alt="Tail Input 内部设计">

*图：Tail Input 内部设计*  
:::
*[用 Draw.io 打开](https://app.diagrams.net/?ui=sketch#Uhttps%3A%2F%2Fdevops-insider.mygraphql.com%2Fzh_CN%2Flatest%2F_images%2Ffluentbit-tail-internal.drawio.svg)*

