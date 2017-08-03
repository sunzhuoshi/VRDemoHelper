#ifndef ARGCARGV_H
#define ARGCARGV_H

#include <tchar.h>

extern TCHAR * _ppszArgv[];

int __cdecl _ConvertCommandLineToArgcArgv(LPCTSTR lpszSysCmdLine);

#endif //ARGCARGV_H
