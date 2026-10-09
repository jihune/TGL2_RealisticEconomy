/* Credit grade of the household and the loan spreads that follow from it. Pure arithmetic: no game access here.
 *
 * Four ratios score 0..3 points each (INVESTIGATION_REPORT.md 12.3). A ratio whose input is unknown is left out and
 * the total is rescaled to 12, so a grade is still produced and the log says which ratios counted. */
#ifndef RE_CREDIT_H
#define RE_CREDIT_H

enum { RE_GRADE_AAA, RE_GRADE_AA, RE_GRADE_A, RE_GRADE_BBB, RE_GRADE_BB, RE_GRADE_B, RE_GRADE_COUNT };
enum { RE_RATIO_DEBT_ASSETS, RE_RATIO_SERVICE_INCOME, RE_RATIO_LIQUID_YEARS, RE_RATIO_NET_WORTH, RE_RATIO_COUNT };

typedef struct {
    double net_worth, assets, debt, liquid; /* dollars */
    double houses, mortgage_balance;        /* dollars; LTV = mortgage_balance / houses */
    double price_index;                     /* the game's inflation index; scales the net worth bands */
    int history_months;                     /* months behind the three annual figures; 0 = unknown */
    double annual_income, annual_expenses, annual_debt_service;
} re_credit_inputs;

typedef struct {
    double debt_assets_max[3];    /* debt other than mortgages / assets other than homes: at most this for 3, 2, 1 points */
    double service_income_max[3]; /* at most this for 3, 2, 1 points */
    double liquid_years_min[3];   /* at least this for 3, 2, 1 points */
    double net_worth_min[3];      /* at least this (times price_index) for 3, 2, 1 points */
    int index_net_worth;          /* 0 = ignore price_index */
    double personal_spread[RE_GRADE_COUNT];
    double mortgage_spread[RE_GRADE_COUNT];
    double ltv_bound[4];
    double ltv_spread[5];
} re_credit_config;

typedef struct {
    int known[RE_RATIO_COUNT];
    int points[RE_RATIO_COUNT];
    double ratio[RE_RATIO_COUNT];
    int total, counted; /* points and how many ratios counted */
    int score12;        /* total rescaled to 0..12 */
    int grade;
    double ltv;
    double personal_spread; /* added to the central-bank rate */
    double mortgage_spread; /* grade spread plus LTV spread */
} re_credit_result;

void re_credit_defaults(re_credit_config *cfg);
void re_credit_evaluate(const re_credit_inputs *in, const re_credit_config *cfg, re_credit_result *out);
/* The two spreads a given grade and LTV would get; used to price the debts at an assumed grade. */
void re_credit_spreads(const re_credit_config *cfg, int grade, double ltv, double *personal, double *mortgage);
const char *re_credit_grade_name(int grade);

#endif
