/* cores4.asi: The Saboteur refuses to run on CPUs with more than 4 cores (Steam exe is SecuROM/Steam-wrapped, so no disk
   patch). Loaded by ThirteenAG's dinput8.dll ASI loader from scripts\. At attach: pin the process to the first 4 logical
   CPUs and make GetSystemInfo report 4 processors (IAT patch in the main module).  Build: see build.cmd (MSVC x86). */
#include <windows.h>

static void (WINAPI *real_GetSystemInfo)(LPSYSTEM_INFO) = 0;
static void (WINAPI *real_GetNativeSystemInfo)(LPSYSTEM_INFO) = 0;

static void WINAPI my_GetSystemInfo(LPSYSTEM_INFO si)
{
    real_GetSystemInfo(si);
    if (si->dwNumberOfProcessors > 4) si->dwNumberOfProcessors = 4;
    si->dwActiveProcessorMask = 0xF;
}
static void WINAPI my_GetNativeSystemInfo(LPSYSTEM_INFO si)
{
    real_GetNativeSystemInfo(si);
    if (si->dwNumberOfProcessors > 4) si->dwNumberOfProcessors = 4;
    si->dwActiveProcessorMask = 0xF;
}

static void patch_iat(HMODULE mod, const char *fn, void *hook, void **orig)
{
    BYTE *base = (BYTE *)mod;
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)base;
    IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)(base + dos->e_lfanew);
    DWORD rva = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if (!rva) return;
    IMAGE_IMPORT_DESCRIPTOR *imp = (IMAGE_IMPORT_DESCRIPTOR *)(base + rva);
    for (; imp->Name; imp++) {
        if (lstrcmpiA((char *)(base + imp->Name), "kernel32.dll")) continue;
        IMAGE_THUNK_DATA *orig_t = (IMAGE_THUNK_DATA *)(base + imp->OriginalFirstThunk);
        IMAGE_THUNK_DATA *t = (IMAGE_THUNK_DATA *)(base + imp->FirstThunk);
        for (; orig_t->u1.AddressOfData; orig_t++, t++) {
            if (orig_t->u1.Ordinal & IMAGE_ORDINAL_FLAG) continue;
            IMAGE_IMPORT_BY_NAME *n = (IMAGE_IMPORT_BY_NAME *)(base + orig_t->u1.AddressOfData);
            if (lstrcmpA((char *)n->Name, fn)) continue;
            DWORD old;
            if (VirtualProtect(&t->u1.Function, sizeof(void *), PAGE_READWRITE, &old)) {
                *orig = (void *)t->u1.Function;
                t->u1.Function = (DWORD_PTR)hook;
                VirtualProtect(&t->u1.Function, sizeof(void *), old, &old);
            }
        }
    }
}

BOOL WINAPI DllMain(HINSTANCE h, DWORD reason, LPVOID r)
{
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(h);
        SetProcessAffinityMask(GetCurrentProcess(), 0xF);
        patch_iat(GetModuleHandleA(NULL), "GetSystemInfo", (void *)my_GetSystemInfo, (void **)&real_GetSystemInfo);
        patch_iat(GetModuleHandleA(NULL), "GetNativeSystemInfo", (void *)my_GetNativeSystemInfo, (void **)&real_GetNativeSystemInfo);
    }
    return TRUE;
}
