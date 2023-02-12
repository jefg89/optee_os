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

After removing the early return and inserting some debug checkpoints:
```
...
armstub: boot optee-os
optee-os: console_init done
optee-os: checkpoint 1
init_primary 1
init_runtime 1
gen_malloc_add_pool 1
```
After that, no further output appears and it hangs.
