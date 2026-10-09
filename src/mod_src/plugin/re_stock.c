#include "re_stock.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "re_lang.h"
#include "re_sites.h"

void re_stock_read(const unsigned char *company, re_stock *out)
{
    out->price = *(const long long *)(company + RE_COMPANY_PRICE);
    out->dividend = *(const long long *)(company + RE_COMPANY_DIVIDEND);
    out->earnings = *(const long long *)(company + RE_COMPANY_EARNINGS);
    out->equity = *(const long long *)(company + RE_COMPANY_EQUITY);
    out->research_month = *(const int *)(company + RE_COMPANY_RESEARCH_MONTH);
    out->desperate_month = *(const int *)(company + RE_COMPANY_DESPERATE_MONTH);
}

void re_stock_view_of(const re_stock *s, int now_month, re_stock_view *v)
{
    double price = (double)s->price, earnings = (double)s->earnings, equity = (double)s->equity;
    memset(v, 0, sizeof *v);
    v->months_since = now_month - s->desperate_month;
    v->below_line = s->price <= 0 || s->equity <= 0 || price < RE_STOCK_LINE * equity;
    if (s->price > 0) {
        v->has_yield = 1;
        v->yield = (double)s->dividend / price;
        v->has_per = s->earnings > 0;
        if (v->has_per)
            v->per = price / earnings;
    }
    if (s->equity <= 0)
        return;
    v->roe = earnings / equity;
    /* the monthly price step: weight on equity 0.8 - 3 x return on equity, kept within 0.4 and 0.8 */
    double w = 0.8 - 3.0 * v->roe;
    w = w < 0.4 ? 0.4 : w > 0.8 ? 0.8 : w;
    double target = w * equity + (1.0 - w) * 20.0 * earnings;
    v->target = target > 0.0 ? (long long)(target + 0.5) : 0;
    if (s->price <= 0)
        return;
    v->has_pbr = 1;
    v->pbr = price / equity;
    v->gap = ((double)v->target - price) / price;
    if (!v->below_line)
        v->to_line = (price - RE_STOCK_LINE * equity) / price;
}

static int rank_before(const re_stock_rank *a, const re_stock_rank *b, int by)
{
    if (by == RE_STOCK_BY_CAP)
        return a->cap > b->cap;
    if (by == RE_STOCK_BY_CHANGE)
        return a->has_change && (!b->has_change || a->change > b->change);
    if (!a->researched || !b->researched)
        return a->researched && !b->researched;
    return by == RE_STOCK_BY_CHEAP ? a->gap > b->gap : a->gap < b->gap;
}

void re_stock_order(const re_stock_rank *rows, int n, int by, int *order)
{
    for (int i = 0; i < n; i++) { /* an insertion sort: a row passes only the rows it is before */
        int at = i;
        while (at > 0 && rank_before(&rows[i], &rows[order[at - 1]], by)) {
            order[at] = order[at - 1];
            at--;
        }
        order[at] = i;
    }
}

/* course in the bits from 48 up, the last month paid in bits 24 to 47, and under them one bit a month counted back
 * from it: bit 0 that month itself */
#define PAID_MASK ((1LL << RE_STOCK_PAID_MONTHS) - 1)

long long re_stock_paid_add(long long record, int course, int month)
{
    long long last = (record >> RE_STOCK_PAID_MONTHS) & PAID_MASK, months = record & PAID_MASK;
    if (record == 0 || (int)(record >> 48) != course) {
        last = month;
        months = 1;
    } else if (month > last) {
        months = month - last >= RE_STOCK_PAID_MONTHS ? 1 : ((months << (month - last)) | 1) & PAID_MASK;
        last = month;
    } else if (last - month < RE_STOCK_PAID_MONTHS)
        months |= 1LL << (last - month);
    return ((long long)course << 48) | (last << RE_STOCK_PAID_MONTHS) | months;
}

int re_stock_paid_has(long long record, int course, int month)
{
    long long back = ((record >> RE_STOCK_PAID_MONTHS) & PAID_MASK) - month;
    return record != 0 && (int)(record >> 48) == course && back >= 0 && back < RE_STOCK_PAID_MONTHS && ((record >> back) & 1);
}

typedef struct {
    char *out;
    unsigned cap, len;
    int lines, failed;
} writer;

static void put(writer *w, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int n = w->failed ? -1 : vsnprintf(w->out + w->len, w->cap - w->len, format, args);
    va_end(args);
    if (n < 0 || (unsigned)n >= w->cap - w->len)
        w->failed = 1;
    else
        w->len += (unsigned)n;
}

static void line(writer *w)
{
    if (w->lines++)
        put(w, "\n");
}

static unsigned done(const writer *w)
{
    if (w->failed && w->cap)
        w->out[0] = 0;
    return w->failed ? 0 : w->len;
}

/* month ends at which the price under the line means the end of the company; none when this is not positive */
static int grace_left(const re_stock_view *v)
{
    return RE_STOCK_GRACE_MONTHS - v->months_since;
}

#define T(msg) re_lang_text(lang, msg)

unsigned re_stock_item(const re_stock_view *v, int researched, int kept, int lang, char *out, unsigned cap)
{
    writer w = {out, cap, 0, 0, cap == 0};
    int parts = 0;
    if (v->has_change)
        put(&w, "%s%+.1f%%", parts++ ? ", " : " (", 100.0 * v->change);
    if (researched && v->has_pbr) {
        put(&w, "%s", parts++ ? ", " : " (");
        put(&w, T(RE_MSG_ITEM_FAIR), 100.0 * v->gap);
    }
    if (kept != RE_STOCK_KEPT_NOT)
        put(&w, "%s%s", parts++ ? ", " : " (", T(kept == RE_STOCK_KEPT_BOARD ? RE_MSG_ITEM_BOARD : RE_MSG_ITEM_SUB));
    if (parts)
        put(&w, ")");
    return done(&w);
}

unsigned re_stock_text(const re_stock_view *v, int lang, const char *target_text, int has_industries, double industries, int red,
                       char *out, unsigned cap)
{
    writer w = {out, cap, 0, 0, cap == 0};
    const char *on = red ? "@RED@" : "", *off = red ? "@@" : "";
    int left = grace_left(v);

    /* the game's hover box is about 34 columns wide and a Korean letter takes two: one short line each */
    line(&w);
    if (v->has_per)
        put(&w, v->per < 100.0 ? "PER %.1f" : "PER %.0f", v->per);
    else
        put(&w, "%s", T(RE_MSG_PER_LOSS));
    if (v->has_pbr)
        put(&w, ", PBR %.2f", v->pbr);
    if (v->has_yield)
        put(&w, T(RE_MSG_HOVER_YIELD), 100.0 * v->yield);

    if (v->has_change) {
        line(&w);
        put(&w, T(RE_MSG_HOVER_CHANGE), 100.0 * v->change);
    }

    if (v->below_line) {
        line(&w);
        put(&w, T(left > 0 ? RE_MSG_HOVER_BANKRUPT : RE_MSG_HOVER_MEASURE), on, v->has_pbr ? T(RE_MSG_HOVER_UNDER) : "", off);
    } else if (left > 0) {
        line(&w);
        put(&w, T(RE_MSG_HOVER_WATCH), on, left, 100.0 * v->to_line, off); /* far from the line too: the player sees how far */
    } else if (v->to_line <= RE_STOCK_NEAR) {
        line(&w);
        put(&w, T(RE_MSG_HOVER_NEAR), 100.0 * v->to_line);
    }

    if (target_text != NULL) {
        line(&w);
        if (v->has_pbr)
            put(&w, T(RE_MSG_HOVER_FAIR), target_text, 100.0 * v->gap);
        else
            put(&w, "%s", T(RE_MSG_NO_FAIR));
        /* a word, not a number: the number is in the company's tab, industry by industry */
        if (has_industries && fabs(industries) >= RE_STOCK_INDUSTRY_SIGN / 2.0)
            put(&w, "%s", T(industries > 0.0 ? RE_MSG_HOVER_TAILWIND : RE_MSG_HOVER_HEADWIND));
    }
    return done(&w);
}

unsigned re_stock_row(const re_stock_view *v, int row, int lang, const char *target_text, int *red, char *out, unsigned cap)
{
    writer w = {out, cap, 0, 0, cap == 0};
    int left = grace_left(v);
    *red = 0;
    /* the window shrinks a value that is wider than about fifteen Korean letters: keep these short */
    switch (row) {
    case RE_STOCK_ROW_PRICE:
        if (target_text != NULL && v->has_pbr && v->has_change)
            put(&w, T(RE_MSG_ROW_CHANGE_FAIR), 100.0 * v->change, target_text);
        else if (target_text != NULL && v->has_pbr)
            put(&w, T(RE_MSG_ROW_FAIR), target_text);
        else if (v->has_change)
            put(&w, T(RE_MSG_ROW_CHANGE), 100.0 * v->change);
        break;
    case RE_STOCK_ROW_EQUITY:
        *red = v->below_line || left > 0;
        put(&w, " (");
        if (v->has_pbr)
            put(&w, *red ? "PBR %.2f, " : "PBR %.2f", v->pbr);
        if (v->below_line)
            put(&w, "%s", T(left > 0 ? RE_MSG_ROW_BANKRUPT : RE_MSG_ROW_MEASURE));
        else if (left > 0)
            put(&w, "%s", T(RE_MSG_ROW_WATCH));
        put(&w, ")");
        break;
    case RE_STOCK_ROW_EARNINGS:
        if (v->has_per)
            put(&w, v->per < 100.0 ? " (PER %.1f)" : " (PER %.0f)", v->per);
        else if (v->roe != 0.0)
            put(&w, T(RE_MSG_ROW_OF_EQUITY), 100.0 * v->roe);
        break;
    case RE_STOCK_ROW_DIVIDEND:
        if (v->has_yield)
            put(&w, T(RE_MSG_ROW_OF_PRICE), 100.0 * v->yield);
        break;
    }
    return done(&w);
}

double re_stock_industry_term(double rate, double weight)
{
    return rate * weight * RE_STOCK_INDUSTRY_FACTOR;
}

unsigned re_stock_industry_row(double term, int lang, const char *per_month, char *out, unsigned cap)
{
    writer w = {out, cap, 0, 0, cap == 0};
    if (term == 0.0)
        return 0;
    /* the game rounds a part of a sign up to a whole one; ten are as wide as the row's value may get */
    int signs = (int)ceil(fabs(term) / RE_STOCK_INDUSTRY_SIGN);
    for (int i = 0; i < signs && i < 10; i++)
        put(&w, term > 0.0 ? "+" : "-");
    if (per_month != NULL)
        put(&w, T(RE_MSG_ROW_PER_MONTH), per_month);
    return done(&w);
}

unsigned re_stock_held(const re_stock_view *v, int lang, const char *name, int *level, char *out, unsigned cap)
{
    writer w = {out, cap, 0, 0, cap == 0};
    int left = grace_left(v);
    if (v->below_line)
        *level = left > 0 ? RE_STOCK_BANKRUPT_DUE : RE_STOCK_MEASURE_DUE;
    else if (left > 0 && v->to_line <= RE_STOCK_NEAR)
        *level = RE_STOCK_WATCH;
    else if (left <= 0 && v->to_line <= RE_STOCK_HELD_NEAR)
        *level = RE_STOCK_CLOSE;
    else
        *level = RE_STOCK_CALM;
    if (*level == RE_STOCK_CALM)
        return 0;
    put(&w, "%s (", name);
    if (*level == RE_STOCK_BANKRUPT_DUE || *level == RE_STOCK_MEASURE_DUE)
        put(&w, "%s", T(*level == RE_STOCK_BANKRUPT_DUE ? RE_MSG_ROW_BANKRUPT : RE_MSG_ROW_MEASURE));
    else
        put(&w, T(*level == RE_STOCK_WATCH ? RE_MSG_HELD_WATCH : RE_MSG_HELD_CLOSE), 100.0 * v->to_line);
    put(&w, ")");
    return done(&w);
}
