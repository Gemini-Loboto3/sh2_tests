#pragma once
#include <windows.h>

//-------------------------------------------
// main exposed module
#define	ADXF_STAT_STOP			(1)			/*	During standstill			*/
#define ADXF_STAT_READING		(2)			/*	During data read-in			*/
#define ADXF_STAT_READEND		(3)			/*	Data read-in end			*/
#define ADXF_STAT_ERROR			(4)			/*	Read-in error outbreak state*/

#define	ADXT_STAT_STOP		(0)		/*	During standstill					*/
#define ADXT_STAT_DECINFO	(1)		/*	Getting header information			*/
#define ADXT_STAT_PREP		(2)		/*	During play preparation				*/
#define ADXT_STAT_PLAYING	(3)		/*	During decode and play				*/
#define ADXT_STAT_DECEND	(4)		/*	Decode end							*/
#define ADXT_STAT_PLAYEND	(5)		/*	Play end							*/
#define ADXT_STAT_ERROR		(6)		/*	Read-in error outbreak state		*/

void* ADXFIC_Create(const char* dname, int mode, char* work, int wksize);

int ADXF_LoadPartitionNw(int ptid, const char* filename, void* ptinfo, void* nfile);
int _ADXF_LoadPartitionNw(int ptid, const char* filename, void* ptinfo, void* nfile);

//-------------------------------------------
// file server module
extern HANDLE hServer;

DWORD WINAPI server_thread(LPVOID params);

//-------------------------------------------
class BE16
{
public:
	__inline WORD w() { return (d[0] << 8) | (d[1]); }
	__inline short s() { return (short)w(); }
private:
	BYTE d[2];
};

class BE32
{
public:
	__inline DWORD dw() { return (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | (d[3]); }
	__inline int i() { return (int)dw(); }
private:
	BYTE d[4];
};

//-------------------------------------------
typedef struct ADX_headerV3
{
	BE16 magic;					// 0
	BE16 copyright_offset;		// 2
	BYTE encoding,				// 4
		block_size,				// 5
		sample_bitdepth,		// 6
		channel_count;			// 7
	BE32 sample_rate,			// 8
		total_samples;			// C
	BE16 highpass_frequency;	// 10
	BYTE version,				// 12
		flags;					// 13
	BE16 loop_align_samples,	// 14
		loop_enabled;			// 16
	BE32 loop_enabled2,			// 18
		loop_sample_begin,		// 1C
		loop_byte_begin,		// 20
		loop_sample_end,		// 24
		loop_byte_end,			// 28
		dummy0,					// 2C
		dummy1,					// 30
		dummy2,					// 34
		dummy3,					// 38
		dummy4;					// 3C
} ADX_headerV3;

typedef struct ADX_headerV4
{
	BE16 magic;					// 0
	BE16 copyright_offset;		// 2
	BYTE encoding,				// 4
		block_size,				// 5
		sample_bitdepth,		// 6
		channel_count;			// 7
	BE32 sample_rate,			// 8
		total_samples;			// C
	BE16 highpass_frequency;	// 10
	BYTE version,				// 12
		flags;					// 13
	BE16 loop_align_samples,	// 14
		dummy0;					// 16
	BE32 dummy1,				// 18
		dummy2,					// 1C
		dummy3,					// 20
		loop_enabled,			// 24
		loop_sample_begin,		// 28
		loop_byte_begin,		// 2C
		loop_sample_end,		// 30
		loop_byte_end,			// 34
		dummy4,					// 38
		dummy5;					// 3C
} ADX_headerV4;

typedef struct ADX_Handle
{
	HANDLE fp;
	//
	u_long copyright_offset,
		block_size,
		sample_bitdepth,
		channel_count,
		sample_rate,
		total_samples,
		highpass_frequency,
		loop_enabled,
		loop_start_index,
		loop_end_index;
	int* past_samples;		// Previously decoded samples from each channel, zeroed at start (size = 2*channel_count)
	u_long sample_index;		// sample_index is the index of sample set that needs to be decoded next
	double coefficient[2];
} ADX_HANDLE;

typedef struct ADX {} ADX;

void adx_set_coeff(ADX_HANDLE* adx);
unsigned decode_adx_standard(ADX_HANDLE* adx, short* buffer, unsigned samples_needed, bool looping_enabled);

int OpenADX(const char* filename, ADX** obj);
void CloseADX(ADX* obj);

//-------------------------------------------
// generic header
typedef struct AIX_HEADER
{
	BYTE magic[3],	// 'AIX'
		type;		// F, P, E
	BE32 next;	// big endian, offset to next header + 8
} AIX_HEADER;

// file header
typedef struct AIXF_HEADER
{
	BYTE magic[4];	// AIXF
} AIXF_HEADER;

// properties header
typedef struct AIXP_HEADER
{
	BYTE magic[4];	// AIXP
	BE32 next;
	BYTE in_channels,
		out_channels;
	BE16 size,
		frames;
} AIXP_HEADER;

// end header
typedef struct AIXE_HEADER
{
	BYTE magic[4];	// AIXE
} AIXE_HEADER;

typedef struct AIX {} AIX;

int OpenAIX(const char* filename, AIX** obj);
void CloseAIX(AIX* obj);
