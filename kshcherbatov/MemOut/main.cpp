#include <iostream>
#include <Windows.h>
#include <Dbghelp.h>

#include <new>
#include <stdio.h>
#include <tchar.h>
#include <mapi.h>

#pragma comment (lib, "Dbghelp.lib")

#define MSG_PART_MAX_SIZE 1024

using namespace std;

static inline void PrefetchMem(volatile INT32 *MemBuff, ULONG32 MemBuffSize, ULONG32 IteratorStep);
void StrStack(char *str);
void StrSystemInfo(SYSTEM_INFO *SystemInfo, char *str);
bool SendEmail2Developer(char *msg);


int main(int argc, char *argv[]) {
	cout << "Hi! This is memory alocation failure test program. It will fail all.. soon)" << endl;

	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	ULONG32 IteratorStep = SystemInfo.dwPageSize >> 1;
	ULONG32 BufferSize = 1024;

	while (BufferSize*sizeof(INT32) < ULLONG_MAX) {
		try {
			INT32 *Buffer = new INT32[BufferSize*sizeof(INT32)];
			PrefetchMem(Buffer, BufferSize, IteratorStep);
			cout << "Memory allocated! BufferSize = " << BufferSize << endl;
		}
		catch (std::bad_alloc& Err) {
			std::cerr << "bad_alloc caught: " << Err.what() << '\n';

			if (MessageBox(NULL, _T("Send e-mail report to developer?"),
				_T("An error has occurred"), 
				MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON1) == IDYES)
			{
				char SysInfoStr[MSG_PART_MAX_SIZE + 1] = {};
				StrSystemInfo(&SystemInfo, SysInfoStr);

				char StackStr[MSG_PART_MAX_SIZE + 1] = {};
				StrStack(StackStr);

				char msg[4*MSG_PART_MAX_SIZE + 1] = {};

				SYSTEMTIME st;
				GetSystemTime(&st);

				snprintf(msg, 4 * MSG_PART_MAX_SIZE, 
					"An error in program has occurred\n"
					"bad_alloc caught: %s\n"
					"Program: \"%s\";\n"
					"Date %02d.%02d.%d %02d:%02d\n"
					"File \"%s\"; Line %d\n"
					"\n"
					"System information:\n"
					"%s"
					"\n"
					"Stack backtrace:\n"
					"%s",
					Err.what(),
					argv[0],
					st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute,
					__FILE__, __LINE__,
					SysInfoStr,
					StackStr);

				if (!SendEmail2Developer(msg))
					MessageBox(NULL, _T("Failed send e-mail report..."),
						_T("Failed"),
						MB_OK | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
			}
			break;
		}

		BufferSize <<= 1;
	}

	return EXIT_SUCCESS;
}


static inline void PrefetchMem(volatile INT32 *MemBuff, ULONG32 MemBuffSize, ULONG32 IteratorStep) {
	for (ULONG32 i = 0; i < MemBuffSize; i += IteratorStep) {
		INT32 temp = MemBuff[i];
	}
}

bool SendEmail2Developer(char *msg) {
	HMODULE m_hLib = LoadLibrary(_T("MAPI32.DLL"));

	if (!m_hLib)
		return false;

	LPMAPISENDMAIL SendMail = (LPMAPISENDMAIL)GetProcAddress(m_hLib, "MAPISendMail");
	if (!SendMail) {
		FreeLibrary(m_hLib);
		return false;
	}

	MapiRecipDesc recipient[1] = { 0 };
	recipient[0].ulRecipClass = MAPI_TO;
	recipient[0].lpszAddress = "kshcherbatov@gmail.com";
	recipient[0].lpszName = "The Developer";

	MapiMessage message;
	ZeroMemory(&message, sizeof(message));
	message.lpszSubject = "Amazing program failed :C";

	message.lpRecips = recipient;
	message.nRecipCount = 1;

	message.lpszNoteText = msg;
	int nError = SendMail(0, 0, &message, MAPI_LOGON_UI | MAPI_DIALOG , 0);

	if (nError != SUCCESS_SUCCESS && nError != MAPI_USER_ABORT && nError != MAPI_E_LOGIN_FAILURE)
		return false;

	FreeLibrary(m_hLib);

	return true;
}

void StrStack(char *str) {
	unsigned int i;
	void *stack[256];
	unsigned short frames;
	SYMBOL_INFO *symbol;
	HANDLE process;

	int str_write_idx = 0;

	process = GetCurrentProcess();

	SymInitialize(
		process, 
		NULL, // No user-defined search path -> use default 
		TRUE  // Load symbols for modules in the current process 
		);

	frames = CaptureStackBackTrace(0, 100, stack, NULL);
	symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
	symbol->MaxNameLen = 100;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	for (i = 0; i < frames; i++) {
		SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
		str_write_idx += snprintf(&str[str_write_idx], 200, 
			"%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address);
	}

	free(symbol);
}


void StrSystemInfo(SYSTEM_INFO *SystemInfo, char *str) {
	int str_write_idx = 0;
	snprintf(str, MSG_PART_MAX_SIZE - 1,
		"dwPageSize = %d\n"
		"lpMinimumApplicationAddress = %p\n"
		"lpMaximumApplicationAddress = %p\n"
		"dwActiveProcessorMask = %ul\n"
		"dwNumberOfProcessors = %ul\n"
		"dwProcessorType = %ul\n"
		"SystemInfo->wProcessorArchitecture = %u\n"
		"wProcessorLevel = %u\n"
		"wProcessorRevision = %u\n"
		"dwAllocationGranularity = %ul\n",
		SystemInfo->dwPageSize,
		SystemInfo->lpMinimumApplicationAddress,
		SystemInfo->lpMaximumApplicationAddress,
		SystemInfo->dwActiveProcessorMask,
		SystemInfo->dwNumberOfProcessors,
		SystemInfo->dwProcessorType,
		SystemInfo->wProcessorArchitecture,
		SystemInfo->wProcessorLevel,
		SystemInfo->wProcessorRevision,
		SystemInfo->dwAllocationGranularity
		);
}