#include "re_data.h"

#include <stdio.h>
#include <string.h>

#include "re_data_coaching.h"

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
    [RE_DATA_VOTE_INTERVAL] = {"the city's vote on its policies, every so many months", "policyVoteInterval=\"24\"",
                               "policyVoteInterval=\"12\""},
};

/* The two donations: the game's donation to a political group (81451, data/activities/lobbyingActivity.xml) with
 * ten and a hundred times its price and a status of ten and a hundred times its points (13551 in data/status/
 * passionStatus.xml: influenceGen 100, given by the activity for 12 months; run1696 counted 783 to 1,188 a month
 * for 1000 over nine months, so the file's "~600 points" is for its length of 6, which the activity overrides),
 * listed for the passion Lobbying at a bank as it is (data/passions/passionLobbying.xml). Once a year, where the
 * game's is once a month: a second donation while the status runs only starts its 12 months again (run1697: paid
 * in October and November, one status's points in December). */
#define DONATION(id, name, money, status, points)                                                                                      \
    "\t<activity>\n\t\t<id>" id "</id>\n\t\t<desc category=\"Activity\" name=\"" name "\"/>\n\t\t<stats active=\"1\">\n\t\t\t<tip>About " \
    points " influence points a month</tip>\n\t\t</stats>\n\t\t<image icon=\"ActionDonateMoney\"/>\n\t\t<action hours=\"10\" "             \
    "interval=\"12\" intervalGlobal=\"1\" influenceUnlocked=\"1\" money=\"" money "\" finType=\"2556\">\n\t\t\t<status id=\"" status      \
    "\" amt=\"12\"/>\n\t\t\t<pref id=\"10075\" amt=\"4\"/>\n\t\t\t<pref id=\"10090\" amt=\"8\"/>\n\t\t</action>\n\t</activity>\n"
#define INFLUENCE(id, name, gen)                                                                                                       \
    "\t<status>\n\t\t<id>" id "</id>\n\t\t<desc category=\"Status Effect\" name=\"" name "\"/>\n\t\t<stats active=\"1\" disease=\"1\" "  \
    "length=\"6\" consequence=\"1\" disableBirthday=\"1\">\n\t\t\t<tip></tip>\n\t\t</stats>\n\t\t<image "                                 \
    "icon=\"StatusPassionPoliticalInfluence\"/>\n\t\t<decay interval=\"1\" default=\"-1.0\"/>\n\t\t<effects influenceGen=\"" gen          \
    "\" influenceVariance=\"0.1\">\n\t\t\t<pref id=\"10075\" amt=\"1\"/>\n\t\t\t<pref id=\"10090\" amt=\"1\"/>\n\t\t</effects>\n\t</status>\n"
/* an overlay's names: the two activities with their tips, or the two statuses */
#define NAMES(a, tip_a, b, tip_b)                                                                                                      \
    "\t<activity>\n\t\t<id>81452</id>\n\t\t<desc name=\"" a "\"/>\n\t\t<stats active=\"1\">\n\t\t\t<tip replace=\"1\">" tip_a              \
    "</tip>\n\t\t</stats>\n\t</activity>\n\t<activity>\n\t\t<id>81453</id>\n\t\t<desc name=\"" b "\"/>\n\t\t<stats active=\"1\">\n\t\t\t" \
    "<tip replace=\"1\">" tip_b "</tip>\n\t\t</stats>\n\t</activity>\n"
#define STATUS_NAMES(a, b)                                                                                                             \
    "\t<status>\n\t\t<id>13552</id>\n\t\t<desc name=\"" a "\"/>\n\t\t<stats active=\"1\"/>\n\t</status>\n\t<status>\n\t\t<id>13553</id>"  \
    "\n\t\t<desc name=\"" b "\"/>\n\t\t<stats active=\"1\"/>\n\t</status>\n"

static const struct {
    int addition;
    const char *doc, *doc_also; /* pieces of text the file has; doc_also may be NULL */
    const char *after;          /* the elements go after this piece; NULL = before the file's last "</xml>" */
    const char *id;             /* the first added id: a file that has it already gets nothing */
    const char *text;
} ADDS[] = {
    {RE_ADD_DONATIONS, "<desc category=\"Activity\" name=\"Donate To Political Group\"/>", NULL, NULL, "<id>81452</id>",
     DONATION("81452", "Fund A Think Tank", "-100000000", "13552", "1,000")
         DONATION("81453", "Fund A Political Party", "-1000000000", "13553", "10,000")},
    {RE_ADD_DONATIONS, "<desc category=\"Status Effect\" name=\"Political Influence\"/>", NULL, NULL, "<id>13552</id>",
     INFLUENCE("13552", "Think Tank Influence", "1000") INFLUENCE("13553", "Party Influence", "10000")},
    {RE_ADD_DONATIONS, "<desc category=\"Passion\" name=\"Lobbying\"/>", NULL, "<activity id=\"81451\" cond=\"50521\"/>",
     "<activity id=\"81452\"", "\n\t\t\t<activity id=\"81452\" cond=\"50521\"/>\n\t\t\t<activity id=\"81453\" cond=\"50521\"/>"},
    /* the overlays of the seven other languages: the game's name there of the donation, and of its status */
    {RE_ADD_DONATIONS, "<id>81451</id>", "<desc name=\"정치 단체에 기부하기\"/>", NULL, "<id>81452</id>",
     NAMES("싱크탱크에 기부하기", "달마다 영향력 포인트 약 1,000", "정당에 기부하기", "달마다 영향력 포인트 약 10,000")},
    {RE_ADD_DONATIONS, "<id>13551</id>", "<desc name=\"정치적 영향력\"/>", NULL, "<id>13552</id>", STATUS_NAMES("싱크탱크 영향력", "정당 영향력")},
    {RE_ADD_DONATIONS, "<id>81451</id>", "<desc name=\"Spende an politische Gruppen\"/>", NULL, "<id>81452</id>",
     NAMES("Spende an einen Thinktank", "Etwa 1.000 Einflusspunkte im Monat", "Spende an eine Partei", "Etwa 10.000 Einflusspunkte im Monat")},
    {RE_ADD_DONATIONS, "<id>13551</id>", "<desc name=\"Politischer Einfluss\"/>", NULL, "<id>13552</id>",
     STATUS_NAMES("Einfluss durch Thinktank", "Einfluss durch Partei")},
    {RE_ADD_DONATIONS, "<id>81451</id>", "<desc name=\"Donaciones a grupos políticos\"/>", NULL, "<id>81452</id>",
     NAMES("Donación a un laboratorio de ideas", "Unos 1.000 puntos de influencia al mes", "Donación a un partido político",
           "Unos 10.000 puntos de influencia al mes")},
    {RE_ADD_DONATIONS, "<id>13551</id>", "<desc name=\"Influencia política\"/>", NULL, "<id>13552</id>",
     STATUS_NAMES("Influencia de think tank", "Influencia de partido")}, /* the longer name ran into the "1a" of its row */
    {RE_ADD_DONATIONS, "<id>81451</id>", "<desc name=\"Faire un don à un groupe politique\"/>", NULL, "<id>81452</id>",
     NAMES("Faire un don à un think tank", "Environ 1 000 points d'influence par mois", "Faire un don à un parti politique",
           "Environ 10 000 points d'influence par mois")},
    {RE_ADD_DONATIONS, "<id>13551</id>", "<desc name=\"Influence politique\"/>", NULL, "<id>13552</id>",
     STATUS_NAMES("Influence de think tank", "Influence de parti")},
    {RE_ADD_DONATIONS, "<id>81451</id>", "<desc name=\"Doação para grupo político\"/>", NULL, "<id>81452</id>",
     NAMES("Doação para think tank", "Cerca de 1.000 pontos de influência por mês", "Doação para partido político",
           "Cerca de 10.000 pontos de influência por mês")},
    {RE_ADD_DONATIONS, "<id>13551</id>", "<desc name=\"Influência política\"/>", NULL, "<id>13552</id>",
     STATUS_NAMES("Influência de think tank", "Influência de partido")},
    {RE_ADD_DONATIONS, "<id>81451</id>", "<desc name=\"政治団体への寄付\"/>", NULL, "<id>81452</id>",
     NAMES("シンクタンクへの寄付", "毎月影響ポイント約1,000", "政党への寄付", "毎月影響ポイント約10,000")},
    {RE_ADD_DONATIONS, "<id>13551</id>", "<desc name=\"政治的影響力\"/>", NULL, "<id>13552</id>", STATUS_NAMES("シンクタンクの影響力", "政党の影響力")},
    {RE_ADD_DONATIONS, "<id>81451</id>", "<desc name=\"向政治团体捐款\"/>", NULL, "<id>81452</id>",
     NAMES("向智库捐款", "每月约1,000影响力点", "向政党捐款", "每月约10,000影响力点")},
    {RE_ADD_DONATIONS, "<id>13551</id>", "<desc name=\"政治影响力\"/>", NULL, "<id>13552</id>", STATUS_NAMES("智库影响力", "政党影响力")},
    /* Personal Coaching, one activity a preference (re_data_coaching.h, from 77650): data/activities/healthActivity.xml
     * and the hospital of data/sites/govSites.xml below Mental Therapy (77611), then the overlays, each known by the
     * game's name there of Mental Therapy */
    {RE_ADD_COACHING, "<desc category=\"Activity\" name=\"Mental Therapy\"/>", NULL, NULL, "<id>77650</id>", COACHING_ACTIVITIES},
    {RE_ADD_COACHING, "<desc category=\"Site\" name=\"Hospital\"/>", NULL, "<activity id=\"77611\"/>", "<activity id=\"77650\"",
     COACHING_SITE},
    {RE_ADD_COACHING, "<id>77611</id>", "<desc name=\"정신 치료\"/>", NULL, "<id>77650</id>", COACHING_NAMES_KO},
    {RE_ADD_COACHING, "<id>77611</id>", "<desc name=\"Psychotherapie\"/>", NULL, "<id>77650</id>", COACHING_NAMES_DE},
    {RE_ADD_COACHING, "<id>77611</id>", "<desc name=\"Terapia mental\"/>", NULL, "<id>77650</id>", COACHING_NAMES_ES},
    {RE_ADD_COACHING, "<id>77611</id>", "<desc name=\"Thérapie mentale\"/>", NULL, "<id>77650</id>", COACHING_NAMES_FR},
    {RE_ADD_COACHING, "<id>77611</id>", "<desc name=\"Terapia Mental\"/>", NULL, "<id>77650</id>", COACHING_NAMES_PT_BR},
    {RE_ADD_COACHING, "<id>77611</id>", "<desc name=\"精神療法\"/>", NULL, "<id>77650</id>", COACHING_NAMES_JP},
    {RE_ADD_COACHING, "<id>77611</id>", "<desc name=\"心理治疗\"/>", NULL, "<id>77650</id>", COACHING_NAMES_ZH_CN},
};
#define ADD_COUNT ((int)(sizeof ADDS / sizeof ADDS[0]))

/* Pieces of text put in the place of others, of any length: `text` takes the place of the first `find` after
 * `after` when it stands before the first closing tag ("</") after `after` (NULL: from the file's start, anywhere) */
typedef struct {
    const char *after, *find, *text;
} piece;

#include "re_data_ko.h"

/* The words for a level of a preference (RE_ADD_LEVEL_WORDS), each language's file known by its word for a dislike or
 * a hate; each piece is the whole element, so that a file whose word is right already is left alone (the Japanese
 * love becomes 大好き, which holds the old 好き). Spanish takes the third person for the dislike as well ("No me
 * gusta", "I do not like it"), as for the like. */
static const piece WORDS_ES[] = {
    {NULL, "<like>Como</like>", "<like>Le gusta</like>"},
    {NULL, "<likeShort>Como</likeShort>", "<likeShort>Gust</likeShort>"},
    {NULL, "<dislike>No me gusta</dislike>", "<dislike>No le gusta</dislike>"},
    {NULL, "<dislikeShort>Dlke</dislikeShort>", "<dislikeShort>NoGu</dislikeShort>"},
    {NULL, "<addictedShort>Añadir</addictedShort>", "<addictedShort>Adic</addictedShort>"},
};
static const piece WORDS_FR[] = {
    {NULL, "<like>Comme</like>", "<like>Aime</like>"},
    {NULL, "<likeShort>Comm</likeShort>", "<likeShort>Aime</likeShort>"},
};
static const piece WORDS_PT_BR[] = {
    {NULL, "<like>Como</like>", "<like>Gosta</like>"},
    {NULL, "<likeShort>Como</likeShort>", "<likeShort>Gost</likeShort>"},
    {NULL, "<dislike>Não Gostei</dislike>", "<dislike>Não gosta</dislike>"},
};
static const piece WORDS_JP[] = {
    {NULL, "<like>楽しむ</like>", "<like>好き</like>"},
    {NULL, "<likeShort>楽しむ</likeShort>", "<likeShort>好き</likeShort>"},
    {NULL, "<love>好き</love>", "<love>大好き</love>"},
    {NULL, "<loveShort>好き</loveShort>", "<loveShort>大好き</loveShort>"},
};
/* the game's donation once a year (RE_ADD_DONATION_YEARLY): its action element, the only one of its kind in its file */
static const piece DONATION_YEARLY[] = {
    {NULL, "<action hours=\"10\" interval=\"1\" intervalGlobal=\"1\" influenceUnlocked=\"1\" money=\"-10000000\" finType=\"2556\">",
     "<action hours=\"10\" interval=\"12\" intervalGlobal=\"1\" influenceUnlocked=\"1\" money=\"-10000000\" finType=\"2556\">"},
};
#define PIECES_OF(a) ((int)(sizeof a / sizeof a[0]))

/* the files that get them, each known by a piece of text only it has */
static const struct {
    int addition;
    const char *doc;
    const piece *pieces;
    int count;
} REPLACES[] = {
    RE_KO_FILES,
    {RE_ADD_LEVEL_WORDS, "<dislike>No me gusta</dislike>", WORDS_ES, PIECES_OF(WORDS_ES)},
    {RE_ADD_LEVEL_WORDS, "<dislike>N'aime pas</dislike>", WORDS_FR, PIECES_OF(WORDS_FR)},
    {RE_ADD_LEVEL_WORDS, "<dislike>Não Gostei</dislike>", WORDS_PT_BR, PIECES_OF(WORDS_PT_BR)},
    {RE_ADD_LEVEL_WORDS, "<hate>嫌悪</hate>", WORDS_JP, PIECES_OF(WORDS_JP)},
    {RE_ADD_DONATION_YEARLY, "<desc category=\"Activity\" name=\"Donate To Political Group\"/>", DONATION_YEARLY,
     PIECES_OF(DONATION_YEARLY)},
};
#define REPLACE_FILES ((int)(sizeof REPLACES / sizeof REPLACES[0]))
#define REPLACE_PIECES                                                                                                                 \
    (RE_KO_PIECES + PIECES_OF(WORDS_ES) + PIECES_OF(WORDS_FR) + PIECES_OF(WORDS_PT_BR) + PIECES_OF(WORDS_JP) + PIECES_OF(DONATION_YEARLY))
_Static_assert(REPLACE_FILES <= RE_DATA_REPLACE_FILES, "more files than re_data_add's caller counts");

static const char *const ADD_WHAT[RE_ADD_COUNT] = {
    [RE_ADD_DONATIONS] = "two donations of the passion Lobbying at a bank: $1M for about 1,000 influence points a month for 12 months, "
                         "$10M for about 10,000",
    [RE_ADD_COACHING] = "Personal Coaching at a hospital, one activity a preference: $500,000 and 20 hours, twice the price from a "
                        "dislike to a like, one preference a year for each person",
    [RE_ADD_KOREAN] = "Korean texts of the game that are wrong or read strangely",
    [RE_ADD_LEVEL_WORDS] = "the words for how much a person likes something, where the language's file says something else",
    [RE_ADD_DONATION_YEARLY] = "the game's donation to a political group once a year for the household, as the two larger ones",
};

const char *re_data_add_what(int addition)
{
    return ADD_WHAT[addition];
}

int re_data_add_rows(void)
{
    return ADD_COUNT;
}

int re_data_replace_files(void)
{
    return REPLACE_FILES;
}

int re_data_replace_of(int file)
{
    return REPLACES[file].addition;
}

const char *re_data_replace_file(int file)
{
    return REPLACES[file].doc;
}

int re_data_replace_pieces(int file)
{
    return REPLACES[file].count;
}

int re_data_add_of(int row)
{
    return ADDS[row].addition;
}

const char *re_data_add_file(int row)
{
    return ADDS[row].doc_also ? ADDS[row].doc_also : ADDS[row].doc;
}

/* the last occurrence of `needle` in `text`, or NULL */
static const char *last(const char *text, const char *needle)
{
    const char *found = NULL;
    for (const char *at = strstr(text, needle); at != NULL; at = strstr(at + 1, needle))
        found = at;
    return found;
}

char *re_data_add(const char *xml, const int *on, void *(*alloc)(unsigned size), unsigned *made, int replaced[RE_DATA_REPLACE_FILES])
{
    /* one edit for each row of ADDS and each piece of REPLACES that belong to this file; `of` is the row of ADDS, or
     * -1 - the file of REPLACES */
    enum { EDITS = ADD_COUNT + REPLACE_PIECES };
    const char *at[EDITS], *text[EDITS];
    size_t cut[EDITS], extra = 0;
    int of[EDITS], wanted = 0;
    for (int i = 0; i < ADD_COUNT; i++) {
        if (!on[ADDS[i].addition] || strstr(xml, ADDS[i].doc) == NULL || (ADDS[i].doc_also && strstr(xml, ADDS[i].doc_also) == NULL) ||
            strstr(xml, ADDS[i].id) != NULL)
            continue;
        const char *place = ADDS[i].after ? strstr(xml, ADDS[i].after) : last(xml, "</xml>");
        if (place == NULL)
            continue;
        at[wanted] = ADDS[i].after ? place + strlen(ADDS[i].after) : place;
        cut[wanted] = 0;
        text[wanted] = ADDS[i].text;
        of[wanted++] = i;
    }
    for (int f = 0; f < REPLACE_FILES; f++) {
        if (!on[REPLACES[f].addition] || strstr(xml, REPLACES[f].doc) == NULL)
            continue;
        for (int p = 0; p < REPLACES[f].count && wanted < EDITS; p++) {
            const piece *r = &REPLACES[f].pieces[p];
            const char *from = r->after ? strstr(xml, r->after) : xml;
            from = from != NULL && r->after ? from + strlen(r->after) : from;
            const char *place = from != NULL ? strstr(from, r->find) : NULL, *close = r->after && from ? strstr(from, "</") : NULL;
            /* in the element `after` is in: before the first closing tag after it, where a file whose text is right
             * already has none */
            if (place == NULL || (close != NULL && place > close))
                continue;
            at[wanted] = place;
            cut[wanted] = strlen(r->find);
            text[wanted] = r->text;
            of[wanted++] = -1 - f;
        }
    }
    if (wanted == 0)
        return NULL;
    for (int i = 0; i < wanted; i++)
        extra += strlen(text[i]);
    size_t len = strlen(xml);
    char *grown = alloc((unsigned)(len + extra + 1));
    if (grown == NULL)
        return NULL;
    /* the text up to each place in the order of the places, the text of that place (in the place of what it
     * replaces), and so on */
    size_t from = 0, to = 0;
    for (;;) {
        int next = -1;
        for (int i = 0; i < wanted; i++)
            if (at[i] != NULL && (next < 0 || at[i] < at[next]))
                next = i;
        if (next < 0)
            break;
        size_t place = (size_t)(at[next] - xml), n = strlen(text[next]);
        at[next] = NULL;
        if (place < from) /* inside what another row replaced: not made */
            continue;
        memcpy(grown + to, xml + from, place - from);
        to += place - from;
        memcpy(grown + to, text[next], n);
        to += n;
        from = place + cut[next];
        if (of[next] >= 0)
            *made |= 1u << of[next];
        else
            replaced[-1 - of[next]]++;
    }
    memcpy(grown + to, xml + from, len - from);
    to += len - from;
    grown[to] = 0;
    return grown;
}

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
