# Realistic Economy

Realistic Economy ist ein Mod für This Grand Life 2. Der Mod ändert die Geldregeln des Spiels. Er verhindert den Gewinn, den du durch das Laden eines Spielstands bekommen kannst. Er zeigt mehr Daten im Aktienfenster und automatisiert Aufgaben im Geschäftsfenster. Er behebt einige Fehler des Spiels.

- Mod-Version: 1.0.0
- Spielversion: nur v1.03.22
- Der Entwickler des Spiels hat diesen Mod nicht gemacht.

Andere Sprachen: [English](README.md), [한국어](README.ko.md), [Español](README.es.md), [Français](README.fr.md), [Português (Brasil)](README.pt-BR.md), [日本語](README.ja.md), [简体中文](README.zh-CN.md)

## Was der Mod tut

### Kreditzinsen
Der Zinssatz eines Kredits ist der Zinssatz der Zentralbank plus ein Aufschlag. Die Bonität deines Haushalts bestimmt den Aufschlag. Es gibt sechs Bonitätsstufen: AAA, AA, A, BBB, BB und B.

Der Mod berechnet die Bonität aus diesen vier Werten:

- Schulden im Vergleich zum Vermögen
- Kreditraten eines Jahres im Vergleich zum Einkommen eines Jahres
- Bargeld im Vergleich zu den Ausgaben eines Jahres
- Nettovermögen

Bei einer Hypothek ist der Aufschlag höher, wenn du einen größeren Teil des Hauspreises leihst. Das Kreditfenster zeigt die Bonität.

### Bargeld für das Monatsende
Die monatliche Zusammenfassung ist das Fenster, das sich beim Monatswechsel öffnet. Der Mod setzt die Zeile "Monatsende: Bargeldbedarf $X" an den Anfang dieses Fensters. Der Betrag ist die Summe deiner Kreditraten und der anderen Kosten, die du am letzten Monatsende bezahlt hast. Löhne und Miete sind solche Kosten.

### Handelssperre nach dem Laden
Wenn du einen Spielstand von vor einem Monatsende lädst, das du schon gespielt hast, sperrt der Mod den Handel. Während der Sperre kannst du keine Aktien kaufen oder verkaufen. Du kannst auch keine Termingeschäfte machen. Die Sperre endet, wenn dieses Monatsende wieder vorbei ist. Die Sperre dauert nicht länger als zwei Monate.

Ohne die Sperre kannst du die Aktienkurse des nächsten Monats ansehen, einen alten Spielstand laden und Aktien kaufen. Du kannst diese Funktion in der Einstellungsdatei abschalten.

### Casino
Der Mod gibt den vier Spielen einen festen Einsatz und feste Chancen.

| Spiel | Einsatz | Ergebnis |
|---|---|---|
| Spielautomaten | $1,000 | 22 %: du bekommst das 3-Fache des Einsatzes. 0,8 %: du bekommst das 40-Fache des Einsatzes. |
| Roulette | $10,000 | 44 %: das 2-Fache. 2 %: das 4-Fache. |
| Blackjack | $100,000 | 45 %: das 2-Fache. 2 %: das 2,5-Fache. |
| Baccarat | $1,000,000 | 46,5 %: das 2-Fache. |

- Du kannst jedes Spiel einmal im Monat spielen.
- Du bekommst oder verlierst das Geld, wenn die Casino-Aktion endet.
- Wenn du speicherst und wieder lädst, bleibt das Ergebnis gleich.
- Wenn du verlierst und dann einen alten Spielstand lädst, zieht der Mod das verlorene Geld wieder ab.
- Die Einsätze steigen mit den Preisen im Spiel.

### Aktienfenster
Der Mod zeigt für jedes Unternehmen diese Werte:

- PBR
- PER
- Dividendenrendite
- Änderung des Aktienkurses gegenüber dem letzten Monat
- fairer Kurs (nur für ein Unternehmen, das du im Spiel recherchiert hast)

Wähle ein Unternehmen aus. Die Werte stehen dann im ersten Reiter. Du kannst auch den Mauszeiger auf ein Unternehmen in der Liste setzen.

Der Mod zeigt eine Warnung bei einem Unternehmen, dem die Insolvenz, eine Verkleinerung oder die Ausgabe neuer Aktien nahe ist. Wenn du Aktien dieses Unternehmens hast, zeigt auch die monatliche Zusammenfassung die Warnung.

Recherche-Abo:

- Mache einen Umschalt-Klick auf ein Unternehmen in der Liste. Der Mod recherchiert dieses Unternehmen dann jeden Monat neu. Du zahlst für jede Recherche eine Gebühr.
- Für ein Abo aller Unternehmen mache einen Strg+Umschalt-Klick auf ein Unternehmen.

Andere Änderungen:

- Vier der fünf Sortierknöpfe haben eine neue Reihenfolge:
  - größter Marktwert
  - niedrigster Kurs im Vergleich zum fairen Kurs
  - höchster Kurs im Vergleich zum fairen Kurs
  - größter Anstieg seit dem letzten Monat
- Das Diagramm eines Unternehmens öffnet sich nur mit der Linie des Aktienkurses.
- Die Diagramme haben eine 6-Monats-Ansicht.
- Die Terminmarktliste zeigt für jeden Eintrag die Inflationsrate und die erwartete Inflationsrate.

### Börsengang und Sitze im Direktorium
Im Spiel bringt dir der Börsengang deines Geschäfts 25 % der Aktien. Du bekommst kein Bargeld. Mit dem Mod behältst du 45 % der Aktien. Der Mod verkauft die anderen 55 % zum Ausgabekurs und gibt dir das Bargeld. Dieses Bargeld ist steuerpflichtiges Einkommen des Jahres.

Je 20 % der Aktien eines börsennotierten Unternehmens garantieren einen Sitz im Direktorium. Das Direktorium hat fünf Sitze. Nominiere im Monat der Wahl ein Mitglied deines Haushalts. Wenn du kein Mitglied nominierst, bekommst du keinen Sitz.

### Kandidaten und Vertragsangebote
Das Einstellungsfenster zeigt für jeden Kandidaten den Lohn pro effektive Stunde. Lohn pro effektive Stunde = Lohn pro Stunde ÷ Arbeitseffizienz. Beispiel: $64.84 ÷ 114 % = $56.88. Die Liste zeigt den Kandidaten mit dem niedrigsten Wert zuerst.

Die Kandidaten eines Monats ändern sich nicht, wenn du einen Spielstand lädst.

Setze den Mauszeiger auf das Auszahlungssymbol eines Vertragsangebots. Der Mod zeigt, wie viel Prozent das Angebot über den Standardkosten der Arbeit zahlt.

Wenn ein Geschäft nicht genug Vermögenswerte hat, sinkt seine Arbeitseffizienz. Die monatliche Zusammenfassung zeigt dann den Namen dieses Geschäfts.

### Automatisierung eines Geschäfts
Der Mod kann fünf Aufgaben für ein Geschäft erledigen. Du startest jede Aufgabe für jedes Geschäft einzeln.

| Aufgabe | So startest du sie | Was der Mod tut |
|---|---|---|
| Mitarbeiter | Schalte den Automatikmodus des Spiels ein. Sein Symbol ist der runde Pfeil im Mitarbeiter-Reiter. | Stellt einen Kandidaten ein, wenn eine Stelle nicht genug Mitarbeiter hat. Entlässt den teuersten Mitarbeiter, wenn eine Stelle drei Monate lang zu viele Mitarbeiter hat. Wenn ein Mitarbeiter mehr Lohn fordert, ersetzt er ihn durch einen günstigeren Kandidaten. Wenn es keinen solchen Kandidaten gibt, nimmt er die Forderung an. |
| Vermögenswerte | Setze im Geschäft das Häkchen, das neue Vermögenswerte automatisch kauft. | Kauft auch die Vermögenswerte, die fehlen. |
| Werbung | Mache einen Umschalt-Klick auf ein Werbesymbol. | Schaltet die bezahlte Werbung ein, wenn die Bekanntheit unter 103 % liegt. Schaltet sie aus, wenn die Bekanntheit 103 % oder mehr beträgt. |
| Verträge | Mache einen Umschalt-Klick auf das Vertragssymbol. | Schließt am Monatsanfang die Angebote ab, die mehr als ihre Standardkosten zahlen. Schließt so viele Angebote ab, wie das Geschäft Platz hat. Tut das nur, wenn auch die Aufgaben Mitarbeiter und Vermögenswerte laufen. |
| Alles | Mache einen Strg+Umschalt-Klick auf ein Werbesymbol. | Erledigt die vier Aufgaben oben. Mietet mehr Grundfläche, wenn die Grundfläche nicht reicht. Sperrt die Klicks, die das Geschäft von Hand ändern. |

- Um eine Aufgabe zu stoppen, mache denselben Klick noch einmal.
- Der Mod nimmt eine Gebühr, wenn er einen Mitarbeiter einstellt, Werbung laufen lässt oder einen Vertrag abschließt.
- Für ein geschlossenes Geschäft tut der Mod nichts.

### Immobilien unter Wert
Wenn eine angebotene Immobilie viel weniger kostet als ihr Wert, zeigt die monatliche Zusammenfassung eine Zeile. Die Zeile nennt die Adresse und den Gewinn. Der Gewinn ist der Wert minus Preis und Kaufkosten.

### Abgeschlossene Ausbildung
Das Spiel senkt den Wert eines Abschlusses oder Zertifikats jeden Monat um 1 %. Nach fünf Jahren bleiben etwa 55 %. Mit dem Mod behält die abgeschlossene Ausbildung eines Haushaltsmitglieds ihren Wert. Nur der Teil über der höchsten Anforderung eines Berufs sinkt. Erfahrung aus der Arbeit sinkt wie im Spiel.

### Behobene Fehler des Spiels
- Das Spiel beendete sich, wenn du ohne Neustart einige Dutzend Mal einen Spielstand geladen hast. Der Mod behebt diesen Fehler.
- Einige übersetzte Texte waren falsch. Der Mod korrigiert sie. Beispiel: Bei einem neuen Kredit stand das Wort "GELD" statt des Betrags.
- Einige Buchstaben in Personennamen erschienen als Kästchen. Der Mod zeigt die richtigen Buchstaben.

## Vor der Installation
- Wenn ein Update die Spielversion ändert, schaltet sich der Mod selbst ab. Warte auf eine neue Version des Mods.
- Ein Virenscanner kann `version.dll` melden. Diese Datei ist der öffentliche Ultimate ASI Loader. Sie lädt den Mod beim Start des Spiels.
- Wenn du zum ersten Mal einen Spielstand aus der Zeit vor dem Mod lädst, schaltet der Mod in allen Geschäften zwei Schalter aus. Das sind der Automatikmodus der Mitarbeiter und der automatische Kauf von Vermögenswerten. Mit dem Mod erledigen diese zwei Schalter mehr Aufgaben. Schalte sie nur in den Geschäften wieder ein, die der Mod führen soll.
- Der Mod fügt keine Fenster und keine Knöpfe hinzu. Die Texte des Mods stehen in den Fenstern des Spiels, in der Sprache des Spiels.

## Installation
1. Schließe das Spiel.
2. Lade die Zip-Datei aus den [Releases](../../releases) herunter.
3. Entpacke die Zip-Datei in den Spielordner. Der Spielordner ist der Ordner mit `TGL2.exe`.
4. Mache einen Doppelklick auf `RealisticEconomy_install.bat`. Diese Datei kopiert zuerst deine Spielstände in den Ordner `saves_before_RealisticEconomy_1`.
5. Starte das Spiel.
6. Sieh dir den Versionstext unten rechts im Hauptmenü an. Wenn der Text "+ Realistic Economy" enthält, ist der Mod installiert.

## Entfernen
1. Schließe das Spiel.
2. Mache einen Doppelklick auf `RealisticEconomy_uninstall.bat`.

Du kannst die Spielstände aus der Zeit mit dem Mod auch ohne den Mod laden.

## Einstellungen und Handbuch
- Die Einstellungsdatei ist `RealisticEconomy.ini`. Nach der Installation liegt sie im Spielordner. In dieser Datei kannst du jede Funktion abschalten.
- Das vollständige Handbuch ist auf Englisch: [docs/RealisticEconomy_README_en.txt](docs/RealisticEconomy_README_en.txt). Es nennt alle Zahlen und Gebühren der Regeln. Es liegt auch in der Zip-Datei.

## Lizenz und Quelltext
Die Lizenz ist MIT. Der Quelltext liegt im Ordner [src](src).
