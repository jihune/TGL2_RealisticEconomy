/* French: the texts the table of re_lang.c has no French for. Included by re_lang.c only.
 * The words are the game's own where it has one (modsLanguages/fr): faillite, sensibilisation (awareness), actifs,
 * surface, ameublement, indemnité de départ, conseil d'administration, entreprise, salariés; the game says "vous". */
static const char *const LANG_FR[RE_MSG_COUNT] = {
    [RE_MSG_HOVER_YIELD] = ", rendement %.1f%%",
    [RE_MSG_HOVER_UNDER] = "PBR < 0.75 : ",
    [RE_MSG_HOVER_BANKRUPT] = "%s%sfaillite en fin de mois%s",
    [RE_MSG_HOVER_MEASURE] = "%s%sréduction ou nouvelles actions en fin de mois%s",
    [RE_MSG_HOVER_WATCH] = "%sSous %d mois : faillite si le cours baisse de %.1f%%%s",
    [RE_MSG_HOVER_NEAR] = "Cours -%.1f%% : réduction ou nouv. actions",
    [RE_MSG_HOVER_FAIR] = "Juste %s (%+.0f%%)",
    [RE_MSG_HOVER_TAILWIND] = ", industrie en essor",
    [RE_MSG_HOVER_HEADWIND] = ", industrie en déclin",
    [RE_MSG_ROW_MEASURE] = "réduction ou dilution",
    [RE_MSG_ROW_PER_MONTH] = " (%s par mois)",
    [RE_MSG_HELD_PREFIX] = "Alerte sur vos actions : ",
    [RE_MSG_HELD_CLOSE] = "cours -%.1f%% : réduction ou nouvelles actions en fin de mois",
    [RE_MSG_HELD_WATCH] = "cours -%.1f%% : faillite en fin de mois",
    [RE_MSG_HELD_MORE] = " et %d de plus",
    [RE_MSG_CASINO_SLOTS] = "Machines à sous",
    [RE_MSG_CASINO_ROULETTE] = "Roulette",
    [RE_MSG_CASINO_BLACKJACK] = "Blackjack",
    [RE_MSG_CASINO_BACCARAT] = "Baccara",
    [RE_MSG_CASINO_WON] = "%s : gain de %s fois la mise (+%s)",
    [RE_MSG_CASINO_LOST] = "%s : rien gagné (-%s)",
    [RE_MSG_CASINO_PLAYED] = "%s : déjà joué ce mois-ci, avant le chargement. Cette partie n'a pas de résultat",
    [RE_MSG_CASINO_BEFORE] = "%s : commencée avant le mod. Le jeu a compté son résultat au début ; rien d'autre n'est compté",
    [RE_MSG_ADOPT_PREFIX] = "Sauvegarde d'avant le mod : ",
    [RE_MSG_ADOPT] = "%s« racheter les actifs usés » désactivé dans %d entreprise(s), gestion automatique de %d salarié(s) "
                     "désactivée. Avec le mod, les deux font davantage : réactivez-les là où vous le voulez",
    [RE_MSG_ADOPT_ASSETS] = "%s« racheter les actifs usés » désactivé dans %d entreprise(s). Avec le mod, il fait davantage : "
                            "réactivez-le là où vous le voulez",
    [RE_MSG_ADOPT_STAFF] = "%sgestion automatique de %d salarié(s) désactivée. Avec le mod, elle fait davantage : réactivez-la "
                           "là où vous le voulez",
    [RE_MSG_ADOPT_CASINO] = "%d partie(s) de casino retirée(s) de la file d'attente. Avec le mod, une partie a une mise et son "
                            "résultat arrive à la fin : remettez-la dans la file si vous la voulez",
    [RE_MSG_ADOPT_CASINO_BEGUN] = "%d partie(s) de casino retirée(s) de la file d'attente. %d avaient commencé : leur résultat est "
                                  "celui que le jeu a donné au début, et elles comptent comme jouées ce mois-ci. Avec le mod, une "
                                  "partie a une mise et son résultat arrive à la fin : remettez-la dans la file si vous la voulez",
    [RE_MSG_CASINO_BACK_PREFIX] = "Casino : ",
    [RE_MSG_CASINO_BACK] = "%s%d partie(s) jouée(s) avant ce chargement comptent toujours (liquidités %s)",
    [RE_MSG_CASINO_TIP] = "Mensuel. Gain : ",
    [RE_MSG_CASINO_TIP_PRIZE] = "%s%s%% : %sx",
    [RE_MSG_STAFF_LINE] = "%s par heure effective",
    [RE_MSG_HIRE_ORDER] = " (le moins cher par heure effective d'abord)",
    [RE_MSG_STAFF_TIP] = "%s par heure effective (rémunération à l'heure / efficacité du travail %.0f%%)",
    [RE_MSG_OFFER_TIP] = "%+.1f%% au-dessus du coût standard\n%s restent par heure",
    [RE_MSG_FUT_EXPECTED] = " (prév. %.2f%%)",
    [RE_MSG_FUT_TIP] = "Prévu : la moyenne du taux jusqu'à l'expiration choisie s'il évolue selon la règle du jeu. Au-dessus du "
                       "taux d'aujourd'hui, un achat est gagnant ; en dessous, une vente.",
    [RE_MSG_FUT_ITEM] = " (%+.1f%%/an)",
    [RE_MSG_FUT_ITEM_AHEAD] = " (%+.1f%% → %+.1f%%)",
    [RE_MSG_FUT_ROW_TIP] = "Inflation %+.2f%% par an\nPrévue, 6 mois : %+.2f%%\nPrévue, 12 mois : %+.2f%%\nPrévue, 24 mois : "
                           "%+.2f%%\nPlus haute : achat. Plus basse : vente",
    [RE_MSG_FUT_SORT] = "Trier par taux prévu\n(12 mois) moins l'actuel",
    [RE_MSG_CHART_HALF_YEAR] = "6 mois",
    [RE_MSG_CHART_INDEX] = " (premier mois affiché = 100)",
    [RE_MSG_RESEARCH_PREFIX] = "Abonnement de recherche : ",
    [RE_MSG_RESEARCH_PAID] = "%s%d entreprises recherchées à nouveau, frais %s",
    [RE_MSG_RESEARCH_SHORT] = "%s%d entreprises recherchées à nouveau, frais %s ; vos liquidités manquaient, donc %s sont "
                              "maintenant une dette",
    [RE_MSG_RESEARCH_KEPT_PREFIX] = "Frais de recherche déjà payés : ",
    [RE_MSG_RESEARCH_KEPT] = "%scharger une sauvegarde antérieure ne les rend pas. %d entreprises payées ce mois-ci ont été "
                             "recherchées à nouveau, frais %s",
    [RE_MSG_RESEARCH_KEPT_SHORT] = "%scharger une sauvegarde antérieure ne les rend pas. %d entreprises payées ce mois-ci ont "
                                   "été recherchées à nouveau, frais %s ; vos liquidités manquaient, donc %s sont maintenant une "
                                   "dette",
    [RE_MSG_RESEARCH_KEPT_TICK] = "Frais de recherche payés avant ce chargement, prélevés à nouveau : %s (%d entreprises)",
    [RE_MSG_RESEARCH_ONE_ON] = "Abonnement : %s activé. Recherche faite, frais %s (chaque mois)",
    [RE_MSG_RESEARCH_ONE_OFF] = "Abonnement : %s désactivé. Pas de frais dès le mois prochain",
    [RE_MSG_RESEARCH_ALL_ON] = "Abonnement : toutes les entreprises activées. %d recherchées maintenant, frais %s",
    [RE_MSG_RESEARCH_ALL_OFF] = "Abonnement : tout désactivé. Pas de frais dès le mois prochain",
    [RE_MSG_RESEARCH_ONE_ON_DONE] = "Abonnement : %s activé. La recherche de ce mois existe déjà, les frais commencent le mois "
                                    "prochain",
    [RE_MSG_RESEARCH_ALL_HAS] = "%s fait partie de l'abonnement à toutes les entreprises (Ctrl+Maj+clic : tout désactiver)",
    [RE_MSG_RESEARCH_NO_CASH] = "Abonnement : non activé, vos liquidités ne couvrent pas les frais de %s",
    [RE_MSG_RESEARCH_BOARD] = "%s : vous siégez à son conseil, sa recherche est gratuite",
    [RE_MSG_HOVER_CHANGE] = "%+.1f%% sur un mois",
    /* short, as in German and Spanish: a wider row gets a smaller type */
    [RE_MSG_ROW_CHANGE] = " (%+.1f%%/mois)",
    [RE_MSG_ROW_CHANGE_FAIR] = " (%+.1f%%, juste %s)",
    [RE_MSG_HOVER_SUB_OFF] = "Maj+clic : s'abonner à la recherche",
    [RE_MSG_HOVER_SUB_ON] = "Recherche abonnée (Maj+clic : arrêter)",
    [RE_MSG_HOVER_SUB_ALL] = "Recherche abonnée (toutes)",
    [RE_MSG_HOVER_SUB_BOARD] = "Au conseil : recherche gratuite",
    [RE_MSG_HOVER_ALL_ON] = "Ctrl+Maj+clic : s'abonner à toutes",
    [RE_MSG_HOVER_ALL_OFF] = "Ctrl+Maj+clic : tout désactiver",
    [RE_MSG_ROW_SUB_ON] = " (abonnée)",
    [RE_MSG_ROW_SUB_BOARD] = " (conseil : gratuit)",
    [RE_MSG_ITEM_FAIR] = "juste %+.0f%%",
    [RE_MSG_ITEM_SUB] = "abonnée",
    [RE_MSG_ITEM_BOARD] = "conseil",
    [RE_MSG_PROPERTY_PREFIX] = "Bien sous sa valeur : ",
    [RE_MSG_PROPERTY_LINE] = "%s%s, gain de %s après les frais d'achat, prix %s",
    [RE_MSG_PROPERTY_NONE] = "%saucun pour le moment",
    [RE_MSG_FIT_PREFIX] = "Efficacité du travail en baisse : ",
    [RE_MSG_FIT_BOTH] = "%s (ameublement %.0f%%, actifs %.0f%%)",
    [RE_MSG_FIT_FURNISH] = "%s (ameublement %.0f%%)",
    [RE_MSG_FIT_ASSETS] = "%s (actifs %.0f%%)",
    [RE_MSG_FIT_NONE] = "%s (un emploi est à l'arrêt : un de ses actifs manque complètement)",
    [RE_MSG_XP_TIP] = "\nMod : une formation qu'un membre de votre ménage a terminée (diplôme, certificat) ne descend pas sous "
                      "ce qui en a été acquis, jusqu'au montant le plus élevé qu'un emploi, une formation ou une activité "
                      "demande. L'expérience de travail diminue chaque mois comme avant.",
    /* The four paragraphs under the game's hover text of the automatic management: kept short. With the first
     * wording the box was taller than a screen of 900 lines and lost the game's first lines at the top (run 525). */
    [RE_MSG_AUTO_TIP_ALL] = "\nMod : cet interrupteur vaut pour tous les salariés de l'entreprise.",
    [RE_MSG_AUTO_TIP_SPARE] = "\nMod : un salarié dont les autres auraient pu faire le travail pendant %d mois est renvoyé sans "
                              "indemnité. Celui qui est seul et inoccupé la moitié du temps est remplacé par un candidat avec "
                              "moins d'heures.",
    [RE_MSG_AUTO_TIP_FILL] = "\nMod : si un emploi a ce mois-ci plus de travail que ses salariés n'ont d'heures, le candidat qui "
                             "fait le travail manquant pour le moins cher est embauché.",
    [RE_MSG_AUTO_TIP] = "\nMod : une demande d'augmentation reçoit sa réponse en fin de mois, juste avant le départ. Si un "
                        "candidat fait le même travail pour moins cher, il est embauché ; sinon l'augmentation est accordée.",
    [RE_MSG_ADVERTS_TIP] = "\nMod : Maj+clic pour confier les publicités de cette entreprise au mod. Il active toutes les "
                           "publicités payantes tant que la sensibilisation est sous %.0f%% et les désactive une fois atteinte. "
                           "Frais en fin de mois : %s pour une publicité active tout le mois.",
    [RE_MSG_ADVERTS_TIP_ON] = "\nMod : le mod s'occupe des publicités de cette entreprise. Il active toutes les publicités "
                              "payantes tant que la sensibilisation est sous %.0f%% et les désactive une fois atteinte. Frais en "
                              "fin de mois : %s pour une publicité active tout le mois. Tant que les icônes sont orange, un clic "
                              "simple ne change rien. Maj+clic pour les reprendre.",
    [RE_MSG_ADVERTS_KEPT] = "%s : le mod s'occupe maintenant des publicités",
    [RE_MSG_ADVERTS_BACK] = "%s : les publicités sont de nouveau à vous",
    [RE_MSG_ADVERTS_LOCKED] = "%s : le mod gère cette publicité (Maj+clic pour reprendre les publicités)",
    [RE_MSG_CONTRACTS_KEPT] = "%s : le mod signe maintenant ses contrats",
    [RE_MSG_CONTRACTS_BACK] = "%s : vous signez de nouveau les contrats",
    [RE_MSG_ALL_KEPT] = "%s : le mod gère tout maintenant",
    [RE_MSG_ALL_BACK] = "%s : tout est de nouveau à vous",
    [RE_MSG_ADVERTS_TIP_ALL] = "\nMod : Ctrl+Maj+clic confie toute l'entreprise au mod (salariés, actifs, publicités, contrats, "
                               "surface) et verrouille ses commandes ; une deuxième fois la reprend. Les contrats seuls : "
                               "Maj+clic sur l'icône Contrats.",
    [RE_MSG_CONTRACT_PREFIX] = "Contrats signés par le mod : ",
    [RE_MSG_CONTRACT_ITEM] = "%s%s %d heures par mois pendant %d mois, %s",
    [RE_MSG_CONTRACT_FEE] = " (frais %s)",
    [RE_MSG_CONTRACT_TICK] = "Le mod a signé %d contrat(s)",
    [RE_MSG_CONTRACT_ROOM] = "%s : pas de surface pour les offres de ce mois",
    [RE_MSG_PREMISES_PREFIX] = "Surface (%s) : ",
    [RE_MSG_PREMISES_GROW] = "%selle double à la fin de ce mois et le loyer augmente de %s par mois. L'entreprise manque de "
                             "surface, le mod a donc activé l'étoile",
    [RE_MSG_PREMISES_MOST] = "%selle manque, et ces locaux ne peuvent pas être agrandis. L'entreprise doit déménager",
    [RE_MSG_PREMISES_OWNED] = "%selle manque. Les locaux vous appartiennent, le mod ne les a donc pas agrandis : cela coûte %s "
                              "d'un coup. Activez l'étoile vous-même si vous le voulez",
    [RE_MSG_FIRM_CLOSED] = "%s : fermée. L'automatisation du mod se repose jusqu'à la réouverture (ni embauche, ni renvoi, ni "
                           "achat, ni signature ; publicités désactivées)",
    [RE_MSG_FIRM_OPENED] = "%s : rouverte. L'automatisation du mod reprend",
    [RE_MSG_LOCKED] = "%s : le mod gère cette entreprise, c'est donc verrouillé. Reprenez-la d'abord : Ctrl+Maj+clic sur une "
                      "icône de publicité",
    [RE_MSG_ASSET_BUY_PREFIX] = "Actifs manquants achetés : ",
    [RE_MSG_ASSET_BUY_ITEM] = "%s%s %s x%d",
    [RE_MSG_ASSET_BUY_TOTAL] = " (%s en tout)",
    [RE_MSG_ASSET_SHORT_PREFIX] = "Actifs non achetés : ",
    [RE_MSG_ASSET_SHORT_ROOM] = "%s%s %s (pas de surface)",
    [RE_MSG_ASSET_SHORT_CASH] = "%s%s %s (liquidités insuffisantes)",
    [RE_MSG_ASSET_SHORT_NONE] = "%s%s %s (rien en vente)",
    [RE_MSG_ASSET_BUY_DEBT] = " (%s en tout, dont %s en dette)",
    [RE_MSG_ASSET_TICK_BUY] = "Actifs manquants achetés : %d pour %s",
    [RE_MSG_ASSET_TICK_DEBT] = " (dont %s en dette, à payer sous trois mois)",
    [RE_MSG_ASSET_REPLACED] = "%s : un neuf acheté, %s",
    [RE_MSG_SORT_CHEAP] = "Trier par\nsous-évaluées\n(recherchées)",
    [RE_MSG_SORT_DEAR] = "Trier par\nsurévaluées\n(recherchées)",
    [RE_MSG_SORT_CHANGE] = "Par variation\nsur un mois",
    [RE_MSG_SORT_CAP] = "Trier par\ncapitalisation\n(grandes d'abord)",
};
