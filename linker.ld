OUTPUT_FORMAT(elf64-x86-64)
ENTRY(start16)

SECTIONS {
    . = 0x10000;
    
    .early16 : {
        *(.early16)
    }
    
    .text : {
        *(.text)
    }
    
    .data : {
        *(.data)
    }
    
    .paging : ALIGN(4096) {
        *(.paging)
    }
    
    .rodata : {
        *(.rodata)
    }
    
    .bss : {
        *(.bss)
    }
}