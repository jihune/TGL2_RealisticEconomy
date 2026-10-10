#include "re_ipo.h"

#include <stddef.h>

re_ipo_draws_fn re_ipo_steps_draws;
re_ipo_steps_fn re_ipo_steps_done;
re_ipo_fee_fn re_ipo_before_fee;
void *re_ipo_steps, *re_ipo_change_money;

long long re_ipo_floor(long long price, long long offer, long long earnings_per_share, int floor_on)
{
    return floor_on && earnings_per_share > 0 && price < offer ? offer : price;
}

long long re_ipo_cash(long long all_shares, long long founder_shares, long long price)
{
    return founder_shares >= 0 && all_shares > founder_shares && price > 0 ? (all_shares - founder_shares) * price : 0;
}

int re_ipo_grade_limits(const double above[RE_IPO_LETTERS], re_ipo_grade *out)
{
    for (int i = 0; i < RE_IPO_LETTERS; i++)
        if (!(above[i] > 0.0) || (i > 0 && !(above[i] < above[i - 1])))
            return 0;
    out->base = (float)above[3];
    out->aaa = above[0] - above[3];
    out->aa = (float)(above[1] - above[3]);
    out->a = (float)(above[2] - above[3]);
    out->c = (float)(above[4] - above[3]);
    out->d = (float)(above[5] - above[3]);
    return 1;
}

int re_ipo_grade_index(const re_ipo_grade *g, float multiple)
{
    float x = multiple - g->base;
    return (double)x > g->aaa ? 0 : x > g->aa ? 1 : x > g->a ? 2 : x > 0.0f ? 3 : x > g->c ? 4 : x > g->d ? 5 : 6;
}

__attribute__((force_align_arg_pointer)) void re_ipo_before_steps(void)
{
    re_ipo_draws_fn draws = re_ipo_steps_draws;
    if (draws != NULL)
        draws(0);
}

__attribute__((force_align_arg_pointer)) void re_ipo_after_steps(const BYTE *frame, long long *price, long long offer,
                                                                 long long earnings_per_share)
{
    re_ipo_draws_fn draws = re_ipo_steps_draws;
    if (draws != NULL)
        draws(1);
    re_ipo_steps_fn fn = re_ipo_steps_done;
    if (fn != NULL)
        fn(frame, price, offer, earnings_per_share);
}

__attribute__((force_align_arg_pointer)) void re_ipo_enter_fee(const BYTE *frame, void *finance)
{
    re_ipo_fee_fn fn = re_ipo_before_fee;
    if (fn != NULL)
        fn(frame, finance);
}

#define SAVE_XMM                                                                                                       \
    "  sub esp, 128\n"                                                                                                 \
    "  movdqu [esp], xmm0\n"                                                                                           \
    "  movdqu [esp + 16], xmm1\n"                                                                                      \
    "  movdqu [esp + 32], xmm2\n"                                                                                      \
    "  movdqu [esp + 48], xmm3\n"                                                                                      \
    "  movdqu [esp + 64], xmm4\n"                                                                                      \
    "  movdqu [esp + 80], xmm5\n"                                                                                      \
    "  movdqu [esp + 96], xmm6\n"                                                                                      \
    "  movdqu [esp + 112], xmm7\n"
#define RESTORE_XMM                                                                                                    \
    "  movdqu xmm0, [esp]\n"                                                                                           \
    "  movdqu xmm1, [esp + 16]\n"                                                                                      \
    "  movdqu xmm2, [esp + 32]\n"                                                                                      \
    "  movdqu xmm3, [esp + 48]\n"                                                                                      \
    "  movdqu xmm4, [esp + 64]\n"                                                                                      \
    "  movdqu xmm5, [esp + 80]\n"                                                                                      \
    "  movdqu xmm6, [esp + 96]\n"                                                                                      \
    "  movdqu xmm7, [esp + 112]\n"                                                                                     \
    "  add esp, 128\n"

/* The price steps take the address of the price in ecx and five stack words that the caller removes: the offer price
 * and the earnings a share (int64 each) and a context. First the callback before the steps, with every register put
 * back for the game's function; that runs with the same arguments; then the callback gets the caller's frame, the
 * price and the two amounts. Every register is as the game's function left it when this returns. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_ipo_steps_hook\n"
        "_re_ipo_steps_hook:\n"
        "  push ebp\n" /* [ebp] = the frame pointer of the listing function */
        "  mov ebp, esp\n"
        "  push ecx\n" /* [ebp - 4] = address of the price */
        "  pushad\n"
        SAVE_XMM
        "  call _re_ipo_before_steps\n"
        RESTORE_XMM
        "  popad\n"
        "  push dword ptr [ebp + 24]\n"
        "  push dword ptr [ebp + 20]\n"
        "  push dword ptr [ebp + 16]\n"
        "  push dword ptr [ebp + 12]\n"
        "  push dword ptr [ebp + 8]\n"
        "  call dword ptr [_re_ipo_steps]\n"
        "  add esp, 20\n"
        "  pushad\n"
        SAVE_XMM
        "  push dword ptr [ebp + 20]\n" /* earnings a share, high and low */
        "  push dword ptr [ebp + 16]\n"
        "  push dword ptr [ebp + 12]\n" /* offer price, high and low */
        "  push dword ptr [ebp + 8]\n"
        "  push dword ptr [ebp - 4]\n"
        "  push dword ptr [ebp]\n"
        "  call _re_ipo_after_steps\n"
        "  add esp, 24\n"
        RESTORE_XMM
        "  popad\n"
        "  mov esp, ebp\n"
        "  pop ebp\n"
        "  ret\n"
        /* The fee: the callback first, with the caller's frame and the finance object the call has in ecx; then the
         * game's cash function with everything as it was. */
        ".globl _re_ipo_fee_hook\n"
        "_re_ipo_fee_hook:\n"
        "  pushad\n"
        SAVE_XMM
        "  push dword ptr [esp + 152]\n" /* ecx as pushad saved it: 128 (xmm) + 24 */
        "  push ebp\n"                   /* pushad leaves ebp alone: still the listing function's frame */
        "  call _re_ipo_enter_fee\n"
        "  add esp, 8\n"
        RESTORE_XMM
        "  popad\n"
        "  jmp dword ptr [_re_ipo_change_money]\n"
        ".att_syntax prefix\n");
