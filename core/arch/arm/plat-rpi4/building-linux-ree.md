## Building Linux/REE
Here is how to build the Linux for the [REE](## "Rich Execution Environment") side of an OP-TEE system for the **Raspberry Pi 4**. You can build it with Buildroot or with Yocto.

### Building with Buildroot
It is assumed that you have already built a Linux (without OP-TEE) for the Raspberry Pi 4 with Buildroot. Only the additional steps to build OP-TEE support into Linux are mentioned here.

Create a kernel configuration fragment file, *kernel-optee.cfg*, with the following content:
```
CONFIG_TEE=y
CONFIG_OPTEE=y
```
Call the ```make menuconfig``` command and make the following settings (replace */d1/buildroot/linux-rpi* with your path):
```
Kernel ---> Additional configuration fragment files ---> /d1/buildroot/linux-rpi/kernel-optee.cfg
Target packages ---> Security ---> optee-client ---> Yes
```
The Hello World sample program from the optee examples is recommended for a first function test on the target. To include the **optee-examples**, make the following settings:
```
Bootloaders ---> optee_os ---> Yes
Bootloaders ---> OP-TEE OS version ---> Custom Git repository
Bootloaders ---> URL of custom repository ---> https://github.com/OP-TEE/optee_os.git
Bootloaders ---> Custom repository version ---> 3.20.0
Bootloaders ---> OP-TEE OS needs host-python-cryptography ---> No
Bootloaders ---> Build core ---> No
Bootloaders ---> Build service TAs and libs ---> No
Bootloaders ---> Target platform (mandatory) ---> rpi3
Target packages ---> Security ---> optee-examples ---> Yes
```
Really enter *rpi3*, because that is the only Raspberry Pi version that optee_os knows.

### Building with Yocto
It is assumed that your development machine is set up to build a Linux for the Raspberry Pi 4 with Yocto. We're starting over here with what's currently the latest release (Mickledore). Customize the steps to suit your needs. Run the following commands (replace */d1/yocto* with your path):
```
cd /d1/yocto
git clone -b mickledore git://git.yoctoproject.org/poky
git clone -b mickledore git://git.openembedded.org/meta-openembedded
git clone -b master git://git.yoctoproject.org/meta-raspberrypi
git clone -b mickledore git://git.yoctoproject.org/meta-arm
cd poky
source oe-init-build-env build-rpi
bitbake-layers add-layer ../../meta-openembedded/meta-oe
bitbake-layers add-layer ../../meta-openembedded/meta-python
bitbake-layers add-layer ../../meta-openembedded/meta-networking
bitbake-layers add-layer ../../meta-raspberrypi
bitbake-layers add-layer ../../meta-arm/meta-arm-toolchain
bitbake-layers add-layer ../../meta-arm/meta-arm
bitbake-layers create-layer ../../meta-kernel
bitbake-layers add-layer ../../meta-kernel
bitbake-layers show-layers
```
The last command should produce an output similar to this:
```
layer                 path                                         priority
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
core                  /d1/yocto/poky/meta                          5
yocto                 /d1/yocto/poky/meta-poky                     5
yoctobsp              /d1/yocto/poky/meta-yocto-bsp                5
openembedded-layer    /d1/yocto/meta-openembedded/meta-oe          5
meta-python           /d1/yocto/meta-openembedded/meta-python      5
networking-layer      /d1/yocto/meta-openembedded/meta-networking  5
raspberrypi           /d1/yocto/meta-raspberrypi                   9
arm-toolchain         /d1/yocto/meta-arm/meta-arm-toolchain        5
meta-arm              /d1/yocto/meta-arm/meta-arm                  5
meta-kernel           /d1/yocto/meta-kernel                        6
```
Create a file *../../meta-kernel/recipes-kernel/linux/linux-raspberrypi_%.bbappend* with this content:
```
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI += "file://kernel-optee.cfg"
```
Create a file *../../meta-kernel/recipes-kernel/linux/files/kernel-optee.cfg* with this content:
```
CONFIG_TEE=y
CONFIG_OPTEE=y
```
Edit the *conf/local.conf* file. Comment out the current setting of MACHINE and add the following lines:
```
MACHINE ?= "raspberrypi4-64"
ENABLE_UART = "1"
IMAGE_INSTALL:append = " optee-client optee-examples"
DL_DIR ?= "${TOPDIR}/../downloads"
```
The last line sets a download folder outside of the build folder.

The default kernel version is 6.1. However, OP-TEE did not work for me with this version. So I changed the kernel version to 5.15. Do that with the following entry, for example in *conf/local.conf*:
```
PREFERRED_VERSION_linux-raspberrypi ?= "5.15.%"
```
The optee-examples require COMPATIBLE_MACHINE to be set. In *conf/local.conf* that didn't work for me. So I entered this in *../../meta-arm/meta-arm/recipes-security/optee/optee.inc*:
```
COMPATIBLE_MACHINE ?= "raspberrypi4-64"
```
Now it's time to start Bitbake. Enter the following command:
```
bitbake core-image-minimal
```
Bitbake will most likely abort with an error because optee_os does not have the *plat-raspberrypi4* platform. You should create a patch for it. Or manually in optee_os copy *plat-rpi3* to *plat-raspberrypi4* and adjust the two values: [CONSOLE_UART_BASE](https://github.com/peter-nebe/optee_os/blob/40f3400e4ff38ad61ff7018efdcf8f9372459761/core/arch/arm/plat-rpi4/platform_config.h#L44) and [CONSOLE_UART_CLK_IN_HZ](https://github.com/peter-nebe/optee_os/blob/40f3400e4ff38ad61ff7018efdcf8f9372459761/core/arch/arm/plat-rpi4/platform_config.h#L46). Then repeat ```bitbake core-image-minimal```.

When Bitbake completed successfully, write the *.wic* image to an SD card. Additionally copy *bl31-optee.bin* to the boot partition. Add these lines to *config.txt*:
```
uart_2ndstage=1
armstub=bl31-optee.bin
kernel_address=0x200000
```
Boot the target and compare the relevant boot outputs to these:
```
...
MESS:00:00:04.358758:0: brfs: File read: /mfs/sd/bl31-optee.bin
MESS:00:00:04.361568:0: Loaded 'bl31-optee.bin' to 0x0 size 0x8fdd8
MESS:00:00:04.367574:0: brfs: File read: 589272 bytes
MESS:00:00:06.095423:0: brfs: File read: /mfs/sd/kernel8.img
MESS:00:00:06.097976:0: Loaded 'kernel8.img' to 0x200000 size 0x16b3a00
MESS:00:00:06.104327:0: Device tree loaded to 0x2eff2d00 (size 0xd22f)
MESS:00:00:06.112178:0: uart: Set PL011 baud rate to 103448.300000 Hz
MESS:00:00:06.119631:0: uart: Baud rate change done...
MESS:00:00:06.121653:0:NOTICE:  BL31: v2.8(debug):2520b8784
NOTICE:  BL31: Built : 13:42:29, Mar 24 2023
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
I/TC: OP-TEE version: bc149c9e-dev (gcc version 10.3.1 20210621 (GNU Toolchain for the A-profile Architecture 10.3-2021.07 (arm-10.29))) #1 Mon Mar 27 16:15:15 UTC 2023 aarch64
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
[    0.000000] Linux version 5.15.92-v8 (oe-user@oe-host) (aarch64-poky-linux-gcc (GCC) 12.2.0, GNU ld (GNU Binutils) 2.40.20230119) #1 SMP PREEMPT Wed Feb 8 16:47:50 UTC 2023
[    0.000000] random: crng init done
[    0.000000] Machine model: Raspberry Pi 4 Model B Rev 1.4
[    0.000000] efi: UEFI not found.
[    0.000000] Reserved memory: created CMA memory pool at 0x000000001ac00000, size 320 MiB
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
[    0.000000] On node 0, zone DMA32: 19456 pages in unavailable ranges
[    0.000000] psci: probing for conduit method from DT.
[    0.000000] psci: PSCIv1.1 detected in firmware.
[    0.000000] psci: Using standard PSCI v0.2 function IDs
[    0.000000] psci: Trusted OS migration not required
[    0.000000] psci: SMC Calling Convention v1.2
...
[    1.390314] optee: probing for conduit method.
[    1.390376] optee: revision 3.20 (bc149c9e)
[    1.406972] optee: dynamic shared memory is enabled
[    1.407662] optee: initialized driver
...
Starting OP-TEE Supplicant: tee-supplicant.
...

Poky (Yocto Project Reference Distro) 4.2 raspberrypi4-64 /dev/ttyS0

raspberrypi4-64 login: root
root@raspberrypi4-64:~# optee_example_hello_world
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
root@raspberrypi4-64:~# 
```
