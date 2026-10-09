#include "re_getter.h"

#include <string.h>

void *re_getter_trampoline;
re_getter_policy_fn re_getter_policy;

/* MSVC 14.x std::string, 32-bit: 16 bytes inline buffer or pointer, then size, then capacity. */
typedef struct {
    char buf[16];
    unsigned size;
    unsigned capacity;
} msvc_string;

/* 0 = not an attribute we handle. Only reads the caller's argument. */
__attribute__((force_align_arg_pointer)) int re_getter_classify(int id, const msvc_string *name)
{
    (void)id;
    if (name->capacity != 15 || name->size != 11) /* both names are 11 characters and stored inline */
        return 0;
    if (memcmp(name->buf, "interestXer", 11) == 0)
        return RE_ATTR_XER;
    if (memcmp(name->buf, "interestMod", 11) == 0)
        return RE_ATTR_MOD;
    return 0;
}

__attribute__((force_align_arg_pointer)) unsigned re_getter_adjust(int id, int attr, unsigned original_bits)
{
    re_getter_policy_fn policy = re_getter_policy;
    return policy ? policy(id, attr, original_bits) : original_bits;
}

/* Stack on entry: [esp] return address, [esp+4 .. esp+27] the std::string argument.
 *
 * Fast path (any other object): eax is the only register touched; it is dead at the original's entry
 * (the original's first use of eax is a write).
 *
 * Slow path: the original is called with a bitwise copy of the string. The original destroys that copy,
 * the game's caller pops its own 24 bytes as it always does. Both attribute names are stored inline, so the
 * copy owns no heap memory and nothing is freed twice or leaked. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_getter_hook\n"
        "_re_getter_hook:\n"
        "  mov eax, [ecx + 0xc]\n"
        "  cmp eax, 71111\n"
        "  je 1f\n"
        "  cmp eax, 71116\n"
        "  je 1f\n"
        "  jmp dword ptr [_re_getter_trampoline]\n"
        "1:\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  mov esi, ecx\n" /* this */
        "  mov ebx, eax\n" /* object id */
        "  push edx\n"
        "  lea edx, [esp + 20]\n" /* the string: 4 (edx) + 12 (saved registers) + 4 (return address) */
        "  push edx\n"
        "  push eax\n"
        "  call _re_getter_classify\n"
        "  add esp, 8\n"
        "  pop edx\n"
        "  test eax, eax\n"
        "  jnz 2f\n"
        "  mov ecx, esi\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  jmp dword ptr [_re_getter_trampoline]\n"
        "2:\n"
        "  push eax\n" /* attribute code, kept across the call */
        "  sub esp, 24\n"
        "  mov edi, esp\n"
        "  push esi\n"
        "  lea esi, [esp + 48]\n" /* 4 (esi) + 24 (copy) + 4 (code) + 12 (saved registers) + 4 (return address) */
        "  mov ecx, 6\n"
        "  rep movsd\n"
        "  pop esi\n"
        "  mov ecx, esi\n"
        "  call dword ptr [_re_getter_trampoline]\n"
        "  add esp, 24\n"
        "  pop ecx\n" /* attribute code */
        "  movd eax, xmm0\n"
        "  push eax\n"
        "  push ecx\n"
        "  push ebx\n"
        "  call _re_getter_adjust\n"
        "  add esp, 12\n"
        "  movd xmm0, eax\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  ret\n"
        ".att_syntax prefix\n");
