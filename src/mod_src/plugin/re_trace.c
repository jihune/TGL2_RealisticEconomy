#include "re_trace.h"

void *re_trace_money_trampoline;
re_trace_money_fn re_trace_money_sink;

__attribute__((force_align_arg_pointer)) void re_trace_money_dispatch(void *self, const unsigned *stack)
{
    re_trace_money_fn sink = re_trace_money_sink;
    if (sink)
        sink(self, stack);
}

/* After pushad the saved registers sit at [esp]: edi, esi, ebp, esp, ebx, edx, ecx (+24), eax (+28).
 * Flags are not preserved; they are not live at a function entry. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_trace_money_hook\n"
        "_re_trace_money_hook:\n"
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
        "  lea eax, [esp + 160]\n" /* 128 (xmm) + 32 (pushad): the stack as the caller left it */
        "  push eax\n"
        "  mov eax, [esp + 156]\n" /* 4 (push) + 128 + 24: the saved ecx */
        "  push eax\n"
        "  call _re_trace_money_dispatch\n"
        "  add esp, 8\n"
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
        "  jmp dword ptr [_re_trace_money_trampoline]\n"
        ".att_syntax prefix\n");
