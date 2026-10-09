/* Read-only access to the running game: the household's balance sheet, its debts and its cash-flow record.
 * Every pointer is checked for readability before it is followed; a failed check leaves the field's `ok` flag at 0. */
#ifndef RE_GAME_H
#define RE_GAME_H

#include <windows.h>

#include "re_business.h"

#define RE_MAX_DEBTS 64
#define RE_MAX_TAGS 48

typedef struct {
    int id, activity, type; /* type 1 = annuity, 2 = bullet at maturity */
    int months_left, length_months, delay_months;
    float stored_rate;    /* 0 = variable */
    long long principal;  /* cents */
    long long due;        /* cents the next monthly pass will charge (game's own instalment function) */
    long long year;       /* cents it costs over a year: the instalments of the next twelve months, or the bullet once */
} re_debt;

typedef struct {
    int tag;
    long long expense, income; /* cents, summed over the window */
} re_tag_sum;

typedef struct {
    /* calendar and identity */
    int ok_calendar;
    long long uni_count;
    int tick, month, year, period; /* period = month + 12 * year, the key of the monthly records */
    char playthrough[64];

    /* economy */
    int ok_economy;
    float growth, base_rate, reserve_rate;
    double price_index; /* inflation index, tag 1531; 0 when not found */
    unsigned market_seed, market_calls;

    /* balance sheet, cents */
    int ok_balance;
    long long net_worth, assets_gross, assets_net, debt, cash, savings, investments, stocks, houses_gross, houses_net;
    int bankrupt_end;

    /* debts */
    int debt_count; /* all of them; -1 = the map could not be read */
    re_debt debts[RE_MAX_DEBTS]; /* the first RE_MAX_DEBTS */
    long long debts_due; /* sum of `due` over all of them: what the next monthly pass charges */
    long long debts_year; /* sum of `year` over all of them */
    /* loans approved at the bank and not yet used; activity 0 = none. `due` is not filled. */
    re_debt pending_mortgage, pending_education;

    /* cash-flow record over the last complete months (at most 12), cents */
    int cf_months; /* 0 = no usable record */
    long long cf_income_total, cf_expense_total;
    int cf_tag_count;
    re_tag_sum cf_tags[RE_MAX_TAGS];
} re_household;

void re_game_init(BYTE *image_base);
void re_game_snapshot(void *main_obj, re_household *h);
/* The same without the balance sheet and the cash flow (ok_balance stays 0): the calendar, the playthrough, the cash,
 * the debts with what the next month end takes for them, and the worth of the homes. A fraction of the time. */
void re_game_snapshot_debts(void *main_obj, re_household *h);

/* The central bank's lending rate per year, the base of every loan rate. 0 when it cannot be read. */
float re_game_reserve_rate(void *main_obj);

/* The id the debt list (PFM + 0xa8) will give to the next debt. 0 when it cannot be read. */
int re_game_next_debt_id(void *debt_manager);

/* Where debt `id` keeps its yearly rate (0 = variable). NULL when there is no such debt. The one place the plugin
 * writes into a debt that is already in the list: the rate of a fixed mortgage, once, right after its purchase. */
float *re_game_debt_rate(void *main_obj, int id);

/* Calls visit(period, tag, expense, income) for every tag of every recorded month in [first, last]. */
typedef void (*re_cashflow_visit)(int period, int tag, long long expense, long long income, void *ctx);
int re_game_cashflow_walk(void *main_obj, int first, int last, re_cashflow_visit visit, void *ctx);

/* The same walk over the statement the yearly tax is computed from (cash movements booked as taxable). */
int re_game_taxable_walk(void *main_obj, int first, int last, re_cashflow_visit visit, void *ctx);

/* The two numbers the yearly tax applies to that statement: the untaxed part in cents and the rate. 0 when unreadable. */
int re_game_tax_terms(void *main_obj, long long *tax_free, float *rate);

/* The listed company with this id as the stock market keeps it; NULL when there is none or the map is unreadable. */
BYTE *re_game_listed_company(void *market, int id);
/* A property the household could buy now (note b17): what is asked for it, and what the game's own formula makes it
 * worth today. The two differ by a draw the game makes when it puts the property on the market. */
typedef struct {
    char address[80];
    long long price, value; /* cents */
} re_listing;
/* The properties for sale, in the order of the game's list. Calls the game's price function for each, without its
 * random part. Returns how many were written (at most `cap`), -1 when the list cannot be read. */
int re_game_listings(void *main_obj, re_listing *out, int cap);

/* What a business of the household loses to missing assets (note b19): the two factors the game multiplies a
 * person's work efficiency by, as stored. 1 = nothing is short. The game does not let them count below 0.5
 * (furnishings) and 0.35 (the assets of a job). */
typedef struct {
    int id; /* what the staff of the business is kept under too */
    char name[64];
    float furnishings, assets; /* assets: the smallest over the jobs of the business */
    /* the running month, counted as the game's month end counts it for the month's mark (note b19 "B4"): the hours
     * the jobs were given, and those still to do */
    int work, undone;
    int adverts;     /* bought adverts switched on */
    float awareness; /* how well the business is known, 1 = 100% */
    int open;        /* 1 = open to its work, 0 = closed (the button of its window): no hours, no month's work */
} re_firm_fit;
/* The household's businesses that are set up (a type and premises). Returns how many were written (at most `cap`),
 * -1 when the list cannot be read. Reads only. */
int re_game_firm_fits(void *main_obj, re_firm_fit *out, int cap);
/* The jobs of a business of the household with the hours of the running month (note b19 "B4", m22): `job` is the
 * job of the game's data, what an employee's record names (the business keeps its jobs under numbers of its own);
 * `allowed` and `overtime` are the hours of work the job was given, `worked` those done; what is still to do is
 * max(0, allowed + overtime - worked). They are hours of work, what an employee's hour times the work efficiency
 * gives. `outsourced`: the game does the hours of that job itself, for money, when its people run out of hours.
 * Returns how many were written (at most `cap`), -1 when the business cannot be read. Reads only. */
typedef struct {
    int job, worked, allowed, overtime, outsourced;
} re_job_hours;
int re_game_firm_jobs(void *firms, int firm, re_job_hours *out, int cap);
/* The contracts a business of the household has signed (note m32): `start` is the month it was signed in, year x 12
 * + month as re_game_period counts; `hours` what its listed jobs bring a month, without the work those set off. The
 * game gives a month its work at the month start from the contracts signed by then, so a contract brings its work
 * from the month after `start`. Returns how many were written, -1 when the business cannot be read or has more than
 * `cap`. Reads only. */
typedef struct {
    int id, type, months, start, hours;
} re_signed;
int re_game_firm_signed(void *firms, int firm, re_signed *out, int cap);
/* How the hours a month of the business's contracts change from the month `month` to the one after it, by the job
 * a contract lists them under: plus those of a contract signed in `month`, whose work starts with the next month
 * (note m32 "Q1"), minus those of a contract whose last month `month` is (run 417: signed in 108 for 24 months, it
 * worked in 132 and was gone in 133). Returns how many jobs were written, -1 when the business cannot be read.
 * Reads only. */
typedef struct {
    int job, hours;
} re_coming;
int re_game_firm_coming(void *firms, int firm, int month, re_coming *out, int cap);
/* The contracts a business is offered this month (note b16 "B6", m33): the id an offer is kept under and the
 * contract, which re_game_offer reads; `shown` gets the business as re_game_offer wants it. Returns how many, -1
 * when the business cannot be read or has more than `cap`. Reads only. */
typedef struct {
    int id;
    const BYTE *contract;
} re_offered;
int re_game_firm_offers(void *firms, int firm, re_offered *out, int cap, const BYTE **shown);
/* How many more contracts the business can hold, by the game's own function: what its type allows less what is
 * signed. -1 when there is no such business. */
int re_game_firm_places(void *firms, int firm);
/* The reputation of a business (1 = 100%) and whether its influence operation is switched on. 0 when there is no
 * such business. Reads only. */
int re_game_firm_reputation(void *firms, int firm, double *reputation, int *influence);
/* 1 = the business is open to its work, 0 = it is closed, -1 = there is no such business. Reads only. With
 * re_game_closed_unseen set (a test's control) every business reads as open, here and in re_firm_fit. */
int re_game_firm_open(void *firms, int firm);
extern int re_game_closed_unseen;
/* Signs an offer with the game's own function, the one its action "Accept Contract" ends in: the contract goes
 * into the signed ones and out of the offers, and the business's list of jobs follows. No money, no notice; the
 * offer's pointer is dead afterwards. 1 when the business has the contract then. */
int re_game_contract_sign(void *firms, int firm, const re_offered *offer);
/* What missing assets leave of an hour of work in one job of a business, as the game stores it (note b19 "A1"):
 * the assets of that job - 1 when the job has none listed - and the furnishings of the business. 0 when the business
 * cannot be read. `re_game_firm_refit` has the game work the two out again, as it does every game hour by itself. */
int re_game_firm_factors(void *firms, int firm, int job, double *assets, double *furnishings);
void re_game_firm_refit(void *firms, int firm);
/* The id of the household's business that `firm_stack` is; 0 when it is not one of the household's. */
int re_game_firm_of(void *firms, const void *firm_stack);
/* The game's own hiring of a candidate of the month's list of that job (note m22): the candidate joins the staff
 * and leaves the list. No money, no notice. 1 when the staff has that person afterwards. */
int re_game_hire(void *firms, int firm, int job, int candidate);
/* An asset a business is short of (note b23). The game gives every asset a business has "powers" by tag (a desk
 * brings 1 of "desk") and works out what the business wants of every tag from its jobs, staff and expected hours;
 * less than wanted lowers the work efficiency. */
typedef struct {
    int tag, wanted, owned;
} re_asset_need;
/* The tags a business of the household has less of than it wants: how many were written (at most `cap`), -1 when
 * the business cannot be read. What is wanted comes from the game's own function, which reads only; what it hands
 * back is freed the way the game frees it. */
int re_game_firm_needs(void *firms, int firm, re_asset_need *out, int cap);
typedef struct {
    int type, power; /* the ware; what one unit of it brings of the tag asked for */
    double space;    /* the floor space of a unit */
    long long price; /* cents a unit costs that business now, as the game's shop works it out; 0 = not for sale */
    int fixed;       /* the game keeps one of it and replaces that one */
    int active;
    double hours;    /* its life as the data gives it: hours of use ("usageHours") or of the clock ("decayHours"); 0 = neither */
    double running;  /* what it uses up in an hour, cents at the day's prices (re_business_asset_ware) */
    char name[48];
} re_ware;
/* The wares of the game's data that bring an asset tag: how many were written (at most `cap`), -1 when the data
 * cannot be read. */
int re_game_tag_wares(void *firms, int firm, int tag, re_ware *out, int cap);
/* How many units of a ware a business of the household has among its assets; 0 when it cannot be read. */
int re_game_firm_asset_units(void *firms, int firm, int type);
/* The name of an asset tag as the game shows it; empty when there is none. */
void re_game_tag_name(void *firms, int firm, int tag, char *out, unsigned cap);
/* The floor space of a business: what its assets and stock take, by the game's own function, and the most its
 * premises hold. 0 when either cannot be read. */
int re_game_firm_space(void *firms, int firm, double *used, double *most);
/* The premises of a business of the household and what making them larger would be (note m33). The game's star in a
 * business window, "upgrade this property at the end of the month", writes the type to grow into; the month end then
 * doubles the floor space, and charges the price once more for an owned property or the month's rent once more,
 * every month from then on, for a rented one. `next` is the type this one grows into, 0 when it is the largest of
 * its kind; `growing` the type the star holds now, 0 when it is off; `cost` what the month end would charge, as the
 * game works it out, in cents. 0 when the premises cannot be read. Reads only. */
typedef struct {
    int house, type, owned, growing, next;
    double space;
    long long cost;
    char address[80];
} re_premises;
int re_game_firm_premises(void *firms, int firm, re_premises *out);
/* Writes the type to grow into, as a click on the star does; 0 switches it off. 0 when the premises are not found. */
int re_game_firm_premises_grow(void *firms, int firm, int type);
/* The business type of a business of the household, as the game's data names it; 0 when there is no such business. */
int re_game_firm_type(void *firms, int firm);
/* Its awareness, 1 = 100%; below 0 when there is no such business. */
double re_game_firm_awareness(void *firms, int firm);
/* The adverts of a business (note m22). Whether one is switched on, as the game's own function says. */
int re_game_advert_on(void *firms, int firm, int advert);
/* What an advert costs that business an hour, in cents, as the game works it out; 0 for an advert that is an
 * activity of the player and no switch. The sign is the game's. */
long long re_game_advert_price(void *firms, int firm, int advert);
/* Switches an advert of a business on or off with the function the click on its icon calls. It does not look
 * whether the advert is on already: the caller does. */
void re_game_advert_switch(void *firms, int firm, int advert, int on);
/* trace: what the adverts of a business cost in their last turn (the game's function runs every 91st hour and
 * the game pays the amounts right after), in cents by advert. Returns how many were written (at most `cap`), -1
 * when the map cannot be read. Reads only. */
int re_game_firm_advert_costs(void *firms, int firm, int *keys, long long *amounts, int cap);
/* trace (note b23): the assets of a business. `types` gets the different item types (at most `cap`), `*records` how
 * many asset records there are, `*auto_buy` and `*hours_taken` the two switches the game's own replacement of a
 * worn-out asset asks for. Returns how many types were written, -1 when the business cannot be read. Reads only. */
int re_game_firm_asset_types(void *firms, int firm, int *types, int cap, int *records, int *auto_buy, int *hours_taken);
/* trace (note b23): the game's own function for an asset of that type that has worn out - its notice, and with both
 * switches on a new unit and the cash movement for it. Nothing wears out or is removed by the call. */
void re_game_asset_replace(void *firms, int firm, int type);

/* Every listed company of the market, in the order of their ids. Returns how many were written (at most `cap`),
 * -1 when the map cannot be read. */
int re_game_companies(void *market, BYTE **out, int cap);

/* The household's finance object, the `this` of the game's cash function; NULL when it cannot be reached. */
void *re_game_finance(void *main_obj);

/* The household; NULL when it cannot be reached. */
void *re_game_people(void *main_obj);

/* The month (month + 12 x year) in which the household last finished an activity that is counted per household.
 * RE_COOLDOWN_NONE when the save has no entry for it: the game's own value for "never". */
#define RE_COOLDOWN_NONE (-999)
#define RE_COOLDOWN_UNREADABLE (-2147483647 - 1)
int re_game_cooldown(void *people, int activity);
/* Writes that month, adding the entry when there is none. One of the few places the plugin writes into the game's
 * data: the entry is what makes the game refuse the activity until the month after. 0 when it cannot be reached. */
int re_game_set_cooldown(void *people, int activity, int month);

/* Takes actions out of the queues of the household's members: every queued action that `wanted` says yes to, asked
 * with its activity and how many of its hours have gone (0: not begun), the way the game takes one out for a right
 * click (no money moves, nothing is written anywhere else; note m27). `out` gets what was taken, at most `cap`.
 * Returns how many were taken, -1 when the household cannot be read. */
typedef struct {
    int activity, hours, gone;
} re_queued;
int re_game_queue_take_out(void *people, int (*wanted)(int activity, int gone), re_queued *out, int cap);

/* What the game's lists show as the money of an activity: its `action.money` through the game's price chain, in
 * cents, negative for a cost. `prices` is the object at RE_ACTION_PRICES of a person's actions. */
long long re_game_activity_money(void *prices, void *object);

/* The stock market the household trades on; NULL when it cannot be reached. */
void *re_game_market(void *main_obj);

/* The player's shares, one entry per company the market keeps a count for (the count can be 0). Returns how many
 * were written, at most `cap`; -1 when the map cannot be read. */
#include "re_futures.h"

typedef struct {
    int company, shares;
} re_holding;
int re_game_holdings(void *market, re_holding *out, int cap);

/* A company's name as the save holds it. UTF-8, zero-terminated. 0 when it cannot be read or does not fit. */
int re_game_company_name(const BYTE *company, char *out, unsigned cap);

/* The industries a listed company sells into (positive weight) or buys from (negative), each with the yearly price
 * change the economy has for it now. `company` is the market's own object (re_game_listed_company), not a window's
 * copy. Returns the count, at most `cap`; -1 when the list or the rates cannot be read. */
typedef struct {
    int id;
    double weight, rate;
} re_industry;
int re_game_company_industries(void *market, const BYTE *company, re_industry *out, int cap);

/* The economy as the rule of the rates needs it (re_futures.h): every industry's rate, index, weight and policy
 * term, the overall rate and index, growth. 0 when something cannot be read or there are more industries than the
 * structure holds. */
int re_game_economy(const void *economy, re_fut_economy *out);
/* The economy object of the loaded game; NULL when it cannot be reached. */
void *re_game_economy_object(void *main_obj);

/* The yearly price change of a ware as the game works it out, 0.025 = 2.5%: what the futures window shows as the
 * asset's inflation rate. The game's own function is called (it reads only); the plugin's stand-in for the rate of
 * an industry list sees the ware's industries on the way. `economy` is re_game_economy_object's. */
double re_game_ware_rate(void *economy, int ware);

/* The object that owns every business and, inside it, the one the efficiency function works on; NULL when they
 * cannot be reached. */
void *re_game_firms(void *main_obj);
void *re_game_jobs(void *firms);

/* A staff record, a candidate or an employee alike: what an hour's worth of work costs with that person, in cents
 * (re_business_work_cost; -1 when it cannot be said). `efficiency` gets the game's own number, 1.0 = 100%. */
long long re_game_staff_cost(void *jobs, const BYTE *staff, double *efficiency);

/* Puts the `count` records from `first` on into the order `order` names (re_business_order), with the game's own
 * copy and destruction of a record. The vector is one the hire tab has just been given for itself. */
#define RE_STAFF_SORT_MAX 8
void re_game_staff_reorder(BYTE *first, int count, const int *order);

/* The employees of a business, as the game keeps them: `*first` is the first record, each RE_STAFF_BYTES long.
 * Returns how many there are, -1 when the list cannot be read. Reads only. */
int re_game_firm_staff(void *firms, int firm, BYTE **first);
/* How many of them the game manages automatically (note b19 "B1": a switch per person, set in the staff tab), asked
 * of the game's own function; `*employees` gets how many there are. -1 when the list cannot be read. */
int re_game_firm_managed(void *firms, int firm, int *employees);
/* Switches that management on for one employee, with the function the staff tab's click calls. */
void re_game_staff_manage(void *firms, int firm, int staff);
/* 1 when one employee has it. */
int re_game_staff_auto(void *firms, int firm, int staff);
/* The game's two switches of a business that do more with the mod, turned off as a click turns them off (note m27):
 * the automatic management of every employee - returns how many had it, -1 when not read - and "buy a worn-out
 * asset again" - returns 1 when it was on, 0 when it was off already, -1 when not read. */
int re_game_firm_unmanage(void *firms, int firm);
int re_game_firm_auto_assets_off(void *firms, int firm);
/* The same two switched on, as a click does it: the staff switch of everybody who has it off (returns how many),
 * and the asset switch with the game's step that takes the month's staff hours for the purchases (returns what the
 * switch was). -1 when the business cannot be read. */
int re_game_firm_manage(void *firms, int firm);
int re_game_firm_auto_assets_on(void *firms, int firm);
/* The record of one employee inside the game's list; NULL when the business has no such person. */
BYTE *re_game_employee(void *firms, int firm, int staff);
/* The name of a business of the household by its id. 0 when there is none or the name cannot be read. */
int re_game_firm_name(void *firms, int firm, char *out, unsigned cap);

/* A staff record, a candidate or an employee alike, as the wage rule needs it (notes b19 "B2", b20): what the
 * record stores, and the two numbers the game is asked for. A wage demand is open while it is above the wage; the
 * game then makes the person leave at the next month end. */
typedef struct {
    int id, job, hours, start; /* hours of a month; start = the month of the hire, month + 12 x year */
    int left;                  /* of those hours, what is not worked yet this month; the month end fills it up again */
    long long wage, demand;    /* cents a month */
    double efficiency;         /* the game's own number, 1.0 = 100% */
    long long standard;        /* the job's standard wage of an hour, cents: what the game's own wage ratio measures against */
    char name[96];             /* "First Last"; empty when it cannot be read */
} re_staff_read;
/* 0 when the record or the jobs object cannot be read. Reads only. */
int re_game_staff_read(void *jobs, const BYTE *record, re_staff_read *out);
/* The standard wage of a job for an hour, in cents, as the game's economy has it today (it follows the price
 * level): what the game measures a wage against. `jobs` is re_game_jobs. */
long long re_game_job_wage(void *jobs, int job);
/* The game's own copy of a staff record into RE_STAFF_BYTES of raw memory, and the end of such a copy. */
void re_game_staff_copy(BYTE *raw, const BYTE *record);
void re_game_staff_end(BYTE *record);
/* One more employee for a business: the game appends its own copy of `record`, as its month end does with the
 * person it finds for the place of one who resigned. No money, no notice. */
void re_game_staff_append(void *firms, int firm, const BYTE *record);
/* One employee out of the staff, with the function the staff tab's dismissal calls before it pays the severance
 * itself: no money, no notice here. */
void re_game_staff_remove(void *firms, int firm, int staff);
/* The answer yes to an open wage demand: the wage becomes `wage` and the demand is closed. */
void re_game_staff_set_wage(void *firms, int firm, int staff, long long wage);

/* The candidates the game keeps for a job of a business until the month ends (note b21): how many there are.
 * RE_CANDIDATES_NONE when no list has been made this month, which is when the game draws one at the next look; a
 * list that was emptied by hiring is still a list. Walks the two maps and adds nothing to them. */
#define RE_CANDIDATES_NONE (-1)
#define RE_CANDIDATES_UNREADABLE (-2)
int re_game_candidates(void *firms, int firm, int job);
/* The game's random streams as the businesses reach them; NULL when the object or one of its thirteen engines
 * cannot be read, or when the jobs and the object that draws ages do not point at the same one. */
BYTE *re_game_randgen(void *firms);
/* The share price of a listed company as the game wrote it down in a month (the record its price chart draws),
 * cents; 0 when there is no record of that month. */
long long re_game_company_price_at(void *market, int company, int period);
/* The "Research Stock" button of a company's tab is not made for a company `re_action_button_none` answers 1 for
 * (a cdecl callback; the hook hands it the company's id): one the mod has researched this month. `re_action_button`
 * is the game's maker, which the hook goes on to otherwise. */
extern void *re_action_button;
extern int (*re_action_button_none)(int company);
void re_action_button_hook(void);
/* A chart window whose series are the tags of one statement and which has `series` among its boxes: in the
 * chart's record of what is ticked only that series stays, and its "all" flag goes off. Returns how many ticks
 * were taken away; -1 for any other chart, which is left alone. The boxes on the screen follow when the game's
 * own function (RE_VA_CHART_TICK with 0) reads the record. */
int re_game_chart_only(void *chart, int series);
/* More about a chart window (note m23 section 5). All of these read and write the window's own tables; the boxes
 * and the picture follow when the game's functions run.
 * - kind: what the chart is of (RE_CHART_KIND_*); -1 when it cannot be read.
 * - colours: every series of the chart gets a colour of `palette` {r, g, b, a}, in the order of the series' tags,
 *   round again when there are more series than colours. Returns the number of series, -1 when not read.
 * - keep: the record of ticks for every series OF THIS CHART that has an entry: count > 0 - the series in `keep`
 *   ticked, the others not; count == 0 - all ticked; count < 0 - none. Other than with count == 0 the flag of the
 *   box for all goes off. A series without an entry gets one from the game when its box is first ticked, with the
 *   flag's value. Returns how many entries changed.
 * - has: 1 when `series` has a box on the page of boxes that is shown.
 * - page: write == 0 notes, for every box of the page shown, its series and what the record says of it (the flag
 *   of the box for all where the record has nothing), up to `count`; write == 1 puts `count` notes back into the
 *   record. Returns the number of boxes noted, -1 when not read. */
int re_game_chart_kind(const void *chart);
int re_game_chart_colours(void *chart, const float (*palette)[4], int count);
int re_game_chart_keep(void *chart, const int *keep, int count);
int re_game_chart_has(const void *chart, int series);
int re_game_chart_page(void *chart, int *series, unsigned char *ticked, int count, int write);
/* More of an open chart (note m23 section 7):
 * - set: every series of the chart gets an entry in the record of ticks, made with the game's own function where
 *   there is none: ticked when among `keep` (`count` > 0), not ticked otherwise; the flag of the box for all goes
 *   off. Other than `keep` above this reaches the series of the pages of boxes not shown yet. Returns how many of
 *   `keep` are series of the chart, -1 when not read.
 * - series: 1 when `series` is one of the chart's, on whatever page its box is.
 * - cap: the months shown are lowered to `months` when they are more; returns the months shown, -1 when not read.
 * - last: the value of `series` in the last month the chart's statement has; 0 when there is none. */
/* A person's experience (note m26). `statement` is the object at RE_PERSON_XP of a person.
 * - decay: the game's own monthly step, every number that requirements are measured against times `factor`,
 *   rounded up.
 * - walk: for every kind of experience that has such a number: what was ever gained of it, what was taken away,
 *   and the number itself, which the visitor may write. Returns the kinds walked, -1 when not read. */
typedef void (*re_xp_visit)(int tag, long long gained, long long lost, long long *effective, void *ctx);
void re_game_xp_decay(void *statement, float factor);
int re_game_xp_walk(void *statement, re_xp_visit visit, void *ctx);
int re_game_chart_set(void *chart, const int *keep, int count);
int re_game_chart_series(const void *chart, int series);
int re_game_chart_cap(void *chart, int months);
int re_game_chart_last(const void *chart, int series, long long *value);
/* The next number of months a chart shows, for its time button: the first of `steps` (rising) above what is shown
 * and under the chart's most, else the most, and from the most the first step again. Stored and returned. */
int re_game_chart_step(void *chart, const int *steps, int count);
/* A month's map <series, int64 value> that a chart's drawing has copied for itself: every value above zero becomes
 * 10000 x value / the series' base, the base being the first value the series was seen with (`base`, `*count` of
 * `cap` entries, kept by the caller for one drawing). 10000 reads as 100.00 on the chart. Returns how many values
 * were changed, -1 when the map cannot be read. */
typedef struct {
    int series;
    long long value;
} re_chart_base;
int re_game_month_index(void *map, re_chart_base *base, int cap, int *count);
/* 1 when a Shift key is among the keys a window of the game (the hire window, the stock window) knows to be held
 * down. */
int re_game_shift_held(void *window);
/* The same for the set of held keys itself, as the game's own "is this key held" gets it in ecx. */
int re_game_keys_shift(const void *keys);
/* The same without a window at hand: the hire window is a part of the object the ticker is in. */
int re_game_shift_down(void *main_obj);
/* The same for a Ctrl key. */
int re_game_ctrl_down(void *main_obj);
/* The business the business window shows, or showed last: the firm id it was built for. 0 when it cannot be read. */
int re_game_window_firm(void *main_obj);

/* An offered contract as re_business_offer_value takes it (note b18 "Q5"), and what the log says besides: the
 * kind of contract, what the game's payout function makes of it today, and the two factors of the difficulty and
 * of the length that are in that amount. */
typedef struct {
    re_offer terms;
    int type;
    long long today; /* cents; without the firm's reputation */
    double difficulty, length;
} re_offer_read;
/* Reads it for the contract of a payout hover text; `shown_firm` is the business that hover text was given. The
 * contract's payout, months and hours are the stored ones. The rest is asked of the game: the standard wage of
 * every job, the payout function on a copy of the contract, the length factor and the three markups. Those
 * functions read the game's data and prices; none of them draws a random number. 0 when anything is not as the
 * plugin expects, with `out` filled as far as it got. */
int re_game_offer(void *main_obj, const BYTE *shown_firm, const BYTE *contract, re_offer_read *out);

/* The month the game's calendar shows, as the key of the monthly records (month + 12 x year); -1 when unreadable. */
int re_game_period(void *main_obj);

/* Adds one line to the monthly summary panel, with one of the game's icons (for example "UISymbolDebt"). UTF-8 text.
 * first = 1 puts it at the top of the month's list, 0 appends it like the game's own lines. Returns 0 when the
 * list cannot be reached. */
int re_game_post_event(void *main_obj, const char *text, const char *icon, int first);

/* Replaces the text of the summary line that starts with `prefix`. 1 = replaced, 0 = no such line, -1 = the list
 * could not be read. The panel shows the new text the next time it is opened. With `text` NULL nothing is changed
 * and the answer says whether there is such a line. */
int re_game_replace_event(void *main_obj, const char *prefix, const char *text);

/* The summary panel builds its rows from a copy of the month's list when it opens, so a line added or changed
 * afterwards is not in an open panel. This has the open panel build its rows again, with the function its own
 * filter buttons call. 1 = rebuilt, 0 = the panel is not showing or cannot be reached. Not for use from inside a
 * text request: a window may be in the middle of its own build there (note b14). */
int re_game_rebuild_summary(void *main_obj);

/* One line in the ticker at the top of the screen, where the game reports what an action did. UTF-8 text.
 * Returns 0 when the ticker cannot be reached. */
int re_game_post_message(void *main_obj, const char *text);

/* The same with the ticker's rank chosen. The ticker shows its lines by rank, 1 to 4, the oldest of a rank first,
 * and a line of rank 2 that waits with four others or more is never shown (ranks 3 and 4: with three others); it
 * is then only in the hover text of the ticker, which lists the last twenty. A month change posts dozens, so a
 * line of rank 2 posted there is lost (seen in the game, run 165). urgent = rank 1, which the game uses for a
 * payment that became debt: always shown, before the others. Otherwise rank 2, as re_game_post_message. */
int re_game_post_urgent(void *main_obj, int urgent, const char *text);

/* The game's hour counter since the start of the playthrough; -1 when it cannot be read. */
long long re_game_ticks(void *main_obj);

/* An amount as the game itself writes it: with the currency symbol, exchange rate and denomination of the player's
 * settings. UTF-8, zero-terminated. Returns 0 when the game's formatter cannot be reached or `cap` is too small. */
int re_game_money_text(void *main_obj, long long cents, char *out, unsigned cap);

/* The stream that drives stock prices. Returns 0 when it cannot be reached. */
int re_game_market_stream(void *main_obj, unsigned **seed, unsigned **calls, void **unit);

#endif
