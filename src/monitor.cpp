#include "monitor.h"

// I got the idea of sleeping to remove user input from:
// http://www.codeproject.com/system/display_states.asp
// The rest is all in MSDN.

// We create our own window to fire messages at.
// Older versions, and a lot of example code on the net, send to HWND_BROADCAST instead but this is bad:
// http://blogs.msdn.com/oldnewthing/archive/2006/06/13/629451.aspx




// modified code from here: https://www.pretentiousname.com/miscsoft/index.html#ScreenSave
int turn_off_monitor(void)
{
	LPCTSTR szClassNameMain = _T("ScreenSaveMain");

	HINSTANCE hInstance = GetModuleHandle(NULL);

	if (hInstance == NULL)
	{
		_tprintf(_T("\nScreenSave: Could not get HINSTANCE.\n"));
		return -1;
	}

	HANDLE hSleepEvent = CreateEvent(NULL, FALSE, FALSE, _T("ScreenSave_Sleep_{CB6E7615-A2EF-43da-8431-446AFE3F3368}"));

	if (hSleepEvent == NULL)
	{
		_tprintf(_T("\nScreenSave: Could not create sleep event.\n"));
		return -1;
	}

	HANDLE hPowerOffEvent = CreateEvent(NULL, FALSE, FALSE, _T("ScreenSave_Power_{CB6E7615-A2EF-43da-8431-446AFE3F3368}"));

	if (hPowerOffEvent == NULL)
	{
		_tprintf(_T("\nScreenSave: Could not create power-off event.\n"));
		CloseHandle(hSleepEvent);
		CloseHandle(hPowerOffEvent);
		return -1;
	}

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= 0; //CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= DefWindowProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szClassNameMain;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_MAIN);

	ATOM atomMain = RegisterClassEx(&wcex);

	if (atomMain == 0)
	{
		_tprintf(_T("\nScreenSave: Could not register window class.\n"));
		CloseHandle(hSleepEvent);
		CloseHandle(hPowerOffEvent);
		return -1;
	}

	HWND hWndMain = CreateWindow(szClassNameMain, szClassNameMain, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, HWND_MESSAGE, NULL, hInstance, NULL);

	if (hWndMain == NULL)
	{
		_tprintf(_T("\nScreenSave: Could not create window.\n"));
		CloseHandle(hSleepEvent);
		CloseHandle(hPowerOffEvent);
		return -1;
	}

	const TCHAR *szReadyEventName = NULL;

	bool bPreventSleep = false;
	bool bPreventPowerOff = false;

	Sleep(500); // Stops recent user input cancelling the request.
	SendMessage(hWndMain, WM_SYSCOMMAND, SC_MONITORPOWER, 2);


	PostQuitMessage(0);

	MSG msg;
	BOOL bRet;
	while (bRet = GetMessage(&msg, NULL, 0, 0)) 
	{
		if (-1 == bRet)
		{
			break;
		}

	//	if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// We do this after closing the window so we don't leave an unpumped window around while we wait.

	if (bPreventSleep && bPreventPowerOff)
	{
		_tprintf(_T("\nScreenSave: The internal /PreventSleep and /PreventPowerOff args shouldn't be used together.\nUse the disable* arguments rather than the prevent* ones.\n"));
	}
	else if (bPreventSleep || bPreventPowerOff)
	{
		bool bWait = true;

		if (NULL == SetThreadExecutionState(
				ES_CONTINUOUS
			    | (bPreventSleep ? ES_SYSTEM_REQUIRED : 0)
				| (bPreventPowerOff ? ES_DISPLAY_REQUIRED : 0)))
		{
			_tprintf(_T("\nScreenSave: SetThreadExecutionState failed.\n"));
			bWait = false;
		}

		if (bWait && szReadyEventName != NULL)
		{
			HANDLE hReadyEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, szReadyEventName);
			
			if (hReadyEvent == NULL)
			{
				_tprintf(_T("\nScreenSave: Failed to open the ready event.\n"));
				bWait = false;
			}
			else
			{
				if (!SetEvent(hReadyEvent))
				{
					_tprintf(_T("\nScreenSave: Failed to set ready event.\n"));
					bWait = false;
				}

				CloseHandle(hReadyEvent);
			}
		}

		if (bWait)
		{
			WaitForSingleObject(bPreventSleep ? hSleepEvent : hPowerOffEvent, INFINITE);
		}
	}


	CloseHandle(hSleepEvent);
	CloseHandle(hPowerOffEvent);
	return 0;
}

void runChildProcess(HINSTANCE hInstance, const TCHAR *szOffArg)
{
	TCHAR szChildReadyEventName[128];
	_stprintf_s(szChildReadyEventName, _T("ScreenSave_Ready_%lx_{CB6E7615-A2EF-43da-8431-446AFE3F3368}"), GetCurrentProcessId());

	HANDLE hChildReadyEvent = CreateEvent(NULL, TRUE, FALSE, szChildReadyEventName);

	if (hChildReadyEvent == NULL)
	{
		_tprintf(_T("\nScreenSave: Failed to create child ready event.\n"));
	}
	else
	{
		TCHAR szOwnPath[_MAX_PATH];

		if (!GetModuleFileName(hInstance, szOwnPath, _countof(szOwnPath)))
		{
			_tprintf(_T("\nScreenSave: Failed to get own exe path.\n"));
		}
		else
		{
			LPWCH envStringsW = GetEnvironmentStringsW();

			STARTUPINFO startupInfo = {0};
			startupInfo.cb = sizeof(startupInfo);
			PROCESS_INFORMATION processInfo = {0};

			TCHAR szChildCommandLine[1024];
			_stprintf_s(szChildCommandLine, _T("ScreenSave.exe %s /readyeventname %s"), szOffArg, szChildReadyEventName);

			if (!CreateProcess(szOwnPath, szChildCommandLine, NULL, NULL, FALSE,
				               CREATE_UNICODE_ENVIRONMENT, envStringsW, NULL,
							   &startupInfo, &processInfo))
			{
				_tprintf(_T("\nScreenSave: Failed to create child process.\n"));
			}
			else
			{
				HANDLE childWaitHandles[2];
				childWaitHandles[0] = hChildReadyEvent;
				childWaitHandles[1] = processInfo.hProcess;

				if (WAIT_OBJECT_0 != WaitForMultipleObjects(_countof(childWaitHandles), childWaitHandles, FALSE, INFINITE))
				{
					_tprintf(_T("\nScreenSave: Child process failed.\n"));
				}

				CloseHandle(processInfo.hProcess);
				CloseHandle(processInfo.hThread);
			}


			if (envStringsW)
			{
				FreeEnvironmentStringsW(envStringsW);
			}
		}

		CloseHandle(hChildReadyEvent);
	}
}
