#ifndef RE_BUSINESS_H
#define RE_BUSINESS_H

#include "re_sites.h"

/* A business of the household (REQUEST.md [30], notes b16 and b18): what an hour's worth of work costs with one
 * employee, and how much more an offered contract pays than the standard cost of the work it brings. The game shows
 * the hourly wage and the efficiency apart and a contract's payout without its cost. Those two change nothing of
 * the game's rules; the answer to a wage demand, further down, does. */

/* Cents for an hour's worth of work: the month's wage over the hours that get done in it. -1 = cannot be said. */
long long re_business_work_cost(long long wage, int hours, double efficiency);
/* order[0 .. n) = the places of cost[0 .. n) from the cheapest on. Equal costs keep the order they came in, and one
 * that cannot be said (-1) goes behind the others. */
void re_business_order(const long long *cost, int n, int *order);

/* An offered contract and a month of the work it brings (note b18 "Q4"). The game makes the payout from that
 * month's cost: labour at the wage of each job times one markup, wares and utilities times another, then the
 * difficulty, the length and the firm's reputation. Nothing here is about the firm's own staff. Money in cents. */
typedef struct {
    long long total; /* T: the whole payout */
    int months;
    double labour;   /* Ls: the listed hours and the hours they set off in other jobs, at the jobs' standard wages */
    double hours;    /* H: the listed hours */
    double induced;  /* G: the hours they set off, as many as the payout counts on */
    double weighted; /* M: the month's cost as the payout weighs it, each part times its markup */
    double policy;   /* A: what a wage policy of the city adds to every standard wage, an hour; M is made without it */
    double x_labour, x_inventory, x_utilities; /* the markups */
} re_offer;
typedef struct {
    double materials; /* X: the month's wares and utilities */
    double cost;      /* C: the standard cost of the month, labour + materials */
    double premium;   /* T / (months x C) - 1: 0.5 = the offer pays half as much again as its work costs */
    double surplus;   /* (T / months - C) / (H + G): what is left of the payout for every hour of work */
} re_offer_value;
/* 0 = cannot be said: a number is missing, or the markups on wares and on utilities differ, so that M cannot be
 * taken apart. */
int re_business_offer_value(const re_offer *offer, re_offer_value *value);

/* A wage demand of an employee the game manages automatically (REQUEST.md [33], [38]; notes b19 "B2" and b20). The
 * game says yes to every one. Here the demand is met unless one of the job's candidates would do that person's work
 * for less: a person who leaves over a demand costs nothing, and the candidate takes the place. */
#define RE_WAGE_PLACE_SHARE 0.9
/* What an hour of the work a job needs costs with one person: the wage of a month over what of that work the person
 * gets done. `able` is what the person can do in the time there is and `need` the work that is wanted, both in
 * hours x work efficiency. Hours the job has no work for make nobody cheaper (REQUEST.md [43]: a need of 12 hours
 * is best met by the lowest wage, not by the best rate of a full month). 0 = nothing of it gets done. */
double re_business_need_cost(long long wage, double able, double need);
#define RE_NEED_LEAST 1.0 /* a need below this counts as this: then the wages themselves are compared */
/* The candidate for a person's place. `need` is the work the job wants from that place in a month: what the person
 * did this month and what the job left undone. A candidate counts who gets at least RE_WAGE_PLACE_SHARE of it done
 * - of the person's whole month when the need is more than that - and the one with whom an hour of the needed work
 * costs least is picked; `cost` gets that cost in cents. Candidates come with 48 to 160 hours a month. -1 = nobody
 * can take the place. (Run 118: with the cheapest hour of a whole month as the measure, a place of 48 hours asked
 * $1,885.86 for went to a person of 88 hours at $3,343.16.) */
int re_business_wage_pick(const long long *wage, const int *hours, const double *efficiency, int n, int place_hours, double place_efficiency,
                          double need, double *cost);
/* Staff filled in during the month (REQUEST.md [43], note m22). The candidate for work nobody of the job has the
 * hours for: the one with whom an hour of that work costs least, a person counting for at most `clock` hours, the
 * game hours the month has left (an employee does one hour of work in a game hour at most). Equal costs: the lower
 * wage. -1 = none. */
int re_business_fill_pick(const long long *wage, const int *hours, const double *efficiency, int n, double need, int clock, double *cost);
/* 1 = a job is short of people: of the `todo` hours of work it still has this month, its people can do `able`, and
 * what is missing is at least RE_FILL_LEAST hours and more than `share` of the `given` hours, the job's work of the
 * whole month. */
#define RE_FILL_LEAST 8.0
int re_business_short(int given, int todo, double able, double share);
/* What missing assets leave of an hour of work (REQUEST.md [45], note b19 "A1"): the game multiplies a person's
 * skill by the assets of the job, which count for no less than RE_ASSETS_FLOOR, and by the furnishings of the
 * business, for no less than RE_FURNISH_FLOOR. 1 = nothing is short. An hour of a person is worth skill x this in
 * hours of the job's work. */
#define RE_ASSETS_FLOOR 0.35
#define RE_FURNISH_FLOOR 0.5
double re_business_asset_factor(double assets, double furnishings);
/* The least the game stores for the assets of a job. A job that has none of one of its assets stands there, and the
 * game does not let such a job be worked at all (FUN_004e8930 lists the asset as a problem, FUN_004ea7f0 marks the
 * job not doable; run 166: 89 hours of a job stayed undone for a month with hired hands idle). A job with a tenth
 * or less of an asset stands there too and is worked; the mod takes both for "an asset is missing altogether". */
#define RE_ASSETS_LEAST 0.1
/* 1 = the job did not need this person in the month that is ending: the `mates` other people of the same job, those
 * who are staying, left unworked at least as much work as this one did, and would still have had RE_SPARE_RESERVE
 * of their month free with it (`others_month`: what they get done in a full month). All in hours x work efficiency.
 * Without the reserve a part-timer in a large job counts as spare whenever the job has a few per cent of slack, and
 * a job without slack leaves work undone in its next busy month. Nobody is spare in a job of one. */
#define RE_SPARE_RESERVE 0.10
int re_business_wage_spare(double did, double others_left, double others_month, int mates);
/* A person alone in a job who stands idle most of the month (REQUEST.md [42]): the candidate who takes the place
 * has fewer hours a month and a lower monthly wage and gets RE_SMALLER_HEADROOM times the work done that the
 * present person did this month (`did`, hours x work efficiency); of those the one with the lowest wage. -1 = none. */
#define RE_SMALLER_HEADROOM 1.25
int re_business_smaller_pick(const long long *wage, const int *hours, const double *efficiency, int n, long long place_wage, int place_hours,
                             double did);
/* An asset a business is short of, bought (REQUEST.md [43], note b23). A business wants so much of every asset tag
 * ("desk", "parking space") and has what its assets bring of it; a ware brings `power` of a tag by the unit. */
typedef struct {
    int power;       /* what a unit brings of the tag; 0 = no ware */
    double space;    /* the floor space of a unit */
    long long price; /* of a unit, cents; 0 or less = not for sale */
    int owned;       /* the business has this ware already */
    int others;      /* units of it in the household's other businesses */
    double hours;    /* its life, hours; 0 = it does not wear */
    double running;  /* what it uses up in an hour, cents */
} re_asset_offer;
/* What an hour of one unit of a tag costs with a ware, cents: the price spread over the ware's life and what it uses
 * up, over what a unit brings - of which no more counts than is `missing` (a ware that brings six where two are
 * missing is paid for six). An hour is an hour of use for a ware that wears with use and an hour of the clock for
 * one that wears with time: an estimate for telling wares apart, not a bill. */
double re_business_asset_hour(const re_asset_offer *offer, int missing);
/* The ware for a tag (REQUEST.md [45]):
 *   1. one the business has already - the player chose it; among several the lowest price for one of the tag;
 *   2. else one the household's other businesses have, the one they have most units of;
 *   3. else, among the wares the cash covers, the one with the lowest re_business_asset_hour;
 *   4. else (the cash covers none) the lowest price for one of the tag.
 * Ties: the lower price for one of the tag, then the less floor space for one. A ware whose unit does not fit into
 * `space`, the floor space left, is passed over as long as another one fits; when none fits the choice is made as
 * if all did, and the purchase then fails for the room, which is what the player is told. -1 = none that brings
 * the tag and has a price. */
int re_business_asset_ware(const re_asset_offer *offer, int n, int missing, long long cash, double space);
typedef struct {
    int wanted, owned;   /* of the tag */
    re_asset_offer unit; /* the ware picked for it */
} re_asset_want;
/* The want a unit is bought for next, when not everything can be bought: among the wants still short whose unit
 * fits into the floor space left and the cash, the one whose unit fills the largest share of its want for the
 * share it takes of whichever is scarcer, room or cash. So with little room the small units go first and with
 * little cash the cheap ones. -1 = nothing more can be bought. */
int re_business_asset_next(const re_asset_want *want, int n, double room, long long cash);
/* Why a want that is still short gets no unit. */
enum { RE_ASSET_SERVED, RE_ASSET_NO_WARE, RE_ASSET_NO_ROOM, RE_ASSET_NO_CASH };
int re_business_asset_why(const re_asset_want *want, double room, long long cash);
/* The adverts of a business type (REQUEST.md [43], note m22). In place of the game's call of its list function for
 * the element "advert" of a type's "brand" when a business window is built: the game's function runs as called,
 * then the callback gets the vector it filled {first, end, room} - elements of 0x18 bytes, the advert at +0xc - and
 * the plain bytes at the head of the type's record the call was handed by value. Every register is as the game's
 * function left it. Not reentrant; the call does not nest. */
typedef void (*re_business_brand_fn)(unsigned char *const *vector, const unsigned *record);
extern re_business_brand_fn re_business_brand;
extern void *re_business_brand_list;  /* the game's function */
void re_business_brand_hook(void);    /* target for re_patch_call; not callable from C */

typedef struct {
    double keep;  /* cents for an hour of the needed work with this person at the demanded wage */
    double fresh; /* the same with the candidate */
} re_wage_costs;
/* 1 = meet the demand, 0 = refuse it. `demanded` is the wage of a month in cents, `need` the work the job wants from
 * the place (as for re_business_wage_pick), `fresh` what re_business_wage_pick gave. The candidate has to be cheaper
 * by more than `margin` (0.03 = 3%). A number that is not positive leaves nothing to compare: 1, what the game
 * does, and both costs 0. */
int re_business_wage_accept(long long demanded, int hours, double efficiency, double need, double fresh, double margin, re_wage_costs *costs);

/* The candidates of a job (REQUEST.md [30], [43]; note b21). The game draws the candidates of a job at the first
 * look at them in a month, from random streams that every hour of play and many clicks move, so loading a save and
 * doing something else first gives other people or other wages. Here the list is drawn from a number that depends
 * only on what is below; `draw` is 0 for the list every look at that month gives (the plugin passes nothing else
 * since the paid new draw was taken out, [43]). */
unsigned long long re_business_list_seed(const char *playthrough, int month, int firm, int job, int draw);
/* The 32-bit seed of one engine for such a list: `stream` 0 to 11 for the game's streams, from 12 on for the
 * reference wages of the list's people in the order they are made. */
unsigned re_business_stream_seed(unsigned long long seed, int stream);

/* The game's random streams as they stood before a list was made: twelve of {seed, calls, engine} and all thirteen
 * engines. About 33 KB, so not for the game's stack. */
typedef struct {
    unsigned char *randgen;
    unsigned unit[RE_RAND_STREAMS][3];
    unsigned char *spare;
    unsigned char engine[RE_RAND_ENGINES][RE_RAND_ENGINE_BYTES];
} re_streams;
/* Keeps the streams of `randgen` in `keep`, then seeds the twelve streams' engines for the list `seed` with the
 * game's own function (`seed_fn`: ecx = an engine, a pointer to the 32-bit seed on the stack, which it removes).
 * The caller has checked that the object and its thirteen engines can be read. */
void re_business_streams_seed(re_streams *keep, unsigned char *randgen, void *seed_fn, unsigned long long seed);
/* Puts everything back. `drawn[i]` gets how many draws stream i gave in between. Returns 1 when the streams, read
 * again afterwards, are what was kept: every engine byte for byte and every call counter. 0 when not; an engine
 * that is not where it was is left alone. */
int re_business_streams_restore(const re_streams *keep, unsigned drawn[RE_RAND_STREAMS]);

/* The game's efficiency function: ecx = jobs, the job id and the record's skill part on the stack, result in xmm0. */
float re_business_efficiency(void *function, void *jobs, int job, const void *skill);
/* The game's payout function on a copy of a contract, the way the game's own call does it: room for `bytes` in the
 * argument area, the game's copy constructor into it, then ecx = firms and the firm id. The payout function destroys
 * the copy. Cents, before the firm's reputation counts. */
long long re_business_payout(void *copy, void *payout, void *firms, int firm, const void *contract, unsigned bytes);
/* The game's factor for a long contract: ecx = firms, the months on the stack, result in xmm0. */
float re_business_length(void *function, void *firms, int months);
/* A number of the game's data by the names of its element and of its attribute: ecx = data, the two names as
 * std::string by value, result in xmm0. The function frees the two strings, so they are filled with the game's own
 * `assign`: a name of more than 15 letters lives on the game's heap. It serves a function that removes the two
 * strings from the stack itself and one that leaves that to its caller. */
float re_business_data_number(void *function, void *data, void *assign, const char *element, unsigned element_length, const char *name,
                              unsigned name_length);

/* The hire tab's list: before the game's function runs, one callback gets the firms object, the business and the
 * job; after it has filled the vector, the other gets the firms object and the vector {first, end, room}. */
typedef void (*re_business_listing_fn)(void *firms, int firm, int job);
typedef void (*re_business_listed_fn)(void *firms, unsigned char **vector);
extern re_business_listing_fn re_business_listing;
extern re_business_listed_fn re_business_listed;
extern void *re_business_list;    /* the game's function */
void re_business_list_hook(void); /* target for re_patch_call; not callable from C */

/* The reference wage of a new person, in place of the game's call for it in the generator of a person: the
 * callback may write another seed where the game put the person's id, then the game's function runs as called. */
typedef void (*re_business_wage_seed_fn)(int *seed);
extern re_business_wage_seed_fn re_business_wage_seed;
extern void *re_business_wage_ref;    /* the game's function */
void re_business_wage_ref_hook(void); /* target for re_patch_call; not callable from C */

/* The name on the row of a candidate's action, "Conduct Interview". A hook in front of a string assign just before
 * the row is made notes the candidate (esi there). A second hook, where the row maker fetches the action's name,
 * puts the callback's text behind the name, in brackets, when the row maker was called from the hire tab. The
 * callback writes its text into `out` and returns the length (0 = leave the name as it is). */
typedef unsigned (*re_business_line_fn)(const unsigned char *staff, char *out, unsigned cap);
extern re_business_line_fn re_business_hire_line;
extern void *re_business_assign;         /* the game's string assign */
extern void *re_business_name;           /* the game's name of an object */
extern unsigned re_business_hire_return; /* where the row maker returns to in the hire tab */
void re_business_hire_row_hook(void);    /* targets for re_patch_call; not callable from C */
void re_business_name_hook(void);

#endif
