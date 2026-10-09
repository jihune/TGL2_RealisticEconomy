/* The plugin's own texts in the game's eight languages. English and Korean are in the table of re_lang.c; every
 * other language has the texts translated before 2026-10-05 in that table and the rest in a file of its own
 * (re_lang_de.h, ...). A text a language has in neither place is the English one.
 * Every translation takes the same printf conversions in the same order as the English text, and a text of a
 * language is in exactly one of the two places (engine test T14). */
#ifndef RE_LANG_H
#define RE_LANG_H

/* English first: it is the fallback. The order is also the order of the columns in re_lang.c. */
enum { RE_LANG_EN, RE_LANG_KO, RE_LANG_DE, RE_LANG_ES, RE_LANG_FR, RE_LANG_PT_BR, RE_LANG_JP, RE_LANG_ZH_CN, RE_LANG_COUNT };

enum {
    /* stocks window: hover text of the company list; a line is at most about 34 columns */
    RE_MSG_PER_LOSS,
    RE_MSG_HOVER_YIELD,      /* %.1f; behind "PER 12.3, PBR 1.29" */
    RE_MSG_HOVER_UNDER,      /* what the two lines below begin with when there is a PBR */
    RE_MSG_HOVER_BANKRUPT,   /* %s colour on, %s the text above or nothing, %s colour off */
    RE_MSG_HOVER_MEASURE,    /* the same */
    RE_MSG_HOVER_WATCH,      /* %s colour on, %d month ends left, %.1f the fall that reaches the line, %s colour off */
    RE_MSG_HOVER_NEAR,       /* %.1f */
    RE_MSG_HOVER_FAIR,       /* %s amount, %+.0f */
    RE_MSG_HOVER_TAILWIND,   /* behind the line above: the industries add to the earnings */
    RE_MSG_HOVER_HEADWIND,   /* ... or take from them */
    RE_MSG_NO_FAIR,
    /* stocks window: behind the amounts of a company's first tab; short, the window shrinks a wide value */
    RE_MSG_ROW_FAIR, /* %s amount */
    RE_MSG_ROW_BANKRUPT,
    RE_MSG_ROW_MEASURE,
    RE_MSG_ROW_PER_MONTH, /* %s amount with its sign; behind the signs of an industry row */
    RE_MSG_ROW_WATCH,
    RE_MSG_ROW_OF_EQUITY, /* %.1f */
    RE_MSG_ROW_OF_PRICE,  /* %.1f */
    /* loan windows, debt tooltip, IPO window */
    RE_MSG_IPO_REST,    /* %s amount; appended to the game's "Retained Shares (25%)" */
    RE_MSG_EDU_FIXED,   /* replaces the game's "Variable interest rate per year" in the education loan window */
    RE_MSG_GRADE_QUOTE, /* %s grade; appended to the rate label of the mortgage window */
    RE_MSG_GRADE,       /* %s grade; appended to a rate label */
    /* monthly summary panel. A line is found again by how it begins, so the *_PREFIX texts are what the lines of
     * their group begin with (T14). */
    RE_MSG_GRADE_PREFIX,
    RE_MSG_GRADE_CHANGE, /* %s before, %s after */
    RE_MSG_FORECAST_PREFIX,
    RE_MSG_FORECAST_LOANS, /* %s prefix, %s amount */
    RE_MSG_FORECAST_CASH,  /* %s prefix, %s amount */
    RE_MSG_LOCK_PREFIX,
    RE_MSG_LOCK_RUNS,      /* how the line of a running lock begins; no other lock line begins like this */
    RE_MSG_LOCK_RUNS_REST, /* %s the text above, %lld hours */
    RE_MSG_LOCK_NONE,      /* %s prefix */
    RE_MSG_LOCK_OVER,      /* %s prefix */
    RE_MSG_AUTO_PREFIX,
    RE_MSG_AUTO_SKIPPED, /* %s prefix, %d purchases */
    RE_MSG_HELD_PREFIX,  /* the month's warning about shares held: "prefix" + "Name (state), Name (state)" */
    RE_MSG_HELD_CLOSE,   /* %.1f the fall that reaches the line; a state */
    RE_MSG_HELD_WATCH,   /* the same within six months of an emergency measure */
    RE_MSG_HELD_MORE,    /* %d companies left out */
    /* ticker */
    RE_MSG_LOCK_REFUSED, /* %lld hours */
    /* casino: the four games in the order of RE_CASINO, ticker lines for a result, the line of the summary panel
     * after a load (found again by RE_MSG_CASINO_BACK_PREFIX), the cooldown line of the activity tooltip */
    RE_MSG_CASINO_SLOTS,
    RE_MSG_CASINO_ROULETTE,
    RE_MSG_CASINO_BLACKJACK,
    RE_MSG_CASINO_BACCARAT,
    RE_MSG_CASINO_WON,    /* %s game, %s times the stake, %s amount gained */
    RE_MSG_CASINO_LOST,   /* %s game, %s stake */
    RE_MSG_CASINO_PLAYED, /* %s game */
    RE_MSG_CASINO_BEFORE, /* %s game: it was begun under the game's own rules, in a save from before the mod */
    /* a save from before the mod, at its first load: the month's line. The prefix, then %d businesses whose "buy
     * worn-out assets again" was switched off and %d employees whose automatic management was */
    RE_MSG_ADOPT_PREFIX,
    RE_MSG_ADOPT,
    RE_MSG_ADOPT_ASSETS, /* the prefix and %d businesses: no employee was under automatic management */
    RE_MSG_ADOPT_STAFF,  /* the prefix and %d employees: no business had the asset switch on */
    /* the same line's sentence about casino games taken out of the queue; it stands behind the prefix or behind the
     * sentence about the switches. %d games; the second with %d of them that had begun */
    RE_MSG_ADOPT_CASINO,
    RE_MSG_ADOPT_CASINO_BEGUN,
    RE_MSG_CASINO_BACK_PREFIX,
    RE_MSG_CASINO_BACK,      /* %s prefix, %d games, %s amount with its sign */
    RE_MSG_CASINO_TIP,       /* how the line over an activity's hover text begins; the prizes follow */
    RE_MSG_CASINO_TIP_PRIZE, /* %s ", " or nothing, %s chance in per cent, %s times the stake */
    /* business: what goes in brackets behind a candidate's "Conduct Interview", what follows "N candidates" above
     * the list, the line in the hover text of a staff card's wage icon, the line in the hover text of an offered
     * contract's payout icon */
    RE_MSG_STAFF_LINE, /* %s amount */
    RE_MSG_HIRE_ORDER,
    RE_MSG_STAFF_TIP, /* %s amount, %.0f efficiency in per cent */
    RE_MSG_OFFER_TIP, /* %+.1f per cent the payout is over the standard cost, %s amount left per hour of work */
    /* futures: what follows the asset's inflation rate in the window, and the line under that label's hover text */
    RE_MSG_FUT_EXPECTED, /* %.2f per cent */
    RE_MSG_FUT_TIP,
    RE_MSG_FUT_ITEM,    /* behind a ware's name in the futures list: %+.1f its yearly price change, per cent */
    RE_MSG_FUT_ITEM_AHEAD, /* the same with where the rate is expected to be: %+.1f now, %+.1f over twelve months */
    RE_MSG_FUT_ROW_TIP, /* above the hover text of such a row: %+.2f the rate now, then expected until 6, 12 and 24 months */
    RE_MSG_FUT_SORT,    /* the hover text of the list's second sort button, whose order the mod replaces */
    RE_MSG_CHART_HALF_YEAR, /* a chart's time button at six months, where the game's text would say "0 years" */
    RE_MSG_CHART_INDEX,     /* behind the title of the chart of all share prices */
    /* research kept fresh: the line of the monthly summary when the subscription is on. The prefix, then one of the two */
    RE_MSG_RESEARCH_PREFIX,
    RE_MSG_RESEARCH_PAID,  /* the prefix, %d companies, the fee */
    RE_MSG_RESEARCH_SHORT, /* the prefix, %d companies, the fee, the part of it the cash did not cover */
    /* a fee paid before a load, taken again ([48]): a line of the monthly summary with a prefix of its own (the two
     * above can stand in the same month), the same with a debt, and the ticker's line (the fee, %d companies) */
    RE_MSG_RESEARCH_KEPT_PREFIX,
    RE_MSG_RESEARCH_KEPT,
    RE_MSG_RESEARCH_KEPT_SHORT,
    RE_MSG_RESEARCH_KEPT_TICK,
    /* ... its switch in the stock window's list: the ticker lines of a Shift-click on a company (%s the company,
     * %s the fee a month) and of a Ctrl+Shift-click (%s the fee a company a month), the line for a company of the
     * board (%s the company), and the last line of the mod's part of a company's hover text, at most 34 columns */
    RE_MSG_RESEARCH_ONE_ON,
    RE_MSG_RESEARCH_ONE_OFF,
    RE_MSG_RESEARCH_ALL_ON,
    RE_MSG_RESEARCH_ALL_OFF,
    RE_MSG_RESEARCH_BOARD,
    /* REQUEST.md [44]: a subscription researches at once. _ONE_ON has the company and the fee taken now, _ALL_ON
     * %d companies researched now and the fee for them. _ONE_ON_DONE (%s the company): this month's research is
     * there already, nothing taken now. _ALL_HAS (%s the company): a Shift-click on a company while every company
     * is subscribed. _NO_CASH (%s the fee): the cash does not cover it, nothing switched. */
    RE_MSG_RESEARCH_ONE_ON_DONE,
    RE_MSG_RESEARCH_ALL_HAS,
    RE_MSG_RESEARCH_NO_CASH,
    /* the price against the month before (REQUEST.md [43]): a line of the hover text (%+.1f), and behind the price
     * of the company's first tab, alone (%+.1f) or with the fair price (%+.1f, %s amount) */
    RE_MSG_HOVER_CHANGE,
    RE_MSG_ROW_CHANGE,
    RE_MSG_ROW_CHANGE_FAIR,
    RE_MSG_HOVER_SUB_OFF,
    RE_MSG_HOVER_SUB_ON,
    RE_MSG_HOVER_SUB_ALL,
    RE_MSG_HOVER_SUB_BOARD,
    /* the line under it, about the Ctrl+Shift-click: what it would do now */
    RE_MSG_HOVER_ALL_ON,
    RE_MSG_HOVER_ALL_OFF,
    /* behind the date of the row "Last Researched" of a company's tab: the company is subscribed, or a household
     * member is on its board (the game's research button is not made for it once it has this month's research) */
    RE_MSG_ROW_SUB_ON,
    RE_MSG_ROW_SUB_BOARD,
    /* behind a company's name in a row of the list, inside the brackets: the fair price against the price (%+.0f),
     * and who keeps the research fresh. Short: the row has the name before it. */
    RE_MSG_ITEM_FAIR,
    RE_MSG_ITEM_SUB,
    RE_MSG_ITEM_BOARD,
    /* property: the month's line about a property for sale under its value. More than one: RE_MSG_HELD_MORE behind it */
    RE_MSG_PROPERTY_PREFIX,
    RE_MSG_PROPERTY_LINE, /* the prefix, the address, the gain after the purchase fees, the price */
    RE_MSG_PROPERTY_NONE, /* the prefix: the month's line when no property counts any more */
    /* business: the month's line about businesses that lose work efficiency to missing assets. The prefix, then for
     * each business one of the four; more of them: RE_MSG_HELD_MORE behind */
    RE_MSG_FIT_PREFIX,
    RE_MSG_FIT_BOTH,    /* the name, %.0f furnishings, %.0f assets */
    RE_MSG_FIT_FURNISH, /* the name, %.0f */
    RE_MSG_FIT_ASSETS,  /* the name, %.0f */
    RE_MSG_FIT_NONE,    /* the name: a job of it has none of one of its assets, and the game does not work such a job */
    /* experience: the line under the game's sentence "experience and education decay by N% every month" */
    RE_MSG_XP_TIP,
    /* business: the line under the game's hover text of the automatic management's switch in a staff row. What
     * that management does with a wage demand leaves no line in the monthly summary (REQUEST.md [38]). */
    RE_MSG_AUTO_TIP,
    RE_MSG_AUTO_TIP_ALL,   /* in front of it: with the mod the switch of a row is the switch of the whole business */
    RE_MSG_AUTO_TIP_SPARE, /* behind it: %d months; who is let go and who is replaced by a person of fewer hours */
    RE_MSG_AUTO_TIP_FILL,  /* behind that: a job with more work than hours gets a candidate hired */
    /* business: the line under the game's hover text of an advert's icon ("Click to activate / deactivate") */
    RE_MSG_ADVERTS_TIP,    /* how to hand the business's adverts to the mod; %.0f the awareness they are on up to, %s the fee */
    RE_MSG_ADVERTS_TIP_ON, /* the mod has them; the same two */
    RE_MSG_ADVERTS_KEPT,   /* the ticker line of the Shift-click that hands them over; %s the business */
    RE_MSG_ADVERTS_BACK,   /* ... and of the one that takes them back */
    RE_MSG_ADVERTS_LOCKED, /* the ticker line of a plain click on an advert the mod switches; %s the business */
    /* business: contracts and a whole business handed to the mod (note m33). Four ticker lines, %s the business;
     * then what goes behind either hover text of an advert icon: how those two hand-overs are made */
    RE_MSG_CONTRACTS_KEPT,
    RE_MSG_CONTRACTS_BACK,
    RE_MSG_ALL_KEPT,
    RE_MSG_ALL_BACK,
    RE_MSG_ADVERTS_TIP_ALL,
    /* the month's line about contracts the mod signed: the prefix, an item a business, the fee behind; its ticker
     * line; and the ticker line of a business whose offers were passed over for floor space */
    RE_MSG_CONTRACT_PREFIX,
    RE_MSG_CONTRACT_ITEM, /* %s ", " or nothing, %s the business, %d hours a month, %d months, %s the whole payout */
    RE_MSG_CONTRACT_FEE,  /* %s amount */
    RE_MSG_CONTRACT_TICK, /* %d contracts */
    RE_MSG_CONTRACT_ROOM, /* %s the business */
    /* premises short of floor space: the prefix with %s the business, then one of three behind %s the prefix: the
     * mod switched the game's star on (%s what the rent rises by), the premises are the largest of their kind, or
     * they are owned (%s what growing costs) */
    RE_MSG_PREMISES_PREFIX,
    RE_MSG_PREMISES_GROW,
    RE_MSG_PREMISES_MOST,
    RE_MSG_PREMISES_OWNED,
    /* business: a business with something of the mod's automation is closed or opened again (ticker); %s the
     * business */
    RE_MSG_FIRM_CLOSED,
    RE_MSG_FIRM_OPENED,
    /* business: a click on a control of a business handed over whole, which the lock does not pass on (ticker); %s
     * the business */
    RE_MSG_LOCKED,
    /* business: the month's lines about assets a business was short of. What was bought: the prefix, then for
     * each ware RE_MSG_ASSET_BUY_ITEM, RE_MSG_HELD_MORE when there are more, RE_MSG_ASSET_BUY_TOTAL behind. What
     * could not be bought: the other prefix, then for each ware one of the three */
    RE_MSG_ASSET_BUY_PREFIX,
    RE_MSG_ASSET_BUY_ITEM,  /* %s ", " or nothing, %s the business, %s the ware, %d units */
    RE_MSG_ASSET_BUY_TOTAL, /* %s amount */
    RE_MSG_ASSET_SHORT_PREFIX,
    RE_MSG_ASSET_SHORT_ROOM, /* %s ", " or nothing, %s the business, %s the ware */
    RE_MSG_ASSET_SHORT_CASH, /* the same */
    RE_MSG_ASSET_SHORT_NONE, /* the same, the asset tag in place of a ware */
    RE_MSG_ASSET_BUY_DEBT,   /* in place of _BUY_TOTAL when the cash did not cover it: %s all of it, %s the debt */
    RE_MSG_ASSET_TICK_BUY,   /* the ticker line of the purchases: %d units, %s what they cost */
    RE_MSG_ASSET_TICK_DEBT,  /* behind it when the cash did not cover it: %s the debt */
    /* the ticker line after the game has bought a worn-out asset again; the game's own is empty. %s the business,
     * %s the amount */
    RE_MSG_ASSET_REPLACED,
    /* the hover texts of four sort buttons of the stock window's list, whose orders the mod replaces: in this
     * order, as RE_STOCK_BY_CHEAP, _DEAR, _CHANGE and _CAP of re_stock.h */
    RE_MSG_SORT_CHEAP,
    RE_MSG_SORT_DEAR,
    RE_MSG_SORT_CHANGE,
    RE_MSG_SORT_CAP,
    RE_MSG_COUNT
};

/* The game's language folder ("ko", "pt-br", ...) as a column; RE_LANG_EN for a language the table does not have. */
int re_lang_of(const char *folder);
const char *re_lang_folder(int lang);
/* Never NULL. */
const char *re_lang_text(int lang, int msg);
/* 1 when the language has its own text, 0 when re_lang_text falls back on English. */
int re_lang_has(int lang, int msg);
/* In how many places the language has this text: the table, the language's file. 1 is right. */
int re_lang_places(int lang, int msg);

#endif
