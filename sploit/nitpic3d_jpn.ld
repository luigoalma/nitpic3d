OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY
{
	savedata (rw) : ORIGIN = 0x0886ab78, LENGTH = 45688
}

SECTIONS {
	. = ORIGIN(savedata);
	PROVIDE(__start__ = .);

	.payload : {
		KEEP( *(.payload.header) )
		. = ALIGN(4);
		KEEP( *(.payload.ROP) )
		. = ALIGN(4);
		PROVIDE(__otherapp_start__ = .);
		KEEP( *(.payload.otherapp) )
		. = (ORIGIN(savedata) + 0xb264);
		PROVIDE(__otherapp_end__ = .);
		KEEP( *(.payload.end) )
		. = LENGTH(savedata);
	} > savedata =0x41
}

__otherapp_size__ = __otherapp_end__ - __otherapp_start__;
__otherapp_savefile_start__ = __otherapp_start__ - __start__;
