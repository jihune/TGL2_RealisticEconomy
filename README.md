# Realistic Economy, a mod for This Grand Life 2

Version 1.0.0 · for game version v1.03.22 · unofficial

[한국어](README.ko.md) · [Deutsch](README.de.md) · [Español](README.es.md) · [Français](README.fr.md) · [Português (Brasil)](README.pt-BR.md) · [日本語](README.ja.md) · [简体中文](README.zh-CN.md)

Money in This Grand Life 2 follows rules closer to real life, and loading an earlier save no longer pays. The mod makes no new window and no new button: it works inside the screens the game already has, and its texts are in the game's language (all eight).

## What's new

### Loans and cash
- Loan interest follows your household's credit grade (AAA to B) and, for a mortgage, the loan-to-value.
- The monthly summary says how much cash the coming month end will take.
- The monthly summary names a property that is for sale well under its value.

### Nothing to gain from loading a save
- Load a save from before a month end you have already passed, and stock and futures trades are locked until that month end has passed again.
- The casino games have a set stake and set odds, each once a month. A result stays after a load.
- A month's job candidates are the same after a load.

### Stock market
- Every company shows PER, PBR, dividend yield, the price against the month before and a fair price. A company close to bankruptcy or to new shares is flagged in advance.
- Research subscription: Shift-click a company and its research is done again every month.
- Listing a company leaves you 45% of the shares and pays cash for the rest. Every 20% you hold guarantees a seat on the board.
- More sort orders for the company list and the futures list, and a six-month view for the charts.

### Businesses
- The hire window shows what an hour of work costs (wage ÷ efficiency), cheapest first.
- Work can be handed to the mod, business by business: staff (hiring for missing hands, letting idle people go, answering pay demands), missing assets, adverts, signing contracts, more floor space. One Ctrl+Shift-click hands over the whole business.

### Faults of the game itself, fixed
- The game ending after a save has been loaded many times.
- Wrong and broken texts in the game's translations (Korean most of all), and boxes in people's names.
- A completed education (degree, certificate) losing value every month.

## Install
1. Close the game. Extract the zip from Releases into the game folder (the one that holds `TGL2.exe`).
2. Double-click `RealisticEconomy_install.bat`. It first copies your saves to `saves_before_RealisticEconomy_1`.
3. Start the game. The version in the lower right corner of the main menu reads "v1.03.22 + Realistic Economy".

To remove the mod, run `RealisticEconomy_uninstall.bat`. Saves made with the mod open without it.

## How to use
Most of it works by itself. These are the clicks; the hover text at each place explains them too.

| Where | Click | What it does |
|---|---|---|
| Company list of the stock window | Shift-click a company | switches that company's research subscription on or off |
| The same list | Ctrl+Shift-click | switches the subscription of every company on or off |
| An advert icon in a business window | Shift-click | hands the adverts to the mod, or takes them back |
| The Contracts icon in a business window | Shift-click | hands signing contracts to the mod, or takes it back |
| An advert icon in a business window | Ctrl+Shift-click | hands the whole business to the mod, or takes it back |
| The automatic-management switch in the staff tab | click | switches it for everybody in that business |
| A business's switch "buy worn-out assets again" | click | the mod buys missing assets too |

Every feature can be switched off in `RealisticEconomy.ini`. The full description is `RealisticEconomy_README_en.txt` in the zip.

## Good to know
- Works with game version v1.03.22 only. On any other version the mod switches itself off.
- An antivirus may question `version.dll`. It is the public Ultimate ASI Loader (MIT), the file that loads the mod when the game starts.
- Not affiliated with the game's developer. MIT licence; the source is in `src`.
