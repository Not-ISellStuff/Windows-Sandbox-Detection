#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <psapi.h>

int cCPU() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);

    if (strstr(info.dwProcessorType == 0x00010000 ? "VMware" : "", "VMware") != NULL ||
        strstr(info.dwProcessorType == 0x00020000 ? "VirtualBox" : "", "VirtualBox") != NULL ||
        strstr(info.dwProcessorType == 0x00030000 ? "Hyper-V" : "", "Hyper-V") != NULL) {
        return 1;
    }

    return 0;
}

int cPROC() {
    DWORD processes[1024], needed, count;
    unsigned int i;

    if (!EnumProcesses(processes, sizeof(processes), &needed)) {
        return 0;
    }

    count = needed / sizeof(DWORD);

    for (i = 0; i < count; i++) {
        char process_name[MAX_PATH] = {0};
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
        
        if (NULL == hProcess) continue;

        HMODULE hMod;
        DWORD cbNeeded;
        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
            GetModuleBaseNameA(hProcess, hMod, process_name, sizeof(process_name) / sizeof(char));

            if (strstr(process_name, "vmware") != NULL || strstr(process_name, "vboxservice") != NULL || 
                strstr(process_name, "qemu") != NULL || strstr(process_name, "VBoxClient") != NULL) {
                CloseHandle(hProcess);
                return 1;
            }
        }

        CloseHandle(hProcess);
    }

    return 0;
}

int cREG() {
    HKEY hKey;
    LONG result;

    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\VMware, Inc.", 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return 1;
    }

    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Oracle\\VirtualBox", 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return 1;
    }

    return 0;
}

int main() {
    if (cCPU()) {
        exit(EXIT_FAILURE);
    }

    if (cPROC()) {
        exit(EXIT_FAILURE);
    }

    if (cREG()) {
        exit(EXIT_FAILURE);
    }

    printf("[+] no sandbox detected");
    
    exit(EXIT_SUCCESS);
}
