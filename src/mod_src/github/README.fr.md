# Realistic Economy

Un mod pour This Grand Life 2 qui change les règles d'argent du jeu. Un ménage très endetté paie plus d'intérêts sur ses prêts. Regarder les cours du mois suivant puis charger une sauvegarde antérieure pour acheter ne sert plus à rien. Charger une sauvegarde après une perte au casino non plus. La fenêtre Bourse affiche les chiffres qu'il faut pour juger une entreprise. Quand vous possédez plusieurs entreprises, vous pouvez confier au mod les embauches, les publicités et les contrats. Quelques défauts du jeu lui-même sont aussi corrigés.

Il fonctionne uniquement avec la version v1.03.22 du jeu. Il n'est pas réalisé par le développeur du jeu. La version actuelle est la 1.0.0.

Autres langues : [English](README.md), [한국어](README.ko.md), [Deutsch](README.de.md), [Español](README.es.md), [Português (Brasil)](README.pt-BR.md), [日本語](README.ja.md), [简体中文](README.zh-CN.md)

## Ce qui change

### Intérêts des prêts
Le taux d'un prêt devient le taux d'intérêt de la Banque centrale plus une marge, et la marge dépend de la note de crédit de votre ménage. Il y a six notes, de AAA à B. La note se calcule à partir de quatre éléments : la dette par rapport aux actifs, les remboursements d'une année par rapport aux revenus d'une année, les liquidités par rapport aux dépenses d'une année et la valeur nette. Une hypothèque coûte plus cher quand la part empruntée du prix de la maison est plus grande. Les fenêtres de prêt affichent votre note à côté du taux d'intérêt.

### Les liquidités que la fin du mois va prendre
Le résumé mensuel, la fenêtre qui s'ouvre au changement de mois, commence par la ligne « Fin de mois : liquidités nécessaires $X ». C'est la somme de vos remboursements de prêts et de ce que les salaires, le loyer et les autres frais ont réellement coûté à la dernière fin de mois.

### Blocage des transactions après un chargement
Si vous chargez une sauvegarde d'avant une fin de mois que vous avez déjà jouée, vous ne pouvez ni acheter ni vendre d'actions ou de contrats à terme jusqu'à ce que cette fin de mois soit passée de nouveau. On ne gagne donc pas d'argent avec des cours déjà vus. Le blocage dure deux mois au plus. Une sauvegarde faite après la fin de mois la plus lointaine que vous avez atteinte n'est pas bloquée. Cette fonction se désactive dans les réglages.

### Casino
Les quatre jeux ont désormais une mise fixe et des chances fixes.

| Jeu | Mise | Gain |
|---|---|---|
| Machines à sous | $1,000 | 22 % de chances de récupérer 3 fois la mise, 0,8 % pour 40 fois |
| Roulette | $10,000 | 44 % pour 2 fois, 2 % pour 4 fois |
| Blackjack | $100,000 | 45 % pour 2 fois, 2 % pour 2,5 fois |
| Baccara | $1,000,000 | 46,5 % pour 2 fois |

Chaque jeu se joue une fois par mois. L'argent bouge quand la partie se termine. Sauvegarder juste avant la fin et recharger donne chaque fois le même résultat. Si vous perdez puis chargez une sauvegarde antérieure, la perte est retirée de nouveau de vos liquidités juste après le chargement. Les mises montent avec les prix du jeu.

### Fenêtre Bourse
Quand une entreprise est sélectionnée, le premier onglet affiche à côté des montants du jeu le PBR, le PER, le rendement du dividende et la variation du cours par rapport au mois précédent. En passant la souris sur une entreprise de la liste, vous voyez les mêmes chiffres sur une ligne. Une entreprise sur laquelle vous avez fait une recherche dans le jeu affiche aussi son juste prix.

Une entreprise sur le point de faire faillite ou d'émettre de nouvelles actions porte un avertissement. Si vous détenez des actions d'une telle entreprise, le résumé mensuel le dit aussi. Le jeu ne prévient qu'après coup.

Avec Maj+clic sur une entreprise de la liste, le mod refait sa recherche chaque mois, contre des frais à chaque fois. Ctrl+Maj+clic le fait pour toutes les entreprises.

Quatre des cinq boutons de tri au-dessus de la liste trient maintenant par capitalisation, par sous-évaluation, par surévaluation et par hausse sur le dernier mois. Le graphique d'une entreprise s'ouvre avec le seul cours de l'action, et les graphiques ont une vue sur six mois. Dans la liste des contrats à terme, chaque ligne affiche son taux d'inflation actuel et le taux attendu.

### Introduction en bourse et conseil d'administration
Dans le jeu, introduire une entreprise en bourse vous donne 25 % des actions et pas d'argent. Avec le mod vous gardez 45 %, et les 55 % restants sont vendus au prix d'introduction et vous sont versés en argent. Cet argent est un revenu imposable de l'année.

Chaque tranche de 20 % détenue dans une entreprise cotée garantit un des cinq sièges du conseil d'administration. Vous devez quand même nommer un membre de votre ménage le mois de l'élection.

### Embauche et offres de contrat
La fenêtre d'embauche affiche pour chaque candidat le salaire par heure effective (salaire horaire ÷ efficacité du travail) et place le moins cher en premier. Une personne payée $64.84 de l'heure avec une efficacité de 114 % coûte $56.88. Les candidats d'un mois restent les mêmes après le chargement d'une sauvegarde.

Passez la souris sur l'icône de paiement d'un contrat proposé pour voir de combien de pour cent il dépasse le coût standard du travail. Quand une entreprise perd de l'efficacité du travail parce qu'il lui manque des actifs, le résumé mensuel la nomme.

### Confier une entreprise au mod
Pour chaque entreprise, vous pouvez confier au mod les tâches ci-dessous une par une. Une embauche, des publicités en cours et la signature d'un contrat coûtent chacune des frais, calculés à partir du salaire horaire d'un métier du jeu.

- Salariés : activez le mode automatique du jeu, l'icône à flèche circulaire dans l'onglet des salariés. Le mod embauche un candidat pour un poste où il manque des bras. Quand un poste a trop de bras trois mois de suite, il renvoie la personne la plus chère. Quand un salarié demande une augmentation, le mod le remplace par un candidat qui fait le même travail pour moins cher, et accorde l'augmentation s'il n'y en a pas.
- Actifs : cochez dans l'entreprise la case qui achète automatiquement de nouveaux actifs quand ils arrivent à expiration. Avec le mod, elle achète aussi les actifs manquants.
- Publicités : Maj+clic sur une icône de publicité. Le mod active les publicités payantes tant que la sensibilisation est sous 103 % et les désactive au-dessus.
- Contrats : Maj+clic sur l'icône des contrats. Au début de chaque mois, le mod signe les offres qui paient plus que leur coût standard, autant que l'entreprise peut en prendre. Il ne le fait que dans une entreprise où les salariés et les actifs lui sont aussi confiés.
- L'entreprise entière : Ctrl+Maj+clic sur une icône de publicité. Cela confie les quatre tâches, et le mod loue en plus davantage de surface quand elle manque. Tant que l'entreprise est confiée, les clics qui la modifient à la main sont verrouillés (embaucher, renvoyer, acheter et vendre des actifs, accepter et résilier des contrats). Un nouveau Ctrl+Maj+clic la reprend.

Tant qu'une entreprise est fermée, le mod n'y fait rien jusqu'à ce que vous la rouvriez.

### Biens en vente sous leur valeur
Quand un bien en vente est proposé nettement sous sa valeur, le résumé mensuel donne son adresse et ce que vous gagneriez après les frais d'achat.

### Formation terminée
Le jeu retire chaque mois 1 % de la valeur d'un diplôme ou d'un certificat. Au bout de cinq ans il en reste environ 55 %, si bien que celui qui ne prend pas le métier peu après son diplôme doit refaire les mêmes études. Avec le mod, une formation qu'un membre du ménage a terminée ne descend pas sous ce qu'elle a apporté. L'expérience acquise en travaillant continue de baisser comme avant.

### Défauts du jeu corrigés
- Le jeu se fermait après quelques dizaines de chargements de sauvegarde sans redémarrage.
- Des textes faux dans les traductions du jeu sont corrigés. En français, quand un secteur était en crise, le jeu annonçait qu'il était « en plein essor ». Dans plusieurs langues, un mot s'affichait à la place d'un nombre ou d'une date.
- Dans certains noms de personnes, des carrés s'affichaient à la place des lettres.

## Avant d'installer
- Quand une mise à jour change la version du jeu, le mod se désactive de lui-même. Il ne change rien tant qu'une édition pour la nouvelle version n'est pas sortie.
- Un antivirus peut se méfier de `version.dll`. C'est l'Ultimate ASI Loader, un chargeur public : le fichier qui charge le mod au lancement du jeu.
- La première fois que vous chargez une sauvegarde d'avant le mod, le mode automatique des salariés et l'achat automatique des actifs sont désactivés dans toutes les entreprises, parce que ces deux interrupteurs font davantage avec le mod. Réactivez-les dans les entreprises que vous voulez confier au mod.
- Le mod n'ajoute ni fenêtre ni bouton. Ses textes s'affichent dans les fenêtres du jeu, dans la langue du jeu.

## Installation
1. Fermez le jeu.
2. Téléchargez le zip depuis les [Releases](../../releases) et extrayez-le dans le dossier du jeu, celui qui contient `TGL2.exe`.
3. Double-cliquez sur `RealisticEconomy_install.bat`. Il copie d'abord vos sauvegardes dans le dossier `saves_before_RealisticEconomy_1`.
4. Lancez le jeu. Le mod est installé si « + Realistic Economy » suit la version en bas à droite du menu principal.

Pour retirer le mod, fermez le jeu et lancez `RealisticEconomy_uninstall.bat`. Les sauvegardes faites avec le mod s'ouvrent sans lui.

## Réglages et description complète
Chaque fonction peut être désactivée dans `RealisticEconomy.ini`, qui se trouve à côté de `TGL2.exe` après l'installation. Le manuel avec les chiffres et les frais de chaque règle est en anglais : [docs/RealisticEconomy_README_en.txt](docs/RealisticEconomy_README_en.txt). Il est aussi dans le zip.

## Licence et code source
Licence MIT. Le code source est dans le dossier [src](src).
