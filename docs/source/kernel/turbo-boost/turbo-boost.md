---
date: 2022-06-29T23:12:15+08:00
draft: true
weight: 1
enableToc: true
enableBlogBreadcrumb: false
tocLevels: ["h2", "h3", "h4"]
description: 
tags:
- performance
- hardware
- c-state
---

> https://vstinner.github.io/intel-cpus.html

## Turbo Boost

In 2005, Intel introduced [SpeedStep](https://en.wikipedia.org/wiki/SpeedStep), a serie of dynamic frequency scaling technologies to reduce the power consumption of laptop CPUs. Turbo Boost is an enhancement of these technologies, now also used on desktop and server CPUs.

Turbo Boost allows to run one or many CPU cores to higher P-states than usual. The maximum P-state is constrained by the following factors:

- The number of active cores (in C0 or C1 state)
- The estimated current consumption of the processor (Imax)
- The estimated power consumption (TDP - Thermal Design Power) of processor
- The temperature of the processor

Example on my laptop:

```
selma$ cat /proc/cpuinfo
model name : Intel(R) Core(TM) i7-3520M CPU @ 2.90GHz
...

selma$ sudo cpupower frequency-info
analyzing CPU 0:
  driver: intel_pstate
  ...
  boost state support:
    Supported: yes
    Active: yes
    3400 MHz max turbo 4 active cores
    3400 MHz max turbo 3 active cores
    3400 MHz max turbo 2 active cores
    3600 MHz max turbo 1 active cores
```

The CPU base frequency is 2.9 GHz. If more than one physical cores is "active" (busy), their frequency can be increased up to 3.4 GHz. If only 1 physical core is active, its frequency can be increased up to 3.6 GHz.

In this example, Turbo Boost is supported and active.

See also the [Linux cpu-freq documentation on CPU boost](https://www.kernel.org/doc/Documentation/cpu-freq/boost.txt).

### Turbo Boost MSR

The bit 38 of the [Model-specific register (MSR)](https://en.wikipedia.org/wiki/Model-specific_register) `0x1a0` can be used to check if the Turbo Boost is enabled:

```
selma$ sudo rdmsr -f 38:38 0x1a0
0
```

`0` means that Turbo Boost is enabled, whereas `1` means disabled (no turbo). (The `-f 38:38` option asks to only display the bit 38.)

If the command doesn't work, you may have to load the `msr` kernel module:

```
sudo modprobe msr
```

Note: I'm not sure that all Intel CPU uses the same MSR.

### intel_state/no_turbo

Turbo Boost can also be disabled at runtime in the `intel_pstate` driver.

Check if Turbo Boost is enabled:

```
selma$ cat /sys/devices/system/cpu/intel_pstate/no_turbo
0
```

where `0` means that Turbo Boost is enabled. Disable Turbo Boost:

```
selma$ echo 1|sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo
```

### CPU flag "ida"

It looks like the Turbo Boost status (supported or not) can also be read by the CPUID(6): "Thermal/Power Management". It gives access to the flag [Intel Dynamic Acceleration (IDA)](https://en.wikipedia.org/wiki/Intel_Dynamic_Acceleration).

The `ida` flag can also be seen in CPU flags of `/proc/cpuinfo`.

## Read the CPU frequency

General information using `cpupower frequency-info`:

```
selma$ cpupower -c 0 frequency-info
analyzing CPU 0:
  driver: intel_pstate
  ...
  hardware limits: 1.20 GHz - 3.60 GHz
  ...
```

The frequency of CPUs is between 1.2 GHz and 3.6 GHz (the base frequency is 2.9 GHz on this CPU).

### Get the frequency of CPUs: turbostat

It looks like the most reliable way to get a relialistic estimation of the CPUs frequency is to use the tool `turbostat`:

```
selma$ sudo turbostat
     CPU Avg_MHz   Busy% Bzy_MHz TSC_MHz
       -     224    7.80    2878    2893
       0     448   15.59    2878    2893
       1       0    0.01    2762    2893
     CPU Avg_MHz   Busy% Bzy_MHz TSC_MHz
       -     139    5.65    2469    2893
       0     278   11.29    2469    2893
       1       0    0.01    2686    2893
    ...
```

- `Avg_MHz`: average frequency, based on APERF
- `Busy%`: CPU usage in percent
- `Bzy_MHz`: busy frequency, based on MPERF
- `TSC_MHz`: fixed frequency, TSC stands for [Time Stamp Counter](https://en.wikipedia.org/wiki/Time_Stamp_Counter)

APERF (average) and MPERF (maximum) are MSR registers that can provide feedback on current CPU frequency.

### Other tools to get the CPU frequency

It looks like the following tools are less reliable to estimate the CPU frequency.

cpuinfo:

```
selma$ grep MHz /proc/cpuinfo
cpu MHz : 1372.289
cpu MHz : 3401.042
```

In April 2016, Len Brown proposed a patch modifying cpuinfo to use APERF and MPERF MSR to estimate the CPU frequency: [x86: Calculate MHz using APERF/MPERF for cpuinfo and scaling_cur_freq](https://lkml.org/lkml/2016/4/1/7).

The `tsc` clock source logs the CPU frequency in kernel logs:

```
selma$ dmesg|grep 'MHz processor'
[    0.000000] tsc: Detected 2893.331 MHz processor
```

cpupower frequency-info:

```
selma$ for core in $(seq 0 1); do sudo cpupower -c $core frequency-info|grep 'current CPU'; done
  current CPU frequency: 3.48 GHz (asserted by call to hardware)
  current CPU frequency: 3.40 GHz (asserted by call to hardware)
```

cpupower monitor:

```
selma$ sudo cpupower monitor -m 'Mperf'
    |Mperf
CPU | C0   | Cx   | Freq
   0|  4.77| 95.23|  1924
   1|  0.01| 99.99|  1751
```
