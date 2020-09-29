#pragma once

#if (defined EUR && defined USA) || (defined EUR && defined JPN) || (defined USA && defined JPN)
#error Select just one region
#endif

#if defined EUR
#define REGION(eur,usa,jpn) eur
#elif defined USA
#define REGION(eur,usa,jpn) usa
#elif defined JPN
#define REGION(eur,usa,jpn) jpn
#else
#error Define a region
#endif

#define ROP_POPPC REGION(0x0010da9c, 0x0010da8c, 0x0010d830)
#define POP_R1PC REGION(0x001f5068, 0x001f5058, 0x001f4744)
#define POP_R3PC REGION(0x0010d524, 0x0010d514, 0x0010d3e0)
#define POP_R2R6PC REGION(0x001e8654, 0x001e8644, 0x001e8d8c)
#define POP_R4LR_BXR1 REGION(0x00114824, 0x00114814, 0x00114668)
#define POP_R4R8LR_BXR2 REGION(0x0018bbfc, 0x0018bbec, 0x0018ae0c)
#define POP_R4R5R6PC REGION(0x0010d60c, 0x0010d5fc, 0x0010d4c8)
#define POP_R4FPPC REGION(0x00110744, 0x00110734, 0x00110664)
#define POP_R4R7PC REGION(0x00111630, 0x00111620, 0x00111550)
#define POP_R4R8PC REGION(0x0010d2d4, 0x0010d2c4, 0x0010d184)

#define ROP_STR_R1TOR0 REGION(0x0010f158, 0x0010f148, 0x0010eeec)
#define ROP_STR_R0TOR1 REGION(0x0010d564, 0x0010d554, 0x0010d420)
#define ROP_LDR_R0FROMR0 REGION(0x0013759c, 0x0013758c, 0x001360bc)
#define ROP_ADDR0_TO_R1 REGION(0x0012d204, 0x0012d1f4, 0x0012c72c)

#define MEMCPY REGION(0x001f4d58, 0x001f4d48, 0x001f4434)

#define svcSleepThread REGION(0x00130014, 0x00130004, 0x0012f53c)

#define GSPGPU_FlushDataCache REGION(0x0013135c, 0x0013134c, 0x00130460)
#define GSPGPU_SERVHANDLEADR REGION(0x002d3488, 0x002d3488, 0x002d2420)

#define IFile_Read REGION(0x001ee8cc, 0x001ee8bc, 0x001edf9c)
#define IFile_Write REGION(0x00126594, 0x00126584, 0x00125cdc)

#define POP_R0PC REGION(0x00126128, 0x00126118, 0x00125cb8)
#define ROP_CMPR0R1_ALT0 REGION(0x0023a250, 0x0023a240, 0x00238820)
#define MEMSET32_OTHER REGION(0x001f4470, 0x001f4460, 0x001f3b4c)
#define svcControlMemory REGION(0x001f01d4, 0x001f01c4, 0x001ef894)
#define ROP_INITOBJARRAY REGION(0x001f3345, 0x001f3335, 0x001f2a25)
#define svcCreateThread REGION(0x0011c440, 0x0011c430, 0x0011c984)
#define svcConnectToPort REGION(0x001260cc, 0x001260bc, 0x00125c5c)
#define svcGetProcessId REGION(0x001260e4, 0x001260d4, 0x00125c74)
#define SRV_GETSERVICEHANDLE REGION(0x001f0268, 0x001f0258, 0x001ef928)
#define ROP_COND_THROWFATALERR REGION(0x0010878c, 0x0010878c, 0x00108674)
#define GXLOW_CMD4 REGION(0x00131460, 0x00131450, 0x00130564)
#define GSP_SHAREDMEM_SETUPFRAMEBUF REGION(0x00137c40, 0x00137c30, 0x00136760)
#define GSPTHREAD_OBJECTADDR REGION(0x002cf580, 0x002cf580, 0x002ce580)
//#define FS_MountSdmc REGION(0x0013060c, 0x001305fc, 0x0012f710) // no direct SDMC access
//#define FS_MountSavedata REGION(0x00134cf4, 0x00134ce4, 0x00133998)
#define IFile_Open REGION(0x001ee9ac, 0x001ee99c, 0x001ee07c)
#define IFile_Close REGION(0x001ee8b4, 0x001ee8a4, 0x001edf84)
#define IFile_Seek REGION(0x001e6bb4, 0x001e6ba4, 0x001e7638)

#define ExceptionHandler_SET_R4R5R6_POP_R4R5R6PC REGION(0x00113680, 0x00113670, 0x001134e0)

#define SAVEDATA_ADDRESS REGION(0x08882f98, 0x08881c48, 0x0886ab78)
#define GARBAGE 0xDEADBEEF

// ldm r0!, {r12-pc}
// this instruction is sitting on top of a THUMB *BLX* instruction
// amazingly, the same BLX is seen in all 3 regions, from the same function, jumping to the same other function
// they both at the **exact** same position relative to each other
// and properly aligned for ARM mode, go figure
// also, thank you zoogie for finding this one
#define STACK_PIVOT REGION(0x001f430c, 0x001f42fc, 0x001f39e8)

#define Thread_Exit REGION(0x0021ee48, 0x0021ee38, 0x0021d41c)
#define BreakHandlerPtr REGION(0x002d2d20, 0x002d2d20, 0x002d1cb8)
#define SRV_Notification_Semaphore REGION(0x002ddaa4, 0x002ddaa4, 0x002dca3c)
#define SRV_THREAD_HANDLE REGION(0x002ddaa8, 0x002ddaa8, 0x002dca40)
#define LightSemaphore_Release REGION(0x001e26c4, 0x001e26b4, 0x001e3900)
#define DSP_ThreadStop REGION(0x00135b64, 0x00135b54, 0x00134808)
#define ERRF_THROW REGION(0x001f029c, 0x001f028c, 0x001ef95c)

// sound/dsp related clear functions
#define SOUND_CLEAR1 REGION(0x001e9604, 0x001e95f4, 0x001e9d18)
#define SOUND_CLEAR2 REGION(0x001143e0, 0x001143d0, 0x00114224)
#define DSP_Exit REGION(0x00113dc0, 0x00113db0, 0x00113c04)

// some thread data
// generic names because I'm unsure what to call these
#define SOMETHREAD1EVENT REGION(0x002d9b74, 0x002d9b74, 0x002d8b0c)
#define SOMETHREAD1TERMINATIONFLAG REGION(0x002d33ec, 0x002d33ec, 0x002d2384)
#define SOMETHREAD1OBJ REGION(0x002d33f8, 0x002d33f8, 0x002d2390)
#define SOMETHREAD2OBJ REGION(0x002e286c, 0x002e286c, 0x002e180c)

// to overflow a multiplication of 200
// Will make r0 point at or near save in memory
// EUR: -0x8
// USA: -0x8
// JPN: -0xC
// Object0 is then accessed with mov r0, [r0, #0x24]
// despite languages on EUR and USA moving around memory, index overflow value is the same
#define OVERFLOW_VALUE REGION(0x07AE1292, 0x07AE1292, 0x0147AC18)
