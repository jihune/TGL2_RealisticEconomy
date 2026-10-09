# Realistic Economy

Ein Mod für This Grand Life 2, der die Geldregeln des Spiels ändert. Ein Haushalt mit vielen Schulden zahlt höhere Kreditzinsen. Die Kurse des nächsten Monats ansehen und dann einen früheren Spielstand laden, um zu kaufen, bringt nichts mehr. Dasselbe gilt für das Laden nach einem Verlust im Casino. Das Aktienfenster zeigt die Zahlen, mit denen sich ein Unternehmen beurteilen lässt. Wer mehrere Geschäfte besitzt, kann Einstellungen, Werbung und Verträge dem Mod überlassen. Einige Fehler des Spiels selbst sind ebenfalls behoben.

Der Mod läuft nur mit der Spielversion v1.03.22. Er stammt nicht vom Entwickler des Spiels. Die aktuelle Version ist 1.0.0.

Andere Sprachen: [English](README.md), [한국어](README.ko.md), [Español](README.es.md), [Français](README.fr.md), [Português (Brasil)](README.pt-BR.md), [日本語](README.ja.md), [简体中文](README.zh-CN.md)

## Was sich ändert

### Kreditzinsen
Der Zins eines Kredits ist jetzt der Zinssatz der Zentralbank plus ein Aufschlag, und der Aufschlag hängt von der Bonität deines Haushalts ab. Es gibt sechs Stufen von AAA bis B. Die Stufe ergibt sich aus vier Dingen: Schulden im Verhältnis zum Vermögen, Jahresraten im Verhältnis zum Jahreseinkommen, Bargeld im Verhältnis zu den Jahresausgaben und Nettovermögen. Eine Hypothek wird teurer, je größer der geliehene Teil des Hauspreises ist. Die Kreditfenster zeigen deine Bonität neben dem Zinssatz.

### Was das Monatsende kostet
Die monatliche Zusammenfassung, also das Fenster, das beim Monatswechsel aufgeht, beginnt mit der Zeile "Monatsende: Bargeldbedarf $X". Das sind deine Kreditraten plus das, was Löhne, Miete und andere Kosten am letzten Monatsende tatsächlich gekostet haben.

### Handelssperre nach dem Laden
Lädst du einen Spielstand von vor einem Monatsende, das du schon gespielt hast, kannst du keine Aktien und keine Termingeschäfte kaufen oder verkaufen, bis dieses Monatsende wieder vorbei ist. So lässt sich mit Kursen, die du schon gesehen hast, kein Geld verdienen. Die Sperre dauert höchstens zwei Monate. Ein Spielstand, der nach dem spätesten erreichten Monatsende gespeichert wurde, wird nicht gesperrt. Die Funktion lässt sich in den Einstellungen abschalten.

### Casino
Die vier Spiele bekommen einen festen Einsatz und feste Chancen.

| Spiel | Einsatz | Gewinn |
|---|---|---|
| Spielautomaten | $1,000 | 22 % Chance auf das 3-Fache des Einsatzes, 0,8 % auf das 40-Fache |
| Roulette | $10,000 | 44 % auf das 2-Fache, 2 % auf das 4-Fache |
| Blackjack | $100,000 | 45 % auf das 2-Fache, 2 % auf das 2,5-Fache |
| Baccarat | $1,000,000 | 46,5 % auf das 2-Fache |

Jedes Spiel geht einmal im Monat. Das Geld fließt, wenn das Spiel endet. Kurz vor dem Ende speichern und immer wieder laden ergibt jedes Mal dasselbe Ergebnis. Verlierst du und lädst dann einen früheren Spielstand, wird der Verlust direkt nach dem Laden wieder abgezogen. Die Einsätze steigen mit den Preisen im Spiel.

### Aktienfenster
Ist ein Unternehmen ausgewählt, stehen im ersten Reiter neben den Beträgen des Spiels PBR, PER, Dividendenrendite und die Kursänderung gegenüber dem Vormonat. Fährst du in der Liste mit der Maus über ein Unternehmen, siehst du dieselben Zahlen in einer Zeile. Bei einem Unternehmen, das du im Spiel recherchiert hast, steht auch der faire Kurs dabei.

Ein Unternehmen, dem die Insolvenz oder die Ausgabe neuer Aktien bevorsteht, trägt eine Warnung. Hältst du Aktien eines solchen Unternehmens, steht das auch in der monatlichen Zusammenfassung. Das Spiel selbst meldet es erst, wenn es passiert ist.

Mit Umschalt-Klick auf ein Unternehmen in der Liste wiederholt der Mod dessen Recherche jeden Monat, jedes Mal gegen eine Gebühr. Strg+Umschalt-Klick gilt für alle Unternehmen.

Vier der fünf Sortierknöpfe über der Liste sortieren jetzt nach Marktwert, nach der stärksten Unterbewertung, nach der stärksten Überbewertung und nach dem Anstieg im letzten Monat. Das Diagramm eines Unternehmens öffnet sich nur mit dem Aktienkurs, und die Diagramme bekommen eine Sechs-Monats-Ansicht. In der Terminmarktliste steht bei jedem Eintrag die aktuelle und die erwartete Inflationsrate.

### Börsengang und Direktorium
Im Spiel bringt der Börsengang eines Geschäfts 25 % der Aktien und kein Bargeld. Mit dem Mod behältst du 45 %, und die übrigen 55 % werden zum Ausgabekurs verkauft und dir bar ausgezahlt. Dieses Geld ist steuerpflichtiges Einkommen des Jahres.

Je 20 % Anteil an einem börsennotierten Unternehmen ist einer der fünf Sitze im Direktorium garantiert. Du musst trotzdem im Monat der Wahl ein Haushaltsmitglied nominieren.

### Einstellen und Vertragsangebote
Das Einstellungsfenster zeigt für jeden Kandidaten den Lohn pro effektive Stunde (Stundenlohn ÷ Arbeitseffizienz) und nennt die günstigsten zuerst. Wer $64.84 pro Stunde bei 114 % Effizienz bekommt, kostet $56.88. Die Kandidaten eines Monats bleiben nach dem Laden dieselben.

Fährst du mit der Maus über das Auszahlungssymbol eines Vertragsangebots, siehst du, wie viel Prozent es über den Standardkosten der Arbeit zahlt. Verliert ein Geschäft Arbeitseffizienz, weil Vermögenswerte fehlen, nennt die monatliche Zusammenfassung es.

### Ein Geschäft dem Mod überlassen
Für jedes Geschäft kannst du die folgenden Aufgaben einzeln übergeben. Eine Einstellung, laufende Werbung und ein Vertragsabschluss kosten jeweils eine Gebühr, die aus dem Stundenlohn eines Berufs im Spiel berechnet wird.

- Mitarbeiter: Schalte den Automatikmodus des Spiels ein, das Symbol mit dem runden Pfeil im Mitarbeiter-Reiter. Der Mod stellt für eine Stelle, an der Hände fehlen, einen Kandidaten ein. Hat eine Stelle drei Monate hintereinander zu viele Hände, entlässt er die teuerste Person. Fordert ein Mitarbeiter mehr Lohn, ersetzt der Mod ihn durch einen Kandidaten, der dieselbe Arbeit günstiger macht. Gibt es keinen, bewilligt er die Erhöhung.
- Vermögenswerte: Setze im Geschäft das Häkchen, das neue Vermögenswerte automatisch kauft, wenn die alten ablaufen. Mit dem Mod kauft es auch fehlende Vermögenswerte.
- Werbung: Umschalt-Klick auf ein Werbesymbol. Der Mod schaltet bezahlte Werbung ein, solange die Bekanntheit unter 103 % liegt, und darüber aus.
- Verträge: Umschalt-Klick auf das Vertragssymbol. Am Monatsanfang schließt der Mod die Angebote ab, die mehr als ihre Standardkosten zahlen, so viele, wie das Geschäft Platz hat. Das tut er nur in einem Geschäft, in dem auch Mitarbeiter und Vermögenswerte übergeben sind.
- Das ganze Geschäft: Strg+Umschalt-Klick auf ein Werbesymbol. Das übergibt alle vier, und der Mod mietet außerdem mehr Grundfläche, wenn sie knapp wird. Solange das Geschäft übergeben ist, sind die Klicks gesperrt, die es von Hand ändern (Einstellen, Entlassen, Kauf und Verkauf von Vermögenswerten, Annehmen und Kündigen von Verträgen). Ein weiterer Strg+Umschalt-Klick holt es zurück.

Solange ein Geschäft geschlossen ist, tut der Mod darin nichts, bis du es wieder öffnest.

### Immobilien unter Wert
Wird eine Immobilie deutlich unter ihrem Wert angeboten, nennt die monatliche Zusammenfassung die Adresse und den Gewinn nach den Kaufkosten.

### Abgeschlossene Ausbildung
Das Spiel zieht vom Wert eines Abschlusses oder Zertifikats jeden Monat 1 % ab. Nach fünf Jahren sind noch etwa 55 % übrig. Wer den Beruf nicht bald nach dem Abschluss antritt, muss dasselbe noch einmal lernen. Mit dem Mod fällt eine Ausbildung, die ein Haushaltsmitglied abgeschlossen hat, nicht unter das, was sie gebracht hat. Erfahrung aus der Arbeit nimmt weiter ab wie bisher.

### Behobene Fehler des Spiels
- Das Spiel beendete sich, nachdem ohne Neustart einige Dutzend Mal ein Spielstand geladen worden war.
- Falsche Texte in den Übersetzungen des Spiels sind korrigiert. Im Deutschen stand zum Beispiel bei einem neuen Kredit das Wort "GELD", wo der Betrag hingehört, und im Fenster zur Mietverlängerung "MONATE" statt der Zahl der Monate.
- In manchen Personennamen standen Kästchen statt Buchstaben.

## Vor der Installation
- Ändert ein Update die Spielversion, schaltet sich der Mod selbst ab. Er ändert nichts, bis eine Ausgabe für die neue Version erschienen ist.
- Ein Virenscanner kann `version.dll` beanstanden. Das ist der öffentliche Ultimate ASI Loader, die Datei, die den Mod beim Start des Spiels lädt.
- Wenn du zum ersten Mal einen Spielstand aus der Zeit vor dem Mod lädst, werden der Automatikmodus der Mitarbeiter und der automatische Kauf von Vermögenswerten in allen Geschäften ausgeschaltet, weil beide Schalter mit dem Mod mehr tun. Schalte sie in den Geschäften wieder ein, um die sich der Mod kümmern soll.
- Der Mod baut kein neues Fenster und keinen neuen Knopf. Seine Texte stehen in den Fenstern des Spiels, in der Sprache des Spiels.

## Installation
1. Schließe das Spiel.
2. Lade die Zip-Datei aus den [Releases](../../releases) herunter und entpacke sie in den Spielordner, also dorthin, wo `TGL2.exe` liegt.
3. Doppelklicke `RealisticEconomy_install.bat`. Die Datei kopiert zuerst deine Spielstände in den Ordner `saves_before_RealisticEconomy_1`.
4. Starte das Spiel. Der Mod ist installiert, wenn hinter der Version unten rechts im Hauptmenü "+ Realistic Economy" steht.

Zum Entfernen schließe das Spiel und starte `RealisticEconomy_uninstall.bat`. Spielstände, die mit dem Mod gespeichert wurden, lassen sich auch ohne ihn laden.

## Einstellungen und ausführliche Beschreibung
Jede Funktion lässt sich in `RealisticEconomy.ini` abschalten. Die Datei liegt nach der Installation neben `TGL2.exe`. Das Handbuch mit den Zahlen und Gebühren jeder Regel gibt es auf Englisch: [docs/RealisticEconomy_README_en.txt](docs/RealisticEconomy_README_en.txt). Es liegt auch in der Zip-Datei.

## Lizenz und Quelltext
MIT-Lizenz. Der Quelltext liegt im Ordner [src](src).
