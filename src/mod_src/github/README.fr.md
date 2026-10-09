# Realistic Economy

Realistic Economy est un mod pour This Grand Life 2. Le mod change les règles d'argent du jeu. Il empêche le gain que vous pouvez obtenir quand vous chargez une sauvegarde. Il ajoute des données à la fenêtre Bourse et des fonctions automatiques à la fenêtre d'entreprise. Il corrige quelques défauts du jeu.

- Version du mod : 1.0.0
- Version du jeu : v1.03.22 uniquement
- Le développeur du jeu n'a pas fait ce mod.

Autres langues : [English](README.md), [한국어](README.ko.md), [Deutsch](README.de.md), [Español](README.es.md), [Português (Brasil)](README.pt-BR.md), [日本語](README.ja.md), [简体中文](README.zh-CN.md)

## Ce que fait le mod

### Intérêts des prêts
Le taux d'intérêt d'un prêt est le taux d'intérêt de la Banque centrale plus une marge. La note de crédit de votre ménage fixe la marge. Il y a six notes de crédit : AAA, AA, A, BBB, BB et B.

Le mod calcule la note de crédit à partir de ces quatre valeurs :

- la dette comparée aux actifs
- les remboursements d'une année comparés aux revenus d'une année
- les liquidités comparées aux dépenses d'une année
- la valeur nette

Pour une hypothèque, la marge est plus élevée quand vous empruntez une plus grande part du prix de la maison. La fenêtre de prêt affiche la note de crédit.

### Liquidités pour la fin du mois
Le résumé mensuel est la fenêtre qui s'ouvre quand le mois change. Le mod place la ligne « Fin de mois : liquidités nécessaires $X » en haut de cette fenêtre. Le montant est la somme de vos remboursements de prêts et des autres frais que vous avez payés à la dernière fin de mois. Les salaires et le loyer sont des frais de ce type.

### Blocage des transactions après un chargement
Si vous chargez une sauvegarde antérieure à une fin de mois déjà passée, le mod bloque les transactions. Pendant le blocage, vous ne pouvez ni acheter ni vendre des actions. Vous ne pouvez pas non plus négocier des contrats à terme. Le blocage s'arrête quand vous passez de nouveau cette fin de mois. Le blocage ne dure pas plus de deux mois.

Sans le blocage, vous pouvez regarder les cours du mois suivant, charger une ancienne sauvegarde et acheter des actions. Vous pouvez désactiver cette fonction dans le fichier de réglages.

### Casino
Le mod donne aux quatre jeux une mise fixe et des chances fixes.

| Jeu | Mise | Résultat |
|---|---|---|
| Machines à sous | $1,000 | 22 % : vous recevez 3 fois la mise. 0,8 % : vous recevez 40 fois la mise. |
| Roulette | $10,000 | 44 % : 2 fois. 2 % : 4 fois. |
| Blackjack | $100,000 | 45 % : 2 fois. 2 % : 2,5 fois. |
| Baccara | $1,000,000 | 46,5 % : 2 fois. |

- Vous pouvez jouer à chaque jeu une fois par mois.
- Vous recevez ou perdez l'argent quand l'action de casino se termine.
- Si vous sauvegardez puis chargez de nouveau, le résultat est le même.
- Si vous perdez puis chargez une ancienne sauvegarde, le mod retire de nouveau l'argent perdu.
- Les mises augmentent avec les prix du jeu.

### Fenêtre Bourse
Le mod affiche ces valeurs pour chaque entreprise :

- PBR
- PER
- rendement du dividende
- variation du cours de l'action depuis le mois dernier
- juste prix (seulement pour une entreprise que vous avez recherchée dans le jeu)

Sélectionnez une entreprise pour voir les valeurs dans le premier onglet. Vous pouvez aussi placer le pointeur de la souris sur une entreprise de la liste.

Le mod affiche un avertissement pour une entreprise proche de la faillite, d'une réduction de taille ou d'une émission de nouvelles actions. Si vous avez des actions de cette entreprise, le résumé mensuel affiche aussi l'avertissement.

Abonnement de recherche :

- Faites Maj+clic sur une entreprise de la liste. Le mod refait alors la recherche de cette entreprise chaque mois. Vous payez des frais pour chaque recherche.
- Pour abonner toutes les entreprises, faites Ctrl+Maj+clic sur une entreprise.

Autres changements :

- Quatre des cinq boutons de tri ont un nouvel ordre :
  - plus grande capitalisation
  - cours le plus bas comparé au juste prix
  - cours le plus haut comparé au juste prix
  - plus forte hausse depuis le mois dernier
- Le graphique d'une entreprise s'ouvre avec la seule ligne du cours de l'action.
- Les graphiques ont une vue sur 6 mois.
- La liste des contrats à terme affiche le taux d'inflation et le taux d'inflation attendu de chaque élément.

### Introduction en bourse et sièges au conseil
Dans le jeu, l'introduction en bourse de votre entreprise vous donne 25 % des actions. Vous ne recevez pas d'argent. Avec le mod, vous gardez 45 % des actions. Le mod vend les 55 % restants au prix d'introduction et vous donne l'argent. Cet argent est un revenu imposable de l'année.

Chaque part de 20 % des actions d'une entreprise cotée garantit un siège au conseil d'administration. Le conseil a cinq sièges. Nommez un membre de votre ménage pendant le mois de l'élection. Si vous ne nommez pas de membre, vous n'obtenez pas de siège.

### Candidats et offres de contrat
La fenêtre d'embauche affiche la rémunération par heure effective de chaque candidat. Rémunération par heure effective = rémunération à l'heure ÷ efficacité du travail. Exemple : $64.84 ÷ 114 % = $56.88. La liste affiche d'abord le candidat qui a la valeur la plus basse.

Les candidats d'un mois ne changent pas quand vous chargez une sauvegarde.

Placez le pointeur de la souris sur l'icône de paiement d'une offre de contrat. Le mod affiche de combien de pour cent l'offre dépasse le coût standard du travail.

Si une entreprise n'a pas assez d'actifs, son efficacité du travail baisse. Le résumé mensuel affiche alors le nom de cette entreprise.

### Automatisation d'une entreprise
Le mod peut faire cinq tâches pour une entreprise. Vous démarrez chaque tâche pour chaque entreprise.

| Tâche | Comment la démarrer | Ce que fait le mod |
|---|---|---|
| Salariés | Activez le mode automatique du jeu. Son icône est la flèche circulaire de l'onglet des salariés. | Embauche un candidat quand un poste n'a pas assez de salariés. Renvoie le salarié le plus cher quand un poste a trop de salariés pendant trois mois. Quand un salarié demande une augmentation, le remplace par un candidat moins cher. S'il n'y a pas de candidat de ce type, accepte la demande. |
| Actifs | Cochez dans l'entreprise la case qui achète automatiquement de nouveaux actifs. | Achète aussi les actifs qui manquent. |
| Publicités | Faites Maj+clic sur une icône de publicité. | Active les publicités payantes quand la sensibilisation est inférieure à 103 %. Les désactive quand la sensibilisation est de 103 % ou plus. |
| Contrats | Faites Maj+clic sur l'icône des contrats. | Au début de chaque mois, signe les offres qui paient plus que leur coût standard. Signe autant d'offres que l'entreprise peut en prendre. Le fait seulement quand les tâches Salariés et Actifs sont aussi actives. |
| Tout | Faites Ctrl+Maj+clic sur une icône de publicité. | Fait les quatre tâches ci-dessus. Loue plus de surface quand la surface ne suffit pas. Verrouille les clics qui modifient l'entreprise à la main. |

- Pour arrêter une tâche, refaites le même clic.
- Le mod prend des frais quand il embauche un salarié, garde des publicités actives ou signe un contrat.
- Le mod ne fait rien pour une entreprise fermée.

### Biens sous leur valeur
Si un bien en vente a un prix très inférieur à sa valeur, le résumé mensuel affiche une ligne. La ligne donne l'adresse et le gain. Le gain est la valeur moins le prix et les frais d'achat.

### Formation terminée
Le jeu baisse chaque mois de 1 % la valeur d'un diplôme ou d'un certificat. Après cinq ans, il reste environ 55 %. Avec le mod, la formation terminée d'un membre de votre ménage garde sa valeur. Seule la part au-dessus de l'exigence la plus haute d'un métier baisse. L'expérience du travail baisse comme dans le jeu.

### Défauts du jeu corrigés
- Le jeu se fermait quand vous chargiez des sauvegardes quelques dizaines de fois sans redémarrage. Le mod corrige ce défaut.
- Certains textes traduits étaient faux. Le mod les corrige. Exemple : pour un secteur en crise, le jeu affichait « en plein essor ».
- Certaines lettres des noms de personnes s'affichaient comme des carrés. Le mod affiche les bonnes lettres.

## Avant l'installation
- Si une mise à jour change la version du jeu, le mod se désactive lui-même. Attendez une nouvelle version du mod.
- Un antivirus peut signaler `version.dll`. Ce fichier est l'Ultimate ASI Loader, qui est public. Il charge le mod au démarrage du jeu.
- Quand vous chargez pour la première fois une sauvegarde antérieure au mod, le mod désactive deux interrupteurs dans toutes les entreprises. Ce sont le mode automatique des salariés et l'achat automatique des actifs. Avec le mod, ces deux interrupteurs font plus de tâches. Réactivez-les seulement dans les entreprises que le mod doit gérer.
- Le mod n'ajoute ni fenêtre ni bouton. Les textes du mod s'affichent dans les fenêtres du jeu, dans la langue du jeu.

## Installation
1. Fermez le jeu.
2. Téléchargez le fichier zip depuis les [Releases](../../releases).
3. Extrayez le fichier zip dans le dossier du jeu. Le dossier du jeu est le dossier qui contient `TGL2.exe`.
4. Double-cliquez sur `RealisticEconomy_install.bat`. Ce fichier copie d'abord vos sauvegardes dans le dossier `saves_before_RealisticEconomy_1`.
5. Démarrez le jeu.
6. Regardez le texte de version en bas à droite du menu principal. Si le texte contient « + Realistic Economy », le mod est installé.

## Désinstallation
1. Fermez le jeu.
2. Double-cliquez sur `RealisticEconomy_uninstall.bat`.

Vous pouvez ouvrir sans le mod les sauvegardes faites avec le mod.

## Réglages et manuel
- Le fichier de réglages est `RealisticEconomy.ini`. Après l'installation, il est dans le dossier du jeu. Dans ce fichier, vous pouvez désactiver chaque fonction.
- Le manuel complet est en anglais : [docs/RealisticEconomy_README_en.txt](docs/RealisticEconomy_README_en.txt). Il donne tous les chiffres et tous les frais des règles. Le fichier zip le contient aussi.

## Licence et code source
La licence est MIT. Le code source est dans le dossier [src](src).
