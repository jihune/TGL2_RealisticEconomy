/* German: the texts the table of re_lang.c has no German for. Included by re_lang.c only.
 * The words are the game's own where it has one (modsLanguages/de): Insolvenz, Bekanntheit, Vermögenswerte,
 * Grundfläche, Einrichtung, Abfindung, Direktorium, Geschäft; the game says "du". */
static const char *const LANG_DE[RE_MSG_COUNT] = {
    [RE_MSG_HOVER_YIELD] = ", Rendite %.1f%%",
    [RE_MSG_HOVER_UNDER] = "PBR < 0.75: ",
    [RE_MSG_HOVER_BANKRUPT] = "%s%sInsolvenz am Monatsende%s",
    [RE_MSG_HOVER_MEASURE] = "%s%sVerkleinerung oder neue Aktien am Monatsende%s",
    [RE_MSG_HOVER_WATCH] = "%sIn %d Mon.: Insolvenz, wenn der Kurs um %.1f%% fällt%s",
    [RE_MSG_HOVER_NEAR] = "Kurs -%.1f%%: Verkleinerung/neue Aktien",
    [RE_MSG_HOVER_FAIR] = "Fair %s (%+.0f%%)",
    [RE_MSG_HOVER_TAILWIND] = ", Branche im Aufwind",
    [RE_MSG_HOVER_HEADWIND] = ", Branche im Abschwung",
    [RE_MSG_ROW_MEASURE] = "Verkleinerung oder Verwässerung",
    [RE_MSG_ROW_PER_MONTH] = " (%s pro Monat)",
    [RE_MSG_HELD_PREFIX] = "Warnung zu gehaltenen Aktien: ",
    [RE_MSG_HELD_CLOSE] = "Kurs -%.1f%%: Verkleinerung oder neue Aktien am Monatsende",
    [RE_MSG_HELD_WATCH] = "Kurs -%.1f%%: Insolvenz am Monatsende",
    [RE_MSG_HELD_MORE] = " und %d weitere",
    [RE_MSG_CASINO_SLOTS] = "Spielautomaten",
    [RE_MSG_CASINO_ROULETTE] = "Roulette",
    [RE_MSG_CASINO_BLACKJACK] = "Blackjack",
    [RE_MSG_CASINO_BACCARAT] = "Baccarat",
    [RE_MSG_CASINO_WON] = "%s: %s-facher Einsatz gewonnen (+%s)",
    [RE_MSG_CASINO_LOST] = "%s: nichts gewonnen (-%s)",
    [RE_MSG_CASINO_PLAYED] = "%s: diesen Monat schon gespielt, vor dem Laden. Dieses Spiel hat kein Ergebnis",
    [RE_MSG_CASINO_BEFORE] = "%s: vor dem Mod begonnen. Das Spiel hat das Ergebnis beim Start verbucht; mehr wird nicht verbucht",
    [RE_MSG_ADOPT_PREFIX] = "Spielstand von vor dem Mod: ",
    [RE_MSG_ADOPT] = "%s„Abgenutzte Vermögenswerte neu kaufen“ in %d Geschäft(en) ausgeschaltet, Automatikmodus von %d "
                     "Mitarbeiter(n) ausgeschaltet. Beide tun mit dem Mod mehr: schalte sie dort wieder ein, wo du das willst",
    [RE_MSG_ADOPT_ASSETS] = "%s„Abgenutzte Vermögenswerte neu kaufen“ in %d Geschäft(en) ausgeschaltet. Der Schalter tut mit dem "
                            "Mod mehr: schalte ihn dort wieder ein, wo du das willst",
    [RE_MSG_ADOPT_STAFF] = "%sAutomatikmodus von %d Mitarbeiter(n) ausgeschaltet. Er tut mit dem Mod mehr: schalte ihn dort "
                           "wieder ein, wo du das willst",
    [RE_MSG_ADOPT_CASINO] = "%d Casinospiel(e) aus der Warteschlange genommen. Mit dem Mod hat ein Spiel einen Einsatz und das "
                            "Ergebnis kommt am Ende: stelle es neu ein, wenn du es willst",
    [RE_MSG_ADOPT_CASINO_BEGUN] = "%d Casinospiel(e) aus der Warteschlange genommen. %d davon hatten begonnen: ihr Ergebnis ist das, "
                                  "was das Spiel beim Start gab, und sie zählen als diesen Monat gespielt. Mit dem Mod hat ein "
                                  "Spiel einen Einsatz und das Ergebnis kommt am Ende: stelle es neu ein, wenn du es willst",
    [RE_MSG_CASINO_BACK_PREFIX] = "Casino: ",
    [RE_MSG_CASINO_BACK] = "%s%d vor diesem Laden gespielte(s) Spiel(e) zählen weiter (Bargeld %s)",
    [RE_MSG_CASINO_TIP] = "Monatl. Gewinn: ",
    [RE_MSG_CASINO_TIP_PRIZE] = "%s%s%%: %sx",
    [RE_MSG_STAFF_LINE] = "%s pro effektive Stunde",
    [RE_MSG_HIRE_ORDER] = " (pro effektive Stunde am günstigsten zuerst)",
    [RE_MSG_STAFF_TIP] = "%s pro effektive Stunde (Lohn pro Stunde / Arbeitseffizienz %.0f%%)",
    [RE_MSG_OFFER_TIP] = "%+.1f%% über Standardkosten\n%s bleiben pro Stunde",
    [RE_MSG_FUT_EXPECTED] = " (erw. %.2f%%)",
    [RE_MSG_FUT_TIP] = "Erwartet: der Durchschnitt der Rate bis zum gewählten Ablauf, wenn sie sich nach der Regel des Spiels "
                       "bewegt. Liegt er über der heutigen Rate, ist ein Kauf im Vorteil, darunter ein Verkauf.",
    [RE_MSG_FUT_ITEM] = " (%+.1f%%/J.)",
    [RE_MSG_FUT_ITEM_AHEAD] = " (%+.1f%% → %+.1f%%)",
    [RE_MSG_FUT_ROW_TIP] = "Inflation %+.2f%% pro Jahr\nErwartet, 6 Monate: %+.2f%%\nErwartet, 12 Monate: %+.2f%%\nErwartet, 24 "
                           "Monate: %+.2f%%\nHöher erwartet: Kauf. Tiefer: Verkauf.",
    [RE_MSG_FUT_SORT] = "Sortieren: erwartete Rate\n(12 Monate) minus aktuelle",
    [RE_MSG_CHART_HALF_YEAR] = "6 Monate",
    [RE_MSG_CHART_INDEX] = " (erster gezeigter Monat = 100)",
    [RE_MSG_RESEARCH_PREFIX] = "Abo der Aktienrecherche: ",
    [RE_MSG_RESEARCH_PAID] = "%s%d Unternehmen neu recherchiert, Gebühr %s",
    [RE_MSG_RESEARCH_SHORT] = "%s%d Unternehmen neu recherchiert, Gebühr %s; dein Bargeld reichte nicht, %s davon sind jetzt Schulden",
    [RE_MSG_RESEARCH_KEPT_PREFIX] = "Schon bezahlte Recherchegebühr: ",
    [RE_MSG_RESEARCH_KEPT] = "%sdas Laden eines früheren Spielstands bringt sie nicht zurück. %d in diesem Monat bezahlte "
                             "Unternehmen wurden neu recherchiert, Gebühr %s",
    [RE_MSG_RESEARCH_KEPT_SHORT] = "%sdas Laden eines früheren Spielstands bringt sie nicht zurück. %d in diesem Monat bezahlte "
                                   "Unternehmen wurden neu recherchiert, Gebühr %s; dein Bargeld reichte nicht, %s davon sind "
                                   "jetzt Schulden",
    [RE_MSG_RESEARCH_KEPT_TICK] = "Vor diesem Laden bezahlte Recherchegebühr erneut abgebucht: %s (%d Unternehmen)",
    [RE_MSG_RESEARCH_ONE_ON] = "Recherche-Abo: %s an. Jetzt recherchiert, Gebühr %s (jeden Monat)",
    [RE_MSG_RESEARCH_ONE_OFF] = "Recherche-Abo: %s aus. Ab nächstem Monat keine Gebühr",
    [RE_MSG_RESEARCH_ALL_ON] = "Recherche-Abo: alle Unternehmen an. %d jetzt recherchiert, Gebühr %s",
    [RE_MSG_RESEARCH_ALL_OFF] = "Recherche-Abo: alle aus. Ab nächstem Monat keine Gebühr",
    [RE_MSG_RESEARCH_ONE_ON_DONE] = "Recherche-Abo: %s an. Die Recherche dieses Monats liegt vor, die Gebühr beginnt nächsten Monat",
    [RE_MSG_RESEARCH_ALL_HAS] = "%s ist im Abo aller Unternehmen (Strg+Umschalt-Klick: alle aus)",
    [RE_MSG_RESEARCH_NO_CASH] = "Recherche-Abo: nicht eingeschaltet, dein Bargeld deckt die Gebühr von %s nicht",
    [RE_MSG_RESEARCH_BOARD] = "%s: du sitzt im Direktorium, die Recherche ist kostenlos",
    [RE_MSG_HOVER_CHANGE] = "%+.1f%% zum Vormonat",
    /* short: with " in einem Monat" the row was wider than the window takes and got a smaller type (run 519) */
    [RE_MSG_ROW_CHANGE] = " (%+.1f%%/Monat)",
    [RE_MSG_ROW_CHANGE_FAIR] = " (%+.1f%%, fair %s)",
    [RE_MSG_HOVER_SUB_OFF] = "Umschalt-Klick: Recherche abonnieren",
    [RE_MSG_HOVER_SUB_ON] = "Recherche im Abo (Umschalt-Klick: aus)",
    [RE_MSG_HOVER_SUB_ALL] = "Recherche im Abo (alle Unternehmen)",
    [RE_MSG_HOVER_SUB_BOARD] = "Im Direktorium: Recherche kostenlos",
    [RE_MSG_HOVER_ALL_ON] = "Strg+Umschalt-Klick: alle abonnieren",
    [RE_MSG_HOVER_ALL_OFF] = "Strg+Umschalt-Klick: alle aus",
    [RE_MSG_ROW_SUB_ON] = " (im Abo)",
    [RE_MSG_ROW_SUB_BOARD] = " (Direktorium: kostenlos)",
    [RE_MSG_ITEM_FAIR] = "fair %+.0f%%",
    [RE_MSG_ITEM_SUB] = "Abo",
    [RE_MSG_ITEM_BOARD] = "Direktorium",
    [RE_MSG_PROPERTY_PREFIX] = "Immobilie unter Wert: ",
    [RE_MSG_PROPERTY_LINE] = "%s%s, Gewinn von %s nach den Kaufkosten, Preis %s",
    [RE_MSG_PROPERTY_NONE] = "%sim Moment keine",
    [RE_MSG_FIT_PREFIX] = "Arbeitseffizienz gesunken: ",
    [RE_MSG_FIT_BOTH] = "%s (Einrichtung %.0f%%, Vermögenswerte %.0f%%)",
    [RE_MSG_FIT_FURNISH] = "%s (Einrichtung %.0f%%)",
    [RE_MSG_FIT_ASSETS] = "%s (Vermögenswerte %.0f%%)",
    [RE_MSG_FIT_NONE] = "%s (eine Arbeit steht still: einer ihrer Vermögenswerte fehlt ganz)",
    [RE_MSG_XP_TIP] = "\nMod: ein Studium, das ein Mitglied deines Haushalts abgeschlossen hat (Abschluss, Diplom, Zertifikat), "
                      "fällt nicht unter das, was davon erworben wurde, höchstens bis zum höchsten Wert, den eine Arbeit, ein "
                      "Studium oder eine Aktivität verlangt. Arbeitserfahrung nimmt wie bisher jeden Monat ab.",
    /* The four paragraphs under the game's hover text of the automatic management: kept short. With the first
     * wording the box was taller than a screen of 900 lines and lost the game's first lines at the top (run 524). */
    [RE_MSG_AUTO_TIP_ALL] = "\nMod: der Schalter gilt für alle Mitarbeiter des Geschäfts.",
    [RE_MSG_AUTO_TIP_SPARE] = "\nMod: wessen Arbeit die anderen %d Monate lang hätten miterledigen können, wird ohne Abfindung "
                              "entlassen. Wer allein arbeitet und so lange zur Hälfte untätig ist, wird durch einen Kandidaten "
                              "mit weniger Stunden ersetzt.",
    [RE_MSG_AUTO_TIP_FILL] = "\nMod: hat eine Arbeit diesen Monat mehr zu tun, als ihre Leute Stunden haben, wird der Kandidat "
                             "eingestellt, der das Fehlende am günstigsten erledigt.",
    [RE_MSG_AUTO_TIP] = "\nMod: eine Lohnforderung wird am Monatsende beantwortet, kurz bevor der Mitarbeiter ginge. Macht ein "
                        "Kandidat dieselbe Arbeit billiger, wird er eingestellt; sonst gibt es die Erhöhung.",
    [RE_MSG_ADVERTS_TIP] = "\nMod: Umschalt-Klick übergibt die Werbung dieses Geschäfts an den Mod. Er schaltet jede bezahlte "
                           "Werbung ein, solange die Bekanntheit unter %.0f%% liegt, und aus, sobald sie erreicht ist. Gebühr am "
                           "Monatsende: %s für eine Werbung, die den ganzen Monat an war.",
    [RE_MSG_ADVERTS_TIP_ON] = "\nMod: der Mod kümmert sich um die Werbung dieses Geschäfts. Er schaltet jede bezahlte Werbung ein, "
                              "solange die Bekanntheit unter %.0f%% liegt, und aus, sobald sie erreicht ist. Gebühr am Monatsende: "
                              "%s für eine Werbung, die den ganzen Monat an war. Solange die Symbole orange sind, schaltet ein "
                              "einfacher Klick nichts. Umschalt-Klick holt sie zurück.",
    [RE_MSG_ADVERTS_KEPT] = "%s: der Mod kümmert sich jetzt um die Werbung",
    [RE_MSG_ADVERTS_BACK] = "%s: die Werbung gehört wieder dir",
    [RE_MSG_ADVERTS_LOCKED] = "%s: diese Werbung schaltet der Mod (Umschalt-Klick holt die Werbung zurück)",
    [RE_MSG_CONTRACTS_KEPT] = "%s: der Mod schließt jetzt die Verträge ab",
    [RE_MSG_CONTRACTS_BACK] = "%s: Verträge schließt du wieder selbst ab",
    [RE_MSG_ALL_KEPT] = "%s: der Mod führt jetzt alles",
    [RE_MSG_ALL_BACK] = "%s: alles gehört wieder dir",
    [RE_MSG_ADVERTS_TIP_ALL] = "\nMod: Strg+Umschalt-Klick übergibt das ganze Geschäft an den Mod (Mitarbeiter, Vermögenswerte, "
                               "Werbung, Verträge, Grundfläche) und sperrt seine Bedienelemente; noch einmal holt es zurück. Nur "
                               "die Verträge: Umschalt-Klick auf das Symbol „Verträge“.",
    [RE_MSG_CONTRACT_PREFIX] = "Vom Mod abgeschlossene Verträge: ",
    [RE_MSG_CONTRACT_ITEM] = "%s%s %d Stunden pro Monat für %d Monate, %s",
    [RE_MSG_CONTRACT_FEE] = " (Gebühr %s)",
    [RE_MSG_CONTRACT_TICK] = "Der Mod hat %d Vertrag/Verträge abgeschlossen",
    [RE_MSG_CONTRACT_ROOM] = "%s: keine Grundfläche für die Angebote dieses Monats",
    [RE_MSG_PREMISES_PREFIX] = "Grundfläche (%s): ",
    [RE_MSG_PREMISES_GROW] = "%ssie verdoppelt sich am Ende dieses Monats, und die Miete steigt um %s pro Monat. Dem Geschäft "
                             "fehlt Grundfläche, deshalb hat der Mod den Stern eingeschaltet",
    [RE_MSG_PREMISES_MOST] = "%ssie reicht nicht, und diese Räume lassen sich nicht vergrößern. Das Geschäft muss umziehen",
    [RE_MSG_PREMISES_OWNED] = "%ssie reicht nicht. Die Räume sind Eigentum, deshalb hat der Mod sie nicht vergrößert: das kostet %s "
                              "auf einmal. Schalte den Stern selbst ein, wenn du es willst",
    [RE_MSG_FIRM_CLOSED] = "%s: geschlossen. Die Automatik des Mods ruht, bis du wieder öffnest (kein Einstellen, Entlassen, "
                           "Kaufen oder Abschließen; Werbung aus)",
    [RE_MSG_FIRM_OPENED] = "%s: wieder geöffnet. Die Automatik des Mods läuft weiter",
    [RE_MSG_LOCKED] = "%s: der Mod führt dieses Geschäft, deshalb ist das gesperrt. Hol es erst zurück: Strg+Umschalt-Klick auf "
                      "ein Werbesymbol",
    [RE_MSG_ASSET_BUY_PREFIX] = "Fehlende Vermögenswerte gekauft: ",
    [RE_MSG_ASSET_BUY_ITEM] = "%s%s %s x%d",
    [RE_MSG_ASSET_BUY_TOTAL] = " (insgesamt %s)",
    [RE_MSG_ASSET_SHORT_PREFIX] = "Nicht gekaufte Vermögenswerte: ",
    [RE_MSG_ASSET_SHORT_ROOM] = "%s%s %s (keine Grundfläche)",
    [RE_MSG_ASSET_SHORT_CASH] = "%s%s %s (zu wenig Bargeld)",
    [RE_MSG_ASSET_SHORT_NONE] = "%s%s %s (nichts im Angebot)",
    [RE_MSG_ASSET_BUY_DEBT] = " (insgesamt %s, davon %s als Schulden)",
    [RE_MSG_ASSET_TICK_BUY] = "Fehlende Vermögenswerte gekauft: %d für %s",
    [RE_MSG_ASSET_TICK_DEBT] = " (%s davon sind Schulden, fällig in drei Monaten)",
    [RE_MSG_ASSET_REPLACED] = "%s: neu gekauft, %s",
    [RE_MSG_SORT_CHEAP] = "Sortieren:\nunterbewertet\n(recherchiert)",
    [RE_MSG_SORT_DEAR] = "Sortieren:\nüberbewertet\n(recherchiert)",
    [RE_MSG_SORT_CHANGE] = "Nach Änderung\nzum Vormonat",
    [RE_MSG_SORT_CAP] = "Sortieren:\nMarktkap.\n(größte zuerst)",
};
