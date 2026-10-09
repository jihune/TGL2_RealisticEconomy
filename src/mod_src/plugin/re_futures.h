#ifndef RE_FUTURES_H
#define RE_FUTURES_H

/* Futures (REQUEST.md [30], note b12 "Q4" and "Q5"). A contract's quote assumes that the asset's inflation rate stays
 * where it is today until the expiry; the price it is settled at follows the rates as they come. The game moves
 * every industry's rate by a rule with two pulls and some noise, and all of its inputs are in the economy object.
 * This follows that rule with the noise at its mean and says what the rate is expected to average until the expiry:
 * above today's rate a purchase is ahead, below it a sale. Nothing of the game's rules changes. */

#define RE_FUT_INDUSTRIES 24
#define RE_FUT_TAGS 8
#define RE_FUT_OVERALL 1531 /* the key of the overall rate and the overall index in the economy's two maps */

typedef struct {
    int id;
    double rate;   /* yearly price change, 0.05 = 5% */
    double index;  /* price level */
    double weight; /* of the industry in the overall rate; the weights sum to 1 */
    double policy; /* what a policy adds to the rate every month, usually 0 */
} re_fut_industry;

typedef struct {
    int count; /* the industries; the overall entry is not among them */
    re_fut_industry industry[RE_FUT_INDUSTRIES];
    double overall_rate, overall_index;
    double growth; /* economic growth, what the overall rate is pulled to */
} re_fut_economy;

/* One industry of an asset and the share of the asset's price that follows it; what the shares leave of 1 follows
 * the overall index. */
typedef struct {
    int id;
    double share;
} re_fut_tag;

/* One month end as the game does it, every random draw at its mean. */
void re_futures_step(re_fut_economy *e);
/* The price level of an asset with these industries. */
double re_futures_level(const re_fut_economy *e, const re_fut_tag *tags, int count);
/* The yearly rate that, held for `months` at simple interest the way a quote is built, gives the price level the
 * rule leads to. */
double re_futures_expected_rate(const re_fut_economy *now, const re_fut_tag *tags, int count, int months);

/* The game's weighted rate of an industry list (ecx = economy; the list by value: first, end, room; ret 0xc): the
 * hook hands the list on before the game's function runs. */
typedef void (*re_futures_listed_fn)(void *economy, const unsigned char *first, const unsigned char *end);
extern re_futures_listed_fn re_futures_listed;
extern void *re_futures_list;    /* the game's function */
void re_futures_list_hook(void); /* target for re_patch_call; not callable from C */

/* The futures window copies the text of the rate ("4.30%") for its label (string copy: ecx = the new string, the
 * source on the stack; edi = the window): after the copy the callback gets the new string and the window. */
typedef void (*re_futures_value_fn)(unsigned char *string, const unsigned char *window);
extern re_futures_value_fn re_futures_value;
extern void *re_futures_copy;     /* the game's string copy */
void re_futures_value_hook(void); /* target for re_patch_call; not callable from C */

#endif
