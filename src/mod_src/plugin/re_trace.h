/* Observation-only detour for the game's central cash function (TGL2.exe 0x00542920, PFM::ChangeMoney).
 *
 * Convention of the original: ecx = PFM, six stack words (int64 amount in cents, finance tag, three more), `ret 0x18`.
 * The detour saves every general register and xmm0-xmm7, hands the caller's stack to the sink, restores everything
 * and continues in the original. It changes nothing the game can see. */
#ifndef RE_TRACE_H
#define RE_TRACE_H

/* stack[0] = return address into the caller, stack[1..6] = the six argument words */
typedef void (*re_trace_money_fn)(void *self, const unsigned *stack);

extern void *re_trace_money_trampoline; /* filled by re_detour */
extern re_trace_money_fn re_trace_money_sink; /* NULL = observe nothing */

void re_trace_money_hook(void); /* entry for the detour; not callable from C */

#endif
