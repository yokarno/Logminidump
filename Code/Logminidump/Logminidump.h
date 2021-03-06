/********************************************************************
(c) Copyright 2014-2016 Mettler-Toledo CT. All Rights Reserved.

File Name: 		Logminidump.h
File Path:		

Description:	Logminidump

Author:			Zhong Min
Created:		2016/10/31 10:33
Remark:	        Logminidump
*********************************************************************/


#pragma once

#include <windows.h>
#include <DbgHelp.h>
#include <stdlib.h>
#pragma comment(lib, "dbghelp.lib")

inline BOOL IsDataSectionNeeded(const WCHAR *pModuleName)
{
    if(pModuleName == 0)
    {
        return FALSE;
    }

    WCHAR szFileName[_MAX_FNAME] = L"";
    _wsplitpath(pModuleName, NULL, NULL, szFileName, NULL);

    if(wcsicmp(szFileName, L"ntdll") == 0)
    { return TRUE; }

    return FALSE;
}

inline BOOL CALLBACK MiniDumpCallback(PVOID pParam,
                                      const PMINIDUMP_CALLBACK_INPUT   pInput,
                                      PMINIDUMP_CALLBACK_OUTPUT        pOutput)
{
    if(pInput == 0 || pOutput == 0)
    { return FALSE; }

    switch(pInput->CallbackType)
    {
    case ModuleCallback:
        if(pOutput->ModuleWriteFlags & ModuleWriteDataSeg)
            if(!IsDataSectionNeeded(pInput->Module.FullPath))
            { pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg); }

    case IncludeModuleCallback:
    case IncludeThreadCallback:
    case ThreadCallback:
    case ThreadExCallback:
        return TRUE;

    default:
        ;
    }

    return FALSE;
}

inline void CreateMiniDump(PEXCEPTION_POINTERS pep, LPCTSTR strFileName)
{
    HANDLE hFile = CreateFile(strFileName, GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
    {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId           = GetCurrentThreadId();
        mdei.ExceptionPointers  = pep;
        mdei.ClientPointers     = NULL;

        MINIDUMP_CALLBACK_INFORMATION mci;
        mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;
        mci.CallbackParam       = 0;

        ::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, (pep != 0) ? &mdei : 0, NULL, &mci);

        CloseHandle(hFile);
    }
}

LONG __stdcall MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
    const char * strFileName = "MTScaleKey.DMP";
    CreateMiniDump(pExceptionInfo, strFileName);

    return EXCEPTION_EXECUTE_HANDLER;
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
    return NULL;
}

// 此函数一旦成功调用，之后对 SetUnhandledExceptionFilter 的调用将无效
void DisableSetUnhandledExceptionFilter()
{
    void *addr = (void *)GetProcAddress(LoadLibrary("kernel32.dll"),
                                        "SetUnhandledExceptionFilter");

    if (addr) {
        unsigned char newJump[100];
        DWORD dwOrgEntryAddr = (DWORD)addr;
        dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far


        void *pNewFunc = &MyDummySetUnhandledExceptionFilter;
        DWORD dwNewEntryAddr = (DWORD)pNewFunc;
        DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;


        newJump[0] = 0xE9;  // JMP absolute
        memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
        SIZE_T bytesWritten;
        BOOL bRet = WriteProcessMemory(GetCurrentProcess(), addr, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
    }
}

void InitMinDump()
{
    //注册异常处理函数
    SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

    //使SetUnhandledExceptionFilter
    DisableSetUnhandledExceptionFilter();
}