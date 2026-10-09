/* What a listed company's own numbers say about its share, by the rules the game applies at every month end
 * (notes b3 and b12): the price moves a fifth of the way to a target made of equity and earnings, and a price under
 * three quarters of the equity a share brings an emergency measure, the second one within six months the end of the
 * company. Arithmetic and wording only; nothing here calls the game.
 */
#ifndef RE_STOCK_H
#define RE_STOCK_H

typedef struct {
    long long price, dividend, earnings, equity; /* cents a share; dividend and earnings per year */
    int research_month;  /* month + 12 x year of the last "research stock" action; 0 = never */
    int desperate_month; /* the same for the last emergency measure; far in the past when there was none */
} re_stock;

typedef struct {
    int has_per, has_pbr, has_yield; /* earnings, equity and price are positive where the ratio needs them */
    double per, pbr, roe, yield;
    long long target; /* cents: where the monthly step pulls the price; 0 when the equity is not positive */
    double gap;       /* (target - price) / price */
    int below_line;   /* the price is under 0.75 x equity, or one of the two is not positive */
    double to_line;   /* the fall, as a fraction of the price, that takes it there; only when not below */
    int months_since; /* since the last emergency measure */
    int has_change;   /* the caller knows the price of the month before (re_stock_view_of leaves this 0) */
    double change;    /* price / that price - 1 */
} re_stock_view;

#define RE_STOCK_LINE 0.75      /* price / equity under this at a month end: emergency measure */
#define RE_STOCK_GRACE_MONTHS 6 /* a second one within this many months: bankrupt, the shares are worth nothing */
#define RE_STOCK_NEAR 0.25      /* the line is named when a fall of this fraction or less reaches it */
#define RE_STOCK_HELD_NEAR 0.10 /* the month's warning about shares held names it only this close, without a measure before */

/* `company` is the game's company object or one of the copies its windows work on: the same layout. */
void re_stock_read(const unsigned char *company, re_stock *out);
void re_stock_view_of(const re_stock *s, int now_month, re_stock_view *v);

/* The lines for a hover text, joined with '\n', no trailing one. Always one line of ratios; with `has_change` a
 * line with the price against the month before; a line only near or under the line and after an emergency
 * measure; one more only when `target_text` (the price target as the game
 * writes amounts) is given, which the caller does for a company the player has had researched. With
 * `has_industries` a word follows the target when the sum of the industry terms, `industries`, is half a sign or
 * more either way. `red` wraps a warning in the game's colour marks. `lang` is a column of re_lang.h. Returns the
 * length, 0 when `cap` is too small. */
unsigned re_stock_text(const re_stock_view *v, int lang, const char *target_text, int has_industries, double industries, int red,
                       char *out, unsigned cap);

/* What goes behind the amount in one of the four money rows of a company's first tab: " (PBR 1.29)" behind the
 * equity a share, for example. `*red` becomes 1 when the row is a warning. Returns the length; 0 = nothing to add. */
enum { RE_STOCK_ROW_PRICE, RE_STOCK_ROW_EQUITY, RE_STOCK_ROW_EARNINGS, RE_STOCK_ROW_DIVIDEND };
unsigned re_stock_row(const re_stock_view *v, int row, int lang, const char *target_text, int *red, char *out, unsigned cap);

/* What goes behind a company's name in a row of the list (REQUEST.md [44]): the price against the month before,
 * for a researched company how far the fair price is from the price, and who keeps the research fresh:
 * " (+2.5%)", " (+17.9%, 적정 +32%, 구독)". `kept`: 0 nobody, 1 a subscription, 2 a seat on the board. Returns the
 * length; 0 = nothing to add (or `cap` too small). */
enum { RE_STOCK_KEPT_NOT, RE_STOCK_KEPT_SUBSCRIBED, RE_STOCK_KEPT_BOARD };
unsigned re_stock_item(const re_stock_view *v, int researched, int kept, int lang, char *out, unsigned cap);

/* The order of the company list, in place of four of the game's (REQUEST.md [43], [48]). */
typedef struct {
    int researched; /* the player has had the company researched and it has a fair price: only then does `gap` count */
    double gap;     /* (fair price - price) / price */
    int has_change;
    double change; /* price / the price of the month before - 1 */
    double cap;    /* price x shares, cents */
} re_stock_rank;
enum { RE_STOCK_BY_CHEAP, RE_STOCK_BY_DEAR, RE_STOCK_BY_CHANGE, RE_STOCK_BY_CAP };
/* `order[i]` becomes the row that goes to place i. By the price against the fair price, the cheapest first or the
 * dearest first: the researched companies, and under them the others as they stood. By change: the company that
 * rose most first, and under them those without a price of the month before. By capitalisation: the largest
 * first. Equal rows keep their order. */
void re_stock_order(const re_stock_rank *rows, int n, int by, int *order);

/* The months in which the fee for a company's research was paid (REQUEST.md [48], note m28 "Q5"): one number a
 * company in the plugin's record of a playthrough. A fee once paid stays paid for that playthrough: a save loaded
 * from before the payment has the research done and the fee taken again. The number keeps the last 24 months and
 * the course of share prices they belong to - `course` is the trade lock's count of how often the price stream was
 * moved, and a month paid in another course says nothing about this one.
 * `re_stock_paid_add` returns the record with `month` in it; `re_stock_paid_has` is 1 when `month` is in it. */
#define RE_STOCK_PAID_MONTHS 24
long long re_stock_paid_add(long long record, int course, int month);
int re_stock_paid_has(long long record, int course, int month);

/* What one industry adds to a company's yearly earnings a share at every month end, as a fraction of the equity a
 * share: the industry's yearly price change x the company's weight in it x 0.025 (the term of the monthly step). */
#define RE_STOCK_INDUSTRY_FACTOR 0.025
#define RE_STOCK_INDUSTRY_SIGN 0.005 /* one + or - of the game's research rows */
double re_stock_industry_term(double rate, double weight);
/* The value of a research row for that term: the game's run of signs and, when `per_month` is given, what the term
 * does to the yearly earnings a share every month, as an amount with its sign: "-- (-$2.17 a month)". Returns the
 * length; 0 for a term of exactly nothing, which the game writes its own way. */
unsigned re_stock_industry_row(double term, int lang, const char *per_month, char *out, unsigned cap);

/* For the month's warning about shares the player holds: "Name (bankrupt at month end)". Returns the length and
 * sets `*level` (higher = more pressing); 0 when the company's state is nothing to warn a holder about. */
enum { RE_STOCK_CALM, RE_STOCK_CLOSE, RE_STOCK_WATCH, RE_STOCK_MEASURE_DUE, RE_STOCK_BANKRUPT_DUE };
unsigned re_stock_held(const re_stock_view *v, int lang, const char *name, int *level, char *out, unsigned cap);

#endif
