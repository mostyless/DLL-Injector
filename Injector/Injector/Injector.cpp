#include "Windows.h"
#include "stdio.h"

bool static inject(const char* dllPath, int pid) {
    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD, FALSE, pid);
    if (!hProcess) {
        printf("[-] Failed to open specified process\n");
        return false;
    }
    printf("[+] Successfully opened specified process\n");

    auto injectionAddr = VirtualAllocEx(hProcess, nullptr, strlen(dllPath), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!injectionAddr) {
        printf("[-] Failed to allocate memory\n");
        return false;
    }
    printf("[+] Successfully allocated memory\n");

    WriteProcessMemory(hProcess, injectionAddr, dllPath, strlen(dllPath), nullptr);

    auto hKernel = GetModuleHandleA("kernel32.dll");
    if (!hKernel) {
        printf("[-] Failed to get kernel32.dll handle\n");
        return false;
    }
    printf("[+] Successfully got kernel32.dll handle\n");

    auto funcAddr = GetProcAddress(hKernel, "LoadLibraryA");
    if (!funcAddr) {
        printf("[-] Failed to get LoadLibraryA function address\n");
        return false;
    }
    printf("[+] Successfully got LoadLibraryA function address\n");

    auto hThread = CreateRemoteThread(
        hProcess,
        nullptr,
        0,
        (LPTHREAD_START_ROUTINE)funcAddr,
        injectionAddr,
        0,
        nullptr
    );
    if (!hThread) {
        printf("[-] Failed to create remote thread\n");
        return false;
    }
    printf("[+] Successfully created remote thread\n");

    if (!CloseHandle(hThread)) {
        printf("[-] Failed to close thread handle\n");
        return false;
    }
    printf("[+] Successfully closed thread handle\n");
    if (!CloseHandle(hProcess)) {
        printf("[-] Failed to close process handle\n");
        return false;
    }
    printf("[+] Successfully closed process handle\n");

    return true;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: Injector <dll path> <pid>");
        return -1;
    }

    if (inject(argv[1], atoi(argv[2])) == false) {
        return -1;
    }
    return 0;
}
