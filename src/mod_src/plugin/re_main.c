/* Realistic Economy plugin for This Grand Life 2 v1.0322: entry point and the features.
 *
 * One DLL, several features. Each feature has a switch in RealisticEconomy.ini, needs a set of code sites to match
 * the analysed build, and reports its own state at start-up. A feature whose sites do not match stays off and says
 * so; the others keep working. On any other game build nothing is patched at all.
 *
 *   credit    loan rates follow the household's credit grade and mortgage LTV (attribute getter detour);
 *             the grade is shown in the rate line of the game's own loan windows and debt tooltips.
 *             A new fixed mortgage is the variable rate plus a premium; a new education loan is fixed at the
 *             central-bank rate of the month it starts in.
 *   forecast  what the coming month end will take out of the cash account: the debt instalments, computed exactly,
 *             and the other month-end payments as they were at the last month end; shown as one line of the game's
 *             monthly summary panel
 *   wording   texts of the game that mistranslate the English original are put right ("수정됨" for a fixed rate
 *             becomes "고정"); most of them Korean, a few in other languages
 *
 * The plugin's own texts come from re_lang.c, in the language the game is set to; English where it has none.
 *   guard     after loading a save from before a month end the playthrough has already passed, the player's stock
 *             and futures trades are refused until that month end has been passed again; the lock has a cap, and a
 *             rollback beyond the cap moves the stock-price random stream instead
 *   ipo       listing a public company pays the founder: the public buys the shares the founder does not keep at the
 *             listing price, the cash is taxable income like the price of a sold firm, the founder keeps 45%, and
 *             no new money goes into the company
 *   casino    the four casino games have a fixed stake each and pay by chance alone, once a month per household;
 *             the result is booked when the action ends, and a game the playthrough has played stays played
 *             whatever save is loaded afterwards
 *   business  a business of the household: every candidate for a job shows what an hour's worth of work costs with
 *             that person (wage over efficiency) and the list starts with the cheapest; every contract on offer
 *             shows how much more it pays than the standard cost of the work it brings; a person hired where
 *             others are under the game's automatic management is put under it too; that management meets a wage
 *             demand only while the person stays cheaper than a new one, and hires for the place of one who leaves
 *             over a demand it did not meet; the candidates of a job are the same whatever save is loaded
 *   experience  a household member's experience and educations are not lost to time below what was gained of them,
 *             up to the most that any requirement of the game's data asks
 *   trace     observation only, for test runs: every cash movement, the loan-rate reads at month end,
 *             the cash-flow record by tag, the stock-price random stream
 *
 * No window is created: text is added to windows the game already has. The plugin writes no file of the game. What
 * it puts into the game's own data is saved by the game like anything else: the lines in the summary panel, the
 * rate of a new fixed loan and the position of the stock-price stream. Loans that already exist are not rewritten.
 * The log is RealisticEconomy.log and what the plugin remembers between sessions is RealisticEconomy.state, both next
 * to the plugin. The settings file is written in one case: [guard] release is put back to 0 once it has been used.
 */
#include <windows.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "re_board.h"
#include "re_business.h"
#include "re_casino.h"
#include "re_credit.h"
#include "re_font.h"
#include "re_futures.h"
#include "re_game.h"
#include "re_getter.h"
#include "re_guard.h"
#include "re_hook.h"
#include "re_ipo.h"
#include "re_lang.h"
#include "re_loan.h"
#include "re_memory.h"
#include "re_sites.h"
#include "re_stock.h"
#include "re_text.h"
#include "re_trace.h"
#include "re_util.h"
#include "re_wording.h"
#include "re_xp.h"

#define PLUGIN_VERSION "1.1.0"
#define MAX_TAG_LIST 16
#define TRACE_LINE_LIMIT 6000
#define MONTH_TICKS 728 /* hours in the game's month; the month-end routine runs when the hour of the month is 727 */

typedef void(__fastcall *main_fn)(void *main_obj);
typedef int(__thiscall *chance_fn)(void *rand_unit);
/* the game's cash function: int64 cents, finance tag, 1 = also booked as taxable income or deductible expense */
typedef void(__thiscall *money_fn)(void *finance, unsigned low, int high, int tag, int taxable, int unused1, int unused2);

static BYTE *g_base;
#define AT(va) (g_base + ((va) - RE_IMAGE_BASE))

static int g_on_credit, g_on_forecast, g_on_wording, g_on_guard, g_on_ipo, g_on_trace, g_can_text;
static int g_lang; /* the game's language as a column of re_lang.h */
#define T(msg) re_lang_text(g_lang, msg)
static int g_trace_money, g_trace_getter, g_trace_cashflow, g_trace_heap, g_market_shift, g_economy_shift, g_market_shift_from;
static int g_load_count, g_heap_sample[4], g_heap_samples;
static int g_getter_installed, g_money_installed, g_text_installed, g_in_monthend;
static int g_loans_tried, g_edu_fixed, g_guard_tried;

/* The load guard. The lock is kept as the hour it ends at; the record it comes from is in the state file. */
static int g_guard_cap_months = 2, g_guard_shift = 1;
static long long g_guard_unlock; /* trades are refused while the game's hour counter is below this; 0 = no lock */
static int g_guard_month_end_pending; /* the loaded save is from the hour of the month end that ends the lock */
static int g_guard_quiet;             /* ... and its panel has no lock line: that hour is not written into the panel */
static int g_guard_auto_refused; /* purchases of the monthly auto transfer refused during this month end */
/* What a month end draws, the price of a listing and the property market of a new month come from the month, not
 * from where the streams stand: [guard] monthEndFixed. `hooked`: both ends of the draws are redirected, so what is
 * seeded is put back. */
static int g_month_fixed = 1, g_month_tried, g_month_hooked, g_listing_hooked, g_property_month_hooked;

/* The listing of a public company. `g_ipo_keep` is what three instructions of the game read in place of the saved
 * fraction once the patch is in; the game's own cap on a holding is 45%. */
static float g_ipo_keep = 0.45f;
static int g_ipo_floor = 1, g_ipo_taxable = 1, g_ipo_tried, g_ipo_installed, g_ipo_steps_hooked;
static long long g_ipo_preview_cash = -1; /* what the last preview of the IPO window would pay; -1 = none since shown */
/* The IPO window's quality letter by listing price / offer price: AAA above 3, AA 2.5, A 2, B 1.5, C 1.2, D 1.
 * For a business earning y of its value a year the twelve price steps give about 1 at 5%, 1.2 at 7.5%, 1.5 at 10%,
 * 2 at 14%, 2.5 at 18%, 3 at 23%. E is a business priced at the floor or losing money. */
static double g_ipo_letters[RE_IPO_LETTERS] = {3.0, 2.5, 2.0, 1.5, 1.2, 1.0};
static re_ipo_grade g_ipo_grade; /* what six instructions of the window read once the patch is in */
static int g_ipo_grade_installed;

/* A seat on the board for a large shareholder: one for every `g_board_per_seat` of the company the household holds. */
static int g_on_board, g_board_tried, g_board_notice = 1, g_board_lines;
static int g_board_prepass; /* test runs: ask for every nominee's total even when the household needs none of them */
static double g_board_per_seat = 0.20;
static float g_board_top = 1000.0f; /* the game's own totals stay below 12.4 (note b11) */
#define BOARD_TRACE_LINES 300

/* The casino. `g_casino_keep`: a game played after the loaded save was written is booked again at the load. */
static int g_on_casino, g_casino_tried, g_casino_installed, g_casino_keep = 1;
static char g_playthrough[64]; /* the id of the loaded playthrough: the section of the state file */

/* A business: what an hour's worth of work costs with an employee, what an offer pays over the cost of its work. */
static int g_on_business, g_business_tried, g_business_sorted, g_business_lines;
static int g_on_memory, g_memory_docs, g_memory_managers, g_memory_delete;
static int g_research_board, g_research_sub; /* stocks: research kept fresh, for the board's companies and by subscription */
static double g_research_hours = 5.0;        /* the fee a company a month: hours of a bank analyst at the standard wage */
static int g_sort_buttons = 1, g_sort_hooked; /* stocks: three sort buttons of the company list order by value and change */
static int g_sort_cap_hooked;                 /* and the second one by capitalisation ([48]) */
static int g_chart_price = 1;                 /* stocks: a company's chart opens with the share price alone */
/* stocks: the economy's chart opens with one series; colours, ticks over a page turn and time steps; all share
 * prices as an index. g_chart_steps: the time button's step is the mod's (its text for half a year is needed) */
static int g_chart_one = 1, g_chart_fixes = 1, g_chart_index = 1, g_chart_hooked, g_chart_steps;
/* stocks: the household's wealth, the stock index and all share prices open with the lines read most, the economy's
 * three with five years (g_chart_caps: that cap is in place, the time button's text has to follow); the radius of a
 * chart's lines, 0 = the game's thin line; a series' last value behind its name */
static int g_chart_open = 1, g_chart_caps, g_chart_values = 1;
static double g_chart_line = 2.0; /* the game draws a chart's axes with this radius */
static int g_row_numbers = 1;                 /* stocks: a row of the company list has the change and the fair price */
static int g_business_fit;                   /* business: the month's line about work efficiency lost to missing assets */
static int g_business_auto_new;              /* business: a person hired where others are managed automatically is too */
/* business: the wage demands of automatically managed staff (wage_wrapper). `g_wage_refuse_all` is a test knob. */
static int g_wage_rule = 1, g_wage_spare = 1, g_wage_refuse_all, g_wage_hooked;
static double g_wage_margin = 0.03; /* a candidate takes the place when cheaper by more than this */
static double g_hire_hours = 20.0;  /* what such a hiring costs: hours of a store manager at the standard wage */
/* business: the candidates of a job are drawn from the playthrough and the month alone (list_begin) */
static int g_candidates_fixed = 1, g_fixed_hooked;
static int g_on_property;                    /* the month's line about a property for sale under its value */
static int g_on_experience, g_xp_hooked;     /* a household member's educations are kept up to what requirements ask */
static int g_on_fonts, g_fonts_later;        /* a letter the font lacks comes from another font (re_font.h); later: at the first load */
static int g_on_names;                       /* a name in letters no font has is written in Latin letters: picked, or read from a save */
static int g_trace_names;                    /* trace: every name the game picks is one of those (`[trace] namesNoFont=1`) */
static double g_property_gap = 0.05;         /* how far under its value a property has to be asked for to be named ... */
static double g_property_amount = 54000.0;   /* ... and by how many dollars of the game's first prices */
static int g_property_tried;
static int g_spare_months = 3;      /* business: for how many month ends in a row a person has to be one too many */
static double g_idle_share = 0.5;   /* ... or a person alone in a job has to leave this share of the hours unworked */
static int g_fill = 1, g_fill_hooked; /* business: a job with more work than its people have hours for gets a candidate hired */
static double g_fill_share = 0.05;    /* ... when more than this share of the job's month is without hands */
static int g_hire_ahead = 1;          /* ... and ahead of contract work that a month's candidates will not staff */
static double g_trace_pool = 1.0;     /* trace: what a month's candidates count for when hiring ahead (`[trace] poolShare`) */
static int g_auto_whole = 1, g_auto_whole_patched; /* business: a click on a row's switch is for the whole business */
/* business: the adverts of a business rest for a month after one in which more than this share of the work was left
 * undone; looking after them costs hours of a PR specialist a month */
/* business: the adverts of a business the player has handed over are switched by the mod, on while the awareness is
 * below what the target needs, off once it is there (adverts_keep) */
static int g_ads_keep = 1, g_ads_keep_hooked;
/* business: the contracts of a business the player has handed over are signed by the mod at a month start, one a
 * month, when the business can carry it (contracts_sign); the premises of one handed over whole are made larger
 * when its floor space is short (premises_grow) */
static int g_contracts_keep = 1, g_contracts_hooked;
static double g_contract_hours = 10.0; /* what a signing costs: hours of a store manager at the standard wage */
static int g_premises_grow = 1;
static int g_lock = 1, g_locks; /* a business handed over whole is locked: the click handlers the lock stands in */
static double g_ads_upto = 1.03;  /* the awareness at which the mod switches a business's adverts off, 1 = 100% */
static double g_ads_hours = 5.0;  /* what a month of one advert looked after costs: hours of a PR specialist */
#define BUSINESS_TRACE_LINES 600

/* Futures, a part of the stocks feature: the industries of whatever the game asked the inflation rate of last, and
 * the economy it asked. The futures window asks for its asset's rate just before it writes the rate's label. */
static void *g_fut_economy;
static re_fut_tag g_fut_tags[RE_FUT_TAGS];
static int g_fut_count = -1, g_fut_tried, g_fut_installed, g_fut_lines;
static int g_fut_list = 1, g_fut_list_hooked; /* stocks: the futures list has the yearly price change and is sorted by it */
#define FUTURES_TRACE_LINES 400

/* What a company's numbers say about its share, written into the stocks window. */
static int g_on_stocks, g_stock_sites, g_stock_tried, g_stock_lines;
#define STOCK_TRACE_LINES 3000 /* with trace on: a list of twelve companies is logged every time it is built */
static LONG g_trace_lines, g_getter_reads;

/* A fixed mortgage rate is the variable rate x factor + premium; the game's own numbers are 1.1 and 0.03. Here the
 * factor is 1 and the premium follows the level of rates: a fixed loan is priced off a long rate that lies between
 * the central-bank rate and the rate the game's economy keeps returning to (its baseInterestRate), so
 * premium = fixedPremium + fixedReversion x (baseInterestRate - central-bank rate). `g_fixed_now` is what the game's
 * two instructions read; it is refreshed after every load and month end. */
static double g_fixed_factor = 1.0, g_fixed_premium = 0.005, g_fixed_reversion = 0.5, g_fixed_now = 0.005;
static int g_new_fixed_id; /* a fixed mortgage created a moment ago, to be priced once the purchase has settled */

/* What the month-end routine moved through the cash function, by kind, in cents. Outflows are stored positive.
 * FLOW_INCOME is the household's wages, which the routine pays in before anything goes out. Every other receipt
 * (a contract a firm was paid for, dividends) comes later in the routine or not every month, so the forecast does
 * not count on it; it is kept as FLOW_RECEIPTS for the log. */
enum {
    FLOW_INCOME,
    FLOW_STAFF,
    FLOW_GOODS,
    FLOW_UTILITIES,
    FLOW_RENT,
    FLOW_PROPERTY_TAX,
    FLOW_TAX,
    FLOW_OTHER,
    FLOW_RECEIPTS,
    FLOW_COUNT
};
static const char *const FLOW_KEYS[FLOW_COUNT] = {"flowIncome", "flowStaff",       "flowGoods", "flowUtilities", "flowRent",
                                                  "flowPropertyTax", "flowTax", "flowOther", "flowReceipts"};
static long long g_flow_now[FLOW_COUNT], g_flow_last[FLOW_COUNT];
static int g_flow_period = -1; /* period of the month end g_flow_last belongs to; -1 = none */
/* Each property pays its tax every third month counted from its purchase, so last month's amount says nothing about
 * this month's. What was paid is kept per month of the quarter (period % 3); -1 = that month was not watched. */
static long long g_property_tax[3] = {-1, -1, -1};
static const char *const PROPERTY_TAX_KEYS[3] = {"propertyTax0", "propertyTax1", "propertyTax2"};
static int g_forecast_period = -1; /* the period whose end the forecast is about */
static long long g_forecast_need;  /* the cash that month end is expected to take, as the forecast's line says; 0 = none or not known */
static int g_forecast_whole;       /* 1 = that number has every payment of a month end in it; 0 = the debts alone (none watched yet) */

static void *g_main_obj;                           /* the game's root object, as handed to the two call sites */
static volatile LONG g_refresh_due, g_refreshing;  /* money moved during the month: grade and forecast need another look */
static DWORD g_refresh_at;
static long long g_shown_due = -1; /* debt instalments the forecast line was last built with */
static long long g_graded_debt = -1, g_graded_houses = -1; /* debt and homes the grade was last computed with */
static int g_graded_count = -1;
static void refresh_if_due(void);

static re_credit_config g_credit_cfg;
static re_credit_result g_credit;
static int g_credit_ready;
static int g_tags_capital[MAX_TAG_LIST], g_tags_business[MAX_TAG_LIST], g_tags_not_income[MAX_TAG_LIST];
static int g_n_capital, g_n_business, g_n_not_income;

static double dollars(long long cents)
{
    return (double)cents / 100.0;
}

static unsigned va_of(unsigned address)
{
    return address - (unsigned)(UINT_PTR)g_base + RE_IMAGE_BASE;
}

static void report_site(const char *line)
{
    re_log("%s", line);
}

/* ---- how long the plugin's own work takes (REQUEST.md [47], note m30). `[trace] timing=1`, whatever trace itself is
 * set to: a run with trace=1 writes thousands of lines and is no measure of a player's game. Every stand-in that the
 * game reaches often, or that does much at once, is timed with the performance counter; a line a kind is written
 * when a month has started and after a load, with how often, how long in all and the longest single time. The game's
 * own month end, month start and load are timed next to the plugin's part of them, as what to hold it against. */
enum {
    TIME_TEXT,       /* a text the game fetches: text_want and text_edit, with the look at the household after cash moved */
    TIME_MONEY,      /* a cash movement */
    TIME_FIRM_BLOCK, /* a business's block of every 91st hour: staff filled in, assets bought for it, adverts switched */
    TIME_MONTH_END,
    TIME_MONTH_START,
    TIME_LOAD,
    TIME_XP, /* the experience of a household member after the game's monthly step */
    /* two things the plugin asks of the game, inside the kinds above: the household's numbers (the game values every
     * asset for the net worth) and the candidates of a job (the game makes the month's people when nobody has looked) */
    TIME_SNAPSHOT,
    TIME_HIRE_LIST,
    TIME_GAME_MONTH_END,
    TIME_GAME_MONTH_START,
    TIME_GAME_LOAD,
    TIME_SAVE_STRING, /* a string the game has read from a save, looked at for a name no font can write */
    TIME_KINDS
};
static const char *const TIME_NAME[TIME_KINDS] = {
    "texts fetched",     "cash movements",    "business blocks (staff, assets, adverts)",
    "the plugin's part of a month end", "the plugin's part of a month start", "the plugin's part of a load",
    "experience kept",   "inside those: the household's numbers read", "inside those: the candidates of a job listed",
    "the game's own month end", "the game's own month start", "the game's own load",
    "strings of a save looked at for a name no font can write",
};
static struct {
    long long ticks, most;
    unsigned calls;
} g_time[TIME_KINDS];
static int g_timing;

static long long time_now(void)
{
    LARGE_INTEGER t;
    if (!g_timing)
        return 0;
    QueryPerformanceCounter(&t);
    return t.QuadPart;
}

static void time_add(int kind, long long since)
{
    if (!g_timing)
        return;
    long long spent = time_now() - since;
    g_time[kind].ticks += spent;
    g_time[kind].calls++;
    if (spent > g_time[kind].most)
        g_time[kind].most = spent;
}

static void snapshot(void *main_obj, re_household *h)
{
    long long since = time_now();
    (re_game_snapshot)(main_obj, h);
    time_add(TIME_SNAPSHOT, since);
}

static void time_report(const char *when, int period)
{
    LARGE_INTEGER per_second;
    if (!g_timing || !QueryPerformanceFrequency(&per_second) || per_second.QuadPart <= 0)
        return;
    for (int kind = 0; kind < TIME_KINDS; kind++) {
        if (g_time[kind].calls)
            re_log("timing (%s, period %d): %s: %u time(s), %.3f ms in all, the longest %.3f ms", when, period, TIME_NAME[kind],
                   g_time[kind].calls, 1000.0 * (double)g_time[kind].ticks / (double)per_second.QuadPart,
                   1000.0 * (double)g_time[kind].most / (double)per_second.QuadPart);
        g_time[kind].ticks = g_time[kind].most = 0;
        g_time[kind].calls = 0;
    }
}

/* whole dollars with thousands separators: 62133140 -> "621,331" */
static const char *usd(long long cents, char out[32])
{
    char digits[24];
    long long whole = (cents < 0 ? -cents : cents) / 100;
    int n = snprintf(digits, sizeof digits, "%lld", whole), at = 0;
    if (cents < 0)
        out[at++] = '-';
    for (int i = 0; i < n; i++) {
        out[at++] = digits[i];
        if ((n - i - 1) % 3 == 0 && i != n - 1)
            out[at++] = ',';
    }
    out[at] = 0;
    return out;
}

/* An amount for a line on screen, written the way the game writes amounts: the player's option decides between
 * dollars and the city's currency, and a save can carry a redenomination. Whole dollars with "$" only when the game's
 * formatter is out of reach. */
static const char *money(void *main_obj, long long cents, char out[64])
{
    char digits[32];
    if (!g_can_text || !re_game_money_text(main_obj, cents, out, 64))
        snprintf(out, 64, "$%s", usd(cents, digits));
    return out;
}

/* The same with a sign in front, for a gain or a loss. */
static const char *signed_money(void *main_obj, long long cents, char out[72])
{
    char amount[64];
    snprintf(out, 72, "%s%s", cents < 0 ? "-" : "+", money(main_obj, cents < 0 ? -cents : cents, amount));
    return out;
}

/* One line of the monthly summary panel, the window that opens after a load and at every month change. The line
 * stays for the month and is saved with the game like the game's own lines. If a line starting with `prefix` is
 * already there its text is replaced, otherwise the line is added. The panel is built from the list when it opens:
 * a line added right after the month-end routine is in the panel at once, any other change shows the next time the
 * panel is opened. */
static void panel_line(void *main_obj, const char *prefix, const char *text, const char *icon, int first)
{
    int how = !g_can_text ? -1 : re_game_replace_event(main_obj, prefix, text);
    if (how == 0)
        how = re_game_post_event(main_obj, text, icon, first) ? 2 : -1;
    re_log("summary panel (%s): %s", how == 1 ? "line replaced" : how == 2 ? "line added" : "not shown, log only", text);
}

/* ------------------------------------------------- text in existing windows */

#define KEY_LOAN_RATE "loanVariableRateYearly" /* rate line of the personal loan, education loan and savings window */
#define KEY_DEBT_RATE "currentInterestRate"    /* rate line of the mortgage window, of a debt's tooltip and of savings */
#define KEY_IPO_KEPT "stockRetainedShares"     /* "Retained Shares (25%)" in the IPO window; the 25 is part of the text */
#define KEY_STOCK_LIST "clickView"             /* hover text of many lists; the stocks window's company list is one */
#define KEY_ACTIVITY_TITLE "activityTitle"     /* "Activity", the first line of an activity's hover text in a list */
#define KEY_HIRE_COUNT "firmStaffHireCandidates" /* "4 candidates", above the list of the hire tab */
#define KEY_STAFF_WAGE_TIP "staffWageTip"        /* the sentence in the hover text of a staff card's wage icon */
#define KEY_OFFER_TIP "contractHasPayoutsTip"    /* the sentence in the hover text of a contract's payout icon */
#define KEY_STAFF_AUTO_TIP "staffAutoManageTip"  /* the hover text of the automatic-management switch in a staff row */
#define KEY_FUT_TIP "assetInflationTitle"        /* hover text of the rate in the futures window */
#define KEY_FUT_SORT "futuresSortByUnitValue"    /* hover text of the futures list's second sort button */
#define KEY_CHART_YEARS "chartYearsTranslate"    /* the text of a chart's time button, "2 years" */
#define KEY_CHART_ALL_PRICES "allSharePriceChartTitle" /* the title of the chart of all share prices */
#define KEY_ADVERT_TIP "clickToActivateDeactivate" /* the last sentence in the hover text of a bought advert's icon */
#define KEY_XP_TIP "chartEffectivenessDecayTip"  /* the experience window's "?": experience decays by a share every month */

enum {
    TEXT_GRADED_RATE = 1,
    TEXT_MORTGAGE_QUOTE,
    TEXT_EDU_RATE,
    TEXT_IPO_KEPT,
    TEXT_STOCK_LIST,
    TEXT_CASINO_TIP,
    TEXT_HIRE_ORDER,
    TEXT_STAFF_TIP,
    TEXT_OFFER_TIP,
    TEXT_FUT_TIP,
    TEXT_AUTO_TIP,
    TEXT_ADVERT_TIP,
    TEXT_SORT_CHEAP, /* the hover texts of four sort buttons of the stock window, in the order of RE_STOCK_BY_* */
    TEXT_SORT_DEAR,
    TEXT_SORT_CHANGE,
    TEXT_SORT_CAP,
    TEXT_FUT_LIST, /* the hover text of a row of the futures list */
    TEXT_FUT_SORT, /* the hover text of that list's second sort button */
    TEXT_CHART_YEARS, /* a chart's time button after a click: "0 years" is half a year */
    TEXT_CHART_YEARS_OPEN, /* the same button when a chart opens: its window is used again and may still show six months */
    TEXT_CHART_INDEX, /* the title of the chart of all share prices says what its numbers are */
    TEXT_XP_TIP, /* under the game's sentence about the monthly decay of experience: what the mod keeps */
    TEXT_WORDING /* + the entry of the wording table */
};

/* ------------------------------------------------------------ business */

/* The hire tab has been given its list: the candidate whose hour of work is cheapest goes first. Only the tab's own
 * copy is put in order; the list the save keeps stays as it was made. */
static void business_listed(void *firms, unsigned char **vector)
{
    void *jobs = re_game_jobs(firms);
    if (jobs == NULL || !re_readable(vector, 12) || vector[0] == NULL || vector[1] < vector[0])
        return;
    int count = (int)((SIZE_T)(vector[1] - vector[0]) / RE_STAFF_BYTES);
    if (count < 1 || count > RE_STAFF_SORT_MAX || !re_readable(vector[0], (SIZE_T)count * RE_STAFF_BYTES))
        return;
    long long cost[RE_STAFF_SORT_MAX];
    int order[RE_STAFF_SORT_MAX], moved = 0;
    for (int i = 0; i < count; i++)
        cost[i] = re_game_staff_cost(jobs, vector[0] + (SIZE_T)i * RE_STAFF_BYTES, NULL);
    re_business_order(cost, count, order);
    for (int i = 0; i < count; i++)
        moved += order[i] != i;
    if (g_on_trace && g_business_lines++ < BUSINESS_TRACE_LINES)
        for (int i = 0; i < count; i++) {
            const BYTE *staff = vector[0] + (SIZE_T)order[i] * RE_STAFF_BYTES;
            re_log("business: hire list, place %d: candidate %d (was place %d), wage %lld cents a month, %d hours, an hour of work %lld cents",
                   i + 1, *(const int *)(staff + RE_STAFF_ID), order[i] + 1, *(const long long *)(staff + RE_STAFF_WAGE),
                   *(const int *)(staff + RE_STAFF_HOURS), cost[order[i]]);
        }
    if (moved)
        re_game_staff_reorder(vector[0], count, order);
}

/* What an hour's worth of work costs with the person of a staff record, as a line of text. 0 = cannot be said. */
static unsigned business_staff_text(int msg, const unsigned char *staff, char *out, unsigned cap)
{
    double efficiency = 0.0;
    long long cost = re_game_staff_cost(re_game_jobs(re_game_firms(g_main_obj)), staff, &efficiency);
    char amount[64];
    if (cost < 0)
        return 0;
    int n = snprintf(out, cap, T(msg), money(g_main_obj, cost, amount), 100.0 * efficiency);
    if (g_on_trace && g_business_lines++ < BUSINESS_TRACE_LINES)
        re_log("business: staff %d: wage %lld cents a month, %d hours, efficiency %.4f: an hour of work %lld cents", *(const int *)(staff + RE_STAFF_ID),
               *(const long long *)(staff + RE_STAFF_WAGE), *(const int *)(staff + RE_STAFF_HOURS), efficiency, cost);
    return n > 0 && (unsigned)n < cap ? (unsigned)n : 0;
}

static unsigned business_hire_line(const unsigned char *staff, char *out, unsigned cap)
{
    return business_staff_text(RE_MSG_STAFF_LINE, staff, out, cap);
}

/* How much more an offered contract pays than the standard cost of the work it brings (note b18), as a line of text:
 * the payout over that cost, and what is left of it for every hour of work. `firm` is the business the hover text
 * was given. 0 = cannot be said. */
static unsigned business_contract_text(int msg, const unsigned char *firm, const unsigned char *contract, char *out, unsigned cap)
{
    re_offer_read r;
    re_offer_value v = {0};
    char amount[64];
    int ok = re_game_offer(g_main_obj, firm, contract, &r) && re_business_offer_value(&r.terms, &v);
    long long left = (long long)(v.surplus + (v.surplus < 0.0 ? -0.5 : 0.5));
    int n = ok ? snprintf(out, cap, T(msg), 100.0 * v.premium, money(g_main_obj, left, amount)) : 0;
    /* an offer that gets no line is logged too: the numbers stand as far as they were read, the rest is 0 */
    if (g_on_trace && g_business_lines++ < BUSINESS_TRACE_LINES)
        re_log("business: contract %d of type %d: payout %lld cents, %d months; a month of its work: %.0f listed hours and %.2f set off, "
               "standard labour %.0f cents, wares and utilities %.0f, standard cost %.0f; the game's payout today %lld cents with "
               "difficulty %.4f and length %.4f, weighted cost %.0f a month, markups %.3f %.3f %.3f, wage policy %.0f cents an hour: %s, "
               "premium %+.2f%%, %.1f cents left per hour of work",
               re_readable(contract, 8) ? *(const int *)(contract + 4) : 0, r.type, r.terms.total, r.terms.months, r.terms.hours,
               r.terms.induced, r.terms.labour, v.materials, v.cost, r.today, r.difficulty, r.length, r.terms.weighted, r.terms.x_labour,
               r.terms.x_inventory, r.terms.x_utilities, r.terms.policy, ok ? "line added" : "NO LINE", 100.0 * v.premium, v.surplus);
    return n > 0 && (unsigned)n < cap ? (unsigned)n : 0;
}

/* The contract a payout hover text is being built for, when it is one on offer; NULL for a signed one. */
static const unsigned char *business_tip_offer(const re_text_regs *regs)
{
    const unsigned char *const *slot = (const unsigned char *const *)(UINT_PTR)(regs->ebp + RE_OFFER_TIP_FRAME_CONTRACT);
    const char *offered = (const char *)(UINT_PTR)(regs->ebp + RE_OFFER_TIP_FRAME_OFFERED);
    return re_readable(slot, 4) && re_readable(offered, 1) && *offered ? *slot : NULL;
}

/* The casino game an activity's hover text is being built for; NULL for any other activity. */
static const re_casino_game *casino_tip_game(const re_text_regs *regs)
{
    const int *id = (const int *)(UINT_PTR)(regs->ebp + RE_HOVER_FRAME_ACTIVITY + RE_OBJ_ID);
    return re_readable(id, 4) ? re_casino_game_of(*id) : NULL;
}

/* The tooltip of a variable-rate debt: only the personal loan and the mortgage are priced by the grade. Arrears, an
 * old variable education loan or a loan product of another mod keep their own rate, so they get no grade. */
static int debt_is_graded(const re_text_regs *regs)
{
    const BYTE *const *slot = (const BYTE *const *)(UINT_PTR)(regs->ebp + RE_DEBT_TIP_FRAME_RECORD);
    if (!re_readable(slot, 4) || !re_readable(*slot, RE_DEBT_RECORD_ACTIVITY + 4))
        return 0;
    int activity = *(const int *)(*slot + RE_DEBT_RECORD_ACTIVITY);
    int graded = activity == RE_ID_PERSONAL_LOAN || activity == RE_ID_MORTGAGE;
    static int lines;
    if (g_on_trace && lines++ < 100)
        re_log("debt tooltip: a variable-rate debt of activity %d: %s", activity, graded ? "the grade is named" : "no grade, its rate is its own");
    return graded;
}

/* 1 = the player has handed the adverts of that business to the mod (a Shift-click on one of its advert icons). The
 * switch is kept by playthrough and business in the mod's state file, not in a save. */
static int adverts_kept(int firm)
{
    char key[40];
    snprintf(key, sizeof key, "advertsf%d", firm);
    return firm > 0 && g_playthrough[0] != 0 && re_state_get(g_playthrough, key, 0) == 1;
}

/* The same for what else of a business can be handed over (note m33): "contracts", "premises", and "all" for a
 * business handed over whole. */
static int business_kept(const char *what, int firm)
{
    char key[40];
    snprintf(key, sizeof key, "%sf%d", what, firm);
    return firm > 0 && g_playthrough[0] != 0 && re_state_get(g_playthrough, key, 0) == 1;
}

static void business_keep(const char *what, int firm, int on)
{
    char key[40];
    snprintf(key, sizeof key, "%sf%d", what, firm);
    re_state_set(g_playthrough, key, on);
}

/* The business an advert icon's hover text is being built for, when it is one of the household's. 0 for any other. */
static int advert_tip_firm(const re_text_regs *regs)
{
    const BYTE *const *icon = (const BYTE *const *)(UINT_PTR)regs->edi;
    void *firms = re_game_firms(g_main_obj);
    char name[64];
    if (firms == NULL || !re_readable(icon, 4) || !re_readable(*icon, RE_ADVERT_WINDOW_FIRM + 4))
        return 0;
    int firm = *(const int *)(*icon + RE_ADVERT_WINDOW_FIRM);
    return re_game_firm_name(firms, firm, name, sizeof name) ? firm : 0;
}

/* The grade goes next to rates that follow it: the two loan windows and the tooltip of a variable-rate personal loan
 * or mortgage. A fixed debt keeps the rate it was given, and savings are not graded. */
static int text_want(const char *key, unsigned len, unsigned caller, const re_text_regs *regs)
{
    refresh_if_due(); /* every text the game fetches passes here; almost always a single flag test */
    unsigned from = va_of(caller);
    if (g_on_credit && len == sizeof KEY_LOAN_RATE - 1 && memcmp(key, KEY_LOAN_RATE, len) == 0)
        return from - RE_VA_EDU_WINDOW < RE_EDU_WINDOW_SIZE             ? TEXT_EDU_RATE
               : from - RE_VA_PERSONAL_WINDOW < RE_PERSONAL_WINDOW_SIZE ? TEXT_GRADED_RATE
                                                                        : 0;
    if (g_on_credit && len == sizeof KEY_DEBT_RATE - 1 && memcmp(key, KEY_DEBT_RATE, len) == 0)
        return from - RE_VA_MORTGAGE_WINDOW < RE_MORTGAGE_WINDOW_SIZE    ? TEXT_MORTGAGE_QUOTE
               : from == RE_VA_DEBT_TIP_VARIABLE && debt_is_graded(regs) ? TEXT_GRADED_RATE
                                                                         : 0;
    if (g_ipo_installed && len == sizeof KEY_IPO_KEPT - 1 && memcmp(key, KEY_IPO_KEPT, len) == 0)
        return TEXT_IPO_KEPT;
    if (g_on_stocks && from == RE_VA_CALL_STOCK_LIST + 5 && len == sizeof KEY_STOCK_LIST - 1 && memcmp(key, KEY_STOCK_LIST, len) == 0)
        return TEXT_STOCK_LIST;
    if (g_casino_installed && from == RE_VA_CALL_ACTIVITY_TITLE + 5 && len == sizeof KEY_ACTIVITY_TITLE - 1 &&
        memcmp(key, KEY_ACTIVITY_TITLE, len) == 0)
        return casino_tip_game(regs) != NULL ? TEXT_CASINO_TIP : 0;
    if (g_business_sorted && from == RE_VA_CALL_HIRE_COUNT + 5 && len == sizeof KEY_HIRE_COUNT - 1 && memcmp(key, KEY_HIRE_COUNT, len) == 0)
        return TEXT_HIRE_ORDER;
    if (g_on_business && from == RE_VA_CALL_STAFF_WAGE_TIP + 5 && len == sizeof KEY_STAFF_WAGE_TIP - 1 &&
        memcmp(key, KEY_STAFF_WAGE_TIP, len) == 0)
        return TEXT_STAFF_TIP;
    if (g_on_business && from == RE_VA_CALL_OFFER_TIP + 5 && len == sizeof KEY_OFFER_TIP - 1 && memcmp(key, KEY_OFFER_TIP, len) == 0)
        return business_tip_offer(regs) != NULL ? TEXT_OFFER_TIP : 0;
    if (g_fut_installed && from == RE_VA_CALL_FUT_TIP + 5 && len == sizeof KEY_FUT_TIP - 1 && memcmp(key, KEY_FUT_TIP, len) == 0)
        return TEXT_FUT_TIP;
    if (g_fut_list_hooked && from == RE_VA_CALL_FUT_LIST_TIP + 5 && len == sizeof KEY_STOCK_LIST - 1 && memcmp(key, KEY_STOCK_LIST, len) == 0)
        return TEXT_FUT_LIST;
    if (g_fut_list_hooked && from - RE_VA_FUT_BUTTONS < RE_FUT_BUTTONS_SIZE && len == sizeof KEY_FUT_SORT - 1 &&
        memcmp(key, KEY_FUT_SORT, len) == 0)
        return TEXT_FUT_SORT;
    if ((g_chart_steps || g_chart_caps) && len == sizeof KEY_CHART_YEARS - 1 && memcmp(key, KEY_CHART_YEARS, len) == 0) {
        if (g_chart_steps && from - RE_VA_CHART_TIME_BUTTON < RE_CHART_TIME_BUTTON_SIZE)
            return TEXT_CHART_YEARS;
        if (from - RE_VA_CHART_OPEN < RE_CHART_OPEN_SIZE)
            return TEXT_CHART_YEARS_OPEN;
    }
    if (g_on_stocks && g_chart_index && from - RE_VA_ALL_PRICES_CHART < RE_ALL_PRICES_CHART_SIZE && len == sizeof KEY_CHART_ALL_PRICES - 1 &&
        memcmp(key, KEY_CHART_ALL_PRICES, len) == 0)
        return TEXT_CHART_INDEX;
    if ((g_wage_hooked || g_auto_whole_patched || g_spare_months > 0 || g_fill_hooked) && g_on_business && from == RE_VA_CALL_STAFF_AUTO_TIP + 5 &&
        len == sizeof KEY_STAFF_AUTO_TIP - 1 &&
        memcmp(key, KEY_STAFF_AUTO_TIP, len) == 0)
        return TEXT_AUTO_TIP;
    if (g_ads_keep_hooked && from == RE_VA_CALL_ADVERT_TIP + 5 && len == sizeof KEY_ADVERT_TIP - 1 && memcmp(key, KEY_ADVERT_TIP, len) == 0)
        return advert_tip_firm(regs) > 0 ? TEXT_ADVERT_TIP : 0;
    if (g_xp_hooked && len == sizeof KEY_XP_TIP - 1 && memcmp(key, KEY_XP_TIP, len) == 0)
        return TEXT_XP_TIP;
    if (g_sort_hooked && from - RE_VA_STOCK_BUTTONS < RE_STOCK_BUTTONS_SIZE) {
        /* "Sort by company type", "by market capitalisation", "by earnings a share (%)", "by listing date": the
         * orders the mod replaces, the last only when its jump was redirected */
        static const char *const keys[] = {"stockSortByType", "stockSortByMarketCap", "stockSortByEarningsPerShare", "stockSortByListing"};
        for (int i = 0; i < 3 + g_sort_cap_hooked; i++)
            if (len == strlen(keys[i]) && memcmp(key, keys[i], len) == 0)
                return TEXT_SORT_CHEAP + i;
    }
    int entry = g_on_wording ? re_wording_want(g_lang, key, len, from) : 0;
    return entry ? TEXT_WORDING + entry - 1 : 0;
}

/* What a company's numbers say about its share: the company comes from the frame of the window code that asked.
 * `target` gets the price target as the game writes amounts, for a company the player has had researched
 * (REQUEST.md [27]: the ratios for all, the target for those); it stays empty otherwise. Returns 0 when the
 * company or the calendar cannot be read. */
static int stock_view(const re_text_regs *regs, int frame_slot, int back, const char *where, re_stock_view *v, char target[64],
                      int *company_id)
{
    const BYTE *const *slot = (const BYTE *const *)(UINT_PTR)(regs->ebp + frame_slot);
    const BYTE *company = re_readable(slot, 4) ? *slot - back : NULL;
    int now = g_main_obj != NULL ? re_game_period(g_main_obj) : -1;
    if (!re_readable(company, RE_COMPANY_ID + 4) || now < 0) {
        static int told;
        if (told++ < 20)
            re_log("stocks: %s: the company or the calendar could not be read, the text is left as it is", where);
        return 0;
    }
    *company_id = *(const int *)(company + RE_COMPANY_ID);
    re_stock s;
    re_stock_read(company, &s);
    re_stock_view_of(&s, now, v);
    /* The price of the month before, from the record the game's price chart draws: the market writes every
     * company's price down once a month, and the record of the running month is the price of today (run 139). */
    void *market = re_game_market(g_main_obj);
    long long before = market != NULL ? re_game_company_price_at(market, *company_id, now - 1) : 0;
    v->has_change = before > 0 && s.price > 0;
    v->change = v->has_change ? (double)s.price / (double)before - 1.0 : 0.0;
    target[0] = 0;
    if (s.research_month != 0)
        money(g_main_obj, v->target, target);
    if (g_on_trace && g_stock_lines++ < STOCK_TRACE_LINES)
        re_log("stocks: %s, company %d: price $%.2f, equity $%.2f, earnings $%.2f, dividend $%.2f a share; researched in month %d, "
               "last emergency measure in month %d, now %d; target $%.2f; the price as recorded for month %d: %lld cents, for %d: %lld",
               where, *(const int *)(company + RE_COMPANY_ID), dollars(s.price), dollars(s.equity), dollars(s.earnings),
               dollars(s.dividend), s.research_month, s.desperate_month, now, dollars(v->target), now,
               market != NULL ? re_game_company_price_at(market, *company_id, now) : 0, now - 1, before);
    return 1;
}

/* The industries of a listed company with the rates the economy has for them now. The windows work on copies of a
 * company, so the list is taken from the market's own object. Returns the count, -1 when it cannot be read. */
#define STOCK_INDUSTRIES 16
static int stock_industries(int company_id, re_industry list[STOCK_INDUSTRIES])
{
    void *market = g_main_obj != NULL ? re_game_market(g_main_obj) : NULL;
    BYTE *listed = market != NULL ? re_game_listed_company(market, company_id) : NULL;
    return listed != NULL ? re_game_company_industries(market, listed, list, STOCK_INDUSTRIES) : -1;
}

/* The hover text of a row of the company list: the lines about the share go above the game's "click to view".
 * One line of ratios for every company, a warning when there is one, for a company the player has had researched
 * the price target with what the industries add to its earnings a month, and last who keeps its research fresh. */
static const char *research_hover(int company);
static unsigned stock_list_edit(char *text, unsigned len, unsigned cap, const re_text_regs *regs)
{
    re_stock_view v;
    char lines[300], target[64];
    int id;
    if (!stock_view(regs, RE_STOCK_LIST_FRAME_CURSOR, RE_STOCK_LIST_CURSOR_OFFSET, "list", &v, target, &id))
        return len;
    re_industry list[STOCK_INDUSTRIES];
    int count = target[0] ? stock_industries(id, list) : -1;
    double industries = 0.0;
    for (int i = 0; i < count; i++)
        industries += re_stock_industry_term(list[i].rate, list[i].weight);
    unsigned n = re_stock_text(&v, g_lang, target[0] ? target : NULL, count >= 0, industries, 1, lines, sizeof lines);
    const char *research = n > 0 ? research_hover(id) : NULL;
    if (research != NULL && n + 1 + strlen(research) < sizeof lines)
        n += (unsigned)snprintf(lines + n, sizeof lines - n, "\n%s", research);
    if (n == 0 || n + 2 + len >= cap)
        return len;
    memmove(text + n + 2, text, len);
    memcpy(text, lines, n);
    memcpy(text + n, "\n\n", 2);
    return n + 2 + len;
}

typedef void *(__thiscall *assign_fn)(void *self, const char *text, unsigned len);

/* A row of the research block of a company's first tab: the game shows, as a run of + or -, what the industry
 * added to the earnings in the month of the research. The row gets today's value instead: the same signs, and
 * behind them what that is in money, the change of the yearly earnings a share at every month end. A row the
 * plugin cannot work out keeps the game's value. */
static void stock_industry_row(const re_text_regs *regs, void *value)
{
    const BYTE *node = (const BYTE *)(UINT_PTR)regs->esi; /* of the map the research stored: industry, value */
    const BYTE *const *slot = (const BYTE *const *)(UINT_PTR)(regs->ebp + RE_STOCK_RESEARCH_FRAME_COMPANY);
    if (!re_readable(node, 0x18) || !re_readable(slot, 4) || !re_readable(*slot, RE_COMPANY_ID + 4))
        return;
    int id = *(const int *)(*slot + RE_COMPANY_ID), industry = *(const int *)(node + 0x10);
    long long equity = *(const long long *)(*slot + RE_COMPANY_EQUITY);
    re_industry list[STOCK_INDUSTRIES];
    int count = stock_industries(id, list);
    for (int i = 0; i < count; i++) {
        if (list[i].id != industry)
            continue;
        char text[120], amount[72];
        double term = re_stock_industry_term(list[i].rate, list[i].weight);
        const char *per_month = equity > 0 ? signed_money(g_main_obj, (long long)(term * (double)equity + (term < 0.0 ? -0.5 : 0.5)), amount)
                                           : NULL; /* the term is a part of the equity a share */
        unsigned len = re_stock_industry_row(term, g_lang, per_month, text, sizeof text);
        if (len)
            ((assign_fn)AT(RE_VA_STR_ASSIGN))(value, text, len);
        if (g_on_trace && g_stock_lines++ < STOCK_TRACE_LINES)
            re_log("stocks: research row, company %d, industry %d: stored at the research %.5f; now rate %.4f x weight %.0f x %.3f = %.5f",
                   id, industry, *(const float *)(node + 0x14), list[i].rate, list[i].weight, RE_STOCK_INDUSTRY_FACTOR, term);
        return;
    }
}

static void stock_researched_row(const re_text_regs *regs, void *value);

/* The four amounts of a company's first tab get their ratio behind them: the equity a share its PBR, the earnings
 * the PER (a loss: what it is of the equity), the dividend what it is of the price, the price the target. That is
 * the stocks feature. The wording feature puts the equity row's Korean label right, which says "shares per share";
 * the label does not come through the translation function, so the wording table cannot carry it. */
static void stock_row(unsigned caller, const re_text_regs *regs, void *label, void *value, unsigned *red)
{
    typedef struct {
        union {
            char buf[16];
            char *ptr;
        } u;
        unsigned size, capacity;
    } game_string;
    static const unsigned calls[] = {RE_VA_CALL_ROW_PRICE, RE_VA_CALL_ROW_EQUITY, RE_VA_CALL_ROW_EARNINGS, RE_VA_CALL_ROW_DIVIDEND};
    unsigned from = va_of(caller);
    if (from == RE_VA_CALL_ROW_INDUSTRY + 5) {
        if (g_on_stocks)
            stock_industry_row(regs, value);
        return;
    }
    if (from == RE_VA_CALL_ROW_RESEARCHED + 5) {
        if (g_on_stocks)
            stock_researched_row(regs, value);
        return;
    }
    int row = 0;
    while (row < 4 && from != calls[row] + 5)
        row++;
    if (row == 4)
        return; /* a row of another tab */
    if (row == RE_STOCK_ROW_EQUITY && g_on_wording && g_lang == RE_LANG_KO) {
        const game_string *name = (const game_string *)label;
        const char *was = name->capacity >= 16 ? name->u.ptr : name->u.buf;
        if (name->size == sizeof "주당 주식" - 1 && re_readable(was, name->size) && memcmp(was, "주당 주식", name->size) == 0)
            ((assign_fn)AT(RE_VA_STR_ASSIGN))(label, "주당 자본", sizeof "주당 자본" - 1);
    }
    re_stock_view v;
    char target[64], text[160];
    int id;
    if (!g_on_stocks || !stock_view(regs, RE_STOCK_TAB_FRAME_COMPANY, 0, "tab", &v, target, &id))
        return;
    game_string *amount = (game_string *)value;
    const char *shown = amount->capacity >= 16 ? amount->u.ptr : amount->u.buf;
    int warn = 0;
    if (amount->size >= 60 || !re_readable(shown, amount->size))
        return;
    memcpy(text, shown, amount->size);
    unsigned added = re_stock_row(&v, row, g_lang, target[0] ? target : NULL, &warn, text + amount->size, sizeof text - amount->size);
    if (added)
        ((assign_fn)AT(RE_VA_STR_ASSIGN))(value, text, amount->size + added);
    if (warn)
        *red = 1;
}

/* The month's warning about shares the household holds: one line of the summary panel that names the companies
 * a holder has to look at now, the most pressing first (REQUEST.md [30]). The game itself reports an emergency
 * measure or a bankruptcy only after it has happened. `period` is the month the line is for. */
#define HELD_COMPANIES 64
#define HELD_NAMED 3
static void held_warning(void *main_obj, int period)
{
    static re_holding held[HELD_COMPANIES];
    static char items[HELD_COMPANIES][160];
    int levels[HELD_COMPANIES], owned = 0, warned = 0, named = 0;
    void *market = re_game_market(main_obj);
    int count = market != NULL ? re_game_holdings(market, held, HELD_COMPANIES) : -1;
    if (count < 0) {
        re_log("shares held: the market or the list of holdings could not be read, no warning line");
        return;
    }
    for (int i = 0; i < count; i++) {
        char name[96];
        BYTE *company = held[i].shares > 0 ? re_game_listed_company(market, held[i].company) : NULL;
        levels[i] = RE_STOCK_CALM;
        if (company == NULL || !re_game_company_name(company, name, sizeof name))
            continue;
        owned++;
        re_stock s;
        re_stock_view v;
        re_stock_read(company, &s);
        re_stock_view_of(&s, period, &v);
        if (!re_stock_held(&v, g_lang, name, &levels[i], items[i], sizeof items[i]))
            levels[i] = RE_STOCK_CALM;
        warned += levels[i] != RE_STOCK_CALM;
        if (g_on_trace)
            re_log("shares held: company %d '%s', %d share(s): price $%.2f, equity $%.2f a share, last emergency measure in month %d, "
                   "month %d: level %d",
                   held[i].company, name, held[i].shares, dollars(s.price), dollars(s.equity), s.desperate_month, period, levels[i]);
    }
    const char *prefix = T(RE_MSG_HELD_PREFIX);
    char text[700];
    int len = snprintf(text, sizeof text, "%s", prefix);
    for (int level = RE_STOCK_BANKRUPT_DUE; level > RE_STOCK_CALM; level--)
        for (int i = 0; i < count && named < HELD_NAMED; i++)
            if (levels[i] == level)
                len += snprintf(text + len, sizeof text - (size_t)len, "%s%s", named++ ? ", " : "", items[i]);
    if (warned > named)
        snprintf(text + len, sizeof text - (size_t)len, T(RE_MSG_HELD_MORE), warned - named);
    re_log("shares held: %d compan%s, %d to warn about", owned, owned == 1 ? "y" : "ies", warned);
    if (warned)
        panel_line(main_obj, prefix, text, "UIIconStockMarket", 1);
}

/* What `hours` of a job cost at its standard wage of today, in cents: the price of what the mod does in a person's
 * place (REQUEST.md [39]). The standard wage follows the game's price level. 0 when it cannot be read. */
static long long job_fee(void *main_obj, int job, double hours)
{
    void *jobs = re_game_jobs(re_game_firms(main_obj));
    long long wage = jobs != NULL ? re_game_job_wage(jobs, job) : 0;
    return wage > 0 && hours > 0.0 ? (long long)(hours * (double)wage) : 0;
}

/* Research kept fresh (REQUEST.md [30], [38], [39], note m21). The game's "Research Stock" takes five of the
 * player's hours a company, and what it stores is of the month it was made in. At every month start the plugin
 * has the game do that research again with the game's own function: for nothing for a company with a household
 * member on its board, and for a fee a company for every other company that is subscribed, by itself or with all
 * of them (whether the household holds its shares does not matter, REQUEST.md [44]). The fee is those five hours
 * at the standard wage of a bank analyst, booked as a deductible
 * expense. It is charged whatever the cash is (REQUEST.md [43]): what the cash does not cover the game's money
 * function turns into a debt, as it does with wages a household cannot pay (three months, then due at once). The
 * fee is booked after the game's month start, not right after the month end: a household can be at nothing for
 * that moment and have its month's receipts a step later (save "55": $0.00, then $237,951.59). The forecast line
 * of the month ("cash needed at this month end") does not have the fee. */
#define LISTED_COMPANIES 256
static int g_month_ended = -1; /* the month whose end has just been worked through; -1 outside the turn of a month */

/* The subscription is switched in the game (REQUEST.md [43], [44]): a Shift-click on a company in the stock
 * window's list for that company, a Ctrl+Shift-click for every listed company at once. Both are kept by
 * playthrough in the state file; [stocks] researchSubscription is what "every company" starts as. */
static int research_all(void)
{
    return g_playthrough[0] != 0 ? re_state_get(g_playthrough, "researchall", g_research_sub) != 0 : g_research_sub;
}

static int research_one(int company)
{
    char key[40];
    snprintf(key, sizeof key, "researchc%d", company);
    return g_playthrough[0] != 0 && re_state_get(g_playthrough, key, 0) != 0;
}

/* 1 when a member of the household sits on the board of a listed company: its research is kept fresh for nothing. */
static int research_on_board(BYTE *listed)
{
    int members[16], household[16];
    int seats = g_research_board ? re_board_members(listed, members, 16) : 0;
    int people = seats > 0 ? re_board_household(listed, household, 16) : 0;
    for (int a = 0; a < seats; a++)
        for (int b = 0; b < people; b++)
            if (members[a] == household[b])
                return 1;
    return 0;
}

/* 1 when the mod keeps that company's research fresh for a fee: not one of the board's, and subscribed by itself
 * or with every company. */
static int research_subscribed(BYTE *listed)
{
    return !research_on_board(listed) && (research_all() || research_one(*(const int *)(listed + RE_COMPANY_ID)));
}

typedef void(__thiscall *research_fn)(void *market, int company);

/* A fee once paid stays paid (REQUEST.md [48], note m28 "Q5"). Which companies are subscribed is the playthrough's,
 * the research and the cash are the save's: a save from before the fee was taken, loaded again, has the cash back
 * and what the research showed is still true, since share prices run the same course again. So the playthrough
 * also keeps in which months a company's fee was paid (re_stock_paid_*), and after a load and after a month start a
 * company that is on record as paid for the month and has no research of that month is researched again and its
 * fee taken again, whatever the subscription is set to now. Cash that is short becomes the game's debt, as for the
 * month start's fee. A month paid before the trade lock moved the price stream (a rollback beyond its cap) is of
 * another course of prices and is not asked for again. [stocks] researchFeeKept=0 leaves the fee with the save. */
static int g_research_fee_kept = 1;

static void research_paid(int company, int month)
{
    char key[40];
    if (!g_research_fee_kept || g_playthrough[0] == 0)
        return;
    snprintf(key, sizeof key, "researchpaid%d", company);
    re_state_set(g_playthrough, key,
                 re_stock_paid_add(re_state_get(g_playthrough, key, 0), (int)re_state_get(g_playthrough, "guardGeneration", 0), month));
}

/* `month`: the month the game is in, as a research of now is stamped. Returns 1 when a fee was taken. */
static int research_kept(void *main_obj, const char *when, int month)
{
    static BYTE *listed[LISTED_COMPANIES];
    static re_household h; /* too large for the game's stack */
    void *market = re_game_market(main_obj), *finance = re_game_finance(main_obj);
    int count = market != NULL ? re_game_companies(market, listed, LISTED_COMPANIES) : -1, due = 0;
    long long each = job_fee(main_obj, RE_JOB_BANK_ANALYST, g_research_hours);
    if (!g_research_fee_kept || g_playthrough[0] == 0 || count < 0 || finance == NULL || month < 0 || each <= 0)
        return 0; /* nothing to keep where the research costs nothing */
    int course = (int)re_state_get(g_playthrough, "guardGeneration", 0);
    for (int i = 0; i < count; i++) {
        char key[40];
        snprintf(key, sizeof key, "researchpaid%d", *(const int *)(listed[i] + RE_COMPANY_ID));
        if (*(const int *)(listed[i] + RE_COMPANY_RESEARCH_MONTH) != month && !research_on_board(listed[i]) &&
            re_stock_paid_has(re_state_get(g_playthrough, key, 0), course, month))
            listed[due++] = listed[i];
    }
    if (due == 0)
        return 0;
    long long fee = each * due;
    snapshot(main_obj, &h);
    long long owed = h.ok_balance && h.cash < fee ? fee - (h.cash > 0 ? h.cash : 0) : 0;
    for (int i = 0; i < due; i++) {
        ((research_fn)AT(RE_VA_RESEARCH))(market, *(const int *)(listed[i] + RE_COMPANY_ID));
        *(int *)(listed[i] + RE_COMPANY_RESEARCH_MONTH) = month; /* at a month start the calendar can still show the month before */
    }
    ((money_fn)AT(RE_VA_CHANGE_MONEY))(finance, (unsigned)-fee, (int)(-fee >> 32), RE_TAG_FEES, 1, 0, 0);
    re_log("research (%s, month %d): %d company(ies) whose fee of this month was paid before and whose research of this month is not "
           "in the game are researched again at $%.2f each: $%.2f taken again, cash before it $%.2f, $%.2f of it more than the cash and "
           "left to the game as a debt",
           when, month, due, dollars(each), dollars(fee), h.ok_balance ? dollars(h.cash) : 0.0, dollars(owed));
    const char *prefix = T(RE_MSG_RESEARCH_KEPT_PREFIX);
    char text[400], amount[64], debt[64];
    if (owed > 0)
        snprintf(text, sizeof text, T(RE_MSG_RESEARCH_KEPT_SHORT), prefix, due, money(main_obj, fee, amount), money(main_obj, owed, debt));
    else
        snprintf(text, sizeof text, T(RE_MSG_RESEARCH_KEPT), prefix, due, money(main_obj, fee, amount));
    panel_line(main_obj, prefix, text, "UIIconStockMarket", 1);
    snprintf(text, sizeof text, T(RE_MSG_RESEARCH_KEPT_TICK), money(main_obj, fee, amount), due);
    re_game_post_message(main_obj, text);
    return 1;
}

static void research_refresh(void *main_obj, const re_household *h)
{
    static BYTE *listed[LISTED_COMPANIES];
    static unsigned char pays[LISTED_COMPANIES];
    void *market = re_game_market(main_obj), *finance = re_game_finance(main_obj);
    int count = market != NULL ? re_game_companies(market, listed, LISTED_COMPANIES) : -1;
    if (count < 0) {
        re_log("research: the market could not be read, nothing researched");
        return;
    }
    int on_board = 0, subscribed = 0, done = 0, all = research_all();
    for (int i = 0; i < count; i++) {
        int sits = research_on_board(listed[i]);
        pays[i] = (unsigned char)research_subscribed(listed[i]);
        on_board += sits;
        subscribed += pays[i];
        if (!sits && !pays[i])
            listed[i] = NULL;
    }
    long long each = job_fee(main_obj, RE_JOB_BANK_ANALYST, g_research_hours), fee = each * subscribed;
    int priced = each > 0 || !(g_research_hours > 0.0); /* no hours set: free; hours set and no wage read: not done */
    int paid = subscribed > 0 && priced && finance != NULL;
    long long owed = paid && h->ok_balance && h->cash < fee ? fee - (h->cash > 0 ? h->cash : 0) : 0; /* becomes a debt */
    for (int i = 0; i < count; i++)
        if (listed[i] != NULL && (!pays[i] || paid)) {
            ((research_fn)AT(RE_VA_RESEARCH))(market, *(const int *)(listed[i] + RE_COMPANY_ID));
            /* Where the calendar still shows the month that just ended, the game has stamped the research with it;
             * what was read is the state the new month starts with, so it is of the new month. */
            int *stamp = (int *)(listed[i] + RE_COMPANY_RESEARCH_MONTH);
            if (g_month_ended >= 0 && *stamp == g_month_ended)
                *stamp = g_month_ended + 1;
            if (pays[i] && fee > 0)
                research_paid(*(const int *)(listed[i] + RE_COMPANY_ID), *stamp);
            done++;
        }
    if (paid && fee > 0)
        ((money_fn)AT(RE_VA_CHANGE_MONEY))(finance, (unsigned)-fee, (int)(-fee >> 32), RE_TAG_FEES, 1, 0, 0);
    re_log("research: %d of %d listed companies researched again: %d with a household member on the board, %d under the "
           "subscription (every company: %s) at $%.2f each, %.1f hours of a bank analyst (%s); the month that ended is period %d, "
           "the calendar shows %d, cash before the fee $%.2f, $%.2f of the fee more than the cash and left to the game as a debt",
           done, count, on_board, subscribed, all ? "on" : "off", dollars(each), g_research_hours,
           subscribed == 0 ? "nothing to pay" : paid ? "paid" : priced ? "the finance object could not be read, not done"
                                                                       : "the wage could not be read, not done",
           g_month_ended, h->ok_calendar ? h->period : -1, h->ok_balance ? dollars(h->cash) : 0.0, dollars(owed));
    if (paid) {
        const char *prefix = T(RE_MSG_RESEARCH_PREFIX);
        char text[280], amount[64], debt[64];
        if (owed > 0)
            snprintf(text, sizeof text, T(RE_MSG_RESEARCH_SHORT), prefix, subscribed, money(main_obj, fee, amount), money(main_obj, owed, debt));
        else
            snprintf(text, sizeof text, T(RE_MSG_RESEARCH_PAID), prefix, subscribed, money(main_obj, fee, amount));
        panel_line(main_obj, prefix, text, "UIIconStockMarket", 1);
    }
}

/* trace: the listed companies against those of the look before, by id. The game ends a company that needs a second
 * emergency measure within six months and lists a new one in its place (note b12 "Q6"); a household can list one
 * itself. Called after a load and when a month has started (REQUEST.md [47]: do the mod's stock features hold for a
 * company that is new?). */
static void listing_trace(void *main_obj, const char *when)
{
    static BYTE *listed[LISTED_COMPANIES];
    static int before[LISTED_COMPANIES], befores = -1;
    void *market = re_game_market(main_obj);
    int count = market != NULL ? re_game_companies(market, listed, LISTED_COMPANIES) : -1, now[LISTED_COMPANIES], added = 0, gone = 0;
    if (count < 0)
        return;
    for (int i = 0; i < count; i++) {
        char name[96] = "";
        int known = befores < 0;
        now[i] = *(const int *)(listed[i] + RE_COMPANY_ID);
        for (int k = 0; k < befores && !known; k++)
            known = before[k] == now[i];
        if (known)
            continue;
        added++;
        re_game_company_name(listed[i], name, sizeof name);
        re_log("stocks (trace, %s): company %d '%s' is listed now and was not at the look before", when, now[i], name);
    }
    for (int k = 0; k < befores; k++) {
        int still = 0;
        for (int i = 0; i < count && !still; i++)
            still = now[i] == before[k];
        if (!still) {
            gone++;
            re_log("stocks (trace, %s): company %d is not listed any more", when, before[k]);
        }
    }
    re_log("stocks (trace, %s): %d listed companies, %d new and %d gone since the look before", when, count, added, gone);
    memcpy(before, now, sizeof(int) * (size_t)count);
    befores = count;
}

/* trace: `[trace] endCompany=1` has the game end a listed company at the next month end, for a test of the above.
 * Right after the first load the first company the household holds shares of - the first listed one when it holds
 * none - is written as having taken an emergency measure this month, with a share price of four tenths of its equity
 * a share: the month end's own test (price under 0.75 of the equity, a measure within the last six months) then
 * ends it, and the game lists a new company. */
static int g_trace_end_company;

static void end_company_trace(void *main_obj)
{
    static BYTE *listed[LISTED_COMPANIES];
    static re_holding held[HELD_COMPANIES];
    static int done;
    void *market = re_game_market(main_obj);
    int now = re_game_period(main_obj);
    if (done || !g_trace_end_company || market == NULL || now < 0)
        return;
    done = 1;
    int count = re_game_companies(market, listed, LISTED_COMPANIES), holdings = re_game_holdings(market, held, HELD_COMPANIES), shares = 0;
    BYTE *company = NULL;
    for (int i = 0; i < holdings && company == NULL; i++)
        if (held[i].shares > 0 && (company = re_game_listed_company(market, held[i].company)) != NULL)
            shares = held[i].shares;
    if (company == NULL && count > 0)
        company = listed[0];
    char name[96] = "";
    if (company == NULL || !re_game_company_name(company, name, sizeof name)) {
        re_log("stocks (trace): endCompany: no listed company could be read, nothing written");
        return;
    }
    long long *price = (long long *)(company + RE_COMPANY_PRICE), equity = *(const long long *)(company + RE_COMPANY_EQUITY), was = *price;
    *price = equity * 4 / 10;
    *(int *)(company + RE_COMPANY_DESPERATE_MONTH) = now;
    re_log("stocks (trace): endCompany: company %d '%s', of which the household holds %d share(s): share price $%.2f written as $%.2f "
           "(0.4 of its equity a share, $%.2f) and an emergency measure in month %d: the game's next month end ends it",
           *(const int *)(company + RE_COMPANY_ID), name, shares, dollars(was), dollars(*price), dollars(equity), now);
}

/* The click on a company in the stock window's list (FUN_007abba0). The game asks twice whether a Ctrl key is held
 * (left, right) - then the company becomes a shortcut ("Ctrl-click to add a shortcut") - and, when none is, shows
 * the company. The mod stands in the two questions and in the showing. A Ctrl key with a Shift key is answered
 * "no Ctrl" and remembered. Where the company is shown, a Shift key that is held switches the subscription: of
 * every listed company when a Ctrl key was held too, else of that company. The left side of the window is then
 * built anew, so that the rows and their hover texts say what holds now, and the company is shown as after a
 * plain click.
 *
 * Switching on works at once (REQUEST.md [44]): every company it covers that has no research of this month yet is
 * researched now and its fee is taken now. A company researched this month already - by an earlier subscription
 * or by the player's own five hours - costs nothing until the next month start, so switching off and on again
 * within a month is not paid twice; the month of a company's research is in the save. When the cash does not
 * cover what is due now nothing is switched. Switching off takes nothing back: the research of this month stays,
 * the next month start leaves the company alone. Returns 1 when something was switched. */
static int g_research_click_all, g_research_switch_hooked;

static int research_switch(int company, int all)
{
    static BYTE *listed[LISTED_COMPANIES];
    static re_household h; /* too large for the game's stack */
    char key[40], text[260], amount[64], name[64] = "";
    void *market = re_game_market(g_main_obj), *finance = re_game_finance(g_main_obj);
    BYTE *clicked = market != NULL ? re_game_listed_company(market, company) : NULL;
    int now = re_game_period(g_main_obj), count = 1, due = 0;
    long long each = job_fee(g_main_obj, RE_JOB_BANK_ANALYST, g_research_hours);
    if (clicked == NULL || finance == NULL || now < 0 || !re_game_company_name(clicked, name, sizeof name) ||
        (each <= 0 && g_research_hours > 0.0)) {
        re_log("research: a click in the company list: the market, the calendar or the analyst's wage could not be read, nothing switched");
        return 0;
    }
    listed[0] = clicked;
    if (all && (count = re_game_companies(market, listed, LISTED_COMPANIES)) < 0)
        return 0;
    if (!all && (research_on_board(clicked) || research_all())) {
        int board = research_on_board(clicked);
        snprintf(text, sizeof text, T(board ? RE_MSG_RESEARCH_BOARD : RE_MSG_RESEARCH_ALL_HAS), name);
        re_log("research: Shift-click on company %d '%s': %s, nothing switched", company, name,
               board ? "a household member is on its board, its research is free and stays" : "every company is subscribed");
        re_game_post_message(g_main_obj, text);
        return 0;
    }
    if (all ? research_all() : research_one(company)) { /* off */
        for (int i = 0; i < count; i++) {
            int id = *(const int *)(listed[i] + RE_COMPANY_ID);
            if (research_one(id)) {
                snprintf(key, sizeof key, "researchc%d", id);
                re_state_set(g_playthrough, key, 0);
            }
        }
        if (all)
            re_state_set(g_playthrough, "researchall", 0);
        snprintf(text, sizeof text, T(all ? RE_MSG_RESEARCH_ALL_OFF : RE_MSG_RESEARCH_ONE_OFF), name);
        if (all)
            re_log("research: Ctrl+Shift-click in the company list: every subscription is off");
        else
            re_log("research: Shift-click on company %d '%s': its subscription is off", company, name);
        re_game_post_message(g_main_obj, text);
        return 1;
    }
    for (int i = 0; i < count; i++) /* on: what has no research of this month */
        due += !research_on_board(listed[i]) && *(const int *)(listed[i] + RE_COMPANY_RESEARCH_MONTH) != now;
    long long fee = each * due;
    snapshot(g_main_obj, &h);
    if (fee > 0 && (!h.ok_balance || h.cash < fee)) {
        snprintf(text, sizeof text, T(RE_MSG_RESEARCH_NO_CASH), money(g_main_obj, fee, amount));
        re_log("research: %s: %d company(ies) to research now for $%.2f, cash $%.2f: not switched on",
               all ? "Ctrl+Shift-click in the company list" : "Shift-click on a company", due, dollars(fee), h.ok_balance ? dollars(h.cash) : 0.0);
        re_game_post_message(g_main_obj, text);
        return 0;
    }
    for (int i = 0; i < count; i++)
        if (!research_on_board(listed[i]) && *(const int *)(listed[i] + RE_COMPANY_RESEARCH_MONTH) != now) {
            ((research_fn)AT(RE_VA_RESEARCH))(market, *(const int *)(listed[i] + RE_COMPANY_ID));
            if (fee > 0)
                research_paid(*(const int *)(listed[i] + RE_COMPANY_ID), now);
        }
    if (fee > 0)
        ((money_fn)AT(RE_VA_CHANGE_MONEY))(finance, (unsigned)-fee, (int)(-fee >> 32), RE_TAG_FEES, 1, 0, 0);
    snprintf(key, sizeof key, all ? "researchall" : "researchc%d", company);
    re_state_set(g_playthrough, key, 1);
    if (all)
        snprintf(text, sizeof text, T(RE_MSG_RESEARCH_ALL_ON), due, money(g_main_obj, fee, amount));
    else
        snprintf(text, sizeof text, T(due ? RE_MSG_RESEARCH_ONE_ON : RE_MSG_RESEARCH_ONE_ON_DONE), name, money(g_main_obj, fee, amount));
    if (all)
        re_log("research: Ctrl+Shift-click in the company list: every company is subscribed; %d researched now for $%.2f, period %d", due,
               dollars(fee), now);
    else
        re_log("research: Shift-click on company %d '%s': its subscription is on; %d researched now for $%.2f, period %d", company, name,
               due, dollars(fee), now);
    re_game_post_message(g_main_obj, text);
    return 1;
}

/* 1 = the game's "Research Stock" button is not made for that company: the mod researches it every month and has
 * done so this month, so the button would take five of the player's hours for what is there already. A company the
 * mod is to research that has no research of this month keeps the button (review R4-02): the subscription is kept by
 * playthrough and the research is in the save, so a save from before the subscription was switched on has the one
 * without the other until the next month start. */
__attribute__((force_align_arg_pointer)) static int research_button_none(int company)
{
    static int lines;
    void *market = g_main_obj != NULL ? re_game_market(g_main_obj) : NULL;
    BYTE *listed = market != NULL ? re_game_listed_company(market, company) : NULL;
    if (!g_research_switch_hooked || g_playthrough[0] == 0 || listed == NULL || !(research_on_board(listed) || research_subscribed(listed)))
        return 0;
    int of = *(const int *)(listed + RE_COMPANY_RESEARCH_MONTH), now = re_game_period(g_main_obj), none = now >= 0 && of == now;
    if (g_on_trace && lines++ < 60)
        re_log("stocks: the \"Research Stock\" button of company %d, which the mod researches: %s (its research is of period %d, now is %d)",
               company, none ? "left out" : "made, there is no research of this month", of, now);
    return none;
}

__attribute__((force_align_arg_pointer)) static int __fastcall stock_row_ctrl_wrapper(void *keys, void *unused, int *key)
{
    typedef int(__thiscall * held_fn)(void *keys, int *key);
    (void)unused;
    int held = ((held_fn)AT(RE_VA_KEY_HELD))(keys, key);
    if (re_readable(key, 4) && *key == RE_KEY_CTRL)
        g_research_click_all = 0; /* the first question of a click */
    if (held && g_research_switch_hooked && re_game_keys_shift(keys)) {
        g_research_click_all = 1;
        return 0;
    }
    return held;
}

__attribute__((force_align_arg_pointer)) static void __fastcall stock_row_show_wrapper(void *window, void *unused, int company)
{
    typedef void(__thiscall * show_fn)(void *window, int company);
    (void)unused;
    typedef void(__fastcall * left_fn)(void *window);
    int all = g_research_click_all;
    g_research_click_all = 0;
    if (g_research_switch_hooked && g_main_obj != NULL && g_playthrough[0] != 0 && (all || re_game_shift_held(window)) &&
        research_switch(company, all))
        ((left_fn)AT(RE_VA_STOCK_LEFT))(window); /* the rows and their hover texts are made when the list is built */
    ((show_fn)AT(RE_VA_STOCK_SHOW))(window, company);
}

/* The two lines under the mod's lines in the hover text of a company of the list: who researches it, with the click
 * that switches it, and what a Ctrl+Shift-click would do now. */
static const char *research_hover(int company)
{
    static char lines[160];
    void *market = g_main_obj != NULL ? re_game_market(g_main_obj) : NULL;
    BYTE *listed = market != NULL ? re_game_listed_company(market, company) : NULL;
    if (!g_research_switch_hooked || listed == NULL)
        return NULL;
    int all = research_all();
    snprintf(lines, sizeof lines, "%s\n%s",
             T(research_on_board(listed) ? RE_MSG_HOVER_SUB_BOARD
               : all                     ? RE_MSG_HOVER_SUB_ALL
               : research_one(company)   ? RE_MSG_HOVER_SUB_ON
                                         : RE_MSG_HOVER_SUB_OFF),
             T(all ? RE_MSG_HOVER_ALL_OFF : RE_MSG_HOVER_ALL_ON));
    return lines;
}

/* The row "Last Researched" of a company's tab: behind the date, that the mod keeps the research fresh. The game's
 * research button is not made for such a company once it has this month's research (research_button_none), and
 * this says why. */
static void stock_researched_row(const re_text_regs *regs, void *value)
{
    const BYTE *const *slot = (const BYTE *const *)(UINT_PTR)(regs->ebp + RE_STOCK_RESEARCH_FRAME_COMPANY);
    void *market = g_main_obj != NULL ? re_game_market(g_main_obj) : NULL;
    if (!g_research_switch_hooked || g_playthrough[0] == 0 || market == NULL || !re_readable(slot, 4) ||
        !re_readable(*slot, RE_COMPANY_ID + 4) || !re_readable(value, 0x18))
        return;
    BYTE *listed = re_game_listed_company(market, *(const int *)(*slot + RE_COMPANY_ID));
    const char *more = listed == NULL                ? NULL
                       : research_on_board(listed)   ? T(RE_MSG_ROW_SUB_BOARD)
                       : research_subscribed(listed) ? T(RE_MSG_ROW_SUB_ON)
                                                     : NULL;
    const BYTE *s = (const BYTE *)value;
    unsigned size = *(const unsigned *)(s + 0x10), capacity = *(const unsigned *)(s + 0x14);
    const char *shown = capacity >= 16 ? *(const char *const *)s : (const char *)s;
    char text[160];
    if (more == NULL || size + strlen(more) >= sizeof text || !re_readable(shown, size))
        return;
    memcpy(text, shown, size);
    memcpy(text + size, more, strlen(more));
    ((assign_fn)AT(RE_VA_STR_ASSIGN))(value, text, size + (unsigned)strlen(more));
}

/* A row of the stock window's company list (REQUEST.md [44]): behind the company's name the price against the
 * month before, for a researched company the fair price against the price, and that the mod keeps its research
 * fresh; the title of a researched company is blue, the blue of the game's coloured texts. The game's maker of
 * such a row (FUN_00610940) copies the name out of its own copy of the company into the title's argument, and the
 * mod stands in that copy. The title's label does not read the game's colour marks, so the colour is set on the
 * label once the row is made (stock_item_made_wrapper). The maker has two more users, which are left alone: the
 * maker's frame has the address it returns to. */
static int g_item_blue; /* the row being made is of a researched company */

__attribute__((force_align_arg_pointer)) static void *__fastcall stock_item_name_wrapper(void *title, void *unused, const BYTE *name)
{
    typedef void *(__thiscall * copy_fn)(void *self, const void *from);
    static int lines;
    (void)unused;
    g_item_blue = 0;
    void *result = ((copy_fn)AT(RE_VA_STR_COPY))(title, name);
    const BYTE *maker = *(const BYTE *const *)__builtin_frame_address(0); /* the frame of the row's maker */
    const BYTE *company = name - RE_COMPANY_NAME, *s = (const BYTE *)title;
    void *market = g_main_obj != NULL ? re_game_market(g_main_obj) : NULL;
    int now = g_main_obj != NULL ? re_game_period(g_main_obj) : -1;
    if (!re_readable(maker, 8) || va_of(*(const unsigned *)(maker + 4)) != RE_VA_STOCK_LIST_ITEM_RET ||
        name != maker + RE_STOCK_ITEM_FRAME_NAME || market == NULL || now < 0 || !re_readable(company, RE_COMPANY_SIZE) ||
        !re_readable(s, 0x18))
        return result;
    unsigned size = *(const unsigned *)(s + 0x10), capacity = *(const unsigned *)(s + 0x14);
    const char *shown = capacity >= 16 ? *(const char *const *)s : (const char *)s;
    re_stock stock;
    re_stock_view v;
    char text[240];
    int id = *(const int *)(company + RE_COMPANY_ID);
    if (size == 0 || size > 100 || !re_readable(shown, size))
        return result;
    re_stock_read(company, &stock);
    re_stock_view_of(&stock, now, &v);
    long long before = re_game_company_price_at(market, id, now - 1);
    v.has_change = before > 0 && stock.price > 0;
    v.change = v.has_change ? (double)stock.price / (double)before - 1.0 : 0.0;
    BYTE *listed = g_research_switch_hooked && g_playthrough[0] != 0 ? re_game_listed_company(market, id) : NULL;
    int kept = listed == NULL                ? RE_STOCK_KEPT_NOT
               : research_on_board(listed)   ? RE_STOCK_KEPT_BOARD
               : research_subscribed(listed) ? RE_STOCK_KEPT_SUBSCRIBED
                                             : RE_STOCK_KEPT_NOT;
    int researched = stock.research_month != 0 && v.has_pbr;
    memcpy(text, shown, size);
    unsigned added = re_stock_item(&v, researched, kept, g_lang, text + size, (unsigned)sizeof text - size);
    g_item_blue = researched;
    if (added)
        ((assign_fn)AT(RE_VA_STR_ASSIGN))(title, text, size + added);
    if (g_on_trace && lines++ < 400)
        re_log("stocks: row of company %d: '%.*s'%s", id, (int)(size + added), text, researched ? ", blue" : "");
    return result;
}

/* The row is made: the game hands the item on, and the title of a researched company gets its colour. */
__attribute__((force_align_arg_pointer)) static void __fastcall stock_item_made_wrapper(void *ui, void *unused, BYTE *item, int *count, int a,
                                                                                         int b)
{
    typedef void(__thiscall * made_fn)(void *ui, BYTE *item, int *count, int a, int b);
    typedef void(__thiscall * colour_fn)(void *node, const BYTE *rgb);
    static const BYTE blue[4] = {91, 91, 214, 0}; /* what the game's "@BLUE@" gives a text */
    (void)unused;
    int tint = g_item_blue;
    g_item_blue = 0;
    void *title = tint && re_readable(item, RE_ITEM_TITLE + 4) ? *(void **)(item + RE_ITEM_TITLE) : NULL;
    void **functions = re_readable(title, 4) ? *(void ***)title : NULL;
    if (re_readable(functions, RE_NODE_COLOUR + 4))
        ((colour_fn)functions[RE_NODE_COLOUR / 4])(title, blue);
    ((made_fn)AT(RE_VA_ITEM_MADE))(ui, item, count, a, b);
}

/* The order of the stock window's company list (REQUEST.md [43], note m23). The window's code copies the market's
 * companies into a vector and has one of the game's functions put it in order: by company type, by market
 * capitalisation, by earnings a share. The mod stands in those three calls and makes its own order: by how far the
 * price is under the fair price, by how far over it, by the change of the price in a month. The fair price is the
 * one the hover text shows, so only a company the player has had researched is ranked by it ([27]); the others
 * follow as they stood. The copies are moved with the game's own copy, assignment and destructor of a company,
 * the three its sort moves them with. */
#define SORT_ROWS 160
static void stock_sort(BYTE *first, BYTE *last, int by)
{
    typedef void *(__thiscall * copy_fn)(void *self, const void *from);
    typedef void(__fastcall * drop_fn)(void *self);
    static BYTE spare[SORT_ROWS * RE_COMPANY_SIZE];
    static re_stock_rank rank[SORT_ROWS];
    static int told, lines;
    int order[SORT_ROWS], n = (int)((last - first) / RE_COMPANY_SIZE), moved = 0;
    int now = g_main_obj != NULL ? re_game_period(g_main_obj) : -1;
    void *market = g_main_obj != NULL ? re_game_market(g_main_obj) : NULL;
    if (n < 2 || n > SORT_ROWS || now < 0 || market == NULL || !re_readable(first, (unsigned)n * RE_COMPANY_SIZE)) {
        if (n >= 2 && told++ < 5)
            re_log("stocks: the list of %d companies is left in the order of their listing: %s", n,
                   n > SORT_ROWS ? "more than the mod puts in order" : "the calendar or the market could not be read");
        return;
    }
    for (int i = 0; i < n; i++) {
        const BYTE *company = first + i * RE_COMPANY_SIZE;
        re_stock s;
        re_stock_view v;
        re_stock_read(company, &s);
        re_stock_view_of(&s, now, &v);
        long long before = re_game_company_price_at(market, *(const int *)(company + RE_COMPANY_ID), now - 1);
        rank[i].researched = s.research_month != 0 && v.has_pbr;
        rank[i].gap = v.gap;
        rank[i].has_change = before > 0 && s.price > 0;
        rank[i].change = rank[i].has_change ? (double)s.price / (double)before - 1.0 : 0.0;
        rank[i].cap = (double)s.price * (double)*(const long long *)(company + RE_COMPANY_SHARES);
    }
    re_stock_order(rank, n, by, order);
    if (g_on_trace && lines++ < 60) {
        char text[1400];
        int at = 0;
        for (int i = 0; i < n && at >= 0 && (size_t)at + 40 < sizeof text; i++) {
            const re_stock_rank *r = &rank[order[i]];
            int id = *(const int *)(first + order[i] * RE_COMPANY_SIZE + RE_COMPANY_ID);
            if (by == RE_STOCK_BY_CAP) /* price x shares, as two numbers: the hover line of a row has the price */
                at += snprintf(text + at, sizeof text - (size_t)at, " %d($%.2fx%lld)", id,
                               (double)*(const long long *)(first + order[i] * RE_COMPANY_SIZE + RE_COMPANY_PRICE) / 100.0,
                               *(const long long *)(first + order[i] * RE_COMPANY_SIZE + RE_COMPANY_SHARES));
            else if (by == RE_STOCK_BY_CHANGE ? r->has_change : r->researched)
                at += snprintf(text + at, sizeof text - (size_t)at, " %d(%+.2f%%)", id, 100.0 * (by == RE_STOCK_BY_CHANGE ? r->change : r->gap));
            else
                at += snprintf(text + at, sizeof text - (size_t)at, " %d(-)", id);
        }
        re_log("stocks: the list in the mod's order %d (0 cheapest first, 1 dearest first, 2 by change, 3 by capitalisation), %d "
               "companies:%s",
               by, n, text);
    }
    for (int i = 0; i < n; i++)
        moved += order[i] != i;
    if (!moved)
        return;
    for (int i = 0; i < n; i++)
        ((copy_fn)AT(RE_VA_COMPANY_CLONE))(spare + i * RE_COMPANY_SIZE, first + i * RE_COMPANY_SIZE);
    for (int i = 0; i < n; i++)
        if (order[i] != i)
            ((copy_fn)AT(RE_VA_COMPANY_ASSIGN))(first + i * RE_COMPANY_SIZE, spare + order[i] * RE_COMPANY_SIZE);
    for (int i = 0; i < n; i++)
        ((drop_fn)AT(RE_VA_COMPANY_DROP))(spare + i * RE_COMPANY_SIZE);
}

/* In place of the game's three sorts: the first copy in ecx, the end in edx. The count and the flag the game
 * pushes stay on the stack for the caller, which removes them itself. */
__attribute__((force_align_arg_pointer)) static void __fastcall stock_sort_cheap(BYTE *first, BYTE *last)
{
    stock_sort(first, last, RE_STOCK_BY_CHEAP);
}

__attribute__((force_align_arg_pointer)) static void __fastcall stock_sort_dear(BYTE *first, BYTE *last)
{
    stock_sort(first, last, RE_STOCK_BY_DEAR);
}

__attribute__((force_align_arg_pointer)) static void __fastcall stock_sort_change(BYTE *first, BYTE *last)
{
    stock_sort(first, last, RE_STOCK_BY_CHANGE);
}

/* The second button, "by listing date" (REQUEST.md [48]): for it the window's code calls nothing and goes on with
 * the copies as the market has them, by company id. The jump that goes on comes here: the first copy is in the
 * frame where the three calls take it from, the end in edi as for them; then on to where the jump went. */
void stock_sort_cap(BYTE *first, BYTE *last);
__attribute__((force_align_arg_pointer)) void stock_sort_cap(BYTE *first, BYTE *last)
{
    stock_sort(first, last, RE_STOCK_BY_CAP);
}
void *stock_sort_cap_on;
void stock_sort_cap_hook(void);
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _stock_sort_cap_hook\n"
        "_stock_sort_cap_hook:\n"
        "  push edi\n"
        "  push dword ptr [ebp - 0x128]\n"
        "  call _stock_sort_cap\n"
        "  add esp, 8\n"
        "  jmp dword ptr [_stock_sort_cap_on]\n"
        ".att_syntax prefix\n");

/* A company's chart (REQUEST.md [43], note m23 section 3). The game's chart window draws every series of the
 * statement it is given, and for a company those are the share price, the equity, the dividend and the earnings
 * a share on one scale. Where the window ticks every box after building them, the mod leaves only the price
 * ticked in a chart that has the price among its series; the other three are a click away, and the box for all
 * of them starts empty so that the first click on it ticks them all. Other charts are left as they are. */
/* More of the charts (REQUEST.md [44], note m23 section 5):
 *   - the three buttons of the economy (growth, the central bank's rate, inflation) open one chart with seventeen
 *     series; it opens with the series of the button alone ([stocks] chartEconomyOne);
 *   - the game draws lines in dark greens and browns chosen by chance; every series gets a colour of a fixed list;
 *     a page turn of the boxes ticks every box again and so loses what was unticked - the boxes come back as they
 *     were, and the box for all is for the series of every page; the time button has more steps ([stocks]
 *     chartFixes for the three);
 *   - the chart of all share prices draws prices of $16 and of $700 on one scale; every price is shown against
 *     what it was in the first month of the picture, as 100 ([stocks] chartIndex).
 * A chart is being opened while the game builds its boxes from FUN_005a81f0; the page turn of the boxes comes to the
 * same builder by another way. */
/* Still more (REQUEST.md [45], note m23 section 7):
 *   - the household's wealth has nine series on one scale and opens with net worth, cash and debt; the stock index
 *     with its growth over a year (the month's jumps about and hides it); all share prices with the companies the
 *     household holds shares of, when one of them has its box on the page shown; the economy's three charts with the
 *     last five years where their window showed more ([stocks] chartOpenWith for the four);
 *   - the game draws a line one pixel wide with soft edges, paler than the colour of its name: every piece is drawn
 *     as a segment with a radius ([stocks] chartLineWidth);
 *   - the label of a series' box has the series' value of the last month behind the name ([stocks] chartValues). */
#define CHART_PAGE 32 /* boxes on a page; the game has 18 */
#define CHART_ECONOMY_MONTHS 60
static void *g_chart_opening; /* the chart whose boxes are being built because it is opened */
static int g_chart_opened_by; /* the series of the economy's button that opens it, 0 for any other chart */
static int g_chart_keep[CHART_PAGE], g_chart_keeps; /* the series another chart keeps ticked when it opens; none: all */
static const char *g_chart_keep_what;

/* where the function that opens a chart was called from, as an address of the game's file; `open` is its frame */
static unsigned chart_opener(const BYTE *open)
{
    return re_readable(open, 8) ? va_of(*(const unsigned *)(open + 4)) : 0;
}

static int chart_economy(unsigned from)
{
    return from == RE_VA_CALL_CHART_OPEN_GROWTH + 5      ? RE_STAT_GROWTH
           : from == RE_VA_CALL_CHART_OPEN_RATE + 5      ? RE_STAT_BANK_RATE
           : from == RE_VA_CALL_CHART_OPEN_INFLATION + 5 ? RE_STAT_INFLATION
                                                         : 0;
}

/* what the chart opened from `from` keeps ticked, into g_chart_keep */
static void chart_keep_for(unsigned from)
{
    static re_holding held[CHART_PAGE];
    g_chart_keeps = 0;
    if (!g_chart_open)
        return;
    if (from == RE_VA_CALL_CHART_OPEN_WEALTH + 5) {
        g_chart_keep[g_chart_keeps++] = RE_STAT_NET_WORTH;
        g_chart_keep[g_chart_keeps++] = RE_STAT_CASH;
        g_chart_keep[g_chart_keeps++] = RE_STAT_DEBT;
        g_chart_keep_what = "the household's wealth, net worth, cash and debt are ticked";
    } else if (from == RE_VA_CALL_CHART_OPEN_INDEX + 5) {
        g_chart_keep[g_chart_keeps++] = RE_STAT_INDEX_YEAR;
        g_chart_keep_what = "the stock index, its growth over a year alone is ticked";
    } else if (from == RE_VA_CALL_CHART_OPEN_ALL_PRICES + 5) {
        void *market = g_main_obj != NULL ? re_game_market(g_main_obj) : NULL;
        int count = market != NULL ? re_game_holdings(market, held, CHART_PAGE) : 0;
        for (int i = 0; i < count; i++)
            if (held[i].shares > 0)
                g_chart_keep[g_chart_keeps++] = held[i].company;
        g_chart_keep_what = "all share prices, the companies the household holds are ticked";
    }
}

__attribute__((force_align_arg_pointer)) static void __fastcall chart_boxes_wrapper(void *chart)
{
    typedef void(__fastcall * boxes_fn)(void *chart);
    /* told apart on the chart's light blue, the first ones the strongest */
    static const float palette[][4] = {
        {0.84f, 0.15f, 0.16f, 1.0f}, {0.12f, 0.47f, 0.71f, 1.0f}, {0.17f, 0.63f, 0.17f, 1.0f}, {1.00f, 0.50f, 0.05f, 1.0f},
        {0.58f, 0.40f, 0.74f, 1.0f}, {0.55f, 0.34f, 0.29f, 1.0f}, {0.89f, 0.47f, 0.76f, 1.0f}, {0.20f, 0.20f, 0.20f, 1.0f},
        {0.74f, 0.74f, 0.13f, 1.0f}, {0.09f, 0.75f, 0.81f, 1.0f}, {0.00f, 0.00f, 0.55f, 1.0f}, {0.60f, 0.00f, 0.30f, 1.0f},
        {0.50f, 0.00f, 0.00f, 1.0f}, {0.00f, 0.39f, 0.00f, 1.0f}, {0.80f, 0.33f, 0.00f, 1.0f}, {0.29f, 0.00f, 0.51f, 1.0f},
        {0.00f, 0.50f, 0.50f, 1.0f}, {0.50f, 0.50f, 0.00f, 1.0f}, {0.85f, 0.00f, 0.55f, 1.0f}, {0.40f, 0.40f, 0.40f, 1.0f},
        {0.00f, 0.60f, 0.80f, 1.0f}, {0.45f, 0.25f, 0.10f, 1.0f}, {0.30f, 0.55f, 0.45f, 1.0f}, {0.70f, 0.30f, 0.30f, 1.0f},
    };
    static int lines;
    unsigned from = chart_opener(*(const BYTE *const *)__builtin_frame_address(0)); /* the frame of FUN_005a81f0 */
    g_chart_opened_by = chart_economy(from);
    chart_keep_for(from);
    int series = g_chart_fixes ? re_game_chart_colours(chart, palette, (int)(sizeof palette / sizeof palette[0])) : -1;
    int months = g_chart_caps && g_chart_opened_by != 0 ? re_game_chart_cap(chart, CHART_ECONOMY_MONTHS) : -1;
    if (g_on_trace && lines++ < 60)
        re_log("chart: opened from 0x%08x, kind %d: %d series given a colour of the list; the button's series %d; %d series to keep "
               "ticked; months shown after the economy's cap %d",
               from, re_game_chart_kind(chart), series, g_chart_opened_by, g_chart_keeps, months);
    g_chart_opening = chart;
    ((boxes_fn)AT(RE_VA_CHART_BOXES_FN))(chart);
    g_chart_opening = NULL;
}

/* one of the series to keep has its box on the page shown: the game draws the series of that page only, and a
 * picture with nothing in it is worse than one with everything */
static int chart_keep_shown(const void *chart)
{
    for (int i = 0; i < g_chart_keeps; i++)
        if (re_game_chart_has(chart, g_chart_keep[i]))
            return 1;
    return 0;
}

__attribute__((force_align_arg_pointer)) static void __fastcall chart_tick_wrapper(void *chart, void *unused, void *all_box, int set)
{
    typedef void(__thiscall * tick_fn)(void *chart, void *all_box, int set);
    static int lines;
    (void)unused;
    int series[CHART_PAGE], noted = -1, changed = -1, opening = chart == g_chart_opening;
    unsigned char ticked[CHART_PAGE];
    const char *what = "every series ticked";
    if (!opening && g_chart_fixes)
        noted = re_game_chart_page(chart, series, ticked, CHART_PAGE, 0); /* a page turn: what the record says of this page */
    ((tick_fn)AT(RE_VA_CHART_TICK))(chart, all_box, set);
    if (opening && g_chart_price && (changed = re_game_chart_only(chart, RE_STAT_PRICE)) >= 0)
        what = "a company's, the share price alone is ticked";
    else if (opening && g_chart_one && g_chart_opened_by != 0 && re_game_chart_kind(chart) == 0 && re_game_chart_has(chart, g_chart_opened_by)) {
        changed = re_game_chart_keep(chart, &g_chart_opened_by, 1);
        what = "the economy's, the series of its button alone is ticked";
    } else if (opening && chart_keep_shown(chart)) {
        changed = re_game_chart_set(chart, g_chart_keep, g_chart_keeps);
        what = g_chart_keep_what;
    } else if (opening)
        /* what an earlier chart of this window left unticked in the record, whichever of the settings did it: all
         * share prices opened while shares were held, then again with none held */
        re_game_chart_keep(chart, NULL, 0);
    else if (noted > 0) {
        changed = re_game_chart_page(chart, series, ticked, noted, 1);
        what = "a page turn, the boxes as they were recorded";
    }
    if (changed >= 0)
        ((tick_fn)AT(RE_VA_CHART_TICK))(chart, all_box, 0); /* the boxes follow what is recorded as ticked */
    if (g_on_trace && lines++ < 80)
        re_log("chart: %s: %s (%d)", opening ? "opened" : "boxes built anew", what, changed);
}

/* A click on the box for all: the game sets the boxes of the page shown; the series of the other pages follow. */
__attribute__((force_align_arg_pointer)) static void __fastcall chart_all_wrapper(void *chart, void *unused, void *all_box, int set)
{
    typedef void(__thiscall * tick_fn)(void *chart, void *all_box, int set);
    (void)unused;
    ((tick_fn)AT(RE_VA_CHART_TICK))(chart, all_box, set);
    if (g_chart_fixes && re_readable(chart, RE_CHART_ALL + 1)) {
        int on = ((const BYTE *)chart)[RE_CHART_ALL] != 0, changed = re_game_chart_keep(chart, NULL, on ? 0 : -1);
        if (g_on_trace)
            re_log("chart: the box for all clicked, now %s: %d series of other pages follow", on ? "on" : "off", changed);
    }
}

/* The drawing has copied a map of a month's record for itself: in the chart of all share prices the prices of the
 * copy become what they are against the first month of the picture. The months come in rising order, twice each
 * (the record has two maps); a month below the one before is the start of a new picture. */
static re_chart_base g_index_base[LISTED_COMPANIES];
static int g_index_bases, g_index_month;
static void *g_index_chart;

__attribute__((force_align_arg_pointer)) static void *__fastcall chart_month_wrapper(void *copy, void *unused, void *source, void *spare)
{
    typedef void *(__thiscall * copy_fn)(void *copy, void *source, void *spare);
    static int lines;
    (void)unused;
    void *result = ((copy_fn)AT(RE_VA_MAP_COPY))(copy, source, spare);
    const BYTE *draw = *(const BYTE *const *)__builtin_frame_address(0); /* the frame of the drawing */
    if (!g_chart_index || !re_readable(draw + RE_CHART_DRAW_FRAME_MONTH, (SIZE_T)(-RE_CHART_DRAW_FRAME_MONTH)))
        return result;
    void *chart = *(void *const *)(const void *)(draw + RE_CHART_DRAW_FRAME_CHART);
    int month = *(const int *)(draw + RE_CHART_DRAW_FRAME_MONTH);
    if (re_game_chart_kind(chart) != RE_CHART_KIND_ALL_PRICES)
        return result;
    if (chart != g_index_chart || month < g_index_month) {
        g_index_chart = chart;
        g_index_bases = 0;
        if (g_on_trace && lines++ < 40)
            re_log("chart: all share prices, a picture from month %d on: every price against its first, as 100", month);
    }
    g_index_month = month;
    re_game_month_index(copy, g_index_base, LISTED_COMPANIES, &g_index_bases);
    return result;
}

/* The time button: in place of the game's 2 years -> 5 years -> the most, half a year and a year come first. */
__attribute__((force_align_arg_pointer)) static void __fastcall chart_time_step(void *chart)
{
    static const int steps[] = {6, 12, 24, 60};
    int months = re_game_chart_step(chart, steps, (int)(sizeof steps / sizeof steps[0]));
    if (g_on_trace)
        re_log("chart: the time button: %d months now", months);
}

/* A piece of a chart's line: the game's call of DrawNode::drawLine(from, to, colour) becomes drawSegment with a
 * radius, through the slot the game's own drawing of the axes uses. */
__attribute__((force_align_arg_pointer)) static void __fastcall chart_line_wrapper(void *node, void *unused, const void *from, const void *to,
                                                                                   const void *colour)
{
    typedef void(__thiscall * segment_fn)(void *node, const void *from, const void *to, float radius, const void *colour);
    (void)unused;
    ((segment_fn) * (void **)AT(RE_VA_DRAW_SEGMENT_SLOT))(node, from, to, (float)g_chart_line, colour);
}

/* A number as a chart writes it at its axis, hundredths of what the statement holds; from ten thousand up without
 * the decimals. */
static const char *chart_number(long long value, char out[48])
{
    char digits[32];
    long long size = value < 0 ? -value : value;
    if (size >= 1000000)
        snprintf(out, 48, "%s", usd(value, digits));
    else
        snprintf(out, 48, "%s.%02d", usd(value, digits), (int)(size % 100));
    return out;
}

/* The label of a series' box: the game copies the series' name for it. The series' value of the last month goes
 * behind the name, so that the numbers can be read without pointing at the lines. */
__attribute__((force_align_arg_pointer)) static void *__fastcall chart_label_wrapper(void *label, void *unused, const void *name)
{
    typedef void *(__thiscall * copy_fn)(void *self, const void *from);
    static int lines;
    (void)unused;
    void *result = ((copy_fn)AT(RE_VA_STR_COPY))(label, name);
    const BYTE *boxes = *(const BYTE *const *)__builtin_frame_address(0), *s = (const BYTE *)label; /* the frame of FUN_005aabb0 */
    long long value = 0;
    char text[200], number[48];
    if (!g_chart_values || !re_readable(boxes + RE_CHART_BOX_FRAME_SERIES, (SIZE_T)(-RE_CHART_BOX_FRAME_SERIES)) || !re_readable(s, 0x18) ||
        !re_game_chart_last(*(void *const *)(const void *)(boxes + RE_CHART_BOX_FRAME_CHART), *(const int *)(boxes + RE_CHART_BOX_FRAME_SERIES),
                            &value))
        return result;
    unsigned size = *(const unsigned *)(s + 0x10), capacity = *(const unsigned *)(s + 0x14);
    const char *was = capacity >= 16 ? *(const char *const *)s : (const char *)s;
    if (size == 0 || size > 120 || !re_readable(was, size))
        return result;
    int n = snprintf(text, sizeof text, "%.*s %s", (int)size, was, chart_number(value, number));
    if (n > 0 && (unsigned)n < sizeof text)
        ((assign_fn)AT(RE_VA_STR_ASSIGN))(label, text, (unsigned)n);
    if (g_on_trace && lines++ < 600)
        re_log("chart: the label of series %d: '%.*s'", *(const int *)(boxes + RE_CHART_BOX_FRAME_SERIES), n, text);
    return result;
}

/* The box of a series is named after its tag, and the Korean name of the equity a share says "shares per share"
 * there as it does in the company's tab (stock_row). */
__attribute__((force_align_arg_pointer)) static void *__fastcall chart_name_wrapper(void *info, void *unused, void *name)
{
    typedef void *(__thiscall * name_fn)(void *info, void *string);
    (void)unused;
    const BYTE *s = (const BYTE *)((name_fn)AT(RE_VA_INFO_NAME))(info, name);
    if (re_readable(s, 0x18)) {
        unsigned size = *(const unsigned *)(s + 0x10), capacity = *(const unsigned *)(s + 0x14);
        const char *was = capacity >= 16 ? *(const char *const *)s : (const char *)s;
        if (size == sizeof "주당 주식" - 1 && re_readable(was, size) && memcmp(was, "주당 주식", size) == 0)
            ((assign_fn)AT(RE_VA_STR_ASSIGN))((void *)(UINT_PTR)s, "주당 자본", sizeof "주당 자본" - 1);
    }
    return (void *)(UINT_PTR)s;
}

/* The line about the property for sale that is furthest under its value (REQUEST.md [30], [38], [42], note b17).
 * The game asks for a property what its formula makes it worth, moved by a draw of about 4% either way, and on the
 * screen it rounds the worth down to two digits, so a cheap one is hard to tell there. What a buyer gains is the
 * worth less what the purchase costs: the price, the game's fee of 3% of the price and 0.1% of the worth. A property
 * counts when that gain is at least the share of the settings of its worth AND at least their amount (dollars of
 * the game's first prices, raised with the price level). Only the one with the largest gain is named.
 * The line is written anew whenever the list may have changed: at a month start (the game makes the list anew), after
 * a load, and when a search of the character has ended. When nothing counts any more and the month has a line, the
 * line says so. It costs nothing. */
#define LISTINGS 400
static void property_alert(void *main_obj, const char *why)
{
    static re_listing listed[LISTINGS];
    static re_household h; /* too large for the game's stack */
    unsigned *seed = NULL, *calls = NULL;
    void *unit = NULL;
    unsigned drawn = re_game_market_stream(main_obj, &seed, &calls, &unit) && calls != NULL ? *calls : 0;
    int count = re_game_listings(main_obj, listed, LISTINGS), best = -1;
    if (count < 0) {
        re_log("property (%s): the list of properties for sale could not be read, no line", why);
        return;
    }
    snapshot(main_obj, &h);
    long long least = (long long)(g_property_amount * 100.0 * (h.ok_economy && h.price_index > 0.0 ? h.price_index : 1.0)), most = 0;
    for (int i = 0; i < count; i++) {
        long long gain = (long long)((1.0 - RE_BUY_VALUE_FEE) * (double)listed[i].value - (1.0 + RE_BUY_FEE) * (double)listed[i].price);
        int counts = listed[i].value > 0 && (double)gain >= g_property_gap * (double)listed[i].value && gain >= least;
        if (counts && (best < 0 || gain > most)) {
            best = i;
            most = gain;
        }
        if (g_on_trace && i < 40)
            re_log("property: '%s' asks $%.2f, worth $%.2f: a gain of $%.2f after the purchase fees, %.2f%% of the worth%s",
                   listed[i].address, dollars(listed[i].price), dollars(listed[i].value), dollars(gain),
                   listed[i].value > 0 ? 100.0 * (double)gain / (double)listed[i].value : 0.0, counts ? ", counts" : "");
    }
    re_log("property (%s): %d for sale; a gain after the purchase fees of at least %.1f%% of the worth and $%.2f: %s%s; random "
           "draws of the market stream before and after: %u, %u",
           why, count, 100.0 * g_property_gap, dollars(least), best < 0 ? "none" : "the largest at ", best < 0 ? "" : listed[best].address,
           drawn, calls != NULL ? *calls : 0);
    const char *prefix = T(RE_MSG_PROPERTY_PREFIX);
    char text[320], price[64], gain[64];
    if (best < 0) {
        if (re_game_replace_event(main_obj, prefix, NULL) != 1)
            return; /* the month has no line that could be out of date */
        snprintf(text, sizeof text, T(RE_MSG_PROPERTY_NONE), prefix);
    } else
        snprintf(text, sizeof text, T(RE_MSG_PROPERTY_LINE), prefix, listed[best].address, money(main_obj, most, gain),
                 money(main_obj, listed[best].price, price));
    panel_line(main_obj, prefix, text, "UIIconRealEstateForSale", 1);
}

/* A search of the character for property has ended; it may have put new properties on the market. */
typedef void *(__thiscall *search_fn)(void *restate, void *first, void *second);
__attribute__((force_align_arg_pointer)) static void *__fastcall search_estate_wrapper(void *restate, void *unused, void *first, void *second)
{
    (void)unused;
    void *result = ((search_fn)AT(RE_VA_SEARCH_ESTATE))(restate, first, second);
    if (g_on_property && g_main_obj != NULL)
        property_alert(g_main_obj, "search for a kind of property");
    return result;
}

__attribute__((force_align_arg_pointer)) static void *__fastcall search_site_wrapper(void *restate, void *unused, void *first, void *second)
{
    (void)unused;
    void *result = ((search_fn)AT(RE_VA_SEARCH_SITE))(restate, first, second);
    if (g_on_property && g_main_obj != NULL)
        property_alert(g_main_obj, "search for a place");
    return result;
}

/* The month's line about businesses that lose work efficiency to missing assets (REQUEST.md [34], note b19 "A").
 * The game multiplies everybody's work in a business by two stored factors: one for the furnishings every job uses
 * (not below 0.5) and one for the assets of the job (not below 0.35). Both fall when the business grows - more
 * hours expected, more staff - without an asset being lost, and no screen says so until the business window is
 * opened. The line names up to three businesses, the worst first, with the factors that are under 1. */
#define OWN_FIRMS 64
#define FIT_NAMED 3
static void fit_alert(void *main_obj, const char *why)
{
    static re_firm_fit firm[OWN_FIRMS];
    float worst[OWN_FIRMS];
    int count = re_game_firm_fits(main_obj, firm, OWN_FIRMS), low = 0, named = 0;
    if (count < 0) {
        re_log("business (%s): the household's businesses could not be read, no line about work efficiency", why);
        return;
    }
    for (int i = 0; i < count; i++) {
        float furnishings = firm[i].furnishings < 0.5f ? 0.5f : firm[i].furnishings;
        float assets = firm[i].assets < 0.35f ? 0.35f : firm[i].assets;
        worst[i] = furnishings < assets ? furnishings : assets;
        low += worst[i] < 0.995f;
        re_log("business (%s): '%s' furnishings %.3f, assets of its worst job %.3f", why, firm[i].name, firm[i].furnishings, firm[i].assets);
    }
    if (low == 0)
        return;
    const char *prefix = T(RE_MSG_FIT_PREFIX);
    char text[480];
    int len = snprintf(text, sizeof text, "%s", prefix);
    for (; named < FIT_NAMED && named < low; named++) {
        int pick = -1;
        for (int i = 0; i < count; i++)
            if (worst[i] < 0.995f && (pick < 0 || worst[i] < worst[pick]))
                pick = i;
        double furnishings = 100.0 * (firm[pick].furnishings < 0.5f ? 0.5f : firm[pick].furnishings);
        double assets = 100.0 * (firm[pick].assets < 0.35f ? 0.35f : firm[pick].assets);
        int both = furnishings < 99.5 && assets < 99.5, none = firm[pick].assets <= (float)RE_ASSETS_LEAST + 1e-6f;
        if (named && (size_t)len < sizeof text)
            len += snprintf(text + len, sizeof text - (size_t)len, ", ");
        if ((size_t)len < sizeof text)
            len += none   ? snprintf(text + len, sizeof text - (size_t)len, T(RE_MSG_FIT_NONE), firm[pick].name)
                   : both ? snprintf(text + len, sizeof text - (size_t)len, T(RE_MSG_FIT_BOTH), firm[pick].name, furnishings, assets)
                          : snprintf(text + len, sizeof text - (size_t)len, T(furnishings < 99.5 ? RE_MSG_FIT_FURNISH : RE_MSG_FIT_ASSETS),
                                     firm[pick].name, furnishings < 99.5 ? furnishings : assets);
        worst[pick] = 2.0f; /* named */
    }
    if (low > named && (size_t)len < sizeof text)
        snprintf(text + len, sizeof text - (size_t)len, T(RE_MSG_HELD_MORE), low - named);
    panel_line(main_obj, prefix, text, "UIIconFurnishings", 1);
}

/* "Interest rate" becomes "Interest rate (credit grade A)". The mortgage window shows a quote: the household's
 * grade and loan-to-value are taken again once the home is bought, so the line says so. The education loan is not
 * graded: its line, which the game shares with the personal loan ("Variable interest rate per year"), says what the
 * plugin makes of it. */
static unsigned futures_list_edit(char *text, unsigned len, unsigned cap, const re_text_regs *regs);
static unsigned text_edit(int want, char *text, unsigned len, unsigned cap, const re_text_regs *regs, const re_text_values *values)
{
    if (want >= TEXT_WORDING) {
        /* trace: a sentence with values in it is a line of the month or of the ticker, not a label of every frame */
        int traced = g_on_trace && (values->count > 0 || strstr(re_wording_game_text(want - TEXT_WORDING), "{{") != NULL);
        char was[512] = "";
        if (traced)
            snprintf(was, sizeof was, "%.*s", (int)len, text);
        unsigned now = re_wording_apply(want - TEXT_WORDING, text, len, cap, values);
        if (traced)
            re_log("wording: '%s' -> '%.*s'", was, (int)now, text);
        return now;
    }
    if (want == TEXT_STOCK_LIST)
        return stock_list_edit(text, len, cap, regs);
    if (want == TEXT_FUT_LIST)
        return futures_list_edit(text, len, cap, regs);
    if (want == TEXT_CHART_YEARS || want == TEXT_CHART_YEARS_OPEN || want == TEXT_CHART_INDEX) {
        /* the button's text is months / 12 with the word for years: six months come out as "0 ..." and get their own
         * text. The title of the chart of all share prices gets what its numbers are behind it. */
        int years = want != TEXT_CHART_INDEX;
        if (want == TEXT_CHART_YEARS_OPEN && g_chart_caps && chart_economy(chart_opener((const BYTE *)(UINT_PTR)regs->ebp)) != 0 &&
            atoi(text) > CHART_ECONOMY_MONTHS / 12) {
            /* written before the boxes are built, where the months shown are lowered: the number the cap will leave */
            char now[64];
            unsigned digits = 0;
            while (digits < len && text[digits] >= '0' && text[digits] <= '9')
                digits++;
            int n = snprintf(now, sizeof now, "%d%.*s", CHART_ECONOMY_MONTHS / 12, (int)(len - digits), text + digits);
            if (n <= 0 || (unsigned)n >= sizeof now || (unsigned)n >= cap)
                return len;
            memcpy(text, now, (unsigned)n);
            return (unsigned)n;
        }
        const char *more = T(years ? RE_MSG_CHART_HALF_YEAR : RE_MSG_CHART_INDEX);
        unsigned n = (unsigned)strlen(more), at = years ? 0 : len;
        if ((years && (len == 0 || text[0] != '0')) || at + n >= cap)
            return len;
        if (years && g_on_trace)
            re_log("chart: the time button's text for six months put right at %s", want == TEXT_CHART_YEARS ? "a click" : "an opening");
        memcpy(text + at, more, n);
        return at + n;
    }
    if ((want >= TEXT_SORT_CHEAP && want <= TEXT_SORT_CAP) || want == TEXT_FUT_SORT) { /* the button says what its order is now */
        const char *label = want == TEXT_FUT_SORT ? T(RE_MSG_FUT_SORT) : T(RE_MSG_SORT_CHEAP + (want - TEXT_SORT_CHEAP));
        unsigned n = (unsigned)strlen(label);
        if (n >= cap)
            return len;
        memcpy(text, label, n);
        return n;
    }
    if (want == TEXT_CASINO_TIP) {
        /* a line above "Activity: Play Slots" says what the game pays: the odds are nowhere else on screen */
        const re_casino_game *game = casino_tip_game(regs);
        char line[300];
        int n = game != NULL ? snprintf(line, sizeof line, "%s", T(RE_MSG_CASINO_TIP)) : 0;
        for (int i = 0; game != NULL && i < game->prizes; i++) {
            char chance[16], times[16];
            snprintf(chance, sizeof chance, "%g", 100.0 * game->prize[i].chance);
            snprintf(times, sizeof times, "%g", game->prize[i].times);
            n += snprintf(line + n, sizeof line - (size_t)n, T(RE_MSG_CASINO_TIP_PRIZE), i ? ", " : "", chance, times);
        }
        if (n <= 0 || (unsigned)n + 1 + len >= cap)
            return len;
        memmove(text + n + 1, text, len);
        memcpy(text, line, (size_t)n);
        text[n] = '\n';
        return (unsigned)n + 1 + len;
    }
    if (want == TEXT_FUT_TIP || want == TEXT_AUTO_TIP || want == TEXT_ADVERT_TIP || want == TEXT_XP_TIP) {
        /* under "Asset Inflation Rate": what the number behind the rate is. Under the game's text about its automatic
         * staff management, which says that every wage demand is accepted: what the mod makes of that. Under "Click
         * to activate / deactivate" of an advert: how to hand the adverts of the business to the mod, or that it has
         * them and how to take them back. Under "experience decays by 1% every month": what does not. */
        char both[1000];
        const char *tip = T(want == TEXT_XP_TIP ? RE_MSG_XP_TIP : RE_MSG_FUT_TIP);
        if (want == TEXT_ADVERT_TIP) {
            char fee[64];
            int at = snprintf(both, sizeof both, T(adverts_kept(advert_tip_firm(regs)) ? RE_MSG_ADVERTS_TIP_ON : RE_MSG_ADVERTS_TIP),
                              100.0 * g_ads_upto, money(g_main_obj, job_fee(g_main_obj, RE_JOB_PR_SPECIALIST, g_ads_hours), fee));
            if (g_contracts_hooked && at >= 0 && (size_t)at < sizeof both)
                snprintf(both + at, sizeof both - (size_t)at, "%s", T(RE_MSG_ADVERTS_TIP_ALL));
            tip = both;
        }
        if (want == TEXT_AUTO_TIP) { /* what the click does with the mod, then what the management does with it */
            int at = snprintf(both, sizeof both, "%s%s", g_auto_whole_patched ? T(RE_MSG_AUTO_TIP_ALL) : "", g_wage_hooked ? T(RE_MSG_AUTO_TIP) : "");
            if (g_spare_months > 0 && at >= 0 && (size_t)at < sizeof both)
                at += snprintf(both + at, sizeof both - (size_t)at, T(RE_MSG_AUTO_TIP_SPARE), g_spare_months);
            if (g_fill_hooked && at >= 0 && (size_t)at < sizeof both)
                snprintf(both + at, sizeof both - (size_t)at, "%s", T(RE_MSG_AUTO_TIP_FILL));
            tip = both;
        }
        unsigned n = (unsigned)strlen(tip);
        if (len + 1 + n >= cap) {
            static int told; /* run 123: a fourth paragraph did not fit and all four were gone without a word */
            if (!told++)
                re_log("text: the mod's %u bytes under a hover text of %u DO NOT FIT into %u, left out (text %d)", n, len, cap, want);
            return len;
        }
        text[len] = '\n';
        memcpy(text + len + 1, tip, n);
        return len + 1 + n;
    }
    if (want == TEXT_HIRE_ORDER || want == TEXT_STAFF_TIP || want == TEXT_OFFER_TIP) {
        /* behind "4 candidates": the order the list is in. Under the sentence of a wage icon's hover text: the number
         * of the row and how it comes about. Under that of an offered contract's payout icon: the payout against the
         * standard cost of the contract's work. */
        char line[240];
        const unsigned char *offer = want == TEXT_OFFER_TIP ? business_tip_offer(regs) : NULL;
        const unsigned char *const *firm = (const unsigned char *const *)(UINT_PTR)(regs->ebp + RE_OFFER_TIP_FRAME_FIRM);
        unsigned n = want == TEXT_HIRE_ORDER  ? (unsigned)snprintf(line, sizeof line, "%s", T(RE_MSG_HIRE_ORDER))
                     : want == TEXT_STAFF_TIP ? business_staff_text(RE_MSG_STAFF_TIP, (const unsigned char *)(UINT_PTR)(regs->ebp + RE_WAGE_TIP_FRAME_STAFF),
                                                                    line + 1, sizeof line - 1)
                     : offer != NULL && re_readable(firm, 4)
                         ? business_contract_text(RE_MSG_OFFER_TIP, *firm, offer, line + 1, sizeof line - 1)
                         : 0;
        if (want != TEXT_HIRE_ORDER && n) {
            line[0] = '\n';
            n++;
        }
        if (n == 0 || len + n >= cap)
            return len;
        memcpy(text + len, line, n);
        return len + n;
    }
    if (want == TEXT_IPO_KEPT) {
        /* the fraction that is really kept, and what the window's preview says the rest sells for */
        int percent = (int)(100.0f * g_ipo_keep + 0.5f); /* 5 to 45 */
        for (unsigned i = 0; i + 3 <= len; i++)
            if (memcmp(text + i, "25%", 3) == 0) {
                if (percent >= 10) {
                    text[i] = (char)('0' + percent / 10);
                    text[i + 1] = (char)('0' + percent % 10);
                } else { /* one digit: the rest of the text moves up */
                    text[i] = (char)('0' + percent);
                    memmove(text + i + 1, text + i + 2, len - i - 2);
                    len--;
                }
                break;
            }
        if (g_ipo_preview_cash > 0) {
            char amount[64];
            int added = snprintf(text + len, cap - len, T(RE_MSG_IPO_REST), money(g_main_obj, g_ipo_preview_cash, amount));
            if (added > 0 && (unsigned)added < cap - len)
                len += (unsigned)added;
        }
        g_ipo_preview_cash = -1;
        return len;
    }
    if (want == TEXT_EDU_RATE) {
        const char *fixed = T(RE_MSG_EDU_FIXED);
        if (!g_edu_fixed || strlen(fixed) >= cap)
            return len;
        strcpy(text, fixed);
        return (unsigned)strlen(fixed);
    }
    if (!g_credit_ready)
        return len;
    int added = snprintf(text + len, cap - len, T(want == TEXT_MORTGAGE_QUOTE ? RE_MSG_GRADE_QUOTE : RE_MSG_GRADE),
                         re_credit_grade_name(g_credit.grade));
    return added > 0 && (unsigned)added < cap - len ? len + (unsigned)added : len;
}

/* ---------------------------------------------------------------- credit */

static int in_list(int tag, const int *tags, int n)
{
    for (int i = 0; i < n; i++)
        if (tags[i] == tag)
            return 1;
    return 0;
}

/* Income and living expenses of the last year from the cash-flow record (cents, annualised later).
 * Finance tags are 2000..2999. A larger tag is an object id: the record books loan proceeds, instalments and
 * deposits under the loan's own id, so those are financing and stay out of both figures. Debt service comes
 * from the debt list instead. */
static void classify_cashflow(const re_household *h, long long *income, long long *expenses)
{
    *income = *expenses = 0;
    for (int i = 0; i < h->cf_tag_count; i++) {
        const re_tag_sum *t = &h->cf_tags[i];
        if (t->tag < 0 || t->tag >= 10000 || in_list(t->tag, g_tags_capital, g_n_capital))
            continue;
        if (in_list(t->tag, g_tags_business, g_n_business)) {
            *income -= t->expense; /* business profit, not revenue, is income */
            continue;
        }
        if (!in_list(t->tag, g_tags_not_income, g_n_not_income))
            *income += t->income;
        *expenses += t->expense;
    }
}

/* The grade decides the rate, the rate decides the instalments, and the instalments feed the grade. Start from
 * the best grade, price the debts at it through the detour, grade again, and stop at the first grade that
 * reproduces itself. A worse assumed grade can only make the result worse, so this ends within six rounds and
 * does not depend on what the grade was before. Leaves `h` priced at the final grade.
 * Runs after a load, after every month end, and during the month when the debts or the homes changed. Variable
 * loans follow the result from that moment through the detour; a fixed loan keeps the rate stored in it.
 * `why`: 'L' after a load, 'M' after a month end, 'D' during the month. */
static void credit_update(void *main_obj, re_household *h, int why)
{
    if (!h->ok_balance) {
        re_log("credit: balance sheet not readable, grade unchanged");
        return;
    }
    re_credit_inputs in;
    re_credit_result r;
    long long income = 0, living = 0;
    int assumed = RE_GRADE_AAA, rounds = 0;
    for (;;) {
        memset(&in, 0, sizeof in);
        in.net_worth = dollars(h->net_worth);
        in.assets = dollars(h->assets_gross);
        in.debt = dollars(h->debt);
        in.liquid = dollars(h->cash + h->savings + h->investments + h->stocks);
        in.houses = dollars(h->houses_gross);
        in.mortgage_balance = dollars(h->houses_gross - h->houses_net);
        in.price_index = h->price_index;
        double ltv = in.houses > 0.0 ? in.mortgage_balance / in.houses : 0.0;
        if (g_getter_installed) {
            re_credit_spreads(&g_credit_cfg, assumed, ltv, &g_credit.personal_spread, &g_credit.mortgage_spread);
            g_credit_ready = 1;
            snapshot(main_obj, h); /* instalments at the assumed grade */
        }
        if (h->cf_months > 0 && h->debt_count >= 0) {
            double to_year = 12.0 / h->cf_months;
            classify_cashflow(h, &income, &living);
            in.history_months = h->cf_months;
            in.annual_debt_service = dollars(h->debts_year);
            in.annual_income = to_year * dollars(income);
            in.annual_expenses = to_year * dollars(living) + in.annual_debt_service;
        }
        re_credit_evaluate(&in, &g_credit_cfg, &r);
        rounds++;
        if (r.grade <= assumed || !g_getter_installed || rounds >= RE_GRADE_COUNT)
            break;
        assumed = r.grade;
    }
    g_credit = r;
    g_credit_ready = 1;
    g_graded_debt = h->debt;
    g_graded_houses = h->houses_gross;
    g_graded_count = h->debt_count;
    if (h->ok_economy)
        g_fixed_now = g_fixed_premium + g_fixed_reversion * ((double)h->base_rate - (double)h->reserve_rate);

    re_log("credit (%s): grade %s (score %d of 12, %d points from %d ratios, settled in %d round(s))",
           why == 'L' ? "after load" : why == 'M' ? "after month end" : "during the month", re_credit_grade_name(r.grade), r.score12,
           r.total, r.counted, rounds);
    re_log("   other debt / other assets %5.1f%%  -> %d   (debt $%.2f less mortgages $%.2f, assets $%.2f less homes $%.2f)",
           100.0 * r.ratio[RE_RATIO_DEBT_ASSETS], r.points[RE_RATIO_DEBT_ASSETS], in.debt, in.mortgage_balance, in.assets, in.houses);
    if (r.known[RE_RATIO_SERVICE_INCOME]) {
        re_log("   debt service / income   %6.1f%%  -> %d   (service $%.2f, income $%.2f a year, from %d month(s))",
               100.0 * r.ratio[RE_RATIO_SERVICE_INCOME], r.points[RE_RATIO_SERVICE_INCOME], in.annual_debt_service,
               in.annual_income, in.history_months);
        re_log("   liquid assets / expenses %5.2f yr -> %d   (liquid $%.2f, expenses $%.2f a year of which living $%.2f)",
               r.ratio[RE_RATIO_LIQUID_YEARS], r.points[RE_RATIO_LIQUID_YEARS], in.liquid, in.annual_expenses,
               in.annual_expenses - in.annual_debt_service);
    } else {
        re_log("   debt service / income and liquid assets / expenses: no cash-flow record or debt list, left out");
    }
    re_log("   net worth          $%.2f -> %d   (bands x price index %.4f)", in.net_worth, r.points[RE_RATIO_NET_WORTH],
           g_credit_cfg.index_net_worth && in.price_index > 0.0 ? in.price_index : 1.0);
    re_log("   mortgage LTV %.1f%%; spreads: personal +%.2f%%, mortgage +%.2f%%; central-bank rate %.2f%%", 100.0 * r.ltv,
           100.0 * r.personal_spread, 100.0 * r.mortgage_spread, 100.0 * h->reserve_rate);
    double variable = h->reserve_rate + r.mortgage_spread, game_variable = h->reserve_rate * 1.1 + 0.02;
    re_log("   rates now: personal %.2f%% (game %.2f%%), variable mortgage %.2f%% (game %.2f%%), new fixed mortgage %.2f%% "
           "(game %.2f%%; premium %+.2f%% at neutral rate %.2f%%)",
           100.0 * (h->reserve_rate + r.personal_spread), 100.0 * (h->reserve_rate * 1.5 + 0.10), 100.0 * variable,
           100.0 * game_variable, 100.0 * (variable * g_fixed_factor + g_fixed_now), 100.0 * (game_variable * 1.1 + 0.03),
           100.0 * g_fixed_now, 100.0 * h->base_rate);
    /* The grade is always visible next to the rate in the loan windows. The summary panel gets a line at the month
     * end, when the grade differs from the one last announced. A change during the month waits for that month end:
     * it would have to add a line from inside a text request, which only the load and month-end steps have done. */
    int before = (int)re_state_get(h->playthrough, "grade", -1);
    if (before != r.grade && why != 'D') {
        re_state_set(h->playthrough, "grade", r.grade);
        if (why == 'M' && before >= 0 && before < RE_GRADE_COUNT) {
            char text[160];
            snprintf(text, sizeof text, T(RE_MSG_GRADE_CHANGE), re_credit_grade_name(before), re_credit_grade_name(r.grade));
            panel_line(main_obj, T(RE_MSG_GRADE_PREFIX), text, "UISymbolDebt", 0);
        }
    }
}

/* The game computes rate = central-bank rate x interestXer + interestMod. Returning 1 and the spread gives
 * rate = central-bank rate + spread. */
__attribute__((force_align_arg_pointer)) static unsigned getter_policy(int id, int attr, unsigned bits)
{
    float game, out;
    memcpy(&game, &bits, 4);
    out = game;
    if (g_on_credit && g_credit_ready) {
        if (attr == RE_ATTR_XER)
            out = 1.0f;
        else
            out = (float)(id == RE_ID_PERSONAL_LOAN ? g_credit.personal_spread : g_credit.mortgage_spread);
    }
    LONG n = InterlockedIncrement(&g_getter_reads);
    if (g_trace_getter && (g_in_monthend || n <= 40))
        re_log("  getter #%ld%s id=%d attr=%s game=%g returned=%g", n, g_in_monthend ? " month-end" : "", id,
               attr == RE_ATTR_XER ? "interestXer" : "interestMod", game, out);
    memcpy(&bits, &out, 4);
    return bits;
}

/* Two of the game's routines hand a new debt to its debt list through a call that is redirected here.
 * Enrolment: the game stores every education loan as variable (rate 0) at the central-bank rate. Here the loan keeps
 * the central-bank rate of the month it starts in.
 * Purchase of a home with an approved mortgage: a fixed one arrives with the rate of the approval, which was quoted
 * before the home and the loan existed. Its id is noted, and the rate is set once the purchase has settled. */
static void loan_created(unsigned *record, void *debt_manager, unsigned return_address)
{
    if (!g_on_credit)
        return;
    long long principal = (long long)(((unsigned long long)record[7] << 32) | record[6]);
    if (va_of(return_address) == RE_VA_CALL_EDU_ADD_DEBT + 5) {
        float rate = re_game_reserve_rate(g_main_obj);
        if (record[1] != 0 || rate <= 0.0f)
            return;
        memcpy(&record[1], &rate, 4);
        re_log("education loan: fixed at %.2f%% a year, the central-bank rate now; $%.2f over %u months, first payment after "
               "%u month(s)",
               100.0 * rate, dollars(principal), record[5], record[12]);
    } else if (record[1] != 0) {
        float quoted;
        memcpy(&quoted, &record[1], 4);
        g_new_fixed_id = re_game_next_debt_id(debt_manager);
        g_refresh_at = GetTickCount();
        g_refresh_due = 1;
        re_log("fixed mortgage: $%.2f over %u months, approved at %.2f%%; debt id %d, rate to be set when the purchase has settled",
               dollars(principal), record[5], 100.0 * quoted, g_new_fixed_id);
    }
}

/* The fixed mortgage that was just created gets the rate of this moment: central-bank rate + the spread of the grade
 * and loan-to-value as they are with the home and the loan on the books + the fixed premium. From then on the rate
 * stays in the loan, whatever the grade does. Returns 1 when a rate was written. */
static int fixed_mortgage_price(void *main_obj, const re_household *h)
{
    int id = g_new_fixed_id;
    float *slot = id > 0 ? re_game_debt_rate(main_obj, id) : NULL;
    if (id <= 0)
        return 0;
    g_new_fixed_id = 0;
    if (slot == NULL || *slot == 0.0f) {
        re_log("fixed mortgage: debt id %d not found as a fixed loan, left as it is", id);
        return 0;
    }
    float before = *slot;
    *slot = (float)((h->reserve_rate + g_credit.mortgage_spread) * g_fixed_factor + g_fixed_now);
    re_log("fixed mortgage: debt id %d priced at purchase: %.2f%% = central bank %.2f%% + grade %s and LTV %.1f%% +%.2f%% + fixed "
           "premium %+.2f%% (approved at %.2f%%)",
           id, 100.0 * *slot, 100.0 * h->reserve_rate, re_credit_grade_name(g_credit.grade), 100.0 * g_credit.ltv,
           100.0 * g_credit.mortgage_spread, 100.0 * g_fixed_now, 100.0 * before);
    return 1;
}

/* Both places that compute a fixed mortgage rate read the same two constants: the quote in the mortgage window
 * and the rate stored when the mortgage is accepted. All four operands were checked at start-up. */
static int fixed_premium_patch(void)
{
    return re_patch_ptr(AT(RE_VA_FIXED_MUL_QUOTE) + 4, AT(RE_VA_CONST_1_1), &g_fixed_factor) &&
           re_patch_ptr(AT(RE_VA_FIXED_ADD_QUOTE) + 4, AT(RE_VA_CONST_0_03), &g_fixed_now) &&
           re_patch_ptr(AT(RE_VA_FIXED_MUL_ACCEPT) + 4, AT(RE_VA_CONST_1_1), &g_fixed_factor) &&
           re_patch_ptr(AT(RE_VA_FIXED_ADD_ACCEPT) + 4, AT(RE_VA_CONST_0_03), &g_fixed_now);
}

/* -------------------------------------------------------------- forecast */

/* One cash movement of the month-end routine goes into its kind. Tags are the finance tags of data/idmap.txt;
 * goods a firm buys carry the goods' own id, which is above 10000. */
static void flow_add(long long amount, int tag)
{
    int kind = amount > 0     ? (tag == 2041 ? FLOW_INCOME : FLOW_RECEIPTS)
               : tag == 2041  ? FLOW_STAFF
               : tag >= 10000 ? FLOW_GOODS
               : tag == 2181  ? FLOW_UTILITIES
               : tag == 2021  ? FLOW_RENT
               : tag == 2086  ? FLOW_PROPERTY_TAX
               : tag == 2311  ? FLOW_TAX
                              : FLOW_OTHER;
    g_flow_now[kind] += amount > 0 ? amount : -amount;
}

static void flow_load(const re_household *h)
{
    g_flow_period = (int)re_state_get(h->playthrough, "flowPeriod", -1);
    for (int i = 0; i < FLOW_COUNT; i++)
        g_flow_last[i] = re_state_get(h->playthrough, FLOW_KEYS[i], 0);
    for (int i = 0; i < 3; i++)
        g_property_tax[i] = re_state_get(h->playthrough, PROPERTY_TAX_KEYS[i], -1);
}

static void flow_commit(const re_household *h)
{
    memcpy(g_flow_last, g_flow_now, sizeof g_flow_last);
    g_flow_period = h->period;
    re_state_set(h->playthrough, "flowPeriod", g_flow_period);
    for (int i = 0; i < FLOW_COUNT; i++)
        re_state_set(h->playthrough, FLOW_KEYS[i], g_flow_last[i]);
    int phase = h->period % 3;
    g_property_tax[phase] = g_flow_now[FLOW_PROPERTY_TAX];
    re_state_set(h->playthrough, PROPERTY_TAX_KEYS[phase], g_property_tax[phase]);
}

/* Property tax expected at the end of `period`: what was paid three months earlier, or the largest amount seen
 * while that month of the quarter has not been watched yet. */
static long long property_tax_expected(int period)
{
    long long tax = g_property_tax[period % 3];
    if (tax < 0)
        for (int i = 0; i < 3; i++)
            if (g_property_tax[i] > tax)
                tax = g_property_tax[i];
    return tax > 0 ? tax : 0;
}

/* The debt instalments are computed exactly. The other month-end payments are taken as they were at the last
 * month end this plugin watched, when that is at most two months away; without one only the debts are forecast.
 * After a load of an earlier save that month end lies ahead of the calendar, and is as good a guide. */
static void forecast_update(void *main_obj, const re_household *h, int verbose)
{
    char a[64], text[200];
    const char *prefix = T(RE_MSG_FORECAST_PREFIX);
    if (h->debt_count < 0) {
        re_log("forecast: the debt list could not be read");
        return;
    }
    g_shown_due = h->debts_due;
    if (verbose) {
        re_log("forecast: the next monthly debt pass will charge $%.2f on %d debt(s); cash now $%.2f", dollars(h->debts_due),
               h->debt_count, dollars(h->cash));
        const re_debt *waiting[2] = {&h->pending_mortgage, &h->pending_education};
        for (int i = 0; i < 2; i++)
            if (waiting[i]->activity > 0)
                re_log("   approved, not used yet: %s (activity %d, stored rate %.4f, principal $%.2f)", i ? "education loan" : "mortgage",
                       waiting[i]->activity, waiting[i]->stored_rate, dollars(waiting[i]->principal));
        for (int i = 0; i < h->debt_count && i < RE_MAX_DEBTS; i++) {
            const re_debt *debt = &h->debts[i];
            char rate[24] = "variable";
            if (debt->stored_rate != 0.0f)
                snprintf(rate, sizeof rate, "fixed %.2f%%", 100.0 * debt->stored_rate);
            re_log("   debt id=%d activity=%d type=%d %s months_left=%d of %d delay=%d principal=$%.2f due=$%.2f year=$%.2f", debt->id,
                   debt->activity, debt->type, rate, debt->months_left, debt->length_months, debt->delay_months,
                   dollars(debt->principal), dollars(debt->due), dollars(debt->year));
        }
    }
    int age = h->period - g_flow_period;
    if (g_flow_period < 0 || age < -2 || age > 2) {
        /* no month end watched yet: the loans are all that can be named, and the line says so */
        re_log("forecast: no month end watched yet for this playthrough (stored period %d, now %d)", g_flow_period, h->period);
        snprintf(text, sizeof text, T(RE_MSG_FORECAST_LOANS), prefix, money(main_obj, h->debts_due, a));
        panel_line(main_obj, prefix, text, "UISymbolDebt", 1);
        g_forecast_need = h->debts_due;
        g_forecast_whole = 0;
        return;
    }
    g_forecast_whole = 1;
    const long long *last = g_flow_last;
    long long property_tax = property_tax_expected(g_forecast_period);
    long long out = last[FLOW_STAFF] + last[FLOW_GOODS] + last[FLOW_UTILITIES] + last[FLOW_RENT] + property_tax + last[FLOW_TAX] +
                    last[FLOW_OTHER] + h->debts_due;
    long long need = out - last[FLOW_INCOME];
    g_forecast_need = need > 0 ? need : 0;
    re_log("forecast: end of period %d, from the month end of period %d: out $%.2f (staff $%.2f, debts $%.2f, goods $%.2f, "
           "utilities $%.2f, rent $%.2f, property tax $%.2f, tax $%.2f, other $%.2f), wages in $%.2f, cash needed $%.2f, "
           "cash now $%.2f; other receipts last time, not counted: $%.2f",
           g_forecast_period, g_flow_period, dollars(out), dollars(last[FLOW_STAFF]), dollars(h->debts_due), dollars(last[FLOW_GOODS]),
           dollars(last[FLOW_UTILITIES]), dollars(last[FLOW_RENT]), dollars(property_tax), dollars(last[FLOW_TAX]),
           dollars(last[FLOW_OTHER]), dollars(last[FLOW_INCOME]), dollars(need), dollars(h->cash), dollars(last[FLOW_RECEIPTS]));
    snprintf(text, sizeof text, T(RE_MSG_FORECAST_CASH), prefix, money(main_obj, need > 0 ? need : 0, a));
    panel_line(main_obj, prefix, text, "UISymbolDebt", 1);
}

/* Grade, then the fixed mortgage that is waiting for its rate, then the grade once more with that rate in place. */
static void regrade(void *main_obj, re_household *h, int why)
{
    credit_update(main_obj, h, why);
    if (fixed_mortgage_price(main_obj, h))
        credit_update(main_obj, h, why);
}

/* A cash movement in the middle of the month can change the debts or the homes: a loan taken or repaid, a home
 * bought or sold. A moment later, from the next text the game fetches, when the transaction that moved the money is
 * over, the household is looked at again. If the debts or homes differ from what the grade was computed with, the
 * grade is computed again, so a loan just taken already counts for the next one and for every variable rate. The
 * forecast line follows the instalments. Wages and rent follow at the next month end. */
static void refresh_if_due(void)
{
    static re_household h;
    if (!g_refresh_due || g_in_monthend || g_main_obj == NULL || GetTickCount() - g_refresh_at < 300)
        return;
    if (InterlockedExchange(&g_refreshing, 1))
        return;
    g_refresh_due = 0;
    /* the debts and the homes first, which is quick; the whole household, 12 to 14 ms of the game's own valuing, only
     * when one of them is not what the grade and the forecast line were made with (note m30) */
    re_game_snapshot_debts(g_main_obj, &h);
    int changed = h.debt_count != g_graded_count || h.debt != g_graded_debt || h.houses_gross != g_graded_houses;
    if (!((g_on_credit && h.debt_count >= 0 && (changed || g_new_fixed_id)) || (g_on_forecast && h.debt_count >= 0 && h.debts_due != g_shown_due))) {
        InterlockedExchange(&g_refreshing, 0);
        return;
    }
    snapshot(g_main_obj, &h);
    changed = h.debt_count != g_graded_count || h.debt != g_graded_debt || h.houses_gross != g_graded_houses;
    if (g_on_credit && h.ok_balance && h.debt_count >= 0 && (changed || g_new_fixed_id))
        regrade(g_main_obj, &h, 'D');
    if (g_on_forecast && h.debt_count >= 0 && h.debts_due != g_shown_due) /* nothing to say when the debts are what they were */
        forecast_update(g_main_obj, &h, 0);
    InterlockedExchange(&g_refreshing, 0);
}

/* ------------------------------------------------- cash movements (forecast and trace) */

__attribute__((force_align_arg_pointer)) static void money_sink(void *self, const unsigned *stack)
{
    if (!re_readable(stack, 28))
        return;
    long long amount = (long long)(((unsigned long long)stack[2] << 32) | stack[1]);
    /* inside the month-end routine, and not from the debt routine: those are the instalments, forecast exactly */
    if (g_in_monthend && g_on_forecast && va_of(stack[0]) - RE_VA_DEBT_PASS >= RE_DEBT_PASS_SIZE)
        flow_add(amount, (int)stack[3]);
    if (!g_in_monthend && (g_on_forecast || g_on_credit)) {
        g_refresh_at = GetTickCount();
        g_refresh_due = 1;
    }
    if (!g_on_trace || !g_trace_money || InterlockedIncrement(&g_trace_lines) > TRACE_LINE_LIMIT)
        return;
    LONG n = g_trace_lines;
    long long cash = 0;
    if (re_readable(self, 0x10)) {
        BYTE *owner = *(BYTE **)((BYTE *)self + 8);
        if (re_readable(owner, 0x230))
            cash = *(long long *)(owner + 0x228);
    }
    re_log("  money #%ld%s caller=0x%08x amount=$%.2f tag=%d args=%u,%u,%u cash_before=$%.2f", n,
           g_in_monthend ? " month-end" : "", va_of(stack[0]), dollars(amount), (int)stack[3], stack[4] & 0xff, stack[5], stack[6],
           dollars(cash));
}

static void cashflow_line(int period, int tag, long long expense, long long income, void *ctx)
{
    (void)ctx;
    re_log("   cashflow period=%d tag=%d expense=$%.2f income=$%.2f", period, tag, dollars(expense), dollars(income));
}

static void taxable_line(int period, int tag, long long expense, long long income, void *ctx)
{
    re_log("   taxable (%s) period=%d tag=%d expense=$%.2f income=$%.2f", (const char *)ctx, period, tag, dollars(expense),
           dollars(income));
}

static void trace_state(void *main_obj, const re_household *h, const char *why)
{
    re_log("-- state (%s)", why);
    re_log("   calendar: uniCount=%lld tick=%d month=%d year=%d period=%d", h->uni_count, h->tick, h->month, h->year, h->period);
    re_log("   playthrough id: %s", h->playthrough);
    re_log("   economy: growth=%g baseInterestRate=%g reserveRate=%g priceIndex=%g", h->growth, h->base_rate, h->reserve_rate,
           h->price_index);
    re_log("   market stream: seed=%u numCalls=%u", h->market_seed, h->market_calls);
    re_log("   net worth $%.2f, assets $%.2f, debt $%.2f, cash $%.2f, savings $%.2f, investments $%.2f, stocks $%.2f",
           dollars(h->net_worth), dollars(h->assets_gross), dollars(h->debt), dollars(h->cash), dollars(h->savings),
           dollars(h->investments), dollars(h->stocks));
    re_log("   houses $%.2f gross, $%.2f net of mortgage; bankruptcy lockout ends at %d", dollars(h->houses_gross),
           dollars(h->houses_net), h->bankrupt_end);
    re_log("   cash-flow record: %d month(s) before period %d, income $%.2f, expenses $%.2f", h->cf_months, h->period,
           dollars(h->cf_income_total), dollars(h->cf_expense_total));
    for (int i = 0; i < h->cf_tag_count; i++)
        re_log("   cashflow 12m tag=%d expense=$%.2f income=$%.2f", h->cf_tags[i].tag, dollars(h->cf_tags[i].expense),
               dollars(h->cf_tags[i].income));
    if (g_trace_cashflow) {
        int months = re_game_cashflow_walk(main_obj, h->period - 2, h->period, cashflow_line, NULL);
        re_log("   cashflow by month: %d month(s) listed (-1 = record not understood)", months);
        /* thirteen months, so that the tax year that ended in June is whole both at its month end and on a load in July */
        long long tax_free;
        float rate;
        months = re_game_taxable_walk(main_obj, h->period - 12, h->period, taxable_line, "year");
        re_log("   taxable by month: %d month(s) listed (-1 = record not understood)", months);
        if (re_game_tax_terms(main_obj, &tax_free, &rate))
            re_log("   tax terms: untaxed part $%.2f (0 = not set yet), rate %.4f", dollars(tax_free), rate);
        else
            re_log("   tax terms: not read");
    }
    re_log("-- state done");
}

/* Takes numbers from one of the game's random streams with the game's own function: from the stream behind stock
 * prices (RE_RANDGEN_MARKET) as the game does when the player trades property or opens the IPO window, from the
 * economy's (RE_RANDGEN_ECONOMY) as an activity with a random amount does. A stream's position is saved with the game. */
static void stream_advance(void *main_obj, int stream, int draws, const char *who)
{
    const char *name = stream == RE_RANDGEN_MARKET ? "market" : "economy";
    unsigned *seed, *calls;
    void *market;
    if (draws <= 0)
        return;
    if (!re_game_market_stream(main_obj, &seed, &calls, &market)) {
        re_log("%s: the %s stream could not be reached, not advanced", who, name);
        return;
    }
    BYTE *unit = (BYTE *)market - RE_RANDGEN_MARKET + stream;
    calls = (unsigned *)(unit + RE_RAND_UNIT_CALLS);
    unsigned before = *calls;
    for (int i = 0; i < draws; i++)
        ((chance_fn)AT(RE_VA_RAND_CHANCE))(unit);
    re_log("%s: %s stream advanced by %d draw(s): numCalls %u -> %u (seed %u)", who, name, draws, before, *calls, *(unsigned *)unit);
}

/* ----------------------------------------------------------------- guard */

/* Asked by the three trade hooks just before the game would buy, sell or open a futures trade. The monthly auto
 * transfer buys through the same function from inside the month-end routine; it is refused like a click, counted,
 * and reported in the summary panel instead of the ticker. */
static int guard_blocked(int trade)
{
    static const char *const names[] = {"purchase", "sale", "futures trade"};
    long long now = g_guard_unlock > 0 && g_main_obj != NULL ? re_game_ticks(g_main_obj) : -1;
    if (now < 0 || now >= g_guard_unlock)
        return 0;
    long long left = g_guard_unlock - now;
    if (g_in_monthend && g_guard_month_end_pending) {
        /* Nothing could be arranged between the load and this month end: an auto-transfer rule takes a five-hour
         * visit to the bank. The purchase is the one the save was going to make anyway, and the save was not written
         * under a rollback lock (re_guard_on_load), when a rule could have been made for prices already seen. */
        re_log("guard: purchase of the auto transfer let through: the save was made in the hour of this month end");
        return 0;
    }
    re_log("guard: %s refused%s, %lld hour(s) of lock left", names[trade], g_in_monthend ? " (auto transfer)" : "", left);
    if (g_in_monthend) {
        g_guard_auto_refused++;
        return 1;
    }
    char text[200];
    snprintf(text, sizeof text, T(RE_MSG_LOCK_REFUSED), left);
    if (g_can_text)
        re_game_post_message(g_main_obj, text);
    return 1;
}

/* The summary line of a running lock is saved with the game like every line of the panel, so a save written while a
 * lock was running carries it: 1 when the loaded panel has such a line, 0 when it has none, -1 when that cannot be
 * told. The line may be in another language than the game is set to now. */
static int guard_lock_line(void *main_obj)
{
    if (!g_can_text)
        return -1;
    for (int lang = 0; lang < RE_LANG_COUNT; lang++) {
        int found = re_lang_has(lang, RE_MSG_LOCK_RUNS) ? re_game_replace_event(main_obj, re_lang_text(lang, RE_MSG_LOCK_RUNS), NULL) : 0;
        if (found != 0)
            return found;
    }
    return 0;
}

/* At a load the record of this playthrough decides the lock and whether the stock-price stream is moved. At a month
 * end the record moves on, and a lock that is still running is announced again in the new month's summary panel. */
static void guard_step(void *main_obj, const re_household *h, int is_load)
{
    const char *prefix = T(RE_MSG_LOCK_PREFIX), *icon = "UISymbolStockMarket";
    char text[200];
    if (is_load)
        g_guard_unlock = 0;
    int was_quiet = g_guard_quiet;
    g_guard_month_end_pending = g_guard_quiet = 0; /* set by a load only, and over once that month end has run */
    if (!h->ok_calendar || h->playthrough[0] == 0) {
        re_log("guard: calendar or playthrough id not readable, nothing recorded");
        return;
    }
    re_guard_record rec = {re_state_get(h->playthrough, "guardFarthest", -1), re_state_get(h->playthrough, "guardShiftUntil", -1),
                           (int)re_state_get(h->playthrough, "guardGeneration", 0), re_state_get(h->playthrough, "guardLockFrom", -1),
                           re_state_get(h->playthrough, "guardLockUntil", -1)};
    long long now = h->uni_count, was_unlock = g_guard_unlock;
    if (is_load) {
        re_guard_decision d;
        re_guard_record before = rec;
        int released = re_ini_int("guard", "release", 0);
        if (released) {
            rec.farthest = rec.shift_until = rec.lock_from = rec.lock_until = -1;
            rec.generation = 0;
            re_ini_set_int("guard", "release", 0);
        }
        int lock_line = guard_lock_line(main_obj);
        re_guard_on_load(&rec, now, (long long)g_guard_cap_months * MONTH_TICKS, g_guard_shift, lock_line, &d);
        g_guard_unlock = d.unlock_tick;
        /* the month's last hour: a lock that was cut to the cap can end at any hour, and its last hour is no month end */
        g_guard_month_end_pending = d.month_end_pending && h->tick == MONTH_TICKS - 1;
        re_log("guard: load at hour %lld; record: farthest month end %lld, stream moved for saves up to %lld, generation %d, "
               "last rollback lock %lld to %lld; the save carries a lock line: %s%s",
               now, before.farthest, before.shift_until, before.generation, before.lock_from, before.lock_until,
               lock_line == 1 ? "yes" : lock_line == 0 ? "no" : "not readable",
               released ? "; record cleared by [guard] release, which is back at 0" : "");
        if (d.unlock_tick > 0)
            re_log("guard: trades locked for %lld hour(s), until hour %lld%s", d.unlock_tick - now, d.unlock_tick,
                   d.capped ? " (cut to the cap)" : "");
        else
            re_log("guard: no lock");
        stream_advance(main_obj, RE_RANDGEN_MARKET, d.draws, "guard");
    } else {
        re_guard_on_month_end(&rec, now);
        now++; /* the counter moves on as soon as the routine has returned */
    }
    re_state_set(h->playthrough, "guardFarthest", rec.farthest);
    re_state_set(h->playthrough, "guardShiftUntil", rec.shift_until);
    re_state_set(h->playthrough, "guardGeneration", rec.generation);
    re_state_set(h->playthrough, "guardLockFrom", rec.lock_from);
    re_state_set(h->playthrough, "guardLockUntil", rec.lock_until);

    if (g_guard_unlock > now) {
        const char *runs = T(RE_MSG_LOCK_RUNS);
        snprintf(text, sizeof text, T(RE_MSG_LOCK_RUNS_REST), runs, g_guard_unlock - now);
        if (!g_guard_month_end_pending)
            panel_line(main_obj, prefix, text, icon, 1);
        else if (g_can_text && re_game_replace_event(main_obj, runs, text) == 1)
            re_log("summary panel (line replaced): %s", text); /* the line of a lock that ended earlier in the month */
        else
            /* The hour before a month end that is about to run, after a load of the game's own autosave: the panel
             * of the old month is closed by that month end, and the player did nothing to be told about. */
            g_guard_quiet = 1;
    } else if (is_load) {
        /* a save made during a lock carries the lock's line; without a lock now, the line must not stay as it is */
        snprintf(text, sizeof text, T(RE_MSG_LOCK_NONE), prefix);
        if (g_can_text && re_game_replace_event(main_obj, prefix, text) == 1)
            re_log("summary panel (line replaced): %s", text);
    } else if (was_unlock > 0) {
        g_guard_unlock = 0;
        re_log("guard: the lock is over");
        if (!was_quiet) {
            snprintf(text, sizeof text, T(RE_MSG_LOCK_OVER), prefix);
            panel_line(main_obj, prefix, text, icon, 0);
        }
    }
    if (!is_load && g_guard_auto_refused > 0) {
        const char *auto_prefix = T(RE_MSG_AUTO_PREFIX);
        snprintf(text, sizeof text, T(RE_MSG_AUTO_SKIPPED), auto_prefix, g_guard_auto_refused);
        panel_line(main_obj, auto_prefix, text, icon, 0);
    }
}

/* ------------------------------------------------------------------- ipo */

/* After the game's twelve price steps, for the window's preview and for the listing itself. With the patched
 * operands the steps start from V / N, so the price that comes out is what the market makes of the whole business. */
static void ipo_steps_done(const BYTE *frame, long long *price, long long offer, long long earnings_per_share)
{
    long long market = *price;
    *price = re_ipo_floor(market, offer, earnings_per_share, g_ipo_floor);
    long long shares = *(const long long *)(frame + RE_IPO_FRAME_SHARES), founder = *(const long long *)(frame + RE_IPO_FRAME_FOUNDER);
    g_ipo_preview_cash = re_ipo_cash(shares, founder, *price);
    if (g_on_trace) {
        static const char *const letters[RE_IPO_LETTERS + 1] = {"AAA", "AA", "A", "B", "C", "D", "E"};
        re_log("ipo: price steps: offer $%.2f a share, earnings $%.2f a share, market $%.2f, listing price $%.2f%s; %lld shares, "
               "founder keeps %lld, the rest would sell for $%.2f; letter %s",
               dollars(offer), dollars(earnings_per_share), dollars(market), dollars(*price),
               *price != market ? " (floor)" : "", shares, founder, dollars(g_ipo_preview_cash),
               g_ipo_grade_installed && offer > 0 ? letters[re_ipo_grade_index(&g_ipo_grade, (float)*price / (float)offer)] : "-");
    }
}

/* The listing itself, just before the game charges its fee: the public pays the founder for the shares the founder
 * does not keep. The money goes through the game's own cash function, flagged as taxable the way a dividend is. */
static void ipo_before_fee(const BYTE *frame, void *finance)
{
    const BYTE *company = *(const BYTE *const *)(frame + RE_IPO_FRAME_COMPANY);
    if (!re_readable(company, RE_COMPANY_SHARES + 8)) {
        re_log("ipo: the company being listed could not be read, nothing paid");
        return;
    }
    long long price = *(const long long *)(company + RE_COMPANY_PRICE), shares = *(const long long *)(company + RE_COMPANY_SHARES);
    long long founder = *(const long long *)(frame + RE_IPO_FRAME_FOUNDER);
    long long cash = re_ipo_cash(shares, founder, price);
    /* The first election opens next month and a member can only be nominated during that month; the game tells
     * the player about elections of a company only when this switch of the company's board tab is on. `company`
     * is the window's template; the listed company is the copy the stock market made of it. */
    if (g_on_board && g_board_notice && re_readable(company, RE_COMPANY_ID + 4)) {
        BYTE *listed = re_game_listed_company(*(void *const *)(frame + RE_IPO_FRAME_MARKET), *(const int *)(company + RE_COMPANY_ID));
        if (listed != NULL)
            listed[RE_COMPANY_BOARD_NOTICE] = 1;
        re_log("board: election notice of the company being listed %s", listed ? "switched on" : "left off (company not found)");
    }
    int period = re_game_period(g_main_obj);
    re_log("ipo: listed %lld shares at $%.2f; the founder keeps %lld (worth $%.2f) and is paid $%.2f for the other %lld%s", shares,
           dollars(price), founder, dollars(founder * price), dollars(cash), shares - founder,
           g_ipo_taxable ? ", booked as taxable income" : "");
    if (cash <= 0)
        return;
    if (g_on_trace)
        re_game_taxable_walk(g_main_obj, period, period, taxable_line, "before");
    ((money_fn)AT(RE_VA_CHANGE_MONEY))(finance, (unsigned)cash, (int)(cash >> 32), 2061, g_ipo_taxable, 0, 0);
    if (g_on_trace)
        re_game_taxable_walk(g_main_obj, period, period, taxable_line, "after");
}

/* The offer price comes last, and only when everything before it is in: with the offer price on all shares and the
 * payment missing, a listing would leave the founder with a fraction of the business and nothing else. Every site
 * was checked at start-up, so a step can only fail when the page cannot be made writable. */
static int ipo_install(void)
{
    static const BYTE from_founder[5] = {0x86, 0x98, 0x02, 0x00, 0x00}, from_public[5] = {0x8e, 0x98, 0x02, 0x00, 0x00},
                      from_getter[5] = {0x81, 0x98, 0x02, 0x00, 0x00};
    static const BYTE founder_hi[4] = {0x44, 0xfc, 0xff, 0xff}, founder_lo[4] = {0x40, 0xfc, 0xff, 0xff};
    static const BYTE all_hi[4] = {0x14, 0xfc, 0xff, 0xff}, all_lo[4] = {0x10, 0xfc, 0xff, 0xff};
    static const BYTE public_hi[6] = {0xff, 0xb5, 0x28, 0xfc, 0xff, 0xff}, public_lo[6] = {0xff, 0xb5, 0x0c, 0xfc, 0xff, 0xff};
    static const BYTE push_zero[6] = {0x6a, 0x00, 0x90, 0x90, 0x90, 0x90};
    BYTE absolute[3][5] = {{0x05}, {0x0d}, {0x05}}; /* the same instructions with an absolute address: xmm0, xmm1, xmm0 */
    const float *keep = &g_ipo_keep;
    for (int i = 0; i < 3; i++)
        memcpy(absolute[i] + 1, &keep, 4);
    re_ipo_steps = AT(RE_VA_IPO_STEPS);
    re_ipo_change_money = AT(RE_VA_CHANGE_MONEY);
    re_ipo_steps_done = ipo_steps_done;
    re_ipo_before_fee = ipo_before_fee;
    int fraction = re_patch_bytes(AT(RE_VA_IPO_KEEP_FOUNDER) + 3, from_founder, absolute[0], 5) &&
                   re_patch_bytes(AT(RE_VA_IPO_KEEP_PUBLIC) + 3, from_public, absolute[1], 5) &&
                   re_patch_bytes(AT(RE_VA_IPO_KEEP_GETTER) + 3, from_getter, absolute[2], 5);
    int payment = fraction && re_patch_call(AT(RE_VA_CALL_IPO_FEE), AT(RE_VA_CHANGE_MONEY), re_ipo_fee_hook) &&
                  re_patch_call(AT(RE_VA_CALL_IPO_STEPS), AT(RE_VA_IPO_STEPS), re_ipo_steps_hook);
    g_ipo_steps_hooked = payment; /* the steps' call is the last of the three, so nothing else leaves it redirected */
    int company = payment && re_patch_bytes(AT(RE_VA_IPO_EQUITY_HI), public_hi, push_zero, 6) &&
                  re_patch_bytes(AT(RE_VA_IPO_EQUITY_LO), public_lo, push_zero, 6) &&
                  re_patch_bytes(AT(RE_VA_IPO_INVEST_HI) + 2, founder_hi, all_hi, 4) &&
                  re_patch_bytes(AT(RE_VA_IPO_INVEST_LO) + 2, founder_lo, all_lo, 4);
    int offer = company && re_patch_bytes(AT(RE_VA_IPO_OFFER_HI) + 2, founder_hi, all_hi, 4) &&
                re_patch_bytes(AT(RE_VA_IPO_OFFER_LO) + 2, founder_lo, all_lo, 4);
    re_log("ipo: founder's fraction %s, payment hooks %s, company without new money %s, offer price on all shares %s; the founder "
           "keeps %.0f%%, floor at the offer price %s, cash taxable %s",
           fraction ? "patched" : "NOT patched", payment ? "installed" : "NOT installed", company ? "patched" : "NOT patched",
           offer ? "patched" : "NOT patched", 100.0 * g_ipo_keep, g_ipo_floor ? "on" : "off", g_ipo_taxable ? "yes" : "no");
    /* The letter only describes the listing, so it goes in last and the listing does not depend on it. */
    int limits = offer && re_ipo_grade_limits(g_ipo_letters, &g_ipo_grade);
    g_ipo_grade_installed = limits && re_patch_ptr(AT(RE_VA_IPO_GRADE_BASE) + 4, AT(RE_VA_CONST_F32_1), &g_ipo_grade.base) &&
                            re_patch_ptr(AT(RE_VA_IPO_GRADE_AAA) + 4, AT(RE_VA_CONST_F64_0_15), &g_ipo_grade.aaa) &&
                            re_patch_ptr(AT(RE_VA_IPO_GRADE_AA) + 3, AT(RE_VA_CONST_F32_0_1), &g_ipo_grade.aa) &&
                            re_patch_ptr(AT(RE_VA_IPO_GRADE_A) + 3, AT(RE_VA_CONST_F32_0_05), &g_ipo_grade.a) &&
                            re_patch_ptr(AT(RE_VA_IPO_GRADE_C) + 3, AT(RE_VA_CONST_F32_M0_05), &g_ipo_grade.c) &&
                            re_patch_ptr(AT(RE_VA_IPO_GRADE_D) + 3, AT(RE_VA_CONST_F32_M0_1), &g_ipo_grade.d);
    re_log("ipo: quality letter by listing price / offer price %s; AAA above %g, AA %g, A %g, B %g, C %g, D %g%s",
           g_ipo_grade_installed ? "patched" : "NOT patched", g_ipo_letters[0], g_ipo_letters[1], g_ipo_letters[2], g_ipo_letters[3],
           g_ipo_letters[4], g_ipo_letters[5], offer && !limits ? " (gradeAbove must be positive and descending)" : "");
    return offer;
}

/* ----------------------------------------------------------------- board */

/* After the game has scored a candidate, for the election and for the ranking of the board tab. The household's
 * holding guarantees seats; a nominee whose total is forced is put above everybody else. When the household has
 * more nominees than guaranteed seats, those who are elected anyway keep their seats and the guarantee goes to the
 * others (REQUEST.md [29]); for that the totals of all nominees with nothing forced are taken once per pass, when
 * the first of the household's nominees comes by. The experience part of the score is left alone. */
static void board_scored(BYTE *company, BYTE *score, int person, unsigned return_address)
{
    static struct {
        const BYTE *company;
        unsigned site;
        int count, ids[32];
        float totals[32];
    } pass; /* the totals with nothing forced, of the pass that is running */
    int nominees[32], household[32];
    if (!re_readable(company, RE_COMPANY_BOARD_NOTICE + 4) || !re_readable(score, RE_SCORE_BYTES))
        return;
    int election = (BYTE *)return_address == AT(RE_VA_CALL_BOARD_ELECT) + 5;
    int count = re_board_nominees(company, nominees, 32), members = re_board_household(company, household, 32);
    void *market = *(void **)(company + RE_COMPANY_MARKET);
    if (count < 0 || members < 0 || !re_readable(market, 0x260)) {
        if (g_board_lines++ < BOARD_TRACE_LINES)
            re_log("board: the nominees (%d), the household (%d) or the market of a company could not be read; score left alone", count,
                   members);
        return;
    }
    float *total = (float *)(score + RE_SCORE_TOTAL), own = *total;
    int same_pass = pass.company == company && pass.site == return_address && pass.count == count &&
                    memcmp(pass.ids, nominees, (size_t)count * sizeof nominees[0]) == 0;
    /* test runs: the totals the game computed for the nominees that have come by in this pass, to compare with */
    static struct {
        int n, ids[32];
        float totals[32];
    } seen;
    if (g_board_prepass) {
        if (count > 0 && person == nominees[0])
            seen.n = 0;
        if (seen.n < 32) {
            seen.ids[seen.n] = person;
            seen.totals[seen.n++] = own;
        }
    }
    int place = re_board_place(nominees, count, household, members, person);
    if (place < 0) {
        /* Test runs: what the plugin asked for must be what the game computes itself. Only for the nominees after
         * the household's first one: the totals of this pass are taken when that one comes by. */
        int after_mine = 0;
        for (int i = 0; g_board_prepass && same_pass && i < count; i++) {
            if (nominees[i] == person && after_mine && g_board_lines++ < BOARD_TRACE_LINES)
                re_log("board: person %d: asked beforehand %.4f, the game's own %.4f: %s", person, pass.totals[i], own,
                       pass.totals[i] == own ? "same" : "DIFFERENT");
            after_mine = after_mine || re_board_place(nominees, count, household, members, nominees[i]) >= 0;
        }
        return;
    }
    int id = *(int *)(company + RE_COMPANY_ID), mine = 0;
    float fraction = re_board_fraction(AT(RE_VA_OWNERSHIP), market, id);
    int seats = re_board_seats(fraction, g_board_per_seat);
    for (int i = 0; i < count; i++)
        mine += re_board_place(nominees, count, household, members, nominees[i]) >= 0;
    const float *natural = NULL;
    if (seats > 0 && (mine > seats || g_board_prepass)) {
        if (place == 0 || !same_pass) {
            pass.company = company;
            pass.site = return_address;
            pass.count = count;
            memcpy(pass.ids, nominees, (size_t)count * sizeof nominees[0]);
            same_pass = re_board_totals(company, nominees, count, person, own, pass.totals);
            if (!same_pass)
                pass.company = NULL;
            int same = 0, different = 0;
            for (int i = 0; g_board_prepass && same_pass && i < count; i++)
                for (int k = 0; k < seen.n; k++)
                    if (seen.ids[k] == nominees[i] && nominees[i] != person) {
                        same += seen.totals[k] == pass.totals[i];
                        different += seen.totals[k] != pass.totals[i];
                    }
            if (g_board_prepass && g_board_lines++ < BOARD_TRACE_LINES)
                re_log("board: %s, company %d: asked for the totals of %d other nominee(s); %d of them the game had computed in "
                       "this pass already: %d the same, %d DIFFERENT",
                       election ? "election" : "tab", id, same_pass ? count - 1 : 0, same + different, same, different);
        }
        natural = same_pass ? pass.totals : NULL;
    }
    int rank = re_board_rank(nominees, natural, count, household, members, seats, person);
    if (rank >= 0)
        *total = re_board_total(g_board_top, rank);
    if ((election && (rank >= 0 || mine > seats)) || (g_on_trace && g_board_lines++ < BOARD_TRACE_LINES))
        re_log("board: %s, company %d, person %d: the household holds %.1f%%, good for %d seat(s), and has %d nominee(s); "
               "its nominee no. %d: total %.2f %s%s",
               election ? "election" : "tab", id, person, 100.0 * fraction, seats, mine, place + 1, own,
               rank >= 0 ? "raised above all others" : "left as it is",
               mine <= seats ? "" : natural == NULL ? " (the other totals could not be asked for)" : " (by the totals with nothing forced)");
}

/* The election first: with only the tab redirected the tab would promise a seat the election does not give. */
static void board_install(void)
{
    re_board_score = AT(RE_VA_BOARD_SCORE);
    re_board_score_free = AT(RE_VA_SCORE_FREE);
    re_board_scored = board_scored;
    int election = re_patch_call(AT(RE_VA_CALL_BOARD_ELECT), AT(RE_VA_BOARD_SCORE), re_board_score_hook);
    int tab = election && re_patch_call(AT(RE_VA_CALL_BOARD_RANK), AT(RE_VA_BOARD_SCORE), re_board_score_hook);
    /* the notice is switched on in the listing itself, by the ipo feature's hook (ipo_before_fee) */
    re_log("board: election call %s, board tab's ranking call %s; one seat for every %.0f%% of a company held, election "
           "notice for own listings %s",
           election ? "redirected" : "NOT redirected", tab ? "redirected" : "NOT redirected", 100.0 * g_board_per_seat,
           !g_board_notice ? "off" : g_ipo_installed ? "on" : "OFF (it needs the ipo feature, which is not installed)");
}

/* ---------------------------------------------------------------- casino */

/* The state file keeps, per playthrough: the number its results are drawn from ("casinoSeed"), and per game the
 * first and the last month it was played in ("casino<id>First", "...Last") and what the game of a month brought,
 * prize minus stake in cents ("casino<id>Month<month>"). */
#define CASINO_NONE (-9223372036854775807LL - 1) /* no such entry */

static const char *casino_key(char key[40], int id, const char *what, int month)
{
    if (month < 0)
        snprintf(key, 40, "casino%d%s", id, what);
    else
        snprintf(key, 40, "casino%d%s%d", id, what, month);
    return key;
}

static long long casino_get(int id, const char *what, int month)
{
    char key[40];
    return re_state_get(g_playthrough, casino_key(key, id, what, month), CASINO_NONE);
}

static void casino_set(int id, const char *what, int month, long long value)
{
    char key[40];
    re_state_set(g_playthrough, casino_key(key, id, what, month), value);
}

/* Made once per playthrough, from the clock: it only has to be something the player cannot know in the game. */
static unsigned long long casino_seed(void)
{
    long long stored = re_state_get(g_playthrough, "casinoSeed", 0);
    if (stored != 0)
        return (unsigned long long)stored;
    LARGE_INTEGER ticks;
    FILETIME now;
    QueryPerformanceCounter(&ticks);
    GetSystemTimeAsFileTime(&now);
    unsigned long long seed = (unsigned long long)ticks.QuadPart * 0x9E3779B97F4A7C15ull ^
                              (((unsigned long long)now.dwHighDateTime << 32) | now.dwLowDateTime) ^
                              ((unsigned long long)GetCurrentProcessId() << 17);
    if (seed == 0)
        seed = 1;
    re_state_set(g_playthrough, "casinoSeed", (long long)seed);
    return seed;
}

static const char *casino_name(const re_casino_game *game)
{
    return T(RE_MSG_CASINO_SLOTS + (int)(game - RE_CASINO));
}

/* The first hour of a casino game: its start goes on record, for the end to know that the game was begun under the
 * mod's rules (REQUEST.md [45], note m27). */
static void casino_start(BYTE *object)
{
    int id = re_readable(object, RE_OBJ_ID + 4) ? *(const int *)(object + RE_OBJ_ID) : 0;
    int now = g_main_obj != NULL ? re_game_period(g_main_obj) : -1;
    if (re_casino_game_of(id) == NULL || now < 0 || g_playthrough[0] == 0)
        return;
    casino_set(id, "Start", -1, now);
    if (g_on_trace)
        re_log("casino: activity %d begins in month %d", id, now);
}

/* The last hour of a casino game, before the game's own money step (which books nothing for it). The game has
 * just written the household's entry for this month. One game per game and month: the stake is what the lists
 * show, the result is the playthrough's for this month, and both go into the cash account as one amount. */
static void casino_end(BYTE *actions, BYTE *record, BYTE *object)
{
    void *main_obj = g_main_obj;
    int id = re_readable(object, RE_OBJ_ID + 4) ? *(const int *)(object + RE_OBJ_ID) : 0;
    const re_casino_game *game = re_casino_game_of(id);
    int now = main_obj != NULL ? re_game_period(main_obj) : -1;
    if (game == NULL || now < 0 || g_playthrough[0] == 0 || !re_readable(actions, RE_ACTION_PEOPLE + 4) ||
        !re_readable(record, RE_RECORD_COUNT + 4)) {
        re_log("casino: the end of activity %d could not be read (month %d), no result", id, now);
        return;
    }
    BYTE *finance = *(BYTE **)(actions + RE_ACTION_FINANCE), *prices = *(BYTE **)(actions + RE_ACTION_PRICES);
    BYTE *people = *(BYTE **)(actions + RE_ACTION_PEOPLE);
    char text[240], amount[72], times_text[16];
    long long last = casino_get(id, "Last", -1);
    if (last != CASINO_NONE && now <= last) {
        /* queued in the loaded save, which the game does not check again: the playthrough has played this month */
        re_game_set_cooldown(people, id, (int)last);
        re_log("casino: %s ends in month %d, but this playthrough has played it up to month %lld: no result",
               re_lang_text(RE_LANG_EN, RE_MSG_CASINO_SLOTS + (int)(game - RE_CASINO)), now, last);
        snprintf(text, sizeof text, T(RE_MSG_CASINO_PLAYED), casino_name(game));
        re_game_post_message(main_obj, text);
        return;
    }
    long long begun = casino_get(id, "Start", -1);
    if (begun == CASINO_NONE || begun > now || begun < now - 1) {
        /* its first hour was not seen: begun under the game's own rules, which book the result in that hour (a save
         * from before the mod). Booking a result of the mod's on top would pay the game twice */
        re_log("casino: %s ends in month %d and its start is not on record (%lld): begun before the mod, no result",
               re_lang_text(RE_LANG_EN, RE_MSG_CASINO_SLOTS + (int)(game - RE_CASINO)), now, begun == CASINO_NONE ? -1 : begun);
        snprintf(text, sizeof text, T(RE_MSG_CASINO_BEFORE), casino_name(game));
        re_game_post_message(main_obj, text);
        return;
    }
    casino_set(id, "Start", -1, CASINO_NONE);
    int count = *(const int *)(record + RE_RECORD_COUNT);
    long long shown = re_readable(prices, 4) ? re_game_activity_money(prices, object) : 0;
    long long stake = -shown * (count > 1 ? count : 1);
    if (stake <= 0 || !re_readable(finance, 0x380)) {
        re_log("casino: activity %d shows $%.2f as its money, or the cash account cannot be reached: no result", id, dollars(shown));
        return;
    }
    unsigned long long seed = casino_seed();
    double times = re_casino_times(game, seed, now);
    long long net = re_casino_net(stake, times);
    /* on disk before the cash moves: closing the game right after a lost one must not undo it */
    casino_set(id, "Month", now, net);
    casino_set(id, "Last", -1, now);
    if (casino_get(id, "First", -1) == CASINO_NONE)
        casino_set(id, "First", -1, now);
    ((money_fn)AT(RE_VA_CHANGE_MONEY))(finance, (unsigned)net, (int)(net >> 32), RE_TAG_GAMBLING, 0, 0, 0);
    re_log("casino: %s in month %d: stake $%.2f (the list shows $%.2f, the record's count is %d), draw %.6f, pays %gx, booked $%.2f; "
           "the household's entry is month %d",
           re_lang_text(RE_LANG_EN, RE_MSG_CASINO_SLOTS + (int)(game - RE_CASINO)), now, dollars(stake), dollars(shown), count,
           re_casino_unit(seed, id, now), times, dollars(net), re_game_cooldown(people, id));
    snprintf(times_text, sizeof times_text, "%g", times);
    if (times > 0.0)
        snprintf(text, sizeof text, T(RE_MSG_CASINO_WON), casino_name(game), times_text, money(main_obj, net, amount));
    else
        snprintf(text, sizeof text, T(RE_MSG_CASINO_LOST), casino_name(game), money(main_obj, stake, amount));
    re_game_post_message(main_obj, text);
}

/* After a load: the games this playthrough played after the loaded save was written. A game is in a save when the
 * household's entry for it is not older than the game's month, because the game's end step writes that entry in
 * the same hour, just before the plugin books (note b15, Q7). What is not in the save is booked again, as one
 * amount, and the entry is moved up; with keepResultsOnLoad=0 it is forgotten instead. Returns 1 when cash moved. */
static int casino_load(void *main_obj)
{
    BYTE *people = re_game_people(main_obj), *finance = re_game_finance(main_obj);
    int games = 0;
    long long total = 0;
    if (people == NULL || finance == NULL || g_playthrough[0] == 0) {
        re_log("casino: the household or the playthrough id could not be read, nothing looked at after this load");
        return 0;
    }
    for (int g = 0; g < RE_CASINO_GAMES; g++) {
        int id = RE_CASINO[g].id, saved = re_game_cooldown(people, id);
        long long last = casino_get(id, "Last", -1), first = casino_get(id, "First", -1);
        if (last == CASINO_NONE || saved == RE_COOLDOWN_UNREADABLE || saved >= last)
            continue;
        for (long long month = first != CASINO_NONE && first > saved ? first : (long long)saved + 1; month <= last; month++) {
            long long net = casino_get(id, "Month", (int)month);
            if (net == CASINO_NONE)
                continue;
            re_log("casino: activity %d, month %lld, $%.2f: not in the loaded save (its entry is month %d): %s", id, month, dollars(net),
                   saved, g_casino_keep ? "counts again" : "forgotten ([casino] keepResultsOnLoad=0)");
            if (g_casino_keep) {
                total += net;
                games++;
            } else
                casino_set(id, "Month", (int)month, CASINO_NONE);
        }
        if (g_casino_keep)
            re_game_set_cooldown(people, id, (int)last);
        else
            casino_set(id, "Last", -1, saved == RE_COOLDOWN_NONE ? CASINO_NONE : saved);
    }
    if (games == 0)
        return 0;
    if (total != 0)
        ((money_fn)AT(RE_VA_CHANGE_MONEY))(finance, (unsigned)total, (int)(total >> 32), RE_TAG_GAMBLING, 0, 0, 0);
    const char *prefix = T(RE_MSG_CASINO_BACK_PREFIX);
    char text[240], amount[72];
    snprintf(text, sizeof text, T(RE_MSG_CASINO_BACK), prefix, games, signed_money(main_obj, total, amount));
    panel_line(main_obj, prefix, text, "ActionGamblingSlots", 1);
    return total != 0;
}

static void casino_install(void)
{
    re_casino_str_free = AT(RE_VA_STR_FREE);
    re_casino_money = AT(RE_VA_ACTION_MONEY);
    re_casino_end = casino_end;
    re_casino_start = casino_start;
    /* the attributes last: without them a game keeps the game's own amount, with them and without the two calls it
     * would charge the stake and never pay */
    int end = re_patch_call(AT(RE_VA_CALL_MONEY_END), AT(RE_VA_ACTION_MONEY), re_casino_money_hook);
    int start = end && re_patch_call(AT(RE_VA_CALL_MONEY_START), AT(RE_VA_ACTION_MONEY), re_casino_money_hook);
    int attributes = start && re_detour5(AT(RE_VA_ATTRIBUTE), RE_GETTER_PROLOGUE, re_casino_attr_hook, &re_casino_attr_trampoline);
    g_casino_installed = attributes;
    re_log("casino: money call at the end %s, at the start %s, attribute detour %s; a game played after the loaded save: %s",
           end ? "redirected" : "NOT redirected", start ? "redirected" : "NOT redirected", attributes ? "installed" : "NOT installed",
           g_casino_keep ? "counts again" : "is forgotten");
    for (int g = 0; g < RE_CASINO_GAMES; g++)
        re_log("casino: %s (activity %d): stake $%.2f before the price index, %.1f%% of the stakes come back",
               re_lang_text(RE_LANG_EN, RE_MSG_CASINO_SLOTS + g), RE_CASINO[g].id, dollars(RE_CASINO[g].stake),
               100.0 * re_casino_return(&RE_CASINO[g]));
}

/* ------------------------------------------------------------ futures */

/* The game is about to work out the inflation rate of an industry list: the list is kept. */
static void futures_listed(void *economy, const unsigned char *first, const unsigned char *end)
{
    int count = 0;
    g_fut_count = -1;
    g_fut_economy = economy;
    if (end < first || (SIZE_T)(end - first) > RE_FUT_TAGS * RE_TAG_BYTES || !re_readable(first, (SIZE_T)(end - first)))
        return;
    for (const unsigned char *p = first; p != end; p += RE_TAG_BYTES, count++) {
        g_fut_tags[count].id = *(const int *)(p + RE_TAG_ID);
        g_fut_tags[count].share = *(const float *)(p + RE_TAG_SHARE);
    }
    g_fut_count = count;
}

/* The futures window's copy of its asset's rate, "4.30%": behind it goes what the game's rule makes the rate
 * average until the expiry the player has chosen. */
static void futures_value(unsigned char *string, const unsigned char *window)
{
    static re_fut_economy e; /* too large for the game's stack */
    if (!re_readable(window, RE_FUT_WINDOW_MONTHS + 4))
        return;
    int months = *(const int *)(window + RE_FUT_WINDOW_MONTHS);
    void *economy = *(void *const *)(window + RE_FUT_WINDOW_ECONOMY);
    unsigned size = *(const unsigned *)(string + 0x10), room = *(const unsigned *)(string + 0x14);
    char text[96];
    if (economy == NULL || economy != g_fut_economy || g_fut_count < 0 || months < 1 || months > 120 || size == 0 || size > 24 ||
        !re_game_economy(economy, &e))
        return;
    double expected = re_futures_expected_rate(&e, g_fut_tags, g_fut_count, months);
    memcpy(text, room >= 16 ? *(const char *const *)string : (const char *)string, size);
    int n = snprintf(text + size, sizeof text - size, T(RE_MSG_FUT_EXPECTED), 100.0 * expected);
    if (n <= 0 || (unsigned)n >= sizeof text - size)
        return;
    ((assign_fn)AT(RE_VA_STR_ASSIGN))(string, text, size + (unsigned)n);
    if (g_on_trace && g_fut_lines++ < FUTURES_TRACE_LINES) {
        text[size] = 0;
        re_log("futures: window, expiry in %d months, the game shows %s: expected average %.5f (%d industries of the asset, first %d with share %.3f)",
               months, text, expected, g_fut_count, g_fut_count ? g_fut_tags[0].id : 0, g_fut_count ? g_fut_tags[0].share : 0.0);
    }
}

/* Test runs: what the rule said a month end would do to every rate, next to what the game did. */
static void futures_month_trace(const re_fut_economy *before, const re_fut_economy *after)
{
    re_fut_economy rule = *before;
    re_futures_step(&rule);
    for (int i = 0; i < rule.count; i++)
        for (int j = 0; j < after->count; j++)
            if (after->industry[j].id == rule.industry[i].id)
                re_log("futures: month end, industry %d: rate %.6f, the rule without its noise %.6f, the game %.6f; index %.6f, the game %.6f",
                       rule.industry[i].id, before->industry[i].rate, rule.industry[i].rate, after->industry[j].rate,
                       before->industry[i].index, after->industry[j].index);
    re_log("futures: month end, overall: rate %.6f, the rule %.6f, the game %.6f; index %.6f, the game %.6f; growth %.6f, after %.6f",
           before->overall_rate, rule.overall_rate, after->overall_rate, before->overall_index, after->overall_index, before->growth,
           after->growth);
}

/* ---- the list of the futures tab (REQUEST.md [44], note m19 "Q5") */

/* The list on the left of the futures tab names a ware and nothing else, and its second sort button orders the
 * wares by what a unit costs, which says nothing about a contract. What a contract turns on is the ware's yearly
 * price change (the "asset inflation rate" the right-hand side shows for the ware that is chosen) and where the
 * game's rule takes it. So: the rate goes behind every name with what it is expected to average over the next
 * twelve months, the second button sorts by the difference of the two (REQUEST.md [45]; it sorted by the rate), and
 * the hover text of a row has the rate with what it is expected to average until each of the three expiries. */
#define FUT_WARES 512

/* A ware's yearly price change now, by the game's own function. 0 = the economy cannot be reached. 2 = the
 * ware's industries are in g_fut_tags as well, kept by the stand-in that the game's function passes on its way
 * (futures_listed); 1 = the rate alone (the game's function did not come by there for this ware). */
static int futures_rate(int ware, double *now)
{
    void *economy = g_main_obj != NULL ? re_game_economy_object(g_main_obj) : NULL;
    if (!g_fut_installed || economy == NULL)
        return 0;
    g_fut_count = -1;
    *now = re_game_ware_rate(economy, ware);
    return g_fut_count >= 0 && g_fut_economy == economy ? 2 : 1;
}

/* What the list is about (REQUEST.md [45]): where the game's rule takes a ware's rate, against the rate now. The
 * expectation is that of one expiry, the middle one of the three the hover text of a row gives. Returns what
 * futures_rate returns; with 1 (no industries known) the expectation is the rate itself. */
#define FUT_OUTLOOK_MONTHS 12
static int futures_outlook(int ware, double *now, double *expected)
{
    static re_fut_economy e; /* too large for the game's stack */
    int got = futures_rate(ware, now);
    if (got == 2 && !re_game_economy(g_fut_economy, &e))
        got = 1;
    *expected = got == 2 ? re_futures_expected_rate(&e, g_fut_tags, g_fut_count, FUT_OUTLOOK_MONTHS) : *now;
    return got;
}

/* In place of the list's sort by unit value: the ware whose rate is expected to rise most first, the one expected
 * to fall most last; wares of one number keep the game's order. A ware without an expectation counts as one that
 * stays where it is. The game's call has the count and a comparison on the stack and removes them itself. */
__attribute__((force_align_arg_pointer)) static void __fastcall futures_sort_wrapper(int *first, int *last)
{
    static double rate[FUT_WARES], ahead[FUT_WARES];
    int n = (int)(last - first), unknown = 0, blind = 0;
    if (n < 1 || n > FUT_WARES || !re_readable(first, (SIZE_T)n * sizeof *first)) {
        re_log("futures: the list by the expected change of the rate: %d wares, NOT sorted", n);
        return;
    }
    for (int i = 0; i < n; i++) {
        double expected = 0.0;
        int got = futures_outlook(first[i], &rate[i], &expected);
        if (!got)
            rate[i] = expected = 0.0;
        ahead[i] = expected - rate[i];
        unknown += !got;
        blind += got == 1;
    }
    if (unknown == n) {
        re_log("futures: the list by the expected change of the rate: no rate could be worked out for %d wares, NOT sorted", n);
        return;
    }
    for (int i = 1; i < n; i++) {
        int ware = first[i], at = i;
        double r = rate[i], a = ahead[i];
        for (; at > 0 && ahead[at - 1] < a; at--) {
            first[at] = first[at - 1];
            rate[at] = rate[at - 1];
            ahead[at] = ahead[at - 1];
        }
        first[at] = ware;
        rate[at] = r;
        ahead[at] = a;
    }
    re_log("futures: the list by the expected change of the rate (%d months): %d wares, %d whose rate could not be worked out, %d "
           "without an expectation, from %+.4f to %+.4f",
           FUT_OUTLOOK_MONTHS, n, unknown, blind, ahead[0], ahead[n - 1]);
    for (int i = 0; g_on_trace && i < n; i++)
        re_log("futures (trace): list place %d: ware %d, rate %+.6f, expected %+.6f, ahead %+.6f", i + 1, first[i], rate[i],
               rate[i] + ahead[i], ahead[i]);
}

/* The row of a ware is being made (FUN_00611180, which also makes rows elsewhere): the game's name function writes
 * the ware's name where the row's title is taken from, and in the futures list the yearly price change goes behind
 * it. */
__attribute__((force_align_arg_pointer)) static void *__fastcall futures_item_name_wrapper(void *info, void *unused, void *title)
{
    typedef void *(__thiscall * name_fn)(void *info, void *string);
    static int lines;
    (void)unused;
    void *result = ((name_fn)AT(RE_VA_INFO_NAME))(info, title);
    const BYTE *maker = *(const BYTE *const *)__builtin_frame_address(0); /* the frame of the row's maker */
    const BYTE *s = (const BYTE *)title;
    double now = 0.0, expected = 0.0;
    char text[160];
    if (!re_readable(maker, RE_FUT_ITEM_FRAME_ASSET + 4) || va_of(*(const unsigned *)(maker + 4)) != RE_VA_FUT_ITEM_RET ||
        !re_readable(s, 0x18))
        return result;
    int ware = *(const int *)(maker + RE_FUT_ITEM_FRAME_ASSET);
    unsigned size = *(const unsigned *)(s + 0x10), room = *(const unsigned *)(s + 0x14);
    const char *shown = room >= 16 ? *(const char *const *)(const void *)s : (const char *)s;
    int got = size == 0 || size > 80 || !re_readable(shown, size) ? 0 : futures_outlook(ware, &now, &expected);
    if (!got)
        return result;
    memcpy(text, shown, size);
    int added = got == 2 ? snprintf(text + size, sizeof text - size, T(RE_MSG_FUT_ITEM_AHEAD), 100.0 * now, 100.0 * expected)
                         : snprintf(text + size, sizeof text - size, T(RE_MSG_FUT_ITEM), 100.0 * now);
    if (added > 0 && (unsigned)added < sizeof text - size)
        ((assign_fn)AT(RE_VA_STR_ASSIGN))(title, text, size + (unsigned)added);
    if (g_on_trace && lines++ < 200)
        re_log("futures: row of ware %d: '%.*s'", ware, (int)(size + (unsigned)(added > 0 ? added : 0)), text);
    return result;
}

/* The hover text of a row, "Click to view / Ctrl-click to add a shortcut": above it the ware's rate and what the
 * rule makes it average until each expiry. The ware is in the frame of the list's builder. */
static unsigned futures_list_edit(char *text, unsigned len, unsigned cap, const re_text_regs *regs)
{
    static re_fut_economy e; /* too large for the game's stack */
    const int *slot = (const int *)(UINT_PTR)(regs->ebp + RE_FUT_LIST_FRAME_ASSET);
    double now = 0.0, expected[3] = {0.0, 0.0, 0.0};
    char line[400];
    int got = re_readable(slot, 4) ? futures_rate(*slot, &now) : 0;
    if (!got)
        return len;
    if (got == 2 && !re_game_economy(g_fut_economy, &e))
        got = 1;
    for (int i = 0; got == 2 && i < 3; i++)
        expected[i] = re_futures_expected_rate(&e, g_fut_tags, g_fut_count, 6 << i); /* 6, 12 and 24 months */
    int n = snprintf(line, sizeof line, T(RE_MSG_FUT_ROW_TIP), 100.0 * now, 100.0 * expected[0], 100.0 * expected[1], 100.0 * expected[2]);
    if (n > 0 && got != 2 && strchr(line, '\n') != NULL) /* no industries known: the rate alone, the first line */
        n = (int)(strchr(line, '\n') - line);
    if (n <= 0 || (unsigned)n >= sizeof line || (unsigned)n + 1 + len >= cap)
        return len;
    memmove(text + n + 1, text, len);
    memcpy(text, line, (size_t)n);
    text[n] = '\n';
    return (unsigned)n + 1 + len;
}

static void futures_install(void)
{
    re_futures_list = AT(RE_VA_RATE_LIST);
    re_futures_copy = AT(RE_VA_STR_COPY);
    re_futures_listed = futures_listed;
    re_futures_value = futures_value;
    int list = re_patch_call(AT(RE_VA_CALL_RATE_LIST), AT(RE_VA_RATE_LIST), re_futures_list_hook);
    g_fut_installed = list && re_patch_call(AT(RE_VA_CALL_FUT_VALUE), AT(RE_VA_STR_COPY), re_futures_value_hook);
    re_log("futures: the industries of a rate %s, the rate's label in the futures window %s", list ? "redirected" : "NOT redirected",
           g_fut_installed ? "redirected" : "NOT redirected");
    if (g_fut_installed && g_fut_list) {
        g_fut_list_hooked = re_patch_call(AT(RE_VA_CALL_FUT_SORT_VALUE), AT(RE_VA_FUT_SORT_VALUE), futures_sort_wrapper) &&
                            re_patch_call(AT(RE_VA_CALL_FUT_ITEM_NAME), AT(RE_VA_INFO_NAME), futures_item_name_wrapper);
        re_log("futures: the list of the tab has a ware's yearly price change behind its name and is sorted by the change expected of "
               "it with the second button: the sort and the name of a row %s",
               g_fut_list_hooked ? "redirected" : "NOT redirected");
    }
}

/* ---- candidates that a load does not change (REQUEST.md [30], [43]; note b21) */

/* The game is about to be asked for the candidates of a job. When it keeps no list for that job it draws one now,
 * from its random streams as they stand, so that the hours played and the clicks made before the look decide who
 * comes: a load and a different click give other people or other wages. Here the streams are kept and seeded from
 * what the list belongs to - the playthrough, the month, the business and the job - and list_end puts them back:
 * the same look gives the same list whatever was done before it, and the look moves no stream. The candidates of a
 * job change with the month and with nothing else. */
static re_streams g_streams; /* too large for the game's stack */
static struct {
    int on, firm, job, month, wages;
    unsigned long long seed;
} g_making;

static void list_begin(void *firms, int firm, int job)
{
    static int unread;
    if (!g_fixed_hooked || g_making.on)
        return;
    int kept = re_game_candidates(firms, firm, job), month = g_main_obj != NULL ? re_game_period(g_main_obj) : -1;
    if (kept >= 0)
        return; /* the game has the list and draws nothing */
    BYTE *randgen = kept == RE_CANDIDATES_NONE && month >= 0 && g_playthrough[0] != 0 ? re_game_randgen(firms) : NULL;
    if (randgen == NULL) {
        if (unread++ < 5)
            re_log("business: candidates of firm %d, job %d: the lists the game keeps, the month, the playthrough id or the random "
                   "streams cannot be read; a list drawn now is drawn as the game draws it",
                   firm, job);
        return;
    }
    g_making.on = 1;
    g_making.firm = firm;
    g_making.job = job;
    g_making.month = month;
    g_making.wages = 0;
    g_making.seed = re_business_list_seed(g_playthrough, month, firm, job, 0);
    /* The generator of a person fetches texts (0x00432990), and every text passes the plugin's hook, where a look
     * at the household may be due: that look waits until the streams are the game's again. */
    InterlockedExchange(&g_refreshing, 1);
    re_business_streams_seed(&g_streams, randgen, AT(RE_VA_RAND_SEED), g_making.seed);
}

/* The generator of a person asks for the reference wage, a draw the game seeds with the person's id. The id counts
 * everything the game has numbered so far, so in a list made from a seed the draw gets a seed of the list's. A
 * negative one would mean "no draw" to the game's function. */
static void list_wage_seed(int *seed)
{
    if (g_making.on)
        *seed = (int)(re_business_stream_seed(g_making.seed, RE_RAND_STREAMS + g_making.wages++) & 0x7fffffffu);
}

/* The list is made: the streams go back, and the log says what the list was drawn from and whether the streams
 * are what they were. */
static void list_end(BYTE *const *vector)
{
    unsigned drawn[RE_RAND_STREAMS], others = 0;
    if (!g_making.on)
        return;
    int same = re_business_streams_restore(&g_streams, drawn);
    int count = re_readable(vector, 12) && vector[1] >= vector[0] ? (int)((SIZE_T)(vector[1] - vector[0]) / RE_STAFF_BYTES) : -1;
    InterlockedExchange(&g_refreshing, 0);
    g_making.on = 0;
    for (int i = 1; i < RE_RAND_STREAMS; i++)
        others += i != RE_RANDGEN_JOB / RE_RAND_UNIT_BYTES ? drawn[i] : 0;
    re_log("business: candidates of firm %d, job %d, month %d: drawn from seed %016llx, %d candidate(s), %d reference wage(s) "
           "seeded; draws taken from the job stream %u, the first stream %u, the ten others %u; the game's streams after it: %s",
           g_making.firm, g_making.job, g_making.month, g_making.seed, count, g_making.wages, drawn[RE_RANDGEN_JOB / RE_RAND_UNIT_BYTES],
           drawn[0], others, same ? "as before, all thirteen engines and twelve call counters" : "MISMATCH, not everything could be put back");
}

/* The two ends of the hire tab's fetch of its list. Before the game's function: the streams. After it: the streams
 * back, then the order of the tab's own copy. */
static void candidates_listing(void *firms, int firm, int job)
{
    list_begin(firms, firm, job);
}

static void candidates_listed(void *firms, unsigned char **vector)
{
    list_end(vector);
    business_listed(firms, vector);
}

/* ---- a month end and a listing price that a load does not change (REQUEST.md [55]; note m37) */

/* At the month end the game draws the economy's growth, the industries' cycles and the listed companies' month - and,
 * when the "create a public company" window is built, the twelve steps of a listing price - from its random streams
 * as they stand. Anything done since the save was loaded that drew from a stream gives another result for the same
 * month: with shares or futures held, a poor month end can be loaded away without a single trade, which the trade
 * lock never sees (run599: one search for a property, and the shares of save "55" end the month $52,214 lower).
 * Here the streams are kept and seeded from what the draws belong to - the playthrough, the month, which of the two
 * it is, and the guard's generation, which a rollback beyond the lock's cap raises - and put back afterwards. So a
 * month end and a month's listing price are the same whatever was done before them, they move no stream, and what
 * was seen beyond the cap does not come again: for the economy too, which the move of the market stream at a load
 * never reached.
 * The same for the property market's month start (REQUEST.md [56]): the game empties the list of properties for
 * sale and fills it again, and makes the month's offer for a property the household sells or lets, all from the
 * market stream as it stands (run618: the stream seven draws further, and the same three addresses of the new month
 * ask $1.4, $0.8 and $1.2 million instead of $1.9, $1.6 and $2.3 million). One seed for that whole routine is not
 * enough: it walks the sites one after the other, and a search of the character makes a new site, after which the
 * same draws fall to other sites (run703: after one search two of the month's three properties had moved to another
 * street and the third was another one). So inside it every site of the list for rent and of the list for sale is
 * drawn from a seed of its own, and so are the offers that come after the two lists (property_draws). */
enum { FIXED_MONTH_END, FIXED_LISTING, FIXED_PROPERTY, FIXED_SITE_SALE, FIXED_SITE_RENT, FIXED_OFFERS };
#define FIXED_NO_FIRM (-1) /* in the place of a business's id: no list of candidates is drawn from such a seed */
#define FIXED_SITE(id) (-2 - (id)) /* a site in that place: below FIXED_NO_FIRM for every site id from 0 on */
static re_streams g_fixed_streams; /* too large for the game's stack */
static struct {
    int on, what, month, generation;
    int sites[2], offers; /* the property market's month start: sites seeded for sale and for rent, the offers */
    int site_low, site_high; /* ... and the lowest and the highest of the sites' numbers */
    LONG refreshing;
    unsigned long long seed;
} g_fixed;

static void fixed_draws(int what, int drawn)
{
    static const char *const names[] = {"the month end's economy and shares", "the price steps of a listing",
                                        "the property market's month start"};
    const int hooked[] = {g_month_hooked, g_listing_hooked, g_property_month_hooked};
    static int unread;
    if (!drawn) {
        /* a listing inside the month end's draws, or inside a list of candidates, is drawn from that seed already */
        if (g_fixed.on || g_making.on || !hooked[what])
            return;
        int month = g_main_obj != NULL ? re_game_period(g_main_obj) : -1;
        unsigned *seed, *calls;
        void *market;
        BYTE *randgen = month >= 0 && g_playthrough[0] != 0 ? re_game_randgen(re_game_firms(g_main_obj)) : NULL;
        /* the streams the economy reaches have to be the ones whose thirteen engines were just checked */
        if (randgen == NULL || !re_game_market_stream(g_main_obj, &seed, &calls, &market) ||
            (BYTE *)market != randgen + RE_RANDGEN_MARKET) {
            if (unread++ < 5)
                re_log("guard: %s: the month, the playthrough id or the random streams cannot be read; drawn as the game draws them",
                       names[what]);
            return;
        }
        g_fixed.on = 1;
        g_fixed.what = what;
        g_fixed.month = month;
        g_fixed.sites[0] = g_fixed.sites[1] = g_fixed.offers = 0;
        g_fixed.generation = (int)re_state_get(g_playthrough, "guardGeneration", 0);
        g_fixed.seed = re_business_list_seed(g_playthrough, month, FIXED_NO_FIRM, what, g_fixed.generation);
        /* as for a list of candidates: a look at the household waits until the streams are the game's again */
        g_fixed.refreshing = InterlockedExchange(&g_refreshing, 1);
        re_business_streams_seed(&g_fixed_streams, randgen, AT(RE_VA_RAND_SEED), g_fixed.seed);
        return;
    }
    if (!g_fixed.on || g_fixed.what != what)
        return;
    const int economy = RE_RANDGEN_ECONOMY / RE_RAND_UNIT_BYTES, market = RE_RANDGEN_MARKET / RE_RAND_UNIT_BYTES;
    unsigned taken[RE_RAND_STREAMS], others = 0;
    int same = re_business_streams_restore(&g_fixed_streams, taken);
    InterlockedExchange(&g_refreshing, g_fixed.refreshing);
    g_fixed.on = 0;
    for (int i = 0; i < RE_RAND_STREAMS; i++)
        others += i != economy && i != market ? taken[i] : 0;
    if (!same || what != FIXED_LISTING || g_on_trace) /* a window draws a listing price at every change of it */
        re_log("guard: %s of month %d, generation %d: drawn from seed %016llx; draws taken from the economy stream %u, the market "
               "stream %u, the ten others %u; the game's streams after it: %s",
               names[what], g_fixed.month, g_fixed.generation, g_fixed.seed, taken[economy], taken[market], others,
               same ? "as before, all thirteen engines and twelve call counters" : "MISMATCH, not everything could be put back");
    if (what == FIXED_PROPERTY)
        re_log("guard: inside it a seed of its own for each of %d site(s) of the list for rent and %d of the list for sale "
               "(numbered %d to %d), and for the offers after the two lists: %s",
               g_fixed.sites[1], g_fixed.sites[0], g_fixed.site_low, g_fixed.site_high, g_fixed.offers ? "yes" : "no");
}

/* Inside the property market's month start, while its streams are the plugin's: they start again from a seed of the
 * part that is about to draw - one site of one of the two lists, or the offers. */
static void property_draws(int part, int site)
{
    if (!g_fixed.on || g_fixed.what != FIXED_PROPERTY)
        return;
    re_business_streams_reseed(g_fixed_streams.randgen, AT(RE_VA_RAND_SEED),
                               re_business_list_seed(g_playthrough, g_fixed.month, site, part, g_fixed.generation));
    if (part == FIXED_OFFERS)
        g_fixed.offers = 1;
    else
        g_fixed.sites[part == FIXED_SITE_RENT]++;
}

static void site_draws(int rent, const BYTE *node)
{
    if (!g_fixed.on || g_fixed.what != FIXED_PROPERTY || !re_readable(node, RE_SITE_NODE_ID + 4))
        return;
    int id = *(const int *)(node + RE_SITE_NODE_ID);
    if (g_fixed.sites[0] + g_fixed.sites[1] == 0 || id < g_fixed.site_low)
        g_fixed.site_low = id;
    if (g_fixed.sites[0] + g_fixed.sites[1] == 0 || id > g_fixed.site_high)
        g_fixed.site_high = id;
    property_draws(rent ? FIXED_SITE_RENT : FIXED_SITE_SALE, FIXED_SITE(id));
}

static void month_draws(int moment)
{
    if (moment == RE_DRAWS_PROPERTY_OFFERS)
        property_draws(FIXED_OFFERS, FIXED_NO_FIRM);
    else
        fixed_draws(moment < RE_DRAWS_PROPERTY ? FIXED_MONTH_END : FIXED_PROPERTY,
                    moment == RE_DRAWS_MONTH_END_DONE || moment == RE_DRAWS_PROPERTY_DONE);
}

static void listing_draws(int drawn)
{
    fixed_draws(FIXED_LISTING, drawn);
}

/* ---- the wage demand of a person under the game's automatic management (REQUEST.md [33], [38], notes b19 "B2", b20) */

/* The month end's staff routine is about to say yes to the wage demand of a person who has the switch; it asks
 * nobody. Here the demand is left open instead, which is what the routine itself does with the demand of a person
 * without the switch: the game makes such a person leave at the NEXT month end, at no cost. Just before that month
 * end the answer is given (wage_answers): by then the job's candidates of the month are there to compare with, and
 * a yes given then raises the wage for the same month a yes given now would. Nobody is added or removed here, the
 * routine is in the middle of its walk over that staff. */
__attribute__((force_align_arg_pointer)) static void __fastcall wage_wrapper(void *staff, void *unused, int firm, int id, unsigned lo, int hi)
{
    typedef void(__thiscall *set_fn)(void *staff, int firm, int id, unsigned lo, int hi);
    (void)unused;
    void *firms = re_game_firms(g_main_obj);
    BYTE *record = firms != NULL ? re_game_employee(firms, firm, id) : NULL;
    long long demanded = (long long)(((unsigned long long)(unsigned)hi << 32) | lo);
    if (record == NULL) { /* an employee that cannot be found gets what the game gives */
        ((set_fn)AT(RE_VA_STAFF_SET_WAGE))(staff, firm, id, lo, hi);
        re_log("business: wage demand, firm %d, employee %d: %lld cents a month asked; the employee could not be read, accepted as the "
               "game does",
               firm, id, demanded);
        return;
    }
    *(long long *)(record + RE_STAFF_DEMAND) = demanded;
    re_log("business: wage demand, firm %d, employee %d: %lld -> %lld cents a month; left open, answered before the next month end", firm,
           id, *(const long long *)(record + RE_STAFF_WAGE), demanded);
}

/* The people who take a place when the month end is over: the game's own copy of each candidate's record, made
 * before the month end throws the candidates away. */
#define WAGE_PLACES 16
#define WAGE_CANDIDATES 16
typedef struct {
    int firm, leaver, id;
    int dismiss; /* 1 = the mod lets the leaver go itself (a place given to a person of fewer hours) */
    char name[96], leaver_name[96];
} wage_place;
static wage_place g_wage_places[WAGE_PLACES];
static BYTE g_wage_records[WAGE_PLACES][RE_STAFF_BYTES] __attribute__((aligned(16)));
static int g_wage_placed;

/* Just before a month end: every open wage demand of a person under the automatic management is answered. Left
 * open, the month end makes the person leave. So the job's candidates of the month are looked at - the list the
 * hire tab shows, made now with the draws that tab would make when nobody has opened it this month. When one of
 * them can take the place and costs less for an hour's worth of work by more than the margin, the demand stays open
 * and a copy of that candidate's record is kept for wage_places. Otherwise the demand is met now. */
static void wage_answers(void *main_obj)
{
    typedef void(__thiscall *list_fn)(void *firms, BYTE **list, int firm, int job);
    typedef void(__thiscall *list_end_fn)(BYTE **list);
    static re_firm_fit own[OWN_FIRMS];
    static re_staff_read c[WAGE_CANDIDATES];
    static re_job_hours work[32];
    void *firms = re_game_firms(main_obj), *jobs = re_game_jobs(firms);
    int owned = re_game_firm_fits(main_obj, own, OWN_FIRMS);
    for (int f = 0; f < owned; f++) {
        BYTE *first = NULL;
        int count = re_game_firm_staff(firms, own[f].id, &first), kinds = re_game_firm_jobs(firms, own[f].id, work, 32);
        for (int i = 0; i < count; i++) {
            re_staff_read s;
            if (!re_game_staff_read(jobs, first + (SIZE_T)i * RE_STAFF_BYTES, &s) || s.demand <= s.wage ||
                !re_game_staff_auto(firms, own[f].id, s.id))
                continue;
            if (!own[f].open) {
                /* closed: there was no work to measure the place by, and every candidate would look good enough */
                re_game_staff_set_wage(firms, own[f].id, s.id, s.demand);
                re_log("business: '%s' (firm %d): %s (employee %d, job %d) is paid %lld cents a month and asked for %lld; the business "
                       "is closed and had no work to measure the place by: the raise is given, as the game's own management gives it",
                       own[f].name, own[f].id, s.name, s.id, s.job, s.wage, s.demand);
                continue;
            }
            /* A person the job did not need this month (REQUEST.md [38]: leaving over a demand costs no severance):
             * the others of the job, those who are staying, left unworked at least as much work as this person
             * did. Then the rise is not given and nobody is hired for the place. */
            double did = (s.hours - s.left) * s.efficiency, others = 0.0, month = 0.0;
            int mates = 0;
            for (int k = 0; g_wage_spare && k < count; k++) {
                re_staff_read o;
                if (k == i || !re_game_staff_read(jobs, first + (SIZE_T)k * RE_STAFF_BYTES, &o) || o.job != s.job || o.demand > o.wage)
                    continue;
                mates++;
                others += o.left * o.efficiency;
                month += o.hours * o.efficiency;
            }
            if (re_business_wage_spare(did, others, month, mates)) {
                re_log("business: '%s' (firm %d): %s (employee %d, job %d) is paid %lld cents a month and asked for %lld; did %.0f hours' "
                       "worth of work this month and the %d other(s) of the job left %.0f unworked: not needed, leaves in this month "
                       "end and nobody is hired for the place",
                       own[f].name, own[f].id, s.name, s.id, s.job, s.wage, s.demand, did, mates, others);
                continue;
            }
            BYTE *list[3] = {NULL, NULL, NULL};
            list_begin(firms, own[f].id, s.job); /* the list the hire tab would show in this month */
            long long listed = time_now();
            ((list_fn)AT(RE_VA_HIRE_LIST))(firms, list, own[f].id, s.job);
            time_add(TIME_HIRE_LIST, listed);
            list_end(list);
            SIZE_T bytes = (SIZE_T)(list[1] - list[0]);
            int n = list[1] > list[0] && bytes % RE_STAFF_BYTES == 0 && re_readable(list[0], bytes) ? (int)(bytes / RE_STAFF_BYTES) : 0;
            long long wage[WAGE_CANDIDATES];
            int hours[WAGE_CANDIDATES];
            /* what the job wants from this place: the work the person did and the work the job left undone */
            double efficiency[WAGE_CANDIDATES], fresh = 0.0, need = did;
            for (int k = 0; k < kinds; k++)
                if (work[k].job == s.job && work[k].allowed + work[k].overtime > work[k].worked)
                    need += work[k].allowed + work[k].overtime - work[k].worked;
            n = n > WAGE_CANDIDATES ? WAGE_CANDIDATES : n;
            for (int k = 0; k < n; k++) {
                int taken = !re_game_staff_read(jobs, list[0] + (SIZE_T)k * RE_STAFF_BYTES, &c[k]);
                for (int p = 0; !taken && p < g_wage_placed; p++) /* promised to another place in this month end */
                    taken = g_wage_places[p].firm == own[f].id && g_wage_places[p].id == c[k].id;
                wage[k] = taken ? 0 : c[k].wage; /* no wage: cannot be priced, is not picked */
                hours[k] = taken ? 0 : c[k].hours;
                efficiency[k] = taken ? 0.0 : c[k].efficiency;
            }
            int pick = re_business_wage_pick(wage, hours, efficiency, n, s.hours, s.efficiency, need, &fresh);
            re_wage_costs costs = {0.0, 0.0};
            int cheaper = pick < 0 || re_business_wage_accept(s.demand, s.hours, s.efficiency, need, fresh, g_wage_margin, &costs);
            int accept = pick < 0 || g_wage_placed == WAGE_PLACES || (cheaper && !g_wage_refuse_all);
            char found[220] = "nobody among them can take the place";
            if (pick >= 0)
                snprintf(found, sizeof found,
                         "%s (candidate %d, %lld cents a month for %d hours at efficiency %.4f) would do an hour of it for %.1f cents",
                         c[pick].name, c[pick].id, c[pick].wage, c[pick].hours, c[pick].efficiency, costs.fresh);
            re_log("business: '%s' (firm %d): %s (employee %d, job %d, %d hours at efficiency %.4f) is paid %lld cents a month and asked for "
                   "%lld; the job wants %.0f hours' worth of work from the place (%.0f done by this person, the rest left undone by the "
                   "job): an hour of it %.1f cents with the raise; %d candidate(s), %s: %s",
                   own[f].name, own[f].id, s.name, s.id, s.job, s.hours, s.efficiency, s.wage, s.demand, need, did,
                   re_business_need_cost(s.demand, s.hours * s.efficiency, need < RE_NEED_LEAST ? RE_NEED_LEAST : need), n, found,
                   accept ? "the raise is given" : cheaper ? "refused by the test setting, leaves in this month end" : "refused, leaves in this month end");
            if (accept)
                re_game_staff_set_wage(firms, own[f].id, s.id, s.demand);
            else {
                wage_place *p = &g_wage_places[g_wage_placed];
                re_game_staff_copy(g_wage_records[g_wage_placed++], list[0] + (SIZE_T)pick * RE_STAFF_BYTES);
                p->firm = own[f].id;
                p->leaver = s.id;
                p->id = c[pick].id;
                p->dismiss = 0;
                memcpy(p->name, c[pick].name, sizeof p->name);
                memcpy(p->leaver_name, s.name, sizeof p->leaver_name);
            }
            ((list_end_fn)AT(RE_VA_STAFF_LIST_END))(list); /* the copies the list function made for this look */
        }
    }
}

/* ---- a staff that fits the work (REQUEST.md [38], [42]; note b20 "People a job does not need") */

/* The people the mod lets go when the month end is over, without a successor. */
static struct {
    int firm, id;
    char name[96];
} g_releases[WAGE_PLACES];
static int g_released;

/* For how many month ends in a row something has held for a job of a business. Kept in the state file as
 * month x 100 + count: a count of an older month, or of a save loaded from before, starts again; the same month
 * end met twice (a load) counts once. `holds` 0 ends the row. */
static int months_running(const char *what, int month, int firm, int job, int holds)
{
    char key[48];
    snprintf(key, sizeof key, "%sf%dj%d", what, firm, job);
    long long kept = re_state_get(g_playthrough, key, 0);
    int count = !holds ? 0 : kept / 100 == month ? (int)(kept % 100) : kept / 100 == month - 1 ? (int)(kept % 100) + 1 : 1;
    count = count > 99 ? 99 : count;
    if (holds || kept != 0)
        re_state_set(g_playthrough, key, holds ? (long long)month * 100 + count : 0);
    return count;
}

/* Just before a month end, after the wage demands are answered: the hours every person did not work this month are
 * still in the records. Nothing is forecast; what is measured has to hold for `spareMonths` month ends in a row.
 * Only people under the automatic management are touched, and nobody who is leaving over a wage demand. One thing
 * is known and not measured yet, the work of a contract signed in the running month: a business with one lets
 * nobody go in that month end, and the next, which has that work in its hours, decides.
 *
 * A job of two or more: somebody whose work the others could have done in the hours they left unworked is one too
 * many. The one whose hour of work costs most is let go after the month end: the month is worked and paid as the
 * game pays it, and no severance is paid (the user's choice, [42]).
 *
 * A job of one who leaves half the hours or more unworked: when the job's candidates have a person of fewer hours
 * and a lower wage who gets a quarter more done than was done this month, that person takes the place after the
 * month end and the present one is let go the same way. */
#define STAFF_MAX 64
#define SIZING_LOOKS 2 /* jobs of one whose candidates are looked at in one month end */
#define SIGNED_MAX 32
static int g_trace_contracts_unseen; /* a test knob: the rule below as it was before it looked at contracts */

/* The contracts a business signed in the running month (REQUEST.md [47], note m32). The game gives a month its work
 * at the month start, from the contracts signed by then: what was signed since is in none of this month's hours and
 * in all of the next month's. Returns how many, and what their listed jobs bring a month. */
static int signed_lately(void *firms, int firm, int month, int *hours)
{
    static re_signed s[SIGNED_MAX];
    int n = g_trace_contracts_unseen ? 0 : re_game_firm_signed(firms, firm, s, SIGNED_MAX), fresh = 0;
    *hours = 0;
    for (int i = 0; i < n; i++)
        if (s[i].start >= month) {
            fresh++;
            *hours += s[i].hours;
        }
    return fresh;
}

static void contracts_trace(void *main_obj, const char *when)
{
    static re_firm_fit own[OWN_FIRMS];
    static re_signed s[SIGNED_MAX];
    void *firms = re_game_firms(main_obj);
    int owned = re_game_firm_fits(main_obj, own, OWN_FIRMS), month = re_game_period(main_obj);
    for (int f = 0; f < owned; f++) {
        char line[800] = "";
        int n = re_game_firm_signed(firms, own[f].id, s, SIGNED_MAX), at = 0;
        for (int i = 0; i < n && (size_t)at < sizeof line; i++)
            at += snprintf(line + at, sizeof line - (size_t)at, " %d of type %d, signed in month %d for %d months, %d hours a month;",
                           s[i].id, s[i].type, s[i].start, s[i].months, s[i].hours);
        re_log("business (trace, %s): '%s' (firm %d) in month %d: %d signed contract(s):%s", when, own[f].name, own[f].id, month, n, line);
        re_premises p;
        double used = 0.0, most = 0.0, reputation = 0.0;
        int influence = 0;
        if (re_game_firm_premises(firms, own[f].id, &p) && re_game_firm_space(firms, own[f].id, &used, &most))
            re_log("business (trace, %s): '%s' (firm %d): its premises '%s' (property %d of type %d) are %s, floor space %.2f in use of "
                   "%.2f; they grow into type %d (0: the largest of their kind), the star holds %d, the month end would charge %lld cents "
                   "for growing; contracts %s, floor space %s, the whole %s",
                   when, own[f].name, own[f].id, p.address, p.house, p.type, p.owned ? "owned" : "rented", used, p.space, p.next, p.growing,
                   p.cost, business_kept("contracts", own[f].id) ? "the mod's" : "the player's",
                   business_kept("premises", own[f].id) ? "the mod's" : "the player's",
                   business_kept("all", own[f].id) ? "handed over" : "not handed over");
        if (re_game_firm_reputation(firms, own[f].id, &reputation, &influence))
            re_log("business (trace, %s): '%s' (firm %d) in month %d: reputation %.4f, its influence operation is %s; the business is %s",
                   when, own[f].name, own[f].id, month, reputation, influence ? "on" : "off", own[f].open ? "open" : "closed");
    }
}

static int filled_in(int month, int firm, int id); /* staff_fill's people, below */
static void staff_sizing(void *main_obj)
{
    typedef void(__thiscall *list_fn)(void *firms, BYTE **list, int firm, int job);
    typedef void(__thiscall *list_end_fn)(BYTE **list);
    static re_firm_fit own[OWN_FIRMS];
    static re_staff_read m[STAFF_MAX], c[WAGE_CANDIDATES];
    void *firms = re_game_firms(main_obj), *jobs = re_game_jobs(firms);
    int owned = re_game_firm_fits(main_obj, own, OWN_FIRMS), month = re_game_period(main_obj), looks = 0;
    g_released = 0;
    if (g_spare_months <= 0 || month < 0 || g_playthrough[0] == 0)
        return;
    for (int f = 0; f < owned; f++) {
        BYTE *first = NULL;
        int count = re_game_firm_staff(firms, own[f].id, &first), firm = own[f].id;
        char ok[STAFF_MAX], managed[STAFF_MAX];
        if (!own[f].open) {
            /* closed: nobody had work this month, which says nothing about who the work needs. The rows of month
             * ends in a row end with the month that is left out. */
            re_log("business: '%s' (firm %d) is closed: its staff is not measured at this month end, nobody is let go or replaced",
                   own[f].name, firm);
            continue;
        }
        count = count > STAFF_MAX ? STAFF_MAX : count;
        for (int i = 0; i < count; i++) {
            ok[i] = (char)re_game_staff_read(jobs, first + (SIZE_T)i * RE_STAFF_BYTES, &m[i]);
            managed[i] = (char)(ok[i] && re_game_staff_auto(firms, firm, m[i].id));
        }
        /* Work that is signed for and not measured yet: nobody leaves this business in this month end. The rows go
         * on, so the next month end, the first that has that work in its hours, decides. */
        int coming = 0, fresh = signed_lately(firms, firm, month, &coming);
        char held[160] = "";
        if (fresh > 0)
            snprintf(held, sizeof held,
                     ": %d contract(s) signed in this month bring their work from the next, %d hours a month and what those set off; "
                     "the staff is measured with it first",
                     fresh, coming);
        for (int i = 0; i < count; i++) {
            int job = m[i].job, staying = 0, one = -1, pick = -1, earlier = 0;
            double worst = 0.0;
            for (int k = 0; ok[i] && k < i; k++)
                earlier |= ok[k] && m[k].job == job;
            if (!ok[i] || earlier)
                continue; /* each job once, at its first person */
            for (int k = i; k < count; k++)
                if (ok[k] && m[k].job == job && m[k].demand <= m[k].wage) {
                    staying++;
                    one = k;
                }
            if (staying >= 2) {
                int filled = 0; /* the one picked was hired last month for a shortage: no wait */
                for (int e = i; e < count && !filled; e++) {
                    double others = 0.0, full = 0.0, cost;
                    if (!managed[e] || m[e].job != job || m[e].demand > m[e].wage || m[e].hours <= 0 || !(m[e].efficiency > 0.0))
                        continue;
                    for (int k = i; k < count; k++)
                        if (k != e && ok[k] && m[k].job == job && m[k].demand <= m[k].wage) {
                            others += m[k].left * m[k].efficiency;
                            full += m[k].hours * m[k].efficiency;
                        }
                    cost = (double)m[e].wage / (m[e].hours * m[e].efficiency);
                    if (!re_business_wage_spare((m[e].hours - m[e].left) * m[e].efficiency, others, full, staying - 1))
                        continue;
                    filled = filled_in(month, firm, m[e].id);
                    if (filled || pick < 0 || cost > worst) {
                        pick = e;
                        worst = cost;
                    }
                }
                int run = months_running("spare", month, firm, job, pick >= 0);
                if (pick < 0)
                    continue;
                int due = filled || run >= g_spare_months, go = due && fresh == 0 && g_released < WAGE_PLACES;
                re_log("business: '%s' (firm %d), job %d, %d people staying: the others could have done the work of %s (employee %d, %d of "
                       "%d hours left, an hour of work %.1f cents) in the hours they left; %s, month end %d of %d in a row like that: %s%s",
                       own[f].name, firm, job, staying, m[pick].name, m[pick].id, m[pick].left, m[pick].hours, worst,
                       filled ? "hired last month for a shortage" : "of the standing staff", run, g_spare_months,
                       go ? "is let go after this month end, without severance" : "stays", due && !go ? held : "");
                if (go) {
                    g_releases[g_released].firm = firm;
                    g_releases[g_released].id = m[pick].id;
                    memcpy(g_releases[g_released++].name, m[pick].name, sizeof m[pick].name);
                    months_running("spare", month, firm, job, 0);
                }
            } else if (staying == 1 && managed[one]) {
                const re_staff_read *s = &m[one];
                int idle = s->hours > 0 && (double)s->left >= g_idle_share * s->hours;
                int run = months_running("idle", month, firm, job, idle);
                if (!idle)
                    continue;
                int filled = filled_in(month, firm, s->id);
                int due = run >= g_spare_months || filled;
                if (!due || fresh > 0 || g_wage_placed == WAGE_PLACES) {
                    re_log("business: '%s' (firm %d), job %d: %s (employee %d), alone in the job, left %d of %d hours unworked; month end "
                           "%d of %d in a row like that: stays%s",
                           own[f].name, firm, job, s->name, s->id, s->left, s->hours, run, g_spare_months, due ? held : "");
                    continue;
                }
                /* The look at a job's candidates has the game make the month's people for that job when nobody has
                 * opened its hire tab: a tenth of a second a job where it was measured, at a month end that takes the
                 * game a second already (note m30). So a job is looked at once in spareMonths month ends, and two
                 * jobs a month end at most; the others have their turn at the next. */
                char look_key[48];
                snprintf(look_key, sizeof look_key, "idlelookf%dj%d", firm, job);
                long long looked = re_state_get(g_playthrough, look_key, -1000);
                int lately = looked <= month && month - looked < g_spare_months; /* a look of a later month is another course's */
                if (!filled && (lately || looks == SIZING_LOOKS)) {
                    re_log("business: '%s' (firm %d), job %d: %s (employee %d), alone in the job, left %d of %d hours unworked for %d "
                           "month ends; %s: stays",
                           own[f].name, firm, job, s->name, s->id, s->left, s->hours, run,
                           lately ? "its candidates were looked at less than spareMonths month ends ago"
                                  : "the candidates of two jobs were looked at in this month end already, its turn is the next");
                    continue;
                }
                looks += !filled;
                re_state_set(g_playthrough, look_key, month);
                BYTE *list[3] = {NULL, NULL, NULL};
                list_begin(firms, firm, job); /* the list the hire tab would show in this month */
                long long listed = time_now();
                ((list_fn)AT(RE_VA_HIRE_LIST))(firms, list, firm, job);
                time_add(TIME_HIRE_LIST, listed);
                list_end(list);
                SIZE_T bytes = (SIZE_T)(list[1] - list[0]);
                int n = list[1] > list[0] && bytes % RE_STAFF_BYTES == 0 && re_readable(list[0], bytes) ? (int)(bytes / RE_STAFF_BYTES) : 0;
                long long wage[WAGE_CANDIDATES];
                int hours[WAGE_CANDIDATES];
                double efficiency[WAGE_CANDIDATES];
                n = n > WAGE_CANDIDATES ? WAGE_CANDIDATES : n;
                for (int k = 0; k < n; k++) {
                    int taken = !re_game_staff_read(jobs, list[0] + (SIZE_T)k * RE_STAFF_BYTES, &c[k]);
                    for (int p = 0; !taken && p < g_wage_placed; p++) /* promised to another place in this month end */
                        taken = g_wage_places[p].firm == firm && g_wage_places[p].id == c[k].id;
                    wage[k] = taken ? 0 : c[k].wage;
                    hours[k] = taken ? 0 : c[k].hours;
                    efficiency[k] = taken ? 0.0 : c[k].efficiency;
                }
                int smaller = re_business_smaller_pick(wage, hours, efficiency, n, s->wage, s->hours, (s->hours - s->left) * s->efficiency);
                if (smaller < 0)
                    re_log("business: '%s' (firm %d), job %d: %s (employee %d), alone in the job, left %d of %d hours unworked for %d "
                           "month ends; none of the %d candidate(s) has fewer hours, a lower wage and gets the work done: stays",
                           own[f].name, firm, job, s->name, s->id, s->left, s->hours, run, n);
                else {
                    wage_place *p = &g_wage_places[g_wage_placed];
                    re_game_staff_copy(g_wage_records[g_wage_placed++], list[0] + (SIZE_T)smaller * RE_STAFF_BYTES);
                    p->firm = firm;
                    p->leaver = s->id;
                    p->id = c[smaller].id;
                    p->dismiss = 1;
                    memcpy(p->name, c[smaller].name, sizeof p->name);
                    memcpy(p->leaver_name, s->name, sizeof p->leaver_name);
                    months_running("idle", month, firm, job, 0);
                    re_log("business: '%s' (firm %d), job %d: %s (employee %d, %lld cents a month for %d hours), alone in the job, left %d "
                           "hours unworked for %d month ends; %s (candidate %d, %lld cents for %d hours at efficiency %.4f) takes the "
                           "place after this month end, and the present one is let go without severance",
                           own[f].name, firm, job, s->name, s->id, s->wage, s->hours, s->left, run, c[smaller].name, c[smaller].id,
                           c[smaller].wage, c[smaller].hours, c[smaller].efficiency);
                }
                ((list_end_fn)AT(RE_VA_STAFF_LIST_END))(list); /* the copies the list function made for this look */
            }
        }
    }
}

/* After the month end. The people let go without a successor leave the staff. The kept candidates take the places
 * of those who are gone - or who are let go here, for a place given to a person of fewer hours - the way the
 * game's own management puts in a person for one who resigned: the record is appended to the staff with the month
 * as its start and the month's hours to work, and the person is put under the automatic management. The business
 * pays for a hiring what it would cost in a store manager's time; by hand it is an interview of 20 of the
 * player's own hours. The payment is booked like the game's own recruiter's fee: staff costs of that business,
 * deductible. Letting somebody go costs nothing: the month end has paid the month. */
static void wage_places(void *main_obj)
{
    void *firms = re_game_firms(main_obj), *finance = re_game_finance(main_obj);
    for (int i = 0; firms != NULL && i < g_released; i++) {
        int there = re_game_employee(firms, g_releases[i].firm, g_releases[i].id) != NULL;
        if (there)
            re_game_staff_remove(firms, g_releases[i].firm, g_releases[i].id);
        re_log("business: firm %d: %s (employee %d) %s", g_releases[i].firm, g_releases[i].name, g_releases[i].id,
               !there                                                                   ? "had left in the month end already"
               : re_game_employee(firms, g_releases[i].firm, g_releases[i].id) == NULL ? "is let go, the job has people to spare"
                                                                                        : "COULD NOT be let go");
    }
    g_released = 0;
    for (int i = 0; i < g_wage_placed; i++) {
        const wage_place *p = &g_wage_places[i];
        BYTE *record = g_wage_records[i];
        if (firms != NULL && finance != NULL && p->dismiss && re_game_employee(firms, p->firm, p->leaver) != NULL)
            re_game_staff_remove(firms, p->firm, p->leaver);
        if (firms == NULL || finance == NULL || re_game_employee(firms, p->firm, p->leaver) != NULL)
            re_log("business: firm %d: %s (employee %d) is still there or the business cannot be reached, %s is not taken on", p->firm,
                   p->leaver_name, p->leaver, p->name);
        else {
            long long fee = job_fee(main_obj, RE_JOB_STORE_MANAGER, g_hire_hours);
            *(int *)(record + RE_STAFF_START) = re_game_period(main_obj);
            *(int *)(record + RE_STAFF_LEFT) = *(const int *)(record + RE_STAFF_HOURS);
            re_game_staff_append(firms, p->firm, record);
            re_game_staff_manage(firms, p->firm, p->id);
            if (fee > 0)
                ((money_fn)AT(RE_VA_CHANGE_MONEY))(finance, (unsigned)-fee, (int)(-fee >> 32), RE_TAG_STAFF, 1, p->firm, 0);
            re_log("business: firm %d: %s (employee %d) takes the place of %s (employee %d%s): wage %lld cents a month, the hiring cost "
                   "%lld cents (%.0f hours of a store manager); the game's staff list %s, automatic management %s",
                   p->firm, p->name, p->id, p->leaver_name, p->leaver, p->dismiss ? ", let go for it" : "",
                   *(const long long *)(record + RE_STAFF_WAGE), fee, g_hire_hours,
                   re_game_employee(firms, p->firm, p->id) != NULL ? "has the new person" : "DOES NOT HAVE the new person",
                   re_game_staff_auto(firms, p->firm, p->id) ? "on" : "OFF");
        }
        re_game_staff_end(record);
    }
    g_wage_placed = 0;
}

/* ---- staff filled in during the month (REQUEST.md [43], [45], note m22) */

/* Every 91st game hour the game runs a block for every business; the mod looks at the household's own there. A job
 * that has people under the automatic management, is not outsourced, and still has more work this month than its
 * people have hours for - by at least RE_FILL_LEAST hours and more than autoHireShort of the job's month - gets
 * people hired: of the job's candidates of the month, the one with whom an hour of the missing work costs least,
 * then the next, until the work has hands or no candidate is left. There is no limit a month (REQUEST.md [45]); the
 * month's candidates are the limit. The hiring is the game's own; the business pays for each what a hiring by the
 * mod costs (hireFeeHours). The block of the month's first hour is passed over: the month's numbers are not settled
 * there. Seen in the game (run 122): people work their hours off early in a month, so a job that is short stands
 * still from the second or third block on, with its work left and no hours.
 *
 * An hour of a person is worth less work while the business is short of assets (re_business_asset_factor), and the
 * hands are counted with that. Where a job is short and assets are part of why, the assets come first: what the
 * business is missing is bought there and then (assets_fill; only when the game's switch for it is on), the game
 * works its two numbers out again, and people are hired for what is still without hands after that.
 *
 * The person is noted as filled in, with the month. At the first month end after that month staff_sizing looks at
 * such a person without the wait of spareMonths: hired for a month's shortage, not needed in the next, let go. */
#define FILL_JOBS 32
static int g_asset_fill = 1, g_asset_fill_hooked;
static int assets_fill(void *main_obj, int buy, int only_firm); /* below */

static const char *fill_key(char key[48], int firm, int id)
{
    snprintf(key, 48, "filled%de%d", firm, id);
    return key;
}

/* 1 = the mod hired that person in the month before this one for work nobody had the hours for */
static int filled_in(int month, int firm, int id)
{
    char key[48];
    return month > 0 && re_state_get(g_playthrough, fill_key(key, firm, id), 0) == month - 1;
}

static void staff_fill(void *main_obj, void *firms, int firm)
{
    typedef void(__thiscall * list_fn)(void *firms, BYTE **list, int firm, int job);
    typedef void(__thiscall * list_end_fn)(BYTE **list);
    static re_job_hours job[FILL_JOBS];
    static re_staff_read m[STAFF_MAX], c[WAGE_CANDIDATES];
    void *jobs = re_game_jobs(firms), *finance = re_game_finance(main_obj);
    BYTE *first = NULL;
    long long ticks = re_game_ticks(main_obj);
    int count = re_game_firm_staff(firms, firm, &first), kinds = re_game_firm_jobs(firms, firm, job, FILL_JOBS);
    int month = re_game_period(main_obj), clock = ticks >= 0 ? RE_MONTH_HOURS - (int)(ticks % RE_MONTH_HOURS) : 0;
    char name[64] = "", read[STAFF_MAX], managed[STAFF_MAX], key[48];
    if (count <= 0 || kinds <= 0 || month < 0 || clock <= 0 || clock == RE_MONTH_HOURS || jobs == NULL || finance == NULL ||
        g_playthrough[0] == 0)
        return;
    count = count > STAFF_MAX ? STAFF_MAX : count;
    for (int i = 0; i < count; i++) {
        read[i] = (char)re_game_staff_read(jobs, first + (SIZE_T)i * RE_STAFF_BYTES, &m[i]);
        managed[i] = (char)(read[i] && re_game_staff_auto(firms, firm, m[i].id));
    }
    re_game_firm_name(firms, firm, name, sizeof name);
    re_coming coming[FILL_JOBS];
    int comings = re_game_firm_coming(firms, firm, month, coming, FILL_JOBS);
    /* a business can keep one job of the game's data under more than one number of its own: the hours are added up,
     * each entry's work that is done counting for no more than that entry was given */
    int distinct = 0;
    for (int j = 0; j < kinds; j++) {
        int k = 0, given = job[j].allowed + job[j].overtime;
        job[j].worked = job[j].worked > given ? given : job[j].worked;
        while (k < distinct && job[k].job != job[j].job)
            k++;
        if (k == distinct)
            job[distinct++] = job[j];
        else {
            job[k].worked += job[j].worked;
            job[k].allowed += job[j].allowed;
            job[k].overtime += job[j].overtime;
            job[k].outsourced |= job[j].outsourced;
        }
    }
    kinds = distinct;
    char seen[900] = ""; /* trace: what was looked at, job by job */
    int at = 0, looked = 0;
    for (int j = 0; j < kinds; j++) {
        int given = job[j].allowed + job[j].overtime, todo = given - job[j].worked, people = 0, under = 0;
        double hands = 0.0, cost = 0.0, assets = 1.0, furnishings = 1.0; /* hands: hours x skill, in the time there is */
        for (int i = 0; i < count; i++)
            if (read[i] && m[i].job == job[j].job) {
                people++;
                under += managed[i];
                hands += (m[i].left < clock ? m[i].left : clock) * m[i].efficiency;
            }
        re_game_firm_factors(firms, firm, job[j].job, &assets, &furnishings);
        double factor = re_business_asset_factor(assets, furnishings), able = hands * factor;
        if (g_on_trace && (size_t)at < sizeof seen - 110)
            at += snprintf(seen + at, sizeof seen - (size_t)at, " job %d: %d of %d to do, hands for %.0f (assets leave %.0f%%), %d people (%d managed)%s;",
                           job[j].job, todo, given, able, 100.0 * factor, people, under, job[j].outsourced ? ", outsourced" : "");
        if (j == kinds - 1 && g_on_trace)
            re_log("business: '%s' (firm %d), %d game hours left in the month:%s", name, firm, clock, seen);
        if (under == 0 || job[j].outsourced)
            continue;
        /* hiring ahead (REQUEST.md [49]): what the business's contracts add to this job's work from the next month
         * on, looked at once for a month and an amount */
        int short_now = re_business_short(given, todo, able, g_fill_share), ahead = 0;
        double month_able = 0.0; /* what the job's people do in a whole month */
        char ahead_key[48];
        for (int k = 0; k < comings; k++)
            ahead += coming[k].job == job[j].job ? coming[k].hours : 0;
        for (int i = 0; i < count; i++)
            if (read[i] && m[i].job == job[j].job)
                month_able += m[i].hours * m[i].efficiency;
        snprintf(ahead_key, sizeof ahead_key, "aheadf%dj%d", firm, job[j].job);
        int look_ahead = ahead > 0 && re_state_get(g_playthrough, ahead_key, 0) != (long long)month * 100000 + ahead;
        if (!short_now && !look_ahead)
            continue;
        if (short_now && factor < 1.0) {
            /* once a block for the business: its missing assets are bought, when the game's switch for that is on */
            int units = looked || !g_asset_fill_hooked ? 0 : assets_fill(main_obj, 1, firm);
            double was = factor, assets_was = assets, furnishings_was = furnishings;
            looked = 1;
            if (units > 0)
                re_game_firm_refit(firms, firm);
            re_game_firm_factors(firms, firm, job[j].job, &assets, &furnishings);
            factor = re_business_asset_factor(assets, furnishings);
            able = hands * factor;
            int still = re_business_short(given, todo, able, g_fill_share), none = assets <= RE_ASSETS_LEAST + 1e-6;
            re_log("business: '%s' (firm %d), job %d: %d hours of work still to do this month; the job's assets at %.2f and the "
                   "furnishings at %.2f left %.0f%% of an hour's work. %d unit(s) of missing assets bought in this block: assets %.2f, "
                   "furnishings %.2f, %.0f%% now; its %d people have hours for %.0f, %s",
                   name, firm, job[j].job, todo, assets_was, furnishings_was, 100.0 * was, units, assets, furnishings, 100.0 * factor, people,
                   able,
                   !still ? "that is enough, nobody is hired"
                   : none ? "but an asset of the job is missing altogether and the game does not work such a job: nobody is hired"
                          : "still short");
            short_now = still && !none;
            look_ahead = look_ahead && !none;
            if (!short_now && !look_ahead)
                continue;
        }
        BYTE *list[3] = {NULL, NULL, NULL};
        list_begin(firms, firm, job[j].job); /* the list the hire tab would show in this month */
        long long listed = time_now();
        ((list_fn)AT(RE_VA_HIRE_LIST))(firms, list, firm, job[j].job);
        time_add(TIME_HIRE_LIST, listed);
        list_end(list);
        SIZE_T bytes = (SIZE_T)(list[1] - list[0]);
        int n = list[1] > list[0] && bytes % RE_STAFF_BYTES == 0 && re_readable(list[0], bytes) ? (int)(bytes / RE_STAFF_BYTES) : 0;
        long long wage[WAGE_CANDIDATES];
        int hours[WAGE_CANDIDATES];
        double efficiency[WAGE_CANDIDATES];
        n = n > WAGE_CANDIDATES ? WAGE_CANDIDATES : n;
        for (int k = 0; k < n; k++) {
            int gone = !re_game_staff_read(jobs, list[0] + (SIZE_T)k * RE_STAFF_BYTES, &c[k]);
            wage[k] = gone ? 0 : c[k].wage;
            hours[k] = gone ? 0 : c[k].hours;
            efficiency[k] = gone ? 0.0 : c[k].efficiency * factor; /* what an hour of that person is worth in this business now */
        }
        double pool = 0.0, gained = 0.0; /* a whole month of this month's candidates, and of those hired in this look */
        for (int k = 0; k < n; k++)
            pool += hours[k] * efficiency[k];
        for (int taken = 0; short_now; taken++) { /* one candidate a turn; a taken one has no wage and is not picked again */
            double need = (double)todo - able;
            int pick = re_business_fill_pick(wage, hours, efficiency, n, need, clock, &cost);
            if (pick < 0) {
                re_log("business: '%s' (firm %d), job %d: %d hours of work still to do this month and its %d people have hours for %.0f, "
                       "%d game hours left in the month; %d hired in this block, none of the %d other candidate(s) can be hired for it",
                       name, firm, job[j].job, todo, people + taken, able, clock, taken, n - taken);
                break;
            }
            long long fee = job_fee(main_obj, RE_JOB_STORE_MANAGER, g_hire_hours);
            int hired = re_game_hire(firms, firm, job[j].job, c[pick].id);
            if (hired) {
                if (!re_game_staff_auto(firms, firm, c[pick].id))
                    re_game_staff_manage(firms, firm, c[pick].id);
                if (fee > 0)
                    ((money_fn)AT(RE_VA_CHANGE_MONEY))(finance, (unsigned)-fee, (int)(-fee >> 32), RE_TAG_STAFF, 1, firm, 0);
                re_state_set(g_playthrough, fill_key(key, firm, c[pick].id), month);
            }
            re_log("business: '%s' (firm %d), job %d: %d hours of work still to do this month and its %d people have hours for %.0f, "
                   "%d game hours left in the month; %.0f hours are without hands: %s (candidate %d, %lld cents a month for %d hours "
                   "at efficiency %.4f, of which the assets leave %.0f%%, an hour of the missing work %.1f cents) %s, hiring %d of this "
                   "look, the hiring cost %lld cents (%.0f hours of a store manager), automatic management %s",
                   name, firm, job[j].job, todo, people + taken, able, clock, need, c[pick].name, c[pick].id, c[pick].wage, c[pick].hours,
                   c[pick].efficiency, 100.0 * factor, cost, hired ? "is hired" : "COULD NOT be hired", taken + 1, hired ? fee : 0,
                   g_hire_hours, hired && re_game_staff_auto(firms, firm, c[pick].id) ? "on" : "OFF");
            if (!hired)
                break;
            able += (hours[pick] < clock ? hours[pick] : clock) * efficiency[pick];
            gained += hours[pick] * efficiency[pick];
            wage[pick] = 0;
            if (!re_business_short(given, todo, able, g_fill_share))
                break;
        }
        if (look_ahead) {
            /* Next month the job has this month's work and what the contracts add. Its people, with those hired just
             * now, do so much of it in a month, and the hirings of that month can add about what a month's list
             * holds - this month's stands for it. What is still without hands then is hired for now, from what this
             * month's list has left: a second month's candidates for work one month's cannot staff. */
            double next = (double)given + ahead, lack = next - month_able * factor - gained, need = lack - pool * g_trace_pool;
            int hired_ahead = 0;
            re_state_set(g_playthrough, ahead_key, (long long)month * 100000 + ahead);
            while (g_hire_ahead && need >= RE_FILL_LEAST && need > g_fill_share * next) {
                int pick = re_business_fill_pick(wage, hours, efficiency, n, need, RE_MONTH_HOURS, &cost);
                long long fee = job_fee(main_obj, RE_JOB_STORE_MANAGER, g_hire_hours);
                if (pick < 0 || !re_game_hire(firms, firm, job[j].job, c[pick].id))
                    break;
                if (!re_game_staff_auto(firms, firm, c[pick].id))
                    re_game_staff_manage(firms, firm, c[pick].id);
                if (fee > 0)
                    ((money_fn)AT(RE_VA_CHANGE_MONEY))(finance, (unsigned)-fee, (int)(-fee >> 32), RE_TAG_STAFF, 1, firm, 0);
                re_state_set(g_playthrough, fill_key(key, firm, c[pick].id), month);
                re_log("business: '%s' (firm %d), job %d: hired ahead for the work of next month: %s (candidate %d, %lld cents a month "
                       "for %d hours at efficiency %.4f, an hour of the missing work %.1f cents), %.0f hours were without hands; the "
                       "hiring cost %lld cents (%.0f hours of a store manager)",
                       name, firm, job[j].job, c[pick].name, c[pick].id, c[pick].wage, c[pick].hours, c[pick].efficiency, cost, need, fee,
                       g_hire_hours);
                need -= hours[pick] * efficiency[pick];
                wage[pick] = 0;
                hired_ahead++;
            }
            re_log("business: '%s' (firm %d), job %d: its contracts change the work by %+d hours a month from the next month on, to %.0f "
                   "hours; its people do %.0f of that in a month and a month's candidates %.0f (the %d of this month's list%s): %s, %d "
                   "hired ahead%s",
                   name, firm, job[j].job, ahead, next, month_able * factor + gained, pool * g_trace_pool, n,
                   g_trace_pool != 1.0 ? ", by the test setting" : "",
                   lack - pool * g_trace_pool >= RE_FILL_LEAST && lack - pool * g_trace_pool > g_fill_share * next
                       ? "a month's candidates would not do"
                       : "that is enough",
                   hired_ahead, g_hire_ahead ? "" : " (autoHireAhead=0)");
        }
        ((list_end_fn)AT(RE_VA_STAFF_LIST_END))(list); /* the copies the list function made for this look */
    }
}

/* trace: all but one of the people of one job of one business leave right after the first load, so that a test has
 * a job far short of hands (REQUEST.md [45]: more than one hiring in one look). `[trace] thinJob=<business>,<job>`. */
static int g_trace_thin[2];

static void thin_job_trace(void *main_obj)
{
    static int done;
    void *firms = re_game_firms(main_obj), *jobs = firms != NULL ? re_game_jobs(firms) : NULL;
    BYTE *first = NULL;
    int ids[STAFF_MAX], n = 0;
    if (done || g_trace_thin[0] <= 0 || jobs == NULL)
        return;
    done = 1;
    int count = re_game_firm_staff(firms, g_trace_thin[0], &first);
    for (int i = 0; i < count && i < STAFF_MAX; i++) {
        re_staff_read s;
        if (re_game_staff_read(jobs, first + (SIZE_T)i * RE_STAFF_BYTES, &s) && s.job == g_trace_thin[1])
            ids[n++] = s.id;
    }
    for (int i = 1; i < n; i++)
        re_game_staff_remove(firms, g_trace_thin[0], ids[i]);
    re_log("business (trace): firm %d, job %d: %d of its %d people let go right after the load", g_trace_thin[0], g_trace_thin[1],
           n > 0 ? n - 1 : 0, n);
}

/* In place of the first call of the game's block for a business every 91st hour (it pays what the adverts left to
 * pay): the mod's look at a business of the household - its staff, then its adverts, which the game's function
 * for them runs after in the same block - then the game's function as called. */
static void adverts_keep(void *firms, int firm);
__attribute__((force_align_arg_pointer)) static void __fastcall firm_tick_wrapper(void *firm_stack)
{
    typedef void(__fastcall * tick_fn)(void *firm_stack);
    long long since = time_now();
    void *firms = g_main_obj != NULL ? re_game_firms(g_main_obj) : NULL;
    int firm = firms != NULL ? re_game_firm_of(firms, firm_stack) : 0;
    if (firm > 0 && g_fill && re_game_firm_open(firms, firm) != 0) /* a closed business has no work to hire for */
        staff_fill(g_main_obj, firms, firm);
    if (firm > 0 && g_ads_keep_hooked)
        adverts_keep(firms, firm);
    time_add(TIME_FIRM_BLOCK, since);
    ((tick_fn)AT(RE_VA_FIRM_TICK))(firm_stack);
}

/* ---- adverts handed to the mod (REQUEST.md [43], note m22) */

/* An advert is a switch of a business: while it is on it brings awareness - which is how much work arrives - and
 * costs money. The game's function for the adverts that are on runs every 91st game hour, eight times a month; each
 * time an advert adds an eighth of its month's awareness, less the more awareness there is already
 * (max(0, 1 - 0.75 x awareness)), and 91 hours of its price are paid. A month end takes 3% of the awareness, then
 * three tenths of what is still above 100%, then what the competition takes. So up to 103% an advert's gain
 * outlasts the month end nearly whole, and above it a third more of it is gone: that is where advertsUpTo stands.
 *
 * For a business the player has handed over (a Shift-click on one of its advert icons) the mod sets the switches in
 * the game's block of every 91st hour, just before the game's function runs: all the paid adverts of the business
 * type on while the awareness is below advertsUpTo, all off once it is there. It uses the game's own switch
 * function; a business window shows what is on when it is opened (an open one does not follow, the game gives an
 * icon its look only there and at a click). An advert that costs nothing is an activity of the player's and is
 * left alone. The paid adverts all bring the same awareness for their money (the dear one twice as much for twice
 * the price), so there is no order to switch them in.
 *
 * The adverts of a business type are in the game's data. The game fetches them whenever a business window is
 * built; the mod stands in that call and keeps the lists by type. The Shift-click that hands a business over is a
 * click in that window, so the list of its type is there by then; the paid adverts of it are written into the
 * state file with the switch, and that is what every later look reads (run 126: the game's other fetch of these
 * lists, FUN_004e8240, does not run when a save is loaded). */
#define BRAND_TYPES 64
#define BRAND_ADVERTS 8
static struct {
    int type, count, advert[BRAND_ADVERTS];
} g_brand[BRAND_TYPES];
static int g_brands;
static int g_advert_icons, g_advert_icons_firm; /* the advert icons of the business window that was built last */

/* After the game's list function, for the element "advert" of a type's "brand": `record` is the head of the type's
 * record the call was handed, its fourth word the type (the reference {1, 0, 1.0, id} every object of the data is
 * named by); the vector has elements of RE_ADVERT_BYTES with the advert at RE_ADVERT_ID. */
static void brand_seen(unsigned char *const *vector, const unsigned *record)
{
    g_advert_icons = 0; /* a business window is being built: its icons come next */
    if (!re_readable(vector, 8) || !re_readable(record, 16))
        return;
    const BYTE *first = vector[0], *end = vector[1];
    SIZE_T bytes = (SIZE_T)(end - first);
    int type = (int)record[3], at = 0, known;
    if (type <= 0 || end < first || bytes % RE_ADVERT_BYTES != 0 || (bytes != 0 && !re_readable(first, bytes)))
        return;
    while (at < g_brands && g_brand[at].type != type)
        at++;
    known = at < g_brands;
    if (!known && g_brands == BRAND_TYPES)
        return;
    g_brand[at].type = type;
    g_brand[at].count = 0;
    for (const BYTE *e = first; e != end && g_brand[at].count < BRAND_ADVERTS; e += RE_ADVERT_BYTES)
        g_brand[at].advert[g_brand[at].count++] = *(const int *)(e + RE_ADVERT_ID);
    if (!known) {
        char line[160] = "";
        g_brands++;
        for (int i = 0, n = 0; i < g_brand[at].count && (size_t)n < sizeof line - 16; i++)
            n += snprintf(line + n, sizeof line - (size_t)n, " %d", g_brand[at].advert[i]);
        re_log("business: the adverts of business type %d (record %u %u %08x %u):%s", type, record[0], record[1], record[2], record[3], line);
    }
}

/* The paid adverts of a business that was handed over, as the hand-over wrote them into the state file. */
static int adverts_of(int firm, int *advert)
{
    char key[48];
    snprintf(key, sizeof key, "advertsf%dn", firm);
    long long count = re_state_get(g_playthrough, key, 0);
    count = count < 0 ? 0 : count > BRAND_ADVERTS ? BRAND_ADVERTS : count;
    for (int i = 0; i < (int)count; i++) {
        snprintf(key, sizeof key, "advertsf%da%d", firm, i);
        advert[i] = (int)re_state_get(g_playthrough, key, 0);
    }
    return (int)count;
}

/* Every 91st hour, before the game's function for the adverts: the switches of a business the player has handed
 * over. How many adverts were on is added up in the state file for the month's fee. */
static void adverts_keep(void *firms, int firm)
{
    if (!adverts_kept(firm))
        return;
    int advert[BRAND_ADVERTS], switched = 0, count = adverts_of(firm, advert);
    double awareness = re_game_firm_awareness(firms, firm);
    char name[64] = "", key[40];
    re_game_firm_name(firms, firm, name, sizeof name);
    if (count == 0 || awareness < 0.0) {
        re_log("business: '%s' (firm %d): the mod has its adverts but its list of them is empty or the business cannot be read; they "
               "are left as they are",
               name, firm);
        return;
    }
    /* a closed business takes no work, so its adverts would be paid for nothing: off until it is open again */
    int open = re_game_firm_open(firms, firm) != 0, want = open && awareness < g_ads_upto;
    for (int i = 0; i < count; i++)
        if (re_game_advert_on(firms, firm, advert[i]) != want) {
            re_game_advert_switch(firms, firm, advert[i], want);
            switched++;
        }
    if (want) {
        snprintf(key, sizeof key, "advertsrunf%d", firm);
        re_state_set(g_playthrough, key, re_state_get(g_playthrough, key, 0) + count);
    }
    if (g_on_trace || switched)
        re_log("business: '%s' (firm %d): awareness %.2f%%, adverts up to %.2f%%%s: its %d advert(s) are %s (%d switched)", name, firm,
               100.0 * awareness, 100.0 * g_ads_upto, open ? "" : ", the business is closed", count, want ? "on" : "off", switched);
}

/* Just before a month end: what looking after the adverts cost. An advert on for a whole month is eight turns and
 * costs advertsFeeHours of a PR specialist at the standard wage; fewer turns cost their share. Booked like the cost
 * of a hiring: staff costs of that business, deductible. It is charged whatever the cash is ([43]): what the cash
 * does not cover the game's money function turns into a debt. With trace=1 the month's numbers of every business
 * are written too: the work left undone, the awareness, the adverts that are on and what they cost last. */
static void adverts_month_end(void *main_obj)
{
    static re_firm_fit own[OWN_FIRMS];
    void *firms = re_game_firms(main_obj), *finance = re_game_finance(main_obj);
    int owned = re_game_firm_fits(main_obj, own, OWN_FIRMS);
    long long month = job_fee(main_obj, RE_JOB_PR_SPECIALIST, g_ads_hours);
    char key[40];
    for (int f = 0; f < owned; f++) {
        int firm = own[f].id;
        snprintf(key, sizeof key, "advertsrunf%d", firm);
        long long turns = g_ads_keep_hooked && g_playthrough[0] != 0 ? re_state_get(g_playthrough, key, 0) : 0, fee = month * turns / 8;
        if (turns > 0) {
            re_state_set(g_playthrough, key, 0);
            if (fee > 0 && finance != NULL) {
                ((money_fn)AT(RE_VA_CHANGE_MONEY))(finance, (unsigned)-fee, (int)(-fee >> 32), RE_TAG_STAFF, 1, firm, 0);
                /* it comes every month, just ahead of the month end's own payments: the forecast of the cash the
                 * next month end needs counts it with them (the cost of a hiring is left out, it comes once) */
                if (g_on_forecast)
                    flow_add(-fee, RE_TAG_STAFF);
            }
            re_log("business: '%s' (firm %d): the mod had adverts on for %lld turn(s) of 91 hours this month; looking after them costs "
                   "%lld cents (%.1f hours of a PR specialist for an advert's whole month of 8 turns): %s",
                   own[f].name, firm, turns, fee, g_ads_hours, fee > 0 && finance != NULL ? "paid" : "nothing charged");
        }
        if (g_on_trace) {
            int keys[12];
            long long amounts[12];
            char line[400] = "";
            int n = re_game_firm_advert_costs(firms, firm, keys, amounts, 12);
            for (int k = 0, at = 0; k < n && (size_t)at < sizeof line - 40; k++)
                at += snprintf(line + at, sizeof line - (size_t)at, " %d: %lld", keys[k], amounts[k]);
            re_log("business: '%s' (firm %d): %d of the month's %d hours of work left undone, awareness %.1f%%, %d advert(s) switched on, "
                   "the mod %s them; what they cost in their last turn, cents by advert (%d):%s",
                   own[f].name, firm, own[f].undone, own[f].work, 100.0 * own[f].awareness, own[f].adverts,
                   adverts_kept(firm) ? "looks after" : "does not look after", n, line);
        }
    }
}

/* The click on an advert's icon. The game asks whether the advert is on, switches it to the opposite and gives the
 * icon the look of what it switched to; an icon gets its look nowhere else but where the window is built. The mod
 * stands in both calls. With a Shift key held the business's adverts are handed to the mod, or taken back, and the
 * advert under the click stays as it is: the game is told the opposite of what the advert is, so what it "switches"
 * to is what the advert is already, and the mod's stand-in for the switch leaves that call out. So the icon keeps
 * its look and it is true. Nothing is switched at a hand-over: the icons of the other adverts would not follow.
 * The mod's first look comes with the next block of every 91st hour. A plain click goes on to the game as it is. */
static int g_advert_click_kept; /* the click being handled switches nothing: its switch call is left out */

/* The look of an advert's icon (REQUEST.md [44]). The game gives an icon one of two colours, white when the advert
 * is on and grey when it is off, in two places: where the business window is built and after a click on the icon.
 * Each asks the game whether the advert is on and then makes the colour with cocos2d's Color3B(r, g, b). The mod
 * stands in the questions, where it learns the business and the advert, and in the three calls of that constructor:
 * an advert the mod switches gets an orange in place of white or grey, so that a business that was handed over is
 * seen at a glance. On or off does not show then - the mod switches every 91st hour and an open window would not
 * follow. The window's build also notes the sprite of every icon, so that a hand-over colours all the icons of the
 * open window and not only the one under the click. */
#define ADVERT_ICONS 12
static struct {
    void *sprite, *functions;
    int advert;
} g_advert_icon[ADVERT_ICONS]; /* g_advert_icons of them, of the business g_advert_icons_firm */
static int g_advert_look_hooked;
static int g_advert_orange;      /* the next colour the game makes for an icon is the mod's */
static int g_advert_being_built; /* inside the window's build: the advert whose icon is being made */
static const BYTE g_advert_tint[3][4] = {{127, 127, 127, 0}, {255, 255, 255, 0}, {255, 168, 64, 0}}; /* off, on, the mod's */

/* 1 = that advert is one the mod switches: a paid advert of a business that was handed over. */
static int advert_is_kept(void *firms, int firm, int advert)
{
    return g_ads_keep_hooked && adverts_kept(firm) && re_game_advert_price(firms, firm, advert) != 0;
}

__attribute__((force_align_arg_pointer)) static int __fastcall advert_on_build_wrapper(void *firms, void *unused, int firm, int advert)
{
    (void)unused;
    g_advert_being_built = advert;
    g_advert_icons_firm = firm;
    g_advert_orange = g_advert_look_hooked && advert_is_kept(firms, firm, advert);
    return re_game_advert_on(firms, firm, advert);
}

__attribute__((force_align_arg_pointer)) static void *__fastcall advert_look_wrapper(void *colour, void *unused, int red, int green, int blue)
{
    typedef void *(__thiscall * colour_fn)(void *colour, int red, int green, int blue);
    (void)unused;
    if (g_advert_being_built != 0) {
        /* called by the window's build, whose frame has the icon's sprite */
        const BYTE *build = *(const BYTE *const *)__builtin_frame_address(0);
        void *const *slot = (void *const *)(const void *)(build + RE_ADVERT_BUILD_FRAME_SPRITE);
        if (re_readable(slot, 4) && re_readable(*slot, 4) && g_advert_icons < ADVERT_ICONS) {
            g_advert_icon[g_advert_icons].sprite = *slot;
            g_advert_icon[g_advert_icons].functions = *(void **)*slot;
            g_advert_icon[g_advert_icons++].advert = g_advert_being_built;
        }
        g_advert_being_built = 0;
    }
    if (g_advert_orange) {
        red = g_advert_tint[2][0];
        green = g_advert_tint[2][1];
        blue = g_advert_tint[2][2];
    }
    g_advert_orange = 0;
    return ((colour_fn) * (void **)AT(RE_VA_COLOUR3_NEW))(colour, red, green, blue);
}

/* Every icon of the open window of that business gets the look that holds now: after a hand-over or a taking back.
 * A sprite is touched only while it still is the object the window's build saw. */
static void advert_paint_all(void *firms, int firm)
{
    typedef void(__thiscall * colour_fn)(void *node, const BYTE *rgb);
    int painted = 0;
    for (int i = 0; g_advert_look_hooked && firm == g_advert_icons_firm && i < g_advert_icons; i++) {
        void *sprite = g_advert_icon[i].sprite, **functions = (void **)g_advert_icon[i].functions;
        if (!re_readable(sprite, 4) || *(void **)sprite != (void *)functions || !re_readable(functions, RE_NODE_COLOUR + 4))
            continue;
        int look = advert_is_kept(firms, firm, g_advert_icon[i].advert) ? 2 : re_game_advert_on(firms, firm, g_advert_icon[i].advert);
        ((colour_fn)functions[RE_NODE_COLOUR / 4])(sprite, g_advert_tint[look]);
        painted++;
    }
    re_log("business: firm %d: %d of the %d advert icon(s) of the open window given their look", firm, painted,
           firm == g_advert_icons_firm ? g_advert_icons : 0);
}

static void business_all(void *firms, int firm, const char *name, int now);

__attribute__((force_align_arg_pointer)) static int __fastcall advert_on_wrapper(void *firms, void *unused, int firm, int advert)
{
    (void)unused;
    char name[64] = "", key[48], text[200];
    int on = re_game_advert_on(firms, firm, advert), shift = g_main_obj != NULL && re_game_shift_down(g_main_obj);
    g_advert_click_kept = 0;
    g_advert_orange = 0;
    if (shift && g_ads_keep_hooked && g_playthrough[0] != 0 && re_game_firm_name(firms, firm, name, sizeof name)) {
        /* with a Ctrl key too: the whole business, of which the adverts are one part (business_all); with
         * contractsKeep=0 there is no such switch and the click is the Shift-click of the adverts */
        int all = g_contracts_hooked && re_game_ctrl_down(g_main_obj), now = all ? !business_kept("all", firm) : !adverts_kept(firm);
        int type = re_game_firm_type(firms, firm), at = 0, paid = 0;
        if (!all && g_locks > 0 && business_kept("all", firm)) {
            /* a business handed over whole is locked, and has no part to take back alone */
            snprintf(text, sizeof text, T(RE_MSG_LOCKED), name);
            re_game_post_message(g_main_obj, text);
            re_log("business: '%s' (firm %d) is handed over whole: a Shift-click on an advert icon switches nothing", name, firm);
            g_advert_click_kept = 1;
            g_advert_orange = g_advert_look_hooked && advert_is_kept(firms, firm, advert);
            return !on;
        }
        while (at < g_brands && g_brand[at].type != type)
            at++;
        /* the paid adverts of the type go into the state file with the switch; one that costs nothing is an
         * activity of the player's and no switch */
        for (int i = 0; now && at < g_brands && i < g_brand[at].count; i++)
            if (re_game_advert_price(firms, firm, g_brand[at].advert[i]) != 0) {
                snprintf(key, sizeof key, "advertsf%da%d", firm, paid++);
                re_state_set(g_playthrough, key, g_brand[at].advert[i]);
            }
        if (now && paid == 0)
            re_log("business: '%s' (firm %d): Shift-click on the icon of advert %d: the paid adverts of its type %d are NOT known (%d "
                   "types known), its adverts stay the player's",
                   name, firm, advert, type, g_brands);
        else {
            if (now) {
                snprintf(key, sizeof key, "advertsf%dn", firm);
                re_state_set(g_playthrough, key, paid);
            }
            snprintf(key, sizeof key, "advertsf%d", firm);
            re_state_set(g_playthrough, key, now);
            if (!all) {
                snprintf(text, sizeof text, T(now ? RE_MSG_ADVERTS_KEPT : RE_MSG_ADVERTS_BACK), name);
                re_game_post_message(g_main_obj, text);
            }
            re_log("business: '%s' (firm %d): Shift-click on the icon of advert %d, which is %s and stays so: its adverts are %s (%d "
                   "paid advert(s) of type %d)",
                   name, firm, advert, on ? "on" : "off", now ? "the mod's to switch from now on" : "the player's again", paid, type);
            advert_paint_all(firms, firm);
        }
        if (all) {
            business_all(firms, firm, name, now);
            snprintf(text, sizeof text, T(now ? RE_MSG_ALL_KEPT : RE_MSG_ALL_BACK), name);
            re_game_post_message(g_main_obj, text);
        }
        g_advert_click_kept = 1;
        g_advert_orange = g_advert_look_hooked && advert_is_kept(firms, firm, advert);
        return !on;
    }
    if (advert_is_kept(firms, firm, advert)) {
        /* a plain click on an advert the mod switches: nothing is switched, and the player is told why */
        if (re_game_firm_name(firms, firm, name, sizeof name)) {
            snprintf(text, sizeof text, T(g_locks > 0 && business_kept("all", firm) ? RE_MSG_LOCKED : RE_MSG_ADVERTS_LOCKED), name);
            re_game_post_message(g_main_obj, text);
        }
        re_log("business: '%s' (firm %d): a plain click on the icon of advert %d, which the mod switches: nothing switched", name, firm,
               advert);
        g_advert_click_kept = 1;
        g_advert_orange = g_advert_look_hooked;
        return !on;
    }
    return on;
}

__attribute__((force_align_arg_pointer)) static void __fastcall advert_click_wrapper(void *firms, void *unused, int firm, int advert, int mode)
{
    typedef void(__thiscall * switch_fn)(void *firms, int firm, int advert, int mode);
    (void)unused;
    if (g_advert_click_kept) {
        g_advert_click_kept = 0;
        return;
    }
    ((switch_fn)AT(RE_VA_ADVERT_SWITCH))(firms, firm, advert, mode);
}

/* A worn-out asset bought again (note b23 "Q5"). With a business's switch "buy worn-out assets again" on, the game
 * puts a new unit in the place of one that has worn out and moves cash for it. Its price function gives a cost as a
 * negative amount, as the cash function wants it; this one caller negates it first, so the household is PAID the
 * price of every asset the game replaces (seen in the game, run 108: +$301.75 under tag 2501 for a new unit in
 * S-Bro). The call is redirected and an amount above zero is turned round. A purchase by hand and the automatic
 * purchase of stock have the sign right and are not touched. */
static int g_asset_charge = 1, g_asset_charge_hooked;
static int g_asset_buying;     /* the mod is inside the game's replacement function for a purchase of its own */
static long long g_asset_paid; /* what that call cost, cents */
static int g_asset_paid_firm;  /* the business the last replacement was paid for; for the ticker line that follows it */
__attribute__((force_align_arg_pointer)) static void __fastcall asset_pay_wrapper(void *finance, void *unused, unsigned low, int high, int tag,
                                                                                    int taxable, int firm, int zero)
{
    (void)unused;
    long long given = (long long)(((unsigned long long)(unsigned)high << 32) | low);
    long long amount = given > 0 && (g_asset_charge || g_asset_buying) ? -given : given; /* the mod's own purchase is always paid for */
    ((money_fn)AT(RE_VA_CHANGE_MONEY))(finance, (unsigned)amount, (int)(amount >> 32), tag, taxable, firm, zero);
    g_asset_paid = g_asset_buying ? g_asset_paid - amount : -amount;
    g_asset_paid_firm = firm;
    re_log("business: firm %d: %s; the game hands over %lld cents under tag %d, booked as %lld", firm,
           g_asset_buying ? "an asset the business is short of is bought" : "a worn-out asset is bought again", given, tag, amount);
}

/* trace, [trace] replaceAsset=<business id>: once, just before the first month end, the game's own function for a
 * worn-out asset is called for the first asset type that business owns, as if one unit of it had worn out (none
 * does; nothing is removed). With the business's switch "buy worn-out assets again" on and the month's staff hours
 * for it taken, the game adds a unit and moves cash. The cash trace then shows which way the money goes and under
 * which tag; the code reads as if the price were paid TO the household (note b23 "Q5"). Before it, the two
 * switches and the asset counts of every business are written. */
static int g_trace_replace_asset;
static void asset_replace_trace(void *main_obj)
{
    static re_firm_fit own[OWN_FIRMS];
    static int done;
    void *firms = re_game_firms(main_obj);
    int owned = re_game_firm_fits(main_obj, own, OWN_FIRMS), types[32], records = 0, auto_buy = 0, taken = 0;
    if (done || g_trace_replace_asset <= 0 || firms == NULL)
        return;
    done = 1;
    for (int f = 0; f < owned; f++) {
        int n = re_game_firm_asset_types(firms, own[f].id, types, 32, &records, &auto_buy, &taken);
        re_log("assets (trace): '%s' (firm %d): %d asset record(s) of %d type(s); worn-out assets are bought again: %s, the month's staff "
               "hours for it taken: %s",
               own[f].name, own[f].id, records, n, auto_buy ? "yes" : "no", taken ? "yes" : "no");
    }
    int n = re_game_firm_asset_types(firms, g_trace_replace_asset, types, 32, &records, &auto_buy, &taken);
    if (n < 1) {
        re_log("assets (trace): firm %d has no asset or cannot be read, nothing called", g_trace_replace_asset);
        return;
    }
    re_log("assets (trace): firm %d, %d record(s): calling the game's function for a worn-out asset of type %d; the cash movement "
           "that follows is the game's own",
           g_trace_replace_asset, records, types[0]);
    re_game_asset_replace(firms, g_trace_replace_asset, types[0]);
    int before = records;
    re_game_firm_asset_types(firms, g_trace_replace_asset, types, 32, &records, &auto_buy, &taken);
    re_log("assets (trace): firm %d: back from the game's function, %d record(s) now (%d before)", g_trace_replace_asset, records, before);
}

/* ---- a business handed over: its contracts, its floor space, all of it (REQUEST.md [48], note m33)
 *
 * Contracts. A business is offered up to three contracts at a month start; signing one is 10 hours of the player's.
 * For a business whose contracts the player has handed over (a Shift-click on the icon "Contracts" of its window)
 * the mod signs, right after the game's month start, every offer of the month the business has a place for and can
 * carry, with the game's own function (REQUEST.md [49]: as many as there are places; a contract's work starts with
 * the next month, and the staff automation hires for it). A contract fails when the share of its work left undone
 * adds up past a tenth of its months (FUN_004f86b0), and giving one up costs reputation: the mod signs only when
 * all of this holds, and never gives one up.
 *   - The business is open and has a place for another contract (the game's own count).
 *   - Its staff and its assets are looked after: the game's two switches are on and the mod's rules for them are
 *     in place. More work needs people and desks, and that is what brings them.
 *   - The month that has just ended left no more of its work undone than the staff automation lets pass without a
 *     hiring (autoHireShort): more than that, and the hirings did not keep up.
 *   - The offer pays more than its work costs at standard wages and prices.
 *   - The floor space holds it, with the offers signed before it in this month: what is in use now, made larger in
 *     the measure of the work, fits the premises - or the premises are rented, can be made larger once, and are the
 *     mod's to make larger (below). A business that had no work has nothing to measure by and takes its smallest
 *     offer alone.
 * The offers that pass are signed from the one that pays most over its cost down, while a place is left. A signing
 * costs contractFeeHours of a store manager, booked like the cost of a hiring. The staff the new work needs is
 * staff_fill's: it hires in the month the work arrives, and ahead of it when a month's candidates will not do.
 *
 * Floor space. For a business handed over whole, the mod switches the game's star on when the floor space is short:
 * a missing asset does not fit, or a contract needs the room. The game then doubles the floor space at the month
 * end and the rent with it. Owned premises are left to the player: making them larger costs their price once more,
 * at once. Premises that are the largest of their kind cannot grow: the player is told that the business has to
 * move.
 *
 * All of it. A Ctrl+Shift-click on an advert icon hands over staff, assets, adverts, contracts and floor space at
 * once - the game's two switches as a click sets them, the mod's three in its state file - and takes them back. */
/* trace: the most floor space the mod takes a business's premises to have, so that a test has a business without
 * room (`[trace] roomMost=<square metres>`), until the mod has made the premises of that business larger */
static double g_trace_room = -1.0;
static int g_trace_room_grown[8];

static double room_most(int firm, double most)
{
    for (int i = 0; i < 8; i++)
        if (g_trace_room_grown[i] == firm)
            return most;
    return g_trace_room >= 0.0 && most > g_trace_room ? g_trace_room : most;
}
#define CONTRACT_OFFERS 8
#define MONTH_WORK_MONTH 1000000000000LL /* the state file's number: month, hours of work and hours undone, six digits each */
#define MONTH_WORK_HOURS 1000000LL

/* The month that is ending, by business: the hours of work it was given and those still undone. Noted just before
 * the game's month end, for the month start that follows it - in the state file, so that the first month start
 * after a load has it too. The month in the number says whose month it is. */
static void month_work_note(void *main_obj)
{
    static re_firm_fit own[OWN_FIRMS];
    int owned = re_game_firm_fits(main_obj, own, OWN_FIRMS), period = re_game_period(main_obj);
    char key[40];
    for (int f = 0; f < owned && period >= 0 && g_playthrough[0] != 0; f++) {
        if (!business_kept("contracts", own[f].id) || own[f].work < 0 || own[f].work >= MONTH_WORK_HOURS)
            continue;
        snprintf(key, sizeof key, "monthworkf%d", own[f].id);
        re_state_set(g_playthrough, key, period * MONTH_WORK_MONTH + own[f].work * MONTH_WORK_HOURS + own[f].undone);
    }
}

static int month_work(int firm, int period, int *work, int *undone)
{
    char key[40];
    snprintf(key, sizeof key, "monthworkf%d", firm);
    long long kept = re_state_get(g_playthrough, key, -1);
    if (kept < 0 || kept / MONTH_WORK_MONTH != period)
        return 0;
    *work = (int)(kept / MONTH_WORK_HOURS % MONTH_WORK_HOURS);
    *undone = (int)(kept % MONTH_WORK_HOURS);
    return 1;
}

/* The floor space of a business is short. Returns 1 when its premises grow at this month's end: the star is on,
 * switched on here or before. Otherwise the player is told why not, once a month. */
static int premises_grow(void *main_obj, void *firms, int firm, const char *name, const char *why)
{
    re_premises p;
    char key[40], prefix[96], text[400], amount[72];
    if (!g_premises_grow || !business_kept("premises", firm))
        return 0;
    if (!re_game_firm_premises(firms, firm, &p)) {
        re_log("premises: '%s' (firm %d): its premises could NOT be read; the star is left as it is", name, firm);
        return 0;
    }
    if (p.growing != 0)
        return 1;
    int month = re_game_period(main_obj), grows = !p.owned && p.next != 0;
    snprintf(key, sizeof key, "premisestoldf%d", firm);
    if (!grows && re_state_get(g_playthrough, key, -1) == month)
        return 0;
    re_state_set(g_playthrough, key, month);
    if (grows) {
        re_game_firm_premises_grow(firms, firm, p.next);
        for (int i = 0; i < 8; i++)
            if (g_trace_room_grown[i] == 0 || g_trace_room_grown[i] == firm) {
                g_trace_room_grown[i] = firm;
                break;
            }
    }
    snprintf(prefix, sizeof prefix, T(RE_MSG_PREMISES_PREFIX), name);
    if (grows)
        snprintf(text, sizeof text, T(RE_MSG_PREMISES_GROW), prefix, money(main_obj, p.cost, amount));
    else if (p.next == 0)
        snprintf(text, sizeof text, T(RE_MSG_PREMISES_MOST), prefix);
    else
        snprintf(text, sizeof text, T(RE_MSG_PREMISES_OWNED), prefix, money(main_obj, p.cost, amount));
    panel_line(main_obj, prefix, text, "UIIconRestateUpgrade", 1);
    re_game_post_urgent(main_obj, 1, text);
    re_log("premises: '%s' (firm %d), %s: '%s' (property %d of type %d, %s, floor space %.2f): %s; the month end would charge %lld cents",
           name, firm, why, p.address, p.house, p.type, p.owned ? "owned" : "rented", p.space,
           grows         ? "the star is switched on, it grows at this month's end"
           : p.next == 0 ? "it is the largest of its kind, the player is told that the business has to move"
                         : "owned premises are left to the player, who is told what it costs",
           p.cost);
    if (grows)
        re_log("premises: '%s' (firm %d): grows into type %d", name, firm, p.next);
    return grows;
}

static void contracts_sign(void *main_obj)
{
    static re_firm_fit own[OWN_FIRMS];
    void *firms = re_game_firms(main_obj), *finance = re_game_finance(main_obj);
    int owned = firms != NULL ? re_game_firm_fits(main_obj, own, OWN_FIRMS) : 0, month = re_game_period(main_obj), signed_now = 0;
    char line[640], amount[72];
    long long fees = 0;
    int len = snprintf(line, sizeof line, "%s", T(RE_MSG_CONTRACT_PREFIX));
    if (!g_contracts_hooked || finance == NULL || month < 0 || g_playthrough[0] == 0)
        return;
    for (int f = 0; f < owned; f++) {
        int firm = own[f].id;
        if (!business_kept("contracts", firm))
            continue;
        re_offered offer[CONTRACT_OFFERS];
        re_signed had[SIGNED_MAX];
        const BYTE *shown = NULL;
        int offers = re_game_firm_offers(firms, firm, offer, CONTRACT_OFFERS, &shown), places = re_game_firm_places(firms, firm);
        int held = re_game_firm_signed(firms, firm, had, SIGNED_MAX), work = -1, undone = -1, employees = 0;
        int managed = re_game_firm_managed(firms, firm, &employees), types[64], records = 0, auto_buy = 0, taken = 0;
        int known = month_work(firm, month - 1, &work, &undone);
        re_game_firm_asset_types(firms, firm, types, 64, &records, &auto_buy, &taken);
        const char *no = offers < 0 || held < 0               ? "its contracts cannot be read"
                         : !own[f].open                       ? "it is closed"
                         : offers == 0                        ? "it has no offer this month"
                         : places <= 0                        ? "it holds as many contracts as a business of its kind can"
                         : !(g_fill_hooked && managed > 0)    ? "its staff is not under the game's automatic management, so nobody would be "
                                                                "hired for the work"
                         : !(g_asset_fill_hooked && auto_buy) ? "its switch \"buy worn-out assets again\" is off, so nothing would be bought for "
                                                                "the people the work needs"
                         : !known                             ? "what the month before left undone is not known: the mod has to see a month "
                                                                "end of this business first"
                         : re_business_short(work, undone, 0.0, g_fill_share)
                             ? "it left more of last month's work undone than the hirings let pass: the staff did not keep up"
                             : NULL;
        if (no != NULL) {
            re_log("contracts: '%s' (firm %d), month %d: %d offer(s), %d place(s), %d signed, last month %d of %d hours of work left "
                   "undone, %d of %d employee(s) managed, the asset switch %s: nothing is signed: %s",
                   own[f].name, firm, month, offers, places, held, undone, work, managed, employees, auto_buy ? "on" : "off", no);
            continue;
        }
        double used = 0.0, most = 0.0, hours_of[CONTRACT_OFFERS], premium_of[CONTRACT_OFFERS], added = 0.0;
        re_offer_read read[CONTRACT_OFFERS];
        re_premises place;
        /* out: 0 = still to decide, 1 = signed, 2 = not to be signed this month */
        int spaced = re_game_firm_space(firms, firm, &used, &most), read_ok[CONTRACT_OFFERS], out[CONTRACT_OFFERS], least = -1, floor = 0;
        int larger = g_premises_grow && business_kept("premises", firm) && re_game_firm_premises(firms, firm, &place) && !place.owned &&
                     (place.next != 0 || place.growing != 0);
        int signed_here = 0;
        most = room_most(firm, most);
        for (int i = 0; i < offers; i++) {
            re_offer_value value;
            read_ok[i] = re_game_offer(main_obj, shown, offer[i].contract, &read[i]) && re_business_offer_value(&read[i].terms, &value);
            hours_of[i] = read_ok[i] ? read[i].terms.hours + read[i].terms.induced : 0.0;
            premium_of[i] = read_ok[i] ? value.premium : 0.0;
            out[i] = 0;
            if (read_ok[i] && (least < 0 || hours_of[i] < hours_of[least]))
                least = i;
            if (g_on_trace && read_ok[i] && read[i].today > 0) {
                /* the payout an offer was made with has the reputation of that moment in it (note b18): what an
                 * influence operation has taken off the reputation is in the number the mod measures */
                double reputation = 0.0;
                int influence = 0;
                re_game_firm_reputation(firms, firm, &reputation, &influence);
                re_log("contracts (trace): '%s' (firm %d), month %d: offer %d pays %lld cents in all; the game's payout function gives "
                       "%lld today before the reputation counts: a factor of %.4f; the business's reputation is %.4f, its influence "
                       "operation is %s",
                       own[f].name, firm, month, offer[i].id, read[i].terms.total, read[i].today,
                       (double)read[i].terms.total / (double)read[i].today, reputation, influence ? "on" : "off");
            }
        }
        /* from the offer that pays most over its cost down, the longer one first where two pay alike (a contract can
         * leave a tenth of its months' work undone before it fails); each is looked at once */
        for (;;) {
            int best = -1, grows = 0;
            for (int i = 0; i < offers; i++)
                if (out[i] == 0 && (best < 0 || premium_of[i] > premium_of[best] + 1e-6 ||
                                    (premium_of[i] > premium_of[best] - 1e-6 && read[i].terms.months > read[best].terms.months)))
                    best = i;
            if (best < 0)
                break;
            double need = work > 0 ? used * (work + added + hours_of[best]) / work : used;
            const char *why = !read_ok[best]               ? "its numbers cannot be read"
                              : premium_of[best] <= 0.0    ? "it pays no more than its work costs"
                              : work <= 0 && best != least ? "the business had no work last month to measure by, only its smallest offer is "
                                                             "looked at"
                              : !spaced                    ? "the floor space cannot be read"
                              : places <= 0                ? "no place is left for it"
                                                           : NULL;
            if (why == NULL && need > most) {
                grows = larger && need <= 2.0 * most;
                if (!grows || !premises_grow(main_obj, firms, firm, own[f].name, "a contract needs the room")) {
                    why = "the floor space does not hold it";
                    grows = 0;
                    floor++;
                }
            }
            re_log("contracts: '%s' (firm %d): offer %d of type %d: %.0f hours a month and %.0f they set off, %d months, %lld cents in all, "
                   "%+.1f%% over its standard cost; floor space %.2f in use of %.2f, %.2f with it: %s",
                   own[f].name, firm, offer[best].id, read[best].type, read[best].terms.hours, read[best].terms.induced,
                   read[best].terms.months, read[best].terms.total, 100.0 * premium_of[best], used, most, need,
                   why != NULL ? why : grows ? "passes, with larger premises" : "passes");
            out[best] = 2;
            if (why != NULL)
                continue;
            int hours = (int)read[best].terms.hours, months = read[best].terms.months, id = offer[best].id, type = read[best].type;
            long long total = read[best].terms.total, fee = job_fee(main_obj, RE_JOB_STORE_MANAGER, g_contract_hours);
            int done = re_game_contract_sign(firms, firm, &offer[best]);
            if (done && fee > 0)
                ((money_fn)AT(RE_VA_CHANGE_MONEY))(finance, (unsigned)-fee, (int)(-fee >> 32), RE_TAG_STAFF, 1, firm, 0);
            re_log("contracts: '%s' (firm %d), month %d: offer %d of type %d, %d hours a month for %d months, %lld cents in all, %+.1f%% "
                   "over its standard cost: %s; the signing costs %lld cents (%.0f hours of a store manager)",
                   own[f].name, firm, month, id, type, hours, months, total, 100.0 * premium_of[best],
                   done ? "is signed" : "COULD NOT be signed", done ? fee : 0, g_contract_hours);
            if (!done)
                break;
            out[best] = 1;
            added += hours_of[best];
            places = re_game_firm_places(firms, firm);
            signed_here++;
            if ((size_t)len < sizeof line - 120)
                len += snprintf(line + len, sizeof line - (size_t)len, T(RE_MSG_CONTRACT_ITEM), signed_now ? ", " : "", own[f].name, hours,
                                months, money(main_obj, total, amount));
            signed_now++;
            fees += fee;
        }
        if (signed_here == 0)
            re_log("contracts: '%s' (firm %d), month %d: none of the %d offer(s) is signed (%d for the floor space)", own[f].name, firm, month,
                   offers, floor);
        else
            re_log("contracts: '%s' (firm %d), month %d: %d of the %d offer(s) signed (%d not for the floor space), %d place(s) left",
                   own[f].name, firm, month, signed_here, offers, floor, places);
        if (floor > 0) {
            if (signed_here == 0) {
                char text[200];
                snprintf(text, sizeof text, T(RE_MSG_CONTRACT_ROOM), own[f].name);
                re_game_post_message(main_obj, text);
            }
            premises_grow(main_obj, firms, firm, own[f].name, "an offered contract does not fit"); /* says why they stay as they are */
        }
    }
    if (signed_now == 0)
        return;
    if (fees > 0 && (size_t)len < sizeof line - 40)
        snprintf(line + len, sizeof line - (size_t)len, T(RE_MSG_CONTRACT_FEE), money(main_obj, fees, amount));
    panel_line(main_obj, T(RE_MSG_CONTRACT_PREFIX), line, "UIIconFirmContract", 1);
    snprintf(amount, sizeof amount, T(RE_MSG_CONTRACT_TICK), signed_now);
    re_game_post_urgent(main_obj, 1, amount);
}

/* The Shift-click on the icon "Contracts" of a business window: the business's contracts are handed to the mod, or
 * taken back. The window of the offers opens as for any click on the icon. */
__attribute__((force_align_arg_pointer)) static void __fastcall offers_window_wrapper(void *window, void *unused, int first, int second, int firm)
{
    typedef void(__thiscall * window_fn)(void *window, int first, int second, int firm);
    void *firms = g_main_obj != NULL ? re_game_firms(g_main_obj) : NULL;
    char name[64] = "", text[200];
    (void)unused;
    if (g_on_trace)
        re_log("contracts (trace): the window of the offers is asked for: %d, %d, firm %d; a Shift key is %s", first, second, firm,
               g_main_obj != NULL && re_game_shift_down(g_main_obj) ? "held" : "not held");
    if (firms != NULL && g_playthrough[0] != 0 && re_game_shift_down(g_main_obj) && re_game_firm_name(firms, firm, name, sizeof name)) {
        int now = !business_kept("contracts", firm);
        business_keep("contracts", firm, now);
        snprintf(text, sizeof text, T(now ? RE_MSG_CONTRACTS_KEPT : RE_MSG_CONTRACTS_BACK), name);
        re_game_post_message(g_main_obj, text);
        re_log("contracts: '%s' (firm %d): Shift-click on the icon of its contracts: signing them is %s", name, firm,
               now ? "the mod's from now on" : "the player's again");
    }
    ((window_fn)AT(RE_VA_OFFERS_WINDOW))(window, first, second, firm);
}

/* The Ctrl+Shift-click on an advert icon: everything of the business at once. The adverts' part is the caller's. */
static void business_all(void *firms, int firm, const char *name, int now)
{
    int staff = now ? re_game_firm_manage(firms, firm) : re_game_firm_unmanage(firms, firm);
    int assets = now ? re_game_firm_auto_assets_on(firms, firm) : re_game_firm_auto_assets_off(firms, firm);
    business_keep("contracts", firm, now);
    business_keep("premises", firm, now);
    business_keep("all", firm, now);
    re_log("business: '%s' (firm %d): Ctrl+Shift-click on an advert icon: all of it is %s: the automatic management of %d employee(s) "
           "switched %s, \"buy worn-out assets again\" was %s and is %s, its contracts and its floor space are %s",
           name, firm, now ? "the mod's from now on" : "the player's again", staff, now ? "on" : "off",
           assets > 0 ? "on" : assets == 0 ? "off" : "unreadable", now ? "on" : "off", now ? "the mod's" : "the player's");
}

/* ---- a business closed and opened again (REQUEST.md [49], note m34)
 *
 * The button "Open" of a business window closes the business to its work: the hours of its jobs go to 0 and no
 * month's work arrives until it is opened again. Moving a business to other premises takes a closed one. A closed
 * business has nothing the mod's rules could measure - nobody works, so everybody looks spare - and nothing the mod
 * could usefully pay for. So while a business is closed the mod leaves it alone: no hiring, nobody let go or
 * replaced, a wage demand is met as the game's own management meets it, no asset is bought, no contract is signed,
 * the star is not touched, and the paid adverts the mod has are off. Every place that acts on a business asks the
 * game's state for it (re_firm_fit.open, re_game_firm_open); what is handed over stays handed over. Here, in place of
 * the game's one call that sets the state, the player is told when a business with something automated changes. */
__attribute__((force_align_arg_pointer)) static void __fastcall firm_state_wrapper(void *business, void *unused, unsigned state)
{
    typedef void(__thiscall * state_fn)(void *business, unsigned state);
    void *firms = g_main_obj != NULL ? re_game_firms(g_main_obj) : NULL;
    int firm = firms != NULL ? re_game_firm_of(firms, business) : 0, was = firm > 0 ? re_game_firm_open(firms, firm) : -1;
    (void)unused;
    ((state_fn)AT(RE_VA_FIRM_STATE))(business, state);
    int now = firm > 0 ? re_game_firm_open(firms, firm) : -1;
    if (was < 0 || now < 0 || was == now || g_playthrough[0] == 0)
        return;
    char name[64] = "", text[280];
    int employees = 0, managed = re_game_firm_managed(firms, firm, &employees), types[64], records = 0, auto_buy = 0, taken = 0;
    re_game_firm_asset_types(firms, firm, types, 64, &records, &auto_buy, &taken);
    int adverts = g_ads_keep_hooked && adverts_kept(firm), contracts = g_contracts_hooked && business_kept("contracts", firm);
    re_game_firm_name(firms, firm, name, sizeof name);
    re_log("business: '%s' (firm %d) is %s: %d of its %d employee(s) are under the automatic management, \"buy worn-out assets again\" "
           "is %s, its adverts are %s, its contracts %s; the mod's rules for it %s",
           name, firm, now ? "open again" : "closed", managed, employees, auto_buy > 0 ? "on" : "off", adverts ? "the mod's" : "the player's",
           contracts ? "the mod's" : "the player's", now ? "go on" : "rest until it is open again");
    if (managed <= 0 && auto_buy <= 0 && !adverts && !contracts)
        return;
    snprintf(text, sizeof text, T(now ? RE_MSG_FIRM_OPENED : RE_MSG_FIRM_CLOSED), name);
    re_game_post_message(g_main_obj, text);
    if (!now && adverts)
        adverts_keep(firms, firm); /* off now, not at the next 91st hour */
}

/* trace: which handler heard that the mouse is over its widget (`[trace] hits=1`). Every handler of a click or of a
 * hover asks the game's one function for that from a place of its own, so the place a yes goes back to names the
 * control. A place is logged when it comes anew or after 1.5 seconds without it: hold the mouse still over a
 * control, then click - what the click adds are the click's handlers. For finding the places the lock below needs. */
static int g_trace_hits;
static void *g_hit_trampoline;

__attribute__((force_align_arg_pointer)) static unsigned __fastcall hit_trace_wrapper(void *input, void *unused, void *node, unsigned flag, float x,
                                                                                    float y)
{
    typedef unsigned(__thiscall * hit_fn)(void *input, void *node, unsigned flag, float x, float y);
    static struct {
        UINT_PTR site;
        DWORD at;
    } seen[96];
    (void)unused;
    unsigned hit = ((hit_fn)g_hit_trampoline)(input, node, flag, x, y);
    if ((hit & 0xff) != 0) {
        UINT_PTR site = (UINT_PTR)((const BYTE *)__builtin_return_address(0) - AT(0x00400000u)) + 0x00400000u - 5;
        DWORD now = GetTickCount();
        int slot = -1, oldest = 0;
        for (int i = 0; i < 96; i++) {
            if (seen[i].site == site)
                slot = i;
            if (seen[i].at < seen[oldest].at)
                oldest = i;
        }
        if (slot < 0 || now - seen[slot].at > 1500)
            re_log("ui (trace): the mouse at (%.0f, %.0f) is over a widget, asked from 0x%08x (flag %u), tick %lu; the business window "
                   "shows firm %d",
                   (double)x, (double)y, (unsigned)site, flag & 0xff, (unsigned long)now,
                   g_main_obj != NULL ? re_game_window_firm(g_main_obj) : 0);
        slot = slot < 0 ? oldest : slot;
        seen[slot].site = site;
        seen[slot].at = now;
    }
    return hit;
}

/* ---- a business handed over whole is locked (REQUEST.md [49], note m34)
 *
 * The player: "차라리 자동화 기능을 켠 경우 ... 기능 개입하려고 하면 마우스 클릭 자체를 안되도록 막는게 나을듯?" A
 * business handed over whole (Ctrl+Shift-click on an advert icon) is the mod's to run, and what the player changes in
 * it by hand the mod's rules would put back at their next look (a person let go is hired again, an asset sold is
 * bought again) or count wrongly. So the clicks that change such a business are not passed on, and a line across
 * the top says how to take the business back: one click, after which everything is the player's.
 *
 * Every click handler of the game asks one function whether the mouse is over its widget and does nothing on a
 * "no". In place of that call in the handlers named in re_sites.h, the answer is "no" while the business window
 * shows a business handed over whole. Not locked: what only shows something (hover texts, the ledger, the "?"), the
 * button "Open" and the premises (a move takes a closed business), the inventory, the influence operation, and the
 * advert icons, whose Ctrl+Shift-click takes the business back. The priority a household member has for working a
 * job is not locked either: that button is the same widget everywhere in the game, and the priorities are set in
 * the person's own windows too. `[business] lockHandedOver=0` leaves every click alone. */

/* 1 = the business window shows a business handed over whole: the player is told, the click is not to be heard */
static int lock_refuse(const void *from)
{
    void *firms = g_main_obj != NULL ? re_game_firms(g_main_obj) : NULL;
    int firm = firms != NULL ? re_game_window_firm(g_main_obj) : 0;
    char name[64] = "", text[280];
    if (firm <= 0 || g_playthrough[0] == 0 || !business_kept("all", firm) || !re_game_firm_name(firms, firm, name, sizeof name))
        return 0;
    static DWORD told; /* the ticker shows its lines one after the other: one line for a burst of clicks */
    DWORD tick = GetTickCount();
    if (told == 0 || tick - told > 4000) {
        snprintf(text, sizeof text, T(RE_MSG_LOCKED), name);
        re_game_post_message(g_main_obj, text);
        told = tick != 0 ? tick : 1;
    }
    re_log("business: '%s' (firm %d) is handed over whole: a click on one of its controls is not passed on (the handler asks from "
           "0x%08x)",
           name, firm, (unsigned)((const BYTE *)from - AT(0x00400000u)) + 0x00400000u - 5);
    return 1;
}

__attribute__((force_align_arg_pointer)) static unsigned __fastcall lock_hit_wrapper(void *input, void *unused, void *node, unsigned flag, float x,
                                                                                   float y)
{
    typedef unsigned(__thiscall * hit_fn)(void *input, void *node, unsigned flag, float x, float y);
    unsigned hit = ((hit_fn)AT(RE_VA_UI_HIT))(input, node, flag, x, y);
    (void)unused;
    return (hit & 0xff) != 0 && lock_refuse(__builtin_return_address(0)) ? hit & ~0xffu : hit;
}

__attribute__((force_align_arg_pointer)) static unsigned __fastcall lock_event_wrapper(void *input, void *unused, void *event, void *node, void *third)
{
    typedef unsigned(__thiscall * hit_fn)(void *input, void *event, void *node, void *third);
    unsigned hit = ((hit_fn)AT(RE_VA_UI_HIT_EVENT))(input, event, node, third);
    (void)unused;
    return (hit & 0xff) != 0 && lock_refuse(__builtin_return_address(0)) ? hit & ~0xffu : hit;
}

static void lock_install(void)
{
    static const unsigned site[] = {RE_VA_LOCK_HIRE_WINDOW, RE_VA_LOCK_STAFF_TAB, RE_VA_LOCK_ASSET_SHOP, RE_VA_LOCK_OFFERS,
                                    RE_VA_LOCK_OUTSOURCE,   RE_VA_LOCK_STAR,      RE_VA_LOCK_ASSET_BOX,  RE_VA_LOCK_GIVE_UP};
    int sites = (int)(sizeof site / sizeof site[0]);
    for (int i = 0; i < sites; i++)
        g_locks += re_patch_call(AT(site[i]), AT(RE_VA_UI_HIT), lock_hit_wrapper);
    g_locks += re_patch_call(AT(RE_VA_LOCK_SELL), AT(RE_VA_UI_HIT_EVENT), lock_event_wrapper);
    re_log("business: a business handed over whole is locked - the clicks that hire, let go, buy or sell an asset, switch a job's "
           "outsourcing, the star or \"buy worn-out assets again\", or sign or give up a contract are not passed on: %d of %d click "
           "handler(s) %s",
           g_locks, sites + 1, g_locks == sites + 1 ? "redirected" : "redirected, the others NOT");
}

/* ---- an asset a business is short of, bought (REQUEST.md [43], note b23) */

/* A business loses work efficiency when it has less of an asset than its jobs, staff and expected hours want (a
 * desk for every clerk, a parking space for every van). The game buys again only what has worn out. For a business
 * with the game's switch "buy worn-out assets again" on - the switch that takes 20 staff hours a month - the mod
 * also buys what is missing, right after the game's month start, when the new month's work is known - and during
 * the month for one business, when a job of it is short of hands while its assets are short (staff_fill):
 *   - what is wanted and what is there, by asset tag: the game's own two numbers (the red line of the business
 *     window);
 *   - the ware for a tag: one the business has already, else the cheapest for one of the tag (re_business_asset_ware);
 *   - a unit is bought only when it fits into the floor space of the premises (past that the game rolls for a
 *     burglary every month end); what cannot be bought is named in the summary and in a ticker line;
 *   - the cash is no limit (REQUEST.md [44]): the game's cash function books what the cash does not cover as its
 *     debt of three months without interest, and the summary and the ticker say how much that was. With
 *     missingAssetsOnDebt=0 a unit is bought only out of the cash that the month end is not expected to take;
 *   - when not everything can be bought, the order is re_business_asset_next's;
 *   - the purchase itself is the game's own function for a worn-out asset: the unit, the price of the shop, the
 *     booking. Its two notices ("has worn out", and one without a text) are left out for the mod's purchases. */
static int g_asset_debt = 1; /* a unit the cash does not cover is bought all the same */
static int g_trace_alone;    /* trace: the household's other businesses are not looked at for the kind of a ware, so that a
                                test reaches the choice by the cost of an hour (`[trace] assetsAlone=1`) */
#define ASSET_NEEDS 24
#define ASSET_WARES 40
#define ASSET_UNITS 12 /* the most a business is bought at one time */

typedef struct {
    char text[16];
    unsigned size, capacity;
} game_string;

/* In place of the two ticker lines of the game's replacement function. A line of the mod's own purchase is left
 * out - its three strings are the callee's to destroy. A line of the game's own replacement goes on as it came,
 * but for one thing: the second line ("a new one was bought") has no text in any language of the game and comes
 * empty (seen in the game, run 135), so the mod gives it one, with what was paid. */
__attribute__((force_align_arg_pointer)) static void __fastcall asset_notice_wrapper(void *notices, void *unused, int kind, game_string text,
                                                                                       game_string sound, game_string third)
{
    typedef void(__thiscall * notice_fn)(void *notices, int kind, game_string text, game_string sound, game_string third);
    typedef void(__thiscall * free_fn)(void *string);
    typedef void(__thiscall * assign_fn)(void *string, const char *text, unsigned length);
    (void)unused;
    if (g_on_trace) {
        const char *shown = text.capacity >= 16 ? *(const char *const *)(const void *)text.text : text.text;
        unsigned size = text.size < 200 ? text.size : 200;
        re_log("assets (trace): the ticker line of type %d of the game's replacement is %s: '%.*s'", kind,
               g_asset_buying ? "left out" : "passed on", re_readable(shown, size) ? (int)size : 0, shown);
    }
    if (!g_asset_buying) {
        char name[64] = "", line[200], amount[64];
        void *firms = g_main_obj != NULL ? re_game_firms(g_main_obj) : NULL;
        if (kind == 2 && text.size == 0 && g_asset_paid_firm > 0 && firms != NULL &&
            re_game_firm_name(firms, g_asset_paid_firm, name, sizeof name)) {
            int length = snprintf(line, sizeof line, T(RE_MSG_ASSET_REPLACED), name, money(g_main_obj, g_asset_paid, amount));
            ((assign_fn)AT(RE_VA_STR_ASSIGN))(&text, line, (unsigned)length);
            re_log("business: the game's empty ticker line after a replacement gets the mod's text: '%s'", line);
        }
        g_asset_paid_firm = 0;
        ((notice_fn)AT(RE_VA_POST_MESSAGE))(notices, kind, text, sound, third); /* the copies handed on are that function's to destroy */
        return;
    }
    ((free_fn)AT(RE_VA_STR_FREE))(&text);
    ((free_fn)AT(RE_VA_STR_FREE))(&sound);
    ((free_fn)AT(RE_VA_STR_FREE))(&third);
}

typedef struct {
    char firm[48], ware[48];
    int units, why;
} asset_note;

/* What the mod bought in the running month, for the one line of the summary panel: the month start's purchases
 * and those made during the month. A load forgets it (the line itself is saved with the game). */
#define ASSET_MONTH_NOTES 16
static asset_note g_asset_month[ASSET_MONTH_NOTES];
static int g_asset_months;
static long long g_asset_month_spent, g_asset_month_debt;

static void asset_month_note(const asset_note *note)
{
    int at = 0;
    while (at < g_asset_months && (strcmp(g_asset_month[at].firm, note->firm) != 0 || strcmp(g_asset_month[at].ware, note->ware) != 0))
        at++;
    if (at < g_asset_months)
        g_asset_month[at].units += note->units;
    else if (g_asset_months < ASSET_MONTH_NOTES)
        g_asset_month[g_asset_months++] = *note;
}

/* `buy` = 0 only reads and writes what it finds into the log (trace, after a load). `only_firm`: 0 = every business
 * of the household, at a month start; a business = that one alone, during the month. Returns the units bought. */
static int assets_fill(void *main_obj, int buy, int only_firm)
{
    static re_firm_fit own[OWN_FIRMS];
    static re_household h; /* too large for the game's stack */
    static re_ware ware[ASSET_WARES];
    static asset_note blocked[16];
    void *firms = re_game_firms(main_obj);
    int owned = re_game_firm_fits(main_obj, own, OWN_FIRMS), blockeds = 0, all_units = 0;
    long long spent = 0, debt = 0;
    if (!only_firm) {
        g_asset_months = 0;
        g_asset_month_spent = g_asset_month_debt = 0;
    }
    if (only_firm && buy && !g_asset_debt && g_on_forecast && !g_forecast_whole) {
        /* out of the cash alone, and what the month end will take is known for the debts only: that waits for the
         * month start, by when a month end has been watched */
        re_log("assets: firm %d during the month: nothing is bought on debt and the month end's payments are not known yet (no month "
               "end watched for this playthrough); left to the month start",
               only_firm);
        return 0;
    }
    snapshot(main_obj, &h);
    /* what may be spent. On debt: no limit. Otherwise the cash less what the forecast expects this month end to
     * take, so that a purchase never is why wages go unpaid */
    long long had = h.ok_balance ? h.cash : 0, taken_soon = g_on_forecast ? g_forecast_need : 0;
    long long cash = g_asset_debt ? LLONG_MAX : had - taken_soon;
    if (g_asset_debt)
        re_log("assets: cash %lld cents; what a purchase costs beyond it becomes the game's debt, so the cash is no limit", had);
    else
        re_log("assets: cash %lld cents, the month end is expected to take %lld: %lld may be spent on missing assets", had, taken_soon, cash);
    for (int f = 0; f < owned; f++) {
        int firm = own[f].id, types[64], records = 0, auto_buy = 0, taken = 0, units = 0, tight = 0;
        if (only_firm && firm != only_firm)
            continue;
        int kinds = re_game_firm_asset_types(firms, firm, types, 64, &records, &auto_buy, &taken);
        re_asset_need need[ASSET_NEEDS];
        re_asset_want want[ASSET_NEEDS];
        re_ware unit[ASSET_NEEDS];
        double used = 0.0, most = 0.0;
        int needs = re_game_firm_needs(firms, firm, need, ASSET_NEEDS);
        if (needs <= 0 || kinds < 0) {
            if (needs < 0 || kinds < 0)
                re_log("assets: '%s' (firm %d): what it wants or has of its assets could NOT be read; nothing is bought", own[f].name, firm);
            continue;
        }
        if (!own[f].open) {
            re_log("assets: '%s' (firm %d) is short of %d asset tag(s) and is closed: nothing is bought until it is open again",
                   own[f].name, firm, needs);
            continue;
        }
        if (!auto_buy || !taken) {
            re_log("assets: '%s' (firm %d) is short of %d asset tag(s); its switch \"buy worn-out assets again\" is %s, so nothing is "
                   "bought",
                   own[f].name, firm, needs, auto_buy ? "on but this month's staff hours for it are not taken" : "off");
            continue;
        }
        if (!re_game_firm_space(firms, firm, &used, &most)) {
            re_log("assets: '%s' (firm %d): its floor space could NOT be read; nothing is bought", own[f].name, firm);
            continue;
        }
        most = room_most(firm, most);
        for (int i = 0; i < needs; i++) {
            re_asset_offer offer[ASSET_WARES];
            int wares = re_game_tag_wares(firms, firm, need[i].tag, ware, ASSET_WARES);
            int missing = need[i].wanted - need[i].owned;
            long long pocket = g_asset_debt ? had - spent : cash; /* what the choice of a ware counts as "the cash covers it" */
            for (int w = 0; w < wares; w++) {
                int has = 0, others = 0;
                for (int t = 0; t < kinds; t++)
                    has |= types[t] == ware[w].type;
                for (int o = 0; o < owned && !g_trace_alone; o++)
                    others += own[o].id == firm ? 0 : re_game_firm_asset_units(firms, own[o].id, ware[w].type);
                offer[w] = (re_asset_offer){ware[w].active && !ware[w].fixed ? ware[w].power : 0, ware[w].space, ware[w].price, has, others,
                                            ware[w].hours, ware[w].running};
                if (g_on_trace)
                    re_log("assets (trace): '%s': tag %d: ware %d '%s' brings %d, floor space %.2f, %lld cents, a life of %.0f hours, uses "
                           "up %.1f cents an hour: an hour of one of the %d missing %.1f cents%s%s%s%s",
                           own[f].name, need[i].tag, ware[w].type, ware[w].name, ware[w].power, ware[w].space, ware[w].price, ware[w].hours,
                           ware[w].running, missing, offer[w].power > 0 ? re_business_asset_hour(&offer[w], missing) : 0.0,
                           has ? ", the business has it" : "", others ? ", the household's other businesses have it" : "",
                           ware[w].fixed ? ", one of a kind" : "", ware[w].active ? "" : ", not active");
            }
            int pick = wares > 0 ? re_business_asset_ware(offer, wares, missing, pocket, most - used) : -1;
            want[i] = (re_asset_want){need[i].wanted, need[i].owned, pick < 0 ? (re_asset_offer){0} : offer[pick]};
            if (pick >= 0)
                unit[i] = ware[pick];
            else {
                memset(&unit[i], 0, sizeof unit[i]);
                re_game_tag_name(firms, firm, need[i].tag, unit[i].name, sizeof unit[i].name);
            }
            re_log("assets: '%s' (firm %d): tag %d: has %d of %d wanted; %d ware(s) bring it, picked %d '%s'%s", own[f].name, firm,
                   need[i].tag, need[i].owned, need[i].wanted, wares, unit[i].type, unit[i].name,
                   pick < 0                   ? ""
                   : offer[pick].owned        ? ": the business has that kind"
                   : offer[pick].others > 0   ? ": the household's other businesses have that kind"
                   : offer[pick].price <= pocket ? ": the lowest cost of an hour among the wares the cash covers"
                                                 : ": the cash covers none, the lowest price");
        }
        re_log("assets: '%s' (firm %d): floor space %.2f of %.2f in use, cash %lld cents%s", own[f].name, firm, used, most,
               g_asset_debt ? had - spent : cash, buy ? "" : "; only looked at");
        if (!buy)
            continue;
        int count[ASSET_NEEDS] = {0};
        for (;;) {
            int next = units < ASSET_UNITS ? re_business_asset_next(want, needs, most - used, cash) : -1, before = records;
            if (next < 0)
                break;
            g_asset_paid = 0;
            g_asset_buying = 1;
            re_game_asset_replace(firms, firm, unit[next].type);
            g_asset_buying = 0;
            re_game_firm_asset_types(firms, firm, types, 64, &records, &auto_buy, &taken);
            if (records != before + 1) {
                re_log("assets: '%s' (firm %d): the game's function was called for ware %d '%s' and the business has %d asset record(s) "
                       "after %d: NOT bought, nothing more is tried for this business",
                       own[f].name, firm, unit[next].type, unit[next].name, records, before);
                want[next].unit.power = 0;
                break;
            }
            double was = used;
            re_game_firm_space(firms, firm, &used, &most);
            most = room_most(firm, most);
            re_log("assets: '%s' (firm %d): bought a unit of ware %d '%s' for %lld cents (looked up: %lld); tag %d: %d of %d now; floor space "
                   "%.2f -> %.2f of %.2f",
                   own[f].name, firm, unit[next].type, unit[next].name, g_asset_paid, unit[next].price, need[next].tag,
                   want[next].owned + unit[next].power, want[next].wanted, was, used, most);
            want[next].owned += unit[next].power;
            cash -= g_asset_paid;
            spent += g_asset_paid;
            count[next]++;
            units++;
            all_units++;
        }
        for (int i = 0; i < needs; i++) {
            int why = units >= ASSET_UNITS ? RE_ASSET_SERVED : re_business_asset_why(&want[i], most - used, cash);
            asset_note note = {"", "", count[i], why};
            snprintf(note.firm, sizeof note.firm, "%s", own[f].name);
            snprintf(note.ware, sizeof note.ware, "%s", unit[i].name);
            if (count[i] > 0)
                asset_month_note(&note);
            if (want[i].owned < want[i].wanted) {
                re_log("assets: '%s' (firm %d): tag %d stays short, %d of %d: %s", own[f].name, firm, need[i].tag, want[i].owned,
                       want[i].wanted,
                       why == RE_ASSET_NO_WARE   ? "no ware that brings it is for sale"
                       : why == RE_ASSET_NO_ROOM ? "a unit does not fit into the floor space"
                       : why == RE_ASSET_NO_CASH ? "the cash does not cover a unit"
                                                 : "the month's limit of units is reached");
                if (why != RE_ASSET_SERVED && blockeds < 16)
                    blocked[blockeds++] = note;
                tight += why == RE_ASSET_NO_ROOM;
            }
        }
        if (tight > 0)
            premises_grow(main_obj, firms, firm, own[f].name, "a missing asset does not fit");
    }
    char text[480], amount[64], owed[64];
    if (spent > 0 && h.ok_balance) {
        /* what did not leave the cash account, the game has booked as a debt */
        snapshot(main_obj, &h);
        debt = h.ok_balance ? spent - (had - h.cash) : 0;
        re_log("assets: %lld cents spent on %d unit(s); the cash went from %lld to %lld cents, so %lld became debt", spent, all_units, had,
               h.cash, debt);
    }
    g_asset_month_spent += spent;
    g_asset_month_debt += debt;
    if (all_units > 0) {
        /* the summary's line: everything bought in the month so far */
        const char *prefix = T(RE_MSG_ASSET_BUY_PREFIX);
        int len = snprintf(text, sizeof text, "%s", prefix), named = 0;
        for (; named < g_asset_months && named < 4 && (size_t)len < sizeof text - 80; named++)
            len += snprintf(text + len, sizeof text - (size_t)len, T(RE_MSG_ASSET_BUY_ITEM), named ? ", " : "", g_asset_month[named].firm,
                            g_asset_month[named].ware, g_asset_month[named].units);
        if (g_asset_months > named && (size_t)len < sizeof text)
            len += snprintf(text + len, sizeof text - (size_t)len, T(RE_MSG_HELD_MORE), g_asset_months - named);
        if ((size_t)len < sizeof text && g_asset_month_debt > 0)
            snprintf(text + len, sizeof text - (size_t)len, T(RE_MSG_ASSET_BUY_DEBT), money(main_obj, g_asset_month_spent, amount),
                     money(main_obj, g_asset_month_debt, owed));
        else if ((size_t)len < sizeof text)
            snprintf(text + len, sizeof text - (size_t)len, T(RE_MSG_ASSET_BUY_TOTAL), money(main_obj, g_asset_month_spent, amount));
        panel_line(main_obj, prefix, text, "UIIconFurnishings", 1);
        /* what was bought just now, in short on the ticker, the line across the top of the screen; urgent, or it is
         * lost among the lines of the month change */
        len = snprintf(text, sizeof text, T(RE_MSG_ASSET_TICK_BUY), all_units, money(main_obj, spent, amount));
        if (debt > 0)
            snprintf(text + len, sizeof text - (size_t)len, T(RE_MSG_ASSET_TICK_DEBT), money(main_obj, debt, owed));
        re_log("assets: ticker line %s: '%s'", g_can_text && re_game_post_urgent(main_obj, 1, text) ? "posted" : "NOT posted", text);
    }
    /* what could not be bought: up to four in the summary, the first on the ticker. At a month start only: during
     * the month the look is at one business and comes every 91st hour, the line would name that business alone and
     * the ticker would repeat itself */
    for (int line = 0; line < 2 && blockeds > 0 && !only_firm; line++) {
        const char *prefix = T(RE_MSG_ASSET_SHORT_PREFIX);
        int len = snprintf(text, sizeof text, "%s", prefix), named = 0;
        for (; named < blockeds && named < (line ? 1 : 4) && (size_t)len < sizeof text - 100; named++)
            len += snprintf(text + len, sizeof text - (size_t)len,
                            T(blocked[named].why == RE_ASSET_NO_ROOM   ? RE_MSG_ASSET_SHORT_ROOM
                              : blocked[named].why == RE_ASSET_NO_CASH ? RE_MSG_ASSET_SHORT_CASH
                                                                       : RE_MSG_ASSET_SHORT_NONE),
                            named ? ", " : "", blocked[named].firm, blocked[named].ware);
        if (blockeds > named && (size_t)len < sizeof text)
            snprintf(text + len, sizeof text - (size_t)len, T(RE_MSG_HELD_MORE), blockeds - named);
        if (line == 0)
            panel_line(main_obj, prefix, text, "UIIconFurnishings", 1);
        else
            re_log("assets: ticker line %s: '%s'", g_can_text && re_game_post_urgent(main_obj, 1, text) ? "posted" : "NOT posted", text);
    }
    return all_units;
}

/* trace: every employee of the household's businesses just before a month end, for seeing who can make a wage demand
 * and whose hours were not needed. The game asks a raise of a person employed for more than six months whose hourly
 * wage is under 0.8 of what a new person of that skill asks on average - the standard wage times g(E), g = 0.4 E +
 * 0.6 under 100% work efficiency and E from there on - and then in one month out of ten (note b19 "B2"). The hours
 * left are those of the month that is ending: the month end gives everybody the month's hours again. */
#define WAGE_TRACE_LINES 120
static void wage_trace(void *main_obj)
{
    static re_firm_fit own[OWN_FIRMS];
    void *firms = re_game_firms(main_obj), *jobs = re_game_jobs(firms);
    int owned = re_game_firm_fits(main_obj, own, OWN_FIRMS), now = re_game_period(main_obj), lines = 0;
    for (int f = 0; f < owned; f++) {
        BYTE *first = NULL;
        int count = re_game_firm_staff(firms, own[f].id, &first);
        for (int i = 0; i < count && lines < WAGE_TRACE_LINES; i++) {
            re_staff_read s;
            if (!re_game_staff_read(jobs, first + (SIZE_T)i * RE_STAFF_BYTES, &s))
                continue;
            double asked = (double)s.standard * (s.efficiency < 1.0 ? 0.4 * s.efficiency + 0.6 : s.efficiency);
            lines++;
            re_log("business: staff of firm %d: employee %d, job %d, wage %lld cents a month, %d hours, %d left, efficiency %.4f, standard "
                   "wage %lld cents an hour, ratio %.3f, %d months employed, automatic management %s, %s",
                   own[f].id, s.id, s.job, s.wage, s.hours, s.left, s.efficiency, s.standard,
                   s.hours > 0 && asked > 0.0 ? (double)s.wage / s.hours / asked : 0.0, now - s.start,
                   re_game_staff_auto(firms, own[f].id, s.id) ? "on" : "off", s.demand > s.wage ? "a wage demand open" : "no wage demand open");
        }
    }
}

/* A candidate is hired (REQUEST.md [33], note b19 "B1"). The game's automatic management is a switch per person that
 * only a click in the staff tab sets, so a business whose staff is managed automatically has a new person outside it
 * until the player clicks again. Here the new person gets the switch when somebody of that business has it. The
 * player sees it in the staff tab and can click it off. */
__attribute__((force_align_arg_pointer)) static void __fastcall hire_wrapper(void *staff, void *unused, int firm, const BYTE *record)
{
    typedef void(__thiscall *append_fn)(void *staff, int firm, const BYTE *record);
    (void)unused;
    void *firms = re_game_firms(g_main_obj);
    int employees = 0, managed = firms != NULL ? re_game_firm_managed(firms, firm, &employees) : -1;
    int id = re_readable(record, RE_STAFF_BYTES) ? *(const int *)(record + RE_STAFF_ID) : 0;
    ((append_fn)AT(RE_VA_STAFF_APPEND))(staff, firm, record);
    if (managed > 0 && id != 0)
        re_game_staff_manage(firms, firm, id);
    re_log("business: firm %d hires %d; %d of its %d employees were managed automatically, the new one %s", firm, id, managed, employees,
           managed > 0 && id != 0 ? "is now too" : "is left as the game has it");
}

static void business_install(void)
{
    if (g_auto_whole) {
        /* The row's click handler takes its branch "for everybody of the business" only when a Shift key is held;
         * here it takes it always, so that any row's switch is the switch of the business (REQUEST.md [42]). */
        static const BYTE held_only[2] = {0x75, 0x33}, always[2] = {0xeb, 0x33};
        g_auto_whole_patched = re_patch_bytes(AT(RE_VA_AUTO_CLICK_SHIFT), held_only, always, 2);
        re_log("business: a click on a staff row's automatic-management switch is for the whole business: %s",
               g_auto_whole_patched ? "patched" : "NOT patched");
    }
    if (g_asset_charge || g_asset_fill) {
        g_asset_charge_hooked = re_patch_call(AT(RE_VA_CALL_ASSET_PAY), AT(RE_VA_CHANGE_MONEY), asset_pay_wrapper);
        re_log("business: the cash movement of the game's replacement of an asset %s; a worn-out asset the game buys again is %s",
               g_asset_charge_hooked ? "redirected" : "NOT redirected", g_asset_charge ? "charged, not paid out" : "left as the game has it");
    }
    if (g_asset_fill) {
        int worn = g_asset_charge_hooked && re_patch_call(AT(RE_VA_CALL_ASSET_EXPIRED), AT(RE_VA_POST_MESSAGE), asset_notice_wrapper);
        int new_one = worn && re_patch_call(AT(RE_VA_CALL_ASSET_BOUGHT), AT(RE_VA_POST_MESSAGE), asset_notice_wrapper);
        g_asset_fill_hooked = worn && new_one;
        re_log("business: an asset a business is short of is bought after the month start, where the game's switch for worn-out assets "
               "is on: the two notices of the game's replacement %s",
               g_asset_fill_hooked ? "redirected" : "NOT redirected");
    }
    if (g_business_auto_new)
        re_log("business: automatic management for a person hired where others have it %s",
               re_patch_call(AT(RE_VA_CALL_HIRE_APPEND), AT(RE_VA_STAFF_APPEND), hire_wrapper) ? "redirected" : "NOT redirected");
    if (g_contracts_keep) {
        g_contracts_hooked = re_patch_call(AT(RE_VA_CALL_OFFERS_WINDOW), AT(RE_VA_OFFERS_WINDOW), offers_window_wrapper);
        re_log("business: the contracts of a business handed over by a Shift-click on its icon \"Contracts\" are signed by the mod at a "
               "month start, every offer the business has a place for and can carry: the icon's click %s; a signing costs %.0f hours "
               "of a store manager; the premises of a business handed over whole (Ctrl+Shift-click on an advert icon) are made larger "
               "when the floor space is short: %s",
               g_contracts_hooked ? "redirected" : "NOT redirected", g_contract_hours, g_premises_grow ? "on" : "off (settings)");
    }
    re_log("business: a closed business is left alone by the mod's rules until it is open again; the player is told when one with "
           "something automated is closed or opened: the game's call for a new state %s",
           re_patch_call(AT(RE_VA_CALL_FIRM_STATE), AT(RE_VA_FIRM_STATE), firm_state_wrapper) ? "redirected" : "NOT redirected");
    if (g_trace_hits) {
        static const BYTE entry[5] = {0x55, 0x8b, 0xec, 0x6a, 0xff};
        re_log("ui (trace): the places that hear \"the mouse is over this widget\" are logged: the game's function %s",
               re_detour5(AT(RE_VA_UI_HIT), entry, hit_trace_wrapper, &g_hit_trampoline) ? "redirected" : "NOT redirected");
    }
    int tick = 0;
    if (g_fill || g_ads_keep)
        tick = re_patch_call(AT(RE_VA_CALL_FIRM_TICK), AT(RE_VA_FIRM_TICK), firm_tick_wrapper);
    if (g_fill) {
        g_fill_hooked = tick;
        re_log("business: the game's block for a business every 91st hour %s; a job of automatically managed staff with more than "
               "%.1f%% of its month's work without hands gets a candidate hired, a hiring costs %.0f hours of a store manager",
               g_fill_hooked ? "redirected" : "NOT redirected", 100.0 * g_fill_share, g_hire_hours);
    }
    if (g_ads_keep) {
        /* the lists of the types' adverts come from the game's own two fetches; the click on an advert's icon hands a
         * business over; the block of every 91st hour is where the switches are set */
        re_business_brand_list = AT(RE_VA_DATA_LIST);
        re_business_brand = brand_seen;
        int window = re_patch_call(AT(RE_VA_CALL_BRAND_WINDOW), AT(RE_VA_DATA_LIST), re_business_brand_hook);
        int click = re_patch_call(AT(RE_VA_CALL_ADVERT_ON_CLICK), AT(RE_VA_ADVERT_ON), advert_on_wrapper) &&
                    re_patch_call(AT(RE_VA_CALL_ADVERT_CLICK), AT(RE_VA_ADVERT_SWITCH), advert_click_wrapper);
        g_ads_keep_hooked = tick && window && click;
        re_log("business: adverts on up to %.1f%% awareness for a business handed over by a Shift-click on an advert icon: the block of "
               "every 91st hour %s, the adverts of a type in the business window %s, the click on an advert icon %s; an advert's "
               "month looked after costs %.1f hours of a PR specialist",
               100.0 * g_ads_upto, tick ? "redirected" : "NOT redirected", window ? "redirected" : "NOT redirected",
               click ? "redirected" : "NOT redirected", g_ads_hours);
        if (g_lock && g_contracts_hooked && g_ads_keep_hooked) /* there is a whole business to hand over */
            lock_install();
        if (g_ads_keep_hooked) {
            /* the three calls of Color3B(r, g, b) go through the import table: `call [slot]`, six bytes. Each becomes
             * a call of the mod's stand-in and a nop */
            static const unsigned look[3] = {RE_VA_ADVERT_LOOK_BUILD, RE_VA_ADVERT_LOOK_ON, RE_VA_ADVERT_LOOK_OFF};
            unsigned slot = (unsigned)(UINT_PTR)AT(RE_VA_COLOUR3_NEW);
            int looks = re_patch_call(AT(RE_VA_CALL_ADVERT_ON_BUILD), AT(RE_VA_ADVERT_ON), advert_on_build_wrapper);
            for (int i = 0; looks && i < 3; i++) {
                BYTE *site = AT(look[i]), was[6] = {0xff, 0x15}, now[6] = {0xe8, 0, 0, 0, 0, 0x90};
                int to = (int)((UINT_PTR)advert_look_wrapper - (UINT_PTR)(site + 5));
                memcpy(was + 2, &slot, 4);
                memcpy(now + 1, &to, 4);
                looks = re_patch_bytes(site, was, now, 6);
            }
            g_advert_look_hooked = looks;
            re_log("business: the advert icons of a business handed over are orange: the window's question and the three colours %s",
                   looks ? "redirected" : "NOT redirected");
        }
    }
    if (g_wage_rule) {
        g_wage_hooked = re_patch_call(AT(RE_VA_CALL_WAGE_ACCEPT), AT(RE_VA_STAFF_SET_WAGE), wage_wrapper);
        re_log("business: wage demands of automatically managed staff %s; a candidate takes the place when cheaper by more than "
               "%.1f%%, a hiring costs %.0f hours of a store manager%s",
               g_wage_hooked ? "redirected" : "NOT redirected", 100.0 * g_wage_margin, g_hire_hours,
               g_wage_refuse_all ? "; TEST SETTING: refused whenever a candidate can take the place" : "");
    }
    re_business_list = AT(RE_VA_HIRE_LIST);
    re_business_assign = AT(RE_VA_STR_ASSIGN);
    re_business_listing = candidates_listing;
    re_business_listed = candidates_listed;
    re_business_name = AT(RE_VA_OBJECT_NAME);
    re_business_hire_line = business_hire_line;
    re_business_hire_return = (unsigned)(UINT_PTR)AT(RE_VA_CALL_HIRE_ROW + 5);
    g_business_sorted = re_patch_call(AT(RE_VA_CALL_HIRE_LIST), AT(RE_VA_HIRE_LIST), re_business_list_hook);
    /* the name's hook first: without the one that says which candidate a row is for it changes nothing */
    int name = re_patch_call(AT(RE_VA_CALL_ROW_NAME), AT(RE_VA_OBJECT_NAME), re_business_name_hook);
    int hire = name && re_patch_call(AT(RE_VA_CALL_HIRE_LINE), AT(RE_VA_STR_ASSIGN), re_business_hire_row_hook);
    re_log("business: hire list by the cost of an hour of work %s; the name on an action's row %s, the candidate of a row %s",
           g_business_sorted ? "redirected" : "NOT redirected", name ? "redirected" : "NOT redirected", hire ? "redirected" : "NOT redirected");
    /* Both need the hire tab's fetch of its list. A list from a seed needs the reference wage's seed too: without
     * it the wages would still follow what the game numbered before the look. */
    re_business_wage_ref = AT(RE_VA_WAGE_REF);
    re_business_wage_seed = list_wage_seed;
    g_fixed_hooked = g_candidates_fixed && g_business_sorted &&
                     re_patch_call(AT(RE_VA_CALL_WAGE_REF), AT(RE_VA_WAGE_REF), re_business_wage_ref_hook);
    re_log("business: the candidates of a job drawn from the playthrough and the month alone: %s",
           g_fixed_hooked ? "redirected" : g_candidates_fixed ? "NOT redirected" : "off (settings)");
}

/* trace, [trace] freeMoney=<dollars>: once, after the first load of the process, that much is put into the cash
 * account the way the "Free Money" button of the game's own debug tools does it (0x00646770: tag 2051, not
 * taxable). A save that does not last a few month ends becomes one they can be tried on; saved from the game's
 * menu it is a save for tests. A negative number takes cash away. */
#define TAG_FREE_MONEY 2051
static double g_trace_free_money;
static int free_money_trace(void *main_obj)
{
    static int done;
    void *finance = re_game_finance(main_obj);
    long long cents = (long long)(g_trace_free_money * 100.0);
    if (done || cents == 0 || finance == NULL)
        return 0;
    done = 1;
    ((money_fn)AT(RE_VA_CHANGE_MONEY))(finance, (unsigned)cents, (int)(cents >> 32), TAG_FREE_MONEY, 0, 0, 0);
    re_log("cash (trace): $%.2f put into the cash account as the game's debug tools do it (tag %d, not taxable)", dollars(cents), TAG_FREE_MONEY);
    return 1;
}

/* A save from before the mod (REQUEST.md [45], note m27). With the mod the game's "buy a worn-out asset again" pays
 * for the asset and buys missing ones too, on debt if need be, and its automatic staff management hires, lets go and
 * answers wage demands by the mod's rules. A household that had them on in the game's meaning did not choose the
 * mod's: at the first load of a playthrough the plugin has no record of, both are switched off in every business,
 * as a click switches them off, and a line of the summary says so. The player switches on again what is wanted.
 * The hour of that first load is kept: a save of the playthrough from before that hour is from before the mod too,
 * whenever it is loaded; one from a later hour has the player's own switches. In the hour itself the line tells
 * them apart, which a save written after the switching has in its month's list; where the first load found nothing
 * switched on there is no line, and every save of that hour is the player's. A playthrough that an earlier build
 * of the plugin has played is known and left as it is.
 * Only a switch the mod makes more of is switched off, as the settings are: the asset switch when the mod charges for
 * a replacement or buys what is missing, the staff's when it hires, lets go or answers wage demands. While none of
 * that is on (the business feature off, or this rule) the playthrough waits: the load at which the first of them is
 * on is its "first load", because up to then the switches kept the game's meaning. */
#define ADOPT_WAITING (-2) /* seen by this build, with nothing in force that gives a switch more to do */
static int g_adopt_switches = 1;

/* The casino games in the queues of a save from before the mod (REQUEST.md [45] and [48], note m27). The game books a
 * casino game's result in its first hour; with the mod a game has a stake, its result comes in its last hour and
 * stays when a save is loaded. A game queued under the first rule was not chosen under the second: it is taken out
 * of the queue, the way a right click takes it out, for the player to queue it again. One that had begun has its
 * result from the game already; taking it out gives the person the remaining hours back, and the household's entry
 * for that game is set to this month, as the game's last hour would have set it: it counts as played.
 * Which save is from before the mod is asked as for the switches, with a record of its own, because the casino
 * feature and the business feature are switched on and off apart: the first load of a playthrough with this rule in
 * force, and after it every save of that hour or an earlier one. A game begun under the mod is known by its start
 * on record and is never taken out. */
static int g_adopt_month;

static int casino_from_before(int activity, int gone)
{
    if (re_casino_game_of(activity) == NULL)
        return 0;
    long long begun = casino_get(activity, "Start", -1);
    /* under way and begun under the mod, as casino_end asks it */
    return gone <= 0 || begun == CASINO_NONE || begun > g_adopt_month || begun < g_adopt_month - 1;
}

/* `known`: the plugin had a record of the playthrough before this load. Writes its sentence behind the `n` bytes
 * `text` has, returns the new length. */
static int adopt_casino(void *main_obj, long long now, int known, char *text, int n, size_t size)
{
    int in_force = g_adopt_switches && g_on_casino && g_casino_installed;
    long long at = re_state_get(g_playthrough, "casinoAdoptedAt", -1);
    if (at == -1 && known) {
        re_state_set(g_playthrough, "casinoAdoptedAt", 0); /* played with the mod's casino up to here: its queue is the player's */
        return n;
    }
    if (at < 0 && !in_force) {
        if (at != ADOPT_WAITING)
            re_state_set(g_playthrough, "casinoAdoptedAt", ADOPT_WAITING);
        return n;
    }
    if (at < 0)
        re_state_set(g_playthrough, "casinoAdoptedAt", now);
    else if (now > at || !in_force)
        return n;
    re_queued taken[8];
    void *people = re_game_people(main_obj);
    g_adopt_month = re_game_period(main_obj);
    int begun = 0, count = people != NULL && g_adopt_month >= 0 ? re_game_queue_take_out(people, casino_from_before, taken, 8) : -1;
    if (count < 0) {
        re_log("adopt: the queues of the household's members could NOT be read: a casino game in them is left there");
        return n;
    }
    for (int i = 0; i < count && i < 8; i++) {
        int under_way = taken[i].gone > 0;
        if (under_way)
            re_game_set_cooldown(people, taken[i].activity, g_adopt_month);
        begun += under_way;
        re_log("adopt: %s (activity %d), %d of its %d hour(s) gone: taken out of the queue%s",
               re_lang_text(RE_LANG_EN, RE_MSG_CASINO_SLOTS + (int)(re_casino_game_of(taken[i].activity) - RE_CASINO)), taken[i].activity,
               taken[i].gone, taken[i].hours,
               under_way ? "; the game booked its result when it began, the household's entry for it is set to this month" : "");
    }
    re_log("adopt: the casino: a save from before the mod (hour %lld of the playthrough, the first load with the casino rule in force was "
           "at hour %lld): %d casino game(s) taken out of the queues of the household's members, %d of them begun (month %d)",
           now, at < 0 ? now : at, count, begun, g_adopt_month);
    if (count == 0 || n >= (int)size - 1)
        return n;
    n += snprintf(text + n, size - (size_t)n, "%s", n > 0 ? ". " : T(RE_MSG_ADOPT_PREFIX));
    if (n < (int)size - 1)
        n += begun > 0 ? snprintf(text + n, size - (size_t)n, T(RE_MSG_ADOPT_CASINO_BEGUN), count, begun)
                       : snprintf(text + n, size - (size_t)n, T(RE_MSG_ADOPT_CASINO), count);
    return n < (int)size ? n : (int)size - 1;
}

/* The two switches. Writes its sentence into `text` and returns its length, 0 when there is nothing to say. */
static int adopt_switches(void *main_obj, long long now, char *text, size_t size)
{
    static re_firm_fit own[OWN_FIRMS];
    const char *prefix = T(RE_MSG_ADOPT_PREFIX);
    int rule = g_adopt_switches && g_on_business;
    int for_assets = rule && (g_asset_charge_hooked || g_asset_fill_hooked);
    int for_staff = rule && (g_fill_hooked || g_wage_hooked || g_spare_months > 0);
    long long at = re_state_get(g_playthrough, "adoptedAt", -1);
    int first = at < 0;
    if (at == -1 && (re_state_get(g_playthrough, "guardFarthest", -1) != -1 || re_state_get(g_playthrough, "grade", -1) != -1 ||
                     re_state_get(g_playthrough, "flowPeriod", -1) != -1 || re_state_get(g_playthrough, "casinoSeed", 0) != 0)) {
        re_state_set(g_playthrough, "adoptedAt", 0);
        re_log("adopt: the first load of this playthrough with this build: an earlier build has played it, nothing switched off");
        return 0;
    }
    if (first && !for_assets && !for_staff) {
        if (at != ADOPT_WAITING) {
            re_state_set(g_playthrough, "adoptedAt", ADOPT_WAITING);
            re_log("adopt: the first load of a playthrough the plugin has no record of: nothing is switched off (settings); it waits "
                   "for the load at which the mod gives a switch more to do");
        }
        return 0;
    }
    if (first) {
        at = now;
        re_state_set(g_playthrough, "adoptedAt", at);
    } else if (now > at || (now == at && (re_state_get(g_playthrough, "adoptedNothing", 0) == 1 ||
                                         re_game_replace_event(main_obj, prefix, NULL) == 1))) {
        /* written with the mod in place: in the hour of the first load that is a save with the line, or any save when
         * that load found nothing switched on */
        if (g_on_trace)
            re_log("adopt: a save of hour %lld, the plugin's first load of the playthrough was at hour %lld: written with the mod in "
                   "place, its switches are the player's",
                   now, at);
        return 0;
    }
    if (!for_assets && !for_staff)
        return 0; /* an older save, and nothing in force now */
    void *firms = re_game_firms(main_obj);
    int owned = re_game_firm_fits(main_obj, own, OWN_FIRMS), assets = 0, staff = 0;
    for (int f = 0; f < owned; f++) {
        int was = for_assets ? re_game_firm_auto_assets_off(firms, own[f].id) : 0;
        int managed = for_staff ? re_game_firm_unmanage(firms, own[f].id) : 0;
        assets += was > 0;
        staff += managed > 0 ? managed : 0;
        re_log("adopt: '%s' (firm %d): \"buy a worn-out asset again\" %s, automatic management of %d employee(s) switched off", own[f].name,
               own[f].id, !for_assets ? "left alone (settings)" : was > 0 ? "switched off" : was == 0 ? "was off" : "NOT read", managed);
    }
    re_log("adopt: a save from before the mod (hour %lld of the playthrough, the plugin's first load of it was at hour %lld): %d "
           "business(es), %d with the asset switch on, %d employee(s) under automatic management; all off now%s",
           now, at, owned, assets, staff,
           for_assets && for_staff ? ""
           : for_assets            ? "; the staff's switch is not looked at and left alone (settings)"
                                   : "; the asset switch is not looked at and left alone (settings)");
    if (first && assets + staff == 0)
        re_state_set(g_playthrough, "adoptedNothing", 1);
    if (assets + staff == 0)
        return 0;
    int n = assets > 0 && staff > 0
                ? snprintf(text, size, T(RE_MSG_ADOPT), prefix, assets, staff)
                : snprintf(text, size, T(assets > 0 ? RE_MSG_ADOPT_ASSETS : RE_MSG_ADOPT_STAFF), prefix, assets > 0 ? assets : staff);
    return n < (int)size ? n : (int)size - 1;
}

static void adopt_save(void *main_obj)
{
    char text[720];
    long long now = re_game_ticks(main_obj);
    if (g_playthrough[0] == 0 || now < 0)
        return;
    /* asked before the switches' part writes its record: what an earlier build left, or an earlier load of this one */
    int known = re_state_get(g_playthrough, "adoptedAt", -1) != -1 || re_state_get(g_playthrough, "guardFarthest", -1) != -1 ||
                re_state_get(g_playthrough, "grade", -1) != -1 || re_state_get(g_playthrough, "flowPeriod", -1) != -1 ||
                re_state_get(g_playthrough, "casinoSeed", 0) != 0;
    int switches = adopt_switches(main_obj, now, text, sizeof text);
    int n = adopt_casino(main_obj, now, known, text, switches, sizeof text);
    if (n > 0) {
        panel_line(main_obj, T(RE_MSG_ADOPT_PREFIX), text, switches > 0 ? "UIIconFurnishings" : "ActionGamblingSlots", 1);
        re_game_post_urgent(main_obj, 1, text);
    }
}

/* ------------------------------------------------------------ call sites */

static int text_want_timed(const char *key, unsigned len, unsigned caller, const re_text_regs *regs)
{
    long long since = time_now();
    int want = text_want(key, len, caller, regs);
    time_add(TIME_TEXT, since);
    return want;
}

static unsigned text_edit_timed(int want, char *text, unsigned len, unsigned cap, const re_text_regs *regs,
                                const re_text_values *values)
{
    long long since = time_now();
    unsigned now = text_edit(want, text, len, cap, regs, values);
    time_add(TIME_TEXT, since);
    return now;
}

__attribute__((force_align_arg_pointer)) static void money_sink_timed(void *self, const unsigned *stack)
{
    long long since = time_now();
    money_sink(self, stack);
    time_add(TIME_MONEY, since);
}

static void after_game_step(void *main_obj, const char *why, int why_is_load)
{
    static re_household h; /* too large for the game's stack */
    snapshot(main_obj, &h);
    memcpy(g_playthrough, h.playthrough, sizeof g_playthrough);
    if (why_is_load)
        adopt_save(main_obj); /* before anything of this load writes into the plugin's record of the playthrough */
    if (why_is_load && g_on_trace && free_money_trace(main_obj))
        snapshot(main_obj, &h); /* the cash account changed */
    if (why_is_load && g_on_trace && g_on_business)
        thin_job_trace(main_obj);
    if (why_is_load && g_on_trace && g_on_stocks) {
        end_company_trace(main_obj);
        listing_trace(main_obj, "load");
    }
    if (why_is_load && g_on_trace && g_on_business)
        contracts_trace(main_obj, "load");
    if (why_is_load && g_casino_installed && casino_load(main_obj))
        snapshot(main_obj, &h); /* the cash account changed */
    if (g_on_trace)
        trace_state(main_obj, &h, why);
    if (g_on_guard)
        guard_step(main_obj, &h, why_is_load); /* before the forecast, whose line goes on top */
    /* after the guard: a load that moved the price stream has made the months paid before it another course's */
    if (why_is_load && g_on_stocks && research_kept(main_obj, "load", h.ok_calendar ? h.period : -1))
        snapshot(main_obj, &h); /* the cash account changed */
    if (why_is_load && g_on_forecast)
        flow_load(&h);
    else if (g_on_forecast)
        flow_commit(&h);
    if (g_on_credit)
        regrade(main_obj, &h, why_is_load ? 'L' : 'M'); /* leaves the debts priced at the new grade */
    if (g_on_stocks && h.ok_calendar)
        held_warning(main_obj, h.period + !why_is_load); /* the calendar still shows the month that just ended */
    if (g_on_forecast) {
        g_forecast_period = h.period + !why_is_load;
        forecast_update(main_obj, &h, 1); /* last: its line goes on top */
    }
    /* A load opens the summary panel inside the game's own routine, before this step, and the panel shows a copy
     * of the list as it was then: its rows are built again so that the lines written above are in it. After a
     * month end the game builds the panel later by itself (note b14). */
    if (why_is_load) { /* the month's purchases so far were another game's */
        g_asset_months = 0;
        g_asset_month_spent = g_asset_month_debt = 0;
    }
    if (why_is_load && g_on_business && g_on_trace && g_asset_fill_hooked)
        assets_fill(main_obj, 0, 0);
    if (why_is_load && g_on_business && g_business_fit)
        fit_alert(main_obj, "load");
    if (why_is_load && g_on_property)
        property_alert(main_obj, "load"); /* the saved list may have a property the saved line does not know of */
    if (why_is_load && g_can_text)
        re_log("summary panel after the load: %s", re_game_rebuild_summary(main_obj) ? "rows built again" : "not showing, left alone");
}

/* trace: how many blocks of each size the process holds, one line a size, for finding what a load leaves behind
 * (analysis/scripts/heap_sizes.py compares the lists of three loads). Sizes up to 64 KB count in steps of 8 bytes,
 * larger ones by their exact size. */
static void trace_heap(int load)
{
    enum { STEP = 8, BINS = 8192, LARGE = 1024, SAMPLES = 60 };
    static unsigned bins[BINS], large_size[LARGE], large_count[LARGE];
    static const void *sample[SAMPLES];
    static unsigned sample_size[SAMPLES];
    HANDLE heaps[64];
    DWORD heap_count = GetProcessHeaps(64, heaps);
    unsigned blocks = 0, larges = 0, dropped = 0, samples = 0, seen = 0;
    unsigned long long bytes = 0;
    memset(bins, 0, sizeof bins);
    for (DWORD i = 0; i < heap_count && i < 64; i++) {
        PROCESS_HEAP_ENTRY entry = {0};
        if (!HeapLock(heaps[i]))
            continue;
        while (HeapWalk(heaps[i], &entry)) {
            if (!(entry.wFlags & PROCESS_HEAP_ENTRY_BUSY))
                continue;
            blocks++;
            bytes += entry.cbData;
            for (int k = 0; k < g_heap_samples; k++)
                if (entry.cbData == (DWORD)g_heap_sample[k] && samples < SAMPLES && seen++ % 4001 == 0) {
                    sample_size[samples] = entry.cbData;
                    sample[samples++] = entry.lpData;
                }
            if (entry.cbData / STEP < BINS) {
                bins[entry.cbData / STEP]++;
                continue;
            }
            unsigned at = 0;
            while (at < larges && large_size[at] != entry.cbData)
                at++;
            if (at == LARGE) {
                dropped++;
                continue;
            }
            if (at == larges) {
                large_size[larges] = entry.cbData;
                large_count[larges++] = 0;
            }
            large_count[at]++;
        }
        HeapUnlock(heaps[i]);
    }
    for (unsigned i = 0; i < BINS; i++)
        if (bins[i])
            re_log("heap %d: size %u count %u", load, i * STEP, bins[i]);
    for (unsigned i = 0; i < larges; i++)
        re_log("heap %d: size %u count %u", load, large_size[i], large_count[i]);
    re_log("heap %d: total blocks %u bytes %llu in %lu heaps, large blocks not listed %u", load, blocks, bytes, heap_count, dropped);
    /* [trace] heapSample=<size>,<size>: what some blocks of those sizes hold, every 4001st of them, as 32-bit words */
    for (unsigned i = 0; i < samples; i++) {
        char line[16 * 9 + 1];
        int length = 0;
        if (!re_readable(sample[i], sample_size[i]))
            continue;
        for (unsigned word = 0; word < sample_size[i] / 4 && word < 16; word++)
            length += snprintf(line + length, sizeof line - (size_t)length, " %08x", ((const unsigned *)sample[i])[word]);
        re_log("heap %d: sample of size %u at %p:%s", load, sample_size[i], sample[i], line);
    }
}

__attribute__((force_align_arg_pointer)) static void __fastcall postload_wrapper(void *main_obj)
{
    /* The game has destroyed the previous scene before it comes here, and its routine below moves money and fetches
     * texts when it starts a new game: nothing may look at the old scene through a refresh in between. */
    g_main_obj = NULL;
    g_refresh_due = 0;
    g_credit_ready = 0;
    long long since = time_now();
    ((main_fn)AT(RE_VA_POSTLOAD))(main_obj);
    time_add(TIME_GAME_LOAD, since); /* the last step of the game's load, not the reading of the save */
    since = time_now();
    re_log("game loaded or started");
    if (g_fonts_later) {
        g_fonts_later = 0;
        re_font_install(g_on_trace);
    }
    if (g_on_trace && g_trace_heap)
        trace_heap(g_load_count + 1);
    if ((g_on_credit || g_trace_getter) && !g_getter_installed) {
        re_getter_policy = getter_policy;
        g_getter_installed = re_detour5(AT(RE_VA_GETTER), RE_GETTER_PROLOGUE, re_getter_hook, &re_getter_trampoline);
        re_log("loan-rate detour installed: %s", g_getter_installed ? "yes" : "NO (bytes differ or page not writable)");
    }
    if (g_on_credit && !g_loans_tried) {
        g_loans_tried = 1;
        re_loan_add_debt = AT(RE_VA_ADD_DEBT);
        re_loan_created = loan_created;
        g_edu_fixed = re_patch_call(AT(RE_VA_CALL_EDU_ADD_DEBT), AT(RE_VA_ADD_DEBT), re_loan_call_hook);
        int purchase = re_patch_call(AT(RE_VA_CALL_BUY_ADD_DEBT), AT(RE_VA_ADD_DEBT), re_loan_call_hook);
        int premium = fixed_premium_patch();
        re_log("fixed-rate loans: education loan hook %s, purchase hook %s, mortgage premium patch %s (fixed = variable + %.2f%% "
               "+ %g x (neutral rate - central-bank rate))",
               g_edu_fixed ? "installed" : "NOT installed", purchase ? "installed" : "NOT installed",
               premium ? "installed" : "NOT installed", 100.0 * g_fixed_premium, g_fixed_reversion);
    }
    if ((g_on_forecast || g_on_credit || (g_on_trace && g_trace_money)) && !g_money_installed) {
        re_trace_money_sink = money_sink_timed;
        g_money_installed = re_detour(AT(RE_VA_CHANGE_MONEY), RE_CHANGE_MONEY_PROLOGUE, sizeof RE_CHANGE_MONEY_PROLOGUE,
                                      re_trace_money_hook, &re_trace_money_trampoline);
        re_log("cash-movement detour installed: %s", g_money_installed ? "yes" : "NO (bytes differ or page not writable)");
    }
    /* every feature that changes a text the game fetches (text_want); with one of them missing here that feature works
     * and says nothing when it is the only one switched on (the casino's line of chances, the experience window's) */
    if ((g_on_credit || g_on_forecast || g_on_wording || g_on_ipo || g_on_stocks || g_on_business || g_on_casino || g_on_experience) &&
        g_can_text && !g_text_installed) {
        re_text_want = text_want_timed;
        re_text_edit = text_edit_timed;
        re_text_assign = AT(RE_VA_STR_ASSIGN);
        g_text_installed = re_detour(AT(RE_VA_TRANSLATE), RE_CHANGE_MONEY_PROLOGUE, sizeof RE_CHANGE_MONEY_PROLOGUE, re_text_hook,
                                     &re_text_trampoline);
        re_log("text detour installed: %s", g_text_installed ? "yes" : "NO (bytes differ or page not writable)");
    }
    if ((g_on_stocks || (g_on_wording && g_lang == RE_LANG_KO)) && g_stock_sites && g_text_installed && !g_stock_tried) {
        g_stock_tried = 1;
        re_text_row = stock_row;
        int rows = re_detour5(AT(RE_VA_STOCK_ROW), RE_GETTER_PROLOGUE, re_text_row_hook, &re_text_row_trampoline);
        re_log(g_on_stocks ? "stocks: ratios in the company list's hover text; row detour of the company tab %s"
                           : "stocks: off; row detour of the company tab %s, for the Korean label of the equity row only",
               rows ? "installed" : "NOT installed");
        if (g_on_stocks) {
            g_research_switch_hooked = re_patch_call(AT(RE_VA_CALL_STOCK_ROW_CTRL), AT(RE_VA_KEY_HELD), stock_row_ctrl_wrapper) &&
                                       re_patch_call(AT(RE_VA_CALL_STOCK_ROW_CTRL_RIGHT), AT(RE_VA_KEY_HELD), stock_row_ctrl_wrapper) &&
                                       re_patch_call(AT(RE_VA_CALL_STOCK_ROW_SHOW), AT(RE_VA_STOCK_SHOW), stock_row_show_wrapper);
            re_log("stocks: the research subscription is switched by a Shift-click on a company of the list (with Ctrl: every "
                   "company): the click's two questions about Ctrl and its showing of the company %s",
                   g_research_switch_hooked ? "redirected" : "NOT redirected");
            re_log("stocks: a research fee paid in a month stays paid when a save from before it is loaded: %s",
                   g_research_fee_kept ? "yes" : "no (settings)");
            if (g_research_switch_hooked) {
                re_action_button = AT(RE_VA_ACTION_BUTTON);
                re_action_button_none = research_button_none;
                re_log("stocks: no \"Research Stock\" button for a company the mod researches: the making of the button %s",
                       re_patch_call(AT(RE_VA_CALL_RESEARCH_BUTTON), AT(RE_VA_ACTION_BUTTON), re_action_button_hook) ? "redirected"
                                                                                                                      : "NOT redirected");
            }
            if (g_sort_buttons) {
                g_sort_hooked = re_patch_call(AT(RE_VA_CALL_STOCK_SORT_TYPE), AT(RE_VA_STOCK_SORT_TYPE), stock_sort_cheap) &&
                                re_patch_call(AT(RE_VA_CALL_STOCK_SORT_CAP), AT(RE_VA_STOCK_SORT_CAP), stock_sort_dear) &&
                                re_patch_call(AT(RE_VA_CALL_STOCK_SORT_EPS), AT(RE_VA_STOCK_SORT_EPS), stock_sort_change);
                re_log("stocks: three sort buttons of the company list (by type, by capitalisation, by earnings a share) order it by "
                       "the price against the fair price, the cheapest or the dearest first, and by the change in a month: the three "
                       "calls %s",
                       g_sort_hooked ? "redirected" : "NOT redirected");
                /* je rel32, the same instruction with the stand-in as its target */
                static const BYTE straight_on[6] = {0x0f, 0x84, 0xa9, 0x00, 0x00, 0x00};
                BYTE to_stand_in[6] = {0x0f, 0x84, 0, 0, 0, 0};
                INT32 distance = (INT32)((const BYTE *)(UINT_PTR)stock_sort_cap_hook - (AT(RE_VA_STOCK_SORT_LISTING) + 6));
                memcpy(to_stand_in + 2, &distance, 4);
                stock_sort_cap_on = AT(RE_VA_STOCK_SORT_ON);
                g_sort_cap_hooked = g_sort_hooked && re_patch_bytes(AT(RE_VA_STOCK_SORT_LISTING), straight_on, to_stand_in, sizeof straight_on);
                re_log("stocks: the second sort button (by listing date, which orders nothing) orders the list by capitalisation, the "
                       "largest first: its jump %s",
                       g_sort_cap_hooked ? "redirected" : "NOT redirected");
            }
            if (g_row_numbers)
                re_log("stocks: a row of the company list has the price against the month before behind the name, and for a "
                       "researched company the fair price: the copy of the name into the row's title %s",
                       re_patch_call(AT(RE_VA_CALL_STOCK_ITEM_NAME), AT(RE_VA_STR_COPY), stock_item_name_wrapper) &&
                               re_patch_call(AT(RE_VA_CALL_STOCK_ITEM_MADE), AT(RE_VA_ITEM_MADE), stock_item_made_wrapper)
                           ? "redirected"
                           : "NOT redirected");
            if (g_chart_price || g_chart_one || g_chart_fixes || g_chart_open) {
                /* both or neither: the ticking tells an opening from a page turn by what the builder's stand-in notes */
                g_chart_hooked = re_patch_call(AT(RE_VA_CALL_CHART_BOXES), AT(RE_VA_CHART_BOXES_FN), chart_boxes_wrapper) &&
                                 re_patch_call(AT(RE_VA_CALL_CHART_TICK_ALL), AT(RE_VA_CHART_TICK), chart_tick_wrapper);
                re_log("stocks: charts: a company's opens with the share price alone (%d), the economy's with the series of its button "
                       "(%d), colours of a list and the ticks kept over a page turn (%d), the wealth's, the index's and all share "
                       "prices with the lines read most and the economy's with five years (%d): the building of the boxes and their "
                       "ticking %s",
                       g_chart_price, g_chart_one, g_chart_fixes, g_chart_open, g_chart_hooked ? "redirected" : "NOT redirected");
            }
            if (g_chart_hooked && g_chart_fixes) {
                static const BYTE game_step[RE_CHART_TIME_STEP_BYTES] = {0x83, 0xf8, 0x18, 0x75, 0x0c, 0xc7, 0x81, 0x2c, 0x01, 0x00, 0x00, 0x3c,
                                                                         0x00, 0x00, 0x00, 0xeb, 0x1d, 0x83, 0xf8, 0x3c, 0x75, 0x0e, 0x8b, 0x81,
                                                                         0x30, 0x01, 0x00, 0x00, 0x89, 0x81, 0x2c, 0x01, 0x00, 0x00, 0xeb, 0x0a,
                                                                         0xc7, 0x81, 0x2c, 0x01, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00};
                BYTE mod_step[RE_CHART_TIME_STEP_BYTES], *site = AT(RE_VA_CHART_TIME_STEP);
                int to = (int)((UINT_PTR)chart_time_step - (UINT_PTR)(site + 5));
                /* call the mod's step with the chart in ecx, jump over the rest of the game's */
                memset(mod_step, 0x90, sizeof mod_step);
                mod_step[0] = 0xe8;
                memcpy(mod_step + 1, &to, 4);
                mod_step[5] = 0xeb;
                mod_step[6] = (BYTE)(RE_VA_CHART_TIME_DONE - (RE_VA_CHART_TIME_STEP + 7));
                int all = re_patch_call(AT(RE_VA_CALL_CHART_TICK_BOX_ALL), AT(RE_VA_CHART_TICK), chart_all_wrapper);
                int step = re_patch_bytes(site, game_step, mod_step, sizeof mod_step);
                re_log("stocks: charts: the box for all is for every page %s; the time button goes 6 months, 1 year, 2, 5, the most: its "
                       "step %s",
                       all ? "redirected" : "NOT redirected", step ? "replaced" : "NOT replaced");
                g_chart_steps = step;
            }
            g_chart_caps = g_chart_hooked && g_chart_open;
            if (g_chart_line > 0.0) {
                /* `call [slot of drawLine]`, six bytes, becomes a call of the mod's stand-in and a nop */
                BYTE *site = AT(RE_VA_CHART_LINE), was[6] = {0xff, 0x15}, now[6] = {0xe8, 0, 0, 0, 0, 0x90};
                unsigned slot = (unsigned)(UINT_PTR)AT(RE_VA_DRAW_LINE_SLOT);
                int to = (int)((UINT_PTR)chart_line_wrapper - (UINT_PTR)(site + 5));
                memcpy(was + 2, &slot, 4);
                memcpy(now + 1, &to, 4);
                re_log("stocks: charts: a line is drawn with a radius of %.2f: the drawing's call of drawLine %s", g_chart_line,
                       re_patch_bytes(site, was, now, 6) ? "replaced" : "NOT replaced");
            }
            if (g_chart_values)
                re_log("stocks: charts: a series' last value behind its name: the copy of the name for the box's label %s",
                       re_patch_call(AT(RE_VA_CALL_CHART_LABEL), AT(RE_VA_STR_COPY), chart_label_wrapper) ? "redirected" : "NOT redirected");
            if (g_chart_index) {
                int index = re_patch_call(AT(RE_VA_CALL_CHART_MONTH_FIRST), AT(RE_VA_MAP_COPY), chart_month_wrapper) &&
                            re_patch_call(AT(RE_VA_CALL_CHART_MONTH_SECOND), AT(RE_VA_MAP_COPY), chart_month_wrapper);
                re_log("stocks: charts: all share prices against the first month of the picture, as 100: the drawing's two copies of a "
                       "month %s",
                       index ? "redirected" : "NOT redirected");
                g_chart_index = index;
            }
        }
        if (g_on_wording && g_lang == RE_LANG_KO)
            re_log("wording: the Korean name of the equity a share in a chart's boxes: the call for a series' name %s",
                   re_patch_call(AT(RE_VA_CALL_CHART_NAME), AT(RE_VA_INFO_NAME), chart_name_wrapper) ? "redirected" : "NOT redirected");
    }
    if (g_on_guard && !g_guard_tried) {
        g_guard_tried = 1;
        re_guard_blocked = guard_blocked;
        re_guard_sell_step = AT(RE_VA_SELL_STEP);
        re_guard_sell_skip = AT(RE_VA_SELL_SKIP);
        int buy = re_detour(AT(RE_VA_STOCK_BUY), RE_STOCK_BUY_PROLOGUE, sizeof RE_STOCK_BUY_PROLOGUE, re_guard_buy_hook,
                            &re_guard_buy_trampoline);
        int sell = re_patch_call(AT(RE_VA_CALL_SELL_STEP), AT(RE_VA_SELL_STEP), re_guard_sell_hook);
        int futures = re_detour5(AT(RE_VA_FUTURES_OPEN), RE_GETTER_PROLOGUE, re_guard_futures_hook, &re_guard_futures_trampoline);
        re_log("trade hooks: buy %s, sell %s, futures %s; lock cap %d month(s), stream moved beyond the cap: %s",
               buy ? "installed" : "NOT installed", sell ? "installed" : "NOT installed", futures ? "installed" : "NOT installed",
               g_guard_cap_months, g_guard_shift ? "yes" : "no");
    }
    if (g_on_ipo && !g_ipo_tried) {
        g_ipo_tried = 1;
        g_ipo_installed = ipo_install();
    }
    if (g_month_fixed && !g_month_tried) { /* after the listing's own redirects: its price steps may be redirected already */
        g_month_tried = 1;
        re_guard_economy_month = AT(RE_VA_ECONOMY_MONTH);
        re_guard_stocks_month = AT(RE_VA_STOCKS_MONTH);
        re_guard_month_draws = month_draws;
        /* seeded at the first call, put back after the second: one of them alone is as good as none */
        g_month_hooked = re_patch_call(AT(RE_VA_CALL_ECONOMY_MONTH), AT(RE_VA_ECONOMY_MONTH), re_guard_economy_hook) &&
                         re_patch_call(AT(RE_VA_CALL_STOCKS_MONTH), AT(RE_VA_STOCKS_MONTH), re_guard_stocks_hook);
        if (!g_ipo_steps_hooked) {
            re_ipo_steps = AT(RE_VA_IPO_STEPS);
            g_ipo_steps_hooked = re_patch_call(AT(RE_VA_CALL_IPO_STEPS), AT(RE_VA_IPO_STEPS), re_ipo_steps_hook);
        }
        g_listing_hooked = g_ipo_steps_hooked;
        re_ipo_steps_draws = listing_draws;
        re_guard_property_month = AT(RE_VA_RESTATE_MONTH);
        re_guard_site_data = AT(RE_VA_SITE_DATA);
        re_guard_own_values = AT(RE_VA_OWN_VALUES);
        re_guard_site_draws = site_draws;
        g_property_month_hooked = re_patch_call(AT(RE_VA_CALL_RESTATE_MONTH), AT(RE_VA_RESTATE_MONTH), re_guard_property_hook);
        /* the three inside it only seed again what the call around them has kept: each works without the others */
        int parts = g_property_month_hooked &&
                    re_patch_call(AT(RE_VA_CALL_RENT_SITE_DATA), AT(RE_VA_SITE_DATA), re_guard_rent_site_hook) &&
                    re_patch_call(AT(RE_VA_CALL_SALE_SITE_DATA), AT(RE_VA_SITE_DATA), re_guard_sale_site_hook) &&
                    re_patch_call(AT(RE_VA_CALL_OWN_VALUES), AT(RE_VA_OWN_VALUES), re_guard_offers_hook);
        re_log("guard: a month end's economy and shares are drawn from the playthrough, the month and the guard's generation, whatever "
               "was done before it: %s; the price steps of a listing likewise: %s; the property market's month start likewise: %s, "
               "and inside it every site and the offers by themselves: %s",
               g_month_hooked ? "redirected" : "NOT redirected", g_listing_hooked ? "redirected" : "NOT redirected",
               g_property_month_hooked ? "redirected" : "NOT redirected", parts ? "redirected" : "NOT redirected");
    }
    if (g_on_board && !g_board_tried) {
        g_board_tried = 1;
        board_install();
    }
    if (g_on_casino && !g_casino_tried) {
        g_casino_tried = 1;
        casino_install();
    }
    if (g_on_stocks && !g_fut_tried) {
        g_fut_tried = 1;
        futures_install();
    }
    if (g_on_business && !g_business_tried) {
        g_business_tried = 1;
        business_install();
    }
    if (g_on_property && !g_property_tried) {
        g_property_tried = 1;
        int kind = re_patch_call(AT(RE_VA_CALL_SEARCH_ESTATE), AT(RE_VA_SEARCH_ESTATE), search_estate_wrapper);
        int place = re_patch_call(AT(RE_VA_CALL_SEARCH_SITE), AT(RE_VA_SEARCH_SITE), search_site_wrapper);
        re_log("property: the line is written anew when the character's search for a kind of property ends (%s) and when the search "
               "for a place ends (%s)",
               kind ? "redirected" : "NOT redirected", place ? "redirected" : "NOT redirected");
    }
    g_credit_ready = 0; /* a different save may have been loaded */
    g_new_fixed_id = 0;
    g_main_obj = main_obj;
    g_refresh_due = 0;
    g_load_count++;
    re_log("load #%d of this session", g_load_count);
    if (g_on_trace && g_load_count >= g_market_shift_from) {
        stream_advance(main_obj, RE_RANDGEN_MARKET, g_market_shift, "trace");
        stream_advance(main_obj, RE_RANDGEN_ECONOMY, g_economy_shift, "trace");
    }
    after_game_step(main_obj, "after load or new game", 1);
    time_add(TIME_LOAD, since);
    time_report("a load", re_game_period(main_obj));
}

__attribute__((force_align_arg_pointer)) static void __fastcall monthend_wrapper(void *main_obj)
{
    re_log("month end: start");
    long long since = time_now();
    memset(g_flow_now, 0, sizeof g_flow_now);
    g_guard_auto_refused = 0;
    g_main_obj = main_obj;
    g_refresh_due = 0;
    static re_fut_economy before, after; /* too large for the game's stack */
    void *economy = g_on_trace && g_fut_installed ? re_game_economy_object(main_obj) : NULL;
    int rates_read = economy != NULL && re_game_economy(economy, &before);
    if (g_on_trace && g_on_business)
        wage_trace(main_obj); /* before the answers: the open demands and the hours nobody needed are still there */
    g_wage_placed = 0;
    if (g_wage_hooked)
        wage_answers(main_obj);
    if (g_on_business && g_business_tried)
        staff_sizing(main_obj); /* after the answers: who is leaving over a demand is known */
    if (g_on_business && g_business_tried)
        adverts_month_end(main_obj);
    if (g_contracts_hooked)
        month_work_note(main_obj);
    if (g_on_trace && g_on_business)
        asset_replace_trace(main_obj);
    g_month_ended = re_game_period(main_obj);
    g_in_monthend = 1;
    time_add(TIME_MONTH_END, since); /* its two parts, before and after the game's, count as two */
    since = time_now();
    ((main_fn)AT(RE_VA_MONTHEND))(main_obj);
    time_add(TIME_GAME_MONTH_END, since);
    since = time_now();
    g_in_monthend = 0;
    re_log("month end: done");
    if (rates_read && re_game_economy(economy, &after))
        futures_month_trace(&before, &after);
    /* before the step below takes its picture of the household: the cost of a hiring is in the cash it sees. It is
     * not among the month end's payments the forecast repeats next month, it does not come every month. */
    if (g_on_business && g_business_tried)
        wage_places(main_obj);
    after_game_step(main_obj, "after month end", 0);
    time_add(TIME_MONTH_END, since);
}

/* The game's month start comes after its month end and before the summary panel opens. It empties the list of
 * properties for sale and fills it again, and it works out again what the contracts of the new month ask of a
 * business, so the two lines about them are written here. */
__attribute__((force_align_arg_pointer)) static void __fastcall monthstart_wrapper(void *main_obj)
{
    long long since = time_now();
    ((main_fn)AT(RE_VA_MONTHSTART))(main_obj);
    time_add(TIME_GAME_MONTH_START, since);
    since = time_now();
    if (g_on_trace && g_on_stocks)
        listing_trace(main_obj, "month start");
    if (g_on_trace && g_on_business)
        contracts_trace(main_obj, "month start");
    if (g_on_stocks && g_month_ended >= 0 && (g_research_board || g_research_sub || g_research_switch_hooked)) {
        static re_household h; /* too large for the game's stack */
        snapshot(main_obj, &h);
        research_refresh(main_obj, &h); /* here, not after the month end: the month's receipts are in the cash account */
    }
    if (g_on_stocks && g_month_ended >= 0)
        research_kept(main_obj, "month start", g_month_ended + 1); /* a month reached a second time, the subscription off since */
    g_month_ended = -1;
    if (g_on_business && g_contracts_hooked)
        contracts_sign(main_obj); /* the month's offers are made; a signed one brings its work from the next month */
    if (g_on_business && g_asset_fill_hooked)
        assets_fill(main_obj, 1, 0); /* before the line about work efficiency, which then says what is left */
    if (g_on_business && g_business_fit)
        fit_alert(main_obj, "month start");
    if (g_on_property)
        property_alert(main_obj, "month start");
    if (g_on_forecast && g_shown_due >= 0) {
        /* a bankruptcy at the month change pays or deletes the debts after the month end's line was written. The
         * line is written again here, where the panel is not built yet; left to the look during the month it would
         * show the debts that are gone until the panel is opened the next time */
        static re_household now; /* too large for the game's stack */
        snapshot(main_obj, &now);
        if (now.debt_count >= 0 && now.debts_due != g_shown_due) {
            re_log("forecast (month start): the debt pass will charge $%.2f, not the $%.2f of the month end's line: the line is written again",
                   dollars(now.debts_due), dollars(g_shown_due));
            forecast_update(main_obj, &now, 0);
        }
    }
    time_add(TIME_MONTH_START, since);
    time_report("a month has started", re_game_period(main_obj));
}

/* ---- an education kept (REQUEST.md [45], [48]; note m26)
 * The game takes 1% a month off every number a requirement is measured against, work experience and educations
 * alike, so a degree counts for less every year and has to be studied again by somebody who did not take up its
 * trade in time. For the members of the household the number of an education (a degree, a diploma, a certificate:
 * what was paid and studied for) now stays at what was ever gained of it, up to the most that any job, education or
 * activity of the game's data asks of it; what lies above that amount decays as before. Work experience is the
 * game's: it decays. The game's own step runs first and the numbers are put right after it, so a save from before
 * the mod is put right at its first month end. */
#define XP_NEEDS 256
#define XP_EDUCATIONS 128
static re_xp_need g_xp_need[XP_NEEDS];
static int g_xp_education[XP_EDUCATIONS];
static int g_xp_needs = -1, g_xp_educations, g_xp_requirements, g_xp_lines; /* -1: the data has not been read yet */

static void xp_data_file(const char *text, unsigned len, void *ctx)
{
    int found = re_xp_scan(text, len, g_xp_need, XP_NEEDS, &g_xp_needs);
    (void)ctx;
    if (found < 0)
        re_log("experience: more kinds of experience are asked for than the table takes (%d); the rest decays as the game has it", XP_NEEDS);
    else
        g_xp_requirements += found;
    if (re_xp_educations(text, len, g_xp_education, XP_EDUCATIONS, &g_xp_educations) < 0)
        re_log("experience: the data has more educations than the table takes (%d); the rest decays as the game has it", XP_EDUCATIONS);
}

typedef struct {
    int kinds, kept;
} xp_tally;

static void xp_keep(int tag, long long gained, long long lost, long long *effective, void *vctx)
{
    xp_tally *t = (xp_tally *)vctx;
    int most = re_xp_most(g_xp_need, g_xp_needs, tag), education = 0;
    for (int i = 0; i < g_xp_educations; i++)
        education |= g_xp_education[i] == tag;
    long long now = education && most > 0 ? re_xp_kept(*effective, gained - lost, most) : *effective;
    t->kinds++;
    if (g_on_trace && g_xp_lines++ < 400)
        re_log("experience: kind %d (%s): gained %lld, taken away %lld, the most a requirement asks %d: %lld after the game's month, "
               "%lld now",
               tag, education ? "an education" : "work experience", gained, lost, most, *effective, now);
    if (now == *effective)
        return;
    *effective = now;
    t->kept++;
}

void re_xp_decay(void *statement, float factor);
__attribute__((force_align_arg_pointer)) void re_xp_decay(void *statement, float factor)
{
    static int lines;
    long long since = time_now();
    if (g_xp_needs < 0) {
        g_xp_needs = 0;
        int files = re_data_files(xp_data_file, NULL);
        re_log("experience: %d requirement(s) for %d kind(s) of experience and %d education(s) read from %d file(s) of the game's data",
               g_xp_requirements, g_xp_needs, g_xp_educations, files);
    }
    re_game_xp_decay(statement, factor);
    xp_tally t = {0, 0};
    int walked = re_game_xp_walk(statement, xp_keep, &t);
    time_add(TIME_XP, since); /* with the game's own step, a walk over one map */
    if (g_on_trace && lines++ < 200)
        re_log("experience: a household member's month: the game's factor %.4f, %d kind(s) of experience (%d), %d education(s) kept from "
               "going below what was gained",
               factor, t.kinds, walked, t.kept);
}

/* the game's call: ecx = the statement, the factor in xmm1, nothing on the stack */
void re_xp_decay_hook(void);
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_xp_decay_hook\n"
        "_re_xp_decay_hook:\n"
        "  sub esp, 8\n"
        "  movss dword ptr [esp + 4], xmm1\n"
        "  mov dword ptr [esp], ecx\n"
        "  call _re_xp_decay\n"
        "  add esp, 8\n"
        "  ret\n"
        ".att_syntax prefix\n");

/* ----------------------------------------------------------------- start */

static void load_settings(void)
{
    g_on_credit = re_ini_int("features", "credit", 1);
    g_on_forecast = re_ini_int("features", "forecast", 1);
    g_on_wording = re_ini_int("features", "wording", 1);
    g_on_guard = re_ini_int("features", "guard", 1);
    g_on_trace = re_ini_int("features", "trace", 0);
    g_guard_cap_months = re_ini_int("guard", "maxLockMonths", 2);
    if (g_guard_cap_months < 0)
        g_guard_cap_months = 0;
    g_guard_shift = re_ini_int("guard", "shiftMarketBeyondCap", 1);
    g_month_fixed = re_ini_int("guard", "monthEndFixed", 1);
    g_on_ipo = re_ini_int("features", "ipo", 1);
    double keep = 0.45;
    re_ini_doubles("ipo", "founderKeeps", &keep, 1);
    g_ipo_keep = (float)(keep < 0.05 ? 0.05 : keep > 0.45 ? 0.45 : keep); /* 0.45 is the game's cap on a holding */
    g_ipo_floor = re_ini_int("ipo", "floorAtSaleValue", 1);
    g_ipo_taxable = re_ini_int("ipo", "cashTaxable", 1);
    re_ini_doubles("ipo", "gradeAbove", g_ipo_letters, RE_IPO_LETTERS);
    g_on_stocks = re_ini_int("features", "stocks", 1);
    g_research_board = re_ini_int("stocks", "researchOnBoard", 1);
    g_research_sub = re_ini_int("stocks", "researchSubscription", 0);
    re_ini_doubles("stocks", "researchFeeHours", &g_research_hours, 1);
    g_research_fee_kept = re_ini_int("stocks", "researchFeeKept", 1);
    g_sort_buttons = re_ini_int("stocks", "sortButtons", 1);
    g_chart_price = re_ini_int("stocks", "chartPriceOnly", 1);
    g_chart_one = re_ini_int("stocks", "chartEconomyOne", 1);
    g_chart_fixes = re_ini_int("stocks", "chartFixes", 1);
    g_chart_index = re_ini_int("stocks", "chartIndex", 1);
    g_chart_open = re_ini_int("stocks", "chartOpenWith", 1);
    g_chart_values = re_ini_int("stocks", "chartValues", 1);
    re_ini_doubles("stocks", "chartLineWidth", &g_chart_line, 1);
    if (!(g_chart_line >= 0.0) || g_chart_line > 8.0)
        g_chart_line = 2.0;
    g_row_numbers = re_ini_int("stocks", "rowNumbers", 1);
    g_fut_list = re_ini_int("stocks", "futuresList", 1);
    g_on_casino = re_ini_int("features", "casino", 1);
    g_casino_keep = re_ini_int("casino", "keepResultsOnLoad", 1);
    g_on_business = re_ini_int("features", "business", 1);
    g_business_fit = re_ini_int("business", "efficiencyAlert", 1);
    g_business_auto_new = re_ini_int("business", "autoManageNewStaff", 1);
    g_wage_rule = re_ini_int("business", "wageDemandRule", 1);
    g_wage_spare = re_ini_int("business", "wageNoRiseWhenSpare", 1);
    re_ini_doubles("business", "wageRefuseMargin", &g_wage_margin, 1);
    g_wage_margin = g_wage_margin < 0.0 ? 0.0 : g_wage_margin > 1.0 ? 1.0 : g_wage_margin;
    re_ini_doubles("business", "hireFeeHours", &g_hire_hours, 1);
    g_hire_hours = g_hire_hours < 0.0 ? 0.0 : g_hire_hours;
    g_candidates_fixed = re_ini_int("business", "candidatesFixed", 1);
    g_fill = re_ini_int("business", "autoHire", 1);
    re_ini_doubles("business", "autoHireShort", &g_fill_share, 1);
    g_fill_share = g_fill_share < 0.0 ? 0.0 : g_fill_share > 1.0 ? 1.0 : g_fill_share;
    g_hire_ahead = re_ini_int("business", "autoHireAhead", 1);
    g_lock = re_ini_int("business", "lockHandedOver", 1);
    g_on_memory = re_ini_int("features", "memory", 1);
    g_on_property = re_ini_int("features", "property", 1);
    g_on_experience = re_ini_int("features", "experience", 1);
    g_on_fonts = re_ini_int("features", "fonts", 1);
    re_ini_doubles("property", "alertBelowValue", &g_property_gap, 1);
    re_ini_doubles("property", "alertBelowAmount", &g_property_amount, 1);
    g_spare_months = re_ini_int("business", "spareMonths", 3);
    re_ini_doubles("business", "idleShare", &g_idle_share, 1);
    g_idle_share = g_idle_share < 0.1 ? 0.1 : g_idle_share > 1.0 ? 1.0 : g_idle_share;
    g_auto_whole = re_ini_int("business", "autoManageWholeBusiness", 1);
    g_adopt_switches = re_ini_int("business", "switchesOffOnFirstLoad", 1);
    g_asset_charge = re_ini_int("business", "assetReplacementCharged", 1);
    g_asset_fill = re_ini_int("business", "missingAssetsBought", 1);
    g_asset_debt = re_ini_int("business", "missingAssetsOnDebt", 1);
    g_ads_keep = re_ini_int("business", "advertsKeep", 1);
    re_ini_doubles("business", "advertsUpTo", &g_ads_upto, 1);
    g_ads_upto = g_ads_upto < 0.05 ? 0.05 : g_ads_upto > 1.3 ? 1.3 : g_ads_upto; /* above 133% an advert adds nothing */
    re_ini_doubles("business", "advertsFeeHours", &g_ads_hours, 1);
    g_contracts_keep = re_ini_int("business", "contractsKeep", 1);
    re_ini_doubles("business", "contractFeeHours", &g_contract_hours, 1);
    g_contract_hours = g_contract_hours < 0.0 ? 0.0 : g_contract_hours;
    g_premises_grow = re_ini_int("business", "premisesGrow", 1);
    g_property_gap = g_property_gap < 0.0 ? 0.0 : g_property_gap; /* 0 = that test is off; both 0 would name every property */
    g_property_amount = g_property_amount < 0.0 ? 0.0 : g_property_amount;
    if (g_property_gap == 0.0 && g_property_amount == 0.0)
        g_property_gap = 0.05;
    g_memory_managers = re_ini_int("memory", "deleteLeftObjects", 1);
    g_on_board = re_ini_int("features", "board", 1);
    re_ini_doubles("board", "holdingPerSeat", &g_board_per_seat, 1);
    if (!(g_board_per_seat >= 0.05)) /* 5% is what the game asks for a nomination */
        g_board_per_seat = 0.05;
    g_board_notice = re_ini_int("board", "electionNotice", 1);
    double top = g_board_top;
    if (g_on_trace) /* a test value for the guaranteed total; the game's own totals stay far below a float's exact range */
        re_ini_doubles("trace", "boardTotal", &top, 1);
    g_board_top = (float)top;
    g_board_prepass = g_on_trace && re_ini_int("trace", "boardAskAll", 0);
    g_wage_refuse_all = g_on_trace && re_ini_int("trace", "refuseWageDemands", 0); /* test runs: the rule says no to every demand */
    g_trace_replace_asset = g_on_trace ? re_ini_int("trace", "replaceAsset", 0) : 0;
    if (g_on_trace)
        re_ini_ints("trace", "thinJob", "", g_trace_thin, 2);
    g_trace_alone = g_on_trace && re_ini_int("trace", "assetsAlone", 0);
    if (g_on_trace)
        re_ini_doubles("trace", "poolShare", &g_trace_pool, 1);
    re_game_closed_unseen = g_on_trace && re_ini_int("trace", "closedUnseen", 0); /* the rules as before they knew a closed business */
    g_trace_hits = g_on_trace && re_ini_int("trace", "hits", 0);
    g_trace_end_company = g_on_trace && re_ini_int("trace", "endCompany", 0);
    g_trace_names = g_on_trace && re_ini_int("trace", "namesNoFont", 0);
    g_trace_contracts_unseen = g_on_trace && re_ini_int("trace", "signedContractsUnseen", 0);
    if (g_trace_contracts_unseen)
        re_log("business: TEST SETTING: who is one too many leaves whatever contract was signed in the month");
    if (g_on_trace)
        re_ini_doubles("trace", "roomMost", &g_trace_room, 1);
    if (g_on_trace)
        re_ini_doubles("trace", "freeMoney", &g_trace_free_money, 1);
    g_timing = re_ini_int("trace", "timing", 0);
    g_trace_money = re_ini_int("trace", "money", 1);
    g_trace_getter = g_on_trace && re_ini_int("trace", "getter", 1);
    g_trace_cashflow = re_ini_int("trace", "cashflow", 1);
    g_trace_heap = re_ini_int("trace", "heap", 0);
    g_heap_samples = re_ini_ints("trace", "heapSample", "", g_heap_sample, 4);
    g_market_shift = re_ini_int("trace", "marketShift", 0);
    g_economy_shift = re_ini_int("trace", "economyShift", 0);
    g_market_shift_from = re_ini_int("trace", "marketShiftFromLoad", 1);

    re_credit_defaults(&g_credit_cfg);
    re_ini_doubles("credit", "debtToAssets", g_credit_cfg.debt_assets_max, 3);
    re_ini_doubles("credit", "serviceToIncome", g_credit_cfg.service_income_max, 3);
    re_ini_doubles("credit", "liquidYears", g_credit_cfg.liquid_years_min, 3);
    re_ini_doubles("credit", "netWorth", g_credit_cfg.net_worth_min, 3);
    g_credit_cfg.index_net_worth = re_ini_int("credit", "indexNetWorth", g_credit_cfg.index_net_worth);
    re_ini_doubles("credit", "personalSpread", g_credit_cfg.personal_spread, RE_GRADE_COUNT);
    re_ini_doubles("credit", "mortgageSpread", g_credit_cfg.mortgage_spread, RE_GRADE_COUNT);
    re_ini_doubles("credit", "ltvBounds", g_credit_cfg.ltv_bound, 4);
    re_ini_doubles("credit", "ltvSpread", g_credit_cfg.ltv_spread, 5);
    re_ini_doubles("credit", "fixedPremium", &g_fixed_premium, 1);
    re_ini_doubles("credit", "fixedReversion", &g_fixed_reversion, 1);
    g_fixed_now = g_fixed_premium;
    /* finance tags of data/idmap.txt: 2061 trades, 2081 property, 2111 savings account, 2501 capital assets,
     * 2506 business sale, 2524 deposit paid; 2526 business costs, 2511 inventory, 2065 stock orders, 2531 advertising,
     * 2541 outsourcing; 2191 gambling */
    g_n_capital = re_ini_ints("credit", "capitalTags", "2061,2081,2111,2501,2506,2524", g_tags_capital, MAX_TAG_LIST);
    g_n_business = re_ini_ints("credit", "businessCostTags", "2526,2511,2065,2531,2541", g_tags_business, MAX_TAG_LIST);
    g_n_not_income = re_ini_ints("credit", "notIncomeTags", "2191", g_tags_not_income, MAX_TAG_LIST);
}

/* A data loader of the game is about to be destroyed: its XML documents are deleted here, the game's destructor
 * deletes none of them (re_memory.h). Runs at the end of every game, so once for every load. */
static void memory_ended(BYTE *loader)
{
    int docs = 0, lists = 0;
    for (unsigned i = 0; i <= RE_LOADER_DOC_LISTS; i++) {
        unsigned at = i < RE_LOADER_DOC_LISTS ? RE_LOADER_DOCS + i * RE_LOADER_DOCS_STEP : RE_LOADER_TEXT_DOCS;
        int deleted = re_memory_release(loader + at, AT(RE_VA_XML_DOC_TABLE), RE_XML_DOC_END_SLOT);
        if (deleted >= 0) {
            docs += deleted;
            lists++;
        }
    }
    g_memory_docs += docs;
    re_log("memory: a data loader ends, %d XML documents deleted from %d lists (%d since the game was started)", docs, lists,
           g_memory_docs);
}

/* The game's clean-up of an ended game deletes most of the objects that hold the game's state and leaves nine of them
 * (re_sites.h). Most of what a load leaves behind is theirs: the monthly records of the household, the businesses
 * and the listed companies. They are deleted here with the function boost has for each class, after the game's own
 * clean-up has run, and their places in the scene are emptied. The game gives the scene up right after this call
 * and never looks at it again; the plugin stops looking at it before. */
static const struct {
    unsigned place, destroy;
} LEFT_BEHIND[] = {
    {RE_MAIN_PFM, RE_VA_PFM_DESTROY},           {RE_MAIN_ECONOMY, RE_VA_ECONOMY_DESTROY}, {RE_MAIN_RESTATE, RE_VA_RESTATE_DESTROY},
    {RE_MAIN_FIRMS, RE_VA_FIRMS_DESTROY},       {RE_MAIN_MARKET, RE_VA_MARKET_DESTROY},   {RE_MAIN_RELATIONS, RE_VA_RELATIONS_DESTROY},
    {RE_MAIN_RICH, RE_VA_RICH_DESTROY},         {RE_MAIN_MOODS, RE_VA_MOODS_DESTROY},     {RE_MAIN_ACHIEVE, RE_VA_ACHIEVE_DESTROY},
};

__attribute__((force_align_arg_pointer)) static void __fastcall game_end_wrapper(BYTE *scene)
{
    typedef void(__stdcall * destroy_fn)(void *address);
    g_main_obj = NULL;
    g_refresh_due = 0;
    g_credit_ready = 0;
    g_fut_economy = NULL;
    ((main_fn)AT(RE_VA_GAME_END))(scene);
    if (!g_memory_delete)
        return; /* the memory feature is off: the wrapper is here for the four lines above alone */
    int deleted = 0;
    for (size_t i = 0; i < sizeof LEFT_BEHIND / sizeof LEFT_BEHIND[0]; i++) {
        void **place = (void **)(scene + LEFT_BEHIND[i].place);
        if (*place != NULL) {
            ((destroy_fn)AT(LEFT_BEHIND[i].destroy))(*place);
            *place = NULL;
            deleted++;
        }
    }
    re_log("memory: a game ends, %d of the %d objects its clean-up leaves deleted", deleted,
           (int)(sizeof LEFT_BEHIND / sizeof LEFT_BEHIND[0]));
}

static const char *state_of(int wanted, int possible)
{
    return !wanted ? "off (settings)" : possible ? "on" : "OFF (a code site does not match)";
}

/* The main menu's corner text, "v1.03.22", gets the mod's name and version behind it: a look at the menu tells whether
 * the mod is running. This stands in for the game's copy of that text on its way to the label. */
__attribute__((force_align_arg_pointer)) static void *__thiscall menu_version_copy(BYTE *copy, const void *from)
{
    typedef void *(__thiscall *copy_fn)(void *self, const void *from);
    void *result = ((copy_fn)AT(RE_VA_STR_COPY))(copy, from);
    unsigned size = *(const unsigned *)(copy + 0x10), cap = *(const unsigned *)(copy + 0x14);
    const char *version = cap >= 16 ? *(const char *const *)copy : (const char *)copy;
    char text[96];
    if (size < 32 && re_readable(version, size)) {
        while (size && version[size - 1] == ' ')
            size--;
        int len = snprintf(text, sizeof text, "%.*s + Realistic Economy " PLUGIN_VERSION, (int)size, version);
        ((assign_fn)AT(RE_VA_STR_ASSIGN))(copy, text, (unsigned)len);
    }
    return result;
}

/* A name in letters no font file of the game has (re_font.h, REQUEST.md [48]). Two functions of the game hand such a
 * name on: the one that picks a line of a list of names, for every new person, rival and company, and the one that
 * reads a string of a save, for everybody a save already has. Each is let run, and what it filled is put right. */
static void *g_name_pick_trampoline, *g_archive_string_trampoline;

static void name_plain(game_string *name, const char *from)
{
    char plain[128];
    const char *text = name->capacity >= 16 ? *(const char *const *)(const void *)name->text : name->text;
    int len = re_font_name(text, name->size, plain, sizeof plain);
    if (len <= 0)
        return;
    re_log("names: '%.*s', %s, has letters no font of the game has and is written '%s'", (int)name->size, text, from, plain);
    ((assign_fn)AT(RE_VA_STR_ASSIGN))(name, plain, (unsigned)len);
}

__attribute__((force_align_arg_pointer)) static void *__stdcall name_pick_wrapper(game_string *name, void *random, game_string list)
{
    typedef void *(__stdcall * pick_fn)(game_string * name, void *random, game_string list);
    static const char *const NO_FONT[3] = {"Arash (\xd8\xa2\xd8\xb1\xd8\xb4)", "\xd8\xb9\xd8\xa7\xd9\x84\xdb\x8c\xd8\xb4\xd8\xa7\xd9\x87",
                                           "\xef\xb7\xb4"}; /* the three lines of IRFirstNamesM.txt */
    static unsigned next;
    void *result = ((pick_fn)g_name_pick_trampoline)(name, random, list); /* the copy handed on is that function's to destroy */
    if (g_trace_names) {
        const char *made = NO_FONT[next++ % 3];
        ((assign_fn)AT(RE_VA_STR_ASSIGN))(name, made, (unsigned)strlen(made));
    }
    if (g_on_names)
        name_plain(name, "picked from a list of names");
    return result;
}

__attribute__((force_align_arg_pointer)) static void __fastcall archive_string_wrapper(void *archive, void *unused, game_string *string)
{
    typedef void(__thiscall * read_fn)(void *archive, game_string *string);
    (void)unused;
    ((read_fn)g_archive_string_trampoline)(archive, string);
    long long since = time_now();
    name_plain(string, "read from a save");
    time_add(TIME_SAVE_STRING, since);
}

BOOL WINAPI DllMain(HINSTANCE self, DWORD reason, LPVOID reserved)
{
    (void)reserved;
    if (reason != DLL_PROCESS_ATTACH)
        return TRUE;
    DisableThreadLibraryCalls(self);
    re_util_init(self);
    load_settings();

    SYSTEMTIME now;
    GetLocalTime(&now);
    g_base = (BYTE *)GetModuleHandleW(NULL);
    re_game_init(g_base);
    const IMAGE_DOS_HEADER *dos = (const IMAGE_DOS_HEADER *)g_base;
    const IMAGE_NT_HEADERS32 *nt = (const IMAGE_NT_HEADERS32 *)(g_base + dos->e_lfanew);
    DWORD stamp = nt->FileHeader.TimeDateStamp, size = nt->OptionalHeader.SizeOfImage;

    re_log("[%04u-%02u-%02u %02u:%02u:%02u] Realistic Economy %s attached, pid=%lu", now.wYear, now.wMonth, now.wDay, now.wHour,
           now.wMinute, now.wSecond, PLUGIN_VERSION, GetCurrentProcessId());
    re_log("  host base=%p link_timestamp=%lu size_of_image=0x%lx", (void *)g_base, stamp, size);
    if (stamp != RE_PIN_TIMESTAMP || size != RE_PIN_SIZE_OF_IMAGE) {
        re_log("  build pin: NO MATCH - this is not game version 1.0322, nothing is patched");
        return TRUE;
    }
    re_log("  build pin: MATCH");
    int core = re_sites_verify(g_base, RE_GROUP_CORE, report_site) == 0;
    int finance = re_sites_verify(g_base, RE_GROUP_FINANCE, report_site) == 0;
    int credit = re_sites_verify(g_base, RE_GROUP_CREDIT, report_site) == 0;
    int money = re_sites_verify(g_base, RE_GROUP_MONEY, report_site) == 0;
    int text = re_sites_verify(g_base, RE_GROUP_TEXT, report_site) == 0;
    int guard = re_sites_verify(g_base, RE_GROUP_GUARD, report_site) == 0;
    int month = re_sites_verify(g_base, RE_GROUP_MONTH, report_site) == 0;
    int ipo = re_sites_verify(g_base, RE_GROUP_IPO, report_site) == 0;
    int board = re_sites_verify(g_base, RE_GROUP_BOARD, report_site) == 0;
    int stocks = re_sites_verify(g_base, RE_GROUP_STOCKS, report_site) == 0;
    int casino = re_sites_verify(g_base, RE_GROUP_CASINO, report_site) == 0;
    int business = re_sites_verify(g_base, RE_GROUP_BUSINESS, report_site) == 0;
    int memory = re_sites_verify(g_base, RE_GROUP_MEMORY, report_site) == 0;
    int property = re_sites_verify(g_base, RE_GROUP_PROPERTY, report_site) == 0;
    int trace = re_sites_verify(g_base, RE_GROUP_TRACE, report_site) == 0;
    int experience = re_sites_verify(g_base, RE_GROUP_EXPERIENCE, report_site) == 0;
    int names = re_sites_verify(g_base, RE_GROUP_NAMES, report_site) == 0;

    re_log("  feature credit:   %s", state_of(g_on_credit, core && finance && credit));
    re_log("  feature forecast: %s", state_of(g_on_forecast, core && finance && money));
    re_log("  feature guard:    %s", state_of(g_on_guard, core && finance && guard));
    /* the streams are reached and seeded the way the candidates of a job are: those sites are the business group's */
    re_log("  guard, a month end drawn from the month: %s",
           state_of(g_on_guard && g_month_fixed, core && finance && guard && month && business));
    re_log("  feature ipo:      %s", state_of(g_on_ipo, core && ipo));
    re_log("  feature board:    %s", state_of(g_on_board, core && board));
    re_log("  feature stocks:   %s", state_of(g_on_stocks, core && text && stocks));
    re_log("  feature casino:   %s", state_of(g_on_casino, core && finance && money && text && casino));
    re_log("  feature business: %s", state_of(g_on_business, core && text && business));
    re_log("  feature memory:   %s", state_of(g_on_memory, memory));
    re_log("  feature property: %s", state_of(g_on_property, core && property));
    re_log("  feature experience: %s", state_of(g_on_experience, experience));
    /* the letters: no site of the game's file, two functions of libcocos2d.dll; the names: two functions of the game */
    re_log("  feature fonts:    %s; its names in Latin letters: %s", state_of(g_on_fonts, 1), state_of(g_on_fonts, text && names));
    re_log("  feature trace:    %s", state_of(g_on_trace, core && finance && credit && money && trace));
    g_lang = re_lang_of(re_language());
    re_log("  text in the game's windows: %s; language folder '%s', the plugin's texts in '%s'",
           text ? "on" : "OFF (a code site does not match), log only", re_language(), re_lang_folder(g_lang));
    g_on_credit = g_on_credit && core && finance && credit;
    g_on_forecast = g_on_forecast && core && finance && money;
    g_on_guard = g_on_guard && core && finance && guard;
    g_month_fixed = g_month_fixed && g_on_guard && month && business;
    g_on_ipo = g_on_ipo && core && ipo;
    g_on_board = g_on_board && core && board;
    g_stock_sites = core && text && stocks;
    g_on_stocks = g_on_stocks && g_stock_sites;
    g_research_sub = g_research_sub && money; /* the fee is booked with the game's cash function */
    g_research_fee_kept = g_research_fee_kept && money;
    g_on_casino = g_on_casino && core && finance && money && text && casino;
    g_on_business = g_on_business && core && text && business;
    g_on_memory = g_on_memory && memory;
    g_on_property = g_on_property && core && property;
    g_on_experience = g_on_experience && experience;
    g_on_trace = g_on_trace && core && finance && credit && money && trace;
    g_trace_getter = g_trace_getter && g_on_trace;
    g_wage_refuse_all = g_wage_refuse_all && g_on_trace;
    g_can_text = text;
    int worded = strcmp(re_language(), re_lang_folder(g_lang)) == 0 && re_wording_has(g_lang);
    re_log("  feature wording:  %s", !worded ? "off (no corrections for the game's language)" : state_of(g_on_wording, core && text));
    g_on_wording = g_on_wording && worded && core && text;
    if (!g_on_credit && !g_on_forecast && !g_on_wording && !g_on_guard && !g_on_ipo && !g_on_board && !g_on_stocks && !g_on_casino &&
        !g_on_business && !g_on_memory && !g_on_property && !g_on_experience && !g_on_fonts && !g_on_trace) {
        re_log("  no feature is on, nothing is patched");
        return TRUE;
    }
    if (g_on_fonts)
        g_fonts_later = re_font_install(g_on_trace) < 0; /* before the game makes its first font, when the library is loaded */
    g_on_names = g_on_fonts && text && names;
    g_trace_names = g_trace_names && g_on_trace && text && names;
    if (g_on_names || g_trace_names) { /* here, before a save is read: the first load is too late for the people of that save */
        static const BYTE pick_entry[6] = {0x53, 0x8b, 0xdc, 0x83, 0xec, 0x08}; /* push ebx / mov ebx, esp / sub esp, 8 */
        int picked = re_detour(AT(RE_VA_NAME_PICK), pick_entry, sizeof pick_entry, name_pick_wrapper, &g_name_pick_trampoline);
        int read = g_on_names &&
                   re_detour5(AT(RE_VA_ARCHIVE_STRING), RE_GETTER_PROLOGUE, archive_string_wrapper, &g_archive_string_trampoline);
        g_on_names = g_on_names && picked;
        g_trace_names = g_trace_names && picked;
        if (g_on_fonts)
            re_log("names: a name in letters no font of the game has is written in Latin letters: the pick of a name %s, the reading of a "
                   "save's text %s",
                   picked ? "redirected" : "NOT redirected", read ? "redirected" : "NOT redirected");
        if (g_trace_names)
            re_log("names: TEST SETTING: every name the game picks is made one of the three in letters no font has");
    }
    int a = re_patch_call(AT(RE_VA_CALL_POSTLOAD), AT(RE_VA_POSTLOAD), postload_wrapper);
    int b = re_patch_call(AT(RE_VA_CALL_MONTHEND), AT(RE_VA_MONTHEND), monthend_wrapper);
    int c = re_patch_call(AT(RE_VA_CALL_MONTHSTART), AT(RE_VA_MONTHSTART), monthstart_wrapper);
    re_log("  call sites redirected: post-load=%d month-end=%d month-start=%d", a, b, c);
    if (g_on_experience) {
        g_xp_hooked = re_patch_call(AT(RE_VA_CALL_XP_DECAY), AT(RE_VA_XP_DECAY), re_xp_decay_hook);
        re_log("experience: a household member's educations are kept up to what requirements ask, work experience decays as in the game: "
               "the month end's decay %s",
               g_xp_hooked ? "redirected" : "NOT redirected");
    }
    if (text)
        re_log("  main menu: the mod's name behind the game's version: %d",
               re_patch_call(AT(RE_VA_CALL_MENU_VERSION), AT(RE_VA_STR_COPY), menu_version_copy));
    if (g_on_memory) { /* the entry of the documents' table has to be the function the sites describe */
        re_memory_ended = memory_ended;
        g_on_memory = *(BYTE **)(AT(RE_VA_XML_DOC_TABLE) + RE_XML_DOC_END_SLOT) == AT(RE_VA_XML_DOC_END) &&
                      re_detour5(AT(RE_VA_LOADER_END), RE_GETTER_PROLOGUE, re_memory_end_hook, &re_memory_end_trampoline);
        re_log("  memory: a data loader deletes its XML documents when it ends: %d", g_on_memory);
    }
    /* Whatever the memory feature is set to: the plugin keeps a pointer to the running game and looks at it from the
     * text detour when cash has moved (refresh_if_due), also from the main menu's texts. With the game's clean-up not
     * wrapped that pointer outlives the game it points to. */
    g_memory_delete = g_on_memory && g_memory_managers;
    if (memory)
        re_log("  game end: the plugin lets go of an ended game there: the call %s; its nine left objects are deleted: %s",
               re_patch_call(AT(RE_VA_CALL_GAME_END), AT(RE_VA_GAME_END), game_end_wrapper) ? "redirected" : "NOT redirected",
               g_memory_delete ? "yes" : "no (settings)");
    return TRUE;
}
