// Logminidump.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string>
#include <windows.h>

#include <Iphlpapi.h>
#include <WinDef.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib,"Iphlpapi.lib")
using namespace std;

#include "Logminidump.h"

int _tmain(int argc, _TCHAR* argv[])
{
    getchar();

    char *szBuffer = new char[100];

    delete szBuffer;
    szBuffer = NULL;

    int n = strlen(szBuffer);

    char *szBuffer1 = new char[n+100];
    delete szBuffer1;
    
    getchar();
	return 0;
}

