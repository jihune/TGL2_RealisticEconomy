#include "re_loan.h"

#include <stddef.h>

void *re_loan_add_debt;
re_loan_created_fn re_loan_created;

__attribute__((force_align_arg_pointer)) void re_loan_enter(unsigned *record, void *debt_manager, unsigned return_address)
{
    re_loan_created_fn fn = re_loan_created;
    if (fn != NULL)
        fn(record, debt_manager, return_address);
}

/* Every register is put back, the xmm ones included: the game is built with whole-program optimisation, so a caller
 * may know which registers this particular callee leaves alone. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_loan_call_hook\n"
        "_re_loan_call_hook:\n"
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
        "  push dword ptr [esp + 160]\n" /* 128 (xmm) + 32 (pushad): the return address */
        "  push dword ptr [esp + 156]\n" /* ecx as pushad saved it (128 + 24), one push further down */
        "  lea eax, [esp + 172]\n"       /* the record: return address + 4, two pushes further down */
        "  push eax\n"
        "  call _re_loan_enter\n"
        "  add esp, 12\n"
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
        "  jmp dword ptr [_re_loan_add_debt]\n"
        ".att_syntax prefix\n");
