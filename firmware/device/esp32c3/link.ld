ENTRY(_start)

MEMORY
{
    ram (rwx) : ORIGIN = 0x40380000, LENGTH = 320K
}

SECTIONS
{
    .text :
    {
        KEEP(*(.entry_addr))
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
    } > ram

    .data :
    {
        *(.data)
        *(.data.*)
    } > ram

    .bss :
    {
        _bss_start = .;
        *(.bss)
        *(.bss.*)
        _bss_end = .;
    } > ram

    .stack (NOLOAD) :
    {
        . = ALIGN(16);
        _stack_bottom = .;
        . += 0x4000;
        _stack_top = .;
    } > ram
}

PROVIDE(printf = 0x40000040);
