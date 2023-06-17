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

> https://www.technikaffe.de/anleitung-32-c_states_p_states_s_states__energieverwaltung_erklaert

### S-States

The **sleep states** are controlled via the energy management of the operating system. After an arbitrary time, the system (if supported) then shuts down to the respective S-state.



| **State** | **description**            | **power consumption** |
| --------- | -------------------------- | --------------------- |
| S0        | Working / on               | 100%                  |
| S1        | Sleep / CPU stopped        |                       |
| S2        | Deeper Sleep / CPU off     |                       |
| S3        | Standby (Save to RAM)      | ~ 1 Watt              |
| S4        | Hibernation (Save to disk) | ~ 0,1 Watt            |
| S5        | System power off           | ~ 0,1 Watt            |

 
