/* Detour for the game's "float attribute of the stats section" getter (TGL2.exe 0x00807530).
 *
 * Convention of the original (analysis/notes/m2_stake_payout_credit.md, Q6): ecx = object, one MSVC std::string
 * by value on the stack (24 bytes), the callee destroys that string, plain `ret`, result in xmm0.
 *
 * The detour looks only at the loan products 71111 (personal loan) and 71116 (mortgage) and at the attributes
 * interestXer and interestMod. Everything else jumps straight to the original.
 */
#ifndef RE_GETTER_H
#define RE_GETTER_H

#define RE_ATTR_XER 1
#define RE_ATTR_MOD 2

#define RE_ID_PERSONAL_LOAN 71111
#define RE_ID_MORTGAGE 71116

/* Receives the float the game read, as raw bits, and returns the bits to hand back. */
typedef unsigned (*re_getter_policy_fn)(int id, int attr, unsigned original_bits);

extern void *re_getter_trampoline;          /* filled by re_detour5 */
extern re_getter_policy_fn re_getter_policy; /* NULL = hand back the original */

void re_getter_hook(void); /* entry for the detour; not callable from C */

#endif
