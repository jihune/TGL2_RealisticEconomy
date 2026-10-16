/* Corrections of the game's data files (the XML files under data). The game reads a file whole and hands the text to its XML
 * reader; a correction replaces one piece of that text before it is read, by a piece of the same length.
 *
 * A piece is replaced only where the file has exactly the game's known text, so a file that somebody else has
 * changed (a data mod, a later version of the game) is left alone. Pure string work: no game access here. */
#ifndef RE_DATA_H
#define RE_DATA_H

enum {
    /* Building Construction's contract "Supply Construction Site" (35081): its 60 hours of a civil engineer hang on
     * the id of the job itself where the id of the contract belongs, so no contract ever asks for them (note m44) */
    RE_DATA_CONTRACT_ENGINEER,
    /* numbers of the game's balance (note m48): each is given with re_data_number before it is used */
    RE_DATA_STOCK_FEE,    /* the fee of a purchase or a sale of shares, a part of the price: 0.01 in the game */
    RE_DATA_FUTURES_FEE,  /* the fee of a futures contract, a part of its margin: 0.01 in the game */
    RE_DATA_SAVINGS_RATE, /* the savings account's rate, times the central-bank rate: 0.5 in the game */
    /* Hydroponics Farm: the income an hour of its delivery driver brings, 63000 cents in the game. With it the type's
     * income is 119% of its cost where the other 46 are at 136 to 142% (the game's own data/statsFirm.txt); 73800
     * makes it 139.7%, their median */
    RE_DATA_HYDROPONICS,
    /* the degree Communications: $90,000 in the game, and the four jobs that ask for it pay $20.15 to $26.35 an hour
     * (data/jobs/careerCommunications.xml; note e13); $45,000 */
    RE_DATA_COMMUNICATIONS,
    /* four degrees that cost more than 21 months of the pay of the jobs they open, where 25 of the other 30 courses
     * with jobs cost 13 months or less (note e15): each at a year of that pay. A price one digit shorter than the
     * game's is followed by one space more, which the game's reader takes between attributes (data/misc/
     * defaultData.xml has two) */
    RE_DATA_DEGREE_COMPUTING, /* Computer Science: $113,000 in the game; $53,800 */
    RE_DATA_DEGREE_ART,       /* Art and Design: $107,300; $51,200 */
    RE_DATA_DEGREE_SCIENCE,   /* Science and Engineering: $115,000; $63,000 */
    RE_DATA_DEGREE_BUSINESS,  /* Business and Finance: $125,000; $56,500 */
    RE_DATA_COUNT
};

/* Corrects the NUL-terminated text of one file in place. `on[i]` says whether correction i is wanted. Returns one
 * bit for each correction made (bit i). A correction whose number is not given, or is the game's, is never made. */
unsigned re_data_correct(char *xml, const int *on);

/* The number a correction puts where the game's file has its own. It is written with as many decimals as the game's
 * (0.02, 0.7). Returns 0 when the correction takes no number or the number does not fit (below 0, 10 or more). */
int re_data_number(int correction, double value);

/* for the log and the test */
const char *re_data_what(int correction);
const char *re_data_find(int correction);
const char *re_data_put(int correction);

#endif
