#include "re_data.h"

#include <stdio.h>
#include <string.h>

/* `put` with a run of '#': a number goes there (re_data_number), as wide as the run */
static const struct {
    const char *what, *find, *put;
} FIXES[RE_DATA_COUNT] = {
    [RE_DATA_CONTRACT_ENGINEER] = {"Building Construction: the contract \"Supply Construction Site\" (35081) asks for its 60 hours of a "
                                   "civil engineer (job 32391)",
                                   "<contractJobGen id=\"32391\" amt=\"60\" cond=\"32391\"/>",
                                   "<contractJobGen id=\"32391\" amt=\"60\" cond=\"35081\"/>"},
    [RE_DATA_STOCK_FEE] = {"the fee of a purchase or a sale of shares, a part of the price", "stockTradingFee=\"0.01\"",
                           "stockTradingFee=\"####\""},
    [RE_DATA_FUTURES_FEE] = {"the fee of a futures contract, a part of its margin", "futuresTradingFee=\"0.01\"", "futuresTradingFee=\"####\""},
    [RE_DATA_SAVINGS_RATE] = {"the savings account's rate, times the central-bank rate", "instrument=\"5\" interestXer=\"0.5\"",
                              "instrument=\"5\" interestXer=\"###\""},
    [RE_DATA_HYDROPONICS] = {"Hydroponics Farm: the income of an hour of its delivery driver, so that the type earns what the others do",
                             "<incomeGen id=\"30601\" amt=\"63000\" prob=\"0.85\"/>", "<incomeGen id=\"30601\" amt=\"73800\" prob=\"0.85\"/>"},
    [RE_DATA_COMMUNICATIONS] = {"the degree Communications: its price, for the wages of the jobs it opens",
                                "<study months=\"36\" hours=\"120\" money=\"-9000000\" allowPartTime=\"1\">",
                                "<study months=\"36\" hours=\"120\" money=\"-4500000\" allowPartTime=\"1\">"},
    [RE_DATA_DEGREE_COMPUTING] = {"the degree Computer Science: its price, a year of the pay of the jobs it opens",
                                  "<study months=\"36\" hours=\"120\" money=\"-11300000\" allowPartTime=\"1\">",
                                  "<study months=\"36\" hours=\"120\" money=\"-5380000\"  allowPartTime=\"1\">"},
    [RE_DATA_DEGREE_ART] = {"the degree Art and Design: its price, a year of the pay of the jobs it opens",
                            "<study months=\"36\" hours=\"120\" money=\"-10730000\" allowPartTime=\"1\">",
                            "<study months=\"36\" hours=\"120\" money=\"-5120000\"  allowPartTime=\"1\">"},
    [RE_DATA_DEGREE_SCIENCE] = {"the degree Science and Engineering: its price, a year of the pay of the jobs it opens",
                                "<study months=\"36\" hours=\"120\" money=\"-11500000\" allowPartTime=\"1\">",
                                "<study months=\"36\" hours=\"120\" money=\"-6300000\"  allowPartTime=\"1\">"},
    [RE_DATA_DEGREE_BUSINESS] = {"the degree Business and Finance: its price, a year of the pay of the jobs it opens",
                                 "<study months=\"36\" hours=\"120\" money=\"-12500000\" allowPartTime=\"1\">",
                                 "<study months=\"36\" hours=\"120\" money=\"-5650000\"  allowPartTime=\"1\">"},
};

static char g_put[RE_DATA_COUNT][64]; /* a `put` with its number in */

const char *re_data_put(int correction)
{
    return g_put[correction][0] ? g_put[correction] : FIXES[correction].put;
}

int re_data_number(int correction, double value)
{
    const char *put = FIXES[correction].put, *slot = strchr(put, '#');
    size_t width = slot != NULL ? strspn(slot, "#") : 0;
    char number[24];
    if (width < 3 || strlen(put) >= sizeof g_put[correction] || !(value >= 0.0) || value >= 10.0)
        return 0;
    if (snprintf(number, sizeof number, "%.*f", (int)width - 2, value) != (int)width)
        return 0; /* 9.96 with one decimal is "10.0" */
    strcpy(g_put[correction], put);
    memcpy(g_put[correction] + (slot - put), number, width);
    return 1;
}

unsigned re_data_correct(char *xml, const int *on)
{
    unsigned made = 0;
    for (int i = 0; i < RE_DATA_COUNT; i++) {
        const char *put = re_data_put(i);
        size_t len = strlen(FIXES[i].find);
        if (!on[i] || strlen(put) != len || strchr(put, '#') != NULL || strcmp(put, FIXES[i].find) == 0)
            continue;
        for (char *at = strstr(xml, FIXES[i].find); at != NULL; at = strstr(at + len, FIXES[i].find)) {
            memcpy(at, put, len);
            made |= 1u << i;
        }
    }
    return made;
}

const char *re_data_what(int correction)
{
    return FIXES[correction].what;
}

const char *re_data_find(int correction)
{
    return FIXES[correction].find;
}
