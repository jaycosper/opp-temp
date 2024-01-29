ENTRY(Reset_Handler)
MEMORY
{
    ROM  (rx): ORIGIN = 0x08000000, LENGTH =  64K
    RAM (rwx): ORIGIN = 0x20000004, LENGTH =  20476  /* 4 bytes < 20K */
}

INCLUDE sections.ld
