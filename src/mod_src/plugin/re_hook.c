#include "re_hook.h"

#include <string.h>

int re_readable(const void *p, SIZE_T count)
{
    MEMORY_BASIC_INFORMATION mbi;
    const BYTE *at = (const BYTE *)p;
    const BYTE *end = at + count;
    if (p == NULL || end < at)
        return 0;
    while (at < end) {
        if (VirtualQuery(at, &mbi, sizeof mbi) != sizeof mbi)
            return 0;
        if (mbi.State != MEM_COMMIT || (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) || mbi.Protect == 0)
            return 0;
        at = (const BYTE *)mbi.BaseAddress + mbi.RegionSize;
    }
    return 1;
}

int re_bytes_equal(const BYTE *at, const BYTE *expected, SIZE_T count)
{
    return re_readable(at, count) && memcmp(at, expected, count) == 0;
}

static int write_code(BYTE *at, const BYTE *bytes, SIZE_T count)
{
    DWORD old;
    if (!VirtualProtect(at, count, PAGE_EXECUTE_READWRITE, &old))
        return 0;
    memcpy(at, bytes, count);
    VirtualProtect(at, count, old, &old);
    FlushInstructionCache(GetCurrentProcess(), at, count);
    return 1;
}

static void rel32(BYTE *out, const BYTE *insn_end, const void *target)
{
    LONG delta = (LONG)((const BYTE *)target - insn_end);
    memcpy(out, &delta, 4);
}

int re_patch_call(BYTE *site, const BYTE *expected_target, const void *new_target)
{
    BYTE expected[5] = {0xE8};
    BYTE patch[5] = {0xE8};
    rel32(expected + 1, site + 5, expected_target);
    if (!re_bytes_equal(site, expected, 5))
        return 0;
    rel32(patch + 1, site + 5, new_target);
    return write_code(site, patch, 5);
}

int re_patch_ptr(BYTE *at, const void *expected, const void *value)
{
    if (!re_bytes_equal(at, (const BYTE *)&expected, 4))
        return 0;
    return write_code(at, (const BYTE *)&value, 4);
}

int re_patch_bytes(BYTE *at, const BYTE *expected, const BYTE *replacement, SIZE_T count)
{
    return re_bytes_equal(at, expected, count) && write_code(at, replacement, count);
}

int re_detour(BYTE *fn, const BYTE *expected, SIZE_T count, const void *hook, void **trampoline)
{
    BYTE jump[12];
    BYTE *tramp;
    if (count < 5 || count > sizeof jump || !re_bytes_equal(fn, expected, count))
        return 0;
    tramp = (BYTE *)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (tramp == NULL)
        return 0;
    memcpy(tramp, expected, count);
    tramp[count] = 0xE9;
    rel32(tramp + count + 1, tramp + count + 5, fn + count);
    FlushInstructionCache(GetCurrentProcess(), tramp, 32);
    *trampoline = tramp; /* must be in place before the entry is redirected */
    memset(jump, 0x90, sizeof jump);
    jump[0] = 0xE9;
    rel32(jump + 1, fn + 5, hook);
    if (!write_code(fn, jump, count)) {
        *trampoline = NULL;
        VirtualFree(tramp, 0, MEM_RELEASE);
        return 0;
    }
    return 1;
}

int re_detour5(BYTE *fn, const BYTE expected5[5], const void *hook, void **trampoline)
{
    return re_detour(fn, expected5, 5, hook, trampoline);
}
