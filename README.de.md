# Realistic Economy, ein Mod für This Grand Life 2

Version 1.0.0 · für die Spielversion v1.03.22 · inoffiziell

[English](README.md) · [한국어](README.ko.md) · [Español](README.es.md) · [Français](README.fr.md) · [Português (Brasil)](README.pt-BR.md) · [日本語](README.ja.md) · [简体中文](README.zh-CN.md)

Das Geld in This Grand Life 2 folgt Regeln, die näher am echten Leben sind, und das Laden eines früheren Spielstands bringt keinen Vorteil mehr. Der Mod baut kein neues Fenster und keinen neuen Knopf: er arbeitet in den Bildschirmen, die das Spiel schon hat, und seine Texte erscheinen in der Sprache des Spiels (in allen acht).

## Was neu ist

### Kredite und Bargeld
- Der Kreditzins richtet sich nach der Bonität deines Haushalts (AAA bis B) und bei einer Hypothek nach dem Beleihungsgrad.
- Die Monatsübersicht nennt das Bargeld, das das kommende Monatsende kostet.
- Die Monatsübersicht nennt eine Immobilie, die deutlich unter ihrem Wert angeboten wird.

### Kein Gewinn durch das Laden eines Spielstands
- Lädst du einen Spielstand von vor einem Monatsende, das du schon erlebt hast, sind Aktien- und Termingeschäfte gesperrt, bis dieses Monatsende wieder vorbei ist.
- Die Casinospiele haben einen festen Einsatz und feste Chancen, jedes einmal im Monat. Ein Ergebnis bleibt nach dem Laden bestehen.
- Die Kandidaten eines Monats sind nach dem Laden dieselben.

### Aktienmarkt
- Jedes Unternehmen zeigt PER, PBR, Dividendenrendite, den Kurs gegenüber dem Vormonat und einen fairen Kurs. Ein Unternehmen kurz vor der Insolvenz oder vor neuen Aktien wird vorher markiert.
- Recherche-Abo: Umschalt-Klick auf ein Unternehmen, und seine Recherche wird jeden Monat erneuert.
- Ein Börsengang lässt dir 45 % der Aktien und zahlt den Rest in bar aus. Je 20 % Anteil ist ein Sitz im Direktorium garantiert.
- Mehr Sortierungen für die Unternehmensliste und die Terminmarktliste, und eine Sechs-Monats-Ansicht für die Diagramme.

### Geschäfte
- Das Einstellungsfenster zeigt, was eine Stunde Arbeit kostet (Lohn ÷ Arbeitseffizienz), die günstigsten zuerst.
- Arbeit lässt sich Geschäft für Geschäft an den Mod übergeben: Mitarbeiter (fehlende Hände einstellen, Untätige entlassen, Lohnforderungen beantworten), fehlende Vermögenswerte, Werbung, Vertragsabschlüsse, mehr Grundfläche. Ein Strg+Umschalt-Klick übergibt das ganze Geschäft.

### Behobene Fehler des Spiels selbst
- Das Spiel beendete sich, nachdem oft ein Spielstand geladen wurde.
- Falsche und kaputte Texte in den Übersetzungen des Spiels (vor allem im Koreanischen) und Kästchen in Personennamen.
- Ein abgeschlossenes Studium (Abschluss, Zertifikat) verlor jeden Monat an Wert.

## Installation
1. Schließe das Spiel. Entpacke die Zip-Datei aus den Releases in den Spielordner (dort liegt `TGL2.exe`).
2. Doppelklicke `RealisticEconomy_install.bat`. Die Datei kopiert zuerst deine Spielstände nach `saves_before_RealisticEconomy_1`.
3. Starte das Spiel. Die Version unten rechts im Hauptmenü lautet "v1.03.22 + Realistic Economy".

Zum Entfernen starte `RealisticEconomy_uninstall.bat`. Mit dem Mod gespeicherte Spielstände lassen sich auch ohne ihn laden.

## Bedienung
Das meiste läuft von selbst. Das sind die Klicks; der Hinweistext an der jeweiligen Stelle erklärt sie ebenfalls.

| Wo | Klick | Was er tut |
|---|---|---|
| Unternehmensliste im Aktienfenster | Umschalt-Klick auf ein Unternehmen | schaltet dessen Recherche-Abo ein oder aus |
| Dieselbe Liste | Strg+Umschalt-Klick | schaltet das Abo aller Unternehmen ein oder aus |
| Ein Werbesymbol im Geschäftsfenster | Umschalt-Klick | übergibt die Werbung an den Mod oder holt sie zurück |
| Das Symbol "Verträge" im Geschäftsfenster | Umschalt-Klick | übergibt die Vertragsabschlüsse an den Mod oder holt sie zurück |
| Ein Werbesymbol im Geschäftsfenster | Strg+Umschalt-Klick | übergibt das ganze Geschäft an den Mod oder holt es zurück |
| Der Schalter für den Automatikmodus im Mitarbeiter-Reiter | Klick | schaltet ihn für alle Mitarbeiter dieses Geschäfts |
| Der Schalter "abgenutzte Vermögenswerte neu kaufen" eines Geschäfts | Klick | der Mod kauft auch fehlende Vermögenswerte |

Jede Funktion lässt sich in `RealisticEconomy.ini` abschalten. Die ausführliche Beschreibung liegt auf Englisch in der Zip-Datei: `RealisticEconomy_README_en.txt`.

## Gut zu wissen
- Funktioniert nur mit der Spielversion v1.03.22. Bei jeder anderen Version schaltet sich der Mod selbst ab.
- Ein Virenscanner kann `version.dll` beanstanden. Das ist der öffentliche Ultimate ASI Loader (MIT), die Datei, die den Mod beim Start des Spiels lädt.
- Kein Bezug zum Entwickler des Spiels. MIT-Lizenz; der Quelltext liegt in `src`.
