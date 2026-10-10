/* Load guard: after loading a save from before a point the playthrough has already visited, the player's stock and
 * futures trades are refused until the game is back at that point. What was seen of the future is then useless.
 *
 * Two parts. The arithmetic (pure, no game access) decides at every load how long the lock lasts and whether the
 * stock-price random stream is moved. The three hooks ask `re_guard_blocked` before the game's code trades:
 *   0x00535120  buy shares            the function's entry: ecx = stock market, two stack arguments (ret 8).
 *                                     Also the monthly auto transfer.
 *   0x007babf9  sell shares           the first call of the selling branch in the stock window's sell handler.
 *                                     A refused sale leaves through the handler's own way out for "nothing to sell".
 *   0x0040ccf0  open a futures trade  the function's entry: ecx = derivatives, four stack arguments (ret 0x10)
 * None of them returns a value, and nothing has changed yet at any of the three points (card b4).
 */
#ifndef RE_GUARD_H
#define RE_GUARD_H

enum { RE_TRADE_BUY, RE_TRADE_SELL, RE_TRADE_FUTURES };

/* What is remembered per playthrough. Ticks are the game's hour counter since the start of the playthrough. The
 * month-end routine runs with the counter at the month's last hour, and the counter moves on right after it: a save
 * that carries that hour was made before the routine ran. */
typedef struct {
    long long farthest;    /* hour of the last month end this line of play has passed; -1 = none */
    long long shift_until; /* saves up to this hour get the stream moved: their future was seen before a rollback */
    int generation;        /* how many draws the stream is moved by; grows with every rollback beyond the cap */
    long long lock_from, lock_until; /* the hours [from, until) of the last lock a rollback brought; -1, -1 = none */
} re_guard_record;

typedef struct {
    long long unlock_tick; /* trades are refused while the hour counter is below this; 0 = no lock */
    int draws;             /* draws to take from the stock-price stream right now */
    int capped;            /* the lock was cut to the cap */
    int month_end_pending; /* the save is from the very hour of the farthest month end: that month end runs again at
                              once, with the standing orders the save has. The game's end-of-month autosave is such a
                              save: it is written in the month's last hour, before the routine. Not set for a save
                              written under the last rollback lock: it may carry orders arranged with the month
                              end's prices already seen. */
} re_guard_decision;

/* A load at hour `now`. `cap_ticks` 0 = no cap. `lock_line`: the loaded save carries the summary line of a running
 * lock (1), does not (0), or that cannot be told (-1, taken as 1 inside the last lock). Updates the record. */
void re_guard_on_load(re_guard_record *rec, long long now, long long cap_ticks, int shift_on, int lock_line, re_guard_decision *out);
/* The month end at hour `now` has been passed. */
void re_guard_on_month_end(re_guard_record *rec, long long now);

typedef int (*re_guard_blocked_fn)(int trade); /* 1 = refuse this trade */
extern re_guard_blocked_fn re_guard_blocked;
extern void *re_guard_buy_trampoline, *re_guard_futures_trampoline; /* filled by re_detour */
extern void *re_guard_sell_step, *re_guard_sell_skip; /* the call's own target, and where a refused sale continues */

void re_guard_buy_hook(void); /* entries for the detours and the redirected call; not callable from C */
void re_guard_sell_hook(void);
void re_guard_futures_hook(void);

/* What a month end draws (note m37 Q4). The game's month-end routine calls the economy's month (growth, a crash),
 * then the industries' cycles and the interest rates, then the month of the listed companies; the first and the last
 * of these calls take ecx only:
 *   0x00695272  call 0x004d37c0   the economy's month
 *   0x00695297  call 0x00533340   the listed companies' month
 * `re_guard_month_draws` is told before the first runs, with every register put back for it, and after the second
 * has returned, with every register as that function left it.
 * The properties for sale of a new month and the month's offers for the household's own properties are drawn when
 * the game's month-start routine calls the property market's month start, again with ecx only:
 *   0x0069558a  call 0x0054a730
 * The callback is told before and after that call in the same way.
 * Inside the property market's month start the game walks its sites twice, for the list for rent (0x0054ded0) and for
 * the list for sale (0x0054c9c0), and draws for one site after the other; a search of the character makes a new site,
 * so with one seed for the whole walk a search would still give other lists. Each walk starts a site with the same
 * call, and `re_guard_site_draws` is told there with the site's map node:
 *   0x0054e058  call 0x0080e290   list for rent, node in edi
 *   0x0054cb08  call 0x0080e290   list for sale, node in esi
 * After the two walks comes 0x0054a787 call 0x0054f010, and after that the offers for the properties the household
 * sells or lets: `re_guard_month_draws` is told RE_DRAWS_PROPERTY_OFFERS before that call. */
enum { RE_DRAWS_MONTH_END, RE_DRAWS_MONTH_END_DONE, RE_DRAWS_PROPERTY, RE_DRAWS_PROPERTY_DONE, RE_DRAWS_PROPERTY_OFFERS };
typedef void (*re_guard_month_fn)(int moment);
typedef void (*re_guard_site_fn)(int rent, const unsigned char *node);
extern re_guard_month_fn re_guard_month_draws;
extern re_guard_site_fn re_guard_site_draws;
/* the redirected calls' own targets */
extern void *re_guard_economy_month, *re_guard_stocks_month, *re_guard_property_month, *re_guard_site_data, *re_guard_own_values;

void re_guard_economy_hook(void); /* for the redirected calls; not callable from C */
void re_guard_stocks_hook(void);
void re_guard_property_hook(void);
void re_guard_sale_site_hook(void);
void re_guard_rent_site_hook(void);
void re_guard_offers_hook(void);

#endif
