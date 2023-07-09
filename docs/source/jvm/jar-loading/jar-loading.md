

### mmap and jar

* [jar unzip not use mmap MAP_SHARED flag](https://bugs.openjdk.java.net/browse/JDK-7085890)


mmap is no longer used in jdk9 (we now have a pure java implementation). for previous releases there is a workaround to turn off the mmap via `-Dsun.zip.disableMemoryMapping=true`

* [Disable the mmap usage in ZipFile implementation by default](https://bugs.openjdk.java.net/browse/JDK-7142247)

Given the benefit of using mmap in ZipFile code is getting less-significant 

(The original ZipFile implementation mmaps in the whole zip/jar file content when the zip/jar file gets opened, in which **it helps the performance at some degree when there are multiple jvms are running on the same machine.** But the recent ZipFile implementation has changed from this approach by **only mapping in the central directory table** these days, for various reasons, the performance saving of using mmap is less significant, if any)

* [To bring j.u.z.ZipFile's native implementation to Java to remove the expensive jni cost and mmap crash risk](https://bugs.openjdk.java.net/browse/JDK-8142508)

The current j.u.z.ZipFile has following major issues:

(1) Its ZIP file format support code is in native C code (shared with the VM via ZipFile.c -> zip_util.c). Any entry lookup, creation requires multiple round-trip of expensive jni invocations.

(2) The current native C implementation code uses mmap to map in the central directory table appears to be a **big risk of vm crash when the underlying jar file gets overwritten with new contents** while it is still being used by other ZipFile.

(3) The use of "filename + lastModified()" cache (at native) appears to be broken if the timestamp is in low resolution,and/or the file is being overwritten.

The clean solution here is to bring the ZIP format support code from native to Java to remove the jni invocation cost and the mmap risk. Also to use the fileKey and lastModified from java.nio.file.attribute.BasicFileAttributes to have better cache matching key.

