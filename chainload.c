#include <windows.h>

#define MY_MAX_PATH 32768

char* my_strrchr(const char* str, char ch) {
    char* last = NULL;
    while(*str) {
        if(*str == ch) last = (char*)str;
        str++;
    }
    return last;
}

int main(void) {
    char src[MY_MAX_PATH], tgt[MY_MAX_PATH], log[MY_MAX_PATH], buf[MY_MAX_PATH];
    DWORD written;
    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    
    GetModuleFileNameA(0, src, MY_MAX_PATH);
    lstrcpyA(tgt, src);
    char* dot = my_strrchr(tgt, '.');
    if(dot) lstrcpyA(dot, ".target.exe");

    char* slash = my_strrchr(src, '\\');
    if(slash) *slash = 0;
    
    wsprintfA(log, "%s\\chainload_%lu.log", src, GetTickCount());

    HANDLE hFile = CreateFileA(log, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(hFile == INVALID_HANDLE_VALUE) ExitProcess(1);

    // Basic info
    DWORD len = wsprintfA(buf, "PID:%lu TID:%lu\nSrc:%s\nTgt:%s\nCmd:%s\n\n",
        GetCurrentProcessId(), GetCurrentThreadId(), src, tgt, GetCommandLineA());
    WriteFile(hFile, buf, len, &written, 0);

    // Environment variables
    WriteFile(hFile, "Environment:\n", 13, &written, 0);
    LPCH env = GetEnvironmentStrings();
    LPCH ptr = env;
    while(*ptr) {
        DWORD len = lstrlenA(ptr);
        WriteFile(hFile, ptr, len, &written, 0);
        WriteFile(hFile, "\n", 1, &written, 0);
        ptr += len + 1;
    }
    FreeEnvironmentStrings(env);
    WriteFile(hFile, "\n", 1, &written, 0);

    // Check target
    if(GetFileAttributesA(tgt) == INVALID_FILE_ATTRIBUTES) {
        len = wsprintfA(buf, "Error: Target not found\n");
        WriteFile(hFile, buf, len, &written, 0);
        CloseHandle(hFile);
        ExitProcess(1);
    }

    // Launch process
    if(CreateProcessA(tgt, GetCommandLineA(), 0, 0, TRUE, 0, 0, 0, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, &len);
        wsprintfA(buf, "Exit code: %lu\n", len);
        WriteFile(hFile, buf, lstrlenA(buf), &written, 0);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        len = wsprintfA(buf, "Launch failed: %lu\n", GetLastError());
        WriteFile(hFile, buf, len, &written, 0);
    }

    CloseHandle(hFile);
    ExitProcess(0);
    return 0;
}