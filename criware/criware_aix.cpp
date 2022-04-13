#include "criware.h"

typedef struct AIX_Handle
{
	HANDLE fp;
} AIX_HANDLE;

int OpenAIX(const char* filename, AIX**obj)
{
	*obj = nullptr;

	HANDLE fp = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fp == INVALID_HANDLE_VALUE)
		return 0;

	AIX_Handle* aix = new AIX_Handle;
	aix->fp = fp;

	// parse header and data
	AIX_HEADER head;
	DWORD read;
	ReadFile(fp, &head, sizeof(head), &read, nullptr);

	bool loop = true;

	//AIXP_HEADER* p;
	//AIXF_HEADER* f;
	//AIXE_HEADER* e;

	int cp = 0, cf = 0, ce = 0;
	while (loop)
	{
		switch (head.type)
		{
		case 'P':
			cp++;
			break;
		case 'E':
			ce++;
			break;
		case 'F':
			cf++;
			break;
		}

		SetFilePointer(fp, head.next.dw(), 0, FILE_CURRENT);
		ReadFile(fp, &head, sizeof(head), &read, nullptr);
		if (read != sizeof(head))
			break;
	}

	*obj = (AIX*)aix;
	return 1;
}

void CloseAIX(AIX* obj)
{
	AIX_Handle* aix = (AIX_Handle*)obj;

	if (aix->fp)
	{
		CloseHandle(aix->fp);
		aix->fp = nullptr;
	}

	delete obj;
}