/* Tests the credit grade arithmetic. No game, no Windows API.
 *
 *   credit_test.exe [--control]
 *
 * Each case names the grade and the spreads it must produce. --control shifts every expected grade by one,
 * so the verdict must then be FAIL.
 */
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../plugin/re_credit.h"

static int g_failures, g_control;

static void expect(const char *what, const re_credit_inputs *in, int grade, int score12, double personal, double mortgage)
{
    re_credit_config cfg;
    re_credit_result r;
    re_credit_defaults(&cfg);
    re_credit_evaluate(in, &cfg, &r);
    if (g_control)
        grade = (grade + 1) % RE_GRADE_COUNT;
    int ok = r.grade == grade && r.score12 == score12 && fabs(r.personal_spread - personal) < 1e-9 &&
             fabs(r.mortgage_spread - mortgage) < 1e-9;
    g_failures += !ok;
    printf("  %-58s grade=%-3s score=%2d points=%d,%d,%d,%d personal=+%.2f%% mortgage=+%.2f%% %s\n", what,
           re_credit_grade_name(r.grade), r.score12, r.points[0], r.points[1], r.points[2], r.points[3], 100 * r.personal_spread,
           100 * r.mortgage_spread, ok ? "ok" : "FAIL");
}

int main(int argc, char **argv)
{
    g_control = argc > 1 && strcmp(argv[1], "--control") == 0;

    /* the user's save "55" as the plugin read it in the game on 2026-10-04 (analysis/out/run2/RealisticEconomy_a.log) */
    re_credit_inputs save55 = {.net_worth = 15227830.71, .assets = 23076581.00, .debt = 7848750.29, .liquid = 8159320.06,
                               .houses = 4725958.28, .mortgage_balance = 3264142.92, .price_index = 1.857871771,
                               .history_months = 12, .annual_income = 1394226.08, .annual_expenses = 3686445.86,
                               .annual_debt_service = 1999136.88};
    expect("save 55: other debt 25%, DSR 143%, 2.2 years liquid, $15.2M", &save55, RE_GRADE_A, 8, 0.05, 0.018 + 0.0060);

    re_credit_inputs earning = save55;
    earning.annual_income = 5400000.0; /* DSR 37%: one point there, nine in all */
    expect("the same household earning $5.4M a year", &earning, RE_GRADE_AA, 9, 0.04, 0.015 + 0.0060);

    re_credit_inputs no_history = save55;
    no_history.history_months = 0;
    expect("save 55 without a cash-flow record (2 ratios, 5 of 6)", &no_history, RE_GRADE_AA, 10, 0.04, 0.015 + 0.0060);

    re_credit_inputs best = {.net_worth = 20e6, .assets = 20e6, .debt = 0, .liquid = 5e6, .price_index = 1.0, .history_months = 12,
                             .annual_income = 1e6, .annual_expenses = 5e5};
    expect("no debt, $20M, 10 years liquid", &best, RE_GRADE_AAA, 12, 0.03, 0.012);

    re_credit_inputs worst = {.net_worth = -5e4, .assets = 1e5, .debt = 1.5e5, .liquid = 1e3, .houses = 1e5, .mortgage_balance = 9.5e4,
                              .price_index = 1.0, .history_months = 12, .annual_income = 3e4, .annual_expenses = 4e4,
                              .annual_debt_service = 2e4};
    expect("debt above assets, DSR 67%, LTV 95%", &worst, RE_GRADE_B, 0, 0.14, 0.040 + 0.0200);

    re_credit_inputs starter = {.net_worth = 2e4, .assets = 2e4, .debt = 0, .liquid = 2e4, .price_index = 1.0, .history_months = 6,
                                .annual_income = 4e4, .annual_expenses = 3e4};
    expect("new character: $20k, no debt, 8 months liquid", &starter, RE_GRADE_A, 8, 0.05, 0.018);

    re_credit_inputs no_income = {.net_worth = 2e6, .assets = 3e6, .debt = 1e6, .liquid = 1e5, .price_index = 1.0, .history_months = 12,
                                  .annual_income = 0, .annual_expenses = 4e5, .annual_debt_service = 1.2e5};
    expect("payments due and no income", &no_income, RE_GRADE_BBB, 5, 0.07, 0.022);

    /* a first home at 80% loan-to-value: the mortgage counts through its payments and its LTV, not as other debt */
    re_credit_inputs buyer = {.net_worth = 9e4, .assets = 3.3e5, .debt = 2.4e5, .liquid = 3e4, .houses = 3e5, .mortgage_balance = 2.4e5,
                              .price_index = 1.0, .history_months = 12, .annual_income = 8e4, .annual_expenses = 6.11e4,
                              .annual_debt_service = 2.11e4};
    expect("first home at 80% LTV, payments 26% of income", &buyer, RE_GRADE_A, 7, 0.05, 0.018 + 0.0120);

    /* the grade moves after a loan: the same buyer borrows $60k unsecured */
    re_credit_inputs borrowed = buyer;
    borrowed.assets = 3.9e5;
    borrowed.debt = 3.0e5;
    borrowed.liquid = 9e4;
    borrowed.annual_debt_service = 3.94e4;
    borrowed.annual_expenses = 7.94e4;
    expect("the same buyer after a $60k personal loan", &borrowed, RE_GRADE_BBB, 5, 0.07, 0.022 + 0.0120);

    re_credit_inputs indexed = {.net_worth = 1.5e6, .assets = 1.5e6, .debt = 0, .liquid = 1.5e6, .price_index = 2.0, .history_months = 12,
                                .annual_income = 1e5, .annual_expenses = 5e4};
    expect("$1.5M at price index 2.0 counts as below $1M", &indexed, RE_GRADE_AA, 10, 0.04, 0.015);

    re_credit_inputs ltv_edge = best;
    ltv_edge.houses = 1e6;
    ltv_edge.mortgage_balance = 4e5;
    expect("LTV exactly 40% takes the lowest band", &ltv_edge, RE_GRADE_AAA, 12, 0.03, 0.012);

    printf("failures=%d control=%d verdict=%s\n", g_failures, g_control, g_failures == 0 ? "PASS" : "FAIL");
    return g_failures != 0;
}
