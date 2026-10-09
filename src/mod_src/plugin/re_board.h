/* A seat on the board for a large shareholder (note b11).
 *
 * An election seats the five best totals. The game computes a candidate's total in FUN_0053efa0 and already adds
 * twice the household's share of the company to it, which loses against an experienced candidate. Two calls of that
 * function are redirected to the hook below:
 *   0x0053e87c  the call that decides the election
 *   0x007b212a  the call that orders the candidates in the board tab, so that the tab shows what the election will do
 * The game's function runs first; then `re_board_scored` may replace the total. Nothing else of the score is
 * touched: the experience part, which the monthly company update reads through a third call, stays as it is.
 */
#ifndef RE_BOARD_H
#define RE_BOARD_H

#include <windows.h>

/* `return_address` is where the redirected call returns to in the game: it tells the two calls apart. */
typedef void (*re_board_scored_fn)(BYTE *company, BYTE *score, int person, unsigned return_address);
extern re_board_scored_fn re_board_scored;
extern void *re_board_score; /* the game's function: ecx = company, (score, person), ret 8 */

void re_board_score_hook(void); /* target for the redirected calls; not callable from C */

enum { RE_BOARD_SEATS = 5 };
/* Seats a holding guarantees: one for every `per_seat` of the company, at most the five seats of a board. */
int re_board_seats(float fraction, double per_seat);
/* Which of the household's nominees `person` is, counting from 0 in the order of the list; -1 when the person is
 * not a nominee or not of the household. Incumbents come first in that list, then nominations in the order made. */
int re_board_place(const int *nominees, int count, const int *household, int members, int person);
/* The total of the place-th guaranteed nominee: `top` for the first and one less for each further one. They must
 * differ: equal totals share one of the five places and would seat a sixth member. */
float re_board_total(float top, int place);
/* The game's ownership function called from C: ecx = stock market, the company id on the stack, result in xmm0. */
float re_board_fraction(void *ownership_fn, void *market, int company_id);

/* The game's destructor of a score (0x00414400: ecx = the score, plain ret). The score function constructs its
 * result at its entry, so a score that is asked for a second time has to be destroyed in between (note b13). */
extern void *re_board_score_free;
/* The total the game gives every nominee with nothing forced, in the order of `nominees`: its own function is
 * called once for each of them except `person`, whose total `own` the game has just computed, into a scratch score
 * that is destroyed again. Nothing of the game is changed by that (note b13). Returns 0 when the two functions are
 * not set. */
int re_board_totals(BYTE *company, const int *nominees, int count, int person, float own, float *totals);
/* Whose total is forced, and in which order: `person`'s number among them counting from 0, or -1 when the total
 * stays the game's own. With no more household nominees than `seats`, all of them, in the order of the list. With
 * more: first those who are elected anyway, by the totals with nothing forced - they keep their seats and use up
 * none of the guarantee - then up to `seats` of the others in the order of the list. `totals` NULL = not known:
 * the first `seats` of the list. */
int re_board_rank(const int *nominees, const float *totals, int count, const int *household, int members, int seats, int person);

/* The persons nominated for the board of `company` (a listed company, or the stocks window's copy of one), in the
 * order of the list. Returns how many, or -1 when the list cannot be read or does not fit in `cap`. */
int re_board_nominees(const BYTE *company, int *out, int cap);
/* The persons who sit on the board of `company` now. Same return. The list is empty during the month of an election. */
int re_board_members(const BYTE *company, int *out, int cap);
/* The household's persons as the company's character manager has them. Same return. */
int re_board_household(const BYTE *company, int *out, int cap);

#endif
