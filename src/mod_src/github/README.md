# Realistic Economy

Realistic Economy is a mod for This Grand Life 2. The mod changes the money rules of the game. It prevents the profit that you can get when you load a save. It adds data to the stock window and automatic functions to the business window. It corrects some faults of the game.

- Mod version: 1.0.0
- Game version: v1.03.22 only
- The game developer did not make this mod.

Other languages: [한국어](README.ko.md), [Deutsch](README.de.md), [Español](README.es.md), [Français](README.fr.md), [Português (Brasil)](README.pt-BR.md), [日本語](README.ja.md), [简体中文](README.zh-CN.md)

## What the mod does

### Loan interest
The interest rate of a loan is the central bank rate plus a margin. The credit grade of your household sets the margin. There are six credit grades: AAA, AA, A, BBB, BB and B.

The mod calculates the credit grade from these four values:

- debt compared with assets
- loan payments for one year compared with income for one year
- cash compared with expenses for one year
- net worth

For a mortgage, the margin is higher when you borrow a larger part of the house price. The loan window shows the credit grade.

### Cash for the month end
The monthly summary is the window that opens when the month changes. The mod puts the line "This month end: cash needed $X" at the top of this window. The amount is the sum of your loan payments and the other costs that you paid at the last month end. Wages and rent are such costs.

### Trade lock after a load
If you load a save from before a month end that you passed, the mod locks trades. During the lock, you cannot buy or sell shares. You also cannot trade futures. The lock stops when you pass that month end again. The lock is not longer than two months.

Without the lock, you can look at the share prices of the next month, load an old save, and buy shares. You can stop this function in the settings file.

### Casino
The mod gives the four games a fixed stake and fixed chances.

| Game | Stake | Result |
|---|---|---|
| Slots | $1,000 | 22%: you get 3 times the stake. 0.8%: you get 40 times the stake. |
| Roulette | $10,000 | 44%: 2 times. 2%: 4 times. |
| Blackjack | $100,000 | 45%: 2 times. 2%: 2.5 times. |
| Baccarat | $1,000,000 | 46.5%: 2 times. |

- You can play each game one time in a month.
- You get or lose the money when the casino action ends.
- If you save and load again, the result is the same.
- If you lose and then load an old save, the mod takes the lost money again.
- The stakes increase with the prices in the game.

### Stock window
The mod shows these values for each company:

- PBR
- PER
- dividend yield
- change of the share price from the last month
- fair price (only for a company that you researched in the game)

Select a company to see the values on the first tab. You can also put the mouse pointer on a company in the list.

The mod shows a warning for a company that is near bankruptcy, a reduction of its size, or an issue of new shares. If you have shares of that company, the monthly summary also shows the warning.

Research subscription:

- Shift-click a company in the list. The mod then researches that company again each month. You pay a fee for each research.
- To subscribe to all companies, Ctrl+Shift-click a company.

Other changes:

- Four of the five sort buttons have a new order:
  - largest market value
  - lowest price compared with the fair price
  - highest price compared with the fair price
  - largest increase from the last month
- The chart of a company opens with only the share price line.
- The charts have a 6-month view.
- The futures list shows the inflation rate and the expected inflation rate of each item.

### IPO and board seats
In the game, an IPO of your business gives you 25% of the shares. You get no cash. With the mod, you keep 45% of the shares. The mod sells the other 55% at the IPO price and gives you the cash. This cash is taxable income for that year.

Each 20% of the shares of a listed company guarantees one board seat. The board has five seats. Nominate a member of your household in the month of the election. If you do not nominate a member, you do not get a seat.

### Job candidates and contract offers
The hire window shows the pay for each effective hour of each candidate. Pay for each effective hour = pay for each hour ÷ work efficiency. Example: $64.84 ÷ 114% = $56.88. The list shows the candidate with the lowest value first.

The candidates of a month do not change when you load a save.

Put the mouse pointer on the payment icon of a contract offer. The mod shows how many percent the offer pays above the standard cost of the work.

If a business does not have sufficient assets, its work efficiency decreases. The monthly summary then shows the name of that business.

### Automation of a business
The mod can do five tasks for a business. You start each task for each business.

| Task | How to start it | What the mod does |
|---|---|---|
| Staff | Start the auto manage mode of the game. Its icon is the round arrow on the staff tab. | Hires a candidate when a job does not have sufficient staff. Dismisses the most expensive employee when a job has too many staff for three months. When an employee demands more pay, replaces the employee with a cheaper candidate. If there is no such candidate, accepts the demand. |
| Assets | Select the check box of the business that buys new assets automatically. | Also buys the assets that are missing. |
| Adverts | Shift-click an advert icon. | Starts the paid adverts when awareness is less than 103%. Stops them when awareness is 103% or more. |
| Contracts | Shift-click the contract icon. | At the start of each month, signs the offers that pay more than their standard cost. Signs as many offers as the business has room for. Does this only when the Staff task and the Assets task are also on. |
| All | Ctrl+Shift-click an advert icon. | Does the four tasks above. Rents more floor space when the floor space is not sufficient. Locks the clicks that change the business manually. |

- To stop a task, do the same click again.
- The mod takes a fee when it hires an employee, keeps adverts on, or signs a contract.
- The mod does nothing for a business that is closed.

### Property below its value
If a property for sale has a price much lower than its value, the monthly summary shows a line. The line gives the address and the gain. The gain is the value minus the price and the purchase fees.

### Completed education
The game decreases the value of a degree or a certificate by 1% each month. After five years, approximately 55% remains. With the mod, a completed education of a member of your household keeps its value. Only the part above the highest job requirement decreases. Experience from work decreases as in the game.

### Corrections of game faults
- The game stopped when you loaded saves some dozens of times without a restart. The mod corrects this fault.
- Some translated texts were incorrect. The mod corrects them. Example: the Spanish text showed "MESm" instead of the number of months.
- Some letters in the names of persons showed as boxes. The mod shows the correct letters.

## Before the installation
- If a game update changes the game version, the mod stops itself. Wait for a new version of the mod.
- An antivirus program can report `version.dll`. This file is the public Ultimate ASI Loader. It loads the mod when the game starts.
- When you load a save from before the mod for the first time, the mod sets two switches to off in all businesses. These are the auto manage mode of the staff and the automatic purchase of assets. With the mod, these two switches do more tasks. Set them to on again only in the businesses that the mod must manage.
- The mod does not add windows or buttons. The texts of the mod show in the windows of the game, in the language of the game.

## Installation
1. Close the game.
2. Download the zip file from [Releases](../../releases).
3. Extract the zip file into the game folder. The game folder is the folder that contains `TGL2.exe`.
4. Double-click `RealisticEconomy_install.bat`. This file first copies your saves to the folder `saves_before_RealisticEconomy_1`.
5. Start the game.
6. Look at the version text at the bottom right of the main menu. If the text contains "+ Realistic Economy", the mod is installed.

## Removal
1. Close the game.
2. Double-click `RealisticEconomy_uninstall.bat`.

You can open the saves from the mod without the mod.

## Settings and manual
- The settings file is `RealisticEconomy.ini`. After the installation, it is in the game folder. In this file, you can stop each function.
- The full manual is [docs/RealisticEconomy_README_en.txt](docs/RealisticEconomy_README_en.txt). It gives all the numbers and fees of the rules. The zip file also contains it.

## License and source
The license is MIT. The source code is in the [src](src) folder.
