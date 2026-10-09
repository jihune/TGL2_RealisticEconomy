#include "re_credit.h"

#include <string.h>

static const char *const GRADE_NAMES[RE_GRADE_COUNT] = {"AAA", "AA", "A", "BBB", "BB", "B"};

const char *re_credit_grade_name(int grade)
{
    return grade >= 0 && grade < RE_GRADE_COUNT ? GRADE_NAMES[grade] : "?";
}

void re_credit_defaults(re_credit_config *cfg)
{
    static const re_credit_config defaults = {
        .debt_assets_max = {0.20, 0.40, 0.60},
        /* lenders' usual debt-to-income lines: comfortable, acceptable, stretched */
        .service_income_max = {0.20, 0.35, 0.50},
        /* a year, six months, three months of spending in reserve */
        .liquid_years_min = {1.0, 0.5, 0.25},
        /* $50,000 is the game's priceIncreaseMinimumWealth, $1,000,000 its influence unlockThreshold,
         * $6,000,000 where the second tier of rivals starts (FUN_00487110) */
        .net_worth_min = {6000000.0, 1000000.0, 50000.0},
        .index_net_worth = 1,
        .personal_spread = {0.03, 0.04, 0.05, 0.07, 0.10, 0.14},
        .mortgage_spread = {0.012, 0.015, 0.018, 0.022, 0.030, 0.040},
        .ltv_bound = {0.40, 0.60, 0.75, 0.85},
        .ltv_spread = {0.0, 0.0025, 0.0060, 0.0120, 0.0200},
    };
    *cfg = defaults;
}

/* smaller is better: 3 points at or below max[0] */
static int points_at_most(double value, const double max[3])
{
    for (int i = 0; i < 3; i++)
        if (value <= max[i])
            return 3 - i;
    return 0;
}

/* larger is better: 3 points at or above min[0] */
static int points_at_least(double value, const double min[3])
{
    for (int i = 0; i < 3; i++)
        if (value >= min[i])
            return 3 - i;
    return 0;
}

void re_credit_evaluate(const re_credit_inputs *in, const re_credit_config *cfg, re_credit_result *out)
{
    memset(out, 0, sizeof *out);

    /* Debt that no home secures, against the assets other than homes. A mortgage is priced by its loan-to-value
     * and weighs on the grade through its payments, so it is not counted a third time here. No such debt is a clean
     * sheet; such debt with nothing to set against it is the worst case. */
    double unsecured = in->debt - in->mortgage_balance, other_assets = in->assets - in->houses;
    out->known[RE_RATIO_DEBT_ASSETS] = 1;
    if (unsecured <= 0.0) {
        out->points[RE_RATIO_DEBT_ASSETS] = 3;
    } else if (other_assets > 0.0) {
        out->ratio[RE_RATIO_DEBT_ASSETS] = unsecured / other_assets;
        out->points[RE_RATIO_DEBT_ASSETS] = points_at_most(out->ratio[RE_RATIO_DEBT_ASSETS], cfg->debt_assets_max);
    }

    if (in->history_months > 0) {
        out->known[RE_RATIO_SERVICE_INCOME] = 1;
        if (in->annual_debt_service <= 0.0) {
            out->points[RE_RATIO_SERVICE_INCOME] = 3;
        } else if (in->annual_income > 0.0) {
            out->ratio[RE_RATIO_SERVICE_INCOME] = in->annual_debt_service / in->annual_income;
            out->points[RE_RATIO_SERVICE_INCOME] = points_at_most(out->ratio[RE_RATIO_SERVICE_INCOME], cfg->service_income_max);
        } /* payments due and no income: 0 points */

        out->known[RE_RATIO_LIQUID_YEARS] = 1;
        if (in->annual_expenses > 0.0) {
            out->ratio[RE_RATIO_LIQUID_YEARS] = in->liquid / in->annual_expenses;
            out->points[RE_RATIO_LIQUID_YEARS] = points_at_least(out->ratio[RE_RATIO_LIQUID_YEARS], cfg->liquid_years_min);
        } else {
            out->points[RE_RATIO_LIQUID_YEARS] = in->liquid > 0.0 ? 3 : 0;
        }
    }

    double index = cfg->index_net_worth && in->price_index > 0.0 ? in->price_index : 1.0;
    double bands[3] = {cfg->net_worth_min[0] * index, cfg->net_worth_min[1] * index, cfg->net_worth_min[2] * index};
    out->known[RE_RATIO_NET_WORTH] = 1;
    out->ratio[RE_RATIO_NET_WORTH] = in->net_worth;
    out->points[RE_RATIO_NET_WORTH] = points_at_least(in->net_worth, bands);

    for (int i = 0; i < RE_RATIO_COUNT; i++) {
        if (out->known[i]) {
            out->total += out->points[i];
            out->counted++;
        }
    }
    /* rescale to the 12-point scale, rounding to nearest */
    out->score12 = (out->total * 12 * 2 + out->counted * 3) / (out->counted * 3 * 2);
    static const int floor_of_grade[RE_GRADE_COUNT] = {11, 9, 7, 5, 3, 0};
    for (out->grade = 0; out->score12 < floor_of_grade[out->grade]; out->grade++)
        ;

    if (in->houses > 0.0)
        out->ltv = in->mortgage_balance / in->houses;
    re_credit_spreads(cfg, out->grade, out->ltv, &out->personal_spread, &out->mortgage_spread);
}

void re_credit_spreads(const re_credit_config *cfg, int grade, double ltv, double *personal, double *mortgage)
{
    int band = 0;
    while (band < 4 && ltv > cfg->ltv_bound[band])
        band++;
    *personal = cfg->personal_spread[grade];
    *mortgage = cfg->mortgage_spread[grade] + cfg->ltv_spread[band];
}
