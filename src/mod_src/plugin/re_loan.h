/* The calls inside the game's routines that hand a new debt to the debt list: enrolment (TGL2.exe 0x004d89e3, a new
 * education loan) and the purchase of a home with an approved mortgage (0x0043af1e).
 *
 * Convention of the function they call (0x004ce2b0): ecx = the household's debt manager, the debt record by value on
 * the stack, removed by the callee. The hook shows the record to `re_loan_created` before the game stores it, then
 * continues in the game's function. Record: +4 the stored yearly rate as a float (0 = variable), +0xc the product
 * id, +0x14 the term in months, +0x18 the principal in cents (64-bit), +0x30 the months before repayment starts.
 */
#ifndef RE_LOAN_H
#define RE_LOAN_H

/* `return_address` is where the redirected call returns to, which tells the call sites apart. */
typedef void (*re_loan_created_fn)(unsigned *record, void *debt_manager, unsigned return_address);

extern void *re_loan_add_debt; /* the game's function; must be set before a call site is redirected */
extern re_loan_created_fn re_loan_created;

void re_loan_call_hook(void); /* the new call target; not callable from C */

#endif
