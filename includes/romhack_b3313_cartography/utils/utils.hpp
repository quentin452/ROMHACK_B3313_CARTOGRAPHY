#pragma once
#include <iostream>
#include <psapi.h>
#include <string>
#include <tlhelp32.h>
#include <windows.h>


std::string GetProcessPath(const std::string &processName) {
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return "Error: Unable to create snapshot";
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return "Error: Unable to get first process";
    }

    do {
        if (processName == pe32.szExeFile) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
            if (hProcess) {
                char path[MAX_PATH];
                if (GetModuleFileNameEx(hProcess, NULL, path, MAX_PATH)) {
                    CloseHandle(hProcess);
                    CloseHandle(hProcessSnap);
                    return std::string(path);
                }
                CloseHandle(hProcess);
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return "Process not found";
}