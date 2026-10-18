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
    /* the city's vote on its policies every 12 months, where the game has 24 (REQUEST.md [76], note e17 B4) */
    RE_DATA_VOTE_INTERVAL,
    RE_DATA_COUNT
};

/* Additions: elements the game reads as if its files had them (REQUEST.md [76]). A file is known by pieces of text
 * only it has; the elements go after a piece of it, or before its last "</xml>". The game's English is in the data
 * files, the other languages in their own overlay files, each known by the game's name there of an element next to
 * the added ones. A file that has an added element's id already is left alone. */
enum {
    /* "Fund A Think Tank" ($1M) and "Fund A Political Party" ($10M): the donation to a political group of the
     * passion Lobbying ($100,000), at a bank, with as many points a dollar: about 1,000 and 10,000 a month for 12
     * months, where the game's gives 100. Once a year each */
    RE_ADD_DONATIONS,
    /* "Personal Coaching: <preference>" at a hospital below "Mental Therapy", one activity for each of the 18
     * preferences (re_data_coaching.h, REQUEST.md [79], [80]): $500,000, 20 hours, once a year for each person and
     * preference in the data; when it ends the plugin moves that preference one step towards a like, takes the price
     * once more from a dislike to a like and makes it one preference a year for the person (re_casino.h,
     * RE_COACHING_FIRST) */
    RE_ADD_COACHING,
    /* Korean texts of the game that are wrong or read strangely, from the proposals of note e19 (REQUEST.md [76]):
     * pieces of text in the Korean language files, of any length (re_data_ko.h) */
    RE_ADD_KOREAN,
    /* the words for how much a person likes something, where they say something else (REQUEST.md [79]): Spanish and
     * Portuguese "Como" and French "Comme" for a like ("as"), Spanish "Añadir" ("to add") for an addiction, Portuguese
     * "Não Gostei" ("I did not like it") for a dislike; Japanese has like and love one step off (楽しむ, 好き) */
    RE_ADD_LEVEL_WORDS,
    /* the game's donation to a political group (81451) once a year for the household, where the game has once a month:
     * a donation while its status runs only starts the status's 12 months again (run1697), so a donation a month
     * paid twelve times for one year's points (REQUEST.md [79]) */
    RE_ADD_DONATION_YEARLY,
    RE_ADD_COUNT
};

#define RE_DATA_REPLACE_FILES 24 /* at most this many files get pieces of text in the place of others */

/* The text of one file with the wanted additions made in it, in memory from `alloc` (NUL-terminated), or NULL when
 * none of them belongs to this file. `on[g]` says whether addition g is wanted. An addition is made of rows, one a
 * file, of elements put in (`made` gets one bit for each row made, bit r), or of pieces of text put in the place of
 * others in a file (`replaced[f]` gets one for each piece put in file f). */
char *re_data_add(const char *xml, const int *on, void *(*alloc)(unsigned size), unsigned *made, int replaced[RE_DATA_REPLACE_FILES]);
/* for the log and the test: the rows, the addition of a row, the piece of text its file is known by */
int re_data_add_rows(void);
int re_data_add_of(int row);
const char *re_data_add_file(int row);
const char *re_data_add_what(int addition);
/* the same for the files that get pieces of text in the place of others, and the number of pieces of each */
int re_data_replace_files(void);
int re_data_replace_of(int file);
const char *re_data_replace_file(int file);
int re_data_replace_pieces(int file);

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
