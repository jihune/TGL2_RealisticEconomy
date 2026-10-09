#include "re_guard.h"

#include <stddef.h>

re_guard_blocked_fn re_guard_blocked;
void *re_guard_buy_trampoline, *re_guard_futures_trampoline, *re_guard_sell_step, *re_guard_sell_skip;
int re_guard_verdict;

/* The lock runs until the farthest month end already passed has been passed again. Beyond the cap it is cut to the
 * cap, and the record is lowered with it, so that loading the same save again gives the same lock and not a longer
 * one. What lay beyond the cap was seen and stays seen: with the stream moved, that future no longer comes. The move
 * is by `generation` draws from the position stored in the save, the same for every load of this generation, so
 * loading again does not draw a new future; only another rollback beyond the cap starts a new generation. Saves from
 * before `shift_until` keep getting the move, because their own stored position still leads to the future that was
 * seen. */
void re_guard_on_load(re_guard_record *rec, long long now, long long cap_ticks, int shift_on, int lock_line, re_guard_decision *out)
{
    out->unlock_tick = 0;
    out->capped = 0;
    out->draws = shift_on && rec->generation > 0 && now <= rec->shift_until ? rec->generation : 0;
    long long ahead = rec->farthest + 1 - now;
    /* The hour lies in the last rollback lock, so the save may have been written under it; the lock's line in the
     * save says that it was. The game keeps five autosaves in turn: the one written at this hour before the rollback
     * is still there, has no such line, and is as good as it was. */
    int under_lock = rec->lock_from <= now && now < rec->lock_until && lock_line != 0;
    out->month_end_pending = ahead == 1 && !under_lock;
    if (ahead <= 0)
        return;
    if (cap_ticks > 0 && ahead > cap_ticks) {
        if (shift_on) {
            if (rec->farthest > rec->shift_until)
                rec->shift_until = rec->farthest;
            rec->generation++;
            out->draws = rec->generation;
        }
        rec->farthest = now + cap_ticks - 1;
        out->capped = 1;
    }
    out->unlock_tick = rec->farthest + 1;
    if (ahead > 1) { /* a rollback; the one hour before a month end that is about to run again is none */
        rec->lock_from = now;
        rec->lock_until = out->unlock_tick;
    }
}

void re_guard_on_month_end(re_guard_record *rec, long long now)
{
    if (now > rec->farthest)
        rec->farthest = now;
}

__attribute__((force_align_arg_pointer)) void re_guard_enter(int trade)
{
    re_guard_blocked_fn fn = re_guard_blocked;
    re_guard_verdict = fn != NULL && fn(trade);
}

/* Every register is put back before the game's code runs. `go_on` continues the trade, `refuse` leaves the way the
 * game's own code would without trading. The game's logic runs on one thread, so one verdict variable is enough. */
#define GUARD_HOOK(name, trade, go_on, refuse)                                                                         \
    ".globl _" name "\n"                                                                                               \
    "_" name ":\n"                                                                                                     \
    "  pushad\n"                                                                                                       \
    "  sub esp, 128\n"                                                                                                 \
    "  movdqu [esp], xmm0\n"                                                                                           \
    "  movdqu [esp + 16], xmm1\n"                                                                                      \
    "  movdqu [esp + 32], xmm2\n"                                                                                      \
    "  movdqu [esp + 48], xmm3\n"                                                                                      \
    "  movdqu [esp + 64], xmm4\n"                                                                                      \
    "  movdqu [esp + 80], xmm5\n"                                                                                      \
    "  movdqu [esp + 96], xmm6\n"                                                                                      \
    "  movdqu [esp + 112], xmm7\n"                                                                                     \
    "  push " trade "\n"                                                                                               \
    "  call _re_guard_enter\n"                                                                                         \
    "  add esp, 4\n"                                                                                                   \
    "  movdqu xmm0, [esp]\n"                                                                                           \
    "  movdqu xmm1, [esp + 16]\n"                                                                                      \
    "  movdqu xmm2, [esp + 32]\n"                                                                                      \
    "  movdqu xmm3, [esp + 48]\n"                                                                                      \
    "  movdqu xmm4, [esp + 64]\n"                                                                                      \
    "  movdqu xmm5, [esp + 80]\n"                                                                                      \
    "  movdqu xmm6, [esp + 96]\n"                                                                                      \
    "  movdqu xmm7, [esp + 112]\n"                                                                                     \
    "  add esp, 128\n"                                                                                                 \
    "  popad\n"                                                                                                        \
    "  cmp dword ptr [_re_guard_verdict], 0\n"                                                                         \
    "  jne 1f\n"                                                                                                       \
    "  jmp dword ptr [_" go_on "]\n"                                                                                   \
    "1:\n" refuse

/* Buying and opening a futures trade are refused at the function's entry: return to the caller and take the stack
 * arguments along. Selling is refused at a call inside the handler: drop that call's return address and go on where
 * the handler goes when there is nothing to sell. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        GUARD_HOOK("re_guard_buy_hook", "0", "re_guard_buy_trampoline", "  ret 8\n")
        GUARD_HOOK("re_guard_sell_hook", "1", "re_guard_sell_step", "  add esp, 4\n  jmp dword ptr [_re_guard_sell_skip]\n")
        GUARD_HOOK("re_guard_futures_hook", "2", "re_guard_futures_trampoline", "  ret 16\n")
        ".att_syntax prefix\n");
