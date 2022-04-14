#pragma once
#include "framework.h"

#define EXE_DIFF	0x400000

template<typename OUTSTUFF, typename INSTUFF>
OUTSTUFF ForceCast(INSTUFF in)
{
	union
	{
		INSTUFF  in;
		OUTSTUFF out;
	}
	u = { in };

	return u.out;
};

enum BioVersion
{
	BV_NEWEUR,		// European 1.00c
	BV_MEDIAKITE,	// Japanese 1.01
	BV_ASIA,		// UK/US for Asia
	BV_UKUSA,		// UK/US
	BV_UKUSA1,		// almost the same as UK/US
	BV_GERMAN,		// Germany
	BV_FRENCH,		// France
	BV_UKVR,		// PowerVR UK/US
	BV_JPVR,		// PowerVR Japanese
	BVV_UKUSADEMO,	//
	BV_UNSUPPORTED	// unknown version
};

#pragma pack(push, 1)
typedef struct {
	BYTE opCode;	// must be 0xE9;
	DWORD offset;	// jump offset
} JMP;

typedef struct
{
	BYTE opCode0;	// must be 0xE8
	DWORD offset0;	// call offset
	BYTE opCode1;	// must be 0xE9
	DWORD offset1;	// reroute offset
} CALLX;

#pragma pack(pop)

#define INJECT(from,to) { \
	((JMP*)(from))->opCode = 0xE9; \
	((JMP*)(from))->offset = (DWORD)(to) - ((DWORD)(from) + sizeof(JMP)); \
}

#define INJECT_EXT(from,to) (*(DWORD*)(from)) = (DWORD)(to)

#define INJECT_CALL(from,to,size)	{\
	if(size > 5)\
		memset(((void*)from), 0x90, size); \
	((JMP*)((void*)from))->opCode = 0xE8; \
	((JMP*)((void*)from))->offset = (DWORD)(to) - ((DWORD)(from) + sizeof(JMP)); \
}

#define INJECT_CALLX(from,fnc,skp)	{\
	((CALLX*)(from))->opCode0 = 0xE8; \
	((CALLX*)(from))->offset0 = (DWORD)(fnc) - ((DWORD)(from) + sizeof(JMP)); \
	((CALLX*)(from))->opCode1 = 0xE9; \
	((CALLX*)(from))->offset1 = (DWORD)(skp) - ((DWORD)(from) + sizeof(CALLX)); \
}\

enum InjectType
{
	IT_JUMP,
	IT_CALL,
	IT_EXTERN,
	IT_NOP
};