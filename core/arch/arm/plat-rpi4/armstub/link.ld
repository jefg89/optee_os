MEMORY
{
  RAM(rwx) : ORIGIN = 0x0, LENGTH = 0x80000000 /* 2GiB */
}

SECTIONS
{
  __StackTop = ORIGIN(RAM) + LENGTH(RAM);
  __OpteeOsEntry = ORIGIN(RAM) + 0x1000;
  output :
  {
    . = ORIGIN(RAM);
    armstub8.o
    uart.o
    . = __OpteeOsEntry;
  }
}
