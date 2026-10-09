# Realistic Economy, un mod pour This Grand Life 2

Version 1.0.0 · pour la version v1.03.22 du jeu · non officiel

[English](README.md) · [한국어](README.ko.md) · [Deutsch](README.de.md) · [Español](README.es.md) · [Português (Brasil)](README.pt-BR.md) · [日本語](README.ja.md) · [简体中文](README.zh-CN.md)

L'argent de This Grand Life 2 suit des règles plus proches de la vie réelle, et charger une sauvegarde antérieure ne rapporte plus rien. Le mod ne crée ni fenêtre ni bouton : il travaille dans les écrans que le jeu possède déjà, et ses textes s'affichent dans la langue du jeu (les huit).

## Nouveautés

### Prêts et liquidités
- Le taux d'un prêt suit la note de crédit de votre ménage (de AAA à B) et, pour une hypothèque, la part du prix empruntée.
- Le résumé mensuel indique les liquidités que la prochaine fin de mois va prendre.
- Le résumé mensuel signale un bien en vente nettement sous sa valeur.

### Charger une sauvegarde ne rapporte rien
- Si vous chargez une sauvegarde d'avant une fin de mois déjà passée, les transactions sur actions et contrats à terme sont bloquées jusqu'à ce que cette fin de mois soit passée de nouveau.
- Les jeux du casino ont une mise fixe et des chances fixes, chacun une fois par mois. Un résultat reste après un chargement.
- Les candidats d'un mois restent les mêmes après un chargement.

### Bourse
- Chaque entreprise affiche PER, PBR, rendement du dividende, le cours par rapport au mois précédent et un juste prix. Une entreprise proche de la faillite ou d'une émission d'actions est signalée à l'avance.
- Abonnement de recherche : faites Maj+clic sur une entreprise, et sa recherche est refaite chaque mois.
- Une introduction en bourse vous laisse 45 % des actions et vous verse le reste en argent. Chaque tranche de 20 % détenue garantit un siège au conseil d'administration.
- Davantage de tris pour la liste des entreprises et celle des contrats à terme, et une vue sur six mois pour les graphiques.

### Entreprises
- La fenêtre d'embauche montre ce que coûte une heure de travail (salaire ÷ efficacité), le moins cher d'abord.
- Vous pouvez confier des tâches au mod, entreprise par entreprise : salariés (embaucher quand il manque des bras, renvoyer ceux qui sont inoccupés, répondre aux demandes d'augmentation), actifs manquants, publicités, signature des contrats, davantage de surface. Un Ctrl+Maj+clic confie l'entreprise entière.

### Défauts du jeu lui-même, corrigés
- Le jeu se fermait après de nombreux chargements de sauvegarde.
- Des textes faux ou cassés dans les traductions du jeu (surtout en coréen) et des carrés dans les noms des personnes.
- Une formation terminée (diplôme, certificat) perdait de sa valeur chaque mois.

## Installation
1. Fermez le jeu. Extrayez le zip des Releases dans le dossier du jeu (celui qui contient `TGL2.exe`).
2. Double-cliquez sur `RealisticEconomy_install.bat`. Il copie d'abord vos sauvegardes dans `saves_before_RealisticEconomy_1`.
3. Lancez le jeu. La version en bas à droite du menu principal indique « v1.03.22 + Realistic Economy ».

Pour retirer le mod, lancez `RealisticEconomy_uninstall.bat`. Les sauvegardes faites avec le mod s'ouvrent sans lui.

## Utilisation
Presque tout fonctionne tout seul. Voici les clics ; l'info-bulle de chaque endroit les explique aussi.

| Où | Clic | Ce qu'il fait |
|---|---|---|
| Liste des entreprises de la fenêtre Bourse | Maj+clic sur une entreprise | active ou désactive l'abonnement de cette entreprise |
| La même liste | Ctrl+Maj+clic | active ou désactive l'abonnement de toutes les entreprises |
| Une icône de publicité dans la fenêtre d'une entreprise | Maj+clic | confie les publicités au mod, ou les reprend |
| L'icône Contrats dans la fenêtre d'une entreprise | Maj+clic | confie la signature des contrats au mod, ou la reprend |
| Une icône de publicité dans la fenêtre d'une entreprise | Ctrl+Maj+clic | confie l'entreprise entière au mod, ou la reprend |
| L'interrupteur de gestion automatique dans l'onglet des salariés | clic | le change pour tous les salariés de cette entreprise |
| L'interrupteur « racheter les actifs usés » d'une entreprise | clic | le mod achète aussi les actifs manquants |

Chaque fonction peut être désactivée dans `RealisticEconomy.ini`. La description complète est en anglais dans le zip : `RealisticEconomy_README_en.txt`.

## Bon à savoir
- Fonctionne uniquement avec la version v1.03.22 du jeu. Avec toute autre version, le mod se désactive de lui-même.
- Un antivirus peut se méfier de `version.dll`. C'est l'Ultimate ASI Loader, public (MIT) : le fichier qui charge le mod au lancement du jeu.
- Sans lien avec le développeur du jeu. Licence MIT ; le code source est dans `src`.
