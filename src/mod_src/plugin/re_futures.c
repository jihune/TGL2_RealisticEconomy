#include "re_futures.h"

#include <math.h>
#include <stddef.h>

#include "re_asm.h"

/* The game's month end for the rates (0x004d5300), in its order: for every industry the rate gets its policy term,
 * a pull of 0.04 x rel toward the overall price level (rel from the indices before this month's change, and
 * 1 + |rate| / 0.04 times as strong while the rate still points away from that level), and a tenth of the gap
 * between growth and last month's overall rate. The overall rate is the weighted sum of the new rates. Then every
 * index grows by a twelfth of its rate. The game's two normal draws have mean 0 and are left out. */
void re_futures_step(re_fut_economy *e)
{
    double drift = 0.1 * (e->growth - e->overall_rate), overall = 0.0;
    for (int i = 0; i < e->count; i++) {
        re_fut_industry *x = &e->industry[i];
        double rate = x->rate + x->policy;
        double rel = (e->overall_index - x->index) / (e->overall_index + x->index);
        double k = (rel < 0.0 && rate > 0.0) || (rel > 0.0 && rate < 0.0) ? 1.0 + fabs(rate) / 0.04 : 1.0;
        x->rate = rate + rel * 0.04 * k + drift;
        overall += x->weight * x->rate;
    }
    for (int i = 0; i < e->count; i++)
        e->industry[i].index *= 1.0 + e->industry[i].rate / 12.0;
    e->overall_rate = overall;
    e->overall_index *= 1.0 + overall / 12.0;
}

double re_futures_level(const re_fut_economy *e, const re_fut_tag *tags, int count)
{
    double level = 0.0, shares = 0.0;
    for (int t = 0; t < count; t++) {
        shares += tags[t].share;
        for (int i = 0; i < e->count; i++)
            if (e->industry[i].id == tags[t].id)
                level += tags[t].share * e->industry[i].index;
    }
    return shares < 1.0 ? level + (1.0 - shares) * e->overall_index : level;
}

double re_futures_expected_rate(const re_fut_economy *now, const re_fut_tag *tags, int count, int months)
{
    re_fut_economy e = *now;
    double start = re_futures_level(&e, tags, count);
    if (months <= 0 || !(start > 0.0))
        return 0.0;
    for (int m = 0; m < months; m++)
        re_futures_step(&e);
    return (re_futures_level(&e, tags, count) / start - 1.0) * 12.0 / (double)months;
}

re_futures_listed_fn re_futures_listed;
void *re_futures_list;
re_futures_value_fn re_futures_value;
void *re_futures_copy;

__attribute__((force_align_arg_pointer)) void re_futures_list_enter(void *economy, const unsigned char *first, const unsigned char *end)
{
    re_futures_listed_fn fn = re_futures_listed;
    if (fn != NULL)
        fn(economy, first, end);
}

__attribute__((force_align_arg_pointer)) void re_futures_after_value(unsigned char *string, const unsigned char *window)
{
    re_futures_value_fn fn = re_futures_value;
    if (fn != NULL)
        fn(string, window);
}

/* re_futures_list_hook: in place of the game's function, ecx = economy, the list's three words on the stack. The
 * callback sees the list; then the game's function runs with every register and the stack as the caller left them.
 *
 * re_futures_value_hook: in place of the string copy, ecx = the new string, one stack word (the source) that the
 * callee removes, edi = the window. The game's copy runs first; then the callback gets the new string and the
 * window; eax is what the copy returned and every other register is as the copy left it. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_futures_list_hook\n"
        "_re_futures_list_hook:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  pushad\n"
        SAVE_XMM
        "  push dword ptr [ebp + 12]\n"
        "  push dword ptr [ebp + 8]\n"
        "  push ecx\n"
        "  call _re_futures_list_enter\n"
        "  add esp, 12\n"
        RESTORE_XMM
        "  popad\n"
        "  pop ebp\n"
        "  jmp dword ptr [_re_futures_list]\n"
        ".globl _re_futures_value_hook\n"
        "_re_futures_value_hook:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ecx\n" /* [ebp - 4] = the new string */
        "  push dword ptr [ebp + 8]\n"
        "  call dword ptr [_re_futures_copy]\n"
        "  push eax\n"
        "  pushad\n"
        SAVE_XMM
        "  push edi\n"
        "  push dword ptr [ebp - 4]\n"
        "  call _re_futures_after_value\n"
        "  add esp, 8\n"
        RESTORE_XMM
        "  popad\n"
        "  pop eax\n"
        "  mov esp, ebp\n"
        "  pop ebp\n"
        "  ret 4\n"
        ".att_syntax prefix\n");
