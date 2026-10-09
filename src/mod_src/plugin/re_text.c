#include "re_text.h"

#include <windows.h>
#include <string.h>

void *re_text_trampoline;
re_text_want_fn re_text_want;
re_text_edit_fn re_text_edit;
void *re_text_assign;

/* MSVC 14.x std::string, 32-bit */
typedef struct {
    union {
        char buf[16];
        char *ptr;
    } u;
    unsigned size;
    unsigned capacity;
} msvc_string;

typedef void *(__thiscall *assign_fn)(void *self, const char *text, unsigned len);

/* One entry per call in flight. A call is identified by the address of its return-address slot, which is unique
 * across threads, so the table needs no thread-local storage. */
typedef struct {
    volatile LONG slot; /* 0 = free */
    unsigned ret;
    msvc_string *out;
    int want;
    re_text_regs regs;
    re_text_values values;
} pending;

#define PENDING_MAX 32
static pending g_pending[PENDING_MAX];

static const char *str_data(const msvc_string *s)
{
    return s->capacity >= 16 ? s->u.ptr : s->u.buf;
}

/* The five pairs of the call: the name of pair i is the string at stack[8 + 12 * i], its value the one behind it. */
static void read_values(const unsigned *stack, re_text_values *out)
{
    out->count = 0;
    out->whole = 1;
    for (int i = 0; i < RE_TEXT_PAIRS; i++) {
        const msvc_string *name = (const msvc_string *)&stack[8 + 12 * i], *value = name + 1;
        if (name->size == 0)
            continue;
        if (name->size > name->capacity || value->size > value->capacity || name->size >= sizeof out->name[0] ||
            value->size >= sizeof out->value[0]) {
            out->whole = 0;
            continue;
        }
        memcpy(out->name[out->count], str_data(name), name->size);
        out->name[out->count][name->size] = 0;
        memcpy(out->value[out->count], str_data(value), value->size);
        out->value[out->count][value->size] = 0;
        out->count++;
    }
}

/* stack[0] = return address, stack[1] = result string, stack[2..7] = the key, stack[8..67] the ten strings behind
 * it; the eight words below stack[0] are the hook's `pushad`. Returns 1 to have the return hijacked. */
__attribute__((force_align_arg_pointer)) int re_text_enter(unsigned *stack)
{
    re_text_want_fn want_fn = re_text_want;
    if (want_fn == NULL || re_text_edit == NULL || re_text_assign == NULL)
        return 0;
    const msvc_string *key = (const msvc_string *)&stack[2];
    if (key->size == 0 || key->size > 64)
        return 0;
    const re_text_regs *regs = (const re_text_regs *)(stack - 8);
    int want = want_fn(str_data(key), key->size, stack[0], regs);
    if (!want)
        return 0;
    for (int i = 0; i < PENDING_MAX; i++) {
        if (InterlockedCompareExchange(&g_pending[i].slot, (LONG)(UINT_PTR)stack, 0) == 0) {
            g_pending[i].ret = stack[0];
            g_pending[i].out = (msvc_string *)(UINT_PTR)stack[1];
            g_pending[i].want = want;
            g_pending[i].regs = *regs;
            read_values(stack, &g_pending[i].values);
            return 1;
        }
    }
    return 0; /* table full: leave this text alone */
}

/* `after` is the stack pointer right after the original returned: the return-address slot + 4 + 0x10c. */
__attribute__((force_align_arg_pointer)) unsigned re_text_leave(unsigned after)
{
    LONG slot = (LONG)(after - 0x110);
    for (int i = 0; i < PENDING_MAX; i++) {
        if (g_pending[i].slot != slot)
            continue;
        unsigned ret = g_pending[i].ret;
        msvc_string *out = g_pending[i].out;
        int want = g_pending[i].want;
        re_text_regs regs = g_pending[i].regs;
        re_text_values values = g_pending[i].values;
        InterlockedExchange(&g_pending[i].slot, 0);
        char text[2048]; /* room for a tooltip of several paragraphs (the staff switch's: the game's 350 bytes and four of the mod's) */
        if (out->size < sizeof text - 96) {
            unsigned len = out->size;
            memcpy(text, str_data(out), len);
            unsigned changed = re_text_edit(want, text, len, sizeof text, &regs, &values);
            if (changed < sizeof text && (changed != len || memcmp(text, str_data(out), len) != 0))
                ((assign_fn)re_text_assign)(out, text, changed);
        }
        return ret;
    }
    return 0; /* cannot happen: every hijacked return has an entry */
}

void *re_text_row_trampoline;
re_text_row_fn re_text_row;

/* stack[0] = return address, [1] = layer, [2] = position, [3] = red flag, [4..9] = label, [10..15] = value */
__attribute__((force_align_arg_pointer)) void re_text_row_enter(unsigned *stack)
{
    re_text_row_fn row = re_text_row;
    if (row != NULL)
        row(stack[0], (const re_text_regs *)(stack - 8), &stack[4], &stack[10], &stack[3]);
}

__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_text_row_hook\n"
        "_re_text_row_hook:\n"
        "  pushad\n"
        "  sub esp, 128\n"
        "  movdqu [esp], xmm0\n"
        "  movdqu [esp + 16], xmm1\n"
        "  movdqu [esp + 32], xmm2\n"
        "  movdqu [esp + 48], xmm3\n"
        "  movdqu [esp + 64], xmm4\n"
        "  movdqu [esp + 80], xmm5\n"
        "  movdqu [esp + 96], xmm6\n"
        "  movdqu [esp + 112], xmm7\n"
        "  lea eax, [esp + 160]\n" /* the return-address slot */
        "  push eax\n"
        "  call _re_text_row_enter\n"
        "  add esp, 4\n"
        "  movdqu xmm0, [esp]\n"
        "  movdqu xmm1, [esp + 16]\n"
        "  movdqu xmm2, [esp + 32]\n"
        "  movdqu xmm3, [esp + 48]\n"
        "  movdqu xmm4, [esp + 64]\n"
        "  movdqu xmm5, [esp + 80]\n"
        "  movdqu xmm6, [esp + 96]\n"
        "  movdqu xmm7, [esp + 112]\n"
        "  add esp, 128\n"
        "  popad\n"
        "  jmp dword ptr [_re_text_row_trampoline]\n"
        ".att_syntax prefix\n");

__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_text_hook\n"
        "_re_text_hook:\n"
        "  pushad\n"
        "  sub esp, 128\n"
        "  movdqu [esp], xmm0\n"
        "  movdqu [esp + 16], xmm1\n"
        "  movdqu [esp + 32], xmm2\n"
        "  movdqu [esp + 48], xmm3\n"
        "  movdqu [esp + 64], xmm4\n"
        "  movdqu [esp + 80], xmm5\n"
        "  movdqu [esp + 96], xmm6\n"
        "  movdqu [esp + 112], xmm7\n"
        "  lea eax, [esp + 160]\n" /* 128 (xmm) + 32 (pushad): the return-address slot */
        "  push eax\n"
        "  call _re_text_enter\n"
        "  add esp, 4\n"
        "  test eax, eax\n"
        "  jz 1f\n"
        "  mov dword ptr [esp + 160], offset _re_text_return\n"
        "1:\n"
        "  movdqu xmm0, [esp]\n"
        "  movdqu xmm1, [esp + 16]\n"
        "  movdqu xmm2, [esp + 32]\n"
        "  movdqu xmm3, [esp + 48]\n"
        "  movdqu xmm4, [esp + 64]\n"
        "  movdqu xmm5, [esp + 80]\n"
        "  movdqu xmm6, [esp + 96]\n"
        "  movdqu xmm7, [esp + 112]\n"
        "  add esp, 128\n"
        "  popad\n"
        "  jmp dword ptr [_re_text_trampoline]\n"
        /* the original returns here for a wanted key; its arguments are already popped */
        "_re_text_return:\n"
        "  push eax\n" /* becomes the real return address */
        "  pushad\n"
        "  lea eax, [esp + 36]\n" /* the stack pointer as the original left it */
        "  push eax\n"
        "  call _re_text_leave\n"
        "  add esp, 4\n"
        "  mov [esp + 32], eax\n"
        "  popad\n"
        "  ret\n"
        ".att_syntax prefix\n");
