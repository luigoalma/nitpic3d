OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS {
	. = 0;

	.text : {
		PROVIDE(__payload_start__ = .);
		KEEP( *(.payload.header) )
		. = ALIGN(4);
		KEEP( *(.payload.ROP) )
		. = ALIGN(4);
		PROVIDE(__otherapp_start__ = .);
		KEEP( *(.payload.otherapp) )
		. = __payload_start__ + 0xb264;
		PROVIDE(__otherapp_end__ = .);
		KEEP( *(.payload.end) )
		. = __payload_start__ + 0xb278;
	} =0x41
}

__otherapp_size__ = __otherapp_end__ - __otherapp_start__;
__otherapp_savefile_start__ = __otherapp_start__ - __payload_start__;
