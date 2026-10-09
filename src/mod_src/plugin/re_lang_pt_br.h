/* Brazilian Portuguese: the texts the table of re_lang.c has no Portuguese for. Included by re_lang.c only.
 * The words are the game's own where it has one (modsLanguages/pt-br): falência, conscientização (awareness),
 * ativos, espaço físico, mobiliário, indenização, Conselho de Administração, empresa, funcionários; "você". */
static const char *const LANG_PT_BR[RE_MSG_COUNT] = {
    [RE_MSG_HOVER_YIELD] = ", dividendo %.1f%%",
    [RE_MSG_HOVER_UNDER] = "PBR < 0.75: ",
    [RE_MSG_HOVER_BANKRUPT] = "%s%sfalência no fim do mês%s",
    [RE_MSG_HOVER_MEASURE] = "%s%sredução ou novas ações no fim do mês%s",
    [RE_MSG_HOVER_WATCH] = "%sEm %d meses: falência se o preço cair %.1f%%%s",
    [RE_MSG_HOVER_NEAR] = "Preço -%.1f%%: redução ou novas ações",
    [RE_MSG_HOVER_FAIR] = "Justo %s (%+.0f%%)",
    [RE_MSG_HOVER_TAILWIND] = ", setor em ascensão",
    [RE_MSG_HOVER_HEADWIND] = ", setor em declínio",
    [RE_MSG_ROW_MEASURE] = "redução ou diluição",
    [RE_MSG_ROW_PER_MONTH] = " (%s por mês)",
    [RE_MSG_HELD_PREFIX] = "Aviso sobre suas ações: ",
    [RE_MSG_HELD_CLOSE] = "preço -%.1f%%: redução ou novas ações no fim do mês",
    [RE_MSG_HELD_WATCH] = "preço -%.1f%%: falência no fim do mês",
    [RE_MSG_HELD_MORE] = " e mais %d",
    [RE_MSG_CASINO_SLOTS] = "Caça-níqueis",
    [RE_MSG_CASINO_ROULETTE] = "Roleta",
    [RE_MSG_CASINO_BLACKJACK] = "Blackjack",
    [RE_MSG_CASINO_BACCARAT] = "Bacará",
    [RE_MSG_CASINO_WON] = "%s: ganhou %s vezes a aposta (+%s)",
    [RE_MSG_CASINO_LOST] = "%s: nada ganho (-%s)",
    [RE_MSG_CASINO_PLAYED] = "%s: já jogado neste mês, antes de carregar. Esta partida não tem resultado",
    [RE_MSG_CASINO_BEFORE] = "%s: iniciada antes do mod. O jogo lançou o resultado ao começar; nada mais é lançado",
    [RE_MSG_ADOPT_PREFIX] = "Jogo salvo antes do mod: ",
    [RE_MSG_ADOPT] = "%s“comprar de novo os ativos gastos” desligado em %d empresa(s), gerenciamento automático de %d "
                     "funcionário(s) desligado. Com o mod os dois fazem mais: ligue de novo onde quiser",
    [RE_MSG_ADOPT_ASSETS] = "%s“comprar de novo os ativos gastos” desligado em %d empresa(s). Com o mod ele faz mais: ligue de "
                            "novo onde quiser",
    [RE_MSG_ADOPT_STAFF] = "%sgerenciamento automático de %d funcionário(s) desligado. Com o mod ele faz mais: ligue de novo "
                           "onde quiser",
    [RE_MSG_ADOPT_CASINO] = "%d partida(s) de cassino retirada(s) da fila de ações. Com o mod uma partida tem aposta e o "
                            "resultado vem quando ela termina: coloque na fila de novo se quiser",
    [RE_MSG_ADOPT_CASINO_BEGUN] = "%d partida(s) de cassino retirada(s) da fila de ações. %d já tinham começado: o resultado é o "
                                  "que o jogo deu ao começar, e contam como jogadas neste mês. Com o mod uma partida tem aposta e "
                                  "o resultado vem quando ela termina: coloque na fila de novo se quiser",
    [RE_MSG_CASINO_BACK_PREFIX] = "Cassino: ",
    [RE_MSG_CASINO_BACK] = "%s%d partida(s) jogada(s) antes de carregar continuam valendo (dinheiro %s)",
    [RE_MSG_CASINO_TIP] = "Mensal. Ganha: ",
    [RE_MSG_CASINO_TIP_PRIZE] = "%s%s%%: %sx",
    [RE_MSG_STAFF_LINE] = "%s por hora efetiva",
    [RE_MSG_HIRE_ORDER] = " (o mais barato por hora efetiva primeiro)",
    [RE_MSG_STAFF_TIP] = "%s por hora efetiva (pagamento por hora / eficiência no trabalho %.0f%%)",
    [RE_MSG_OFFER_TIP] = "%+.1f%% acima do custo padrão\n%s sobram por hora",
    [RE_MSG_FUT_EXPECTED] = " (esp. %.2f%%)",
    [RE_MSG_FUT_TIP] = "Esperada: a média da taxa até o vencimento escolhido se ela seguir a regra do jogo. Acima da taxa de "
                       "hoje, a compra sai na frente; abaixo, a venda.",
    [RE_MSG_FUT_ITEM] = " (%+.1f%%/ano)",
    [RE_MSG_FUT_ITEM_AHEAD] = " (%+.1f%% → %+.1f%%)",
    [RE_MSG_FUT_ROW_TIP] = "Inflação %+.2f%% ao ano\nEsperada, 6 meses: %+.2f%%\nEsperada, 12 meses: %+.2f%%\nEsperada, 24 "
                           "meses: %+.2f%%\nEsperada maior: compra. Menor: venda.",
    [RE_MSG_FUT_SORT] = "Ordenar por taxa esperada\n(12 meses) menos a atual",
    [RE_MSG_CHART_HALF_YEAR] = "6 meses",
    [RE_MSG_CHART_INDEX] = " (primeiro mês mostrado = 100)",
    [RE_MSG_RESEARCH_PREFIX] = "Assinatura de pesquisa: ",
    [RE_MSG_RESEARCH_PAID] = "%s%d empresas pesquisadas de novo, taxa %s",
    [RE_MSG_RESEARCH_SHORT] = "%s%d empresas pesquisadas de novo, taxa %s; faltou dinheiro, então %s agora é uma dívida",
    [RE_MSG_RESEARCH_KEPT_PREFIX] = "Taxa de pesquisa já paga: ",
    [RE_MSG_RESEARCH_KEPT] = "%scarregar um jogo salvo anterior não a devolve. %d empresas pagas neste mês foram pesquisadas de "
                             "novo, taxa %s",
    [RE_MSG_RESEARCH_KEPT_SHORT] = "%scarregar um jogo salvo anterior não a devolve. %d empresas pagas neste mês foram "
                                   "pesquisadas de novo, taxa %s; faltou dinheiro, então %s agora é uma dívida",
    [RE_MSG_RESEARCH_KEPT_TICK] = "Taxa de pesquisa paga antes de carregar, cobrada de novo: %s (%d empresas)",
    [RE_MSG_RESEARCH_ONE_ON] = "Assinatura: %s ligada. Pesquisada agora, taxa %s (todo mês)",
    [RE_MSG_RESEARCH_ONE_OFF] = "Assinatura: %s desligada. Sem taxa a partir do próximo mês",
    [RE_MSG_RESEARCH_ALL_ON] = "Assinatura: todas as empresas ligadas. %d pesquisadas agora, taxa %s",
    [RE_MSG_RESEARCH_ALL_OFF] = "Assinatura: todas desligadas. Sem taxa a partir do próximo mês",
    [RE_MSG_RESEARCH_ONE_ON_DONE] = "Assinatura: %s ligada. A pesquisa deste mês já existe, a taxa começa no próximo mês",
    [RE_MSG_RESEARCH_ALL_HAS] = "%s está na assinatura de todas as empresas (Ctrl+Shift-clique: todas desligadas)",
    [RE_MSG_RESEARCH_NO_CASH] = "Assinatura: não ligada, seu dinheiro não cobre a taxa de %s",
    [RE_MSG_RESEARCH_BOARD] = "%s: você está no Conselho, a pesquisa é grátis",
    [RE_MSG_HOVER_CHANGE] = "%+.1f%% sobre o mês anterior",
    /* short, as in German and Spanish: a wider row gets a smaller type */
    [RE_MSG_ROW_CHANGE] = " (%+.1f%%/mês)",
    [RE_MSG_ROW_CHANGE_FAIR] = " (%+.1f%%, justo %s)",
    [RE_MSG_HOVER_SUB_OFF] = "Shift-clique: assinar a pesquisa",
    [RE_MSG_HOVER_SUB_ON] = "Pesquisa assinada (Shift-clique: sair)",
    [RE_MSG_HOVER_SUB_ALL] = "Pesquisa assinada (todas as empresas)",
    [RE_MSG_HOVER_SUB_BOARD] = "No Conselho: pesquisa grátis",
    [RE_MSG_HOVER_ALL_ON] = "Ctrl+Shift-clique: assinar todas",
    [RE_MSG_HOVER_ALL_OFF] = "Ctrl+Shift-clique: desligar todas",
    [RE_MSG_ROW_SUB_ON] = " (assinada)",
    [RE_MSG_ROW_SUB_BOARD] = " (Conselho: grátis)",
    [RE_MSG_ITEM_FAIR] = "justo %+.0f%%",
    [RE_MSG_ITEM_SUB] = "assinada",
    [RE_MSG_ITEM_BOARD] = "Conselho",
    [RE_MSG_PROPERTY_PREFIX] = "Imóvel abaixo do valor: ",
    [RE_MSG_PROPERTY_LINE] = "%s%s, ganho de %s depois das taxas de compra, preço %s",
    [RE_MSG_PROPERTY_NONE] = "%snenhum no momento",
    [RE_MSG_FIT_PREFIX] = "Eficiência no trabalho em queda: ",
    [RE_MSG_FIT_BOTH] = "%s (mobiliário %.0f%%, ativos %.0f%%)",
    [RE_MSG_FIT_FURNISH] = "%s (mobiliário %.0f%%)",
    [RE_MSG_FIT_ASSETS] = "%s (ativos %.0f%%)",
    [RE_MSG_FIT_NONE] = "%s (um trabalho está parado: falta por completo um dos seus ativos)",
    [RE_MSG_XP_TIP] = "\nMod: uma formação que um membro da sua família concluiu (graduação, diploma, certificado) não cai abaixo "
                      "do que foi obtido nela, até a quantidade mais alta que algum trabalho, formação ou atividade pede. A "
                      "experiência de trabalho diminui todo mês como antes.",
    /* The four paragraphs under the game's hover text of the automatic management: kept short, as in German and
     * French, where the first wording made the box taller than a screen of 900 lines. */
    [RE_MSG_AUTO_TIP_ALL] = "\nMod: este interruptor vale para todos os funcionários da empresa.",
    [RE_MSG_AUTO_TIP_SPARE] = "\nMod: um funcionário cujo trabalho os outros poderiam ter feito por %d meses é demitido sem "
                              "indenização. Quem está sozinho e parado metade do tempo é trocado por um candidato de menos "
                              "horas.",
    [RE_MSG_AUTO_TIP_FILL] = "\nMod: se um trabalho tem neste mês mais serviço do que horas do seu pessoal, é contratado o "
                             "candidato que faz o que falta por menos dinheiro.",
    [RE_MSG_AUTO_TIP] = "\nMod: um pedido de aumento é respondido no fim do mês, pouco antes de o funcionário sair. Se um "
                        "candidato faz o mesmo trabalho por menos, ele é contratado; se não, o aumento é dado.",
    [RE_MSG_ADVERTS_TIP] = "\nMod: Shift-clique para entregar os anúncios desta empresa ao mod. Ele liga todos os anúncios pagos "
                           "enquanto a conscientização estiver abaixo de %.0f%% e desliga quando ela chegar lá. Taxa no fim do "
                           "mês: %s por um anúncio ligado o mês inteiro.",
    [RE_MSG_ADVERTS_TIP_ON] = "\nMod: o mod cuida dos anúncios desta empresa. Ele liga todos os anúncios pagos enquanto a "
                              "conscientização estiver abaixo de %.0f%% e desliga quando ela chegar lá. Taxa no fim do mês: %s "
                              "por um anúncio ligado o mês inteiro. Enquanto os ícones estiverem laranja, um clique simples não "
                              "muda nada. Shift-clique para pegar de volta.",
    [RE_MSG_ADVERTS_KEPT] = "%s: o mod cuida dos anúncios agora",
    [RE_MSG_ADVERTS_BACK] = "%s: os anúncios são seus de novo",
    [RE_MSG_ADVERTS_LOCKED] = "%s: o mod controla este anúncio (Shift-clique para pegar os anúncios de volta)",
    [RE_MSG_CONTRACTS_KEPT] = "%s: o mod assina os contratos agora",
    [RE_MSG_CONTRACTS_BACK] = "%s: assinar contratos é com você de novo",
    [RE_MSG_ALL_KEPT] = "%s: o mod cuida de tudo agora",
    [RE_MSG_ALL_BACK] = "%s: tudo é seu de novo",
    [RE_MSG_ADVERTS_TIP_ALL] = "\nMod: Ctrl+Shift-clique entrega a empresa inteira ao mod (funcionários, ativos, anúncios, "
                               "contratos, espaço físico) e trava os controles; de novo, pega de volta. Só os contratos: "
                               "Shift-clique no ícone Contratos.",
    [RE_MSG_CONTRACT_PREFIX] = "Contratos assinados pelo mod: ",
    [RE_MSG_CONTRACT_ITEM] = "%s%s %d horas por mês durante %d meses, %s",
    [RE_MSG_CONTRACT_FEE] = " (taxa %s)",
    [RE_MSG_CONTRACT_TICK] = "O mod assinou %d contrato(s)",
    [RE_MSG_CONTRACT_ROOM] = "%s: sem espaço físico para as ofertas deste mês",
    [RE_MSG_PREMISES_PREFIX] = "Espaço físico (%s): ",
    [RE_MSG_PREMISES_GROW] = "%sele dobra no fim deste mês e o aluguel sobe %s por mês. A empresa está sem espaço, então o mod "
                             "ligou a estrela",
    [RE_MSG_PREMISES_MOST] = "%sestá faltando, e este imóvel não pode ser ampliado. A empresa precisa se mudar",
    [RE_MSG_PREMISES_OWNED] = "%sestá faltando. O imóvel é próprio, então o mod não o ampliou: isso custa %s de uma vez. Ligue a "
                              "estrela você mesmo se quiser",
    [RE_MSG_FIRM_CLOSED] = "%s: fechada. A automação do mod descansa até você abrir de novo (sem contratar, demitir, comprar "
                           "ou assinar; anúncios desligados)",
    [RE_MSG_FIRM_OPENED] = "%s: aberta de novo. A automação do mod continua",
    [RE_MSG_LOCKED] = "%s: o mod cuida desta empresa, então isto está travado. Pegue de volta antes: Ctrl+Shift-clique em um "
                      "ícone de anúncio",
    [RE_MSG_ASSET_BUY_PREFIX] = "Ativos que faltavam, comprados: ",
    [RE_MSG_ASSET_BUY_ITEM] = "%s%s %s x%d",
    [RE_MSG_ASSET_BUY_TOTAL] = " (%s no total)",
    [RE_MSG_ASSET_SHORT_PREFIX] = "Ativos não comprados: ",
    [RE_MSG_ASSET_SHORT_ROOM] = "%s%s %s (sem espaço físico)",
    [RE_MSG_ASSET_SHORT_CASH] = "%s%s %s (falta dinheiro)",
    [RE_MSG_ASSET_SHORT_NONE] = "%s%s %s (nada à venda)",
    [RE_MSG_ASSET_BUY_DEBT] = " (%s no total, %s como dívida)",
    [RE_MSG_ASSET_TICK_BUY] = "Ativos que faltavam, comprados: %d por %s",
    [RE_MSG_ASSET_TICK_DEBT] = " (%s é dívida, vence em três meses)",
    [RE_MSG_ASSET_REPLACED] = "%s: um novo comprado, %s",
    [RE_MSG_SORT_CHEAP] = "Ordenar por\nsubvalorizadas\n(pesquisadas)",
    [RE_MSG_SORT_DEAR] = "Ordenar por\nsupervalorizadas\n(pesquisadas)",
    [RE_MSG_SORT_CHANGE] = "Por variação\nno último mês",
    [RE_MSG_SORT_CAP] = "Ordenar por\nvalor de mercado\n(maior primeiro)",
};
