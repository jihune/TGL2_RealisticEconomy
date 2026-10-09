#include "re_memory.h"

#include "re_asm.h"
#include "re_hook.h"

void *re_memory_end_trampoline;
re_memory_end_fn re_memory_ended;

int re_memory_release(BYTE *list, const void *table, unsigned slot)
{
    typedef void *(__thiscall *end_fn)(void *self, unsigned flags);
    if (!re_readable(list, 2 * sizeof(void *)))
        return -1;
    void ***first = *(void ****)list, ***end = *(void ****)(list + sizeof(void *));
    SIZE_T bytes = (SIZE_T)((BYTE *)end - (BYTE *)first);
    if (end < first || bytes % sizeof(void *) != 0 || (bytes != 0 && !re_readable(first, bytes)))
        return -1;
    int count = 0;
    for (void ***at = first; at != end; at++) {
        void **object = *at;
        if (re_readable(object, sizeof(void *)) && *object == table) {
            ((end_fn) * (void **)((BYTE *)table + slot))(object, 1);
            count++;
        }
    }
    *(void ****)(list + sizeof(void *)) = first;
    return count;
}

__attribute__((force_align_arg_pointer)) void re_memory_end(BYTE *loader)
{
    re_memory_end_fn ended = re_memory_ended;
    if (ended)
        ended(loader);
}

/* Entry of the loader's destructor: ecx = loader, nothing on the stack but the return address. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_memory_end_hook\n"
        "_re_memory_end_hook:\n"
        "  pushad\n"
        SAVE_XMM
        "  push ecx\n"
        "  call _re_memory_end\n"
        "  add esp, 4\n"
        RESTORE_XMM
        "  popad\n"
        "  jmp dword ptr [_re_memory_end_trampoline]\n"
        ".att_syntax prefix\n");
