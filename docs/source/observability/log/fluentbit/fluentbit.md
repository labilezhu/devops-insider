# Fluent Bit 

## Fluent Bit 内部设计

```{toctree}
:maxdepth: 2
fluentbit-internal.md
```


## 使用

### Input


#### Tail

#### File Rotation

File rotation is properly handled, including logrotate's *copytruncate* mode.

Note that the `Path` patterns **cannot** match the rotated files. Otherwise, the rotated file would be read again and lead to duplicate records.

### Metrics

 - [https://docs.fluentbit.io/manual/administration/monitoring](https://docs.fluentbit.io/manual/administration/monitoring#metric-descriptions)

Dashboard: 
 - https://github.com/fluent/fluent-bit-docs/blob/8172a24d278539a1420036a9434e9f56d987a040/monitoring/dashboard.json
 - https://grafana.com/grafana/dashboards/7752-logging-dashboard/

