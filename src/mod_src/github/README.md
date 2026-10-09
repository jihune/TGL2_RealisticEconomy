# Realistic Economy

A mod for This Grand Life 2 that changes the game's money rules. A household with a lot of debt pays more interest on its loans. Seeing next month's share prices and then loading an earlier save to buy no longer works, and neither does loading a save after a loss at the casino. The stock window shows the figures you need to judge a company. Once you own several businesses, their hiring, adverts and contracts can be left to the mod. A few faults of the game itself are fixed as well.

It works with game version v1.03.22 only. It is not made by the game's developer. The current version is 1.0.0.

Other languages: [한국어](README.ko.md), [Deutsch](README.de.md), [Español](README.es.md), [Français](README.fr.md), [Português (Brasil)](README.pt-BR.md), [日本語](README.ja.md), [简体中文](README.zh-CN.md)

## What changes

### Loan interest
A loan's rate becomes the central bank rate plus a margin, and the margin depends on your household's credit grade. There are six grades, from AAA down to B. The grade is worked out from four things: debt against assets, a year's repayments against a year's income, cash against a year's spending, and net worth. A mortgage costs more the larger the share of the house price you borrow. The loan windows show your grade next to the interest rate.

### Cash the month end will take
The monthly summary, the window that opens when the month changes, starts with the line "This month end: cash needed $X". It is your loan payments plus what wages, rent and other costs actually took at the last month end.

### Trade lock after loading a save
Load a save from before a month end you have already played past, and you cannot buy or sell shares or futures until that month end has passed again. This keeps you from profiting on prices you have already seen. The lock lasts two months at most. A save written later than the furthest month end you have reached is not locked. You can switch this feature off in the settings.

### Casino
The four games get a set stake and set odds.

| Game | Stake | Payout |
|---|---|---|
| Slots | $1,000 | 22% chance of 3 times the stake back, 0.8% chance of 40 times |
| Roulette | $10,000 | 44% chance of 2 times, 2% chance of 4 times |
| Blackjack | $100,000 | 45% chance of 2 times, 2% chance of 2.5 times |
| Baccarat | $1,000,000 | 46.5% chance of 2 times |

Each game can be played once a month. The money moves when the game ends. Saving just before the end and loading again gives the same result every time. If you lose and then load an earlier save, the loss is taken from your cash again right after the load. The stakes rise with the game's prices.

### Stock window
With a company selected, the first tab shows PBR, PER, dividend yield and the price change over the last month next to the game's amounts. Hovering over a company in the list shows the same figures in one line. A company you have researched with "Research Stock" also shows its fair price.

A company that is about to go bankrupt or to issue new shares carries a warning. If you hold shares in such a company, the monthly summary says so too. The game itself tells you only after it has happened.

Shift-click a company in the list and the mod repeats its research every month, for a fee each time. Ctrl+Shift-click does this for every company.

Four of the five sort buttons above the list now sort by market value, by most underpriced, by most overpriced and by the rise over the last month. A company's chart opens with only the share price switched on, and the charts get a six-month view. In the futures list every item shows its current inflation rate and the expected one.

### Going public and the board
In the game, listing a business gives you 25% of the shares and no cash. With the mod you keep 45%, and the other 55% is sold at the listing price and paid to you in cash. That cash is taxable income of that year.

Every 20% you hold of a listed company guarantees one of the five board seats. You still have to nominate a household member in the month the election is held.

### Hiring and contract offers
The hire window shows each candidate's wage per effective hour (hourly wage ÷ work efficiency) and lists the cheapest first. Someone paid $64.84 an hour at 114% efficiency costs $56.88. A month's candidates stay the same after you load a save.

Hover over the payout icon of an offered contract to see how many percent it pays over the standard cost of the work. When a business loses work efficiency because assets are missing, the monthly summary names it.

### Leaving a business to the mod
For each business you can hand over the tasks below one by one. Hiring someone, running adverts and signing a contract each cost a fee, worked out from the hourly wage of a job in the game.

- Staff: switch on the game's auto manage mode, the round-arrow icon in the staff tab. The mod hires a candidate for a job that is short of hands. Where a job has had spare hands for three months in a row, it lets the most expensive person go. When an employee asks for a raise, the mod replaces them with a candidate who does the same work for less, and grants the raise if there is none.
- Assets: tick the business's box that buys new assets automatically when they expire. With the mod it buys missing assets too.
- Adverts: Shift-click an advert icon. The mod switches paid adverts on while awareness is under 103% and off above that.
- Contracts: Shift-click the contracts icon. At the start of each month the mod signs the offers that pay more than their standard cost, as many as the business has room for. It does so only in a business where staff and assets are handed over as well.
- The whole business: Ctrl+Shift-click an advert icon. This hands over all four and also lets the mod rent more floor space when it runs short. While the business is handed over, the clicks that change it by hand are locked (hiring, firing, buying and selling assets, accepting and cancelling contracts). Ctrl+Shift-click again to take it back.

While a business is closed, the mod does nothing in it until you open it again.

### Property for sale under its value
When a property on the market is priced well under its value, the monthly summary gives its address and what you would gain after the purchase fees.

### Completed education
The game takes 1% off the value of a degree or certificate every month. After five years about 55% is left, so someone who does not start the job soon after the degree has to study the same thing again. With the mod, an education that a household member has completed does not drop below what it gave. Experience gained by working still fades as before.

### Faults of the game that are fixed
- The game used to shut down after a save had been loaded a few dozen times without a restart.
- Wrong texts in the game's translations are corrected. In Korean a fixed interest rate was labelled with the word for "modified". In several languages a placeholder word was shown where a number or a date belonged, such as "MESm" for a number of months in Spanish.
- Some people's names showed boxes in place of letters.

## Before you install
- When a game update changes the version, the mod switches itself off. It changes nothing until a release for the new version is out.
- An antivirus may flag `version.dll`. It is the public Ultimate ASI Loader, the file that loads the mod when the game starts.
- The first time you load a save from before the mod, the auto manage mode of the staff and the automatic asset purchase are switched off in every business, because these two switches do more with the mod. Switch them back on in the businesses you want the mod to look after.
- The mod adds no window and no button. Its texts appear inside the game's own windows, in the game's language.

## Install
1. Close the game.
2. Download the zip from [Releases](../../releases) and extract it into the game folder, the one that holds `TGL2.exe`.
3. Double-click `RealisticEconomy_install.bat`. It first copies your saves to the folder `saves_before_RealisticEconomy_1`.
4. Start the game. The mod is installed if the version in the lower right corner of the main menu has "+ Realistic Economy" after it.

To remove the mod, close the game and run `RealisticEconomy_uninstall.bat`. Saves written with the mod open without it.

## Settings and the full description
Each feature can be switched off in `RealisticEconomy.ini`, which sits next to `TGL2.exe` after the install. The manual with every rule's numbers and fees is [docs/RealisticEconomy_README_en.txt](docs/RealisticEconomy_README_en.txt). It is in the zip as well.

## Licence and source
MIT licence. The source is in the [src](src) folder.
