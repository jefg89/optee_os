At the moment, as I write this document, there is no publicly available port of OPTEE-OS to the Raspberry Pi 4. The RPi3, on the other hand, is officially supported. So I want to try to adapt the RPi3 port to the RPi4. However, I know neither the Raspberry Pi nor the OPTEE-OS. So the motto is: Learning by doing.

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

The interesting thing is that the OPTEE-OS runs without the ARM Trusted Firmware (TF-A) up to this point. Only at the [end](https://github.com/peter-nebe/optee_os/blob/40f3400e4ff38ad61ff7018efdcf8f9372459761/core/arch/arm/kernel/entry_a64.S#L443) of the initialization does OPTEE-OS make a Secure Monitor Call (SMC). The SMC is made in place of the usual return. So I did the following steps:
- Moving the early return until just [before](https://github.com/peter-nebe/optee_os/blob/40f3400e4ff38ad61ff7018efdcf8f9372459761/core/arch/arm/kernel/entry_a64.S#L426) the SMC
- Inserting additional [debug output](https://github.com/peter-nebe/optee_os/blob/40f3400e4ff38ad61ff7018efdcf8f9372459761/core/arch/arm/kernel/entry_a64.S#L341)
- Changing [configuration values](https://github.com/peter-nebe/optee_os/blob/40f3400e4ff38ad61ff7018efdcf8f9372459761/core/arch/arm/plat-rpi4/conf.mk#L3)
- [Commenting out](https://github.com/peter-nebe/optee_os/blob/40f3400e4ff38ad61ff7018efdcf8f9372459761/core/arch/arm/kernel/entry_a64.S#L317) function calls that are stuck

This reveals the initialization of the OPTEE-OS:
```
...
armstub: boot optee-os
D/TC:0   console_init:48 done
D/TC:0   get_aslr_seed:1566 Cannot find /secure-chosen
D/TC:0   plat_get_aslr_seed:114 Warning: no ASLR seed
D/TC:0   add_phys_mem:635 ROUNDDOWN(0xfe215040, CORE_MMU_PGDIR_SIZE) type IO_NSEC 0xfe200000 size 0x00200000
D/TC:0   add_phys_mem:635 TEE_SHMEM_START type NSEC_SHM 0x08000000 size 0x00400000
D/TC:0   add_phys_mem:635 TA_RAM_START type TA_RAM 0x00701000 size 0x00800000
D/TC:0   add_phys_mem:635 VCORE_UNPG_RW_PA type TEE_RAM_RW 0x00067000 size 0x0069a000
D/TC:0   add_phys_mem:635 VCORE_UNPG_RX_PA type TEE_RAM_RX 0x00001000 size 0x00066000
D/TC:0   add_va_space:675 type RES_VASPACE size 0x00a00000
D/TC:0   add_va_space:675 type SHM_VASPACE size 0x02000000
D/TC:0   dump_mmap_table:800 type RES_VASPACE  va 0x00000000..0x009fffff pa 0x00000000..0x009fffff size 0x00a00000 (pgdir)
D/TC:0   dump_mmap_table:800 type SHM_VASPACE  va 0x00000000..0x01ffffff pa 0x00000000..0x01ffffff size 0x02000000 (pgdir)
D/TC:0   dump_mmap_table:800 type TEE_RAM_RX   va 0x00001000..0x00066fff pa 0x00001000..0x00066fff size 0x00066000 (smallpg)
D/TC:0   dump_mmap_table:800 type TEE_RAM_RW   va 0x00067000..0x00700fff pa 0x00067000..0x00700fff size 0x0069a000 (smallpg)
D/TC:0   dump_mmap_table:800 type TA_RAM       va 0x00701000..0x00f00fff pa 0x00701000..0x00f00fff size 0x00800000 (smallpg)
D/TC:0   dump_mmap_table:800 type NSEC_SHM     va 0x08000000..0x083fffff pa 0x08000000..0x083fffff size 0x00400000 (pgdir)
D/TC:0   dump_mmap_table:800 type IO_NSEC      va 0xfe200000..0xfe3fffff pa 0xfe200000..0xfe3fffff size 0x00200000 (pgdir)
D/TC:0   core_mmu_xlat_table_alloc:526 xlat tables used 1 / 8
D/TC:0   core_mmu_xlat_table_alloc:526 xlat tables used 2 / 8
D/TC:0   core_mmu_xlat_table_alloc:526 xlat tables used 3 / 8
D/TC:0   core_mmu_xlat_table_alloc:526 xlat tables used 4 / 8
D/TC:0   core_mmu_xlat_table_alloc:526 xlat tables used 5 / 8
F/TC:0   checkpoint:54 .
D/TC:0   console_init:48 done
F/TC:0   checkpoint1:58 checkpoint 1
F/TC:0 0 init_primary:1369 .
F/TC:0 0 init_runtime:613 .
F/TC:0 0 gen_malloc_add_pool:876 .
F/TC:0 0 gen_malloc_add_pool:878 .
F/TC:0 0 gen_malloc_add_pool:881 .
F/TC:0 0 gen_malloc_add_pool:883 .
F/TC:0 0 init_runtime:615 .
I/TC: 
F/TC:0 0 init_primary:1371 .
D/TC:0 0 select_vector_wa_spectre_v2:630 SMCCC_ARCH_WORKAROUND_1 (0x80008000) unavailable
D/TC:0 0 select_vector_wa_spectre_v2:632 SMC Workaround for CVE-2017-5715 not needed (if ARM-TF is up to date)
F/TC:0 0 checkpoint2:62 checkpoint 2
I/TC: Non-secure external DT found
I/TC: OP-TEE version: 3044b1ab-dev (gcc version 10.3.1 20210621 (GNU Toolchain for the A-profile Architecture 10.3-2021.07 (arm-10.29))) #172 Mon Feb 27 18:05:22 UTC 2023 aarch64
I/TC: WARNING: This OP-TEE configuration might be insecure!
I/TC: WARNING: Please check https://optee.readthedocs.io/en/latest/architecture/porting_guidelines.html
I/TC: Primary CPU initializing
D/TC:0 0 boot_init_primary_late:1422 Executing at offset 0 with virtual load address 0x1000
D/TC:0 0 call_initcalls:40 level 1 register_time_source()
D/TC:0 0 call_initcalls:40 level 1 teecore_init_pub_ram()
D/TC:0 0 call_initcalls:40 level 2 probe_dt_drivers_early()
D/TC:0 0 call_initcalls:40 level 3 check_ta_store()
D/TC:0 0 check_ta_store:417 TA store: "Secure Storage TA"
D/TC:0 0 check_ta_store:417 TA store: "REE"
D/TC:0 0 call_initcalls:40 level 3 verify_pseudo_tas_conformance()
D/TC:0 0 call_initcalls:40 level 3 tee_cryp_init()
D/TC:0 0 call_initcalls:40 level 4 tee_fs_init_key_manager()
D/TC:0 0 call_initcalls:40 level 5 probe_dt_drivers()
D/TC:0 0 call_initcalls:40 level 6 mobj_init()
D/TC:0 0 call_initcalls:40 level 6 default_mobj_init()
D/TC:0 0 call_initcalls:40 level 6 ftmn_boot_tests()
D/TC:0 0 ftmn_boot_tests:198 Calling simple_call()
D/TC:0 0 ftmn_boot_tests:198 Return from simple_call()
D/TC:0 0 ftmn_boot_tests:199 Calling two_level_call()
D/TC:0 0 ftmn_boot_tests:199 Return from two_level_call()
D/TC:0 0 ftmn_boot_tests:200 Calling chained_calls()
D/TC:0 0 ftmn_boot_tests:200 Return from chained_calls()
D/TC:0 0 ftmn_boot_tests:202 *************************************************
D/TC:0 0 ftmn_boot_tests:203 **************  Tests complete  *****************
D/TC:0 0 ftmn_boot_tests:204 *************************************************
D/TC:0 0 call_initcalls:40 level 7 release_probe_lists()
D/TC:0 0 call_finalcalls:59 level 1 release_external_dt()
I/TC: Primary CPU switching to normal world boot
F/TC:0 0 checkpoint:54 .
F/TC:0   checkpoint3:66 checkpoint 3
armstub: boot kernel
[    0.000000] Booting Linux on physical CPU 0x0000000000 [0x410fd083]
[    0.000000] Linux version 5.10.92-v8 (peter@PC1.localdomain) (aarch64-none-linux-gnu-gcc (GNU Toolchain for the A-profile Architecture 10.3-2021.07 (arm-10.29)) 10.3.1 20210621, GNU ld (GNU Toolchain for the A-profile Architecture 10.3-2021.07 (arm-10.29)) 2.36.1.20210621) #1 SMP PREEMPT Fri Jan 27 16:40:27 CET 2023
[    0.000000] random: fast init done
[    0.000000] Machine model: Raspberry Pi 4 Model B Rev 1.4
...
```

The next steps will be:
- Enabling the MMU
- Enabling the synchronization primitive ([cpu_spin_lock_xsave](https://github.com/peter-nebe/optee_os/blob/40f3400e4ff38ad61ff7018efdcf8f9372459761/lib/libutils/isoc/bget_malloc.c#L149))
- Start the TF-A with Secure Monitor
- Initialization of the OPTEE kernel driver

Unfortunately, I have no idea why the MMU and the synchronization primitives aren't working. If anyone knows the solution to the problems, please let me know.
