#include "re_wording.h"

#include <string.h>

#include "re_lang.h"

typedef struct {
    int lang;            /* column of re_lang.h: the entry is for the game set to this language */
    const char *key;
    const char *game;    /* modsLanguages/<language>/baseTranslate.xml as shipped, placeholders included */
    const char *better;  /* the same placeholders, each at most once */
    unsigned from, size; /* only for requests from this range of code; size 0 = from anywhere */
    const char *nameless; /* the text for a line whose first placeholder came without a value, NULL = `better` */
} wording;

/* English originals, for the reader: Fixed / Variable (interest type), Fixed (cooldown type), Deposit and Minimum
 * Deposit in the mortgage window (the down payment; the same key is right as "예금" in the savings windows),
 * "Could not afford X", "... Debt added with 20% penalty for total M", "... Debt added for M", "L added for M",
 * "Max borrowed!", "Current Rate" (of tax), "No property bought, deposit repaid.", "Applying for new loan, previous
 * deposit returned.", "X industry is busting!" (the Korean said it was thriving), "never" as the date a stock was
 * last researched (the Korean is the adverb, "absolutely not"), "Last Researched" (the activity is called 주식 조사).
 * Other languages: only what shows at a glance. Spanish has "Corregido" (corrected) for a fixed rate, like the Korean;
 * the French sentence for a busting industry is the one for a booming industry, word for word.
 * "L:I stolen due to lack of floorspace" and "L:I recovered by police" (lines of the month): L is the name of the
 * home or of the business premises. A household without a home has no such name: the theft line then starts with
 * "??", which the game puts in for an empty name (0x00557880), and the recovery line with the colon. The Korean adds
 * a particle in both spellings to whatever the item is called.
 * "C has entered bankruptcy! Your N shares in the company are now worthless.", "C issues N shares to avoid
 * bankruptcy! Your ownership is diluted by P.", "C is downsizing operations to avoid bankruptcy! Equity per share
 * reduced by P.", "C has listed on the stock market!": the Korean puts one particle behind every company name,
 * right or not ("Holmes 아케이드이"), and calls the equity a share "shares per share" as elsewhere. */
static const wording TABLE[] = {
    {RE_LANG_KO, "interestRateFixed", "수정됨", "고정", 0, 0, NULL},
    {RE_LANG_KO, "interestRateVariable", "변수", "변동", 0, 0, NULL},
    {RE_LANG_KO, "cooldownTypeFixed", "수정됨", "고정", 0, 0, NULL},
    {RE_LANG_KO, "depositTitle", "예금", "계약금", 0x00631340u, 9884u, NULL},
    {RE_LANG_KO, "depositMinimum", "최소 예금", "최소 계약금", 0x00631340u, 9884u, NULL},
    {RE_LANG_KO, "cannotAffordProduct", "감당할 수 없음 {{PRODUCT}}", "돈이 모자람: {{PRODUCT}}", 0, 0, NULL},
    {RE_LANG_KO, "cannotAffordPenalty", "감당할 수 없는 {{PRODUCT}}! 총 {{MONEY}}에 20% 페널티가 부과된 부채가 추가되었습니다.",
     "돈이 모자라 {{PRODUCT}} 미납! 20% 페널티를 더한 부채 {{MONEY}} 추가", 0, 0, NULL},
    {RE_LANG_KO, "cannotAffordDebtAdded", "{{PRODUCT}}을 구매할 수 없습니다. 부채가 {{MONEY}}에 추가되었습니다.",
     "돈이 모자라 {{PRODUCT}} 미납. 부채 {{MONEY}} 추가", 0, 0, NULL},
    {RE_LANG_KO, "loanAdded", "{{LOANTYPE}}이 {{MONEY}}로 추가되었습니다.", "{{LOANTYPE}} {{MONEY}} 추가", 0, 0, NULL},
    {RE_LANG_KO, "maxBorrowed", "최대 빌린!", "한도까지 빌렸습니다!", 0, 0, NULL},
    {RE_LANG_KO, "currentTaxRate", "현재 요금", "현재 세율", 0, 0, NULL},
    {RE_LANG_KO, "noPropertyDepositRepaid", "부동산을 구매하지 않아 보증금이 반환되었습니다.", "부동산을 사지 않아 계약금을 돌려받았습니다.", 0, 0, NULL},
    {RE_LANG_KO, "newLoanReturnDeposit", "신규 대출 신청, 이전 보증금 반환.", "새 대출을 신청해 이전 계약금을 돌려받았습니다.", 0, 0, NULL},
    {RE_LANG_KO, "industryBust", "{{INDUSTRYNAME}} 산업이 활기를 띠고 있습니다!", "{{INDUSTRYNAME}} 산업이 불황을 겪고 있습니다!", 0, 0, NULL},
    {RE_LANG_KO, "never", "절대", "없음", 0x007b5870u, 5306u, NULL},
    {RE_LANG_KO, "stockLastResearched", "최근 연구", "최근 조사", 0, 0, NULL},
    {RE_LANG_KO, "itemStolen", "{{LOCATIONNAME}}:{{ITEMNAME}}이(가) 바닥 공간 부족으로 도난당함",
     "{{LOCATIONNAME}}: {{ITEMNAME}} 도난 (바닥 공간 부족)", 0, 0, "{{ITEMNAME}} 도난 (집이나 사업장 없음)"},
    {RE_LANG_KO, "itemRecovered", "{{LOCATIONNAME}}:{{ITEMNAME}}이(가) 경찰에 의해 회수됨",
     "{{LOCATIONNAME}}: 도난당한 {{ITEMNAME}} 경찰이 되찾음", 0, 0, "도난당한 {{ITEMNAME}} 경찰이 되찾음"},
    {RE_LANG_KO, "stockBankruptWorthless", "{{COMPANYNAME}}이 파산했습니다! 귀하의 {{SHARENUMBER}} 주식은 이제 무가치합니다.",
     "{{COMPANYNAME}} 파산: 가진 주식 {{SHARENUMBER}}주는 가치가 없어졌습니다", 0, 0, NULL},
    {RE_LANG_KO, "stockCapitalRaiseDesperate",
     "{{COMPANYNAME}}이 파산을 피하기 위해 {{NUMBERSHARES}} 주식을 발행했습니다! 귀하의 지분율이 {{NUMBERPERCENTAGE}} 희석되었습니다.",
     "{{COMPANYNAME}}: 파산을 피하려고 새 주식 {{NUMBERSHARES}}주 발행. 내 지분율이 {{NUMBERPERCENTAGE}} 줄었습니다", 0, 0, NULL},
    {RE_LANG_KO, "stockDownsizeDesperate",
     "{{COMPANYNAME}}이 파산을 피하기 위해 사업 규모를 축소합니다! 주당 지분이 {{NUMBERPERCENTAGE}} 감소했습니다.",
     "{{COMPANYNAME}}: 파산을 피하려고 사업 규모 축소. 주당 자본이 {{NUMBERPERCENTAGE}} 줄었습니다", 0, 0, NULL},
    {RE_LANG_KO, "stockListing", "{{COMPANYNAME}}이 주식 시장에 상장했습니다!", "{{COMPANYNAME}}: 주식 시장에 새로 상장", 0, 0, NULL},
    /* From here on: names the language file misspells (the game fills in DATESTRING, the file says "데이터스트링"), so
     * the game shows the bare word. The value comes from the call (re_wording_apply's `call`). Found by
     * analysis/scripts/lang_placeholders.py; the names are the strings the game's code passes. */
    {RE_LANG_KO, "futuresDeclinedBankruptcy", "최근 파산으로 인해 선물을 거래할 수 없습니다. 파산은 {{데이터스트링}}에 만료됩니다.",
     "최근 파산 때문에 선물을 거래할 수 없습니다. 파산 기록은 {{DATESTRING}}에 없어집니다.", 0, 0, NULL},
    {RE_LANG_KO, "kinEditRelationshipsTitle", "{{PERSONNAME}} ({{연령월}}) 관계", "{{PERSONNAME}} ({{AGEYEARSMONTHS}}) 관계", 0, 0, NULL},
    {RE_LANG_EN, "itemStolen", "{{LOCATIONNAME}}:{{ITEMNAME}} stolen due to lack of floorspace",
     "{{LOCATIONNAME}}:{{ITEMNAME}} stolen due to lack of floorspace", 0, 0, "{{ITEMNAME}} stolen: no home or premises to keep it in"},
    {RE_LANG_EN, "itemRecovered", "{{LOCATIONNAME}}:{{ITEMNAME}} recovered by police", "{{LOCATIONNAME}}:{{ITEMNAME}} recovered by police", 0,
     0, "{{ITEMNAME}} recovered by police"},
    {RE_LANG_ES, "interestRateFixed", "Corregido", "Fijo", 0, 0, NULL},
    {RE_LANG_ES, "cooldownTypeFixed", "Corregido", "Fijo", 0, 0, NULL},
    {RE_LANG_ES, "timeMonthsTranslate", "{{MES}}m", "{{MONTHS}}m", 0, 0, NULL},
    {RE_LANG_ES, "personalityCompatible", "{{PERSONFIRST}} y {{PERSONECOND}} son {{PERCENT}}% compatibles.",
     "{{PERSONFIRST}} y {{PERSONSECOND}} son {{PERCENT}}% compatibles.", 0, 0, NULL},
    {RE_LANG_ES, "employeeWageDemand", "({{FIRMNAME}}, {{JOBTITLE}}) ¡{STAFFNAME}} exige un aumento de sueldo!",
     "({{FIRMNAME}}, {{JOBTITLE}}) ¡{{STAFFNAME}} exige un aumento de sueldo!", 0, 0, NULL},
    {RE_LANG_ES, "educationCompleted", "¡{{PERSONNAME}} ha completado {{EDUCACIÓN}} con un {{PERCENT}}% de nota!",
     "¡{{PERSONNAME}} ha completado {{EDUCATION}} con un {{PERCENT}}% de nota!", 0, 0, NULL},
    {RE_LANG_ES, "itemStolen", "{{LOCATIONNAME}}:{{ITEMNAME}} robado por falta de suelo",
     "{{LOCATIONNAME}}:{{ITEMNAME}} robado por falta de suelo", 0, 0, "{{ITEMNAME}} robado: no hay casa ni local donde guardarlo"},
    {RE_LANG_ES, "itemRecovered", "{{LOCATIONNAME}}:{{ITEMNAME}} recuperado por la policía",
     "{{LOCATIONNAME}}:{{ITEMNAME}} recuperado por la policía", 0, 0, "{{ITEMNAME}} recuperado por la policía"},
    {RE_LANG_FR, "industryBust", "L'industrie {{INDUSTRYNAME}} est en plein essor !", "L'industrie {{INDUSTRYNAME}} est en crise !", 0, 0, NULL},
    {RE_LANG_FR, "stockCapitalRaiseDesperate",
     "{{COMPANYNAME}} émet des actions {{NUMBERHARES}} pour éviter la faillite ! Votre participation est diluée de {{NUMBERPERCENTAGE}}.",
     "{{COMPANYNAME}} émet {{NUMBERSHARES}} actions pour éviter la faillite ! Votre participation est diluée de {{NUMBERPERCENTAGE}}.", 0, 0,
     NULL},
    /* the file's lines end with CR LF; the better text is written with LF like the plugin's own hover texts */
    {RE_LANG_FR, "tenantConfirmLease",
     "Confirmer le bail du locataire ?\r\n{{HOUSEADDRESS}}\r\nEstimation du marché : {{MARKETRENT}}\r\nLoyer proposé : "
     "{{RENTPRICE}}\r\nDurée du bail : {{RENTMONTHES}} mois",
     "Confirmer le bail du locataire ?\n{{HOUSEADDRESS}}\nEstimation du marché : {{MARKETRENT}}\nLoyer proposé : {{RENTPRICE}}\nDurée "
     "du bail : {{RENTMONTHS}} mois",
     0, 0, NULL},
    {RE_LANG_FR, "itemStolen", "{{LOCATIONNAME}}:{{ITEMNAME}} volé par manque de place",
     "{{LOCATIONNAME}}:{{ITEMNAME}} volé par manque de place", 0, 0, "{{ITEMNAME}} volé : ni logement ni locaux où le garder"},
    {RE_LANG_FR, "itemRecovered", "{{LOCATIONNAME}}:{{ITEMNAME}} récupéré par la police",
     "{{LOCATIONNAME}}:{{ITEMNAME}} récupéré par la police", 0, 0, "{{ITEMNAME}} récupéré par la police"},
    /* German, Japanese and Chinese write the names without braces, which the game fills in all the same */
    {RE_LANG_DE, "loanAdded", "LOANTYPE für GELD hinzugefügt", "{{LOANTYPE}} für {{MONEY}} hinzugefügt", 0, 0, NULL},
    /* PERSONAME is the name the game passes (the English file has it so) */
    {RE_LANG_DE, "rejectedMeetingsDescription",
     "PERSONNAME: NUMBER potenzieller Freunde, die aufgrund der Filtereinstellungen abgelehnt wurden",
     "{{PERSONAME}}: {{NUMBER}} mögliche Freunde wegen der Filtereinstellungen abgelehnt", 0, 0, NULL},
    {RE_LANG_DE, "leaseExpiringRenewPopup",
     "Der Mietvertrag für HOUSEADDRESS läuft aus. Mietvertrag für MONEY pro Monat für MONATE Monate verlängern?",
     "Der Mietvertrag für {{HOUSEADDRESS}} läuft aus. Mietvertrag für {{MONEY}} pro Monat für {{MONTHS}} Monate verlängern?", 0, 0, NULL},
    {RE_LANG_DE, "itemStolen", "LOCATIONNAME: ITEMNAME wegen Platzmangels gestohlen",
     "{{LOCATIONNAME}}: {{ITEMNAME}} wegen Platzmangels gestohlen", 0, 0,
     "{{ITEMNAME}} gestohlen: weder Zuhause noch Geschäftsräume zum Aufbewahren"},
    {RE_LANG_DE, "itemRecovered", "LOCATIONNAME: ITEMNAME von der Polizei sichergestellt",
     "{{LOCATIONNAME}}: {{ITEMNAME}} von der Polizei sichergestellt", 0, 0, "{{ITEMNAME}} von der Polizei sichergestellt"},
    {RE_LANG_PT_BR, "educationLoanTip",
     "Leia as condições e clique em Accept (Aceitar). Em seguida, inscreva-se em um curso. O empréstimo será aplicado "
     "automaticamente com as condições abaixo. Você tem direito a {{NÚMERO PERMITIDO}} empréstimos educacionais por toda a vida.",
     "Leia as condições e clique em Accept (Aceitar). Em seguida, inscreva-se em um curso. O empréstimo será aplicado "
     "automaticamente com as condições abaixo. Você tem direito a {{NUMALLOWED}} empréstimos educacionais por toda a vida.",
     0, 0, NULL},
    {RE_LANG_PT_BR, "tenantLeaseExpired", "{O contrato de aluguel do locatário expira este mês",
     "{{HOUSEADDRESS}}: o contrato de aluguel do locatário expira este mês", 0, 0, NULL},
    {RE_LANG_PT_BR, "itemStolen", "{{LOCATIONNAME}}:{{ITEMNAME}} roubado devido à falta de espaço no chão",
     "{{LOCATIONNAME}}:{{ITEMNAME}} roubado devido à falta de espaço no chão", 0, 0,
     "{{ITEMNAME}} roubado: sem casa nem imóvel onde guardar"},
    {RE_LANG_PT_BR, "itemRecovered", "{{LOCATIONNAME}}:{{ITEMNAME}} recuperado pela polícia",
     "{{LOCATIONNAME}}:{{ITEMNAME}} recuperado pela polícia", 0, 0, "{{ITEMNAME}} recuperado pela polícia"},
    {RE_LANG_JP, "qualifiedFromPreviousHours", "以前に少なくともX時間完了", "以前に少なくとも{{HOURS}}時間完了", 0, 0, NULL},
    {RE_LANG_JP, "passionMaximum", "キャラクターあたりの最大情熱数", "情熱はキャラクター1人につき最大{{NUMBER}}個", 0, 0, NULL},
    {RE_LANG_JP, "invitePersonKickedOutRecently", "PERSONNAMEさんは最近追い出されたばかりだ。再参加は○月○日以降可能",
     "{{PERSONNAME}}さんは最近追い出されたばかりだ。再参加は{{JOINDATE}}以降可能", 0, 0, NULL},
    /* The Japanese file has, from preferenceChangeNote to birthdayLowPointsTip, under each key the sentence of the
     * key after it (the birthday window): each gets the sentence that stands one key earlier in the file, and the
     * first, which the file has nowhere, a new one. */
    {RE_LANG_JP, "preferenceChangeNote", "お誕生日おめでとうございます、PERSONNAME！",
     "{{PERSONNAME}}は現在、{{PREFERENCE}}に対して{{FEELING}}と感じている", 0, 0, NULL},
    {RE_LANG_JP, "happyBirthdayPerson", "健康ポイントでこの変更を有効にする。", "お誕生日おめでとうございます、{{PERSONNAME}}！", 0, 0, NULL},
    {RE_LANG_JP, "birthdayHealthPointsRequiredTip", "幸福ポイントでこの変更を有効にする。", "健康ポイントでこの変更を有効にする。", 0, 0, NULL},
    {RE_LANG_JP, "birthdayHappinessPointsRequiredTip", "以下の変更を適用した後の健康ポイント。", "幸福ポイントでこの変更を有効にする。", 0, 0,
     NULL},
    {RE_LANG_JP, "birthdayHealthPointsChangeTip", "以下の変更を適用した後の幸福ポイント。", "以下の変更を適用した後の健康ポイント。", 0, 0,
     NULL},
    {RE_LANG_JP, "birthdayHappinessPointsChangeTip", "変更を有効にするためにポイントを消費する：", "以下の変更を適用した後の幸福ポイント。", 0,
     0, NULL},
    {RE_LANG_JP, "birthdaySpendPointsTip", "無効 - 健康ポイントまたは幸福ポイントがマイナスになった場合、強制的に変更する。",
     "変更を有効にするためにポイントを消費する：", 0, 0, NULL},
    {RE_LANG_JP, "birthdayForcedChangePointsTip", "無効 - 健康ポイントまたは幸福ポイントが十分でない。",
     "無効 - 健康ポイントまたは幸福ポイントがマイナスになった場合、強制的に変更する。", 0, 0, NULL},
    {RE_LANG_JP, "birthdayLowPointsTip", "思春期の気分の変動を引き起こした：", "無効 - 健康ポイントまたは幸福ポイントが十分でない。", 0, 0, NULL},
    {RE_LANG_JP, "itemStolen", "LOCATIONNAME:ITEMNAME 床面積不足により盗難", "{{LOCATIONNAME}}:{{ITEMNAME}} 床面積不足により盗難", 0, 0,
     "{{ITEMNAME}} 盗難 (保管する家も事業所もない)"},
    {RE_LANG_JP, "itemRecovered", "LOCATIONNAME:ITEMNAME 警察により回収", "{{LOCATIONNAME}}:{{ITEMNAME}} 警察により回収", 0, 0,
     "{{ITEMNAME}} 警察により回収"},
    {RE_LANG_ZH_CN, "isBorn", "名字 出生！", "{{NAME}} 出生了！", 0, 0, NULL},
    {RE_LANG_ZH_CN, "chartHouseholdFinancialTitle", "家庭收入/支出（货币)", "家庭收入/支出（{{CURRENCY}}）", 0, 0, NULL},
    {RE_LANG_ZH_CN, "chartHouseholdTaxTitle", "家庭纳税申报单（货币)", "家庭纳税申报单（{{CURRENCY}}）", 0, 0, NULL},
    {RE_LANG_ZH_CN, "itemStolen", "LOCATIONNAME：ITEMNAME因空间不足被盗", "{{LOCATIONNAME}}：{{ITEMNAME}}因空间不足被盗", 0, 0,
     "{{ITEMNAME}}被盗（没有可存放的住宅或营业场所）"},
    {RE_LANG_ZH_CN, "itemRecovered", "LOCATIONNAME：ITEMNAME被警方追回", "{{LOCATIONNAME}}：{{ITEMNAME}}被警方追回", 0, 0,
     "{{ITEMNAME}}被警方追回"},
};
#define COUNT ((int)(sizeof TABLE / sizeof TABLE[0]))
#define MAX_VALUES RE_TEXT_PAIRS

int re_wording_count(void)
{
    return COUNT;
}

int re_wording_lang(int entry)
{
    return TABLE[entry].lang;
}

const char *re_wording_key(int entry)
{
    return TABLE[entry].key;
}

const char *re_wording_game_text(int entry)
{
    return TABLE[entry].game;
}

int re_wording_has(int lang)
{
    for (int i = 0; i < COUNT; i++)
        if (TABLE[i].lang == lang)
            return 1;
    return 0;
}

int re_wording_want(int lang, const char *key, unsigned len, unsigned caller_va)
{
    for (int i = 0; i < COUNT; i++) {
        const wording *w = &TABLE[i];
        if (w->lang == lang && strlen(w->key) == len && memcmp(w->key, key, len) == 0 && (w->size == 0 || caller_va - w->from < w->size))
            return i + 1;
    }
    return 0;
}

typedef struct {
    const char *name, *value; /* name points into the template, at the text between {{ and }} */
    unsigned name_len, value_len;
} placeholder;

/* first occurrence of `needle` in [from, end), or NULL */
static const char *find(const char *from, const char *end, const char *needle, unsigned n)
{
    for (; from + n <= end; from++)
        if (memcmp(from, needle, n) == 0)
            return from;
    return NULL;
}

/* Reads `text` as the template `tpl` with its placeholders filled in. Returns the number of placeholders, or -1
 * when the text is not of that shape. A value runs to the first occurrence of the literal that follows it; the
 * last literal has to end the text. */
static int match(const char *tpl, const char *text, unsigned len, placeholder out[MAX_VALUES])
{
    const char *t = text, *end = text + len, *tpl_end = tpl + strlen(tpl);
    int n = 0;
    for (;;) {
        const char *open = find(tpl, tpl_end, "{{", 2);
        unsigned literal = (unsigned)((open ? open : tpl_end) - tpl);
        if ((unsigned)(end - t) < literal || memcmp(t, tpl, literal) != 0)
            return -1;
        t += literal;
        if (open == NULL)
            return t == end ? n : -1;
        const char *close = find(open, tpl_end, "}}", 2);
        if (close == NULL || n == MAX_VALUES)
            return -1;
        tpl = close + 2;
        const char *next_open = find(tpl, tpl_end, "{{", 2);
        unsigned next_literal = (unsigned)((next_open ? next_open : tpl_end) - tpl);
        const char *stop;
        if (next_open == NULL) /* the last literal: it must be the tail of the text */
            stop = (unsigned)(end - t) >= next_literal ? end - next_literal : NULL;
        else
            stop = next_literal ? find(t, end, tpl, next_literal) : NULL; /* two placeholders in a row cannot be told apart */
        if (stop == NULL)
            return -1;
        out[n].name = open + 2;
        out[n].name_len = (unsigned)(close - open - 2);
        out[n].value = t;
        out[n].value_len = (unsigned)(stop - t);
        n++;
        t = stop;
    }
}

static const placeholder *value_of(const placeholder *values, int n, const char *name, unsigned name_len)
{
    for (int i = 0; i < n; i++)
        if (values[i].name_len == name_len && memcmp(values[i].name, name, name_len) == 0)
            return &values[i];
    return NULL;
}

/* Writes the entry's better text with these values over `text`: the `nameless` one when the first placeholder of
 * the better text came without a value (empty, or the "??" the game writes for an empty name). 0, and `text` as it
 * was, when a placeholder has no value among them or the result does not fit. */
static int rewrite(const wording *w, const placeholder *values, int n, char *text, unsigned cap, unsigned *len)
{
    char result[512];
    unsigned at = 0;
    const char *p = w->better, *p_end = p + strlen(p);
    const char *open = find(p, p_end, "{{", 2), *close = open ? find(open, p_end, "}}", 2) : NULL;
    if (w->nameless != NULL && close != NULL) {
        const placeholder *first = value_of(values, n, open + 2, (unsigned)(close - open - 2));
        if (first != NULL && (first->value_len == 0 || (first->value_len == 2 && memcmp(first->value, "??", 2) == 0))) {
            p = w->nameless;
            p_end = p + strlen(p);
        }
    }
    while (p < p_end) { /* a literal, then the value of the placeholder after it */
        open = find(p, p_end, "{{", 2);
        close = open ? find(open, p_end, "}}", 2) : NULL;
        unsigned literal = (unsigned)((close ? open : p_end) - p);
        const placeholder *value = close ? value_of(values, n, open + 2, (unsigned)(close - open - 2)) : NULL;
        if (close && value == NULL)
            return 0; /* a placeholder these values do not have */
        unsigned value_len = value ? value->value_len : 0;
        if (at + literal + value_len >= sizeof result)
            return 0;
        memcpy(result + at, p, literal);
        at += literal;
        if (value_len)
            memcpy(result + at, value->value, value_len);
        at += value_len;
        p = close ? close + 2 : p_end;
    }
    if (at >= cap)
        return 0;
    memcpy(text, result, at);
    *len = at;
    return 1;
}

/* 1 when `text` is what the game makes of the entry's own wording with the values of this call: every "{{" and "}}"
 * removed, then each name replaced by its value, in the call's order (FUN_0081d7c0). A carriage return counts for
 * nothing on either side: the file's lines end with CR LF, and what the game keeps of that was not looked at. */
static int game_made(const char *tpl, const re_text_values *call, const char *text, unsigned len)
{
    char made[1024], next[1024];
    unsigned at = 0;
    for (const char *p = tpl; *p; p++) {
        if ((p[0] == '{' && p[1] == '{') || (p[0] == '}' && p[1] == '}')) {
            p++;
            continue;
        }
        if (*p == '\r')
            continue;
        if (at + 1 >= sizeof made)
            return 0;
        made[at++] = *p;
    }
    for (int i = 0; i < call->count; i++) {
        unsigned name_len = (unsigned)strlen(call->name[i]), value_len = (unsigned)strlen(call->value[i]), to = 0;
        for (unsigned from = 0; from < at;) {
            int hit = name_len > 0 && from + name_len <= at && memcmp(made + from, call->name[i], name_len) == 0;
            unsigned add = hit ? value_len : 1;
            if (to + add >= sizeof next)
                return 0;
            memcpy(next + to, hit ? call->value[i] : made + from, add);
            to += add;
            from += hit ? name_len : 1;
        }
        memcpy(made, next, to);
        at = to;
    }
    unsigned seen = 0;
    for (unsigned i = 0; i < len; i++) {
        if (text[i] == '\r')
            continue;
        if (seen >= at || made[seen] != text[i])
            return 0;
        seen++;
    }
    return seen == at;
}

unsigned re_wording_apply(int entry, char *text, unsigned len, unsigned cap, const re_text_values *call)
{
    placeholder values[MAX_VALUES];
    if (entry < 0 || entry >= COUNT)
        return len;
    const wording *w = &TABLE[entry];
    int n = match(w->game, text, len, values);
    if (n >= 0 && rewrite(w, values, n, text, cap, &len))
        return len;
    /* The text does not hold every value the better text names: the language file misspells a name, has a brace too
     * few or carries another key's sentence, and the game left the value out. The call still has it. */
    if (call == NULL || !call->whole || !game_made(w->game, call, text, len))
        return len;
    n = 0;
    for (; n < call->count && n < MAX_VALUES; n++) {
        values[n].name = call->name[n];
        values[n].name_len = (unsigned)strlen(call->name[n]);
        values[n].value = call->value[n];
        values[n].value_len = (unsigned)strlen(call->value[n]);
    }
    rewrite(w, values, n, text, cap, &len);
    return len;
}
