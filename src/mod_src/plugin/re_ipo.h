/* Listing a public company so that the founder is paid: the public buys the founder's own shares at the listing
 * price, the founder keeps a fraction, and no new money goes into the company (notes m11 and m12).
 *
 * The game's listing function FUN_005362a0 keeps doing the work. Nine of its operands are changed in place by
 * re_main, and two of its calls are redirected to the hooks below:
 *   0x00536783  call 0x53cb50   the twelve price steps before the listing. Afterwards `re_ipo_steps_done` may
 *                               change the price. Runs for the IPO window's preview as well. The steps draw from
 *                               the game's random streams: `re_ipo_steps_draws` is called with 0 just before them
 *                               and with 1 just after, before `re_ipo_steps_done` (note m37 Q4).
 *   0x00536e2c  call 0x542920   the listing fee. Just before it `re_ipo_before_fee` is told, and can pay the founder.
 * Both callbacks get the frame pointer of FUN_005362a0, whose locals hold the share counts and the company.
 */
#ifndef RE_IPO_H
#define RE_IPO_H

#include <windows.h>

typedef void (*re_ipo_steps_fn)(const BYTE *frame, long long *price, long long offer, long long earnings_per_share);
typedef void (*re_ipo_fee_fn)(const BYTE *frame, void *finance);
typedef void (*re_ipo_draws_fn)(int drawn);
extern re_ipo_draws_fn re_ipo_steps_draws;
extern re_ipo_steps_fn re_ipo_steps_done;
extern re_ipo_fee_fn re_ipo_before_fee;
extern void *re_ipo_steps, *re_ipo_change_money; /* the two calls' own targets */

void re_ipo_steps_hook(void); /* targets for the redirected calls; not callable from C */
void re_ipo_fee_hook(void);

/* The listing price: never below the offer price while the business earns anything, when the floor is on. */
long long re_ipo_floor(long long price, long long offer, long long earnings_per_share, int floor_on);
/* What the public pays for the shares the founder does not keep, in cents; 0 when the numbers make no sense. */
long long re_ipo_cash(long long all_shares, long long founder_shares, long long price);

/* The quality letter of the IPO window. The game takes m = price / equity per share of the previewed company and
 * compares m - 1 with 0.15, 0.1, 0.05, 0, -0.05, -0.1: above the first AAA, then AA, A, B, C, D, else E. With no new
 * money in the company m is the listing price over the offer price, 2 and more for a business that earns 14% of its
 * value, so every letter would be AAA. Five of the limits are constants the instructions read and can be pointed
 * elsewhere; the limit of B is a zeroed register. So the number that is subtracted becomes the multiple B starts
 * above, and the other five are kept relative to it. */
typedef struct {
    float base;  /* subtracted from m */
    double aaa;  /* the game compares this one in double precision */
    float aa, a; /* B: above 0 */
    float c, d;
} re_ipo_grade;
enum { RE_IPO_LETTERS = 6 }; /* letters with a lower limit: AAA, AA, A, B, C, D */
/* above[i] = the multiple letter i starts above. Returns 0 and fills nothing unless they are positive and descending. */
int re_ipo_grade_limits(const double above[RE_IPO_LETTERS], re_ipo_grade *out);
/* The letter the game's comparisons give with those limits: 0 = AAA .. 6 = E. */
int re_ipo_grade_index(const re_ipo_grade *g, float multiple);

#endif
