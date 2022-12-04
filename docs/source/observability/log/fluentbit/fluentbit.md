# Fluentbit



## Input



### Tail

#### File Rotation

File rotation is properly handled, including logrotate's *copytruncate* mode.

Note that the `Path` patterns **cannot** match the rotated files. Otherwise, the rotated file would be read again and lead to duplicate records.
