/* Addresses inside TGL2.exe v1.0322 (image base 0x400000) and the bytes expected there.
 * Evidence for each address: INVESTIGATION_REPORT.md and analysis/notes (m1, m2, m4, m9, a6, b2, b4, b6, b7). */
#ifndef RE_SITES_H
#define RE_SITES_H

#include <windows.h>

#define RE_PIN_TIMESTAMP 1789284493u
#define RE_PIN_SIZE_OF_IMAGE 0x6d4000u
#define RE_IMAGE_BASE 0x400000u

/* Each site belongs to one group. A feature is switched on only when every site of the groups it needs matches. */
enum {
    RE_GROUP_CORE,
    RE_GROUP_FINANCE,
    RE_GROUP_CREDIT,
    RE_GROUP_MONEY,
    RE_GROUP_TEXT,
    RE_GROUP_GUARD,
    RE_GROUP_MONTH,
    RE_GROUP_IPO,
    RE_GROUP_BOARD,
    RE_GROUP_STOCKS,
    RE_GROUP_CASINO,
    RE_GROUP_BUSINESS,
    RE_GROUP_MEMORY,
    RE_GROUP_PROPERTY,
    RE_GROUP_TRACE,
    RE_GROUP_EXPERIENCE,
    RE_GROUP_NAMES,
    RE_GROUP_COUNT
};

/* call sites that get their target replaced */
#define RE_VA_CALL_POSTLOAD 0x006c1304u /* call 0x691880, ecx = main, last step of loading or starting a game */
#define RE_VA_CALL_MONTHEND 0x0068ebb2u /* call 0x695020, ecx = main, when the month tick reaches 727 */

#define RE_VA_CALL_MONTHSTART 0x0068ed9cu /* call 0x695430, ecx = main: the month start, after the month end and before
                                          * the summary panel opens; it makes the property market anew (note b17) */

/* functions */
#define RE_VA_POSTLOAD 0x00691880u
#define RE_VA_MONTHSTART 0x00695430u
#define RE_VA_MONTHEND 0x00695020u
#define RE_VA_GETTER 0x00807530u       /* float stats attribute by name: ecx = object, std::string by value */
#define RE_VA_NET_WORTH 0x00542300u    /* ecx = PFM, returns int64 cents */
#define RE_VA_TOTAL_ASSETS 0x00542330u /* ecx = PFM, one stack flag (1 = houses net of mortgage) */
#define RE_VA_DEBT 0x004d0600u         /* ecx = PFM + 0xa8 */
#define RE_VA_SAVINGS 0x004d0c30u      /* ecx = PFM + 0xa8 */
#define RE_VA_INVESTMENTS 0x004d0690u  /* ecx = PFM + 0xa8 */
#define RE_VA_HOUSES 0x00550540u       /* ecx = [PFM + 0x10], one stack flag */
#define RE_VA_STOCKS 0x00535650u       /* ecx = [PFM + 0x18] */
#define RE_VA_BANKRUPT_END 0x00547560u /* ecx = PFM */
#define RE_VA_INSTALMENT 0x004d0110u   /* ecx = PFM + 0xa8; activity id, principal lo, hi, months, float stored rate */
#define RE_VA_CHANGE_MONEY 0x00542920u /* ecx = PFM; int64 amount, tag, ... (ret 0x18): every cash movement */
#define RE_VA_RAND_CHANCE 0x00672940u  /* ecx = RandUnit, xmm1 = probability: one uniform draw, numCalls + 1 */
#define RE_VA_DEBT_PASS 0x004cefe0u    /* the monthly debt routine; calls of the cash function from inside it are instalments */
#define RE_DEBT_PASS_SIZE 3887u
#define RE_VA_TRANSLATE 0x0081d7c0u    /* ecx = text manager; result string pointer, key and ten values by value (ret 0x10c) */
#define RE_VA_STR_ASSIGN 0x00405950u   /* ecx = std::string; pointer, length (ret 8) */
#define RE_VA_STR_FREE 0x004058b0u     /* ecx = std::string: releases its memory */
#define RE_VA_STR_COPY 0x00405500u     /* ecx = a new std::string; the string to copy (ret 4) */
/* call 0x405500 in the main menu's setup: the version text ("v1.03.22") is copied for the label in the corner, which
 * is anchored at its right end. The menu's own copy also goes to its buttons and stays as it is. */
#define RE_VA_CALL_MENU_VERSION 0x006bf239u
/* The game's own way to write an amount: currency symbol, exchange rate and denomination as the options and the save
 * have them (the option "usdOnly" decides between dollars and the city's currency). */
#define RE_VA_MONEY_TEXT 0x00502ab0u /* ecx = [PFM + 0x78]; result string, int64 cents, 1 = with the symbol, 0 (ret 0x14) */
#define RE_PFM_MONEY_FORMAT 0x78
/* A line of the monthly summary panel is a 0x90-byte record: text at +0, icon name at +0x30 (both std::string). */
#define RE_VA_EVENT_CTOR 0x00516a70u /* ecx = record */
#define RE_VA_EVENT_COPY 0x0046f7f0u /* ecx = destination, one stack argument: source (ret 4) */
#define RE_VA_EVENT_ADD 0x0065b0f0u  /* ecx = event list owner; the record by value, then a flag: 1 = append (ret 0x94) */
/* the monthly summary panel as a window (note b14): the rows are built from a copy of the month's list when the
 * panel opens, and again by this function, which the panel's own filter buttons and page arrows call */
#define RE_VA_SUMMARY_ROWS 0x007ce700u   /* ecx = panel, no stack arguments, plain ret; does nothing while not shown */
#define RE_VA_SUMMARY_SLOT 0x007cee60u   /* the panel's vtable slot 11: jmp to the function above */
#define RE_VA_SUMMARY_VTABLE 0x008f7998u /* UISummary */
#define RE_VA_SUMMARY_SHOWN 0x007ce72au  /* cmp byte ptr [ebx + 0xc], 0: the function's own test */
#define RE_VA_POSTLOAD_PANEL 0x0069199fu /* mov ecx, [edi + 0x3b4] / add ecx, 0x9b4 / call open: a load opens the panel */
#define RE_MAIN_UI 0x3b4                 /* pointer to the UI object inside the main object */
#define RE_UI_SUMMARY 0x9b4              /* the panel inside the UI object */
#define RE_SUMMARY_SHOWN 0xc             /* byte */
#define RE_SUMMARY_PAGER 0xf0            /* pointer, set when the panel has been opened */
#define RE_VA_EVENT_DTOR 0x00516b80u /* ecx = record */
#define RE_VA_ADD_DEBT 0x004ce2b0u   /* ecx = PFM + 0xa8; the 0x60-byte debt record by value: gives it an id and stores it */
#define RE_VA_CALL_EDU_ADD_DEBT 0x004d89e3u /* that call inside the enrol routine: the new education loan */
#define RE_VA_CALL_BUY_ADD_DEBT 0x0043af1eu /* that call inside the purchase routine: the mortgage of the home being bought */
#define RE_DEBTINV_OWNER 0x8                /* DebtInvM: pointer to the object that holds the id counter */
#define RE_OWNER_LAST_ID 0x36c              /* the id given out last; a new debt gets this + 1 */
/* The windows that show a loan rate fetch the rate line's text themselves, so the address a text request returns
 * to tells them apart. The two keys are shared with the savings windows, which must stay as they are. */
#define RE_VA_PERSONAL_WINDOW 0x0062f180u /* personal loan window */
#define RE_PERSONAL_WINDOW_SIZE 7512u
#define RE_VA_MORTGAGE_WINDOW 0x00631340u /* mortgage window */
#define RE_MORTGAGE_WINDOW_SIZE 9884u
#define RE_VA_EDU_WINDOW 0x00634f00u /* education-loan window */
#define RE_EDU_WINDOW_SIZE 5981u
#define RE_VA_DEBT_TIP_VARIABLE 0x0061ba57u /* a debt's tooltip (0x0061b330), rate line of its variable-rate branch */
#define RE_VA_CALL_DEBT_TIP 0x0061ba52u     /* the call that returns there */
#define RE_VA_DEBT_TIP_RECORD 0x0061b366u   /* mov eax, [ebp + 0xc]: the tooltip's debt record, its second argument */
#define RE_DEBT_TIP_FRAME_RECORD 0xc
#define RE_DEBT_RECORD_ACTIVITY 0xc /* the loan product the debt was made from */
/* A fixed mortgage rate is variable x 1.1 + 0.03, computed in two places from two shared constants (doubles). */
#define RE_VA_FIXED_MUL_QUOTE 0x0063acb6u  /* mulsd xmm0, [1.1]: the rate the mortgage window shows */
#define RE_VA_FIXED_ADD_QUOTE 0x0063acbeu  /* addsd xmm0, [0.03] */
#define RE_VA_FIXED_MUL_ACCEPT 0x00634460u /* the same two when the mortgage is accepted: the rate that is stored */
#define RE_VA_FIXED_ADD_ACCEPT 0x00634468u
#define RE_VA_CONST_1_1 0x008fa110u
#define RE_VA_CONST_0_03 0x008f9fb0u

/* the player's trades (card b4); none returns a value */
#define RE_VA_STOCK_BUY 0x00535120u    /* ecx = stock market; stock id, quantity (ret 8); also the monthly auto transfer */
#define RE_VA_FUTURES_OPEN 0x0040ccf0u /* ecx = derivatives; asset, months, units, side (ret 0x10) */
/* Selling has no function of its own: the stock window's sell handler (0x007ba8d0) does it inline. The handler is a
 * mouse callback that runs for every click while the window is open, so its entry is no place to refuse a sale. The
 * branch that sells starts with one call and is entered only with a quantity; with none, the handler leaves through
 * RE_VA_SELL_SKIP, and the stack is the same at both points. */
#define RE_VA_SELL_TEST 0x007babe9u      /* test esi, esi / je RE_VA_SELL_SKIP: nothing to sell */
#define RE_VA_CALL_SELL_STEP 0x007babf9u /* the first call of the selling branch, ecx only, no stack arguments */
#define RE_VA_SELL_STEP 0x00454ae0u      /* its target */
#define RE_VA_SELL_SKIP 0x007bae10u      /* the handler's way out */
#define RE_VA_POST_MESSAGE 0x007e0010u /* ecx = the ticker's message list; type, text and two more strings by value (ret 0x4c) */
#define RE_DEBTINV_UI 0x34             /* DebtInvM: pointer to the object that holds the ticker at the top of the screen */
#define RE_UI_MESSAGES 0x478           /* the ticker's message list inside it */

/* What a month end draws (note m37 Q4): two calls inside the month-end routine 0x00695020, each with ecx only and no
 * stack arguments, to functions that end in a plain `ret`. Between the two the routine calls the industries' cycles
 * (0x004d5300), the interest rates (0x004d3cc0) and 0x004d6310; the property market's month comes after them. */
#define RE_VA_CALL_ECONOMY_MONTH 0x00695272u /* call 0x4d37c0, ecx = economy: growth and a crash */
#define RE_VA_ECONOMY_MONTH 0x004d37c0u
#define RE_VA_ECONOMY_MONTH_RET 0x004d3ca9u
#define RE_VA_CALL_STOCKS_MONTH 0x00695297u /* call 0x533340, ecx = stock market: the month of every listed company */
#define RE_VA_STOCKS_MONTH 0x00533340u
#define RE_VA_STOCKS_MONTH_RET 0x00533566u
/* The property market's month start, called by the month-start routine 0x00695430 with ecx only; a plain `ret`. It
 * empties the list of properties for sale and fills it again, then makes the month's offer for every property the
 * household has put up for sale or for rent; all of it drawn from the market stream (note b17). */
#define RE_VA_CALL_RESTATE_MONTH 0x0069558au /* call 0x54a730, ecx = [main + 0x390] */
#define RE_VA_RESTATE_MONTH 0x0054a730u
#define RE_VA_RESTATE_MONTH_RET 0x0054c87cu
/* Inside it the game fills the list for rent (0x0054ded0) and the list for sale (0x0054c9c0): each walks a copy of
 * the map of sites and starts a site with a call of 0x0080e290 for the site's type, [node + 0x18]. The node's
 * [+ 0x14] is what the making of a listing gets as the site and looks its street name up with (0x0054cef0). Then
 * 0x0054f010, and after it the offers for the properties the household sells or lets. */
#define RE_VA_RENT_SITE_TYPE 0x0054e020u      /* mov eax, [edi + 0x18]: in the walk for rent the node is in edi */
#define RE_VA_CALL_RENT_SITE_DATA 0x0054e058u /* call 0x80e290 */
#define RE_VA_SALE_SITE_TYPE 0x0054cad0u      /* mov eax, [esi + 0x18]: in the walk for sale it is in esi */
#define RE_VA_CALL_SALE_SITE_DATA 0x0054cb08u /* call 0x80e290 */
#define RE_VA_SITE_DATA 0x0080e290u
#define RE_VA_SALE_SITE_ID 0x0054cd8au        /* lea eax, [esi + 0x14]: the site handed to the making of a listing */
#define RE_VA_CALL_OWN_VALUES 0x0054a787u     /* call 0x54f010 */
#define RE_VA_OWN_VALUES 0x0054f010u
#define RE_SITE_NODE_ID 0x14

/* The listing of a public company, FUN_005362a0 (notes b5 and m12). The IPO window runs it without committing, for
 * its preview; that run leaves before the investment sites and the fee. */
#define RE_VA_IPO_KEEP_FOUNDER 0x005366a6u /* mulss xmm0, [esi + 0x298]: founder's shares = N x fraction */
#define RE_VA_IPO_KEEP_PUBLIC 0x005366b3u  /* movss xmm1, [esi + 0x298]: the public's shares = N x (1 - fraction) */
#define RE_VA_IPO_KEEP_GETTER 0x00535c50u  /* movss xmm0, [ecx + 0x298] / ret: the fraction for the window */
#define RE_VA_IPO_OFFER_HI 0x005366fcu     /* push [ebp - 0x3bc]: offer price = V / founder's shares, the divisor */
#define RE_VA_IPO_OFFER_LO 0x00536708u     /* push [ebp - 0x3c0] */
#define RE_VA_IPO_EQUITY_HI 0x0053678bu    /* push [ebp - 0x3d8]: equity a share = (price x public shares + V) / N */
#define RE_VA_IPO_EQUITY_LO 0x00536791u    /* push [ebp - 0x3f4] */
#define RE_VA_IPO_INVEST_HI 0x00536bd5u    /* push [ebp - 0x3bc]: investment by type = value x N / founder's shares */
#define RE_VA_IPO_INVEST_LO 0x00536bdbu    /* push [ebp - 0x3c0] */
#define RE_VA_CALL_IPO_STEPS 0x00536783u   /* call 0x53cb50: the twelve price steps before the listing */
#define RE_VA_IPO_STEPS 0x0053cb50u /* ecx = address of the price; offer price, earnings a share, context; caller cleans */
#define RE_VA_CALL_IPO_FEE 0x00536e2cu     /* call 0x542920: the listing fee, the listing's only cash movement */
#define RE_VA_IPO_ALL_SHARES 0x00536725u   /* push [ebp - 0x3ec]: N, as the divisor of the earnings a share */
#define RE_VA_IPO_GRANT 0x00536dd9u        /* mov eax, [ebp - 0x3e0] / mov ecx, esi: the company, for the share grant */
#define RE_VA_IPO_GRANT_SHARES 0x00536de1u /* push 0 / push [ebp - 0x3c0]: lot cost 0, founder's shares */
/* frame of FUN_005362a0, relative to its ebp, as the three instructions above show it */
#define RE_IPO_FRAME_SHARES (-0x3f0)  /* int64, all shares */
#define RE_IPO_FRAME_FOUNDER (-0x3c0) /* int64, the founder's shares */
#define RE_IPO_FRAME_COMPANY (-0x3e0) /* pointer to the company being listed */
#define RE_COMPANY_PRICE 0x100        /* int64, cents a share */
#define RE_COMPANY_SHARES 0x108       /* int64 */
/* The IPO window (0x00497ca0) turns price / equity per share of the previewed company into a quality letter. */
#define RE_VA_IPO_GRADE_BASE 0x00499705u /* subss xmm1, [1.0] */
#define RE_VA_IPO_GRADE_AAA 0x00499710u  /* comisd xmm0, [0.15] */
#define RE_VA_IPO_GRADE_AA 0x00499721u   /* comiss xmm1, [0.1] */
#define RE_VA_IPO_GRADE_A 0x00499731u    /* comiss xmm1, [0.05]; B is `comiss xmm1, xmm0` against zero */
#define RE_VA_IPO_GRADE_C 0x00499750u    /* comiss xmm1, [-0.05] */
#define RE_VA_IPO_GRADE_D 0x00499760u    /* comiss xmm1, [-0.1] */
#define RE_VA_CONST_F32_1 0x008f9f80u
#define RE_VA_CONST_F64_0_15 0x008fa000u
#define RE_VA_CONST_F32_0_1 0x008f9e7cu
#define RE_VA_CONST_F32_0_05 0x008f9e74u
#define RE_VA_CONST_F32_M0_05 0x008fa69cu
#define RE_VA_CONST_F32_M0_1 0x008fa6a4u

/* the board of a listed company (note b11) */
#define RE_VA_BOARD_SCORE 0x0053efa0u      /* a candidate's score: ecx = company, (score, person), ret 8 */
#define RE_VA_CALL_BOARD_ELECT 0x0053e87cu /* its call when an election is decided */
#define RE_VA_CALL_BOARD_RANK 0x007b212au  /* its call that orders the candidates in the board tab */
#define RE_VA_OWNERSHIP 0x00535010u        /* ecx = stock market, (company id), ret 4: the household's share, in xmm0 */
#define RE_VA_SCORE_INIT 0x00414350u       /* constructs a score; the score function calls it on its result first */
#define RE_VA_CALL_SCORE_INIT 0x0053efe3u  /* that call */
#define RE_VA_SCORE_FREE 0x00414400u       /* destroys a score: ecx = the score, plain ret */
#define RE_VA_CALL_SCORE_FREE 0x007b2166u  /* the board tab's own call of it, on the score of the redirected call */
/* instructions that show the layout the board feature reads */
#define RE_VA_BOARD_NOMINEES 0x0053e85au /* mov ecx, [ebx + 0x158] / mov esi, [ecx]: the list, ebx = company */
#define RE_VA_BOARD_PERSON 0x0053e870u   /* push [esi + 8] / lea edi, [esi + 8] / mov ecx, ebx: a node's person */
#define RE_VA_BOARD_KEY 0x0053e881u      /* lea eax, [ebp - 0x34]: the total, 0x14 into the score at [ebp - 0x48] */
#define RE_VA_BOARD_RANK_ARGS 0x007b211bu /* lea ecx, [ebp - 0x2ac] / push [eax]: the tab's copy of the company */
#define RE_VA_BOARD_NOTICE 0x0053d88cu   /* cmp byte ptr [ebx + 0x17c], 0: tell the player about an election */
#define RE_VA_HOUSEHOLD_IDS 0x0065c8aau  /* mov eax, [edx + 0x36c]: start of the household's person ids */
#define RE_VA_COMPANY_PEOPLE 0x0053f832u /* mov ecx, [edi + 0x98]: the character manager, edi = company */
#define RE_VA_COMPANY_ID 0x0053f893u     /* mov eax, [edi + 0x134] */
#define RE_VA_COMPANY_MARKET 0x0053f899u /* mov esi, [edi + 0x78] */
#define RE_VA_MARKET_COMPANIES 0x005300b1u /* mov eax, [ebx + 0x164] / mov esi, [eax]: the listed companies, ebx = market */
#define RE_VA_MARKET_COMPANY 0x005300c9u   /* lea ecx, [esi + 0x18] / call: the company inside a node of that map */
#define RE_VA_IPO_MARKET 0x00536dc6u       /* mov esi, [ebp - 0x3ac] / mov ecx, esi: the market in the listing's frame */
#define RE_IPO_FRAME_MARKET (-0x3ac)
#define RE_MARKET_COMPANIES 0x164    /* std::map<company id, company> */
#define RE_MARKET_NODE_COMPANY 0x18
#define RE_COMPANY_MARKET 0x78
#define RE_COMPANY_PEOPLE 0x98
#define RE_COMPANY_ID 0x134
#define RE_COMPANY_NOMINEES 0x158     /* std::list<int>: sentinel node pointer, size; a node is next, previous, id */
#define RE_COMPANY_BOARD_NOTICE 0x17c /* byte */
/* research kept fresh (note m21_research_fresh.md) */
#define RE_VA_RESEARCH 0x00532bd0u      /* ecx = market; company id (ret 4): what the action "Research Stock" does at its end */
#define RE_VA_RESEARCH_RET 0x00532f3cu
#define RE_VA_BOARD_MEMBERS 0x0053f549u /* lea eax, [ecx + 0x150] in the game's "is this person on the board" */
#define RE_COMPANY_BOARD_MEMBERS 0x150  /* std::list<int>, like the nominees */
#define RE_TAG_FEES 2063                /* the finance tag "financial transaction fees" of data/idmap.txt */
#define RE_PEOPLE_HOUSEHOLD 0x36c     /* int vector: begin, end */
#define RE_SCORE_TOTAL 0x14           /* float */
#define RE_SCORE_BYTES 0x20

/* The stocks window (note b12). The address a request for a text or a row returns to names the place, and the
 * caller's frame holds the company it is about. */
#define RE_VA_STOCK_ROW 0x007b6d30u     /* one row of a tab: ecx = window; layer, position, red flag, label, value */
#define RE_VA_STOCK_ROW_RET 0x007b70c9u /* its `ret 0x3c`: three words and two std::string by value */
/* its calls for the four amounts of a company's first tab (0x007b5010) */
#define RE_VA_CALL_ROW_PRICE 0x007b53f0u
#define RE_VA_CALL_ROW_EQUITY 0x007b553bu
#define RE_VA_CALL_ROW_EARNINGS 0x007b5686u
#define RE_VA_CALL_ROW_DIVIDEND 0x007b57ccu
#define RE_VA_STOCK_TAB_COMPANY 0x007b5074u /* mov [ebp - 0x148], edi: the window's copy of the company */
#define RE_STOCK_TAB_FRAME_COMPANY (-0x148)
/* the price of a company month by month (note m23): the market keeps a statement for every company - the class of
 * the household's cash flow statement - and writes the price into it once a month, as income of the tag 2711 */
#define RE_VA_MARKET_HISTORY 0x00534c55u     /* add esi, 0x208: std::map<company id, statement> */
#define RE_MARKET_HISTORY 0x208
#define RE_VA_HISTORY_PRICE 0x00534c6cu      /* push 0xa97 */
#define RE_STAT_PRICE 2711
#define RE_HISTORY_NODE_STATEMENT 0x18       /* the statement in its node (0x0040d400 returns node + 0x18) */
#define RE_VA_COMPANY_RESEARCHED 0x007b5fbfu /* mov ecx, [ebp - 0xf0] / cmp dword ptr [ecx + 0x2c], 0: researched at all */
#define RE_VA_CALL_STOCK_LIST 0x007ab74bu   /* asks for "clickView", the hover text of a row of the company list */
#define RE_VA_STOCK_LIST_CURSOR 0x007ab22au /* lea edx, [edi + 0x48] / mov [ebp - 0x130], edx: 0x48 into the row's company */
#define RE_VA_STOCK_LIST_ID 0x007ab25du     /* mov edi, [edx + 0xec]: its id, 0x134 into the company */
#define RE_STOCK_LIST_FRAME_CURSOR (-0x130)
#define RE_STOCK_LIST_CURSOR_OFFSET 0x48
#define RE_VA_COMPANY_PRICE 0x007b52bfu     /* mov edi, [esi + 0x100] */
#define RE_VA_COMPANY_EQUITY 0x007b540au    /* mov edi, [eax + 0x120] */
#define RE_VA_COMPANY_EARNINGS 0x007b554cu  /* mov edi, [eax + 0x118] */
#define RE_VA_COMPANY_DIVIDEND 0x007b5691u  /* mov edi, [eax + 0x110] */
#define RE_VA_COMPANY_DESPERATE 0x00530474u /* mov [eax + 0x28], ecx: the month of an emergency measure */
#define RE_COMPANY_DESPERATE_MONTH 0x28
#define RE_COMPANY_RESEARCH_MONTH 0x2c
#define RE_COMPANY_DIVIDEND 0x110 /* int64, cents a share and year */
#define RE_COMPANY_EARNINGS 0x118 /* the same */
#define RE_COMPANY_EQUITY 0x120   /* int64, cents a share */
/* What the industries do to a company's earnings: the "research stock" action (0x00532bd0) walks the company's
 * list of industries and stores yearly rate x 5 x weight x 0.005 for each; the research block of the first tab
 * (0x007b5870) shows one row per stored value. The monthly step adds the same terms to the earnings. */
#define RE_VA_COMPANY_INDUSTRIES 0x00532d56u /* mov esi, [eax] / lea eax, [ebx + 8]: first element of the company's vector */
#define RE_VA_INDUSTRY_WEIGHT 0x00532d7bu    /* movups xmm1, [esi] / movq xmm0, [esi + 0x10]: int64 weight at +0 */
#define RE_VA_INDUSTRY_ID 0x00532dbdu        /* movups xmm0, [esi] / psrldq xmm0, 0xc: the industry's id at +0xc */
#define RE_VA_INDUSTRY_NEXT 0x00532e33u      /* add esi, 0x18 */
#define RE_VA_MARKET_RATES 0x00532da8u       /* mov eax, [eax + 0xd4] / add eax, 0xb8: market -> economy -> rate per industry */
#define RE_VA_CALL_ROW_INDUSTRY 0x007b6c54u  /* a row of the research block: esi = node of the stored map */
#define RE_VA_INDUSTRY_ROW_ID 0x007b6bb5u    /* mov eax, [esi + 0x10]: the industry of that row */
#define RE_STOCK_RESEARCH_FRAME_COMPANY (-0xf0) /* RE_VA_COMPANY_RESEARCHED reads it */
#define RE_COMPANY_INDUSTRIES 0x0 /* std::vector: first, last */
#define RE_INDUSTRY_BYTES 0x18
#define RE_INDUSTRY_ID 0xc
#define RE_MARKET_ECONOMY 0xd4
#define RE_ECON_RATES 0xb8 /* std::map<industry id, float>: yearly price change of the industry */
/* the player's shares and a company's name, for the month's warning about shares held */
#define RE_VA_MARKET_HOLDINGS 0x00534fd4u /* lea esi, [ecx + 0x258]: std::map<company id, int shares> */
#define RE_VA_COMPANY_COPY 0x007ac7b4u    /* lea ecx, [ebp - 0x260]: the header's copy of the company */
#define RE_VA_COMPANY_NAME 0x007ac8a5u    /* lea eax, [ebp - 0x254]: its name, a std::string 0xc into the company */
#define RE_HEADER_FRAME_COMPANY (-0x260)
#define RE_MARKET_HOLDINGS 0x258
#define RE_COMPANY_NAME 0xc

/* The casino (note b15). The four games are activities; the plugin answers their attributes, lets the game book
 * nothing for them, and books the result itself when the action ends. */
#define RE_VA_ATTRIBUTE 0x00808b10u /* a float attribute of an object: ecx = [object + 0x18], edx = its id; type, section
                                       and name as std::string by value, destroyed by the callee; plain ret; xmm0 */
#define RE_VA_ACTION_MONEY 0x00438440u     /* the money of an action: ecx = the person's actions; result, queue record,
                                              activity object, section, name, 8 bytes, phase, afford flag */
#define RE_VA_ACTION_MONEY_RET 0x0043873cu /* its `ret 0x4c` */
#define RE_VA_CALL_MONEY_START 0x004401e7u /* the first hour of an action: phase 1, a cost is booked */
#define RE_VA_CALL_MONEY_END 0x00439be2u   /* its last hour: phase 0, an income is booked */
#define RE_VA_MONEY_PHASE 0x004385e2u      /* cmp byte ptr [ebp + 0x4c], 0 */
#define RE_ACTION_MONEY_PHASE 0x48         /* the same slot from esp at the function's entry */
#define RE_VA_ACTION_FINANCE 0x0043864du   /* mov ecx, [eax + 0xc]: the household's finance object, eax = the actions */
#define RE_VA_ACTION_PRICES 0x004384afu    /* mov ecx, [esi + 0x10]: the `this` of RE_VA_ACTIVITY_MONEY */
#define RE_VA_ACTION_PEOPLE 0x00439ae3u    /* mov ecx, [edi + 0x3c]: the household */
#define RE_VA_RECORD_COUNT 0x004384feu     /* mov eax, [eax + 0x88]: what the game multiplies the amount by */
#define RE_VA_ACTIVITY_ID 0x0043865bu      /* mov eax, [eax + 0xc]: the activity object's id */
#define RE_ACTION_FINANCE 0xc
#define RE_ACTION_PRICES 0x10
#define RE_ACTION_PEOPLE 0x3c
#define RE_RECORD_COUNT 0x88
#define RE_VA_ACTIVITY_MONEY 0x004d4740u /* what the lists show: ecx = prices; object, section, name (ret 0x34); int64
                                            cents, negative = a cost */
#define RE_VA_MAIN_PEOPLE 0x0068ea76u      /* mov esi, [edi + 0x3b8] */
#define RE_MAIN_PEOPLE 0x3b8
#define RE_VA_PEOPLE_COOLDOWNS 0x0065c4a4u /* lea esi, [ecx + 0x420]: std::map<activity id, month of the household's last> */
#define RE_PEOPLE_COOLDOWNS 0x420
#define RE_VA_MAP_AT 0x00420620u /* std::map<int, int>::operator[]: ecx = map, pointer to the key (ret 4); pointer to the value */
/* A person's queue of actions (notes m1, m27). FUN_0065c880 gives a person of the household: ecx = the household; a
 * shared pointer to fill (the person, the count), the person's id, whether a miss is reported (ret 0xc). The queue
 * is a std::list at person + 0x648: the head node, then how many. A node is 0xa4 bytes: next, previous, and the
 * record that FUN_00668a50 reads from a save: its id, its hours, the hours gone, a string, three more, the activity
 * at + 0x6c. For a right click the game takes a record out (FUN_00705d30) by unlinking the node, releasing its four
 * strings and deleting it, as FUN_00440cd0 does for all of them at a member's month end; no money moves and
 * nothing else of the game knows the record. */
#define RE_VA_PERSON_OF 0x0065c880u
#define RE_VA_PERSON_OF_RET 0x0065c924u    /* ret 0xc */
#define RE_VA_PERSON_QUEUE 0x006607c2u     /* lea ecx, [esi + 0x648], in the month end of a member */
#define RE_PERSON_QUEUE 0x648
#define RE_VA_QUEUE_STRINGS 0x00440cf2u    /* lea ecx, [esi + 0x5c], and + 0x44, + 0x2c, + 0x14 eight bytes on each */
#define RE_QUEUE_NODE_STRING 0x14          /* the first of the four, 0x18 bytes each */
#define RE_VA_QUEUE_NODE_BYTES 0x00440d12u /* push 0xa4 */
#define RE_QUEUE_NODE_BYTES 0xa4
#define RE_VA_CALL_QUEUE_DELETE 0x00440d18u
#define RE_VA_DELETE 0x00831fe8u           /* operator delete: the block, its size; the caller cleans */
#define RE_VA_RECORD_GONE 0x00668ad5u      /* lea eax, [ebx + 8]: "a_actionPassTicks" */
#define RE_RECORD_HOURS 4
#define RE_RECORD_GONE 8
#define RE_VA_RECORD_ACTIVITY 0x00668b2eu  /* lea eax, [ebx + 0x6c]: "a_actionObjInfoID" */
#define RE_RECORD_ACTIVITY 0x6c
/* ... and then tells the windows: FUN_006a3ec0, ecx = the object at person + 0x5d8; the id at + 8 of the object at
 * person + 0x640, and a copy of the record by value, which the callee releases (ret 0xa0: 4 and the record's
 * 0x9c). FUN_0042f290 makes the copy: ecx = where, the record (ret 4). Without it the queue's row of icons keeps
 * the action until the clock next moves (run 372). */
#define RE_VA_RECORD_COPY 0x0042f290u
#define RE_VA_RECORD_COPY_RET 0x0042f3abu  /* ret 4 */
#define RE_VA_QUEUE_GONE 0x006a3ec0u
#define RE_VA_QUEUE_GONE_RET 0x006a3fe8u   /* ret 0xa0 */
#define RE_VA_PERSON_SELF 0x00705fccu      /* mov eax, [ebx + 0x640] ... */
#define RE_PERSON_SELF 0x640
#define RE_VA_PERSON_SELF_ID 0x00705fd8u   /* ... push dword ptr [eax + 8] */
#define RE_PERSON_SELF_ID 8
#define RE_VA_PERSON_WINDOWS 0x00705fd2u   /* mov ecx, [ebx + 0x5d8] */
#define RE_PERSON_WINDOWS 0x5d8
/* the hover text of an activity in a list (0x0055e6b0) asks for "activityTitle" here, its first line; the activity
 * object is the local at [ebp - 0xe0], filled before that */
#define RE_VA_CALL_ACTIVITY_TITLE 0x0055ee21u
#define RE_VA_HOVER_ACTIVITY 0x0055e88du /* lea eax, [ebp - 0xe0] / mov dword ptr [ecx], 1: the object the lookup fills */
#define RE_HOVER_FRAME_ACTIVITY (-0xe0)

/* futures (note b12 "Q4", "Q5"): what the rule of the rates makes of an asset's inflation rate until the expiry.
 * These sites belong to the stocks group: the futures tab is a part of the stocks window. */
#define RE_VA_RATE_LIST 0x004d44a0u      /* ecx = economy; an industry list by value (ret 0xc): its weighted rate */
#define RE_VA_RATE_LIST_RET 0x004d45bfu
#define RE_VA_CALL_RATE_LIST 0x004d46f5u /* the rate of an asset or an activity asks for it here */
#define RE_VA_CALL_FUT_VALUE 0x007bf1ddu /* string copy of the rate's text for the futures window's label; edi = the window */
#define RE_VA_CALL_FUT_TIP 0x007bf351u   /* translate "assetInflationTitle", that label's hover text */
#define RE_VA_FUT_ECONOMY 0x007bf136u    /* mov ecx, [edi + 0x4c] */
#define RE_VA_FUT_MONTHS 0x007c3262u     /* push dword ptr [ebx + 0x160] */
#define RE_VA_ECON_GROWTH 0x004d533bu    /* movss xmm2, [edi + 0x9c] */
#define RE_VA_ECON_INDEX 0x004d545fu     /* add edi, 0xb0 */
#define RE_VA_ECON_CUSTOM 0x004d5414u    /* mov eax, [edi + 0x78] / add eax, 0xa0 */
#define RE_VA_CUSTOM_WEIGHTS 0x00503176u /* lea edi, [ecx + 0x90] */
#define RE_VA_TAG_ID 0x004d44f4u         /* mov eax, [esi + 4], esi = an element + 8 */
#define RE_VA_TAG_NEXT 0x004d4524u       /* add esi, 0x18 */
#define RE_FUT_WINDOW_ECONOMY 0x4c
#define RE_FUT_WINDOW_MONTHS 0x160 /* the expiry chosen with the three buttons */
/* the list of the futures tab (REQUEST.md [44], note m19 "Q5") */
#define RE_VA_WARE_RATE 0x004d45d0u     /* ecx = economy; a ware (ret 4); a factor in xmm2, 1.0 where the futures window asks:
                                           the ware's yearly price change in xmm0 */
#define RE_VA_WARE_RATE_RET 0x004d4730u
#define RE_VA_FUT_BUTTONS 0x007bd040u   /* FUN_007bd040 builds the list's two sort buttons with their hover texts */
#define RE_FUT_BUTTONS_SIZE 1191u
#define RE_VA_FUT_LIST 0x007bd880u      /* FUN_007bd880 builds the list: the wares in the chosen order, a page of rows */
#define RE_VA_FUT_SORT_FIRST 0x007bdbdeu      /* mov ecx, edi: the first ware id ... */
#define RE_VA_CALL_FUT_SORT_VALUE 0x007bdbeeu /* ... sorted by unit value: ecx = first, edx = behind the last; the count and
                                                 the comparison on the stack */
#define RE_VA_FUT_SORT_VALUE 0x007c5db0u
#define RE_VA_FUT_SORT_CLEAN 0x007bdbf3u      /* add esp, 8: the caller removes those two */
#define RE_VA_FUT_LIST_ASSET 0x007bdf04u      /* mov [ebp - 0x64], eax: the row's ware in the builder's frame, in what the
                                                 row's click is given; it is still there when the hover text is made. The
                                                 slot the ware is first kept in, [ebp - 0x120], is used for something else
                                                 by then (run 178: every row's hover text had the same rate) */
#define RE_FUT_LIST_FRAME_ASSET (-0x64)
#define RE_VA_CALL_FUT_ITEM 0x007bdd55u       /* the builder has the row made ... */
#define RE_VA_FUT_ITEM_RET 0x007bdd5au
#define RE_VA_FUT_ITEM 0x00611180u            /* ... by FUN_00611180: ecx = ui; the result, the ware, two more */
#define RE_VA_FUT_ITEM_ASSET 0x006111b3u      /* mov ecx, [ebp + 0xc]: the ware */
#define RE_FUT_ITEM_FRAME_ASSET 0xc
#define RE_VA_CALL_FUT_ITEM_NAME 0x0061126fu  /* the ware's name as shown, made straight into the argument that becomes the
                                                 row's title (RE_VA_INFO_NAME). The copy that follows, of [ebp - 0x40], is the
                                                 name of the ware's picture: text added there loses the picture (run 177) */
#define RE_VA_CALL_FUT_LIST_TIP 0x007be17eu   /* the builder asks for "clickView", the hover text of the row */
#define RE_ECON_GROWTH 0x9c
#define RE_ECON_INDEX 0xb0     /* std::map<int, float>: price index by industry */
#define RE_ECON_CUSTOM 0x78
#define RE_CUSTOM_WEIGHTS 0x90 /* std::map<int, float>: the industries' weights in the overall rate, not yet scaled to 1 */
#define RE_CUSTOM_POLICY 0xa0  /* std::map<int, float>: added to a rate every month */
#define RE_TAG_BYTES 0x18      /* an element of an industry list */
#define RE_TAG_SHARE 8         /* float */
#define RE_TAG_ID 0xc

/* business (notes b16, b18): what an hour's worth of work costs with a given employee, what an offered contract pays
 * over the standard cost of its work. A staff record is 0x148 bytes, a candidate or an employee alike. */
#define RE_VA_HIRE_LIST 0x004e9ca0u      /* ecx = firms; result vector, firm id, job id (ret 0xc): the candidates of a job */
#define RE_VA_HIRE_LIST_RET 0x004e9ea0u
#define RE_VA_CALL_HIRE_LIST 0x006278ebu /* the hire tab fetches its list and walks it in that order */
#define RE_VA_STAFF_COPY 0x0046f950u     /* ecx = raw memory for a staff record; the record to copy (ret 4) */
#define RE_VA_STAFF_COPY_RET 0x0046fa26u
#define RE_VA_STAFF_END_SKILL 0x0050da00u  /* ecx = record + RE_STAFF_SKILL: the first half of a record's destruction */
#define RE_VA_STAFF_END 0x004c2810u        /* ecx = record: the second half */
#define RE_VA_STAFF_EFFICIENCY 0x0051a4f0u /* ecx = jobs; job id, the record's skill part (ret 8); xmm0, 1.0 = 100% */
#define RE_VA_STAFF_EFFICIENCY_RET 0x0051abd5u
#define RE_VA_CALL_HIRE_LINE 0x00627af2u   /* a string assign shortly before a candidate's row is made; esi = the candidate */
#define RE_VA_ACTION_ROW 0x0060b9a0u       /* makes the row of an action; aligned frame, its arguments from ebx */
#define RE_VA_ACTION_ROW_FRAME 0x0060b9adu /* mov ebp, [ebx + 4]: its return address */
#define RE_VA_CALL_HIRE_ROW 0x00627bd6u    /* the hire tab's "Conduct Interview" row */
#define RE_VA_OBJECT_NAME 0x00668f70u      /* ecx = an object's data; the string to fill with its name (ret 4) */
#define RE_VA_OBJECT_NAME_RET 0x00669028u
#define RE_VA_CALL_ROW_NAME 0x0060bea3u /* the row maker fetches the action's name */
#define RE_VA_CALL_HIRE_COUNT 0x0062752eu     /* translate "firmStaffHireCandidates", the hire tab's second line */
#define RE_VA_CALL_STAFF_WAGE_TIP 0x005eb7aeu /* translate "staffWageTip" in the hover text of a card's wage icon */
#define RE_VA_CALL_OFFER_TIP 0x005f135du      /* translate "contractHasPayoutsTip" in a contract's payout hover text */
#define RE_VA_MAIN_FIRMS 0x0069558fu          /* mov ecx, [ebx + 0x394] */
#define RE_VA_FIRMS_JOBS 0x004ea3d8u          /* mov ecx, [edi + 4] */
#define RE_VA_HIRE_STEP 0x00627d10u           /* add esi, 0x148 */
#define RE_VA_HIRE_ID 0x00627b87u             /* push dword ptr [esi + 0x38] */
#define RE_VA_CARD_HOURS 0x00608c60u          /* mov eax, [ebp + 0x148]: the card maker has the record at [ebp + 0xc] */
#define RE_VA_CARD_WAGE 0x00608c6fu           /* push dword ptr [ebp + 0x134] */
#define RE_VA_CARD_SKILL 0x0060970bu          /* lea eax, [ebp + 0xec] */
#define RE_VA_CARD_JOB 0x00609712u            /* push dword ptr [ebp + 0xe8] */
#define RE_VA_WAGE_TIP_WAGE 0x005eb335u       /* mov eax, [ebp + 0x130]: the icons' maker has the record at [ebp + 8] */
#define RE_VA_OFFER_TIP_CONTRACT 0x005f11f8u  /* mov ebx, [ebp + 0xc] */
#define RE_VA_OFFER_TIP_OFFERED 0x005f16f8u   /* cmp byte ptr [ebp + 0x1c], 0 */
#define RE_VA_CONTRACT_TOTAL 0x005f1714u      /* push dword ptr [ecx + 0x18] */
#define RE_VA_CONTRACT_MONTHS 0x005f2ab8u     /* movd xmm0, dword ptr [ebx + 8] */
#define RE_VA_CONTRACT_HOURS 0x005f28afu      /* lea edx, [ebx + 0x30] */
#define RE_MAIN_FIRMS 0x394
#define RE_FIRMS_JOBS 4
#define RE_STAFF_BYTES 0x148
#define RE_STAFF_ID 0x38
#define RE_STAFF_JOB 0xdc
#define RE_STAFF_SKILL 0xe0
#define RE_STAFF_WAGE 0x128 /* int64, cents a month */
#define RE_STAFF_HOURS 0x13c
#define RE_CARD_FRAME_STAFF 0xc
#define RE_WAGE_TIP_FRAME_STAFF 8
#define RE_OFFER_TIP_FRAME_CONTRACT 0xc
#define RE_OFFER_TIP_FRAME_OFFERED 0x1c
#define RE_CONTRACT_MONTHS 8
#define RE_CONTRACT_TOTAL 0x18 /* int64, cents: all of the payout while the contract is only offered */
#define RE_CONTRACT_HOURS 0x30 /* std::map job -> hours a month */
/* the contracts a business has signed, as the month start walks them for their six-monthly payouts (note m32) */
#define RE_VA_SIGNED_CONTRACTS 0x004e53a1u /* mov eax, dword ptr [ebx + 0x1e4]: ebx = the business */
#define RE_VA_SIGNED_MONTHS 0x004e53b4u    /* mov eax, dword ptr [esi + 0x20]: esi = a node of that map */
#define RE_VA_SIGNED_START 0x004e53bcu     /* add eax, dword ptr [esi + 0x24] */
#define RE_FIRM_SIGNED 0x1e4
#define RE_SIGNED_NODE_CONTRACT 0x18 /* a node: 0x10, the contract's id, and the contract on the next eight bytes */
#define RE_CONTRACT_START 0xc        /* year x 12 + month, as re_game_period counts */
/* a contract signed by the mod (REQUEST.md [48], note m33). The month's offers are a map like the signed ones, at
 * FirmStack + 0x1dc. The action "Accept Contract" ends (FUN_00439570, 0x0043cf25 on) with: how many more the
 * business can hold, FUN_004f7be0 (ecx = the business: its type's maxContracts less the size of the signed map),
 * then FUN_004ebe50 (ecx = firms; the firm id and the contract by value, ret 0x54, as the payout function takes
 * them), which puts the contract into the signed map, takes it out of the offers and brings the business's list of
 * jobs in line. The icon "Contracts" of a business window opens the offers with FUN_005dbec0 (ecx = that window;
 * two words and then the firm id, ret 0xc; run 399 saw 1, 33, 17332 for the business 17332). */
#define RE_VA_FIRM_OFFERS 0x004f8619u        /* lea ecx, [esi + 0x1dc] */
#define RE_FIRM_OFFERS 0x1dc
#define RE_VA_FIRM_PLACES 0x004f7be0u
#define RE_VA_FIRM_PLACES_SIGNED 0x004f7c3cu /* mov edi, [esi + 0x1e8]: how many are signed */
#define RE_VA_FIRM_PLACES_RET 0x004f7c76u    /* plain ret */
#define RE_VA_CONTRACT_SIGN 0x004ebe50u
#define RE_VA_CONTRACT_SIGN_RET 0x004ebf31u  /* ret 0x54 */
#define RE_VA_OFFERS_WINDOW 0x005dbec0u
#define RE_VA_OFFERS_WINDOW_RET 0x005dc9b8u  /* ret 0xc */
#define RE_VA_CALL_OFFERS_WINDOW 0x005bc528u
#define RE_VA_OFFERS_WINDOW_FIRM 0x005bc514u /* push dword ptr [edi + 0x34]: pushed first, so the last word */
/* a business's reputation and its influence operation (REQUEST.md [48], note m33). The reputation is a float the
 * game keeps between 0.01 and 2; FUN_004f9fa0 adds to it, with a reason. The switch "influence operation" of a
 * business window is one byte, and its click (FUN_005ba670) changes nothing else. While it is on, every hour of
 * work the business books counts into its influence hours; the firm's month end (FUN_004f2bb0) then takes
 * reputation for it (reason 0x715), and a later update turns the hours into influence. */
#define RE_VA_FIRM_REPUTATION 0x004f9fb9u /* addss xmm3, dword ptr [esi + 0x20c] */
#define RE_FIRM_REPUTATION 0x20c
#define RE_VA_FIRM_INFLUENCE 0x004f6113u /* cmp byte ptr [edi + 0x234], 0 */
#define RE_FIRM_INFLUENCE 0x234
/* a business open (1) or closed (0) to its work (REQUEST.md [49], note m34). The button of a business window, the
 * confirm window of a closing that costs reputation and a move to other premises all end in FUN_004e80f0 (ecx =
 * firms; the firm id and the new state; ret 8), which hands the state to the business's FUN_004f33d0 (ecx = the
 * business; the state; ret 4; that is its one caller). Closing sets the hours of every job to 0, and the month end
 * gives a month's work to an open business only (FUN_004f35d0). Moving a business takes a closed one. */
#define RE_VA_FIRM_OPEN_SET 0x004f33fcu  /* mov dword ptr [edi + 0x4c], eax */
#define RE_VA_FIRM_OPEN_TEST 0x004f3695u /* cmp dword ptr [esi + 0x4c], 1 */
#define RE_FIRM_OPEN 0x4c
#define RE_VA_FIRM_STATE 0x004f33d0u
#define RE_VA_FIRM_STATE_RET 0x004f35c8u  /* ret 4 */
#define RE_VA_CALL_FIRM_STATE 0x004e8121u /* in FUN_004e80f0 */
/* the business window, and the clicks on its controls (REQUEST.md [49], note m34). The window is a part of the UI
 * object (RE_DEBTINV_UI), at 0xf50; its word at RE_ADVERT_WINDOW_FIRM is the business it shows. Every handler of a
 * click or of a hover asks one function whether the mouse is over its widget, each from a place of its own:
 * FUN_006b6ee0 (ecx = the UI's input object; the widget's node, a flag and the mouse's place, two floats by value;
 * ret 0x10; al). A handler that hears "no" does nothing. */
#define RE_VA_UI_FIRM_WINDOW 0x004f3569u /* add ecx, 0xf50 */
#define RE_UI_FIRM_WINDOW 0xf50
#define RE_VA_UI_HIT 0x006b6ee0u
#define RE_VA_UI_HIT_EVENT 0x006b7130u /* the same asked with the mouse event: ecx as above; the event, the node and a
                                        * third word; ret 0xc; al. It asks FUN_006b6ee0 itself */
/* the places the click handlers of a business window ask from, found by clicking each control with `[trace] hits=1`
 * (runs 497 and 498): what opens the hire window of a job, the staff tab, the asset shop and the month's offers; the
 * outsourcing box of a job, the star of the floor space, the box "buy worn-out assets again", the icon that gives a
 * contract up; and, through the event's variant, the right click on an asset's or an inventory item's icon */
#define RE_VA_LOCK_HIRE_WINDOW 0x005cb23du
#define RE_VA_LOCK_STAFF_TAB 0x005bd91au
#define RE_VA_LOCK_ASSET_SHOP 0x005c00dau
#define RE_VA_LOCK_OFFERS 0x005bc26au
#define RE_VA_LOCK_OUTSOURCE 0x005d8285u
#define RE_VA_LOCK_STAR 0x005cd293u
#define RE_VA_LOCK_ASSET_BOX 0x005d7965u
#define RE_VA_LOCK_GIVE_UP 0x005d7dafu
#define RE_VA_LOCK_SELL 0x005d1dbau
/* an offer against the standard cost of its work (note b18 "Q5"). The payout function makes a contract's payout from
 * today's wages, wares and utilities; the game runs it once, on a copy of the contract, when it makes the offer. */
#define RE_VA_CONTRACT_PAYOUT 0x004eae90u /* ecx = firms; firm id, then the contract by value, which it destroys
                                           * (ret 0x54); edx:eax, cents, before the firm's reputation counts */
#define RE_VA_CONTRACT_PAYOUT_RET 0x004ebe41u
#define RE_VA_CONTRACT_COPY 0x0043df40u /* ecx = raw memory for a contract; the contract to copy (ret 4) */
#define RE_VA_CONTRACT_COPY_RET 0x0043e00eu
#define RE_VA_OFFER_ARGUMENT 0x004f84ffu    /* sub esp, 0x50: the game's own call makes room for the copy, ... */
#define RE_VA_CALL_OFFER_COPY 0x004f8510u   /* ... copies the contract into it ... */
#define RE_VA_CALL_OFFER_PAYOUT 0x004f851eu /* ... and calls the payout function */
#define RE_VA_PAYOUT_TYPE 0x004eaf55u       /* mov eax, [ebx + 0xc]: the payout function has its contract at ebx + 0xc */
#define RE_VA_PAYOUT_MONTHS 0x004ebd6cu     /* mov esi, [ebx + 0x14] */
#define RE_VA_PAYOUT_HOURS 0x004eaf96u      /* mov eax, [ebx + 0x3c] */
#define RE_VA_JOB_WAGE 0x0051b5f0u /* ecx = jobs; job id (ret 4); edx:eax, cents: the standard wage of an hour of a job */
#define RE_VA_JOB_WAGE_RET 0x0051b7f4u
#define RE_VA_LENGTH_BONUS 0x004ec2c0u /* ecx = firms; months (ret 4); xmm0: the payout's factor for a long contract */
#define RE_VA_LENGTH_BONUS_RET 0x004ec304u
#define RE_VA_CALL_LENGTH_BONUS 0x005f19b2u /* the payout hover text asks for it, for its line "contract length" */
#define RE_VA_DATA_NUMBER 0x00816c30u /* ecx = data; the names of an element and of its attribute as std::string by
                                       * value, which it destroys (ret 0x30); xmm0: a number of defaultData.xml */
#define RE_VA_DATA_NUMBER_RET 0x00816d26u
#define RE_VA_CALL_MARKUP_LABOUR 0x004ebb80u /* the payout function's three calls of it: the markup on labour, ... */
#define RE_VA_CALL_MARKUP_INVENTORY 0x004ebc32u
#define RE_VA_CALL_MARKUP_UTILITIES 0x004ebce4u
#define RE_VA_OFFER_TIP_FIRM 0x005f11d0u   /* mov eax, [ebp + 8]: the business the payout hover text is for */
#define RE_VA_FIRM_ID 0x005f1bd2u          /* push dword ptr [eax + 0x24] */
#define RE_VA_FIRMS_DATA 0x004eaf45u       /* mov ecx, [esi + 0x54] */
#define RE_VA_FIRMS_SETTINGS 0x004ebd8bu   /* mov eax, [eax + 0x78] */
#define RE_VA_SETTINGS_REVENUE 0x004ebd8eu /* mulss xmm0, [eax + 0x40] */
#define RE_VA_FIRMS_ECONOMY 0x004ebda6u    /* mov ecx, [eax + 0xc] */
#define RE_VA_WAGE_POLICY_KEY 0x0051b756u  /* mov dword ptr [ebp - 0x90], 0x13c5: the wage function's key ... */
#define RE_VA_WAGE_POLICY_MAP 0x0051b760u  /* lea ecx, [esi + 0x98]: ... into the settings' map of factors */
/* the jobs that work in a listed job sets off: data + 0x1f4 [business type][job]["SPECIAL"]["CONTRACTANYGEN"], four
 * maps inside one another and a list. The payout function gets the list with four lookups that add a key that is
 * not there; the plugin walks the maps and adds nothing. */
#define RE_VA_DATA_SPECIALS 0x004eb547u        /* add ecx, 0x1f4 */
#define RE_VA_CALL_SPECIALS_TYPE 0x004eb54eu   /* by business type, ... */
#define RE_VA_CALL_SPECIALS_JOB 0x004eb558u    /* ... by job, ... */
#define RE_VA_CALL_SPECIALS_GROUP 0x004eb562u  /* ... by "SPECIAL" ... */
#define RE_VA_CALL_SPECIALS_NAME 0x004eb56cu   /* ... and by "CONTRACTANYGEN" */
#define RE_VA_SPECIALS_TYPE 0x00820f90u        /* the four lookups: ecx = map, pointer to the key (ret 4) */
#define RE_VA_SPECIALS_JOB 0x00821000u
#define RE_VA_SPECIALS_GROUP 0x00821070u
#define RE_VA_SPECIALS_NAME 0x00821110u
#define RE_VA_SPECIALS_TYPE_VALUE 0x00820fc4u  /* lea eax, [esi + 0x14]: what a lookup by id returns, esi = the node */
#define RE_VA_SPECIALS_JOB_VALUE 0x00821034u
#define RE_VA_SPECIALS_GROUP_VALUE 0x008210c3u /* lea eax, [esi + 0x28]: the same behind a std::string */
#define RE_VA_SPECIALS_NAME_VALUE 0x00821163u
#define RE_VA_INDUCED_AMOUNT 0x004eb6d3u /* mov ecx, [edi] / mov esi, edx / mov edx, [edi + 4]: int64, edi = an element */
#define RE_VA_INDUCED_CHANCE 0x004eb6e9u /* mulss xmm0, [edi + 8] */
#define RE_VA_INDUCED_JOB 0x004eb7b2u    /* mov eax, [edi + 0xc] */
#define RE_VA_INDUCED_NEXT 0x004eba8au   /* add edi, 0x18 */
#define RE_OFFER_TIP_FRAME_FIRM 8
#define RE_PAYOUT_FRAME_CONTRACT 0xc
#define RE_CONTRACT_TYPE 0 /* the kind of contract, an id of data/contracts */
#define RE_CONTRACT_BYTES 0x50
#define RE_FIRM_ID 0x24
#define RE_FIRMS_ECONOMY 0xc
#define RE_FIRMS_DATA 0x54
#define RE_FIRMS_SETTINGS 0x78     /* the object that also holds RE_POLICY_VALUES */
#define RE_SETTINGS_REVENUE 0x40   /* float: the difficulty's factor on what a business is paid */
#define RE_TAG_WAGE_POLICY 5061    /* in RE_POLICY_VALUES: a city policy's factor on wages, 1 = none */
#define RE_DATA_SPECIALS 0x1f4
#define RE_SPECIALS_ID_VALUE 0x14  /* in a node of the two outer maps */
#define RE_SPECIALS_TEXT_VALUE 0x28 /* in a node of the two inner maps */
#define RE_INDUCED_BYTES 0x18      /* an element of the list: int64 amount, then */
#define RE_INDUCED_CHANCE 8        /* float */
#define RE_INDUCED_JOB 0xc
/* what a business loses to missing assets (note b19 "A1", "A3"): two stored factors the game works out every hour */
#define RE_VA_FIRMS_OWN 0x006951adu        /* mov eax, [edi + 0x9c]: the household's businesses */
#define RE_VA_FIRMS_OWN_RECORD 0x006951c0u /* lea ecx, [esi + 0x18] */
#define RE_VA_FIRM_TYPE 0x004e9b3fu        /* cmp [eax + 0xc], edx: 0 = no type chosen yet */
#define RE_VA_FIRM_PREMISES 0x004e9b2au    /* cmp dword ptr [eax + 0x48], 0 */
#define RE_VA_FIRM_NAME 0x0070fcd2u        /* lea eax, [ecx + 0x54] */
#define RE_VA_FIRM_FURNISH 0x004f72cau     /* movss [edi + 0x80], xmm0 */
#define RE_VA_FIRM_ASSETS 0x004f6fcbu      /* lea eax, [ecx + 0x78] */
#define RE_FIRMS_OWN 0x9c /* std::map<firm id, business>; a business is 0x240 bytes */
#define RE_FIRMS_NODE_FIRM 0x18
#define RE_FIRM_BYTES 0x240
#define RE_FIRM_TYPE 0xc
#define RE_FIRM_PREMISES 0x48
#define RE_FIRM_NAME 0x54    /* std::string */
#define RE_FIRM_FURNISH 0x80 /* float, at most 1: the furnishings every job of the business uses */
#define RE_FIRM_ASSETS 0x78  /* std::map<job, float 0.1 .. 1>: the assets of each job */
/* what an hour of work is worth (note b19 "A1"): FUN_0051a320 multiplies a person's skill by the assets of the job,
 * not below 0.35, and by the furnishings, not below 0.5. FUN_004f6f60 works the two stored numbers out again; the
 * game calls it every game hour */
#define RE_VA_WORK_ASSETS_FLOOR 0x0051a3c4u  /* movss xmm1, [0x8f9ed8]; then maxss with the job's assets */
#define RE_VA_WORK_FURNISH_FLOOR 0x0051a41fu /* movss xmm0, [0x8f9ef4]; then maxss with the furnishings */
#define RE_VA_ASSETS_FLOOR 0x008f9ed8u       /* the float 0.35 */
#define RE_VA_FURNISH_FLOOR 0x008f9ef4u      /* the float 0.5 */
#define RE_VA_FIRM_REFIT 0x004f6f60u         /* ecx = business, nothing else */
#define RE_VA_FIRM_REFIT_RET 0x004f731bu
/* the staff of a business and the game's automatic management of a person (note b19 "B1"): a switch per employee
 * that only the staff tab sets, and the month end's own replacement of a person who resigns */
#define RE_VA_STAFF_APPEND 0x0046fa30u     /* ecx = staff; firm id, the record to copy (ret 8): one more employee */
#define RE_VA_STAFF_APPEND_RET 0x0046fa6fu
#define RE_VA_CALL_HIRE_APPEND 0x004ea05bu /* in the hire of a candidate */
#define RE_VA_STAFF_AUTO 0x004ec310u       /* ecx = firms; firm id, staff id (ret 8); al = managed automatically */
#define RE_VA_STAFF_AUTO_RET 0x004ec360u
#define RE_VA_STAFF_AUTO_SET 0x004ec370u   /* ecx = firms; firm id, staff id, on or off (ret 0xc) */
#define RE_VA_STAFF_AUTO_SET_RET 0x004ec3b8u
#define RE_VA_FIRMS_STAFF 0x004ea058u      /* mov ecx, [ebx + 0x1c] */
#define RE_VA_STAFF_BY_FIRM 0x0046fa3bu    /* lea ecx, [edi + 0xa4] */
#define RE_VA_STAFF_BY_FIRM_LIST 0x004781f4u /* lea eax, [esi + 0x14] */
#define RE_FIRMS_STAFF 0x1c
#define RE_STAFF_BY_FIRM 0xa4      /* in the staff object: std::map<firm id, std::vector<staff record>> */
#define RE_STAFF_BY_FIRM_LIST 0x14 /* the vector in a node of that map */
/* a wage demand of an automatically managed employee, and the replacement of one who leaves over a demand that was
 * not met (notes b19 "B2", b20). The month end's staff routine answers such a demand with one call that writes the
 * new wage; a demand left open makes the person leave at the next month end, and nobody is hired for the place. */
#define RE_VA_STAFF_SET_WAGE 0x0046fc00u      /* ecx = staff; firm id, staff id, int64 wage (ret 0x10) */
#define RE_VA_STAFF_SET_WAGE_MISS 0x0046fc60u /* ret 0x10: no such employee */
#define RE_VA_STAFF_SET_WAGE_RET 0x0046fc8eu
#define RE_VA_SET_WAGE_WAGE 0x0046fc66u    /* mov [esi + 0x128], eax */
#define RE_VA_SET_WAGE_DEMAND 0x0046fc75u  /* mov dword ptr [esi + 0x130], 0: the demand is met */
#define RE_VA_CALL_WAGE_ACCEPT 0x00471b63u /* in the staff routine, for a person who has the switch */
#define RE_VA_WAGE_ACCEPT_WAGE 0x00471b52u /* push edx / push eax: the demanded wage */
#define RE_VA_WAGE_ACCEPT_ID 0x00471b5au   /* push dword ptr [edi + 0x38]: edi = the employee's record, in the list */
#define RE_VA_WAGE_ACCEPT_FIRM 0x00471b5du /* push dword ptr [esi + 0x10]: the key of the staff map's node */
#define RE_VA_DEMAND_STORE 0x004716a9u     /* mov [edi + 0x130], eax: what the routine does without the switch */
#define RE_VA_DEMAND_OPEN_HIGH 0x0046ff2cu /* cmp ecx, [edi + 0x134]: a wage under the demand, the person leaves */
#define RE_VA_DEMAND_OPEN_LOW 0x0046ff3au  /* cmp eax, [edi + 0x130] */
#define RE_VA_CALL_RATIO_WAGE 0x0051b222u  /* the routine's test "paid under 80%" measures against the standard wage */
#define RE_VA_HIRE_START 0x004ea04fu       /* mov [esi + 0x140], eax: the month of the hire, in the game's hire function */
#define RE_VA_STAFF_REFILL 0x00471b71u     /* mov [edi + 0x138], eax: the month end gives everybody the month's hours again */
#define RE_VA_STAFF_LIST_END 0x00478650u   /* ecx = a vector of staff records: ends each, frees it (plain ret) */
#define RE_VA_STAFF_LIST_END_SKILL 0x00478668u /* its two calls for a record ... */
#define RE_VA_STAFF_LIST_END_REST 0x0047866fu
#define RE_VA_STAFF_LIST_END_STEP 0x00478674u  /* add esi, 0x148 */
#define RE_VA_STAFF_LIST_END_RET 0x004786d5u   /* pop esi / ret */
#define RE_VA_RECRUIT_FEE_TWICE 0x00471c72u    /* shld eax, ecx, 1: the game's own replacement costs two wages, ... */
#define RE_VA_RECRUIT_FEE_TAG 0x00471c76u      /* push 0x7f9: ... booked under the staff tag ... */
#define RE_VA_RECRUIT_FEE_TAXABLE 0x00471c6bu  /* push 1 */
#define RE_VA_RECRUIT_FEE_FIRM 0x00471c5eu     /* push 0 / push dword ptr [edi + 0x10]: ... with the firm id behind */
#define RE_VA_CALL_RECRUIT_FEE 0x00471c82u
#define RE_VA_PERSON_NAME 0x004c3da0u     /* ecx = a person, the start of a staff record; fills "First Last" */
#define RE_VA_CALL_CARD_NAME 0x00608ad6u  /* the staff card asks it of the record */
#define RE_VA_CARD_PERSON 0x00608ad3u     /* lea ecx, [ebp + 0xc] */
#define RE_VA_NAME_FIRST 0x004c3dfeu      /* lea eax, [esi + 0x10]: the length of the std::string at +0 */
#define RE_VA_NAME_LAST 0x004c3e20u       /* lea ecx, [esi + 0x18] */
#define RE_VA_CALL_STAFF_AUTO_TIP 0x00629edfu /* translate "staffAutoManageTip", the hover text of a row's switch */
#define RE_STAFF_FIRST_NAME 0    /* std::string */
#define RE_STAFF_LAST_NAME 0x18  /* std::string */
#define RE_STAFF_DEMAND 0x130    /* int64, cents a month; open while it is above the wage */
#define RE_STAFF_START 0x140     /* the month of the hire, month + 12 x year */
#define RE_STAFF_LEFT 0x138      /* the hours of the month not worked yet */
/* a staff that fits the work (REQUEST.md [38], [42]): one employee taken out of the staff, and the click that
 * switches the automatic management */
#define RE_VA_STAFF_REMOVE 0x0046fa90u     /* ecx = staff; firm id, staff id (ret 8): the employee is gone; no money, no notice */
#define RE_VA_STAFF_REMOVE_RET 0x0046fb75u
#define RE_VA_AUTO_CLICK_SHIFT 0x00629b68u /* jne 0x629b9d after the look for a held Shift: to "for everybody of the business" */
/* the adverts of a business (notes b22, m22). An advert is a switch of a business. Every 91st game hour, eight
 * times a month, the game's function 0x004fa3b0 gives each advert that is switched on its turn: an eighth of the
 * month's awareness and reputation, and 91 hours of its price. The month end counts the hours the jobs of a
 * business were given and those still to do; the same numbers are read here just before it. */
#define RE_VA_FIRM_ADVERTS_END 0x004e6b55u  /* mov edx, [eax + 0x224] */
#define RE_VA_FIRM_ADVERTS 0x004e6b5bu      /* mov ecx, [eax + 0x220]: the adverts switched on, a vector */
#define RE_VA_ADVERT_NEXT 0x004e6b6du       /* add ecx, 0x18 */
#define RE_VA_FIRM_AWARENESS 0x004f7539u    /* movss xmm0, [eax + 0x208] */
#define RE_VA_FIRM_ADVERT_COSTS 0x004fa9afu /* lea ecx, [eax + 0x178]: what the adverts cost in their last turn, std::map<advert, int64 cents> */
#define RE_VA_ADVERT_COST_VALUE 0x00494cb4u /* lea eax, [esi + 0x18]: the amount in a node of such a map */
#define RE_VA_FIRM_JOBS 0x004f67c0u         /* mov ecx, [ecx + 0x1cc]: in the function that sums the hours still to do */
#define RE_VA_JOB_OVERTIME 0x004f67d0u      /* mov esi, [eax + 0x60]: eax = a node of that map */
#define RE_VA_JOB_WORKED 0x004f67d5u        /* sub esi, [eax + 0x58] */
#define RE_VA_JOB_ALLOWED 0x004f67d8u       /* add esi, [eax + 0x5c] */
#define RE_VA_CALL_ADVERT_TIP 0x005c74a0u   /* translate "clickToActivateDeactivate", the end of an advert icon's hover text */
#define RE_VA_ADVERT_TIP_WINDOW 0x005c7490u /* mov ecx, [edi]: edi = the icon's object, its first word the business window */
#define RE_VA_ADVERT_TIP_FIRM 0x005c5ae1u   /* mov eax, [ecx + 0xfc]: the id of the business that window shows */
#define RE_FIRM_ADVERTS 0x220
#define RE_ADVERT_BYTES 0x18
#define RE_FIRM_AWARENESS 0x208    /* float, 1 = 100%; it can be above */
#define RE_FIRM_ADVERT_COSTS 0x178
#define RE_ADVERT_COST_VALUE 0x18
#define RE_FIRM_JOBS 0x1cc         /* std::map<job, record>; in a node of it, hours of the running month: */
#define RE_JOB_NODE_WORKED 0x58    /* done, */
#define RE_JOB_NODE_ALLOWED 0x5c   /* given at the month start, */
#define RE_JOB_NODE_OVERTIME 0x60  /* and added since */
#define RE_ADVERT_WINDOW_FIRM 0xfc
/* staff filled in during the month (note m22): the first call of the block the game runs for every business every
 * 91st hour, a job's outsourcing switch, the game's own hiring of a candidate */
#define RE_VA_FIRM_TICK 0x004f7a40u      /* ecx = a business (plain ret): pays what its adverts left to pay last time */
#define RE_VA_FIRM_TICK_RET 0x004f7bccu
#define RE_VA_CALL_FIRM_TICK 0x004e5882u /* in the hourly work of the household's businesses, every 91st hour */
#define RE_VA_FIRM_TICK_FIRM 0x004e587fu /* lea ecx, [esi + 0x18]: esi = the node of the business */
#define RE_VA_JOB_OUTSOURCED 0x004f3224u /* cmp dword ptr [esi + 0x48], 0: the game does the hours of such a job itself, for money */
#define RE_VA_JOB_KIND 0x004f326eu       /* push dword ptr [esi + 0x20]: the job of the game's data; the map's key is a number of the business's own */
#define RE_VA_HIRE 0x004e9f70u           /* ecx = firms; firm id, job id, candidate id (ret 0xc): the candidate joins the staff */
#define RE_VA_HIRE_RET 0x004ea0d3u
#define RE_VA_HIRE_RET_EARLY 0x004ea03cu
#define RE_JOB_NODE_OUTSOURCED 0x48
#define RE_JOB_NODE_KIND 0x20
/* adverts handed to the mod (note m22): the adverts of a business type as the game fetches them, the switch of an
 * advert, its price, and where the held keys are found from the object the ticker is in */
#define RE_VA_DATA_LIST 0x00807f30u         /* ecx = an empty vector; a record (0x38) and two std::string by value, removed by the CALLER (plain ret) */
#define RE_VA_DATA_LIST_RET 0x00807fddu
#define RE_VA_CALL_BRAND_WINDOW 0x005c2fdbu /* the adverts of a type's brand, when the business window is built */
#define RE_VA_BRAND_WINDOW_ARGS 0x005c2fe0u /* add esp, 0x68 */
#define RE_VA_BRAND_WINDOW_NAME 0x005c2f86u /* push "brand" */
#define RE_VA_TEXT_BRAND 0x008de814u
#define RE_VA_BRAND_ADVERT 0x005c3000u      /* mov eax, [esi + 0xc]: the advert in an element of that list */
#define RE_VA_ADVERT_SWITCH 0x004e7b60u     /* ecx = firms; firm id, advert id, 1 = on / 0 = off (ret 0xc); does not look whether it is on */
#define RE_VA_ADVERT_SWITCH_RET 0x004e7bd8u
#define RE_VA_CALL_ADVERT_CLICK 0x005c57b2u /* the click on an advert's icon */
#define RE_VA_ADVERT_ON 0x004e6b30u         /* ecx = firms; firm id, advert id (ret 8): eax = 1 when the advert is on, else 0 */
#define RE_VA_ADVERT_ON_RET 0x004e6b7eu
#define RE_VA_ADVERT_ON_RET_NONE 0x004e6b86u
#define RE_VA_CALL_ADVERT_ON_CLICK 0x005c578eu /* the click asks whether the advert is on ... */
#define RE_VA_ADVERT_CLICK_MODE 0x005c5793u    /* lea esi, [eax + 1]: ... switches it to the opposite ... */
#define RE_VA_ADVERT_CLICK_LOOK 0x005c57bdu    /* test esi, esi: ... and gives the icon the look of what it switched to */
/* An icon's look is a colour of its sprite: white (255) for an advert that is on, grey (127) for one that is off.
 * The colour is made with cocos2d's Color3B(r, g, b), called through the import table (`call [slot]`, 6 bytes). */
#define RE_VA_COLOUR3_NEW 0x008d39acu          /* the import slot of that constructor: ecx = the colour; r, g, b (removes 12) */
#define RE_VA_ADVERT_LOOK_ON 0x005c57d7u       /* the click: the colour of an advert switched on ... */
#define RE_VA_ADVERT_LOOK_OFF 0x005c57f2u      /* ... and of one switched off */
#define RE_VA_CALL_ADVERT_ON_BUILD 0x005c319du /* the window's build asks whether an advert is on ... */
#define RE_VA_ADVERT_LOOK_BUILD 0x005c31d9u    /* ... makes the colour for it ... */
#define RE_VA_ADVERT_LOOK_SPRITE 0x005c31dfu   /* ... and hands it to the icon's sprite: mov ecx, [ebp - 0x34c] */
#define RE_ADVERT_BUILD_FRAME_SPRITE (-0x34c)  /* the sprite in the frame of the window's build, set before the colour is made */
#define RE_VA_ADVERT_PRICE 0x004e7d80u      /* ecx = firms; firm id, advert id (ret 8): cents an hour in edx:eax, 0 for an advert that is an activity */
#define RE_VA_ADVERT_PRICE_RET 0x004e8026u
#define RE_VA_UI_HIRE_WINDOW 0x005cb2a2u    /* add ebx, 0x1348: the hire window inside the object RE_DEBTINV_UI points at */
#define RE_UI_HIRE_WINDOW 0x1348
#define RE_ADVERT_ID 0xc
#define RE_MONTH_HOURS 728 /* game hours in a month; the block above runs eight times in it */
/* the assets of a business (note b23), for a trace only: the records, the two switches the game's own replacement
 * of a worn-out asset asks for, and that replacement */
#define RE_VA_FIRM_STOCKPILE 0x004e6d93u    /* lea ecx, [eax + 0x184]: its first member is std::map<instance, asset record> */
#define RE_VA_ASSET_NODE_TYPE 0x0055b629u   /* mov eax, [esi + 0x24]: the item type of a node's record */
#define RE_VA_ASSET_REPLACE 0x004f42a0u     /* ecx = a business; the item type (ret 4): "expired", and a new one when both switches are on */
#define RE_VA_ASSET_REPLACE_RET 0x004f4ac9u
#define RE_VA_CALL_ASSET_PAY 0x004f480au    /* its cash movement for the new unit; the amount comes negated once too often */
#define RE_VA_ASSET_PAY_NEG_LOW 0x004f47edu /* neg ecx: the low half of the price the game's price function returned ... */
#define RE_VA_ASSET_PAY_NEG_HIGH 0x004f4800u /* neg eax: ... and the high half, behind an adc */
#define RE_VA_FIRM_AUTO_ASSET 0x004f4587u   /* cmp byte ptr [edi + 0x1b5], 0: worn-out assets are bought again */
#define RE_VA_FIRM_SUPPLY_HOURS 0x004f4594u /* cmp byte ptr [edi + 0x1c0], 0: the month's staff hours for it were taken */
#define RE_FIRM_STOCKPILE 0x184
/* an asset a business is short of, bought (note b23, D-070): what a business wants and has of every asset tag, the
 * wares that bring a tag, a ware's floor space, price and name, the floor space of the premises, and the two
 * notices of the game's replacement */
#define RE_VA_FIRM_DESIRED 0x004e9020u     /* ecx = firms; a std::map<tag, int> to fill, the firm id (ret 8); reads only */
#define RE_VA_FIRM_DESIRED_RET 0x004e90e6u
#define RE_VA_FIRM_OWNED 0x004e8fe9u       /* add eax, 0x194: std::map<tag, int>, what the assets of the business bring */
#define RE_FIRM_OWNED 0x194
#define RE_VA_MAP_ERASE 0x00405e60u        /* ecx = a std::map; four bytes of scratch, first, last (ret 0xc) */
#define RE_VA_MAP_ERASE_RET 0x00405f4eu
#define RE_VA_SIZED_DELETE 0x00831fe8u     /* cdecl: a block, its bytes */
#define RE_VA_WARE_INFO 0x0080e160u        /* ecx = data; the info to fill, a flag, the ware as a list element by value (ret 0x20) */
#define RE_VA_WARE_INFO_RET 0x0080e267u
#define RE_VA_TAG_INFO 0x0080edc0u         /* the same for an asset tag */
#define RE_VA_TAG_INFO_RET 0x0080eecau
#define RE_INFO_BYTES 0x34
#define RE_INFO_STRING 0x1c                /* a std::string of the info, the caller's to destroy */
#define RE_VA_INFO_NUMBER 0x008075f0u      /* ecx = an info; element and attribute as std::string by value, removed by the CALLER: xmm0 */
#define RE_VA_INFO_NUMBER_RET 0x00807695u  /* plain ret */
#define RE_VA_INFO_NAME 0x00668f70u        /* ecx = an info; a std::string to construct (ret 4) */
#define RE_VA_INFO_NAME_RET 0x00669028u
#define RE_VA_WARE_PRICE 0x004d4740u       /* ecx = prices; an info, "shop" and "money" as std::string by value (ret 0x34): cents, negative = a cost */
#define RE_VA_WARE_PRICE_RET 0x004d47dbu
#define RE_VA_PRICE_NOW 0x004d4820u        /* ecx = prices; an info, an int64 of cents (ret 0xc): that amount at the day's prices of
                                              the info's industries, edx:eax. FUN_005595a0 prices a business's utilities with it:
                                              the utility's "baseCost" x the amount used */
#define RE_VA_PRICE_NOW_RET 0x004d4c54u
#define RE_VA_INFO_LIST 0x00811480u        /* ecx = data; the vector to fill, an info, 0, then three std::string by value: the
                                              element's parent ("tags"), the element ("consumption"), a condition ("") (ret 0x54).
                                              The vector is the caller's: elements of RE_INDUCED_BYTES */
#define RE_VA_INFO_LIST_RET 0x00811a00u
#define RE_VA_LIST_FREE 0x00405710u        /* ecx = such a vector: its block goes back */
#define RE_VA_FIRM_PRICES 0x004f45f6u      /* mov ecx, [edi + 0x94] */
#define RE_FIRM_PRICES 0x94
#define RE_VA_FIRM_DATA 0x004f4328u        /* mov ecx, [edi + 0xdc] */
#define RE_FIRM_DATA 0xdc
#define RE_VA_DATA_LISTS 0x007a0369u       /* add ecx, 0x1ec: std::map<id, map<element, map<child, vector of list elements>>> */
#define RE_DATA_LISTS 0x1ec
#define RE_VA_SPACE_USED 0x00805500u       /* ecx = [business + 0xe0]; the stockpile of the business (ret 4): floor space in use, xmm0 */
#define RE_VA_SPACE_USED_RET 0x008056f3u
#define RE_VA_FIRM_SPACE 0x004f2ed0u       /* mov ecx, [esi + 0xe0] */
#define RE_FIRM_SPACE 0xe0
#define RE_VA_FIRM_HOUSES 0x004f2eb2u      /* mov ecx, [esi + 0x98]: where the premises [business + 0x48] are looked up */
#define RE_FIRM_HOUSES 0x98
#define RE_VA_HOUSES_OWN 0x0054e7dbu       /* lea ecx, [edi + 0xac]: std::map<id, house record>, the household's */
#define RE_HOUSES_OWN 0xac
#define RE_VA_HOUSES_OTHER 0x0054e7fdu     /* lea ecx, [edi + 0xb4]: the others */
#define RE_HOUSES_OTHER 0xb4
#define RE_HOUSES_NODE_RECORD 0x18
#define RE_HOUSE_FLOORSPACE 0x80           /* float: the most the premises hold */
/* premises made larger (REQUEST.md [48], note m33). A property record starts with the reference to its type (a list
 * element: the type's id at + 0xc). The star of a business window, "upgrade this property at the end of the month",
 * writes the type to grow into at + 0x88, 0 for none (the click, 0x005cd341). The month end (FUN_0054f150) then asks
 * FUN_00550cd0 (ecx = the property manager; the property's id, 4 for an owned one and 2 for a rented one; ret 8;
 * edx:eax) what it costs - the price once more, or the month's rent once more - and, when the cash allows it,
 * doubles the floor space, gives the record the new type and charges the price or raises the rent. What a type
 * grows into is its `<property><upgrade id=".."/>` in the game's data; FUN_0080f4c0 gives a property type's info
 * as RE_VA_WARE_INFO gives a ware's. */
#define RE_HOUSE_TYPE 0xc
#define RE_VA_HOUSE_GROW_CLICK 0x005cd341u /* mov [eax + 0x88], esi */
#define RE_VA_HOUSE_GROW_ASKED 0x0054f19cu /* cmp dword ptr [esi + 0x88], 0, the month end */
#define RE_HOUSE_GROW 0x88
#define RE_HOUSE_OWNED 4                   /* what the game hands FUN_00550cd0 for the two maps */
#define RE_HOUSE_RENTED 2
#define RE_VA_GROW_COST 0x00550cd0u
#define RE_VA_GROW_COST_RET 0x00550d98u    /* ret 8 */
#define RE_VA_RESTATE_INFO 0x0080f4c0u
#define RE_VA_RESTATE_INFO_RET 0x0080f5cau /* ret 0x20 */
#define RE_VA_POST_MESSAGE_RET 0x007e0544u   /* RE_VA_POST_MESSAGE removes its type and three strings: ret 0x4c */
#define RE_VA_CALL_ASSET_EXPIRED 0x004f4582u /* the replacement's ticker line that the asset has worn out (type 1) */
#define RE_VA_CALL_ASSET_BOUGHT 0x004f4a64u  /* ... and that a new one was bought (type 2); its text has no translation and comes empty */
#define RE_ASSET_NODE_TYPE 0x24
#define RE_ASSET_NODE_BYTES 0x60 /* the head of a node, the key, a record of 0x48 bytes */
#define RE_FIRM_AUTO_ASSET 0x1b5
/* a click that switches it on (FUN_005d78d0, 0x005d7a16) calls FUN_004f5380 next (ecx = the business; plain ret):
 * while the month's 20 staff hours for the purchases are not taken yet, they are taken from somebody of the job
 * that does them who has more than 20 left */
#define RE_VA_FIRM_SUPPLY 0x004f5380u
#define RE_VA_CALL_FIRM_SUPPLY 0x005d7a16u
#define RE_FIRM_SUPPLY_HOURS 0x1c0
/* Jobs of the game's data (data/jobs, listed in the game's own data/statsJob.txt) whose standard hourly wage prices
 * what the mod does in a person's place (REQUEST.md [39]): hiring for a business, the monthly look at the property
 * market, the research of a listed company. */
#define RE_JOB_STORE_MANAGER 30111
#define RE_JOB_ESTATE_AGENT 31211
#define RE_JOB_BANK_ANALYST 31661
#define RE_JOB_PR_SPECIALIST 32441 /* looking after the adverts of a business */
#define RE_TAG_STAFF 2041        /* the finance tag of wages and of the recruiter's fee */
/* candidates that a load does not change (note b21). The candidates of a month are kept as
 * std::map<firm id, std::map<job id, std::vector<staff record>>>; the inner map is the tree type the staff of a
 * business is kept in, so RE_STAFF_BY_FIRM_LIST is where its nodes have their list too. */
#define RE_VA_FIRMS_CANDIDATES 0x004e9cd3u    /* lea esi, [ebx + 0xac] */
#define RE_VA_CALL_CANDIDATES_FIRM 0x004e9d07u /* the list function looks up the lists of the business ... */
#define RE_VA_CANDIDATES_BY_FIRM 0x004ecdd0u
#define RE_VA_CALL_CANDIDATES_KEY 0x004e9d0eu /* ... and counts the entries for the job */
#define RE_VA_MAP_COUNT 0x0040d470u
#define RE_VA_CANDIDATES_KEPT 0x004e9d13u     /* test eax, eax / jne: an entry is there, nothing is drawn */
#define RE_VA_CANDIDATES_JOBS 0x004ece04u     /* lea eax, [esi + 0x14]: the lists of a business in its node */
#define RE_VA_STAFF_LIST_KEY 0x004781d7u      /* cmp [eax + 0x10], edx: the key of a node, where the game looks a list up */
#define RE_VA_STAFF_LIST_NEW 0x0040f4e8u      /* mov dword ptr [eax + 0x14], 0: a new node has an empty list */
#define RE_VA_STAFF_TREE_NODE 0x00478b97u     /* push 0x20: the size of a node, where the game frees a map of lists */
/* the month end empties the lists of a business */
#define RE_VA_MONTHEND_JOBS 0x004ed0a2u       /* lea edi, [esi + 0x14] */
#define RE_VA_MONTHEND_FIRM_NODE 0x004ed0ddu  /* push 0x1c: the size of a node of the outer map */
#define RE_FIRMS_CANDIDATES 0xac
#define RE_CANDIDATES_JOBS 0x14
#define RE_STAFF_BY_FIRM_KEY 0x10 /* in a node of a map of lists, in front of RE_STAFF_BY_FIRM_LIST */
/* The random streams (RandGen): twelve of {seed, calls, engine} and a thirteenth engine that is seeded before every
 * use. An engine is a 32-bit Mersenne Twister, 624 words and the place in them. The businesses, the jobs and the
 * object the ages come from each keep a pointer to the same RandGen. */
#define RE_VA_RAND_SEED 0x00670a70u         /* ecx = engine; pointer to a 32-bit seed (ret 4) */
#define RE_VA_RAND_SEED_RET 0x00670b05u
#define RE_VA_RAND_SEED_PLACE 0x00670a7bu   /* mov dword ptr [ecx + 0x9c0], 1 */
#define RE_VA_RAND_SEED_WORDS 0x00670ab2u   /* cmp dword ptr [ecx + 0x9c0], 0x270 */
#define RE_VA_RANDGEN_ENGINE_NEW 0x0067063cu /* push 0x9c4: the size of an engine */
#define RE_VA_RANDGEN_SECOND 0x006709bfu    /* lea ecx, [esi + 0xc]: the second stream */
#define RE_VA_RANDGEN_LAST 0x0067095au      /* mov [edi + 0x84], eax: the seed of the twelfth */
#define RE_VA_RANDGEN_SPARE 0x00670996u     /* mov [edi + 0x90], ecx: the thirteenth engine */
#define RE_VA_RAND_UNIT_ENGINE 0x00672909u  /* push dword ptr [ecx + 8], ecx = a stream */
#define RE_VA_RAND_UNIT_CALLS 0x0067290cu   /* inc dword ptr [ecx + 4] */
#define RE_VA_FIRMS_RANDGEN 0x004ea191u     /* mov eax, [ecx + 0x40] */
#define RE_VA_RANDGEN_JOB 0x004ea194u       /* add eax, 0x60: the stream a person is drawn from */
#define RE_VA_JOBS_RANDGEN 0x0051b4a4u      /* mov eax, [esi + 0x40] */
#define RE_VA_FIRMS_AGES 0x004e9d98u        /* mov ecx, [ebx + 0x14]: the object that draws a new person's age */
#define RE_VA_AGES_RANDGEN 0x004ea5dcu      /* mov eax, [ebx + 0x40] */
#define RE_RAND_STREAMS 12
#define RE_RAND_ENGINES 13
#define RE_RAND_UNIT_BYTES 0xc
#define RE_RAND_UNIT_CALLS 4
#define RE_RAND_UNIT_ENGINE 8
#define RE_RAND_ENGINE_BYTES 0x9c4
#define RE_RANDGEN_SPARE 0x90
#define RE_RANDGEN_JOB 0x60
#define RE_FIRMS_RANDGEN 0x40
#define RE_JOBS_RANDGEN 0x40
#define RE_FIRMS_AGES 0x14
#define RE_AGES_RANDGEN 0x40
/* the reference wage of a new person: one normal from the thirteenth engine, seeded with the person's id */
#define RE_VA_WAGE_REF 0x0051b480u          /* ecx = jobs; job id, skill part, seed (ret 0xc); edx:eax */
#define RE_VA_WAGE_REF_RET 0x0051b5deu
#define RE_VA_CALL_WAGE_REF 0x004ea3e2u     /* in the generator of a person */
#define RE_VA_WAGE_REF_ID 0x004ea3dbu       /* push dword ptr [eax + 0x38]: the seed it is handed there */
#define RE_VA_WAGE_REF_SEED 0x0051b496u     /* cmp dword ptr [ebp + 0x10], 0: the third argument; negative = no draw */
#define RE_VA_WAGE_REF_ENGINE 0x0051b521u   /* mov ecx, [ecx + 0x90] */
#define RE_VA_CALL_WAGE_REF_SEED 0x0051b527u
/* the staff tab of the hire window is where the game asks for Shift */
#define RE_VA_STAFF_TAB_SHIFT 0x00629b56u       /* mov dword ptr [ebp + 8], 0xc */
#define RE_VA_STAFF_TAB_KEYS 0x00629b5eu        /* mov ecx, [esi + 0x70]: the keys held down, esi = the window */
#define RE_VA_CALL_STAFF_TAB_HELD 0x00629b61u   /* counts the key among them */
#define RE_VA_KEY_HELD 0x005229d0u
#define RE_VA_STAFF_TAB_SHIFT_RIGHT 0x00629b71u /* mov dword ptr [ebp + 8], 0xd */
#define RE_VA_KEYS_NODE 0x0040e330u             /* mov esi, [ecx + 0x10]: a key in its node, as in every map here */
#define RE_WINDOW_KEYS 0x70 /* pointer to a std::set<int> */
#define RE_KEY_SHIFT 0xc
#define RE_KEY_SHIFT_RIGHT 0xd
#define RE_KEY_CTRL 0xe
#define RE_KEY_CTRL_RIGHT 0xf
/* the stock window's list of companies: a click on a row (FUN_007abba0) asks whether a Ctrl key is held - then
 * the company becomes a shortcut - and, when none is, shows the company */
#define RE_VA_STOCK_ROW_KEYS 0x007abc4eu         /* mov ebx, [edi + 0x70]: the keys held down, edi = the window */
#define RE_VA_STOCK_ROW_CTRL 0x007abc47u         /* mov dword ptr [ebp + 8], 0xe */
#define RE_VA_CALL_STOCK_ROW_CTRL 0x007abc53u
#define RE_VA_STOCK_ROW_CTRL_RIGHT 0x007abc63u   /* mov dword ptr [ebp + 8], 0xf */
#define RE_VA_CALL_STOCK_ROW_CTRL_RIGHT 0x007abc6du
#define RE_VA_KEY_HELD_RET 0x00522a38u           /* ret 4 */
#define RE_VA_CALL_STOCK_ROW_SHOW 0x007abc7fu
#define RE_VA_STOCK_SHOW 0x007ac6f0u             /* ecx = the stock window; the company id (ret 4) */
#define RE_VA_STOCK_SHOW_RET 0x007ae00cu
/* the stock window's left side, built anew: its list of companies with the buttons above it. The game calls it
 * when a trade or a listing has changed what the list shows (FUN_007bbfc0). */
#define RE_VA_STOCK_LEFT 0x007ac0b0u     /* ecx = the stock window; no argument */
#define RE_VA_STOCK_LEFT_RET 0x007ac6eau /* ret */
/* a company's first tab (FUN_007b5870): the "Research Stock" button is made by the game's maker of an action
 * button, which hands a shared pointer back through its first argument; the tab adds the button only when that
 * pointer is not null. The sixth stack word of the call is the company's id. */
#define RE_VA_CALL_RESEARCH_BUTTON 0x007b5a97u
#define RE_VA_ACTION_BUTTON 0x0060bfb0u
#define RE_VA_ACTION_BUTTON_RET 0x0060c423u      /* ret 0x84: re_action_button_hook removes as much when it makes none */
#define RE_VA_RESEARCH_BUTTON_COMPANY 0x007b5a72u /* push dword ptr [esi + 0x134]: the company's id */
#define RE_VA_RESEARCH_BUTTON_THREE 0x007b5a7eu   /* push ecx / push 0 / push [eax + 8]: three more words */
#define RE_VA_RESEARCH_BUTTON_TWO 0x007b5a94u     /* push 0xb / push eax: and the last two, the result's place last */
#define RE_VA_CALL_ROW_RESEARCHED 0x007b5fbau     /* the row "Last Researched"; RE_VA_COMPANY_RESEARCHED is its return */
/* a row of the list of companies (FUN_00610940, the game's maker of an "item title": picture, name, shares held).
 * It works on a copy of the company and hands the copy's name to the title as a string by value. */
#define RE_VA_CALL_STOCK_ITEM_NAME 0x00610ab0u /* copies the name: ecx = the argument's place, the name's address */
#define RE_VA_STOCK_ITEM_NAME 0x00610aa1u      /* lea eax, [ebp - 0x1fc]: the name in the copy */
#define RE_STOCK_ITEM_FRAME_NAME (-0x1fc)
#define RE_VA_STOCK_LIST_ITEM_RET 0x007ab360u  /* where the list's builder gets back from the maker */
/* ... and what it does next with the item it has made: ecx = the UI object; the item and its count (a shared
 * pointer by value), 0x14, 0 (ret 0x10). The item is a small record: its title, a label, is at + 0xc. */
#define RE_VA_CALL_STOCK_ITEM_MADE 0x00610b28u
#define RE_VA_ITEM_MADE 0x006b0690u
#define RE_VA_ITEM_MADE_RET 0x006b07a1u
#define RE_VA_ITEM_TITLE 0x006b2b8cu /* mov [eax + 0xc], ecx: the label the item's maker has just made */
#define RE_ITEM_TITLE 0xc
/* a node's colour: the virtual function the game calls with the address of three bytes (red, green, blue) */
#define RE_VA_NODE_COLOUR 0x005ab47au /* call dword ptr [eax + 0x25c], for the label of a chart's box */
#define RE_NODE_COLOUR 0x25c
/* the order of that list (note m23): FUN_007aae40 copies the market's companies into a vector and, by the sort
 * button that is lit (window + 0x30), has one of four functions put it in order. Three of them take the first
 * copy in ecx, the end in edx and, on the stack, a count and a flag, which the caller removes itself. */
#define RE_VA_CALL_STOCK_SORT_TYPE 0x007ab153u /* button 3, "by company type" */
#define RE_VA_STOCK_SORT_TYPE 0x007c4e20u
#define RE_VA_CALL_STOCK_SORT_CAP 0x007ab18bu /* button 4, "by market capitalisation" */
#define RE_VA_STOCK_SORT_CAP 0x007c5240u
#define RE_VA_CALL_STOCK_SORT_EPS 0x007ab1c3u /* button 5, "by earnings a share" */
#define RE_VA_STOCK_SORT_EPS 0x007c56b0u
#define RE_VA_STOCK_SORT_FIRST 0x007ab127u      /* mov ecx, [ebp - 0x128]: the first copy */
#define RE_VA_STOCK_SORT_FIRST_CAP 0x007ab178u  /* the same before the second call */
#define RE_VA_STOCK_SORT_FIRST_EPS 0x007ab1b0u  /* and before the third */
#define RE_VA_STOCK_SORT_LAST 0x007ab150u       /* mov edx, edi: the end */
#define RE_VA_STOCK_SORT_LAST_CAP 0x007ab188u
#define RE_VA_STOCK_SORT_LAST_EPS 0x007ab1c0u
#define RE_VA_STOCK_SORT_TYPE_DONE 0x007ab158u  /* jmp 0x7ab1c8 */
#define RE_VA_STOCK_SORT_CAP_DONE 0x007ab190u   /* jmp 0x7ab1c8 */
#define RE_VA_STOCK_SORT_CLEAN 0x007ab1c8u      /* add esp, 8: the caller removes the count and the flag */
#define RE_VA_STOCK_SORT_LISTING 0x007ab11cu    /* je 0x7ab1cb after cmp eax, 1: button 2, "by listing date", sorts nothing */
#define RE_VA_STOCK_SORT_ON 0x007ab1cbu         /* mov eax, [esi + 0x144]: where the list goes on after any order */
#define RE_VA_COMPANY_STEP 0x007c5382u          /* lea edi, [ebx + 0x188]: the next copy, in the game's own sort */
#define RE_COMPANY_SIZE 0x188
#define RE_VA_COMPANY_CLONE 0x00495080u         /* ecx = memory for a company; the company to copy (ret 4) */
#define RE_VA_COMPANY_CLONE_RET 0x004952c2u
#define RE_VA_COMPANY_ASSIGN 0x0048f240u        /* ecx = a company; the one it becomes a copy of (ret 4) */
#define RE_VA_COMPANY_ASSIGN_RET 0x0048f56fu
#define RE_VA_COMPANY_DROP 0x0053c440u          /* ecx = a company: its destructor */
#define RE_VA_COMPANY_DROP_RET 0x0053c532u
#define RE_VA_STOCK_BUTTONS 0x007bc550u         /* FUN_007bc550 builds the five sort buttons with their hover texts */
#define RE_STOCK_BUTTONS_SIZE 2790u
/* the chart window (the class UIChart, note m23 section 3), which draws every chart of the game. FUN_005aabb0
 * builds the boxes that switch its series, one for a series, and ends by ticking them all. */
#define RE_VA_CALL_CHART_TICK_ALL 0x005ab772u
#define RE_VA_CHART_TICK 0x005ab7a0u     /* ecx = chart; the "all" box, 1 = every box as the "all" flag, 0 = as recorded (ret 8) */
#define RE_VA_CHART_TICK_RET 0x005ab869u
#define RE_VA_CHART_ALL 0x005ab7afu      /* movzx eax, byte ptr [ebx + 0x14c]: the "all" flag */
#define RE_CHART_ALL 0x14c
#define RE_VA_CHART_BOXES 0x005ab7bdu    /* mov eax, [ebx + 0x158]: std::map<series, box>, the boxes of the page shown */
#define RE_CHART_BOXES 0x158
#define RE_VA_CHART_TICKS 0x005ab7cdu    /* lea ecx, [ebx + 0x144]: std::map<series, bool>, what is ticked */
#define RE_CHART_TICKS 0x144
#define RE_VA_TICK_NODE_VALUE 0x00440c44u /* lea eax, [esi + 0x14]: the bool in its node */
#define RE_TICK_NODE_VALUE 0x14
#define RE_VA_CHART_KIND 0x005a8276u     /* mov [edi + 0xfc], eax: what the chart is of */
#define RE_CHART_KIND 0xfc
#define RE_CHART_KIND_STATEMENT 1        /* the tags of one statement: a company's, the household's wealth */
#define RE_VA_CALL_CHART_NAME 0x005ab24eu /* the name of a series' tag, for its box */
/* more of the charts (REQUEST.md [44], note m23 section 5) */
#define RE_VA_CHART_OPEN 0x005a81f0u       /* FUN_005a81f0 opens a chart: ecx = chart; statement, kind, most months, titles */
#define RE_CHART_OPEN_SIZE 2561u           /* it writes the time button's text too: months / 12 and the word for years */
#define RE_VA_CALL_CHART_BOXES 0x005a8ba0u /* in it: the series have their colours, now the boxes are built ... */
#define RE_VA_CHART_BOXES_FN 0x005aabb0u   /* ... by FUN_005aabb0 (ecx = chart, nothing else). A page turn of the boxes
                                              comes to that function too, not through this call */
#define RE_VA_CHART_COLOURS 0x005a8a47u    /* lea eax, [edi + 0x150]: std::map<series, Color4F>, one entry a series of the chart */
#define RE_CHART_COLOURS 0x150
#define RE_COLOUR_NODE_VALUE 0x14          /* four floats (FUN_005ab960 returns node + 0x14) */
#define RE_VA_COLOUR_NODE_VALUE 0x005ab994u /* lea eax, [esi + 0x14] */
/* where FUN_005a81f0 was called from says what the chart is: its return address is in its frame */
#define RE_VA_CALL_CHART_OPEN_GROWTH 0x007e61feu    /* the button of economic growth */
#define RE_VA_CALL_CHART_OPEN_RATE 0x007e72feu      /* ... of the central bank's rate */
#define RE_VA_CALL_CHART_OPEN_INFLATION 0x007e7712u /* ... of inflation */
#define RE_VA_ALL_PRICES_CHART 0x006a02d0u          /* FUN_006a02d0 opens the chart of all share prices (690 bytes) */
#define RE_ALL_PRICES_CHART_SIZE 690u
#define RE_CHART_KIND_ALL_PRICES 4
#define RE_STAT_GROWTH 1511    /* the three series the economy's buttons are named after (data/idmap.txt) */
#define RE_STAT_BANK_RATE 1521
#define RE_STAT_INFLATION 1531
#define RE_VA_CALL_CHART_TICK_BOX_ALL 0x005ac36fu /* a click on the box for all: the flag is turned round and every box of the
                                                     page set to it */
/* the drawing, FUN_005a8f20: for every month shown it takes the month's record from the statement and copies the
 * record's two maps <series, int64 value> for itself */
#define RE_VA_CALL_CHART_MONTH_FIRST 0x005a9156u
#define RE_VA_CALL_CHART_MONTH_SECOND 0x005a9313u
#define RE_VA_MAP_COPY 0x00410320u         /* ecx = the new map; the source and one more word (ret 8) */
#define RE_VA_MAP_COPY_RET 0x0041038fu
#define RE_VA_CHART_DRAW_MONTH 0x005a9115u /* mov [ebp - 0x118], ecx: the month being read, in the drawing's frame */
#define RE_CHART_DRAW_FRAME_MONTH (-0x118)
#define RE_VA_CHART_DRAW_CHART 0x005a9170u /* mov ecx, [ebp - 0xfc]: the chart */
#define RE_CHART_DRAW_FRAME_CHART (-0xfc)
#define RE_MONTH_NODE_VALUE 0x18           /* the int64 of a series in such a map's node */
/* the time button, FUN_005a8c00: 24 months -> 60 -> the most -> 24 */
#define RE_VA_CHART_TIME_BUTTON 0x005a8c00u /* its click: the step, then the button's text, months / 12 and the word for years */
#define RE_CHART_TIME_BUTTON_SIZE 794u
#define RE_VA_CHART_SHOWN 0x005a8cacu      /* mov eax, [ecx + 0x12c]: the months shown; ecx = chart */
#define RE_CHART_SHOWN 0x12c
#define RE_CHART_MOST 0x130
#define RE_VA_CHART_TIME_STEP 0x005a8cb2u  /* the 46 bytes that work out the next number of months and store it */
#define RE_VA_CHART_TIME_DONE 0x005a8ce0u  /* what follows them */
#define RE_CHART_TIME_STEP_BYTES 46
/* more of the charts (REQUEST.md [45], note m23 section 7): three more openers, the lines' width, a value behind a
 * series' name */
#define RE_VA_CALL_CHART_OPEN_WEALTH 0x0058bf8eu     /* the household's wealth (the finance window's last tab) */
#define RE_VA_CALL_CHART_OPEN_INDEX 0x007e654du      /* the stock index (the fourth button at the economy's numbers) */
#define RE_VA_CALL_CHART_OPEN_ALL_PRICES 0x006a054fu /* all share prices */
#define RE_STAT_NET_WORTH 1741  /* three of the nine series of the household's wealth (data/idmap.txt) */
#define RE_STAT_CASH 1742
#define RE_STAT_DEBT 1745
#define RE_STAT_INDEX_YEAR 1551 /* the stock index's growth over a year; 1546 is the growth of the month */
#define RE_VA_MAP_INT_AT 0x00440c10u     /* ecx = a std::map<int, ...>; the key's address (ret 4): the value's address, the
                                            entry made when there was none. The chart's ticking writes its record with it */
#define RE_VA_MAP_INT_AT_RET 0x00440c4au
#define RE_VA_CHART_STATEMENT 0x005a827fu /* mov [edi + 0x100], esi: the statement the chart draws */
#define RE_CHART_STATEMENT 0x100
/* the drawing puts every piece of a line down with DrawNode::drawLine, through the import table (`call [slot]`, six
 * bytes); its axes with DrawNode::drawSegment, which takes a radius */
#define RE_VA_CHART_LINE 0x005a96feu
#define RE_VA_DRAW_LINE_SLOT 0x008d37a8u
#define RE_VA_CHART_AXIS 0x005a9814u /* mov esi, [the slot of drawSegment] */
#define RE_VA_DRAW_SEGMENT_SLOT 0x008d37acu
/* FUN_005aabb0, a series' box: the name is copied for the label that goes next to the box */
#define RE_VA_CALL_CHART_LABEL 0x005ab2f1u  /* call string copy: ecx = the label's text; the name */
#define RE_VA_CHART_BOX_SERIES 0x005ab15au  /* mov [ebp - 0x230], ecx: the series whose box is being built */
#define RE_CHART_BOX_FRAME_SERIES (-0x230)
#define RE_VA_CHART_BOX_CHART 0x005aabf3u   /* mov [ebp - 0x22c], edi: the chart */
#define RE_CHART_BOX_FRAME_CHART (-0x22c)

/* experience (note m26): a person's experience is a statement at person + 0x4d8. What was ever gained of a kind is
 * in its map at + 0x10 (what was taken away at + 0x00), what requirements are measured against in its map at + 0x30.
 * FUN_0050db30 (ecx = statement, xmm1 = the factor, no stack arguments) multiplies every value of the map at + 0x30
 * and rounds it up; the month end of a household member calls it with 1 - xpDecayRate */
#define RE_VA_XP_DECAY 0x0050db30u
#define RE_VA_XP_EFFECTIVE 0x0050db3fu   /* mov eax, [edi + 0x30] */
#define RE_XP_EFFECTIVE 0x30
#define RE_XP_GAINED 0x10
#define RE_XP_LOST 0x00
#define RE_VA_CALL_XP_DECAY 0x006608aeu  /* in the month end of a household member (FUN_00660510) */
#define RE_VA_XP_STATEMENT 0x006608a4u   /* lea ecx, [edi + 0x4d8] */
#define RE_PERSON_XP 0x4d8
#define RE_VA_XP_GAINED 0x0050d7e4u      /* lea esi, [ecx + 0x10]: the reader of what was gained, for the "lifetime" column */
#define RE_VA_XP_DECAY_RET 0x0050dbbau   /* ret: nothing on the stack */

/* names (note m29): FUN_0081c640 reads one list of extraData/namesPeople and gives one of its lines, drawn with one
 * random number: the std::string to fill, the random unit, the list's file name by value (ret 0x20). Every name of a
 * person, of a rival and of a new company comes from it. FUN_0082b330 (ecx = the text archive, the std::string to
 * fill; ret 4) reads one string of a save: its length, a space, its bytes. */
#define RE_VA_NAME_PICK 0x0081c640u
#define RE_VA_NAME_PICK_RET 0x0081c83du      /* ret 0x20 */
#define RE_VA_ARCHIVE_STRING 0x0082b330u
#define RE_VA_ARCHIVE_STRING_RET 0x0082b407u /* ret 4 */

/* property (note b17_property_listings.md): the properties for sale and the two amounts of each */
#define RE_VA_HOUSE_PRICE 0x0054d190u      /* ecx = RestateM; record, std::string kind by value, flag (ret 0x20); edx:eax, cents */
#define RE_VA_HOUSE_PRICE_FLAG 0x0054d4e9u /* cmp byte ptr [ebp + 0x24], 0 / je: only the flag lets it draw a random number */
#define RE_VA_HOUSE_PRICE_RET 0x0054d569u
#define RE_VA_MAIN_RESTATE 0x00695584u     /* mov ecx, [ebx + 0x390] */
#define RE_VA_RESTATE_FOR_SALE 0x0054c922u /* mov eax, [edx + 0x9c] */
#define RE_VA_FOR_SALE_RECORD 0x0054c940u  /* lea ecx, [esi + 0x18] */
#define RE_VA_FOR_SALE_BYTES 0x0054c950u   /* add dword ptr [edi + 4], 0xb8 */
#define RE_VA_HOUSE_STATE 0x005e6ee8u      /* mov eax, [esi + 0x48] / cmp eax, 3 */
#define RE_VA_HOUSE_ASKING 0x005e6ef4u     /* mov eax, [esi + 0x28] / mov esi, [esi + 0x2c] */
#define RE_VA_HOUSE_ADDRESS 0x0054dc58u    /* lea ecx, [esi + 0x64] */
/* The character's two searches for property, each at the end of its action (note b17 "What the search action does"):
 * they put new properties on the market. ecx = real estate; two stack arguments (ret 8). */
#define RE_VA_SEARCH_ESTATE 0x00551c10u
#define RE_VA_SEARCH_ESTATE_RET 0x0055206eu
#define RE_VA_CALL_SEARCH_ESTATE 0x0043c61du
#define RE_VA_SEARCH_SITE 0x00552080u
#define RE_VA_SEARCH_SITE_RET 0x00552214u
#define RE_VA_CALL_SEARCH_SITE 0x0043c3d7u
/* What a purchase costs besides the price (note b17 "Fees and taxes"): the game multiplies the price by the double
 * at 0x8f9fb0 (0.03) and the property's value by the one at 0x8f9f28 (0.001). */
#define RE_VA_BUY_FEE 0x0050987bu       /* mulsd xmm0, qword ptr [0x8f9fb0] */
#define RE_VA_BUY_VALUE_FEE 0x0050984fu /* mulsd xmm0, qword ptr [0x8f9f28] */
#define RE_BUY_FEE 0.03
#define RE_BUY_VALUE_FEE 0.001
#define RE_RESTATE_FOR_SALE 0x9c           /* std::map<int, house record> */
#define RE_FOR_SALE_NODE_RECORD 0x18
#define RE_HOUSE_BYTES 0xb8
#define RE_HOUSE_STATE 0x48 /* int */
#define RE_HOUSE_FOR_SALE 3
#define RE_HOUSE_ASKING 0x28  /* int64, cents */
#define RE_HOUSE_ADDRESS 0x64 /* std::string */

/* memory (note m20_load_leak.md): the object that reads the game's data files keeps every XML document in 16 lists
 * and its destructor deletes none of them. */
#define RE_VA_LOADER_END 0x0080b560u        /* ecx = loader: its destructor */
#define RE_VA_LOADER_END_DOC 0x0080b595u    /* push 1 / call [eax + 0x40]: how it deletes the one document it does delete */
#define RE_VA_LOADER_END_FIRST 0x0080b59au  /* lea eax, [esi + 0xc]: the first list */
#define RE_VA_LOADER_END_SECOND 0x0080b5d7u /* lea eax, [esi + 0x20] */
#define RE_VA_LOADER_END_LAST 0x0080b854u   /* lea eax, [esi + 0x124]: the last of the lists that lie 0x14 apart */
#define RE_VA_LOADER_END_TEXT 0x0080b914u   /* lea eax, [esi + 0x130]: the list of the language files */
#define RE_VA_XML_DOC_END 0x0069b700u       /* ecx = document; 1 = free its memory too (ret 4): the deleting destructor */
#define RE_VA_XML_DOC_END_TABLE 0x0069b72cu /* mov dword ptr [esi], the document's table of virtual functions */
#define RE_VA_XML_DOC_TABLE 0x008ef950u
#define RE_LOADER_DOCS 0xc        /* std::vector of documents, the first of RE_LOADER_DOC_LISTS */
#define RE_LOADER_DOCS_STEP 0x14  /* to the next list: a std::map lies between two of them */
#define RE_LOADER_DOC_LISTS 15    /* one for each folder of data */
#define RE_LOADER_TEXT_DOCS 0x130 /* std::vector of documents: the language files */
#define RE_XML_DOC_END_SLOT 0x40  /* in a document's table: its deleting destructor */

/* The game's clean-up of an ended game deletes most of the objects that hold the game's state and leaves nine of
 * them. Each of the nine is read from a save through boost, so the exe has boost's `destroy` for its class (slot 6
 * of iserializer<text_iarchive, T>: one argument on the stack, ret 4, `delete (T *)address`). For each: where a new
 * game pushes the object's size and stores the object in the scene, that function, and the same size pushed for
 * operator delete at the end of the destructor it runs. */
#define RE_VA_GAME_END 0x00693130u      /* ecx = the scene of a game: the clean-up before the next game is made */
#define RE_VA_CALL_GAME_END 0x006c10deu /* its one call, in the function that starts or loads a game */
#define RE_VA_PFM_NEW 0x0068fc5cu       /* push 0x378: the household's money (PFM) */
#define RE_VA_PFM_STORE 0x0068fc89u     /* mov [edi + 0x388], eax */
#define RE_VA_PFM_DESTROY 0x006784d0u
#define RE_VA_PFM_DELETE 0x0067bde0u
#define RE_VA_PFM_FREE 0x0067be77u
#define RE_VA_ECONOMY_NEW 0x0068fc84u /* push 0x150: EconomyM */
#define RE_VA_ECONOMY_STORE 0x0068fcb1u
#define RE_VA_ECONOMY_DESTROY 0x006784f0u
#define RE_VA_ECONOMY_DELETE 0x0067be90u
#define RE_VA_ECONOMY_FREE 0x0067beffu
#define RE_VA_RESTATE_NEW 0x0068fcacu /* push 0xd4: RestateM, homes and land */
#define RE_VA_RESTATE_STORE 0x0068fcd9u
#define RE_VA_RESTATE_DESTROY 0x00678510u
#define RE_VA_RESTATE_DELETE 0x0067bf20u
#define RE_VA_RESTATE_FREE 0x0067c051u
#define RE_VA_FIRMS_NEW 0x0068fcd4u /* push 0xc4: FirmM, the businesses */
#define RE_VA_FIRMS_STORE 0x0068fd01u
#define RE_VA_FIRMS_DESTROY 0x00678530u
#define RE_VA_FIRMS_DELETE 0x0067c070u
#define RE_VA_FIRMS_FREE 0x0067c14bu
#define RE_VA_MARKET_NEW 0x0068fcfcu /* push 0x2b8: MarketM, the listed companies */
#define RE_VA_MARKET_STORE 0x0068fd29u
#define RE_VA_MARKET_DESTROY 0x00678550u
#define RE_VA_MARKET_DELETE 0x0067c160u
#define RE_VA_MARKET_FREE 0x0067c2c8u
#define RE_VA_RELATIONS_NEW 0x0068fd52u /* push 0xf0: RelationM; its destroy has the destructor's call in itself */
#define RE_VA_RELATIONS_STORE 0x0068fd79u
#define RE_VA_RELATIONS_DESTROY 0x00678590u
#define RE_VA_RELATIONS_FREE 0x006785a2u
#define RE_VA_RICH_NEW 0x0068fd74u /* push 0x104: RichM; the same shape */
#define RE_VA_RICH_STORE 0x0068fda1u
#define RE_VA_RICH_DESTROY 0x006785d0u
#define RE_VA_RICH_FREE 0x006785e2u
#define RE_VA_MOODS_NEW 0x0068fd24u /* push 0xc8: MoodM */
#define RE_VA_MOODS_STORE 0x0068fd4cu
#define RE_VA_MOODS_DESTROY 0x00678570u
#define RE_VA_MOODS_DELETE 0x0067c2f0u
#define RE_VA_MOODS_FREE 0x0067c384u
#define RE_VA_ACHIEVE_NEW 0x0068fd9cu /* push 0xac: AchieveM */
#define RE_VA_ACHIEVE_STORE 0x0068fdd1u
#define RE_VA_ACHIEVE_DESTROY 0x00678610u
#define RE_VA_ACHIEVE_DELETE 0x0067c3a0u
#define RE_VA_ACHIEVE_FREE 0x0067c436u
#define RE_MAIN_RESTATE 0x390
#define RE_MAIN_MARKET 0x398
#define RE_MAIN_RELATIONS 0x39c
#define RE_MAIN_RICH 0x3a0
#define RE_MAIN_MOODS 0x414
#define RE_MAIN_ACHIEVE 0x418

/* object layout */
#define RE_MAIN_PFM 0x388
#define RE_MAIN_ECONOMY 0x38c
#define RE_MAIN_COUNTER 0x3d0
#define RE_MAIN_GAMEDATA 0x3f8
#define RE_ECON_RANDGEN 0x40
#define RE_ECON_GROWTH 0x9c
#define RE_ECON_BASE_RATE 0xa8
#define RE_ECON_RESERVE_RATE 0xac
#define RE_RANDGEN_MARKET 0x48 /* RandUnit {seed, numCalls, engine*} that drives stock prices */
#define RE_RANDGEN_ECONOMY 0x6c /* the one of growth, a crash and the industries' cycles (note b2: 0x004d37c0, 0x004d5300) */
#define RE_PFM_DEBTINV 0xa8    /* DebtInvM inside PFM */
#define RE_DEBTINV_EVENTS 0x38 /* pointer to the owner of the monthly summary's event list */
#define RE_EVENTS_LIST 0x3c4   /* std::list of the month's lines inside that owner */
#define RE_PFM_CASHFLOW 0x2d0  /* IncomeSt booked by every cash movement */
#define RE_PFM_TAXABLE 0x280   /* IncomeSt the yearly tax is computed from: cash movements with the taxable flag */
#define RE_PFM_TAX_FREE 0x230  /* int64 cents: the part of a year's taxable income that is not taxed (0 until first used) */
#define RE_POLICY_VALUES 0x98  /* std::map<tag, float> inside the object at PFM + 0x78 */
#define RE_TAG_TAX_RATE 5011
#define RE_DEBTINV_DEBTS 0xa0  /* std::map<int, Instrument> */
#define RE_DEBTINV_PENDING_MORTGAGE 0xb8   /* an approved mortgage waiting for a purchase: one debt record */
#define RE_DEBTINV_PENDING_EDUCATION 0x118 /* an approved education loan waiting for an enrolment */
#define RE_OBJ_ID 0xc

extern const BYTE RE_GETTER_PROLOGUE[5]; /* also the first five bytes of the futures function */
extern const BYTE RE_CHANGE_MONEY_PROLOGUE[6];
extern const BYTE RE_STOCK_BUY_PROLOGUE[6];

/* Checks the sites of one group against the bytes of the analysed build. `report` gets one line per site.
 * Returns the number of sites that do not match (0 = safe to use). Reads only. */
int re_sites_verify(BYTE *base, int group, void (*report)(const char *line));

#endif
