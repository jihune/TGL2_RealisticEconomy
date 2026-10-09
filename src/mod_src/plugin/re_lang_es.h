/* Spanish: the texts the table of re_lang.c has no Spanish for. Included by re_lang.c only.
 * The words are the game's own where it has one (modsLanguages/es): quiebra, conciencia (awareness), activos,
 * superficie, mobiliario, indemnización, Consejo de Administración, negocio; the game says "tú". */
static const char *const LANG_ES[RE_MSG_COUNT] = {
    [RE_MSG_HOVER_YIELD] = ", dividendo %.1f%%",
    [RE_MSG_HOVER_UNDER] = "PBR < 0.75: ",
    [RE_MSG_HOVER_BANKRUPT] = "%s%squiebra a fin de mes%s",
    [RE_MSG_HOVER_MEASURE] = "%s%sreducción o nuevas acciones a fin de mes%s",
    [RE_MSG_HOVER_WATCH] = "%sEn %d meses: quiebra si el precio cae un %.1f%%%s",
    [RE_MSG_HOVER_NEAR] = "Precio -%.1f%%: reducción o nuevas acciones",
    [RE_MSG_HOVER_FAIR] = "Justo %s (%+.0f%%)",
    [RE_MSG_HOVER_TAILWIND] = ", industria en alza",
    [RE_MSG_HOVER_HEADWIND] = ", industria en declive",
    [RE_MSG_ROW_MEASURE] = "reducción o dilución",
    [RE_MSG_ROW_PER_MONTH] = " (%s al mes)",
    [RE_MSG_HELD_PREFIX] = "Aviso sobre tus acciones: ",
    [RE_MSG_HELD_CLOSE] = "precio -%.1f%%: reducción o nuevas acciones a fin de mes",
    [RE_MSG_HELD_WATCH] = "precio -%.1f%%: quiebra a fin de mes",
    [RE_MSG_HELD_MORE] = " y %d más",
    [RE_MSG_CASINO_SLOTS] = "Tragaperras",
    [RE_MSG_CASINO_ROULETTE] = "Ruleta",
    [RE_MSG_CASINO_BLACKJACK] = "Blackjack",
    [RE_MSG_CASINO_BACCARAT] = "Bacará",
    [RE_MSG_CASINO_WON] = "%s: ganaste %s veces la apuesta (+%s)",
    [RE_MSG_CASINO_LOST] = "%s: sin premio (-%s)",
    [RE_MSG_CASINO_PLAYED] = "%s: ya se jugó este mes, antes de cargar. Esta partida no tiene resultado",
    [RE_MSG_CASINO_BEFORE] = "%s: empezada antes del mod. El juego anotó su resultado al empezar; no se anota nada más",
    [RE_MSG_ADOPT_PREFIX] = "Partida guardada antes del mod: ",
    [RE_MSG_ADOPT] = "%s«comprar de nuevo los activos gastados» desactivado en %d negocio(s), gestión automática de %d "
                     "empleado(s) desactivada. Con el mod ambos hacen más: actívalos de nuevo donde quieras",
    [RE_MSG_ADOPT_ASSETS] = "%s«comprar de nuevo los activos gastados» desactivado en %d negocio(s). Con el mod hace más: "
                            "actívalo de nuevo donde quieras",
    [RE_MSG_ADOPT_STAFF] = "%sgestión automática de %d empleado(s) desactivada. Con el mod hace más: actívala de nuevo donde "
                           "quieras",
    [RE_MSG_ADOPT_CASINO] = "%d partida(s) de casino retirada(s) de la cola de acciones. Con el mod una partida tiene apuesta y "
                            "su resultado llega al terminar: ponla en cola otra vez si la quieres",
    [RE_MSG_ADOPT_CASINO_BEGUN] = "%d partida(s) de casino retirada(s) de la cola de acciones. %d ya habían empezado: su resultado "
                                  "es el que dio el juego al empezar, y cuentan como jugadas este mes. Con el mod una partida "
                                  "tiene apuesta y su resultado llega al terminar: ponla en cola otra vez si la quieres",
    [RE_MSG_CASINO_BACK_PREFIX] = "Casino: ",
    [RE_MSG_CASINO_BACK] = "%s%d partida(s) jugada(s) antes de cargar siguen contando (efectivo %s)",
    [RE_MSG_CASINO_TIP] = "Mensual. Gana: ",
    [RE_MSG_CASINO_TIP_PRIZE] = "%s%s%%: %sx",
    [RE_MSG_STAFF_LINE] = "%s por hora efectiva",
    [RE_MSG_HIRE_ORDER] = " (primero el más barato por hora efectiva)",
    [RE_MSG_STAFF_TIP] = "%s por hora efectiva (pago por hora / eficiencia laboral %.0f%%)",
    [RE_MSG_OFFER_TIP] = "%+.1f%% sobre el coste estándar\n%s quedan por hora",
    [RE_MSG_FUT_EXPECTED] = " (esp. %.2f%%)",
    [RE_MSG_FUT_TIP] = "Esperada: la media de la tasa hasta el vencimiento elegido si se mueve según la regla del juego. Por "
                       "encima de la tasa de hoy conviene comprar; por debajo, vender.",
    [RE_MSG_FUT_ITEM] = " (%+.1f%%/año)",
    [RE_MSG_FUT_ITEM_AHEAD] = " (%+.1f%% → %+.1f%%)",
    [RE_MSG_FUT_ROW_TIP] = "Inflación %+.2f%% al año\nEsperada, 6 meses: %+.2f%%\nEsperada, 12 meses: %+.2f%%\nEsperada, 24 "
                           "meses: %+.2f%%\nEsperada mayor: compra. Menor: venta.",
    [RE_MSG_FUT_SORT] = "Ordenar por tasa esperada\n(12 meses) menos la actual",
    [RE_MSG_CHART_HALF_YEAR] = "6 meses",
    [RE_MSG_CHART_INDEX] = " (primer mes mostrado = 100)",
    [RE_MSG_RESEARCH_PREFIX] = "Suscripción de investigación: ",
    [RE_MSG_RESEARCH_PAID] = "%s%d empresas investigadas de nuevo, cuota %s",
    [RE_MSG_RESEARCH_SHORT] = "%s%d empresas investigadas de nuevo, cuota %s; te faltaba efectivo, así que %s es ahora una deuda",
    [RE_MSG_RESEARCH_KEPT_PREFIX] = "Cuota de investigación ya pagada: ",
    [RE_MSG_RESEARCH_KEPT] = "%scargar una partida anterior no la devuelve. %d empresas pagadas este mes se investigaron de "
                             "nuevo, cuota %s",
    [RE_MSG_RESEARCH_KEPT_SHORT] = "%scargar una partida anterior no la devuelve. %d empresas pagadas este mes se investigaron de "
                                   "nuevo, cuota %s; te faltaba efectivo, así que %s es ahora una deuda",
    [RE_MSG_RESEARCH_KEPT_TICK] = "Cuota de investigación pagada antes de cargar, cobrada de nuevo: %s (%d empresas)",
    [RE_MSG_RESEARCH_ONE_ON] = "Suscripción: %s activada. Investigada ahora, cuota %s (cada mes)",
    [RE_MSG_RESEARCH_ONE_OFF] = "Suscripción: %s desactivada. Sin cuota desde el mes que viene",
    [RE_MSG_RESEARCH_ALL_ON] = "Suscripción: todas las empresas activadas. %d investigadas ahora, cuota %s",
    [RE_MSG_RESEARCH_ALL_OFF] = "Suscripción: todas desactivadas. Sin cuota desde el mes que viene",
    [RE_MSG_RESEARCH_ONE_ON_DONE] = "Suscripción: %s activada. Ya tiene la investigación de este mes, la cuota empieza el mes que "
                                    "viene",
    [RE_MSG_RESEARCH_ALL_HAS] = "%s está en la suscripción de todas las empresas (Ctrl+Mayús+clic: todas desactivadas)",
    [RE_MSG_RESEARCH_NO_CASH] = "Suscripción: no activada, tu efectivo no cubre la cuota de %s",
    [RE_MSG_RESEARCH_BOARD] = "%s: estás en su Consejo, su investigación es gratis",
    [RE_MSG_HOVER_CHANGE] = "%+.1f%% sobre el mes anterior",
    /* short: with " en un mes" the row was wider than the window takes and got a smaller type (run 520) */
    [RE_MSG_ROW_CHANGE] = " (%+.1f%%/mes)",
    [RE_MSG_ROW_CHANGE_FAIR] = " (%+.1f%%, justo %s)",
    [RE_MSG_HOVER_SUB_OFF] = "Mayús+clic: suscribir investigación",
    [RE_MSG_HOVER_SUB_ON] = "Investigación suscrita (Mayús+clic: no)",
    [RE_MSG_HOVER_SUB_ALL] = "Investigación suscrita (todas)",
    [RE_MSG_HOVER_SUB_BOARD] = "En el Consejo: investigación gratis",
    [RE_MSG_HOVER_ALL_ON] = "Ctrl+Mayús+clic: suscribir todas",
    [RE_MSG_HOVER_ALL_OFF] = "Ctrl+Mayús+clic: desactivar todas",
    [RE_MSG_ROW_SUB_ON] = " (suscrita)",
    [RE_MSG_ROW_SUB_BOARD] = " (Consejo: gratis)",
    [RE_MSG_ITEM_FAIR] = "justo %+.0f%%",
    [RE_MSG_ITEM_SUB] = "suscrita",
    [RE_MSG_ITEM_BOARD] = "Consejo",
    [RE_MSG_PROPERTY_PREFIX] = "Propiedad por debajo de su valor: ",
    [RE_MSG_PROPERTY_LINE] = "%s%s, ganancia de %s tras las tarifas de compra, precio %s",
    [RE_MSG_PROPERTY_NONE] = "%sninguna por ahora",
    [RE_MSG_FIT_PREFIX] = "Baja la eficiencia laboral: ",
    [RE_MSG_FIT_BOTH] = "%s (mobiliario %.0f%%, activos %.0f%%)",
    [RE_MSG_FIT_FURNISH] = "%s (mobiliario %.0f%%)",
    [RE_MSG_FIT_ASSETS] = "%s (activos %.0f%%)",
    [RE_MSG_FIT_NONE] = "%s (un trabajo está parado: le falta del todo uno de sus activos)",
    [RE_MSG_XP_TIP] = "\nMod: una educación que un miembro de tu hogar completó (título, diploma, certificado) no baja de lo que "
                      "se obtuvo de ella, hasta la cantidad más alta que pide algún trabajo, educación o actividad. La "
                      "experiencia de trabajo decae cada mes como antes.",
    /* The four paragraphs under the game's hover text of the automatic management: kept short, as in German and
     * French, where the first wording made the box taller than a screen of 900 lines. */
    [RE_MSG_AUTO_TIP_ALL] = "\nMod: este interruptor vale para todos los empleados del negocio.",
    [RE_MSG_AUTO_TIP_SPARE] = "\nMod: a un empleado cuyo trabajo podrían haber hecho los demás durante %d meses se le despide sin "
                              "indemnización. Quien está solo y parado la mitad del tiempo se sustituye por un candidato de "
                              "menos horas.",
    [RE_MSG_AUTO_TIP_FILL] = "\nMod: si un trabajo tiene este mes más tarea que horas su gente, se contrata al candidato que hace "
                             "lo que falta por menos dinero.",
    [RE_MSG_AUTO_TIP] = "\nMod: una petición de aumento se responde a fin de mes, justo antes de que el empleado se vaya. Si un "
                        "candidato hace el mismo trabajo por menos, se le contrata; si no, se concede el aumento.",
    [RE_MSG_ADVERTS_TIP] = "\nMod: Mayús+clic para dejar los anuncios de este negocio al mod. Activa todos los anuncios de pago "
                           "mientras la conciencia esté por debajo del %.0f%% y los desactiva al llegar. Cuota a fin de mes: %s "
                           "por un anuncio activo todo el mes.",
    [RE_MSG_ADVERTS_TIP_ON] = "\nMod: el mod se ocupa de los anuncios de este negocio. Activa todos los anuncios de pago mientras "
                              "la conciencia esté por debajo del %.0f%% y los desactiva al llegar. Cuota a fin de mes: %s por un "
                              "anuncio activo todo el mes. Mientras los iconos estén en naranja, un clic normal no cambia nada. "
                              "Mayús+clic para recuperarlos.",
    [RE_MSG_ADVERTS_KEPT] = "%s: el mod se ocupa ahora de los anuncios",
    [RE_MSG_ADVERTS_BACK] = "%s: los anuncios vuelven a ser tuyos",
    [RE_MSG_ADVERTS_LOCKED] = "%s: este anuncio lo maneja el mod (Mayús+clic para recuperar los anuncios)",
    [RE_MSG_CONTRACTS_KEPT] = "%s: el mod firma ahora sus contratos",
    [RE_MSG_CONTRACTS_BACK] = "%s: vuelves a firmar tú los contratos",
    [RE_MSG_ALL_KEPT] = "%s: el mod lo lleva todo ahora",
    [RE_MSG_ALL_BACK] = "%s: todo vuelve a ser tuyo",
    [RE_MSG_ADVERTS_TIP_ALL] = "\nMod: Ctrl+Mayús+clic deja todo el negocio al mod (empleados, activos, anuncios, contratos, "
                               "superficie) y bloquea sus controles; otra vez lo recupera. Solo los contratos: Mayús+clic en el "
                               "icono Contratos.",
    [RE_MSG_CONTRACT_PREFIX] = "Contratos firmados por el mod: ",
    [RE_MSG_CONTRACT_ITEM] = "%s%s %d horas al mes durante %d meses, %s",
    [RE_MSG_CONTRACT_FEE] = " (cuota %s)",
    [RE_MSG_CONTRACT_TICK] = "El mod firmó %d contrato(s)",
    [RE_MSG_CONTRACT_ROOM] = "%s: sin superficie para las ofertas de este mes",
    [RE_MSG_PREMISES_PREFIX] = "Superficie (%s): ",
    [RE_MSG_PREMISES_GROW] = "%sse duplica al final de este mes y el alquiler sube %s al mes. Al negocio le falta superficie, "
                             "así que el mod activó la estrella",
    [RE_MSG_PREMISES_MOST] = "%sfalta, y este local no se puede ampliar. El negocio tiene que mudarse",
    [RE_MSG_PREMISES_OWNED] = "%sfalta. El local es propio, así que el mod no lo amplió: cuesta %s de una vez. Activa tú la "
                              "estrella si lo quieres",
    [RE_MSG_FIRM_CLOSED] = "%s: cerrado. La automatización del mod descansa hasta que lo abras (no contrata, despide, compra ni "
                           "firma; anuncios desactivados)",
    [RE_MSG_FIRM_OPENED] = "%s: abierto de nuevo. La automatización del mod continúa",
    [RE_MSG_LOCKED] = "%s: el mod lleva este negocio, así que esto está bloqueado. Recupéralo antes: Ctrl+Mayús+clic en un icono "
                      "de anuncio",
    [RE_MSG_ASSET_BUY_PREFIX] = "Activos que faltaban, comprados: ",
    [RE_MSG_ASSET_BUY_ITEM] = "%s%s %s x%d",
    [RE_MSG_ASSET_BUY_TOTAL] = " (%s en total)",
    [RE_MSG_ASSET_SHORT_PREFIX] = "Activos no comprados: ",
    [RE_MSG_ASSET_SHORT_ROOM] = "%s%s %s (sin superficie)",
    [RE_MSG_ASSET_SHORT_CASH] = "%s%s %s (falta efectivo)",
    [RE_MSG_ASSET_SHORT_NONE] = "%s%s %s (nada en venta)",
    [RE_MSG_ASSET_BUY_DEBT] = " (%s en total, %s como deuda)",
    [RE_MSG_ASSET_TICK_BUY] = "Activos que faltaban, comprados: %d por %s",
    [RE_MSG_ASSET_TICK_DEBT] = " (%s es una deuda, vence en tres meses)",
    [RE_MSG_ASSET_REPLACED] = "%s: comprado uno nuevo, %s",
    [RE_MSG_SORT_CHEAP] = "Ordenar por\ninfravaloradas\n(investigadas)",
    [RE_MSG_SORT_DEAR] = "Ordenar por\nsobrevaloradas\n(investigadas)",
    [RE_MSG_SORT_CHANGE] = "Por cambio\ndel último mes",
    [RE_MSG_SORT_CAP] = "Ordenar por\ncap. bursátil\n(mayor primero)",
};
