#include "criware.h"
#include "criware_file.h"

// ------------------------------------------------
// ADX stream code, normal file wrapping
int ADXStream::Open(const char* filename)
{
	fp = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fp == INVALID_HANDLE_VALUE)
		return S_FALSE;

	return S_OK;
}

void ADXStream::Close()
{
	CloseHandle(fp);
}

void ADXStream::Read(void* buffer, size_t size)
{
	DWORD read;
	ReadFile(fp, buffer, size, &read, nullptr);
}

void ADXStream::Seek(u_long pos, u_long mode)
{
	SetFilePointer(fp, pos, nullptr, mode);
}

// ------------------------------------------------
// AIX stream code, a lot of shit going on here
void AIXParent::Open(HANDLE fp, u_long stream_no)
{
	this->fp = fp;
	this->stream_no = stream_no;

	streams = new AIXStream[stream_no];
}

void AIXParent::Close()
{
	if (stream_no) delete[] streams;

	CloseHandle(fp);
}

void AIXStream::Read(void* buffer, size_t size)
{
}

void AIXStream::Seek(u_long pos, u_long mode)
{
}