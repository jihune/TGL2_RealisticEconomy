#include "re_lang.h"

#include <stddef.h>
#include <string.h>

static const char *const FOLDERS[RE_LANG_COUNT] = {"en", "ko", "de", "es", "fr", "pt-br", "jp", "zh_CN"};

/* One row per text, one column per language in the order of re_lang.h: en, ko, de, es, fr, pt-br (jp and zh_CN
 * have no column here: a row leaves them NULL). A new text gets its English and Korean here, NULL in the four
 * columns after them, and a line in each of the language files included below the table.
 * Numbers are written by printf, with a point; the texts write 0.75 the same way in every language.
 * PER, PBR and ROE are left as they are everywhere. */
static const char *const TEXTS[RE_MSG_COUNT][RE_LANG_COUNT] = {
    [RE_MSG_PER_LOSS] = {"PER: loss", "PER 적자", "PER: Verlust", "PER: pérdidas", "PER : perte", "PER: prejuízo"},
    /* The texts from here to RE_MSG_HOVER_INDUSTRIES were rewritten short on 2026-10-05 (REQUEST.md [30]) and are in
     * English and Korean only: the other languages come in one pass when the mod's texts no longer change. */
    [RE_MSG_HOVER_YIELD] = {", yield %.1f%%", ", 배당 %.1f%%", NULL, NULL, NULL, NULL},
    [RE_MSG_HOVER_UNDER] = {"PBR < 0.75: ", "PBR 0.75 미만: ", NULL, NULL, NULL, NULL},
    /* What a company under the line does is named as the game names it in a company's history ("downsizing",
     * "capital raise"); a word of the plugin's own for both was not understood (REQUEST.md [32]). */
    [RE_MSG_HOVER_BANKRUPT] = {"%s%sbankrupt at month end%s", "%s%s월말에 파산%s", NULL, NULL, NULL, NULL},
    [RE_MSG_HOVER_MEASURE] = {"%s%sdownsizing or new shares at month end%s", "%s%s월말 축소·증자%s", NULL, NULL, NULL, NULL},
    [RE_MSG_HOVER_WATCH] = {"%sWithin %d mo.: bankrupt if the price falls %.1f%%%s", "%s%d개월 안에 주가 -%.1f%%면 파산%s", NULL, NULL,
                            NULL, NULL},
    /* the Korean line with "규모 축소·증자" was one letter too wide for the hover box (run 53) */
    [RE_MSG_HOVER_NEAR] = {"Price -%.1f%%: downsizing or new shares", "주가 -%.1f%%면 월말 축소·증자", NULL, NULL, NULL, NULL},
    [RE_MSG_HOVER_FAIR] = {"Fair %s (%+.0f%%)", "적정 %s (%+.0f%%)", NULL, NULL, NULL, NULL},
    [RE_MSG_HOVER_TAILWIND] = {", industry tailwind", ", 업종 호조", NULL, NULL, NULL, NULL},
    [RE_MSG_HOVER_HEADWIND] = {", industry headwind", ", 업종 부진", NULL, NULL, NULL, NULL},
    [RE_MSG_NO_FAIR] = {"No fair price (the equity is gone)", "적정 주가 없음 (자본이 바닥남)",
                        "Kein fairer Kurs (Eigenkapital aufgebraucht)", "Sin precio justo (el capital se ha agotado)",
                        "Pas de juste prix (capitaux propres épuisés)", "Sem preço justo (o patrimônio líquido acabou)"},
    [RE_MSG_ROW_FAIR] = {" (fair %s)", " (적정 %s)", " (fair %s)", " (justo %s)", " (juste %s)", " (justo %s)"},
    [RE_MSG_ROW_BANKRUPT] = {"bankrupt at month end", "월말 파산", "Insolvenz am Monatsende", "quiebra a fin de mes",
                             "faillite en fin de mois", "falência no fim do mês"},
    [RE_MSG_ROW_MEASURE] = {"downsizing or dilution", "월말 축소·증자", NULL, NULL, NULL, NULL},
    [RE_MSG_ROW_PER_MONTH] = {" (%s a month)", " (월 %s)", NULL, NULL, NULL, NULL},
    [RE_MSG_ROW_WATCH] = {"bankruptcy watch", "파산 주의", "Insolvenzgefahr", "riesgo de quiebra", "risque de faillite",
                          "risco de falência"},
    /* the words for "of the equity" are too long for the row in these four languages: the window shrank the value */
    [RE_MSG_ROW_OF_EQUITY] = {" (%.1f%% of equity)", " (자본의 %.1f%%)", " (ROE %.1f%%)", " (ROE %.1f%%)", " (ROE %.1f%%)",
                              " (ROE %.1f%%)"},
    [RE_MSG_ROW_OF_PRICE] = {" (%.1f%% of the price)", " (주가의 %.1f%%)", " (%.1f%% des Kurses)", " (%.1f%% del precio)",
                             " (%.1f%% du cours)", " (%.1f%% do preço)"},
    [RE_MSG_IPO_REST] = {", the rest sells for %s", ", 매각 대금 %s", ", der Rest wird für %s verkauft", ", el resto se vende por %s",
                         ", le reste se vend pour %s", ", o restante é vendido por %s"},
    [RE_MSG_EDU_FIXED] = {"Fixed interest rate per year (the central-bank rate of the month you enrol)",
                          "연간 고정 이자율 (등록하는 달의 기준금리)",
                          "Fester Zinssatz pro Jahr (Zinssatz der Zentralbank im Monat der Einschreibung)",
                          "Tipo de interés fijo anual (el del Banco Central en el mes de la matrícula)",
                          "Taux d'intérêt fixe par an (celui de la Banque centrale le mois de l'inscription)",
                          "Taxa de juros fixa por ano (a do Banco Central no mês da matrícula)"},
    [RE_MSG_GRADE_QUOTE] = {" (credit grade %s, taken again at purchase)", " (신용등급 %s, 구입할 때 다시 계산)",
                            " (Bonität %s, beim Kauf neu berechnet)", " (calificación crediticia %s, se recalcula al comprar)",
                            " (note de crédit %s, recalculée à l'achat)", " (nota de crédito %s, recalculada na compra)"},
    [RE_MSG_GRADE] = {" (credit grade %s)", " (신용등급 %s)", " (Bonität %s)", " (calificación crediticia %s)",
                      " (note de crédit %s)", " (nota de crédito %s)"},
    [RE_MSG_GRADE_PREFIX] = {"Credit grade ", "신용등급 ", "Bonität ", "Calificación crediticia ", "Note de crédit ",
                             "Nota de crédito "},
    [RE_MSG_GRADE_CHANGE] = {"Credit grade %s -> %s", "신용등급 %s → %s", "Bonität %s -> %s", "Calificación crediticia %s -> %s",
                             "Note de crédit %s -> %s", "Nota de crédito %s -> %s"},
    [RE_MSG_FORECAST_PREFIX] = {"This month end: ", "이번 월말 ", "Monatsende: ", "Fin de mes: ", "Fin de mois : ", "Fim do mês: "},
    [RE_MSG_FORECAST_LOANS] = {"%sloan payments %s", "%s대출 납부 %s", "%sKreditraten %s", "%spagos de préstamos %s",
                               "%sremboursements de prêts %s", "%sparcelas de empréstimos %s"},
    [RE_MSG_FORECAST_CASH] = {"%scash needed %s", "%s필요 현금 %s", "%sBargeldbedarf %s", "%sefectivo necesario %s",
                              "%sliquidités nécessaires %s", "%sdinheiro necessário %s"},
    [RE_MSG_LOCK_PREFIX] = {"Trade lock", "거래 잠금", "Handelssperre", "Bloqueo de operaciones", "Blocage des transactions",
                            "Bloqueio de negociação"},
    [RE_MSG_LOCK_RUNS] = {"Trade lock: another ", "거래 잠금: 앞으로 ", "Handelssperre: noch ", "Bloqueo de operaciones: quedan ",
                          "Blocage des transactions : encore ", "Bloqueio de negociação: faltam "},
    [RE_MSG_LOCK_RUNS_REST] = {"%s%lld hour(s) (stocks, futures)", "%s%lld시간 (주식·선물)", "%s%lld Std. (Aktien, Terminmarkt)",
                               "%s%lld h (acciones, futuros)", "%s%lld h (actions, contrats à terme)", "%s%lld h (ações, futuros)"},
    [RE_MSG_LOCK_NONE] = {"%s: none", "%s 없음", "%s: keine", "%s: ninguno", "%s : aucun", "%s: nenhum"},
    [RE_MSG_LOCK_OVER] = {"%s over", "%s 해제", "%s aufgehoben", "%s terminado", "%s levé", "%s encerrado"},
    [RE_MSG_AUTO_PREFIX] = {"Auto transfer:", "자동 이체 매수", "Automatische Überweisung:", "Transferencia automática:",
                            "Transfert automatique :", "Transferência automática:"},
    [RE_MSG_AUTO_SKIPPED] = {"%s %d stock purchase(s) skipped (trade lock)", "%s %d건 건너뜀 (거래 잠금)",
                             "%s %d Aktienkauf/-käufe übersprungen (Handelssperre)", "%s %d compra(s) de acciones omitida(s) (bloqueo)",
                             "%s %d achat(s) d'actions ignoré(s) (blocage)", "%s %d compra(s) de ações ignorada(s) (bloqueio)"},
    [RE_MSG_HELD_PREFIX] = {"Shares held, warning: ", "보유 주식 경고: ", NULL, NULL, NULL, NULL},
    [RE_MSG_HELD_CLOSE] = {"price -%.1f%%: downsizing or new shares at month end", "주가 -%.1f%%면 월말 규모 축소·증자", NULL, NULL, NULL,
                           NULL},
    [RE_MSG_HELD_WATCH] = {"price -%.1f%%: bankrupt at month end", "주가 -%.1f%%면 월말 파산", NULL, NULL, NULL, NULL},
    [RE_MSG_HELD_MORE] = {" and %d more", " 외 %d곳", NULL, NULL, NULL, NULL},
    [RE_MSG_LOCK_REFUSED] = {"Trade lock: no stock or futures trades for another %lld hour(s)",
                             "거래 잠금 중: 앞으로 %lld시간 동안 주식·선물 거래 불가",
                             "Handelssperre: noch %lld Stunde(n) kein Aktien- oder Terminhandel",
                             "Bloqueo de operaciones: sin operar con acciones ni futuros durante %lld hora(s) más",
                             "Blocage des transactions : pas d'actions ni de contrats à terme pendant encore %lld heure(s)",
                             "Bloqueio de negociação: sem negociar ações ou futuros por mais %lld hora(s)"},
    [RE_MSG_CASINO_SLOTS] = {"Slots", "슬롯", NULL, NULL, NULL, NULL},
    [RE_MSG_CASINO_ROULETTE] = {"Roulette", "룰렛", NULL, NULL, NULL, NULL},
    [RE_MSG_CASINO_BLACKJACK] = {"Blackjack", "블랙잭", NULL, NULL, NULL, NULL},
    [RE_MSG_CASINO_BACCARAT] = {"Baccarat", "바카라", NULL, NULL, NULL, NULL},
    [RE_MSG_CASINO_WON] = {"%s: won %sx the stake (+%s)", "%s: 판돈의 %s배 당첨 (+%s)", NULL, NULL, NULL, NULL},
    [RE_MSG_CASINO_LOST] = {"%s: nothing won (-%s)", "%s: 꽝 (-%s)", NULL, NULL, NULL, NULL},
    [RE_MSG_CASINO_PLAYED] = {"%s: already played this month, before the load. This game has no result",
                              "%s: 이번 달에는 로드 전에 이미 했습니다. 이번 판은 결과 없음", NULL, NULL, NULL, NULL},
    [RE_MSG_CASINO_BEFORE] = {"%s: begun before the mod. The game booked its result when it began; nothing more is booked",
                              "%s: 모드를 넣기 전에 시작한 판입니다. 결과는 시작할 때 이미 났고, 더 들어오거나 나가는 돈은 없습니다", NULL,
                              NULL, NULL, NULL},
    [RE_MSG_ADOPT_PREFIX] = {"A save from before the mod: ", "모드를 넣기 전의 세이브: ", NULL, NULL, NULL, NULL},
    [RE_MSG_ADOPT] = {"%s\"buy worn-out assets again\" switched off in %d business(es), automatic management of %d employee(s) "
                      "switched off. Both do more with the mod: switch them on again where you want that",
                      "%s사업체 %d곳의 낡은 자산 자동 구매와 직원 %d명의 자동 관리를 껐습니다. 모드에서는 두 스위치가 하는 일이 "
                      "늘었으니 원하는 곳만 다시 켜세요",
                      NULL, NULL, NULL, NULL},
    [RE_MSG_ADOPT_ASSETS] = {"%s\"buy worn-out assets again\" switched off in %d business(es). It does more with the mod: switch it on "
                             "again where you want that",
                             "%s사업체 %d곳의 낡은 자산 자동 구매를 껐습니다. 모드에서는 이 스위치가 하는 일이 늘었으니 원하는 곳만 다시 "
                             "켜세요",
                             NULL, NULL, NULL, NULL},
    [RE_MSG_ADOPT_STAFF] = {"%sautomatic management of %d employee(s) switched off. It does more with the mod: switch it on again "
                            "where you want that",
                            "%s직원 %d명의 자동 관리를 껐습니다. 모드에서는 이 스위치가 하는 일이 늘었으니 원하는 곳만 다시 켜세요", NULL,
                            NULL, NULL, NULL},
    [RE_MSG_ADOPT_CASINO] = {"%d casino game(s) taken out of the action queue. With the mod a game has a stake and its result comes "
                             "when it ends: queue it again if you want it",
                             "대기열에 있던 카지노 행동 %d개를 뺐습니다. 모드에서는 판돈을 걸고 결과가 끝날 때 나오니 하려면 다시 "
                             "넣으세요",
                             NULL, NULL, NULL, NULL},
    [RE_MSG_ADOPT_CASINO_BEGUN] = {"%d casino game(s) taken out of the action queue. %d of them had begun: its result is what the game "
                                   "gave when it began, and it counts as played this month. With the mod a game has a stake and its "
                                   "result comes when it ends: queue it again if you want it",
                                   "대기열에 있던 카지노 행동 %d개를 뺐습니다. 그 가운데 %d개는 이미 시작한 판이라 결과는 시작할 때 난 "
                                   "그대로이고 이번 달에 한 것으로 칩니다. 모드에서는 판돈을 걸고 결과가 끝날 때 나오니 하려면 다시 "
                                   "넣으세요",
                                   NULL, NULL, NULL, NULL},
    [RE_MSG_CASINO_BACK_PREFIX] = {"Casino: ", "카지노: ", NULL, NULL, NULL, NULL},
    [RE_MSG_CASINO_BACK] = {"%s%d game(s) played before this load still count (cash %s)", "%s로드 전에 한 도박 %d판은 그대로 유지 (현금 %s)",
                            NULL, NULL, NULL, NULL},
    /* one line of the hover box, about 34 columns: "Monthly. Wins: 22%: 3x, 0.8%: 40x" */
    [RE_MSG_CASINO_TIP] = {"Monthly. Wins: ", "월 1회. 당첨 ", NULL, NULL, NULL, NULL},
    [RE_MSG_CASINO_TIP_PRIZE] = {"%s%s%%: %sx", "%s%s%%: %s배", NULL, NULL, NULL, NULL},
    /* the game's own words: "시간당 지불" (pay per hour), "업무 효율성" (work efficiency), "근무 시간" (job hours) */
    [RE_MSG_STAFF_LINE] = {"%s per effective hour", "효율 반영 시간당 %s", NULL, NULL, NULL, NULL},
    [RE_MSG_HIRE_ORDER] = {" (cheapest per effective hour first)", " (효율 반영 시간당 임금이 낮은 순)", NULL, NULL, NULL, NULL},
    [RE_MSG_STAFF_TIP] = {"%s per effective hour (pay per hour / work efficiency %.0f%%)",
                          "효율 반영 시간당 임금 %s (시간당 지불 ÷ 업무 효율성 %.0f%%)", NULL, NULL, NULL, NULL},
    /* the payout against the standard cost of the work the contract brings (note b18): the hours it lists and the
     * hours they set off, at the jobs' standard wages, with the wares and utilities of that work. The firm's own
     * staff and what it is paid are not in it. */
    /* two lines: the hover text is about 38 characters wide and breaks a longer line in the middle of the amount */
    [RE_MSG_OFFER_TIP] = {"%+.1f%% over standard cost\n%s left per hour", "표준 비용 %+.1f%%\n시간당 %s 남음", NULL, NULL, NULL, NULL},
    /* "(expected 5.05%)" ran out of the window on the right (run 181) */
    [RE_MSG_FUT_EXPECTED] = {" (exp. %.2f%%)", " (예상 %.2f%%)", NULL, NULL, NULL, NULL},
    [RE_MSG_FUT_TIP] = {"Expected: what the rate averages until the chosen expiry if it moves by the game's own rule. "
                        "Above today's rate a purchase is ahead, below it a sale.",
                        "예상: 게임 규칙대로 움직일 때 고른 만료 월까지의 평균 인플레이션율. 지금 값보다 높으면 구매가, 낮으면 판매가 "
                        "유리합니다.",
                        NULL, NULL, NULL, NULL},
    /* short: a long title is drawn in a smaller type (run 181, " (+8.5% a year)") */
    [RE_MSG_FUT_ITEM] = {" (%+.1f%%/yr)", " (연 %+.1f%%)", NULL, NULL, NULL, NULL},
    /* the rate now and the twelve months' expectation; no words, the hover text of the row has them */
    [RE_MSG_FUT_ITEM_AHEAD] = {" (%+.1f%% → %+.1f%%)", " (%+.1f%% → %+.1f%%)", NULL, NULL, NULL, NULL},
    /* short lines: the hover text is about 38 characters wide */
    [RE_MSG_FUT_ROW_TIP] = {"Inflation %+.2f%% a year\nExpected, 6 months: %+.2f%%\nExpected, 12 months: %+.2f%%\nExpected, 24 months: "
                            "%+.2f%%\nExpected higher: buy. Lower: sell.",
                            "인플레이션율 연 %+.2f%%\n예상 6개월 %+.2f%%\n예상 12개월 %+.2f%%\n예상 24개월 %+.2f%%\n예상이 높으면 구매, "
                            "낮으면 판매",
                            NULL, NULL, NULL, NULL},
    [RE_MSG_FUT_SORT] = {"Sort by expected rate\n(12 months) minus rate now", "12개월 예상 - 지금 비율\n큰 순 정렬", NULL, NULL, NULL, NULL},
    [RE_MSG_CHART_HALF_YEAR] = {"6 months", "6개월", NULL, NULL, NULL, NULL},
    [RE_MSG_CHART_INDEX] = {" (first month shown = 100)", " (그림의 첫 달 = 100)", NULL, NULL, NULL, NULL},
    [RE_MSG_RESEARCH_PREFIX] = {"Stock research subscription: ", "주식 조사 구독: ", NULL, NULL, NULL, NULL},
    [RE_MSG_RESEARCH_PAID] = {"%s%d companies researched again, fee %s", "%s회사 %d곳을 다시 조사했습니다, 구독료 %s", NULL, NULL, NULL,
                              NULL},
    [RE_MSG_RESEARCH_SHORT] = {"%s%d companies researched again, fee %s; your cash was short, so %s of it is now a debt",
                               "%s회사 %d곳을 다시 조사했습니다, 구독료 %s. 돈이 모자라 부채 %s 추가", NULL, NULL, NULL, NULL},
    [RE_MSG_RESEARCH_KEPT_PREFIX] = {"Research fee paid before: ", "이미 낸 조사 구독료: ", NULL, NULL, NULL, NULL},
    [RE_MSG_RESEARCH_KEPT] = {"%sloading an earlier save does not bring it back. %d companies paid for this month were researched "
                              "again, fee %s",
                              "%s예전 세이브를 불러와도 돌아오지 않습니다. 이번 달에 구독료를 냈던 회사 %d곳을 다시 조사했고 %s 를 "
                              "다시 냈습니다",
                              NULL, NULL, NULL, NULL},
    [RE_MSG_RESEARCH_KEPT_SHORT] = {"%sloading an earlier save does not bring it back. %d companies paid for this month were "
                                    "researched again, fee %s; your cash was short, so %s of it is now a debt",
                                    "%s예전 세이브를 불러와도 돌아오지 않습니다. 이번 달에 구독료를 냈던 회사 %d곳을 다시 조사했고 "
                                    "%s 를 다시 냈습니다. 돈이 모자라 부채 %s 추가",
                                    NULL, NULL, NULL, NULL},
    [RE_MSG_RESEARCH_KEPT_TICK] = {"Research fee paid before this load, taken again: %s (%d companies)",
                                   "불러오기 전에 냈던 조사 구독료를 다시 냈습니다: %s (회사 %d곳)", NULL, NULL, NULL, NULL},
    /* the ticker shows a line letter by letter and long ones in full (run 131) */
    [RE_MSG_RESEARCH_ONE_ON] = {"Research subscription: %s on. Researched now, fee %s (every month)",
                                "조사 구독: %s 켬. 지금 조사했습니다, 구독료 %s (달마다)", NULL, NULL, NULL, NULL},
    [RE_MSG_RESEARCH_ONE_OFF] = {"Research subscription: %s off. No fee from next month", "조사 구독: %s 끔. 다음 달부터 받지 않습니다",
                                 NULL, NULL, NULL, NULL},
    [RE_MSG_RESEARCH_ALL_ON] = {"Research subscription: every company on. %d researched now, fee %s",
                                "조사 구독: 모든 회사 켬. %d곳을 지금 조사했습니다, 구독료 %s", NULL, NULL, NULL, NULL},
    [RE_MSG_RESEARCH_ALL_OFF] = {"Research subscription: all off. No fee from next month", "조사 구독: 전부 끔. 다음 달부터 받지 않습니다",
                                 NULL, NULL, NULL, NULL},
    [RE_MSG_RESEARCH_ONE_ON_DONE] = {"Research subscription: %s on. This month's research is there, the fee starts next month",
                                     "조사 구독: %s 켬. 이번 달 조사는 이미 있어 다음 달부터 받습니다", NULL, NULL, NULL, NULL},
    [RE_MSG_RESEARCH_ALL_HAS] = {"%s is subscribed with every company (Ctrl+Shift-click: all off)",
                                 "%s: 모든 회사 구독에 들어 있습니다 (Ctrl+Shift-클릭: 전부 끔)", NULL, NULL, NULL, NULL},
    [RE_MSG_RESEARCH_NO_CASH] = {"Research subscription: not switched on, your cash does not cover the fee of %s",
                                 "조사 구독: 현금이 모자라 켜지 못했습니다 (구독료 %s)", NULL, NULL, NULL, NULL},
    [RE_MSG_RESEARCH_BOARD] = {"%s: you are on its board, its research is free", "%s: 이사회 회사라 조사가 무료입니다", NULL, NULL, NULL,
                               NULL},
    [RE_MSG_HOVER_CHANGE] = {"%+.1f%% on the month before", "전월 대비 %+.1f%%", NULL, NULL, NULL, NULL},
    [RE_MSG_ROW_CHANGE] = {" (%+.1f%% in a month)", " (전월 %+.1f%%)", NULL, NULL, NULL, NULL},
    /* without "on the month before": with it the row is wider than the window takes and gets a smaller type (run 140) */
    [RE_MSG_ROW_CHANGE_FAIR] = {" (%+.1f%%, fair %s)", " (%+.1f%%, 적정 %s)", NULL, NULL, NULL, NULL},
    [RE_MSG_HOVER_SUB_OFF] = {"Shift-click: subscribe to research", "Shift-클릭: 조사 구독 켜기", NULL, NULL, NULL, NULL},
    [RE_MSG_HOVER_SUB_ON] = {"Research subscribed (Shift-click: off)", "조사 구독 중 (Shift-클릭: 끄기)", NULL, NULL, NULL, NULL},
    [RE_MSG_HOVER_SUB_ALL] = {"Research subscribed (every company)", "조사 구독 중 (모든 회사)", NULL, NULL, NULL, NULL},
    [RE_MSG_HOVER_SUB_BOARD] = {"On the board: research is free", "이사회 회사: 조사 무료", NULL, NULL, NULL, NULL},
    [RE_MSG_HOVER_ALL_ON] = {"Ctrl+Shift-click: subscribe to all", "Ctrl+Shift-클릭: 모든 회사 구독", NULL, NULL, NULL, NULL},
    [RE_MSG_HOVER_ALL_OFF] = {"Ctrl+Shift-click: all off", "Ctrl+Shift-클릭: 구독 전부 끄기", NULL, NULL, NULL, NULL},
    [RE_MSG_ROW_SUB_ON] = {" (subscribed)", " (구독 중)", NULL, NULL, NULL, NULL},
    [RE_MSG_ROW_SUB_BOARD] = {" (board: free)", " (이사회: 무료)", NULL, NULL, NULL, NULL},
    [RE_MSG_ITEM_FAIR] = {"fair %+.0f%%", "적정 %+.0f%%", NULL, NULL, NULL, NULL},
    [RE_MSG_ITEM_SUB] = {"subscribed", "구독", NULL, NULL, NULL, NULL},
    [RE_MSG_ITEM_BOARD] = {"board", "이사회", NULL, NULL, NULL, NULL},
    [RE_MSG_PROPERTY_PREFIX] = {"Property under its value: ", "저평가 매물: ", NULL, NULL, NULL, NULL},
    [RE_MSG_PROPERTY_LINE] = {"%s%s, a gain of %s after the purchase fees, price %s", "%s%s, 수수료를 내고도 %s 이득, 매수가 %s", NULL,
                              NULL, NULL, NULL},
    [RE_MSG_PROPERTY_NONE] = {"%snone at the moment", "%s지금은 없습니다", NULL, NULL, NULL, NULL},
    [RE_MSG_FIT_PREFIX] = {"Work efficiency down: ", "업무 효율 저하: ", NULL, NULL, NULL, NULL},
    [RE_MSG_FIT_BOTH] = {"%s (furnishings %.0f%%, assets %.0f%%)", "%s (가구 %.0f%%, 자산 %.0f%%)", NULL, NULL, NULL, NULL},
    [RE_MSG_FIT_FURNISH] = {"%s (furnishings %.0f%%)", "%s (가구 %.0f%%)", NULL, NULL, NULL, NULL},
    [RE_MSG_FIT_ASSETS] = {"%s (assets %.0f%%)", "%s (자산 %.0f%%)", NULL, NULL, NULL, NULL},
    [RE_MSG_FIT_NONE] = {"%s (a job stands still: one of its assets is missing altogether)",
                         "%s (자산 하나가 아예 없어 일이 멈춘 직무가 있음)", NULL, NULL, NULL, NULL},
    [RE_MSG_XP_TIP] = {"\nMod: an education a member of your household completed (a degree, a diploma, a certificate) does not go "
                       "below what was gained of it, up to the highest amount that any job, education or activity asks for. Work "
                       "experience decays every month as before.",
                       "\n모드: 가계 구성원이 마친 교육(학위, 졸업장, 인증서)은 얻은 만큼에서 더 줄지 않습니다. 그 교육을 요구하는 "
                       "직업·교육·활동 가운데 가장 높은 요구치까지만 지킵니다. 일해서 얻은 경험은 전처럼 달마다 줄어듭니다.",
                       NULL, NULL, NULL, NULL},
    /* the game's own word for the wage of an employee in its hover text of the automatic management: "급여" */
    [RE_MSG_AUTO_TIP_ALL] = {"\nMod: this switch is for the whole business. A click on anybody's switches everybody.",
                             "\n모드: 이 스위치는 사업체 전체에 걸립니다. 누구 것을 눌러도 직원 모두가 같이 켜지고 꺼집니다.", NULL,
                             NULL, NULL, NULL},
    [RE_MSG_AUTO_TIP_SPARE] = {"\nMod: an employee whose work the others could have done for %d months running is let go without "
                               "severance. Somebody alone in a job who is idle half the time for that long is replaced by a "
                               "candidate of fewer hours, if there is one.",
                               "\n모드: %d달 연속으로 다른 직원들이 남는 시간에 할 수 있었던 일만 한 직원은 퇴직금 없이 "
                               "내보냅니다. 직무에 혼자인데 그동안 시간의 절반 이상을 놀았으면, 근무 시간이 더 적은 지원자가 "
                               "있을 때 그 사람으로 바꿉니다.",
                               NULL, NULL, NULL, NULL},
    [RE_MSG_AUTO_TIP_FILL] = {"\nMod: a job that still has more work this month than its people have hours for gets a candidate "
                              "hired: the one who gets the missing work done for the least money.",
                              "\n모드: 이번 달 일이 직원들의 남은 시간보다 많은 직무에는 지원자를 한 명 뽑습니다. 모자란 일을 가장 "
                              "적은 돈으로 해 줄 사람을 고릅니다.",
                              NULL, NULL, NULL, NULL},
    /* "that month": the month in which the demand is open. The game makes the demand while it turns the month and
     * lets the person go when it turns the next one; the mod answers just before that (REQUEST.md [40], [41]). */
    [RE_MSG_AUTO_TIP] = {"\nMod: a pay rise is answered at the end of that month, just before the employee would leave. If a "
                         "job candidate of that moment would do the same work for less, that one is hired instead; if not, "
                         "the rise is given.",
                         "\n모드: 급여 인상 요구에는 그 달 말, 직원이 나가기 직전에 답합니다. 그때의 지원자 가운데 같은 일을 "
                         "더 싸게 할 사람이 있으면 그 사람으로 바꾸고, 없으면 올려 줍니다.",
                         NULL, NULL, NULL, NULL},
    [RE_MSG_ADVERTS_TIP] = {"\nMod: Shift-click to hand this business's adverts to the mod. It switches every paid advert on while "
                            "awareness is below %.0f%% and off once it is there. Fee at the month end: %s for an advert that was on "
                            "all month.",
                            "\n모드: Shift-클릭하면 이 사업체의 광고를 모드가 맡습니다. 인지도가 %.0f%% 아래면 유료 광고를 모두 켜고 "
                            "그 이상이면 끕니다. 관리비는 월말에 받습니다: 한 달 내내 켠 광고 하나에 %s.",
                            NULL, NULL, NULL, NULL},
    [RE_MSG_ADVERTS_TIP_ON] = {"\nMod: the mod looks after this business's adverts. It switches every paid advert on while awareness "
                               "is below %.0f%% and off once it is there. Fee at the month end: %s for an advert that was on all "
                               "month. While the icons are orange a plain click switches nothing. Shift-click to take them back.",
                               "\n모드: 이 사업체의 광고는 모드가 맡고 있습니다. 인지도가 %.0f%% 아래면 유료 광고를 모두 켜고 그 "
                               "이상이면 끕니다. 관리비는 월말에 받습니다: 한 달 내내 켠 광고 하나에 %s. 아이콘이 주황색인 동안에는 그냥 "
                               "클릭으로 켜고 끌 수 없습니다. 그만두려면 Shift-클릭.",
                               NULL, NULL, NULL, NULL},
    /* the ticker shows about 30 Korean letters and cuts the rest */
    [RE_MSG_ADVERTS_KEPT] = {"%s: the mod looks after the adverts now", "%s: 광고를 모드가 맡습니다", NULL, NULL, NULL, NULL},
    [RE_MSG_ADVERTS_BACK] = {"%s: the adverts are yours again", "%s: 광고를 직접 관리합니다", NULL, NULL, NULL, NULL},
    [RE_MSG_ADVERTS_LOCKED] = {"%s: the mod switches this advert (Shift-click to take the adverts back)",
                               "%s: 이 광고는 모드가 켜고 끕니다 (되찾으려면 Shift-클릭)", NULL, NULL, NULL, NULL},
    [RE_MSG_CONTRACTS_KEPT] = {"%s: the mod signs its contracts now", "%s: 계약 체결을 모드가 맡습니다", NULL, NULL, NULL, NULL},
    [RE_MSG_CONTRACTS_BACK] = {"%s: signing contracts is yours again", "%s: 계약 체결을 직접 합니다", NULL, NULL, NULL, NULL},
    [RE_MSG_ALL_KEPT] = {"%s: the mod runs all of it now", "%s: 전부 모드가 맡습니다", NULL, NULL, NULL, NULL},
    [RE_MSG_ALL_BACK] = {"%s: all of it is yours again", "%s: 전부 직접 관리합니다", NULL, NULL, NULL, NULL},
    [RE_MSG_ADVERTS_TIP_ALL] = {"\nMod: Ctrl+Shift-click hands the whole business to the mod (staff, assets, adverts, contracts, floor "
                                "space) and locks its controls; again takes it back. Contracts alone: Shift-click the Contracts icon.",
                                "\n모드: Ctrl+Shift-클릭하면 이 사업체를 전부 모드에 맡기고(직원, 자산, 광고, 계약, 면적) 직접 조작을 "
                                "잠급니다. 다시 하면 되찾습니다. 계약만 맡기려면 계약 아이콘을 Shift-클릭.",
                                NULL, NULL, NULL, NULL},
    [RE_MSG_CONTRACT_PREFIX] = {"Contracts signed by the mod: ", "모드가 맺은 계약: ", NULL, NULL, NULL, NULL},
    [RE_MSG_CONTRACT_ITEM] = {"%s%s %d hours a month for %d months, %s", "%s%s 달마다 %d시간, %d개월, %s", NULL, NULL, NULL, NULL},
    [RE_MSG_CONTRACT_FEE] = {" (fee %s)", " (수수료 %s)", NULL, NULL, NULL, NULL},
    [RE_MSG_CONTRACT_TICK] = {"The mod signed %d contract(s)", "모드가 계약 %d건을 맺었습니다", NULL, NULL, NULL, NULL},
    [RE_MSG_CONTRACT_ROOM] = {"%s: no floor space for this month's offers", "%s: 면적이 모자라 계약을 맺지 않았습니다", NULL, NULL, NULL,
                              NULL},
    [RE_MSG_PREMISES_PREFIX] = {"Floor space (%s): ", "면적 (%s): ", NULL, NULL, NULL, NULL},
    [RE_MSG_PREMISES_GROW] = {"%sit doubles at the end of this month, and the rent rises by %s a month. The business is short of "
                              "floor space, so the mod switched the star on",
                              "%s이번 달 말에 2배로 늘리고 월세가 %s 오릅니다. 면적이 모자라서 모드가 별표를 켰습니다", NULL, NULL, NULL,
                              NULL},
    [RE_MSG_PREMISES_MOST] = {"%sshort of it, and these premises cannot be made larger. The business has to move",
                              "%s모자라는데 이 부동산은 더 키울 수 없습니다. 더 넓은 곳으로 이사해야 합니다", NULL, NULL, NULL, NULL},
    [RE_MSG_PREMISES_OWNED] = {"%sshort of it. The premises are owned, so the mod did not make them larger: that costs %s at once. "
                               "Switch the star on yourself if you want it",
                               "%s모자랍니다. 소유한 부동산이라 모드가 키우지 않았습니다. 키우려면 %s 가 한 번에 듭니다. 원하면 "
                               "별표를 직접 켜십시오",
                               NULL, NULL, NULL, NULL},
    [RE_MSG_FIRM_CLOSED] = {"%s: closed. The mod's automation rests until you open it again (no hiring, letting go, buying or "
                            "signing; adverts off)",
                            "%s: 닫았습니다. 다시 열 때까지 모드의 자동화는 쉽니다(채용, 내보내기, 자산 구매, 계약 체결을 하지 않고 "
                            "광고는 꺼 둡니다)",
                            NULL, NULL, NULL, NULL},
    [RE_MSG_FIRM_OPENED] = {"%s: open again. The mod's automation goes on", "%s: 다시 열었습니다. 모드의 자동화가 이어집니다", NULL, NULL,
                            NULL, NULL},
    [RE_MSG_LOCKED] = {"%s: the mod runs this business, so this is locked. Take it back first: Ctrl+Shift-click an advert icon",
                       "%s: 전부 모드에 맡긴 사업체라 잠겨 있습니다. 직접 하려면 먼저 광고 아이콘을 Ctrl+Shift-클릭해 되찾으세요", NULL,
                       NULL, NULL, NULL},
    [RE_MSG_ASSET_BUY_PREFIX] = {"Missing assets bought: ", "모자란 자산 구매: ", NULL, NULL, NULL, NULL},
    [RE_MSG_ASSET_BUY_ITEM] = {"%s%s %s x%d", "%s%s %s %d개", NULL, NULL, NULL, NULL},
    [RE_MSG_ASSET_BUY_TOTAL] = {" (%s in all)", " (모두 %s)", NULL, NULL, NULL, NULL},
    [RE_MSG_ASSET_SHORT_PREFIX] = {"Assets not bought: ", "사지 못한 자산: ", NULL, NULL, NULL, NULL},
    [RE_MSG_ASSET_SHORT_ROOM] = {"%s%s %s (no floor space)", "%s%s %s (면적 부족)", NULL, NULL, NULL, NULL},
    [RE_MSG_ASSET_SHORT_CASH] = {"%s%s %s (not enough cash)", "%s%s %s (현금 부족)", NULL, NULL, NULL, NULL},
    [RE_MSG_ASSET_SHORT_NONE] = {"%s%s %s (nothing for sale)", "%s%s %s (파는 물건 없음)", NULL, NULL, NULL, NULL},
    [RE_MSG_ASSET_BUY_DEBT] = {" (%s in all, %s of it as a debt)", " (모두 %s, 그 가운데 부채 %s)", NULL, NULL, NULL, NULL},
    [RE_MSG_ASSET_TICK_BUY] = {"Missing assets bought: %d for %s", "모자란 자산 %d개 구매: %s", NULL, NULL, NULL, NULL},
    [RE_MSG_ASSET_TICK_DEBT] = {" (%s of it is a debt, due in three months)", " (부채 %s, 석 달 안에 갚아야 합니다)", NULL, NULL, NULL,
                                NULL},
    [RE_MSG_ASSET_REPLACED] ={"%s: a new one bought, %s", "%s: 새것으로 구매, %s", NULL, NULL, NULL, NULL},
    /* short lines: the game cuts a hover text of these buttons off at the window's left edge, the third button's
     * after sixteen Latin letters (run 144) */
    [RE_MSG_SORT_CHEAP] = {"Sort by\nundervalued\n(researched)", "저평가순으로 정렬\n(조사한 회사만)", NULL, NULL, NULL, NULL},
    [RE_MSG_SORT_DEAR] = {"Sort by\novervalued\n(researched)", "고평가순으로 정렬\n(조사한 회사만)", NULL, NULL, NULL, NULL},
    [RE_MSG_SORT_CHANGE] = {"Sort by change\non the month before", "전월 대비\n오른 순으로 정렬", NULL, NULL, NULL, NULL},
    [RE_MSG_SORT_CAP] = {"Sort by\nmarket cap\n(largest first)", "시가총액\n큰 순으로 정렬", NULL, NULL, NULL, NULL},
};

/* What the table does not have: one file a language, `[RE_MSG_...] = "text"` a line. */
#include "re_lang_de.h"
#include "re_lang_es.h"
#include "re_lang_fr.h"
#include "re_lang_jp.h"
#include "re_lang_pt_br.h"
#include "re_lang_zh_cn.h"

static const char *const *const MORE[RE_LANG_COUNT] = {
    [RE_LANG_DE] = LANG_DE, [RE_LANG_ES] = LANG_ES, [RE_LANG_FR] = LANG_FR,
    [RE_LANG_PT_BR] = LANG_PT_BR, [RE_LANG_JP] = LANG_JP, [RE_LANG_ZH_CN] = LANG_ZH_CN,
};

int re_lang_of(const char *folder)
{
    for (int i = 0; i < RE_LANG_COUNT; i++)
        if (strcmp(folder, FOLDERS[i]) == 0)
            return i;
    return RE_LANG_EN;
}

const char *re_lang_folder(int lang)
{
    return lang >= 0 && lang < RE_LANG_COUNT ? FOLDERS[lang] : FOLDERS[RE_LANG_EN];
}

int re_lang_places(int lang, int msg)
{
    if (lang < 0 || lang >= RE_LANG_COUNT || msg < 0 || msg >= RE_MSG_COUNT)
        return 0;
    return (TEXTS[msg][lang] != NULL) + (MORE[lang] != NULL && MORE[lang][msg] != NULL);
}

int re_lang_has(int lang, int msg)
{
    return re_lang_places(lang, msg) > 0;
}

const char *re_lang_text(int lang, int msg)
{
    if (msg < 0 || msg >= RE_MSG_COUNT)
        return "";
    if (!re_lang_has(lang, msg))
        return TEXTS[msg][RE_LANG_EN];
    return TEXTS[msg][lang] != NULL ? TEXTS[msg][lang] : MORE[lang][msg];
}
