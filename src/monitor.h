#ifndef MONITOR_H
#define MONITOR_H
    #include "stdafx.h"
    #include "resource.h"

    int turn_off_monitor(void);
    void runChildProcess(HINSTANCE hInstance, const TCHAR *szOffArg);
    
#endif // MONITOR_H