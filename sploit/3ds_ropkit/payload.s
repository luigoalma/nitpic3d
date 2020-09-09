	.arm

	#include "../ropconstants.h"

	#define ROPKIT_LINEARMEM_REGIONBASE 0x30000000
	#define ROPKIT_LINEARMEM_BUF (ROPKIT_LINEARMEM_REGIONBASE+0x100000)

	#define ROPKIT_BINPAYLOAD_PATH ""
	@#define ROPKIT_MOUNTSAVEDATA
	#define ROPKIT_BINLOAD_ADDR ROPKIT_LINEARMEM_BUF
	#define ROPKIT_BINLOAD_SIZE 0x1000
	#define ROPKIT_TMPDATA 0x0FFFc000
	#define ROPKIT_BINLOAD_TEXTOFFSET 0x0000
	#define ROPKIT_ENABLETERMINATE_GSPTHREAD
	#define ROPKIT_BEFOREJUMP_CACHEBUFADDR 0x30000000
	#define ROPKIT_BEFOREJUMP_CACHEBUFSIZE 0x800000  //large gsgpu flush fixes our new3ds L2 cache issues - and increases stability for old3ds

	#define ROPBUFLOC(x) (x)

	#include "ropkit_ropinclude.s"

ropstackstart:
	CALLFUNC_NOSP MEMCPY, ROPKIT_BINLOAD_ADDR, 0x100000, ROPKIT_BINLOAD_SIZE, 0
	CALLFUNC_NOSP MEMCPY, ROPKIT_BINLOAD_ADDR, (miniapp), (miniappend - miniapp), 0
	#include "ropkit_boototherapp.s"
	#include "miniapp.s"

#ifdef ROP_POPR3_ADDSPR3_POPPC
ropkit_cmpobject:
	.word (ROPBUFLOC(ropkit_cmpobject) + 0x4) @ Vtable-ptr
	.fill (0x40 / 4), 4, STACK_PIVOT @ Vtable
#endif
