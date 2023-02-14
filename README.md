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

After removing the early return, inserting some debug checkpoints, and the latest configuration changes:
```
...
armstub: boot optee-os
D/TC:0   console_init:48 done
D/TC:0   add_phys_mem:635 ROUNDDOWN(0xFE215040, CORE_MMU_PGDIR_SIZE) type IO_NSEC 0xfe200000 size 0x00200000
D/TC:0   add_phys_mem:635 TEE_SHMEM_START type NSEC_SHM 0x08000000 size 0x00400000
D/TC:0   add_phys_mem:635 TA_RAM_START type TA_RAM 0x10800000 size 0x00800000
D/TC:0   add_phys_mem:635 TEE_RAM_START type TEE_RAM_RWX 0x10100000 size 0x00700000
D/TC:0   add_va_space:675 type RES_VASPACE size 0x00a00000
D/TC:0   add_va_space:675 type SHM_VASPACE size 0x02000000
D/TC:0   dump_mmap_table:800 type TEE_RAM_RWX  va 0x10100000..0x107fffff pa 0x10100000..0x107fffff size 0x00700000 (smallpg)
D/TC:0   dump_mmap_table:800 type RES_VASPACE  va 0x10800000..0x111fffff pa 0x00000000..0x009fffff size 0x00a00000 (pgdir)
D/TC:0   dump_mmap_table:800 type SHM_VASPACE  va 0x11200000..0x131fffff pa 0x00000000..0x01ffffff size 0x02000000 (pgdir)
D/TC:0   dump_mmap_table:800 type NSEC_SHM     va 0x13200000..0x135fffff pa 0x08000000..0x083fffff size 0x00400000 (pgdir)
D/TC:0   dump_mmap_table:800 type TA_RAM       va 0x13600000..0x13dfffff pa 0x10800000..0x10ffffff size 0x00800000 (pgdir)
D/TC:0   dump_mmap_table:800 type IO_NSEC      va 0x13e00000..0x13ffffff pa 0xfe200000..0xfe3fffff size 0x00200000 (pgdir)
D/TC:0   core_mmu_xlat_table_alloc:526 xlat tables used 1 / 5
D/TC:0   core_mmu_xlat_table_alloc:526 xlat tables used 2 / 5
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
```
After that, no further output appears and it hangs.
