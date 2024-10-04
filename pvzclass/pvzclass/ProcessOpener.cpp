﻿#include "ProcessOpener.h"
#include <TLHELP32.H>
#include <direct.h>
#include <string>
LPCWSTR ProcessOpener::ProcessName = L"PlantsVsZombies.exe";
LPCWSTR ProcessOpener::WindowTitle = L"Plants vs. Zombies";
LPCWSTR ProcessOpener::Directory = L".";
LPCWSTR ProcessOpener::ExecuteableName = L"PlantsVsZombies.exe";

DWORD ProcessOpener::OpenByProcessName(LPCWSTR processName)
{
	DWORD pid = NULL;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)return NULL;
	BOOL ret = Process32First(hProcessSnap, &pe32);
	while (ret)
	{
		if (!strcmp(pe32.szExeFile, "PlantsVsZombies.exe"))pid = pe32.th32ProcessID;
		ret = Process32Next(hProcessSnap, &pe32);
	}
	CloseHandle(hProcessSnap);
	return pid;
}

DWORD ProcessOpener::OpenByWindowTitle(LPCWSTR processName)
{
	HWND hwnd = FindWindow(NULL, (LPCSTR) processName);
	DWORD pid = NULL;
	GetWindowThreadProcessId(hwnd, &pid);
	return pid;
}

DWORD ProcessOpener::OpenByFilePath(LPCWSTR directory, LPCWSTR executeableName)
{
	int _ = _wchdir(directory);
	STARTUPINFO startupInfo = { 0 };
	PROCESS_INFORMATION processInformation = { 0 };
	BOOL ret = CreateProcess((LPCSTR) executeableName, NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &startupInfo, &processInformation);
	if (ret)
	{
		CloseHandle(processInformation.hProcess);
		CloseHandle(processInformation.hThread);
		return processInformation.dwProcessId;
	}
	return NULL;
}

DWORD ProcessOpener::Open()
{
	DWORD pid = OpenByProcessName(ProcessName);
	if (pid)return pid;
	pid = OpenByWindowTitle(WindowTitle);
	if (pid)return pid;
	return OpenByFilePath(Directory, ExecuteableName);
}
