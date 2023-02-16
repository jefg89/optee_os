First steps:
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

Next Steps:
- Removing the early return
- Inserting additional [debug output](https://github.com/peter-nebe/optee_os/blob/b703ae578cd6cfa2d3751331f1477ab734655e90/core/arch/arm/kernel/entry_a64.S#L318)
- Changing [configuration values](https://github.com/peter-nebe/optee_os/blob/b703ae578cd6cfa2d3751331f1477ab734655e90/core/arch/arm/plat-rpi4/conf.mk#L4)
- [Commenting out](https://github.com/peter-nebe/optee_os/blob/b703ae578cd6cfa2d3751331f1477ab734655e90/core/arch/arm/kernel/entry_a64.S#L317) function calls that are stuck

This reveals some of the OPTEE-OS initialization work:
```
...
armstub: boot optee-os
D/TC:0   console_init:48 done
D/TC:0   add_phys_mem:635 ROUNDDOWN(0xFE215040, CORE_MMU_PGDIR_SIZE) type IO_NSEC 0xfe200000 size 0x00200000
D/TC:0   add_phys_mem:635 TEE_SHMEM_START type NSEC_SHM 0x08000000 size 0x00400000
D/TC:0   add_phys_mem:635 TA_RAM_START type TA_RAM 0x00701000 size 0x00800000
D/TC:0   add_phys_mem:635 VCORE_UNPG_RW_PA type TEE_RAM_RW 0x0006c000 size 0x00695000
D/TC:0   add_phys_mem:635 VCORE_UNPG_RX_PA type TEE_RAM_RX 0x00001000 size 0x0006b000
D/TC:0   add_va_space:675 type RES_VASPACE size 0x00a00000
D/TC:0   add_va_space:675 type SHM_VASPACE size 0x02000000
D/TC:0   dump_mmap_table:800 type TEE_RAM_RX   va 0x00001000..0x0006bfff pa 0x00001000..0x0006bfff size 0x0006b000 (smallpg)
D/TC:0   dump_mmap_table:800 type TEE_RAM_RW   va 0x0006c000..0x00700fff pa 0x0006c000..0x00700fff size 0x00695000 (smallpg)
D/TC:0   dump_mmap_table:800 type TA_RAM       va 0x00701000..0x00f00fff pa 0x00701000..0x00f00fff size 0x00800000 (smallpg)
D/TC:0   dump_mmap_table:800 type RES_VASPACE  va 0x01000000..0x019fffff pa 0x00000000..0x009fffff size 0x00a00000 (pgdir)
D/TC:0   dump_mmap_table:800 type SHM_VASPACE  va 0x01a00000..0x039fffff pa 0x00000000..0x01ffffff size 0x02000000 (pgdir)
D/TC:0   dump_mmap_table:800 type NSEC_SHM     va 0x03a00000..0x03dfffff pa 0x08000000..0x083fffff size 0x00400000 (pgdir)
D/TC:0   dump_mmap_table:800 type IO_NSEC      va 0x03e00000..0x03ffffff pa 0xfe200000..0xfe3fffff size 0x00200000 (pgdir)
D/TC:0   core_mmu_xlat_table_alloc:526 xlat tables used 1 / 5
D/TC:0   core_mmu_xlat_table_alloc:526 xlat tables used 2 / 5
D/TC:0   core_mmu_xlat_table_alloc:526 xlat tables used 3 / 5
D/TC:0   core_mmu_xlat_table_alloc:526 xlat tables used 4 / 5
F/TC:0   checkpoint:54 .
F/TC:0   checkpoint1:58 checkpoint 1
F/TC:0 0 init_primary:1356 .
F/TC:0 0 init_runtime:613 .
F/TC:0 0 gen_malloc_add_pool:876 .
F/TC:0 0 gen_malloc_add_pool:878 .
F/TC:0 0 gen_malloc_add_pool:881 .
F/TC:0 0 gen_malloc_add_pool:883 .
F/TC:0 0 init_runtime:615 .
I/TC: 
F/TC:0 0 init_primary:1358 .
F/TC:0 0 init_primary:1371 .
F/TC:0 0 init_primary:1374 .
F/TC:0 0 thread_init_per_cpu:766 .
F/TC:0 0 thread_init_per_cpu:768 .
F/TC:0 0 init_primary:1376 .
F/TC:0 0 init_primary:1378 .
F/TC:0 0 checkpoint2:62 checkpoint 2
I/TC: OP-TEE version: 7dbee63a-dev (gcc version 10.3.1 20210621 (GNU Toolchain for the A-profile Architecture 10.3-2021.07 (arm-10.29))) #86 Thu Feb 16 07:29:34 UTC 2023 aarch64
I/TC: WARNING: This OP-TEE configuration might be insecure!
I/TC: WARNING: Please check https://optee.readthedocs.io/en/latest/architecture/porting_guidelines.html
I/TC: Primary CPU initializing
D/TC:0 0 call_preinitcalls:21 level 2 mobj_mapped_shm_init()
D/TC:0 0 mobj_mapped_shm_init:463 Shared memory address range: 1a00000, 3a00000
D/TC:0 0 call_initcalls:40 level 1 register_time_source()
D/TC:0 0 call_initcalls:40 level 1 teecore_init_pub_ram()
D/TC:0 0 check_pa_matches_va:2156 va 0x3a00000 maps 0x8000000, expect 0x3a00000
E/TC:0 0 Panic at core/mm/core_mmu.c:2158 <check_pa_matches_va>
E/TC:0 0 TEE load address @ 0x1000
E/TC:0 0 Call stack:
E/TC:0 0  0x00008790
E/TC:0 0  0x00013218
E/TC:0 0  0x0001da58
E/TC:0 0  0x0001dbcc
E/TC:0 0  0x0001df98
E/TC:0 0  0x00014f90
E/TC:0 0  0x000084d8
```
The flow ends with a failure of the [address mapping check](https://github.com/peter-nebe/optee_os/blob/b703ae578cd6cfa2d3751331f1477ab734655e90/core/mm/core_mmu.c#L2155). That is also clear because the MMU is not enabled.

It's certainly not the smartest thing to do, but I'm not familiar with OPTEE and it's a learning project. If anyone knows the solution to the problems, please let me know.
