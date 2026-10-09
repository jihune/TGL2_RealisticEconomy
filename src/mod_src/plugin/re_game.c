#include "re_game.h"

#include <string.h>

#include "re_business.h"
#include "re_futures.h"
#include "re_hook.h"
#include "re_sites.h"

typedef long long(__thiscall *this_fn0)(void *self);
typedef long long(__thiscall *this_fn1)(void *self, int flag);
typedef int(__thiscall *this_int_fn0)(void *self);
typedef long long(__thiscall *instalment_fn)(void *self, int activity, unsigned lo, int hi, int months, float rate);

static BYTE *g_base;
#define AT(va) (g_base + ((va) - RE_IMAGE_BASE))

void re_game_init(BYTE *image_base)
{
    g_base = image_base;
}

/* MSVC std::map node: left, parent, right, colour byte, isnil byte at +0xd, value from +0x10.
 * The map object is {head node pointer, element count}. */
#define NODE_HEADER 0x10
static const BYTE *node_ptr(const BYTE *n, int slot)
{
    return *(const BYTE *const *)(n + slot * 4);
}

typedef int (*node_visit)(const BYTE *node, void *ctx);

/* In-order walk. Returns the element count, or -1 when a pointer is unreadable or the count disagrees with the
 * stored size - the caller then treats the whole structure as not understood. */
static int map_walk(const BYTE *map, SIZE_T node_bytes, node_visit visit, void *ctx)
{
    if (!re_readable(map, 8))
        return -1;
    const BYTE *head = node_ptr(map, 0);
    unsigned size = *(const unsigned *)(map + 4), count = 0;
    if (size > 100000 || !re_readable(head, NODE_HEADER) || head[0xd] == 0)
        return -1;
    const BYTE *n = node_ptr(head, 0);
    while (n != head) {
        if (count >= size || !re_readable(n, node_bytes) || n[0xd] != 0)
            return -1;
        count++;
        if (visit && !visit(n, ctx))
            return -1;
        const BYTE *right = node_ptr(n, 2);
        if (!re_readable(right, NODE_HEADER))
            return -1;
        if (right[0xd] == 0) {
            n = right;
            for (;;) {
                const BYTE *left = node_ptr(n, 0);
                if (!re_readable(left, NODE_HEADER))
                    return -1;
                if (left[0xd] != 0)
                    break;
                n = left;
            }
        } else {
            const BYTE *parent = node_ptr(n, 1);
            for (;;) {
                if (!re_readable(parent, NODE_HEADER))
                    return -1;
                if (parent[0xd] != 0 || n != node_ptr(parent, 2))
                    break;
                n = parent;
                parent = node_ptr(n, 1);
            }
            n = parent;
        }
    }
    return count == size ? (int)count : -1;
}

/* ---- inflation index: std::map<int, float> at EconomyM + 0xb0, key 1531 ---- */
static int visit_index(const BYTE *n, void *ctx)
{
    if (*(const int *)(n + 0x10) == 1531)
        *(double *)ctx = *(const float *)(n + 0x14);
    return 1;
}

/* ---- debts: std::map<int, Instrument> at PFM + 0xa8 + 0xa0, record at node + 0x18 ---- */
typedef struct {
    re_household *h;
    BYTE *debtinv;
} debt_ctx;

static void read_debt(const BYTE *record, re_debt *d)
{
    d->type = *(const int *)record;
    d->stored_rate = *(const float *)(record + 4);
    d->id = *(const int *)(record + 8);
    d->activity = *(const int *)(record + 0xc);
    d->months_left = *(const int *)(record + 0x10);
    d->length_months = *(const int *)(record + 0x14);
    d->principal = *(const long long *)(record + 0x18);
    d->delay_months = *(const int *)(record + 0x30);
    d->due = d->year = 0;
}

static int visit_debt(const BYTE *n, void *vctx)
{
    debt_ctx *c = (debt_ctx *)vctx;
    re_household *h = c->h;
    re_debt beyond, *d = h->debt_count < RE_MAX_DEBTS ? &h->debts[h->debt_count] : &beyond; /* every debt is summed */
    h->debt_count++;
    read_debt(n + 0x18, d);
    if (d->delay_months > 0)
        return 1; /* grace period: the monthly pass only counts it down */
    instalment_fn instalment = (instalment_fn)AT(RE_VA_INSTALMENT);
    if (d->type == 1) {
        d->due = instalment(c->debtinv, d->activity, (unsigned)d->principal, (int)(d->principal >> 32), d->months_left, d->stored_rate);
        d->year = (d->months_left < 12 ? d->months_left : 12) * d->due; /* a loan about to end has fewer left */
    } else if (d->type == 2) {
        /* A bullet is collected in the pass that takes its months from 1 to 0, with that month's interest on top.
         * A one-month annuity is exactly that sum; the game's function answers 0 when the rate is 0. */
        long long once = instalment(c->debtinv, d->activity, (unsigned)d->principal, (int)(d->principal >> 32), 1, d->stored_rate);
        if (once < d->principal)
            once = d->principal;
        if (d->months_left <= 1)
            d->due = once;
        if (d->months_left <= 12)
            d->year = once; /* it falls due once: not twelve times a year, and not in the months before either */
    }
    h->debts_due += d->due;
    h->debts_year += d->year;
    return 1;
}

typedef struct {
    int id;
    float *rate;
} rate_ctx;

static int visit_debt_rate(const BYTE *n, void *vctx)
{
    rate_ctx *c = (rate_ctx *)vctx;
    if (*(const int *)(n + 0x20) == c->id)
        c->rate = (float *)(UINT_PTR)(n + 0x1c);
    return 1;
}

/* ---- cash-flow record: IncomeSt at PFM + 0x2d0, std::map<period, IncomePeriod> at + 0x20.
 * Outer node: period at +0x10, then {expense map, expense total, income map, income total} from +0x18.
 * Inner node: tag at +0x10, int64 cents at +0x18. ---- */
typedef struct {
    int first, last, period, months, broken;
    long long sum; /* of the inner map being walked */
    int income;    /* which inner map is being walked */
    re_household *h;
    re_cashflow_visit visit;
    void *visit_ctx;
} cf_ctx;

static void add_tag(re_household *h, int tag, long long expense, long long income)
{
    int i;
    for (i = 0; i < h->cf_tag_count && h->cf_tags[i].tag != tag; i++)
        ;
    if (i == h->cf_tag_count) {
        if (i == RE_MAX_TAGS) /* more distinct tags than room: lump the rest together */
            i = RE_MAX_TAGS - 1, tag = -1;
        else
            h->cf_tag_count++;
        h->cf_tags[i].tag = tag;
    }
    h->cf_tags[i].expense += expense;
    h->cf_tags[i].income += income;
}

static int visit_cf_tag(const BYTE *n, void *vctx)
{
    cf_ctx *c = (cf_ctx *)vctx;
    int tag = *(const int *)(n + 0x10);
    long long cents = *(const long long *)(n + 0x18);
    c->sum += cents;
    if (c->h)
        add_tag(c->h, tag, c->income ? 0 : cents, c->income ? cents : 0);
    if (c->visit)
        c->visit(c->period, tag, c->income ? 0 : cents, c->income ? cents : 0, c->visit_ctx);
    return 1;
}

static int visit_cf_period(const BYTE *n, void *vctx)
{
    cf_ctx *c = (cf_ctx *)vctx;
    c->period = *(const int *)(n + 0x10);
    if (c->period < c->first || c->period > c->last)
        return 1;
    long long expense_total = *(const long long *)(n + 0x20), income_total = *(const long long *)(n + 0x30);
    c->income = 0;
    c->sum = 0;
    if (map_walk(n + 0x18, 0x20, visit_cf_tag, c) < 0 || c->sum != expense_total)
        c->broken = 1;
    c->income = 1;
    c->sum = 0;
    if (map_walk(n + 0x28, 0x20, visit_cf_tag, c) < 0 || c->sum != income_total)
        c->broken = 1;
    c->months++;
    if (c->h) {
        c->h->cf_expense_total += expense_total;
        c->h->cf_income_total += income_total;
    }
    return 1;
}

static BYTE *pfm_of(void *main_obj)
{
    if (!re_readable(main_obj, 0x400))
        return NULL;
    BYTE *pfm = *(BYTE **)((BYTE *)main_obj + RE_MAIN_PFM);
    return re_readable(pfm, 0x380) ? pfm : NULL;
}

static int statement_walk(void *main_obj, unsigned statement, int first, int last, re_cashflow_visit visit, void *ctx)
{
    BYTE *pfm = pfm_of(main_obj);
    cf_ctx c = {.first = first, .last = last, .visit = visit, .visit_ctx = ctx};
    if (pfm == NULL || map_walk(pfm + statement + 0x20, 0x38, visit_cf_period, &c) < 0 || c.broken)
        return -1;
    return c.months;
}

int re_game_cashflow_walk(void *main_obj, int first, int last, re_cashflow_visit visit, void *ctx)
{
    return statement_walk(main_obj, RE_PFM_CASHFLOW, first, last, visit, ctx);
}

int re_game_taxable_walk(void *main_obj, int first, int last, re_cashflow_visit visit, void *ctx)
{
    return statement_walk(main_obj, RE_PFM_TAXABLE, first, last, visit, ctx);
}

typedef struct {
    int found;
    float rate;
} tax_ctx;

static int visit_tax_rate(const BYTE *n, void *vctx)
{
    tax_ctx *c = (tax_ctx *)vctx;
    if (*(const int *)(n + 0x10) == RE_TAG_TAX_RATE) {
        c->found = 1;
        c->rate = *(const float *)(n + 0x14);
    }
    return 1;
}

int re_game_tax_terms(void *main_obj, long long *tax_free, float *rate)
{
    BYTE *pfm = pfm_of(main_obj);
    tax_ctx c = {0, 0.0f};
    if (pfm == NULL)
        return 0;
    BYTE *policy = *(BYTE **)(pfm + RE_PFM_MONEY_FORMAT);
    if (!re_readable(policy, RE_POLICY_VALUES + 8) || map_walk(policy + RE_POLICY_VALUES, 0x18, visit_tax_rate, &c) < 0 || !c.found)
        return 0;
    *tax_free = *(long long *)(pfm + RE_PFM_TAX_FREE);
    *rate = c.rate;
    return 1;
}

typedef struct {
    int id;
    BYTE *company;
} company_ctx;

static int visit_company(const BYTE *n, void *vctx)
{
    company_ctx *c = (company_ctx *)vctx;
    if (*(const int *)(n + 0x10) == c->id)
        c->company = (BYTE *)n + RE_MARKET_NODE_COMPANY;
    return 1;
}

BYTE *re_game_listed_company(void *market, int id)
{
    company_ctx c = {id, NULL};
    if (!re_readable(market, RE_MARKET_COMPANIES + 8) ||
        map_walk((const BYTE *)market + RE_MARKET_COMPANIES, RE_MARKET_NODE_COMPANY + RE_COMPANY_BOARD_NOTICE + 1, visit_company, &c) < 0)
        return NULL;
    return c.company;
}

typedef struct {
    BYTE **out;
    int cap, count;
} companies_ctx;

static int visit_listed(const BYTE *n, void *vctx)
{
    companies_ctx *c = (companies_ctx *)vctx;
    if (c->count < c->cap)
        c->out[c->count] = (BYTE *)n + RE_MARKET_NODE_COMPANY;
    c->count++;
    return 1;
}

int re_game_companies(void *market, BYTE **out, int cap)
{
    companies_ctx c = {out, cap, 0};
    if (!re_readable(market, RE_MARKET_COMPANIES + 8) ||
        map_walk((const BYTE *)market + RE_MARKET_COMPANIES, RE_MARKET_NODE_COMPANY + RE_COMPANY_BOARD_NOTICE + 1, visit_listed, &c) < 0)
        return -1;
    return c.count < cap ? c.count : cap;
}

void *re_game_finance(void *main_obj)
{
    return pfm_of(main_obj);
}

void *re_game_people(void *main_obj)
{
    if (!re_readable(main_obj, 0x400))
        return NULL;
    BYTE *people = *(BYTE **)((BYTE *)main_obj + RE_MAIN_PEOPLE);
    return re_readable(people, RE_PEOPLE_COOLDOWNS + 8) ? people : NULL;
}

typedef struct {
    int id, month;
} cooldown_ctx;

static int visit_cooldown(const BYTE *n, void *vctx)
{
    cooldown_ctx *c = (cooldown_ctx *)vctx;
    if (*(const int *)(n + 0x10) == c->id)
        c->month = *(const int *)(n + 0x14);
    return 1;
}

int re_game_cooldown(void *people, int activity)
{
    cooldown_ctx c = {activity, RE_COOLDOWN_NONE};
    if (!re_readable(people, RE_PEOPLE_COOLDOWNS + 8) || map_walk((const BYTE *)people + RE_PEOPLE_COOLDOWNS, 0x18, visit_cooldown, &c) < 0)
        return RE_COOLDOWN_UNREADABLE;
    return c.month;
}

int re_game_set_cooldown(void *people, int activity, int month)
{
    typedef int *(__thiscall *map_at_fn)(void *map, const int *key);
    if (!re_readable(people, RE_PEOPLE_COOLDOWNS + 8))
        return 0;
    *((map_at_fn)AT(RE_VA_MAP_AT))((BYTE *)people + RE_PEOPLE_COOLDOWNS, &activity) = month;
    return 1;
}

int re_game_queue_take_out(void *people, int (*wanted)(int activity, int gone), re_queued *out, int cap)
{
    typedef struct {
        void **functions; /* [1] lets the object go, [2] the count itself */
        volatile LONG use, weak;
    } counted;
    typedef struct {
        BYTE *person;
        counted *count;
    } shared;
    typedef struct {
        BYTE bytes[RE_QUEUE_NODE_BYTES - 8];
    } record_copy;
    typedef shared *(__thiscall * person_fn)(void *people, shared * out, int id, char report);
    typedef void(__thiscall * gone_fn)(void *self);
    typedef void(__cdecl * delete_fn)(void *block, unsigned size);
    typedef void(__thiscall * copy_fn)(record_copy * to, const void *record);
    typedef void(__thiscall * told_fn)(void *windows, int person, record_copy record);
    BYTE *p = (BYTE *)people;
    if (!re_readable(p, RE_PEOPLE_COOLDOWNS + 8))
        return -1;
    const int *begin = *(const int *const *)(p + RE_PEOPLE_HOUSEHOLD), *end = *(const int *const *)(p + RE_PEOPLE_HOUSEHOLD + 4);
    if (end < begin || end - begin > 64 || (end != begin && !re_readable(begin, (SIZE_T)(end - begin) * sizeof *begin)))
        return -1;
    int taken = 0;
    for (const int *id = begin; id != end; id++) {
        shared who = {NULL, NULL};
        ((person_fn)AT(RE_VA_PERSON_OF))(people, &who, *id, 0);
        if (re_readable(who.person, RE_PERSON_QUEUE + 8)) { /* the windows and the id are in front of the queue */
            BYTE **head = *(BYTE ***)(who.person + RE_PERSON_QUEUE);
            for (BYTE **node = re_readable(head, 8) ? (BYTE **)head[0] : head, **next; node != head; node = next) {
                if (!re_readable(node, RE_QUEUE_NODE_BYTES))
                    break;
                next = (BYTE **)node[0];
                const BYTE *record = (const BYTE *)(node + 2);
                int activity = *(const int *)(record + RE_RECORD_ACTIVITY), gone = *(const int *)(record + RE_RECORD_GONE);
                if (!wanted(activity, gone))
                    continue;
                if (taken < cap) {
                    out[taken].activity = activity;
                    out[taken].hours = *(const int *)(record + RE_RECORD_HOURS);
                    out[taken].gone = gone;
                }
                taken++;
                record_copy copy; /* the game's own steps for a right click on the record, in its order */
                ((copy_fn)AT(RE_VA_RECORD_COPY))(&copy, record);
                ((BYTE ***)node[1])[0] = (BYTE **)node[0];
                ((BYTE ***)node[0])[1] = (BYTE **)node[1];
                --*(int *)(who.person + RE_PERSON_QUEUE + 4);
                for (int s = 3; s >= 0; s--)
                    ((gone_fn)AT(RE_VA_STR_FREE))((BYTE *)node + RE_QUEUE_NODE_STRING + s * 0x18);
                ((delete_fn)AT(RE_VA_DELETE))(node, RE_QUEUE_NODE_BYTES);
                BYTE *windows = *(BYTE **)(who.person + RE_PERSON_WINDOWS), *self = *(BYTE **)(who.person + RE_PERSON_SELF);
                if (re_readable(windows, 4) && re_readable(self, RE_PERSON_SELF_ID + 4))
                    ((told_fn)AT(RE_VA_QUEUE_GONE))(windows, *(const int *)(self + RE_PERSON_SELF_ID), copy); /* releases the copy */
                else
                    for (int s = 3; s >= 0; s--)
                        ((gone_fn)AT(RE_VA_STR_FREE))(copy.bytes + RE_QUEUE_NODE_STRING - 8 + s * 0x18);
            }
        }
        if (who.count != NULL && InterlockedDecrement(&who.count->use) == 0) { /* the pointer handed out is given back */
            ((gone_fn)who.count->functions[1])(who.count);
            if (InterlockedDecrement(&who.count->weak) == 0)
                ((gone_fn)who.count->functions[2])(who.count);
        }
    }
    return taken;
}

long long re_game_activity_money(void *prices, void *object)
{
    /* two std::string by value, short enough to sit in the string itself; the callee destroys them */
    typedef struct {
        char buf[16];
        unsigned size, capacity;
    } inline_string;
    typedef long long(__thiscall *activity_money_fn)(void *prices, void *object, inline_string section, inline_string name);
    inline_string section = {"action", 6, 15}, name = {"money", 5, 15};
    return ((activity_money_fn)AT(RE_VA_ACTIVITY_MONEY))(prices, object, section, name);
}

void *re_game_market(void *main_obj)
{
    BYTE *pfm = pfm_of(main_obj);
    BYTE *market = pfm != NULL ? *(BYTE **)(pfm + 0x18) : NULL; /* the object RE_VA_STOCKS takes */
    return re_readable(market, RE_MARKET_HOLDINGS + 8) ? market : NULL;
}

typedef struct {
    re_holding *out;
    int cap, count;
} holdings_ctx;

static int visit_holding(const BYTE *n, void *vctx)
{
    holdings_ctx *c = (holdings_ctx *)vctx;
    if (c->count < c->cap) {
        c->out[c->count].company = *(const int *)(n + 0x10);
        c->out[c->count].shares = *(const int *)(n + 0x14);
    }
    c->count++;
    return 1;
}

int re_game_holdings(void *market, re_holding *out, int cap)
{
    holdings_ctx c = {out, cap, 0};
    if (!re_readable(market, RE_MARKET_HOLDINGS + 8) || map_walk((const BYTE *)market + RE_MARKET_HOLDINGS, 0x18, visit_holding, &c) < 0)
        return -1;
    return c.count < cap ? c.count : cap;
}

/* the text of an MSVC std::string at `s`; 0 when it is empty, does not fit or cannot be read */
static int string_text(const BYTE *s, char *out, unsigned cap)
{
    if (!re_readable(s, 0x18))
        return 0;
    unsigned size = *(const unsigned *)(s + 0x10), room = *(const unsigned *)(s + 0x14);
    const char *text = room >= 16 ? *(const char *const *)s : (const char *)s;
    if (size == 0 || size >= cap || !re_readable(text, size))
        return 0;
    memcpy(out, text, size);
    out[size] = 0;
    return 1;
}

int re_game_company_name(const BYTE *company, char *out, unsigned cap)
{
    return string_text(company + RE_COMPANY_NAME, out, cap);
}

static int visit_job_assets(const BYTE *n, void *vctx)
{
    float *least = (float *)vctx, now = *(const float *)(n + 0x14);
    if (now < *least)
        *least = now;
    return 1;
}

static int visit_job_hours(const BYTE *n, void *vctx)
{
    re_firm_fit *f = (re_firm_fit *)vctx;
    int given = *(const int *)(n + RE_JOB_NODE_ALLOWED) + *(const int *)(n + RE_JOB_NODE_OVERTIME);
    int left = given - *(const int *)(n + RE_JOB_NODE_WORKED);
    f->work += given;
    f->undone += left > 0 ? left : 0;
    return 1;
}

typedef struct {
    re_firm_fit *out;
    int cap, count;
} fits_ctx;

static int visit_own_firm(const BYTE *n, void *vctx)
{
    fits_ctx *c = (fits_ctx *)vctx;
    const BYTE *firm = n + RE_FIRMS_NODE_FIRM;
    if (*(const int *)(firm + RE_FIRM_TYPE) == 0 || *(const int *)(firm + RE_FIRM_PREMISES) <= 0 || c->count >= c->cap)
        return 1; /* not set up: the game's own hourly work skips it too */
    re_firm_fit *f = &c->out[c->count];
    const BYTE *advert = *(const BYTE *const *)(firm + RE_FIRM_ADVERTS), *end = *(const BYTE *const *)(firm + RE_FIRM_ADVERTS + 4);
    f->id = *(const int *)(n + NODE_HEADER);
    f->furnishings = *(const float *)(firm + RE_FIRM_FURNISH);
    f->assets = 1.0f;
    f->work = f->undone = 0;
    f->adverts = end > advert && (SIZE_T)(end - advert) % RE_ADVERT_BYTES == 0 ? (int)((SIZE_T)(end - advert) / RE_ADVERT_BYTES) : 0;
    f->awareness = *(const float *)(firm + RE_FIRM_AWARENESS);
    f->open = re_game_closed_unseen || *(const int *)(firm + RE_FIRM_OPEN) == 1;
    if (!string_text(firm + RE_FIRM_NAME, f->name, sizeof f->name) || map_walk(firm + RE_FIRM_ASSETS, 0x18, visit_job_assets, &f->assets) < 0)
        return 1;
    if (map_walk(firm + RE_FIRM_JOBS, RE_JOB_NODE_OVERTIME + 4, visit_job_hours, f) < 0)
        f->work = f->undone = -1; /* not understood: nothing is concluded from it */
    c->count++;
    return 1;
}

int re_game_firm_fits(void *main_obj, re_firm_fit *out, int cap)
{
    BYTE *firms = re_game_firms(main_obj);
    fits_ctx c = {out, cap, 0};
    if (firms == NULL || !re_readable(firms, RE_FIRMS_OWN + 8) ||
        map_walk(firms + RE_FIRMS_OWN, RE_FIRMS_NODE_FIRM + RE_FIRM_BYTES, visit_own_firm, &c) < 0)
        return -1;
    return c.count;
}

typedef struct {
    void *restate;
    re_listing *out;
    int cap, count;
} listings_ctx;

static int visit_for_sale(const BYTE *n, void *vctx)
{
    typedef struct {
        char text[16];
        unsigned size, capacity;
    } inline_string;
    typedef long long(__thiscall * house_price_fn)(void *restate, const BYTE *record, inline_string kind, int draw);
    listings_ctx *c = (listings_ctx *)vctx;
    const BYTE *record = n + RE_FOR_SALE_NODE_RECORD;
    long long price = *(const long long *)(record + RE_HOUSE_ASKING);
    if (*(const int *)(record + RE_HOUSE_STATE) != RE_HOUSE_FOR_SALE || price <= 0 || c->count >= c->cap)
        return 1;
    re_listing *l = &c->out[c->count];
    if (!string_text(record + RE_HOUSE_ADDRESS, l->address, sizeof l->address))
        return 1;
    inline_string kind = {"buyK", 4, 15};
    l->price = price;
    l->value = ((house_price_fn)AT(RE_VA_HOUSE_PRICE))(c->restate, record, kind, 0); /* 0: no random draw, nothing written */
    c->count += l->value > 0;
    return 1;
}

int re_game_listings(void *main_obj, re_listing *out, int cap)
{
    if (!re_readable(main_obj, 0x400))
        return -1;
    BYTE *restate = *(BYTE **)((BYTE *)main_obj + RE_MAIN_RESTATE);
    listings_ctx c = {restate, out, cap, 0};
    if (!re_readable(restate, RE_RESTATE_FOR_SALE + 8) ||
        map_walk(restate + RE_RESTATE_FOR_SALE, RE_FOR_SALE_NODE_RECORD + RE_HOUSE_BYTES, visit_for_sale, &c) < 0)
        return -1;
    return c.count;
}

typedef struct {
    int id, found;
    float rate;
} industry_ctx;

static int visit_rate(const BYTE *n, void *vctx)
{
    industry_ctx *c = (industry_ctx *)vctx;
    if (*(const int *)(n + 0x10) == c->id) {
        c->rate = *(const float *)(n + 0x14);
        c->found = 1;
    }
    return 1;
}

int re_game_company_industries(void *market, const BYTE *company, re_industry *out, int cap)
{
    if (!re_readable(market, RE_MARKET_ECONOMY + 4) || !re_readable(company, RE_COMPANY_INDUSTRIES + 8))
        return -1;
    const BYTE *economy = *(const BYTE *const *)((const BYTE *)market + RE_MARKET_ECONOMY);
    const BYTE *first = *(const BYTE *const *)(company + RE_COMPANY_INDUSTRIES);
    const BYTE *last = *(const BYTE *const *)(company + RE_COMPANY_INDUSTRIES + 4);
    SIZE_T bytes = (SIZE_T)(last - first);
    if (!re_readable(economy, RE_ECON_RATES + 8) || last < first || bytes % RE_INDUSTRY_BYTES != 0 || bytes > 64 * RE_INDUSTRY_BYTES ||
        (bytes != 0 && !re_readable(first, bytes)))
        return -1;
    int count = 0;
    for (const BYTE *e = first; e != last && count < cap; e += RE_INDUSTRY_BYTES) {
        industry_ctx c = {*(const int *)(e + RE_INDUSTRY_ID), 0, 0.0f};
        long long weight = *(const long long *)e;
        if (weight == 0) /* the game leaves these out too */
            continue;
        if (map_walk(economy + RE_ECON_RATES, 0x18, visit_rate, &c) < 0)
            return -1;
        out[count].id = c.id;
        out[count].weight = (double)weight;
        out[count].rate = c.found ? c.rate : 0.0; /* an industry the city has no rate for counts as 0, as in the game */
        count++;
    }
    return count;
}

typedef struct {
    re_fut_economy *e;
    int what; /* 0 rates, 1 index, 2 weights, 3 policy */
    int ok;
} economy_ctx;

static int visit_economy(const BYTE *n, void *vctx)
{
    economy_ctx *c = (economy_ctx *)vctx;
    int id = *(const int *)(n + 0x10);
    double value = *(const float *)(n + 0x14);
    if (id == RE_FUT_OVERALL) {
        if (c->what == 0)
            c->e->overall_rate = value;
        else if (c->what == 1)
            c->e->overall_index = value;
        return 1;
    }
    for (int i = 0; i < c->e->count; i++)
        if (c->e->industry[i].id == id) {
            double *slot = c->what == 1 ? &c->e->industry[i].index : c->what == 2 ? &c->e->industry[i].weight : &c->e->industry[i].policy;
            if (c->what != 0)
                *slot = value;
            return 1;
        }
    if (c->what != 0) /* the game walks the rates; an entry of another map without a rate takes no part */
        return 1;
    if (c->e->count >= RE_FUT_INDUSTRIES) {
        c->ok = 0;
        return 1;
    }
    c->e->industry[c->e->count++] = (re_fut_industry){id, value, 0.0, 0.0, 0.0};
    return 1;
}

void *re_game_economy_object(void *main_obj)
{
    BYTE *market = (BYTE *)re_game_market(main_obj);
    BYTE *economy = market != NULL ? *(BYTE **)(market + RE_MARKET_ECONOMY) : NULL;
    return re_readable(economy, RE_ECON_RATES + 8) ? economy : NULL;
}

/* The game's function for the yearly price change of a ware (RE_VA_WARE_RATE): ecx = economy, the ware on the
 * stack (removed by the callee), a factor in xmm2, the rate comes back in xmm0. Not a convention C can call. */
unsigned re_game_ware_rate_raw(void *economy, int ware, void *rate_fn, float factor);
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_game_ware_rate_raw\n"
        "_re_game_ware_rate_raw:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  mov ecx, [ebp + 8]\n"
        "  movss xmm2, dword ptr [ebp + 20]\n"
        "  push dword ptr [ebp + 12]\n"
        "  call dword ptr [ebp + 16]\n"
        "  movd eax, xmm0\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

double re_game_ware_rate(void *economy, int ware)
{
    union {
        unsigned bits;
        float rate;
    } got = {re_game_ware_rate_raw(economy, ware, AT(RE_VA_WARE_RATE), 1.0f)};
    return got.rate;
}

/* The game's monthly decay of a person's experience (RE_VA_XP_DECAY): ecx = the statement, the factor in xmm1,
 * nothing on the stack. */
void re_game_xp_decay_raw(void *statement, void *decay_fn, float factor);
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_game_xp_decay_raw\n"
        "_re_game_xp_decay_raw:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  mov ecx, [ebp + 8]\n"
        "  movss xmm1, dword ptr [ebp + 16]\n"
        "  call dword ptr [ebp + 12]\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

void re_game_xp_decay(void *statement, float factor)
{
    re_game_xp_decay_raw(statement, AT(RE_VA_XP_DECAY), factor);
}

int re_game_economy(const void *economy, re_fut_economy *out)
{
    const BYTE *econ = (const BYTE *)economy;
    economy_ctx c = {out, 0, 1};
    memset(c.e, 0, sizeof *c.e);
    if (!re_readable(econ, RE_ECON_RATES + 8))
        return 0;
    const BYTE *custom = *(const BYTE *const *)(econ + RE_ECON_CUSTOM);
    if (!re_readable(custom, RE_CUSTOM_POLICY + 8))
        return 0;
    const BYTE *maps[4] = {econ + RE_ECON_RATES, econ + RE_ECON_INDEX, custom + RE_CUSTOM_WEIGHTS, custom + RE_CUSTOM_POLICY};
    for (c.what = 0; c.what < 4; c.what++)
        if (map_walk(maps[c.what], 0x18, visit_economy, &c) < 0)
            return 0;
    double weights = 0.0;
    for (int i = 0; i < c.e->count; i++)
        weights += c.e->industry[i].weight;
    for (int i = 0; i < c.e->count && weights > 0.0; i++)
        c.e->industry[i].weight /= weights;
    c.e->growth = *(const float *)(econ + RE_ECON_GROWTH);
    return c.ok && c.e->count > 0 && c.e->overall_index > 0.0;
}

void *re_game_firms(void *main_obj)
{
    if (!re_readable(main_obj, 0x400))
        return NULL;
    BYTE *firms = *(BYTE **)((BYTE *)main_obj + RE_MAIN_FIRMS);
    return re_readable(firms, RE_FIRMS_JOBS + 4) ? firms : NULL;
}

void *re_game_jobs(void *firms)
{
    if (!re_readable(firms, RE_FIRMS_JOBS + 4))
        return NULL;
    BYTE *jobs = *(BYTE **)((BYTE *)firms + RE_FIRMS_JOBS);
    return re_readable(jobs, 4) ? jobs : NULL;
}

long long re_game_staff_cost(void *jobs, const BYTE *staff, double *efficiency)
{
    if (jobs == NULL || !re_readable(staff, RE_STAFF_BYTES))
        return -1;
    double e = re_business_efficiency(AT(RE_VA_STAFF_EFFICIENCY), jobs, *(const int *)(staff + RE_STAFF_JOB), staff + RE_STAFF_SKILL);
    if (efficiency != NULL)
        *efficiency = e;
    return re_business_work_cost(*(const long long *)(staff + RE_STAFF_WAGE), *(const int *)(staff + RE_STAFF_HOURS), e);
}

void re_game_staff_reorder(BYTE *first, int count, const int *order)
{
    typedef void *(__thiscall *copy_fn)(void *self, const void *from);
    typedef void(__thiscall *end_fn)(void *self);
    static BYTE spare[RE_STAFF_SORT_MAX][RE_STAFF_BYTES] __attribute__((aligned(16)));
    copy_fn copy = (copy_fn)AT(RE_VA_STAFF_COPY);
    end_fn end_skill = (end_fn)AT(RE_VA_STAFF_END_SKILL), end = (end_fn)AT(RE_VA_STAFF_END);
    if (count > RE_STAFF_SORT_MAX)
        return;
    for (int i = 0; i < count; i++)
        copy(spare[i], first + (SIZE_T)order[i] * RE_STAFF_BYTES);
    for (int i = 0; i < count; i++) { /* as the game ends a record: the skill part, then the rest */
        BYTE *slot = first + (SIZE_T)i * RE_STAFF_BYTES;
        end_skill(slot + RE_STAFF_SKILL);
        end(slot);
        copy(slot, spare[i]);
        end_skill(spare[i] + RE_STAFF_SKILL);
        end(spare[i]);
    }
}

/* ---- an offered contract against the standard cost of its work (note b18 "Q5") ---- */
#define OFFER_JOBS 16      /* the game's data gives a contract at most five jobs, and a job sets off at most two */
#define OFFER_HOURS 100000 /* of one job in a month */
#define OFFER_MONTHS 1200

/* the node of a map that has this key: an id, or a name when `name` is set */
typedef struct {
    int id;
    const char *name;
    const BYTE *node;
} key_ctx;

static int visit_key(const BYTE *n, void *vctx)
{
    key_ctx *c = (key_ctx *)vctx;
    char name[32];
    if (c->name != NULL ? string_text(n + NODE_HEADER, name, sizeof name) && strcmp(name, c->name) == 0
                        : *(const int *)(n + NODE_HEADER) == c->id)
        c->node = n;
    return 1;
}

int re_game_firm_staff(void *firms, int firm, BYTE **first)
{
    if (!re_readable(firms, RE_FIRMS_STAFF + 4))
        return -1;
    const BYTE *staff = *(BYTE *const *)((const BYTE *)firms + RE_FIRMS_STAFF);
    key_ctx c = {firm, NULL, NULL};
    if (!re_readable(staff, RE_STAFF_BY_FIRM + 8) || map_walk(staff + RE_STAFF_BY_FIRM, RE_STAFF_BY_FIRM_LIST + 12, visit_key, &c) < 0)
        return -1;
    if (c.node == NULL)
        return 0;
    BYTE *const *list = (BYTE *const *)(c.node + RE_STAFF_BY_FIRM_LIST);
    SIZE_T bytes = (SIZE_T)(list[1] - list[0]);
    if (list[1] < list[0] || bytes % RE_STAFF_BYTES != 0 || bytes > 1000 * RE_STAFF_BYTES || (bytes != 0 && !re_readable(list[0], bytes)))
        return -1;
    *first = list[0];
    return (int)(bytes / RE_STAFF_BYTES);
}

int re_game_staff_auto(void *firms, int firm, int staff)
{
    typedef int(__thiscall *auto_fn)(void *firms, int firm, int staff);
    return (((auto_fn)AT(RE_VA_STAFF_AUTO))(firms, firm, staff) & 0xff) != 0; /* only al is the answer */
}

int re_game_firm_managed(void *firms, int firm, int *employees)
{
    BYTE *first = NULL;
    int count = re_game_firm_staff(firms, firm, &first), managed = 0;
    if (count < 0)
        return -1;
    for (int i = 0; i < count; i++)
        managed += re_game_staff_auto(firms, firm, *(const int *)(first + (SIZE_T)i * RE_STAFF_BYTES + RE_STAFF_ID));
    *employees = count;
    return managed;
}

int re_game_firm_unmanage(void *firms, int firm)
{
    typedef void(__thiscall *auto_set_fn)(void *firms, int firm, int staff, int on);
    BYTE *first = NULL;
    int count = re_game_firm_staff(firms, firm, &first), off = 0;
    if (count < 0)
        return -1;
    for (int i = 0; i < count; i++) {
        int id = *(const int *)(first + (SIZE_T)i * RE_STAFF_BYTES + RE_STAFF_ID);
        if (!re_game_staff_auto(firms, firm, id))
            continue;
        ((auto_set_fn)AT(RE_VA_STAFF_AUTO_SET))(firms, firm, id, 0);
        off++;
    }
    return off;
}

void re_game_staff_manage(void *firms, int firm, int staff)
{
    typedef void(__thiscall *auto_set_fn)(void *firms, int firm, int staff, int on);
    ((auto_set_fn)AT(RE_VA_STAFF_AUTO_SET))(firms, firm, staff, 1);
}

BYTE *re_game_employee(void *firms, int firm, int staff)
{
    BYTE *first = NULL;
    int count = re_game_firm_staff(firms, firm, &first);
    for (int i = 0; i < count; i++)
        if (*(const int *)(first + (SIZE_T)i * RE_STAFF_BYTES + RE_STAFF_ID) == staff)
            return first + (SIZE_T)i * RE_STAFF_BYTES;
    return NULL;
}

typedef struct {
    int *keys;
    long long *amounts;
    int cap, count;
} costs_ctx;

static int visit_advert_cost(const BYTE *n, void *vctx)
{
    costs_ctx *c = (costs_ctx *)vctx;
    if (c->count < c->cap) {
        c->keys[c->count] = *(const int *)(n + NODE_HEADER);
        c->amounts[c->count++] = *(const long long *)(n + RE_ADVERT_COST_VALUE);
    }
    return 1;
}

int re_game_firm_advert_costs(void *firms, int firm, int *keys, long long *amounts, int cap)
{
    key_ctx c = {firm, NULL, NULL};
    costs_ctx costs = {keys, amounts, cap, 0};
    if (!re_readable(firms, RE_FIRMS_OWN + 8) ||
        map_walk((const BYTE *)firms + RE_FIRMS_OWN, RE_FIRMS_NODE_FIRM + RE_FIRM_BYTES, visit_key, &c) < 0 || c.node == NULL ||
        map_walk(c.node + RE_FIRMS_NODE_FIRM + RE_FIRM_ADVERT_COSTS, RE_ADVERT_COST_VALUE + 8, visit_advert_cost, &costs) < 0)
        return -1;
    return costs.count;
}

typedef struct {
    int *types;
    int cap, count, records;
} assets_ctx;

static int visit_asset_type(const BYTE *n, void *vctx)
{
    assets_ctx *c = (assets_ctx *)vctx;
    int type = *(const int *)(n + RE_ASSET_NODE_TYPE), known = 0;
    for (int i = 0; i < c->count; i++)
        known |= c->types[i] == type;
    if (!known && c->count < c->cap)
        c->types[c->count++] = type;
    c->records++;
    return 1;
}

static const BYTE *own_firm(void *firms, int firm)
{
    key_ctx c = {firm, NULL, NULL};
    if (!re_readable(firms, RE_FIRMS_OWN + 8) ||
        map_walk((const BYTE *)firms + RE_FIRMS_OWN, RE_FIRMS_NODE_FIRM + RE_FIRM_BYTES, visit_key, &c) < 0 || c.node == NULL)
        return NULL;
    return c.node + RE_FIRMS_NODE_FIRM;
}

typedef struct {
    re_job_hours *out;
    int cap, count;
} job_hours_ctx;

static int visit_job(const BYTE *n, void *vctx)
{
    job_hours_ctx *c = (job_hours_ctx *)vctx;
    if (c->count < c->cap) {
        re_job_hours *j = &c->out[c->count];
        j->job = *(const int *)(n + RE_JOB_NODE_KIND);
        j->worked = *(const int *)(n + RE_JOB_NODE_WORKED);
        j->allowed = *(const int *)(n + RE_JOB_NODE_ALLOWED);
        j->overtime = *(const int *)(n + RE_JOB_NODE_OVERTIME);
        j->outsourced = *(const int *)(n + RE_JOB_NODE_OUTSOURCED) > 0;
    }
    c->count++;
    return 1;
}

int re_game_firm_jobs(void *firms, int firm, re_job_hours *out, int cap)
{
    const BYTE *stack = own_firm(firms, firm);
    job_hours_ctx c = {out, cap, 0};
    if (stack == NULL || map_walk(stack + RE_FIRM_JOBS, RE_JOB_NODE_OVERTIME + 4, visit_job, &c) < 0)
        return -1;
    return c.count < cap ? c.count : cap;
}

int re_game_firm_of(void *firms, const void *firm_stack)
{
    if (!re_readable(firm_stack, RE_FIRM_ID + 4))
        return 0;
    int id = *(const int *)((const BYTE *)firm_stack + RE_FIRM_ID);
    return own_firm(firms, id) == (const BYTE *)firm_stack ? id : 0;
}

int re_game_firm_type(void *firms, int firm)
{
    const BYTE *stack = own_firm(firms, firm);
    return stack != NULL ? *(const int *)(stack + RE_FIRM_TYPE) : 0;
}

int re_game_firm_factors(void *firms, int firm, int job, double *assets, double *furnishings)
{
    const BYTE *stack = own_firm(firms, firm);
    key_ctx c = {job, NULL, NULL};
    if (stack == NULL || map_walk(stack + RE_FIRM_ASSETS, 0x18, visit_key, &c) < 0)
        return 0;
    *assets = c.node != NULL ? *(const float *)(c.node + 0x14) : 1.0;
    *furnishings = *(const float *)(stack + RE_FIRM_FURNISH);
    return 1;
}

void re_game_firm_refit(void *firms, int firm)
{
    typedef void(__fastcall *refit_fn)(void *business);
    const BYTE *stack = own_firm(firms, firm);
    if (stack != NULL)
        ((refit_fn)AT(RE_VA_FIRM_REFIT))((void *)(UINT_PTR)stack);
}

double re_game_firm_awareness(void *firms, int firm)
{
    const BYTE *stack = own_firm(firms, firm);
    return stack != NULL ? *(const float *)(stack + RE_FIRM_AWARENESS) : -1.0;
}

int re_game_advert_on(void *firms, int firm, int advert)
{
    typedef int(__thiscall *on_fn)(void *firms, int firm, int advert);
    return ((on_fn)AT(RE_VA_ADVERT_ON))(firms, firm, advert) != 0;
}

long long re_game_advert_price(void *firms, int firm, int advert)
{
    typedef long long(__thiscall *price_fn)(void *firms, int firm, int advert);
    return ((price_fn)AT(RE_VA_ADVERT_PRICE))(firms, firm, advert);
}

void re_game_advert_switch(void *firms, int firm, int advert, int on)
{
    typedef void(__thiscall *switch_fn)(void *firms, int firm, int advert, int mode);
    ((switch_fn)AT(RE_VA_ADVERT_SWITCH))(firms, firm, advert, on != 0);
}

int re_game_hire(void *firms, int firm, int job, int candidate)
{
    typedef void(__thiscall *hire_fn)(void *firms, int firm, int job, int candidate);
    ((hire_fn)AT(RE_VA_HIRE))(firms, firm, job, candidate);
    return re_game_employee(firms, firm, candidate) != NULL;
}

int re_game_firm_auto_assets_off(void *firms, int firm)
{
    BYTE *stack = (BYTE *)(UINT_PTR)own_firm(firms, firm);
    if (stack == NULL)
        return -1;
    int was = stack[RE_FIRM_AUTO_ASSET] != 0;
    stack[RE_FIRM_AUTO_ASSET] = 0; /* what a click on the switch writes when it turns it off (FUN_005d78d0) */
    return was;
}

int re_game_firm_auto_assets_on(void *firms, int firm)
{
    typedef void(__fastcall * supply_fn)(void *business);
    BYTE *stack = (BYTE *)(UINT_PTR)own_firm(firms, firm);
    if (stack == NULL)
        return -1;
    int was = stack[RE_FIRM_AUTO_ASSET] != 0;
    if (!was) { /* the two steps of the click that turns it on */
        stack[RE_FIRM_AUTO_ASSET] = 1;
        ((supply_fn)AT(RE_VA_FIRM_SUPPLY))(stack);
    }
    return was;
}

int re_game_firm_manage(void *firms, int firm)
{
    BYTE *first = NULL;
    int count = re_game_firm_staff(firms, firm, &first), on = 0;
    if (count < 0)
        return -1;
    for (int i = 0; i < count; i++) {
        int id = *(const int *)(first + (SIZE_T)i * RE_STAFF_BYTES + RE_STAFF_ID);
        if (re_game_staff_auto(firms, firm, id))
            continue;
        re_game_staff_manage(firms, firm, id);
        on++;
    }
    return on;
}

int re_game_firm_asset_types(void *firms, int firm, int *types, int cap, int *records, int *auto_buy, int *hours_taken)
{
    const BYTE *stack = own_firm(firms, firm);
    assets_ctx c = {types, cap, 0, 0};
    if (stack == NULL || map_walk(stack + RE_FIRM_STOCKPILE, RE_ASSET_NODE_BYTES, visit_asset_type, &c) < 0)
        return -1;
    *records = c.records;
    *auto_buy = stack[RE_FIRM_AUTO_ASSET] != 0;
    *hours_taken = stack[RE_FIRM_SUPPLY_HOURS] != 0;
    return c.count;
}

typedef struct {
    int type, units;
} units_ctx;

static int visit_asset_unit(const BYTE *n, void *vctx)
{
    units_ctx *c = (units_ctx *)vctx;
    c->units += *(const int *)(n + RE_ASSET_NODE_TYPE) == c->type;
    return 1;
}

int re_game_firm_asset_units(void *firms, int firm, int type)
{
    const BYTE *stack = own_firm(firms, firm);
    units_ctx c = {type, 0};
    if (stack == NULL || map_walk(stack + RE_FIRM_STOCKPILE, RE_ASSET_NODE_BYTES, visit_asset_unit, &c) < 0)
        return 0;
    return c.units;
}

/* ---- an asset a business is short of (note b23) ---- */

/* An element of a list of the game's data, and what names a ware or a tag to the data: {1, 0, 1.0, id, 1.0}. */
typedef struct {
    long long amount;
    float chance;
    int id;
    double one;
} list_element;
/* A std::string that holds its text itself (up to 15 letters): what the game's functions take by value. */
typedef struct {
    char text[16];
    unsigned size, capacity;
} short_string;

/* The game's info of a ware or of an asset tag, filled by the game; `info_done` gives its string back. */
static int info_of(const BYTE *stack, int id, int tag, BYTE *info)
{
    typedef void *(__thiscall * info_fn)(void *data, void *info, int complain, list_element key);
    void *data = *(void *const *)(stack + RE_FIRM_DATA);
    list_element key = {1, 1.0f, id, 1.0};
    if (!re_readable(data, RE_DATA_LISTS + 8))
        return 0;
    memset(info, 0, RE_INFO_BYTES + 4);
    ((info_fn)AT(tag ? RE_VA_TAG_INFO : RE_VA_WARE_INFO))(data, info, 0, key);
    return 1;
}

static void info_done(BYTE *info)
{
    typedef void(__thiscall * free_fn)(void *string);
    ((free_fn)AT(RE_VA_STR_FREE))(info + RE_INFO_STRING);
}

static double info_number(BYTE *info, const char *element, const char *attribute)
{
    return re_business_data_number(AT(RE_VA_INFO_NUMBER), info, AT(RE_VA_STR_ASSIGN), element, (unsigned)strlen(element), attribute,
                                   (unsigned)strlen(attribute));
}

static void info_name(BYTE *info, char *out, unsigned cap)
{
    typedef void *(__thiscall * name_fn)(void *info, void *string);
    typedef void(__thiscall * free_fn)(void *string);
    BYTE name[0x18] = {0};
    out[0] = 0;
    ((name_fn)AT(RE_VA_INFO_NAME))(info, name);
    if (!string_text(name, out, cap))
        out[0] = 0;
    ((free_fn)AT(RE_VA_STR_FREE))(name);
}

typedef struct {
    const BYTE *owned;
    re_asset_need *out;
    int cap, count;
} needs_ctx;

static int visit_need(const BYTE *n, void *vctx)
{
    needs_ctx *c = (needs_ctx *)vctx;
    key_ctx have = {*(const int *)(n + NODE_HEADER), NULL, NULL};
    int wanted = *(const int *)(n + NODE_HEADER + 4);
    if (map_walk(c->owned, 0x18, visit_key, &have) < 0)
        return 0;
    int owned = have.node != NULL ? *(const int *)(have.node + NODE_HEADER + 4) : 0;
    if (owned < wanted && c->count < c->cap) {
        re_asset_need *need = &c->out[c->count++];
        need->tag = have.id;
        need->wanted = wanted;
        need->owned = owned;
    }
    return 1;
}

int re_game_firm_needs(void *firms, int firm, re_asset_need *out, int cap)
{
    typedef void *(__thiscall * desired_fn)(void *firms, void *map, int firm);
    typedef void(__thiscall * erase_fn)(void *map, void *scratch, void *first, void *last);
    typedef void(__cdecl * delete_fn)(void *block, unsigned bytes);
    const BYTE *stack = own_firm(firms, firm);
    if (stack == NULL)
        return -1;
    void *wanted[2] = {NULL, NULL}, *scratch = NULL; /* a std::map: its head node, its size */
    ((desired_fn)AT(RE_VA_FIRM_DESIRED))(firms, wanted, firm);
    needs_ctx c = {stack + RE_FIRM_OWNED, out, cap, 0};
    int walked = map_walk((const BYTE *)wanted, 0x18, visit_need, &c);
    if (re_readable(wanted[0], 0x18)) { /* given back the way the game does it: the entries, then the head */
        ((erase_fn)AT(RE_VA_MAP_ERASE))(wanted, &scratch, *(void **)wanted[0], wanted[0]);
        ((delete_fn)AT(RE_VA_SIZED_DELETE))(wanted[0], 0x18);
    }
    return walked < 0 ? -1 : c.count;
}

typedef struct {
    const BYTE *stack;
    int tag;
    re_ware *out;
    int cap, count;
} wares_ctx;

/* What a ware uses up in an hour, in cents at the day's prices. Its lists "consumption" (while it is used) and
 * "consumptionPassive" (all the time) name a utility - electricity, fuel, maintenance - and how much of it an hour;
 * a utility has a "baseCost", and the game's price function says what that amount costs today. The game pays a
 * business's utilities the same way (FUN_005595a0).
 *
 * The lists come from the game's own getter, as FUN_004455f0 asks for them: the map the powers of a ware are read
 * from holds a ware's "power" and "industry" and not these two (seen in the game, run 200). The getter takes its
 * three names as std::string by value and destroys them; the longer name is built by the game's assign, so that
 * the block it frees is one of its own. The vector it fills is a copy and goes back through the game's function. */
static double ware_running(const BYTE *stack, BYTE *ware_info)
{
    typedef void(__thiscall * list_fn)(void *data, const BYTE **vector, void *info, int zero, short_string parent, short_string element,
                                       short_string condition);
    typedef void(__fastcall * list_free_fn)(const BYTE **vector);
    typedef void(__thiscall * assign_fn)(void *string, const char *text, unsigned length);
    typedef long long(__thiscall * now_fn)(void *prices, void *info, long long cents);
    static const char *const lists[] = {"consumption", "consumptionPassive"};
    void *data = *(void *const *)(stack + RE_FIRM_DATA), *prices = *(void *const *)(stack + RE_FIRM_PRICES);
    double sum = 0.0;
    for (int l = 0; l < 2; l++) {
        const BYTE *vector[3] = {NULL, NULL, NULL};
        short_string parent = {"tags", 4, 15}, element = {"", 0, 15}, condition = {"", 0, 15};
        ((assign_fn)AT(RE_VA_STR_ASSIGN))(&element, lists[l], (unsigned)strlen(lists[l]));
        ((list_fn)AT(RE_VA_INFO_LIST))(data, vector, ware_info, 0, parent, element, condition);
        SIZE_T bytes = (SIZE_T)(vector[1] - vector[0]);
        if (vector[1] > vector[0] && bytes % RE_INDUCED_BYTES == 0 && bytes <= 64 * RE_INDUCED_BYTES && re_readable(vector[0], bytes))
            for (const BYTE *e = vector[0]; e != vector[1]; e += RE_INDUCED_BYTES) {
                BYTE info[RE_INFO_BYTES + 4];
                if (!info_of(stack, *(const int *)(e + RE_INDUCED_JOB), 1, info))
                    continue;
                /* a thousand units, so that the game's whole cents lose nothing */
                long long thousand = (long long)(info_number(info, "stats", "baseCost") * 1000.0);
                if (thousand > 0)
                    sum += *(const float *)(e + RE_INDUCED_CHANCE) * (double)((now_fn)AT(RE_VA_PRICE_NOW))(prices, info, thousand) / 1000.0;
                info_done(info);
            }
        ((list_free_fn)AT(RE_VA_LIST_FREE))(vector);
    }
    return sum;
}

/* one id of the data: a ware when it has a list of what it brings, and wanted here when that list has the tag */
static int visit_ware(const BYTE *n, void *vctx)
{
    typedef long long(__thiscall * price_fn)(void *prices, void *info, short_string element, short_string attribute);
    wares_ctx *c = (wares_ctx *)vctx;
    const key_ctx path[2] = {{0, "tags", NULL}, {0, "power", NULL}};
    const BYTE *at = n + RE_SPECIALS_ID_VALUE;
    for (int level = 0; level < 2; level++) {
        key_ctx key = path[level];
        if (map_walk(at, RE_SPECIALS_TEXT_VALUE + 8, visit_key, &key) < 0)
            return 0;
        if (key.node == NULL)
            return 1;
        at = key.node + RE_SPECIALS_TEXT_VALUE;
    }
    const BYTE *first = *(const BYTE *const *)at, *last = *(const BYTE *const *)(at + 4);
    SIZE_T bytes = (SIZE_T)(last - first);
    if (last < first || bytes % RE_INDUCED_BYTES != 0 || bytes > 64 * RE_INDUCED_BYTES || (bytes != 0 && !re_readable(first, bytes)))
        return 1;
    long long power = 0;
    for (const BYTE *e = first; e != last; e += RE_INDUCED_BYTES)
        if (*(const int *)(e + RE_INDUCED_JOB) == c->tag)
            power += *(const long long *)e;
    if (power <= 0 || power > 100000 || c->count >= c->cap)
        return 1;
    re_ware *w = &c->out[c->count];
    BYTE info[RE_INFO_BYTES + 4];
    short_string shop = {"shop", 4, 15}, money = {"money", 5, 15};
    memset(w, 0, sizeof *w);
    w->type = *(const int *)(n + NODE_HEADER);
    w->power = (int)power;
    if (!info_of(c->stack, w->type, 0, info))
        return 1;
    w->space = info_number(info, "stats", "floorspace");
    w->fixed = info_number(info, "stats", "fixedAsset") != 0.0;
    w->active = info_number(info, "stats", "active") != 0.0;
    w->hours = info_number(info, "stats", "usageHours");
    if (!(w->hours > 0.0))
        w->hours = info_number(info, "stats", "decayHours");
    w->running = ware_running(c->stack, info);
    w->price = -((price_fn)AT(RE_VA_WARE_PRICE))(*(void *const *)(c->stack + RE_FIRM_PRICES), info, shop, money);
    info_name(info, w->name, sizeof w->name);
    info_done(info);
    c->count++;
    return 1;
}

int re_game_tag_wares(void *firms, int firm, int tag, re_ware *out, int cap)
{
    const BYTE *stack = own_firm(firms, firm);
    if (stack == NULL)
        return -1;
    const BYTE *data = *(const BYTE *const *)(stack + RE_FIRM_DATA);
    wares_ctx c = {stack, tag, out, cap, 0};
    if (!re_readable(data, RE_DATA_LISTS + 8) || !re_readable(*(const BYTE *const *)(stack + RE_FIRM_PRICES), 4) ||
        map_walk(data + RE_DATA_LISTS, RE_SPECIALS_ID_VALUE + 8, visit_ware, &c) < 0)
        return -1;
    return c.count;
}

void re_game_tag_name(void *firms, int firm, int tag, char *out, unsigned cap)
{
    const BYTE *stack = own_firm(firms, firm);
    BYTE info[RE_INFO_BYTES + 4];
    out[0] = 0;
    if (stack == NULL || !info_of(stack, tag, 1, info))
        return;
    info_name(info, out, cap);
    info_done(info);
}

int re_game_firm_space(void *firms, int firm, double *used, double *most)
{
    const BYTE *stack = own_firm(firms, firm);
    if (stack == NULL)
        return 0;
    void *space = *(void *const *)(stack + RE_FIRM_SPACE);
    const BYTE *houses = *(const BYTE *const *)(stack + RE_FIRM_HOUSES);
    key_ctx house = {*(const int *)(stack + RE_FIRM_PREMISES), NULL, NULL};
    if (!re_readable(space, 4) || !re_readable(houses, RE_HOUSES_OTHER + 8))
        return 0;
    if (map_walk(houses + RE_HOUSES_OWN, RE_HOUSES_NODE_RECORD + RE_HOUSE_BYTES, visit_key, &house) < 0)
        return 0;
    if (house.node == NULL && map_walk(houses + RE_HOUSES_OTHER, RE_HOUSES_NODE_RECORD + RE_HOUSE_BYTES, visit_key, &house) < 0)
        return 0;
    if (house.node == NULL)
        return 0;
    *most = *(const float *)(house.node + RE_HOUSES_NODE_RECORD + RE_HOUSE_FLOORSPACE);
    *used = re_business_length(AT(RE_VA_SPACE_USED), space, (int)(UINT_PTR)(stack + RE_FIRM_STOCKPILE));
    return 1;
}

int re_game_firm_premises(void *firms, int firm, re_premises *out)
{
    typedef long long(__thiscall * cost_fn)(void *houses, int house, int kind);
    typedef void *(__thiscall * info_fn)(void *data, void *info, int complain, list_element key);
    typedef void(__thiscall * list_fn)(void *data, const BYTE **vector, void *info, int zero, short_string parent, short_string element,
                                       short_string condition);
    typedef void(__fastcall * list_free_fn)(const BYTE **vector);
    const BYTE *stack = own_firm(firms, firm);
    memset(out, 0, sizeof *out);
    if (stack == NULL)
        return 0;
    BYTE *houses = *(BYTE *const *)(stack + RE_FIRM_HOUSES);
    void *data = *(void *const *)(stack + RE_FIRM_DATA);
    key_ctx house = {*(const int *)(stack + RE_FIRM_PREMISES), NULL, NULL};
    if (!re_readable(houses, RE_HOUSES_OTHER + 8) || !re_readable(data, RE_DATA_LISTS + 8) ||
        map_walk(houses + RE_HOUSES_OWN, RE_HOUSES_NODE_RECORD + RE_HOUSE_BYTES, visit_key, &house) < 0)
        return 0;
    if (house.node == NULL && map_walk(houses + RE_HOUSES_OTHER, RE_HOUSES_NODE_RECORD + RE_HOUSE_BYTES, visit_key, &house) < 0)
        return 0;
    if (house.node == NULL)
        return 0;
    const BYTE *record = house.node + RE_HOUSES_NODE_RECORD;
    int kind = *(const int *)(record + RE_HOUSE_STATE);
    if (kind != RE_HOUSE_OWNED && kind != RE_HOUSE_RENTED)
        return 0;
    out->house = house.id;
    out->type = *(const int *)(record + RE_HOUSE_TYPE);
    out->owned = kind == RE_HOUSE_OWNED;
    out->growing = *(const int *)(record + RE_HOUSE_GROW);
    out->space = *(const float *)(record + RE_HOUSE_FLOORSPACE);
    string_text(record + RE_HOUSE_ADDRESS, out->address, sizeof out->address);
    out->cost = ((cost_fn)AT(RE_VA_GROW_COST))(houses, house.id, kind); /* as the month end asks it: no random number */
    /* what the type grows into: the first <upgrade> of its <property>; none is the largest of its kind */
    BYTE info[RE_INFO_BYTES + 4];
    const BYTE *vector[3] = {NULL, NULL, NULL};
    short_string parent = {"property", 8, 15}, element = {"upgrade", 7, 15}, condition = {"", 0, 15};
    list_element type;
    memcpy(&type, record, sizeof type);
    memset(info, 0, sizeof info);
    ((info_fn)AT(RE_VA_RESTATE_INFO))(data, info, 0, type);
    ((list_fn)AT(RE_VA_INFO_LIST))(data, vector, info, 0, parent, element, condition);
    SIZE_T bytes = (SIZE_T)(vector[1] - vector[0]);
    if (vector[1] > vector[0] && bytes % RE_INDUCED_BYTES == 0 && bytes <= 8 * RE_INDUCED_BYTES && re_readable(vector[0], bytes))
        out->next = *(const int *)(vector[0] + RE_INDUCED_JOB);
    ((list_free_fn)AT(RE_VA_LIST_FREE))(vector);
    info_done(info);
    return 1;
}

int re_game_firm_premises_grow(void *firms, int firm, int type)
{
    const BYTE *stack = own_firm(firms, firm);
    if (stack == NULL)
        return 0;
    const BYTE *houses = *(const BYTE *const *)(stack + RE_FIRM_HOUSES);
    key_ctx house = {*(const int *)(stack + RE_FIRM_PREMISES), NULL, NULL};
    if (!re_readable(houses, RE_HOUSES_OTHER + 8) ||
        map_walk(houses + RE_HOUSES_OWN, RE_HOUSES_NODE_RECORD + RE_HOUSE_BYTES, visit_key, &house) < 0)
        return 0;
    if (house.node == NULL && map_walk(houses + RE_HOUSES_OTHER, RE_HOUSES_NODE_RECORD + RE_HOUSE_BYTES, visit_key, &house) < 0)
        return 0;
    if (house.node == NULL)
        return 0;
    *(int *)(UINT_PTR)(house.node + RE_HOUSES_NODE_RECORD + RE_HOUSE_GROW) = type; /* what a click on the star writes */
    return 1;
}

void re_game_asset_replace(void *firms, int firm, int type)
{
    typedef void(__thiscall *replace_fn)(void *business, int type);
    const BYTE *stack = own_firm(firms, firm);
    if (stack != NULL)
        ((replace_fn)AT(RE_VA_ASSET_REPLACE))((void *)(UINT_PTR)stack, type);
}

int re_game_firm_name(void *firms, int firm, char *out, unsigned cap)
{
    key_ctx c = {firm, NULL, NULL};
    return re_readable(firms, RE_FIRMS_OWN + 8) &&
           map_walk((const BYTE *)firms + RE_FIRMS_OWN, RE_FIRMS_NODE_FIRM + RE_FIRM_BYTES, visit_key, &c) >= 0 && c.node != NULL &&
           string_text(c.node + RE_FIRMS_NODE_FIRM + RE_FIRM_NAME, out, cap);
}

int re_game_staff_read(void *jobs, const BYTE *record, re_staff_read *out)
{
    typedef long long(__thiscall *wage_fn)(void *jobs, int job);
    if (jobs == NULL || !re_readable(record, RE_STAFF_BYTES))
        return 0;
    out->id = *(const int *)(record + RE_STAFF_ID);
    out->job = *(const int *)(record + RE_STAFF_JOB);
    out->hours = *(const int *)(record + RE_STAFF_HOURS);
    out->start = *(const int *)(record + RE_STAFF_START);
    out->left = *(const int *)(record + RE_STAFF_LEFT);
    out->wage = *(const long long *)(record + RE_STAFF_WAGE);
    out->demand = *(const long long *)(record + RE_STAFF_DEMAND);
    out->efficiency = re_business_efficiency(AT(RE_VA_STAFF_EFFICIENCY), jobs, out->job, record + RE_STAFF_SKILL);
    out->standard = ((wage_fn)AT(RE_VA_JOB_WAGE))(jobs, out->job);
    /* the game's own name of a person: the first name, a space, the last name */
    char *name = out->name;
    name[0] = 0;
    if (string_text(record + RE_STAFF_FIRST_NAME, name, 47))
        strcat(name, " ");
    if (!string_text(record + RE_STAFF_LAST_NAME, name + strlen(name), 47) && name[0] != 0)
        name[strlen(name) - 1] = 0;
    return 1;
}

long long re_game_job_wage(void *jobs, int job)
{
    typedef long long(__thiscall *wage_fn)(void *jobs, int job);
    return ((wage_fn)AT(RE_VA_JOB_WAGE))(jobs, job);
}

void re_game_staff_copy(BYTE *raw, const BYTE *record)
{
    typedef void *(__thiscall *copy_fn)(void *self, const void *from);
    ((copy_fn)AT(RE_VA_STAFF_COPY))(raw, record);
}

void re_game_staff_end(BYTE *record)
{
    typedef void(__thiscall *end_fn)(void *self);
    ((end_fn)AT(RE_VA_STAFF_END_SKILL))(record + RE_STAFF_SKILL); /* as the game ends a record: the skill part, then the rest */
    ((end_fn)AT(RE_VA_STAFF_END))(record);
}

void re_game_staff_append(void *firms, int firm, const BYTE *record)
{
    typedef void(__thiscall *append_fn)(void *staff, int firm, const BYTE *record);
    ((append_fn)AT(RE_VA_STAFF_APPEND))(*(void **)((BYTE *)firms + RE_FIRMS_STAFF), firm, record);
}

void re_game_staff_remove(void *firms, int firm, int staff)
{
    typedef void(__thiscall *remove_fn)(void *staff, int firm, int id);
    ((remove_fn)AT(RE_VA_STAFF_REMOVE))(*(void **)((BYTE *)firms + RE_FIRMS_STAFF), firm, staff);
}

void re_game_staff_set_wage(void *firms, int firm, int staff, long long wage)
{
    typedef void(__thiscall *set_fn)(void *staff, int firm, int id, unsigned lo, int hi);
    ((set_fn)AT(RE_VA_STAFF_SET_WAGE))(*(void **)((BYTE *)firms + RE_FIRMS_STAFF), firm, staff, (unsigned)wage, (int)(wage >> 32));
}

/* ---- the candidates of a job, the random streams they are drawn from, the Shift key (note b21) ---- */

/* The map job -> list of a business: 1 = `*jobs` is it, 0 = the business has none, -1 = cannot be read. */
static int candidates_of(void *firms, int firm, BYTE **jobs)
{
    key_ctx c = {firm, NULL, NULL};
    if (!re_readable(firms, RE_FIRMS_CANDIDATES + 8) ||
        map_walk((const BYTE *)firms + RE_FIRMS_CANDIDATES, RE_CANDIDATES_JOBS + 8, visit_key, &c) < 0)
        return -1;
    if (c.node == NULL)
        return 0;
    *jobs = (BYTE *)(UINT_PTR)c.node + RE_CANDIDATES_JOBS;
    return 1;
}

int re_game_candidates(void *firms, int firm, int job)
{
    BYTE *jobs = NULL;
    key_ctx c = {job, NULL, NULL};
    int has = candidates_of(firms, firm, &jobs);
    if (has < 0 || (has && map_walk(jobs, RE_STAFF_BY_FIRM_LIST + 12, visit_key, &c) < 0))
        return RE_CANDIDATES_UNREADABLE;
    if (c.node == NULL)
        return RE_CANDIDATES_NONE;
    BYTE *const *list = (BYTE *const *)(c.node + RE_STAFF_BY_FIRM_LIST);
    SIZE_T bytes = (SIZE_T)(list[1] - list[0]);
    return list[1] >= list[0] && bytes % RE_STAFF_BYTES == 0 && bytes <= 1000 * RE_STAFF_BYTES ? (int)(bytes / RE_STAFF_BYTES)
                                                                                                : RE_CANDIDATES_UNREADABLE;
}

BYTE *re_game_randgen(void *firms)
{
    if (!re_readable(firms, RE_FIRMS_RANDGEN + 4))
        return NULL;
    const BYTE *f = (const BYTE *)firms, *jobs = *(BYTE *const *)(f + RE_FIRMS_JOBS), *ages = *(BYTE *const *)(f + RE_FIRMS_AGES);
    BYTE *randgen = *(BYTE *const *)(f + RE_FIRMS_RANDGEN);
    if (!re_readable(randgen, RE_RANDGEN_SPARE + 4) || !re_readable(jobs, RE_JOBS_RANDGEN + 4) || !re_readable(ages, RE_AGES_RANDGEN + 4) ||
        *(BYTE *const *)(jobs + RE_JOBS_RANDGEN) != randgen || *(BYTE *const *)(ages + RE_AGES_RANDGEN) != randgen)
        return NULL;
    for (int i = 0; i < RE_RAND_ENGINES; i++)
        if (!re_readable(*(BYTE **)(randgen + (i < RE_RAND_STREAMS ? i * RE_RAND_UNIT_BYTES + RE_RAND_UNIT_ENGINE : RE_RANDGEN_SPARE)),
                         RE_RAND_ENGINE_BYTES))
            return NULL;
    return randgen;
}

int re_game_shift_down(void *main_obj)
{
    BYTE *pfm = pfm_of(main_obj);
    BYTE *ui = pfm != NULL ? *(BYTE **)(pfm + RE_PFM_DEBTINV + RE_DEBTINV_UI) : NULL;
    return re_readable(ui, RE_UI_HIRE_WINDOW + RE_WINDOW_KEYS + 4) && re_game_shift_held(ui + RE_UI_HIRE_WINDOW);
}

long long re_game_company_price_at(void *market, int company, int period)
{
    key_ctx statement = {company, NULL, NULL}, month = {period, NULL, NULL}, price = {RE_STAT_PRICE, NULL, NULL};
    if (!re_readable(market, RE_MARKET_HISTORY + 8) ||
        map_walk((const BYTE *)market + RE_MARKET_HISTORY, RE_HISTORY_NODE_STATEMENT + 0x28, visit_key, &statement) < 0 ||
        statement.node == NULL)
        return 0;
    /* a statement as the cash flow's: months at +0x20, a month's income by tag at +0x28 of its node */
    if (map_walk(statement.node + RE_HISTORY_NODE_STATEMENT + 0x20, 0x38, visit_key, &month) < 0 || month.node == NULL ||
        map_walk(month.node + 0x28, 0x20, visit_key, &price) < 0 || price.node == NULL)
        return 0;
    return *(const long long *)(price.node + 0x18);
}

typedef struct {
    const BYTE *ticks;
    int keep, cleared;
} tick_ctx;

/* a box of the chart: its series' tick. The record of ticks outlives a chart - the window is used again for the
 * next one - so it has series of earlier charts too, which are not touched. */
static int visit_box(const BYTE *n, void *vctx)
{
    tick_ctx *c = (tick_ctx *)vctx;
    int series = *(const int *)(n + NODE_HEADER);
    key_ctx tick = {series, NULL, NULL};
    if (map_walk(c->ticks, NODE_HEADER + 8, visit_key, &tick) < 0 || tick.node == NULL)
        return 1;
    BYTE *value = (BYTE *)(UINT_PTR)tick.node + RE_TICK_NODE_VALUE;
    c->cleared += *value && series != c->keep;
    *value = series == c->keep;
    return 1;
}

int re_game_chart_only(void *chart, int series)
{
    BYTE *c = (BYTE *)chart;
    key_ctx box = {series, NULL, NULL};
    tick_ctx ticks = {c + RE_CHART_TICKS, series, 0};
    if (!re_readable(c, RE_CHART_BOXES + 8) || *(const int *)(c + RE_CHART_KIND) != RE_CHART_KIND_STATEMENT ||
        map_walk(c + RE_CHART_BOXES, NODE_HEADER + 8, visit_key, &box) < 0 || box.node == NULL ||
        map_walk(c + RE_CHART_BOXES, NODE_HEADER + 8, visit_box, &ticks) < 0)
        return -1;
    c[RE_CHART_ALL] = 0;
    return ticks.cleared;
}

int re_game_chart_kind(const void *chart)
{
    return re_readable(chart, RE_CHART_BOXES + 8) ? *(const int *)((const BYTE *)chart + RE_CHART_KIND) : -1;
}

typedef struct {
    const float (*palette)[4];
    int count, at;
} colour_ctx;

static int visit_colour(const BYTE *n, void *vctx)
{
    colour_ctx *c = (colour_ctx *)vctx;
    memcpy((BYTE *)(UINT_PTR)n + RE_COLOUR_NODE_VALUE, c->palette[c->at++ % c->count], 4 * sizeof(float));
    return 1;
}

int re_game_chart_colours(void *chart, const float (*palette)[4], int count)
{
    colour_ctx c = {palette, count, 0};
    if (!re_readable(chart, RE_CHART_BOXES + 8) || count < 1)
        return -1;
    return map_walk((const BYTE *)chart + RE_CHART_COLOURS, RE_COLOUR_NODE_VALUE + 4 * sizeof(float), visit_colour, &c);
}

typedef struct {
    const BYTE *ticks;
    const int *keep;
    int count, changed;
} keep_ctx;

/* a series of the chart (an entry of its colours): its tick in the record, when the record has one */
static int visit_series(const BYTE *n, void *vctx)
{
    keep_ctx *c = (keep_ctx *)vctx;
    int series = *(const int *)(n + NODE_HEADER), on = c->count == 0;
    key_ctx tick = {series, NULL, NULL};
    for (int i = 0; i < c->count; i++)
        on |= c->keep[i] == series;
    if (map_walk(c->ticks, NODE_HEADER + 8, visit_key, &tick) < 0 || tick.node == NULL)
        return 1;
    BYTE *value = (BYTE *)(UINT_PTR)tick.node + RE_TICK_NODE_VALUE;
    c->changed += (*value != 0) != on;
    *value = (BYTE)on;
    return 1;
}

int re_game_chart_keep(void *chart, const int *keep, int count)
{
    BYTE *c = (BYTE *)chart;
    keep_ctx ticks = {c + RE_CHART_TICKS, keep, count < 0 ? 1 : count, 0};
    static const int none = 0; /* no series has the tag 0: with it nothing is kept */
    if (count < 0)
        ticks.keep = &none;
    if (!re_readable(c, RE_CHART_BOXES + 8) || map_walk(c + RE_CHART_COLOURS, NODE_HEADER + 8, visit_series, &ticks) < 0)
        return -1;
    if (count != 0)
        c[RE_CHART_ALL] = 0;
    return ticks.changed;
}

typedef struct {
    BYTE *ticks;
    const int *keep;
    int count, kept;
} set_ctx;

/* a series of the chart: its tick in the record, made by the game's own function when the record has none */
static int visit_series_set(const BYTE *n, void *vctx)
{
    typedef BYTE *(__thiscall * at_fn)(void *map, const int *key);
    set_ctx *c = (set_ctx *)vctx;
    int series = *(const int *)(n + NODE_HEADER), on = 0;
    for (int i = 0; i < c->count; i++)
        on |= c->keep[i] == series;
    BYTE *value = ((at_fn)AT(RE_VA_MAP_INT_AT))(c->ticks, &series);
    if (!re_readable(value, 1))
        return 0;
    *value = (BYTE)on;
    c->kept += on;
    return 1;
}

int re_game_chart_set(void *chart, const int *keep, int count)
{
    BYTE *c = (BYTE *)chart;
    set_ctx ticks = {c + RE_CHART_TICKS, keep, count, 0};
    if (!re_readable(c, RE_CHART_BOXES + 8) || count < 1 || map_walk(c + RE_CHART_COLOURS, NODE_HEADER + 8, visit_series_set, &ticks) < 0)
        return -1;
    c[RE_CHART_ALL] = 0;
    return ticks.kept;
}

int re_game_chart_series(const void *chart, int series)
{
    key_ctx entry = {series, NULL, NULL};
    return re_readable(chart, RE_CHART_BOXES + 8) && map_walk((const BYTE *)chart + RE_CHART_COLOURS, NODE_HEADER + 8, visit_key, &entry) >= 0 &&
           entry.node != NULL;
}

int re_game_chart_cap(void *chart, int months)
{
    BYTE *c = (BYTE *)chart;
    if (!re_readable(c, RE_CHART_BOXES + 8))
        return -1;
    int *shown = (int *)(c + RE_CHART_SHOWN);
    if (*shown > months)
        *shown = months;
    return *shown;
}

static int visit_last(const BYTE *n, void *vctx)
{
    *(const BYTE **)vctx = n; /* the walk goes up the keys: the one left standing is the last month */
    return 1;
}

int re_game_chart_last(const void *chart, int series, long long *value)
{
    const BYTE *c = (const BYTE *)chart, *month = NULL;
    key_ctx entry = {series, NULL, NULL};
    if (!re_readable(c, RE_CHART_BOXES + 8))
        return 0;
    /* a statement as the cash flow's: months at +0x20; a month's two maps <series, int64> at +0x18 and +0x28 of its node */
    const BYTE *statement = *(const BYTE *const *)(c + RE_CHART_STATEMENT);
    if (!re_readable(statement, 0x28) || map_walk(statement + 0x20, 0x38, visit_last, &month) < 0 || month == NULL)
        return 0;
    for (int i = 0; i < 2 && entry.node == NULL; i++)
        if (map_walk(month + (i ? 0x18 : 0x28), 0x20, visit_key, &entry) < 0)
            return 0;
    if (entry.node == NULL)
        return 0;
    *value = *(const long long *)(entry.node + RE_MONTH_NODE_VALUE);
    return 1;
}

int re_game_chart_has(const void *chart, int series)
{
    key_ctx box = {series, NULL, NULL};
    return re_readable(chart, RE_CHART_BOXES + 8) && map_walk((const BYTE *)chart + RE_CHART_BOXES, NODE_HEADER + 8, visit_key, &box) >= 0 &&
           box.node != NULL;
}

typedef struct {
    const BYTE *chart;
    int *series;
    unsigned char *ticked;
    int cap, count, write;
} page_ctx;

/* a box of the page shown: what the record says of its series, or the flag of the box for all when the record has
 * nothing; or, the other way, what was noted is written into the record */
static int visit_page_box(const BYTE *n, void *vctx)
{
    page_ctx *c = (page_ctx *)vctx;
    int series = *(const int *)(n + NODE_HEADER);
    key_ctx tick = {series, NULL, NULL};
    if (map_walk(c->chart + RE_CHART_TICKS, NODE_HEADER + 8, visit_key, &tick) < 0)
        return 0;
    if (c->write) {
        for (int i = 0; i < c->count && tick.node != NULL; i++)
            if (c->series[i] == series)
                *((BYTE *)(UINT_PTR)tick.node + RE_TICK_NODE_VALUE) = c->ticked[i];
        return 1;
    }
    if (c->count < c->cap) {
        c->series[c->count] = series;
        c->ticked[c->count++] = tick.node != NULL ? tick.node[RE_TICK_NODE_VALUE] != 0 : c->chart[RE_CHART_ALL] != 0;
    }
    return 1;
}

int re_game_chart_page(void *chart, int *series, unsigned char *ticked, int count, int write)
{
    page_ctx c = {(const BYTE *)chart, series, ticked, count, write ? count : 0, write};
    if (!re_readable(chart, RE_CHART_BOXES + 8) || map_walk(c.chart + RE_CHART_BOXES, NODE_HEADER + 8, visit_page_box, &c) < 0)
        return -1;
    return c.count;
}

typedef struct {
    re_chart_base *base;
    int cap, *count, scaled;
} index_ctx;

static int visit_month_value(const BYTE *n, void *vctx)
{
    index_ctx *c = (index_ctx *)vctx;
    int series = *(const int *)(n + NODE_HEADER), at = 0;
    long long *value = (long long *)(UINT_PTR)(n + RE_MONTH_NODE_VALUE);
    if (*value <= 0)
        return 1;
    while (at < *c->count && c->base[at].series != series)
        at++;
    if (at == *c->count) {
        if (at == c->cap)
            return 1; /* more series than the table takes: left as it is */
        c->base[at].series = series;
        c->base[at].value = *value;
        (*c->count)++;
    }
    *value = (long long)((double)*value * 10000.0 / (double)c->base[at].value + 0.5);
    c->scaled++;
    return 1;
}

int re_game_month_index(void *map, re_chart_base *base, int cap, int *count)
{
    index_ctx c = {base, cap, count, 0};
    return map_walk((const BYTE *)map, RE_MONTH_NODE_VALUE + 8, visit_month_value, &c) < 0 ? -1 : c.scaled;
}

typedef struct {
    const BYTE *statement;
    re_xp_visit visit;
    void *ctx;
} xp_ctx;

static int visit_xp(const BYTE *n, void *vctx)
{
    xp_ctx *c = (xp_ctx *)vctx;
    int tag = *(const int *)(n + NODE_HEADER);
    key_ctx gained = {tag, NULL, NULL}, lost = {tag, NULL, NULL};
    if (map_walk(c->statement + RE_XP_GAINED, 0x20, visit_key, &gained) < 0 || map_walk(c->statement + RE_XP_LOST, 0x20, visit_key, &lost) < 0)
        return 0;
    c->visit(tag, gained.node != NULL ? *(const long long *)(gained.node + 0x18) : 0,
             lost.node != NULL ? *(const long long *)(lost.node + 0x18) : 0, (long long *)(UINT_PTR)(n + 0x18), c->ctx);
    return 1;
}

int re_game_xp_walk(void *statement, re_xp_visit visit, void *ctx)
{
    xp_ctx c = {(const BYTE *)statement, visit, ctx};
    if (!re_readable(statement, RE_XP_EFFECTIVE + 8))
        return -1;
    return map_walk(c.statement + RE_XP_EFFECTIVE, 0x20, visit_xp, &c);
}

int re_game_chart_step(void *chart, const int *steps, int count)
{
    BYTE *c = (BYTE *)chart;
    if (!re_readable(c, RE_CHART_BOXES + 8) || count < 1)
        return -1;
    int *shown = (int *)(c + RE_CHART_SHOWN), most = *(const int *)(c + RE_CHART_MOST), next = -1;
    for (int i = 0; i < count && next < 0; i++)
        if (steps[i] > *shown && steps[i] < most)
            next = steps[i];
    /* past the last step under the most: the most; from the most (or anything above the steps): the first again */
    *shown = next >= 0 ? next : *shown < most ? most : steps[0] < most ? steps[0] : most;
    return *shown;
}

int re_game_shift_held(void *window)
{
    return re_readable(window, RE_WINDOW_KEYS + 4) && re_game_keys_shift(*(void *const *)((const BYTE *)window + RE_WINDOW_KEYS));
}

int re_game_window_firm(void *main_obj)
{
    BYTE *pfm = pfm_of(main_obj);
    BYTE *ui = pfm != NULL ? *(BYTE **)(pfm + RE_PFM_DEBTINV + RE_DEBTINV_UI) : NULL;
    return re_readable(ui, RE_UI_FIRM_WINDOW + RE_ADVERT_WINDOW_FIRM + 4) ? *(const int *)(ui + RE_UI_FIRM_WINDOW + RE_ADVERT_WINDOW_FIRM) : 0;
}

int re_game_ctrl_down(void *main_obj)
{
    BYTE *pfm = pfm_of(main_obj);
    BYTE *ui = pfm != NULL ? *(BYTE **)(pfm + RE_PFM_DEBTINV + RE_DEBTINV_UI) : NULL;
    if (!re_readable(ui, RE_UI_HIRE_WINDOW + RE_WINDOW_KEYS + 4))
        return 0;
    const void *keys = *(void *const *)(ui + RE_UI_HIRE_WINDOW + RE_WINDOW_KEYS);
    key_ctx left = {RE_KEY_CTRL, NULL, NULL}, right = {RE_KEY_CTRL_RIGHT, NULL, NULL};
    return map_walk((const BYTE *)keys, NODE_HEADER + 4, visit_key, &left) >= 0 &&
           map_walk((const BYTE *)keys, NODE_HEADER + 4, visit_key, &right) >= 0 && (left.node != NULL || right.node != NULL);
}

int re_game_keys_shift(const void *keys)
{
    key_ctx left = {RE_KEY_SHIFT, NULL, NULL}, right = {RE_KEY_SHIFT_RIGHT, NULL, NULL};
    /* the set the game counts a key in (0x005229d0); a key is the whole of a node's value */
    return map_walk((const BYTE *)keys, NODE_HEADER + 4, visit_key, &left) >= 0 &&
           map_walk((const BYTE *)keys, NODE_HEADER + 4, visit_key, &right) >= 0 && (left.node != NULL || right.node != NULL);
}

typedef struct {
    int count, job[OFFER_JOBS], hours[OFFER_JOBS];
} offer_jobs;

static int visit_offer_job(const BYTE *n, void *vctx)
{
    offer_jobs *c = (offer_jobs *)vctx;
    if (c->count == OFFER_JOBS)
        return 0;
    c->job[c->count] = *(const int *)(n + 0x10);
    c->hours[c->count++] = *(const int *)(n + 0x14);
    return 1;
}

int re_game_offer(void *main_obj, const BYTE *shown_firm, const BYTE *contract, re_offer_read *out)
{
    typedef long long(__thiscall *wage_fn)(void *jobs, int job);
    wage_fn wage = (wage_fn)AT(RE_VA_JOB_WAGE);
    BYTE *firms = (BYTE *)re_game_firms(main_obj);
    void *jobs = re_game_jobs(firms);
    re_offer *o = &out->terms;
    memset(out, 0, sizeof *out);
    if (jobs == NULL || !re_readable(firms, RE_FIRMS_OWN + 8) || !re_readable(shown_firm, RE_FIRM_ID + 4) ||
        !re_readable(contract, RE_CONTRACT_BYTES))
        return 0;
    const BYTE *economy = *(const BYTE *const *)(firms + RE_FIRMS_ECONOMY), *data = *(const BYTE *const *)(firms + RE_FIRMS_DATA);
    const BYTE *settings = *(const BYTE *const *)(firms + RE_FIRMS_SETTINGS);
    if (!re_readable(economy, RE_ECON_INDEX + 8) || !re_readable(data, RE_DATA_SPECIALS + 8) || !re_readable(settings, RE_POLICY_VALUES + 8))
        return 0;
    out->type = *(const int *)(contract + RE_CONTRACT_TYPE);
    o->total = *(const long long *)(contract + RE_CONTRACT_TOTAL);
    o->months = *(const int *)(contract + RE_CONTRACT_MONTHS);

    /* The business as the game keeps it, by the id of the one the hover text was given: the payout function looks
     * it up the same way, and its type names the lists below. */
    key_ctx firm = {*(const int *)(shown_firm + RE_FIRM_ID), NULL, NULL};
    key_ctx policy = {RE_TAG_WAGE_POLICY, NULL, NULL}, index = {RE_FUT_OVERALL, NULL, NULL};
    offer_jobs listed = {0};
    if (map_walk(firms + RE_FIRMS_OWN, RE_FIRMS_NODE_FIRM + RE_FIRM_BYTES, visit_key, &firm) < 0 || firm.node == NULL ||
        map_walk(settings + RE_POLICY_VALUES, 0x18, visit_key, &policy) < 0 ||
        map_walk(economy + RE_ECON_INDEX, 0x18, visit_key, &index) < 0 || index.node == NULL ||
        map_walk(contract + RE_CONTRACT_HOURS, 0x18, visit_offer_job, &listed) < 0)
        return 0;
    int type = *(const int *)(firm.node + RE_FIRMS_NODE_FIRM + RE_FIRM_TYPE);
    if (type == 0 || o->total <= 0 || o->months <= 0 || o->months > OFFER_MONTHS)
        return 0;
    /* What a wage policy of the city adds to the standard wage of every job, worked out as the wage function does:
     * whole cents of (factor - 1) x 10 dollars, then the overall price level. No entry is a factor of 0 there too. */
    float factor = policy.node != NULL ? *(const float *)(policy.node + 0x14) : 0.0f;
    o->policy = (double)(long long)((float)(long long)(((double)factor - 1.0) * 1000.0) * *(const float *)(index.node + 0x14));

    for (int i = 0; i < listed.count; i++) {
        int job = listed.job[i], hours = listed.hours[i];
        if (hours < 0 || hours > OFFER_HOURS)
            return 0;
        if (hours == 0)
            continue;
        /* The jobs this one sets off: four keys deep, a list behind the last. A key that is not there means none:
         * the payout function adds an empty entry for it and finds none either. */
        const key_ctx path[4] = {{type, NULL, NULL}, {job, NULL, NULL}, {0, "SPECIAL", NULL}, {0, "CONTRACTANYGEN", NULL}};
        const BYTE *at = data + RE_DATA_SPECIALS;
        for (int level = 0; at != NULL && level < 4; level++) {
            key_ctx key = path[level];
            SIZE_T value = level < 2 ? RE_SPECIALS_ID_VALUE : RE_SPECIALS_TEXT_VALUE;
            if (map_walk(at, value + 8, visit_key, &key) < 0)
                return 0;
            at = key.node != NULL ? key.node + value : NULL;
        }
        const BYTE *first = at != NULL ? *(const BYTE *const *)at : NULL, *last = at != NULL ? *(const BYTE *const *)(at + 4) : NULL;
        SIZE_T bytes = (SIZE_T)(last - first);
        if (last < first || bytes % RE_INDUCED_BYTES != 0 || bytes > OFFER_JOBS * RE_INDUCED_BYTES || (bytes != 0 && !re_readable(first, bytes)))
            return 0;
        double hour = (double)wage(jobs, job), set_off = 0.0; /* an hour of the job with what it sets off; the hours set off */
        if (!(hour > 0.0))
            return 0;
        for (const BYTE *e = first; e != last; e += RE_INDUCED_BYTES) {
            double share = (double)*(const long long *)e * *(const float *)(e + RE_INDUCED_CHANCE);
            if (!(share >= 0.0) || share > 100.0)
                return 0;
            double other = (double)wage(jobs, *(const int *)(e + RE_INDUCED_JOB));
            if (!(other > 0.0))
                return 0;
            hour += share * other;
            set_off += share;
        }
        o->labour += hours * hour;
        o->hours += hours;
        o->induced += hours * set_off;
    }
    if (!(o->hours > 0.0))
        return 0;

    /* The month's cost as the payout weighs it: the game's own function on a copy of the contract, as at the making
     * of the offer but with today's prices, without the three factors that come after the weighing. */
    out->today = re_business_payout(AT(RE_VA_CONTRACT_COPY), AT(RE_VA_CONTRACT_PAYOUT), firms, firm.id, contract, RE_CONTRACT_BYTES);
    out->difficulty = *(const float *)(settings + RE_SETTINGS_REVENUE);
    out->length = re_business_length(AT(RE_VA_LENGTH_BONUS), firms, o->months);
    if (out->today <= 0 || !(out->difficulty > 0.0) || !(out->length > 0.0))
        return 0;
    o->weighted = (double)out->today / (o->months * out->difficulty * out->length);
    static const char *const markups[3] = {"contractJobHoursPayoutXer", "contractInventoryPayoutXer", "contractUtilityPayoutXer"};
    double *const into[3] = {&o->x_labour, &o->x_inventory, &o->x_utilities};
    for (int i = 0; i < 3; i++)
        *into[i] = re_business_data_number(AT(RE_VA_DATA_NUMBER), (void *)(UINT_PTR)data, AT(RE_VA_STR_ASSIGN), "business", 8, markups[i],
                                           (unsigned)strlen(markups[i]));
    return 1;
}

typedef struct {
    re_signed *out;
    int cap, count;
} signed_ctx;

static int visit_signed(const BYTE *n, void *vctx)
{
    signed_ctx *c = (signed_ctx *)vctx;
    const BYTE *contract = n + RE_SIGNED_NODE_CONTRACT;
    offer_jobs listed = {0};
    if (c->count == c->cap || map_walk(contract + RE_CONTRACT_HOURS, 0x18, visit_offer_job, &listed) < 0)
        return 0;
    re_signed *s = &c->out[c->count++];
    s->id = *(const int *)(n + 0x10);
    s->type = *(const int *)(contract + RE_CONTRACT_TYPE);
    s->months = *(const int *)(contract + RE_CONTRACT_MONTHS);
    s->start = *(const int *)(contract + RE_CONTRACT_START);
    s->hours = 0;
    for (int i = 0; i < listed.count; i++)
        s->hours += listed.hours[i];
    return 1;
}

int re_game_firm_signed(void *firms, int firm, re_signed *out, int cap)
{
    const BYTE *stack = own_firm(firms, firm);
    signed_ctx c = {out, cap, 0};
    if (stack == NULL || map_walk(stack + RE_FIRM_SIGNED, RE_SIGNED_NODE_CONTRACT + RE_CONTRACT_BYTES, visit_signed, &c) < 0)
        return -1;
    return c.count;
}

typedef struct {
    re_coming *out;
    int cap, count, month;
} coming_ctx;

static int visit_coming(const BYTE *n, void *vctx)
{
    coming_ctx *c = (coming_ctx *)vctx;
    const BYTE *contract = n + RE_SIGNED_NODE_CONTRACT;
    int start = *(const int *)(contract + RE_CONTRACT_START), months = *(const int *)(contract + RE_CONTRACT_MONTHS);
    int sign = start == c->month ? 1 : start + months == c->month ? -1 : 0;
    offer_jobs listed = {0};
    if (sign == 0)
        return 1;
    if (map_walk(contract + RE_CONTRACT_HOURS, 0x18, visit_offer_job, &listed) < 0)
        return 0;
    for (int i = 0; i < listed.count; i++) {
        int k = 0;
        while (k < c->count && c->out[k].job != listed.job[i])
            k++;
        if (k == c->count) {
            if (c->count == c->cap)
                return 0;
            c->out[c->count].job = listed.job[i];
            c->out[c->count++].hours = 0;
        }
        c->out[k].hours += sign * listed.hours[i];
    }
    return 1;
}

int re_game_firm_coming(void *firms, int firm, int month, re_coming *out, int cap)
{
    const BYTE *stack = own_firm(firms, firm);
    coming_ctx c = {out, cap, 0, month};
    if (stack == NULL || map_walk(stack + RE_FIRM_SIGNED, RE_SIGNED_NODE_CONTRACT + RE_CONTRACT_BYTES, visit_coming, &c) < 0)
        return -1;
    return c.count;
}

typedef struct {
    re_offered *out;
    int cap, count;
} offered_ctx;

static int visit_offered(const BYTE *n, void *vctx)
{
    offered_ctx *c = (offered_ctx *)vctx;
    if (c->count == c->cap)
        return 0;
    c->out[c->count].id = *(const int *)(n + 0x10);
    c->out[c->count++].contract = n + RE_SIGNED_NODE_CONTRACT;
    return 1;
}

int re_game_firm_offers(void *firms, int firm, re_offered *out, int cap, const BYTE **shown)
{
    const BYTE *stack = own_firm(firms, firm);
    offered_ctx c = {out, cap, 0};
    if (stack == NULL || map_walk(stack + RE_FIRM_OFFERS, RE_SIGNED_NODE_CONTRACT + RE_CONTRACT_BYTES, visit_offered, &c) < 0)
        return -1;
    *shown = stack;
    return c.count;
}

int re_game_firm_places(void *firms, int firm)
{
    typedef int(__thiscall * places_fn)(void *business);
    const BYTE *stack = own_firm(firms, firm);
    return stack != NULL ? ((places_fn)AT(RE_VA_FIRM_PLACES))((void *)(UINT_PTR)stack) : -1;
}

int re_game_firm_reputation(void *firms, int firm, double *reputation, int *influence)
{
    const BYTE *stack = own_firm(firms, firm);
    if (stack == NULL)
        return 0;
    *reputation = *(const float *)(const void *)(stack + RE_FIRM_REPUTATION);
    *influence = stack[RE_FIRM_INFLUENCE] != 0;
    return 1;
}

int re_game_closed_unseen;

int re_game_firm_open(void *firms, int firm)
{
    const BYTE *stack = own_firm(firms, firm);
    return stack == NULL ? -1 : re_game_closed_unseen || *(const int *)(const void *)(stack + RE_FIRM_OPEN) == 1;
}

int re_game_contract_sign(void *firms, int firm, const re_offered *offer)
{
    re_signed now[32];
    int id = offer->id;
    /* the copy goes onto the stack as for the payout function; the callee takes the offer out of its map */
    re_business_payout(AT(RE_VA_CONTRACT_COPY), AT(RE_VA_CONTRACT_SIGN), firms, firm, offer->contract, RE_CONTRACT_BYTES);
    int count = re_game_firm_signed(firms, firm, now, 32);
    for (int i = 0; i < count; i++)
        if (now[i].id == id)
            return 1;
    return 0;
}

int re_game_period(void *main_obj)
{
    if (!re_readable(main_obj, 0x400))
        return -1;
    BYTE *counter = *(BYTE **)((BYTE *)main_obj + RE_MAIN_COUNTER);
    return re_readable(counter, 0x1c) ? *(int *)(counter + 0x10) + 12 * *(int *)(counter + 0x14) : -1;
}

/* In place of the call that makes the "Research Stock" button of a company's tab. The game's maker takes its
 * object in ecx and 0x84 bytes of arguments, which it removes: first the place of the shared pointer it hands back,
 * sixth the company's id, at the end three short strings by value. When the callback says so for that company no
 * button is made: the pointer is set to null, which the tab takes as "nothing to add", and the arguments are
 * removed here (the strings are short ones and own no memory). */
void *re_action_button;
int (*re_action_button_none)(int company);
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_action_button_hook\n"
        "_re_action_button_hook:\n"
        "  push ecx\n"
        "  push dword ptr [esp + 28]\n" /* the sixth argument: 4 x 6 above the return address, 4 more for the push */
        "  call dword ptr [_re_action_button_none]\n"
        "  add esp, 4\n"
        "  pop ecx\n"
        "  test eax, eax\n"
        "  jnz 1f\n"
        "  jmp dword ptr [_re_action_button]\n"
        "1:\n"
        "  mov eax, [esp + 4]\n"
        "  mov dword ptr [eax], 0\n"
        "  mov dword ptr [eax + 4], 0\n"
        "  ret 0x84\n"
        ".att_syntax prefix\n");

/* Adds a line to the monthly summary panel the way the game's own routines do (0x004cfa3e .. 0x004cfa66): construct
 * the 0x90-byte record, set its text and icon, push the "append" flag, copy the record into the argument area, call
 * the add function (which removes the copy and the flag), destroy the local.
 * fns = {constructor, string assign, copy constructor, add, destructor}. */
void re_game_event_raw(void *owner, const char *text, unsigned len, const char *icon, unsigned icon_len, void *const *fns,
                       int append);
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_game_event_raw\n"
        "_re_game_event_raw:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  sub esp, 0x94\n"
        "  mov esi, esp\n" /* the local record */
        "  mov ebx, [ebp + 28]\n"
        "  mov ecx, esi\n"
        "  call dword ptr [ebx]\n"
        "  mov ecx, esi\n"
        "  push dword ptr [ebp + 16]\n"
        "  push dword ptr [ebp + 12]\n"
        "  call dword ptr [ebx + 4]\n"
        "  lea ecx, [esi + 0x30]\n"
        "  push dword ptr [ebp + 24]\n"
        "  push dword ptr [ebp + 20]\n"
        "  call dword ptr [ebx + 4]\n"
        "  push dword ptr [ebp + 32]\n" /* 1 = append to the month's list (what the game's own callers pass), 0 = put first */
        "  sub esp, 0x90\n"
        "  mov ecx, esp\n"
        "  push esi\n"
        "  call dword ptr [ebx + 8]\n"
        "  mov ecx, [ebp + 8]\n"
        "  call dword ptr [ebx + 12]\n"
        "  mov ecx, esi\n"
        "  call dword ptr [ebx + 16]\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

int re_game_post_event(void *main_obj, const char *text, const char *icon, int first)
{
    BYTE *pfm = pfm_of(main_obj);
    if (pfm == NULL)
        return 0;
    BYTE *owner = *(BYTE **)(pfm + RE_PFM_DEBTINV + RE_DEBTINV_EVENTS);
    if (!re_readable(owner, 0x400))
        return 0;
    void *const fns[5] = {AT(RE_VA_EVENT_CTOR), AT(RE_VA_STR_ASSIGN), AT(RE_VA_EVENT_COPY), AT(RE_VA_EVENT_ADD), AT(RE_VA_EVENT_DTOR)};
    re_game_event_raw(owner, text, (unsigned)strlen(text), icon, (unsigned)strlen(icon), fns, !first);
    return 1;
}

/* The month's lines are a std::list at owner + 0x3c4 (head node pointer, then the count); a node is next, previous
 * and the 0x90-byte record, whose text is the std::string at node + 8. */
int re_game_replace_event(void *main_obj, const char *prefix, const char *text)
{
    typedef void *(__thiscall *assign_fn)(void *self, const char *p, unsigned n);
    BYTE *pfm = pfm_of(main_obj);
    if (pfm == NULL)
        return -1;
    BYTE *owner = *(BYTE **)(pfm + RE_PFM_DEBTINV + RE_DEBTINV_EVENTS);
    if (!re_readable(owner, 0x400))
        return -1;
    BYTE *head = *(BYTE **)(owner + RE_EVENTS_LIST);
    unsigned count = *(unsigned *)(owner + RE_EVENTS_LIST + 4);
    size_t prefix_len = strlen(prefix);
    if (count > 1000 || !re_readable(head, 8))
        return -1;
    BYTE *node = *(BYTE **)head;
    for (unsigned i = 0; i < count && node != head; i++) {
        if (!re_readable(node, 8 + 0x90))
            return -1;
        const BYTE *s = node + 8;
        unsigned size = *(const unsigned *)(s + 0x10), cap = *(const unsigned *)(s + 0x14);
        const char *data = cap >= 16 ? *(const char *const *)s : (const char *)s;
        if (size >= prefix_len && re_readable(data, prefix_len) && memcmp(data, prefix, prefix_len) == 0) {
            if (text != NULL)
                ((assign_fn)AT(RE_VA_STR_ASSIGN))(node + 8, text, (unsigned)strlen(text));
            return 1;
        }
        node = *(BYTE **)node;
    }
    return 0;
}

int re_game_rebuild_summary(void *main_obj)
{
    typedef void(__fastcall *rows_fn)(void *panel);
    if (!re_readable(main_obj, RE_MAIN_UI + 4))
        return 0;
    BYTE *ui = *(BYTE **)((BYTE *)main_obj + RE_MAIN_UI);
    if (!re_readable(ui, RE_UI_SUMMARY + 0x100))
        return 0;
    BYTE *panel = ui + RE_UI_SUMMARY;
    /* the panel of this very game, opened once, and showing */
    if (*(void **)panel != (void *)AT(RE_VA_SUMMARY_VTABLE) || panel[RE_SUMMARY_SHOWN] == 0 || *(void **)(panel + RE_SUMMARY_PAGER) == NULL)
        return 0;
    ((rows_fn)AT(RE_VA_SUMMARY_ROWS))(panel);
    return 1;
}

/* Builds the three std::string arguments of the ticker's message function on the stack the way the game does (empty,
 * then assign through the game's own routine so the memory comes from the game's heap), pushes the type and calls it.
 * The callee removes all 0x4c bytes. The game's own call with plain text: 0x004cfdbe .. 0x004cfdca. */
void re_game_post_raw(void *list, int type, const char *text, unsigned len, void *assign_fn, void *post_fn);
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_game_post_raw\n"
        "_re_game_post_raw:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  sub esp, 72\n"
        "  xor eax, eax\n"
        "  mov [esp], al\n"
        "  mov dword ptr [esp + 16], 0\n"
        "  mov dword ptr [esp + 20], 15\n"
        "  mov [esp + 24], al\n"
        "  mov dword ptr [esp + 40], 0\n"
        "  mov dword ptr [esp + 44], 15\n"
        "  mov [esp + 48], al\n"
        "  mov dword ptr [esp + 64], 0\n"
        "  mov dword ptr [esp + 68], 15\n"
        "  mov ecx, esp\n" /* the text string is the first of the three */
        "  push dword ptr [ebp + 20]\n"
        "  push dword ptr [ebp + 16]\n"
        "  call dword ptr [ebp + 24]\n" /* assign: thiscall, pops its two arguments */
        "  push dword ptr [ebp + 12]\n"
        "  mov ecx, [ebp + 8]\n"
        "  call dword ptr [ebp + 28]\n" /* pops the type and the three strings */
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

int re_game_post_urgent(void *main_obj, int urgent, const char *text)
{
    BYTE *pfm = pfm_of(main_obj);
    if (pfm == NULL)
        return 0;
    BYTE *ui = *(BYTE **)(pfm + RE_PFM_DEBTINV + RE_DEBTINV_UI);
    if (!re_readable(ui, RE_UI_MESSAGES + 0x120))
        return 0;
    re_game_post_raw(ui + RE_UI_MESSAGES, urgent ? 1 : 2, text, (unsigned)strlen(text), AT(RE_VA_STR_ASSIGN), AT(RE_VA_POST_MESSAGE));
    return 1;
}

int re_game_post_message(void *main_obj, const char *text)
{
    return re_game_post_urgent(main_obj, 0, text);
}

long long re_game_ticks(void *main_obj)
{
    if (!re_readable(main_obj, 0x400))
        return -1;
    BYTE *counter = *(BYTE **)((BYTE *)main_obj + RE_MAIN_COUNTER);
    return re_readable(counter, 0x1c) ? *(long long *)counter : -1;
}

int re_game_money_text(void *main_obj, long long cents, char *out, unsigned cap)
{
    typedef void(__thiscall *money_fn)(void *self, void *result, unsigned lo, int hi, int with_symbol, int zero);
    typedef void(__thiscall *free_fn)(void *self);
    BYTE *pfm = pfm_of(main_obj);
    if (pfm == NULL)
        return 0;
    BYTE *format = *(BYTE **)(pfm + RE_PFM_MONEY_FORMAT);
    if (!re_readable(format, 0x90))
        return 0;
    struct {
        union {
            char buf[16];
            char *ptr;
        } u;
        unsigned size, capacity;
    } s = {{{0}}, 0, 15}; /* the game builds its string here */
    ((money_fn)AT(RE_VA_MONEY_TEXT))(format, &s, (unsigned)cents, (int)(cents >> 32), 1, 0);
    const char *text = s.capacity >= 16 ? s.u.ptr : s.u.buf;
    int ok = s.size < cap && re_readable(text, s.size);
    if (ok) {
        memcpy(out, text, s.size);
        out[s.size] = 0;
    }
    ((free_fn)AT(RE_VA_STR_FREE))(&s);
    return ok;
}

int re_game_market_stream(void *main_obj, unsigned **seed, unsigned **calls, void **unit)
{
    if (!re_readable(main_obj, 0x400))
        return 0;
    BYTE *econ = *(BYTE **)((BYTE *)main_obj + RE_MAIN_ECONOMY);
    if (!re_readable(econ, 0xb8))
        return 0;
    BYTE *randgen = *(BYTE **)(econ + RE_ECON_RANDGEN);
    if (!re_readable(randgen, 0x90))
        return 0;
    BYTE *market = randgen + RE_RANDGEN_MARKET;
    *seed = (unsigned *)market;
    *calls = (unsigned *)(market + 4);
    *unit = market;
    return 1;
}

int re_game_next_debt_id(void *debt_manager)
{
    if (!re_readable(debt_manager, 0x10))
        return 0;
    BYTE *owner = *(BYTE **)((BYTE *)debt_manager + RE_DEBTINV_OWNER);
    return re_readable(owner, RE_OWNER_LAST_ID + 4) ? *(int *)(owner + RE_OWNER_LAST_ID) + 1 : 0;
}

float *re_game_debt_rate(void *main_obj, int id)
{
    BYTE *pfm = pfm_of(main_obj);
    rate_ctx c = {id, NULL};
    if (pfm == NULL || map_walk(pfm + RE_PFM_DEBTINV + RE_DEBTINV_DEBTS, 0x50, visit_debt_rate, &c) < 0)
        return NULL;
    return c.rate;
}

float re_game_reserve_rate(void *main_obj)
{
    if (!re_readable(main_obj, 0x400))
        return 0.0f;
    BYTE *econ = *(BYTE **)((BYTE *)main_obj + RE_MAIN_ECONOMY);
    return re_readable(econ, 0xb8) ? *(float *)(econ + RE_ECON_RESERVE_RATE) : 0.0f;
}

/* `whole` 0: the calendar, the playthrough, the cash, the debts and the worth of the homes only. The rest has the
 * game value every asset of the household, three times over, and walk a year of its cash flow: 12 to 14 ms where it
 * was measured (note m30), and the look after a cash movement needs it only when the debts or the homes changed. */
static void snapshot_into(void *main_obj, re_household *h, int whole)
{
    memset(h, 0, sizeof *h);
    h->debt_count = -1;
    if (!re_readable(main_obj, 0x400))
        return;
    BYTE *m = (BYTE *)main_obj;
    BYTE *econ = *(BYTE **)(m + RE_MAIN_ECONOMY);
    BYTE *counter = *(BYTE **)(m + RE_MAIN_COUNTER);
    BYTE *gamedata = *(BYTE **)(m + RE_MAIN_GAMEDATA);

    if (re_readable(counter, 0x1c)) {
        h->uni_count = *(long long *)counter;
        h->tick = *(int *)(counter + 0xc);
        h->month = *(int *)(counter + 0x10);
        h->year = *(int *)(counter + 0x14);
        h->period = h->month + 12 * h->year;
        h->ok_calendar = 1;
    }
    if (re_readable(gamedata, 0x30)) {
        const BYTE *s = gamedata + 0xc; /* a_uniqueID, MSVC std::string */
        unsigned size = *(const unsigned *)(s + 0x10), cap = *(const unsigned *)(s + 0x14);
        const char *text = cap >= 16 ? *(const char *const *)s : (const char *)s;
        if (size < sizeof h->playthrough && re_readable(text, size))
            memcpy(h->playthrough, text, size);
    }
    if (re_readable(econ, 0xb8)) {
        h->growth = *(float *)(econ + RE_ECON_GROWTH);
        h->base_rate = *(float *)(econ + RE_ECON_BASE_RATE);
        h->reserve_rate = *(float *)(econ + RE_ECON_RESERVE_RATE);
        h->ok_economy = 1;
        if (map_walk(econ + 0xb0, 0x18, visit_index, &h->price_index) < 0)
            h->price_index = 0.0;
        unsigned *seed, *calls;
        void *unit;
        if (re_game_market_stream(main_obj, &seed, &calls, &unit)) {
            h->market_seed = *seed;
            h->market_calls = *calls;
        }
    }

    BYTE *pfm = pfm_of(main_obj);
    if (pfm == NULL)
        return;
    if (whole) {
        h->net_worth = ((this_fn0)AT(RE_VA_NET_WORTH))(pfm);
        h->assets_gross = ((this_fn1)AT(RE_VA_TOTAL_ASSETS))(pfm, 0);
        h->assets_net = ((this_fn1)AT(RE_VA_TOTAL_ASSETS))(pfm, 1);
        h->savings = ((this_fn0)AT(RE_VA_SAVINGS))(pfm + RE_PFM_DEBTINV);
        h->investments = ((this_fn0)AT(RE_VA_INVESTMENTS))(pfm + RE_PFM_DEBTINV);
    }
    h->debt = ((this_fn0)AT(RE_VA_DEBT))(pfm + RE_PFM_DEBTINV);
    BYTE *cash_owner = *(BYTE **)(pfm + 8);
    if (re_readable(cash_owner, 0x230))
        h->cash = *(long long *)(cash_owner + 0x228);
    BYTE *restate = *(BYTE **)(pfm + 0x10);
    if (re_readable(restate, 0x40)) {
        h->houses_gross = ((this_fn1)AT(RE_VA_HOUSES))(restate, 0);
        if (whole)
            h->houses_net = ((this_fn1)AT(RE_VA_HOUSES))(restate, 1);
    }
    BYTE *market = *(BYTE **)(pfm + 0x18);
    if (whole && re_readable(market, 0x40))
        h->stocks = ((this_fn0)AT(RE_VA_STOCKS))(market);
    if (whole)
        h->bankrupt_end = ((this_int_fn0)AT(RE_VA_BANKRUPT_END))(pfm);
    h->ok_balance = whole; /* the balance sheet is there only in a whole one */

    h->debt_count = 0;
    debt_ctx dctx = {h, pfm + RE_PFM_DEBTINV};
    if (map_walk(pfm + RE_PFM_DEBTINV + RE_DEBTINV_DEBTS, 0x50, visit_debt, &dctx) < 0) {
        h->debt_count = -1;
        h->debts_due = h->debts_year = 0;
    }
    read_debt(pfm + RE_PFM_DEBTINV + RE_DEBTINV_PENDING_MORTGAGE, &h->pending_mortgage);
    read_debt(pfm + RE_PFM_DEBTINV + RE_DEBTINV_PENDING_EDUCATION, &h->pending_education);

    if (whole && h->ok_calendar) {
        cf_ctx c = {.first = h->period - 12, .last = h->period - 1, .h = h};
        if (map_walk(pfm + RE_PFM_CASHFLOW + 0x20, 0x38, visit_cf_period, &c) < 0 || c.broken) {
            h->cf_months = 0;
            h->cf_tag_count = 0;
            h->cf_income_total = h->cf_expense_total = 0;
        } else {
            h->cf_months = c.months;
        }
    }
}

void re_game_snapshot(void *main_obj, re_household *h)
{
    snapshot_into(main_obj, h, 1);
}

void re_game_snapshot_debts(void *main_obj, re_household *h)
{
    snapshot_into(main_obj, h, 0);
}
