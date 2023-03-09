As of this writing, there is no publicly available port of OPTEE-OS to the Raspberry Pi 4. The RPi3, on the other hand, is officially supported. So I want to try to adapt the RPi3 port to the RPi4. However, I know neither the Raspberry Pi nor the OPTEE-OS. So the motto is: Learning by doing.

Where does the platform configuration need to be adjusted? So far I've only found two values: [CONSOLE_UART_BASE](https://github.com/peter-nebe/optee_os/blob/40f3400e4ff38ad61ff7018efdcf8f9372459761/core/arch/arm/plat-rpi4/platform_config.h#L44) and [CONSOLE_UART_CLK_IN_HZ](https://github.com/peter-nebe/optee_os/blob/40f3400e4ff38ad61ff7018efdcf8f9372459761/core/arch/arm/plat-rpi4/platform_config.h#L46).

Then I did the following steps:
- Extension of the [armstub](core/arch/arm/plat-rpi4/armstub) bootloader so that it boots OPTEE-OS before the kernel
- Implementation of an [early return](https://github.com/peter-nebe/optee_os/blob/d2012188dfb5ed9558ecaf60e44db7a99433caa4/core/arch/arm/kernel/entry_a64.S#L338) in the boot sequence of OPTEE-OS

This results in the following boot flow:
```
...
MESS:00:00:04.349741:0: brfs: File read: /mfs/sd/armstub8-optee.bin
MESS:00:00:04.352896:0: Loaded 'armstub8-optee.bin' to 0x0 size 0x6bcf0
MESS:00:00:04.359249:0: brfs: File read: 441584 bytes
MESS:00:00:04.963304:0: brfs: File read: /mfs/sd/kernel8.img
MESS:00:00:04.965861:0: Loaded 'kernel8.img' to 0x80000 size 0x7d0395
MESS:00:00:06.143564:0: Kernel relocated to 0x200000
MESS:00:00:06.145416:0: Device tree loaded to 0x2eff2f00 (size 0xd062)
MESS:00:00:06.153120:0: uart: Set PL011 baud rate to 103448.300000 Hz
MESS:00:00:06.160745:0: uart: Baud rate change done...
MESS:00:00:06.162774:0:
armstub: boot optee-os
optee-os: console_init done
optee-os: return
armstub: boot kernel
[    0.000000] Booting Linux on physical CPU 0x0000000000 [0x410fd083]
[    0.000000] Linux version 5.15.61-v8+ (dom@buildbot) (aarch64-linux-gnu-gcc-8 (Ubuntu/Linaro 8.4.0-3ubuntu1) 8.4.0, GNU ld (GNU Binutils for Ubuntu) 2.34) #1579 SMP PREEMPT Fri Aug 26 11:16:44 BST 2022
[    0.000000] random: crng init done
[    0.000000] Machine model: Raspberry Pi 4 Model B Rev 1.4
...
```

The interesting thing is that the OPTEE-OS runs without the ARM Trusted Firmware (TF-A) up to this point. Only at the [end](https://github.com/peter-nebe/optee_os/blob/40f3400e4ff38ad61ff7018efdcf8f9372459761/core/arch/arm/kernel/entry_a64.S#L443) of the initialization does OPTEE-OS make a Secure Monitor Call (SMC). 

However, the people at Linaro were kind enough to point out that I also need to sort out the TF-A side of things. I've tried that. The current status of this can be seen here: https://github.com/peter-nebe/arm-trusted-firmware

After many intermediate steps, I finally got the TF-A to completely initialize the OPTEE-OS and then boot Linux, as can be seen here:
```
NOTICE:  BL31: v2.8(debug):d40fb5894-dirty
NOTICE:  BL31: Built : 12:20:21, Mar  8 2023
INFO:    Changed device tree to advertise PSCI.
INFO:    ARM GICv2 driver initialized
INFO:    BL31: Initializing runtime services
INFO:    BL31: cortex_a72: CPU workaround for 859971 was applied
WARNING: BL31: cortex_a72: CPU workaround for 1319367 was missing!
INFO:    BL31: cortex_a72: CPU workaround for cve_2017_5715 was applied
INFO:    BL31: cortex_a72: CPU workaround for cve_2018_3639 was applied
INFO:    BL31: cortex_a72: CPU workaround for cve_2022_23960 was applied
INFO:    BL31: Initializing BL32
I/TC: 
I/TC: Non-secure external DT found
I/TC: OP-TEE version: 80ed22a9-dev (gcc version 10.3.1 20210621 (GNU Toolchain for the A-profile Architecture 10.3-2021.07 (arm-10.29))) #401 Wed Mar  8 11:11:36 UTC 2023 aarch64
I/TC: WARNING: This OP-TEE configuration might be insecure!
I/TC: WARNING: Please check https://optee.readthedocs.io/en/latest/architecture/porting_guidelines.html
I/TC: Primary CPU initializing
I/TC: Primary CPU switching to normal world boot
INFO:    BL31: Preparing for EL3 exit to normal world
INFO:    Entry point address = 0x200000
INFO:    SPSR = 0x3c9
I/TC: Secondary CPU 1 initializing
I/TC: Secondary CPU 1 switching to normal world boot
I/TC: Secondary CPU 2 initializing
I/TC: Secondary CPU 2 switching to normal world boot
I/TC: Secondary CPU 3 initializing
I/TC: Secondary CPU 3 switching to normal world boot
I/TC: Reserved shared memory is enabled
I/TC: Dynamic shared memory is enabled
I/TC: Normal World virtualization support is disabled
I/TC: Asynchronous notifications are disabled
[    0.000000] Booting Linux on physical CPU 0x0000000000 [0x410fd083]
[    0.000000] Linux version 5.10.92-v8 (peter@PC1.localdomain) (aarch64-none-linux-gnu-gcc (GNU Toolchain for the A-profile Architecture 10.3-2021.07 (arm-10.29)) 10.3.1 20210621, GNU ld (GNU Toolchain for the A-profile Architecture 10.3-2021.07 (arm-10.29)) 2.36.1.20210621) #2 SMP PREEMPT Fri Feb 24 14:09:04 CET 2023
[    0.000000] random: fast init done
[    0.000000] Machine model: Raspberry Pi 4 Model B Rev 1.4
[    0.000000] efi: UEFI not found.
[    0.000000] Reserved memory: created CMA memory pool at 0x0000000027c00000, size 64 MiB
[    0.000000] OF: reserved mem: initialized node linux,cma, compatible id shared-dma-pool
[    0.000000] Zone ranges:
[    0.000000]   DMA      [mem 0x0000000000000000-0x000000003fffffff]
[    0.000000]   DMA32    [mem 0x0000000040000000-0x000000007fffffff]
[    0.000000]   Normal   empty
[    0.000000] Movable zone start for each node
[    0.000000] Early memory node ranges
[    0.000000]   node   0: [mem 0x0000000000000000-0x000000000007ffff]
[    0.000000]   node   0: [mem 0x0000000000080000-0x0000000007ffffff]
[    0.000000]   node   0: [mem 0x0000000008000000-0x00000000083fffff]
[    0.000000]   node   0: [mem 0x0000000008400000-0x00000000100fffff]
[    0.000000]   node   0: [mem 0x0000000010100000-0x0000000010ffffff]
[    0.000000]   node   0: [mem 0x0000000011000000-0x000000003b3fffff]
[    0.000000]   node   0: [mem 0x0000000040000000-0x000000007fffffff]
[    0.000000] Initmem setup node 0 [mem 0x0000000000000000-0x000000007fffffff]
[    0.000000] psci: probing for conduit method from DT.
[    0.000000] psci: PSCIv1.1 detected in firmware.
[    0.000000] psci: Using standard PSCI v0.2 function IDs
[    0.000000] psci: Trusted OS migration not required
[    0.000000] psci: SMC Calling Convention v1.2
...
[    3.974143] optee: probing for conduit method.
[    3.974206] optee: revision 3.20 (80ed22a9)
[    3.990691] optee: dynamic shared memory is enabled
[    3.991379] optee: initialized driver
...
Starting tee-supplicant: Using device /dev/teepriv0.
OK
...

Welcome to Buildroot Linux
RPI1 login: root
# 
# optee_example_hello_world
D/TA:  TA_CreateEntryPoint:39 has been called
D/TA:  TA_OpenSessionEntryPoint:68 has been called
I/TA: Hello World!
Invoking TA to increment 42
D/TA:  inc_value:105 has been called
I/TA: Got value: 42 from NW
I/TA: Increase value to: 43
TA incremented value to 43
I/TA: Goodbye!
D/TA:  TA_DestroyEntryPoint:50 has been called
# 
```

The log outputs look good. **OP-TEE runs on the Raspberry Pi 4!**

Starting with the RPi3, only the two CONSOLE_UART... values (see above) were adjusted in the platform configuration. Also in the [TF-A](https://github.com/peter-nebe/arm-trusted-firmware) only a handful of lines had to be inserted. Furthermore, of course, the Linux kernel must be built with the default OP-TEE driver (e.g. with buildroot).

This fork of OPTEE-OS also requires my fork of TF-A. They can be **built** together as follows:
```
cd <development root>/arm-trusted-firmware/plat/rpi/rpi4
./mk-rpi4
```

I already have a project that uses the RPi4 port: https://github.com/peter-nebe/optee-security-test

#### Disclaimer
The same applies to the RPi4 as to the [RPi3](https://optee.readthedocs.io/en/latest/building/devices/rpi3.html#disclaimer): This port of TF-A and OPTEE-OS is **NOT SECURE!** It is provided solely for **educational purposes** and **prototyping**.
