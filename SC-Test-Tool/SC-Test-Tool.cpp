// SC-Test-Tool.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>

typedef int(__cdecl *PRINTF)(
	const char *format, ...
	);
PRINTF printf;

void print_banner()
{
	printf("SCT v1.0 (x86) (SC Test tool) - Copyright (c) 2017 (bluewolker@yahoo.com)\n");
	printf("https://github.com/bluewolker/sc-test-tool\n\n");
}

PCHAR* CommandLineToArgvA(PCHAR CmdLine, int* _argc)
{
	PCHAR* argv;
	PCHAR  _argv;
	ULONG   len;
	ULONG   argc;
	CHAR   a;
	ULONG   i, j;

	BOOLEAN  in_QM;
	BOOLEAN  in_TEXT;
	BOOLEAN  in_SPACE;

	len = strlen(CmdLine);
	i = ((len + 2) / 2) * sizeof(PVOID) + sizeof(PVOID);

	argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
		i + (len + 2) * sizeof(CHAR));

	_argv = (PCHAR)(((PUCHAR)argv) + i);

	argc = 0;
	argv[argc] = _argv;
	in_QM = FALSE;
	in_TEXT = FALSE;
	in_SPACE = TRUE;
	i = 0;
	j = 0;

	while (a = CmdLine[i]) {
		if (in_QM) {
			if (a == '\"') {
				in_QM = FALSE;
			}
			else {
				_argv[j] = a;
				j++;
			}
		}
		else {
			switch (a) {
			case '\"':
				in_QM = TRUE;
				in_TEXT = TRUE;
				if (in_SPACE) {
					argv[argc] = _argv + j;
					argc++;
				}
				in_SPACE = FALSE;
				break;
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				if (in_TEXT) {
					_argv[j] = '\0';
					j++;
				}
				in_TEXT = FALSE;
				in_SPACE = TRUE;
				break;
			default:
				in_TEXT = TRUE;
				if (in_SPACE) {
					argv[argc] = _argv + j;
					argc++;
				}
				_argv[j] = a;
				j++;
				in_SPACE = FALSE;
				break;
			}
		}
		i++;
	}
	_argv[j] = '\0';
	argv[argc] = NULL;

	(*_argc) = argc;
	return argv;
}

int main()
{
	HINSTANCE hMsvcrt = LoadLibrary("msvcrt.dll");
	printf = (PRINTF)GetProcAddress(hMsvcrt, "printf");

	print_banner();

	char* pCommandLineStr = GetCommandLineA();
	int argc;
	char** argv = CommandLineToArgvA(pCommandLineStr, &argc);

	if (argc < 2)
	{
		printf("Usage: %s [FileName]\n", argv[0]);
		return -1;
	}

	char* pFileName = argv[1];
	HANDLE hFile = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile %s Error:%d\n", pFileName, GetLastError());
		return -1;
	}

	DWORD dwFileSize = GetFileSize(hFile, 0);
	if (dwFileSize <= 0 || dwFileSize > 10 * 1024 * 1024)
	{
		printf("GetFileSize Error:%d\n", GetLastError());
		CloseHandle(hFile);
		return -1;
	}

	BYTE* pMem = (BYTE*)VirtualAlloc(0, dwFileSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	DWORD dwRead;
	if (!ReadFile(hFile, pMem, dwFileSize, &dwRead, 0))
	{
		printf("ReadFile Error:%d\n", GetLastError());
		CloseHandle(hFile);
		return -1;
	}

	CloseHandle(hFile);

	((void(*)(void))pMem)();

	ExitProcess(0);
}

