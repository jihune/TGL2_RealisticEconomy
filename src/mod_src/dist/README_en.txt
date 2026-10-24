Realistic Economy - a mod for This Grand Life 2

The mod's version and packaging date are on the first line of RealisticEconomy_SHA256.txt in the RealisticEconomy
folder.

For game version v1.03.22 only (shown at the bottom right of the main menu). On any other version the mod changes
nothing and writes one line to RealisticEconomy.log. Unofficial; not connected with the game's developer.


What is a matter of taste, and how to switch it off
---------------------------------------------------

The mod does three kinds of things: it repairs faults of the game, it shows numbers the game hides, and it changes
rules or numbers of the game. The first two make the game neither easier nor harder. The third is a matter of
taste. Everything of that kind is listed here, and each can be switched off; what is off is the game as it was.

How: open RealisticEconomy.ini in the game folder with Notepad, change the value of the line, save, and start the
game again. "[features] balance=0" means: under the line [features], change balance=1 to balance=0.

   What changes the game's numbers
     - Balance (14 below). Share fee 2% (the game: 1%), futures fee 10% of the margin (1%), savings rate 0.8 (0.5)
       of the central-bank rate, interest on cash, a rent that can come down at a renewal, a new business starts
       with 25% awareness (0%), Hydroponics Farm's income, the price of five degrees. The fees make the game
       harder; the interest on cash, the awareness and the degree prices make it easier.
          All of it off: [features] balance=0
          One by one: put the game's number in [balance] (each line names it).
     - Competition comes earlier (the last of 14). In a business type's market the firms count 1.25 times. With
       more firms of a type, competition sets in sooner than in the game and takes more awareness every month.
       It makes the game harder.
          Off: [balance] competitionWeight=1
     - A business's monthly bonus for completed work goes down with the share left undone (14). The game takes the
       whole bonus of 3.6 points for one hour left; the mod lowers it with the share, to nothing at 2%. It makes the
       game easier.
          Off: [balance] reputationBonusFade=0
     - Money into influence (15). A bank has two large donations ($1M, $10M), and the city votes on its policies
       every 12 months (the game: 24). Something to spend the money of the late game on.
          Off: [features] influence=0
          One by one: [influence] bigDonations=0, [influence] voteEvery12Months=0
     - Personal Coaching (16). At a hospital, for money, a preference you choose moves one step towards a like, one
       preference a year for each person. In the game preferences change only on a birthday.
          Off: [features] coaching=0
     - Loan rates by credit grade (1). A good grade borrows cheaper than in the game, a poor one dearer.
          Off: [features] credit=0
     - A listing pays cash (4). You keep 45% of the shares and get the money for the rest (the game: 25%, no cash).
          Off: [features] ipo=0
     - Board seats (5). Every 20% of a listed company guarantees a seat on its board.
          Off: [features] board=0
     - Casino (8). Set stakes and chances, each game once a month (the game: stakes that grow with your net worth,
       no limit).
          Off: [features] casino=0
     - A completed education does not decay (12). The game takes 1% a month off a degree's number too.
          Off: [features] experience=0
     - Two loopholes of the game that are closed (1): a mortgage larger than the amount approved, and an
       auto-transfer rule that buys shares without cash.
          Back to the game: [credit] mortgageUpToApproval=0, [credit] transferNeedsCash=0

   What keeps a load from changing a result
     - The trade lock after a load (3). After you load a save from before a month end you have already passed,
       trades in shares and futures are refused until that month end has passed again. A nuisance for somebody
       who goes back to earlier saves often.
          Off: [features] guard=0 (the fixed month end below goes with it)
          Once: [guard] release=1
     - The fixed month end (3). The month's growth, share prices, a listing price and the next month's properties
       for sale are the same after a load.
          Off: [guard] monthEndFixed=0
     - Casino results kept (8): [casino] keepResultsOnLoad=0
     - The month's job candidates kept (9): [business] candidatesFixed=0

   Business automation (9 below)
     - Handing businesses to the mod opens once your household has listed a company of its own (since 1.4.0). A
       listing needs the game's unlock of "Rivals, Influence and Policies" (net worth $1M times the price index)
       first. Until then a click that would hand something over is refused, and the game's two switches do only
       what the game does with them. Whatever a save had handed over is taken back when it is loaded (as your own
       click would take it back), and the monthly summary names the businesses.
          Open from the start: [business] automationEarly=1
     - Until you hand something over, the mod buys, hires and signs nothing in a business. Handing over is done
       business by business, inside the game:
          - the game's two switches ("automatic staff management", "buy worn-out assets again"): where they are
            on, the mod's rules are added (hiring and letting go, buying assets that are missing);
          - Shift-click on an advert icon: the adverts; Shift-click on the contract icon: signing contracts;
          - Ctrl+Shift-click on an advert icon: the whole business. The game's "buy inventory" is switched on too
            (since 1.5.0), and a new business nobody works in yet is started by the mod, its first people first
            (since 1.5.0).
       The same click takes it back. What is handed over costs a fee (the list near the end, "What the mod does
       in your place, its switches and what it costs").
     - A business handed over whole is locked against your own changes (hiring, letting go, assets, contracts)
       until you take it back.
          Not locked: [business] lockHandedOver=0
     - A save from before the mod: at its first load the game's two switches are switched off in every business,
       and the monthly summary says so. Switch on again what you want the mod to do.
          Left as they are: [business] switchesOffOnFirstLoad=0
     - No automation at all: [features] business=0 (all of 9 goes, the cost of an effective hour in the hire tab
       too). The switches that turn off one kind of automation alone are in that list.


How you work the mod inside the game (at a glance)
--------------------------------------------------

The mod makes no window and no button of its own. You click what the game has while holding a key, or you use a
switch of the game as it is. A click with a key held, and the new order of a sort button, are also in the hover
text of the thing you click (as the game's own Ctrl-click is).

   A click with a key held (what the mod adds)
     - Shift-click a company in the stock window's list: its research subscription on and off (6 below).
     - Ctrl+Shift-click any company in that list: the subscription of every listed company on, and all off
       again.
     - Shift-click one of the advert icons in a business window: that business's adverts are handed to the
       mod, and taken back by another one (9 below). While the mod has them the icons of the paid adverts are
       orange and a plain click on one switches nothing.
     - Shift-click the Contracts icon in a business window (left of the word "Contracts"): signing that
       business's contracts is handed to the mod, and taken back by another one ("Signing contracts handed to
       the mod" in 9 below).
     - Ctrl+Shift-click one of the advert icons in a business window: the whole business is handed to the mod
       (staff, assets, adverts, signing contracts, more floor space when it runs short), and all of it is taken
       back by another one ("A whole business handed to the mod" in 9 below). While it is handed over, the
       clicks that change that business (hiring, letting go, buying and selling assets, accepting and giving up
       contracts and a few more) are locked.
   None of this takes a click the game uses: the game's Ctrl-click (add a shortcut) and right click are as
   they were.

   A switch of the game, used as it is (you click it as before; the mod does more behind it)
     - The round-arrow icon in the staff tab (automatic staff management): a click on anybody's switches
       everybody of that business. Staff with it on also get the mod's staff automation (9 below).
     - A business's "buy worn-out assets again": while it is on the mod buys missing assets too, and what your
       cash does not cover becomes a debt ("Missing assets bought" in 9 below). The hover text of this switch
       says so under the game's own sentence, in lines that begin with "Mod:" (from 1.1.2 on).
     - A save from before the mod: in it you switched these two on for what the game does with them. So the
       first time the mod loads that playthrough it switches both off in every business and says so in the
       monthly summary: "A save from before the mod: ...". The line across the top of the screen points
       there: "A save from before the mod: the mod changed things. See the Monthly Summary". Switch on again
       the businesses you want the mod to look after. A save you write after that keeps its switches. An older save, from
       before that first load, is switched off again whenever it is loaded (a business you have handed over
       whole since then is left out). To keep the switches as the save has them, set switchesOffOnFirstLoad=0
       under [business] in the settings file. A playthrough played with an earlier version of this mod is left
       as it is.

   A button that does something else now (a plain click)
     - The second, third, fourth and fifth sort button above the stock window's company list: the largest
       market capitalisation first, undervalued first, overvalued first, the largest rise in a month first.
     - The second sort button above the list of the futures tab: the largest "12 months' expectation minus the
       rate now" first.
     - The time button at the top right of a chart: 6 months -> 1 year -> 2 years -> 5 years -> the most.

   The rest (loan rates, the trade lock, the fixed month end, the casino, fixed candidates and so on) works by
   itself once it is on. It is switched in RealisticEconomy.ini; the table is at the end, under "What the mod does
   in your place, its switches and what it costs".


What it does
------------

1. Loan rates by credit grade
   A loan rate becomes "central-bank rate + spread". The spread follows the household's credit grade
   (AAA, AA, A, BBB, BB, B) and its mortgage loan-to-value. Four ratios give the grade:
     - debt other than mortgages / assets other than homes
     - yearly debt payments / yearly income
     - liquid assets / yearly expenses
     - net worth
   Yearly debt payments are the instalments every loan asks for over the next twelve months (fewer for a loan
   that ends before that), plus once every debt that is paid in one sum three months on (arrears, tax).
   The grade is shown next to the rate in the bank's personal loan and mortgage windows and in the tooltip of a
   variable-rate personal loan or mortgage, as in "Interest rate (credit grade A)".
     - The grade is taken again after a load, at every month end, and whenever a loan or a home is added or removed.
     - Variable-rate loans follow the grade at once, including the ones you already have.
     - A new fixed-rate mortgage gets its rate at the moment the home is bought and keeps it.
       Fixed rate = variable rate + 0.5 points + 0.5 x (7.5% - central-bank rate).
       Fixing is dear while rates are low and cheap while they are high.
     - A new education loan is fixed at the central-bank rate of the month you enrol.
     - Fixed-rate loans you already have are not touched.
   Two holes of the game are closed (since 1.2.0).
     - A mortgage up to its approval ([credit] mortgageUpToApproval): the mortgage window approves a loan of four
       times the deposit. The purchase then made the whole price less the deposit the loan and took no cash for
       the price: with a small deposit and cash at hand a property was bought on nearly 100% credit (in the test
       save a deposit of $303,511 and an approval of $1,214,044 bought a warehouse of $6,350,911 with a loan of
       $6,047,400, and the cash stayed). With the mod the loan is the approved amount and the rest is paid in cash
       at the purchase. mortgageUpToApproval=0 leaves the game as it is.
     - A share rule of the Monthly Auto Transfer needs the cash ([credit] transferNeedsCash): a rule that buys
       shares (the bank's "Manage Auto Transfers") bought them whatever the cash was, and what the cash did not
       cover became a debt without interest, due after three months (in the test save $2,324,604 of cash bought
       shares for $19,025,875, and $16,701,271 became debt). With the mod a rule buys in a month only when the
       cash covers the price and the fee; when it does not, the monthly summary says "Auto transfer: 1 stock
       purchase(s) skipped (not enough cash)". A rule for the savings account was skipped by the game already
       when the cash is short. transferNeedsCash=0 leaves the game as it is.

2. Month-end cash forecast
   The monthly summary panel (the window that opens when the month changes) gets one line at the top:
   "This month end: cash needed $X". Loan payments are exact; wages, rent and the other month-end costs are what
   was really paid at the last month end. So in the first month with the mod the line names the loan payments only.

3. Trade lock after a load
   After you load a save from before a month end you have already passed, buying shares, selling shares and
   futures trades are refused until that month end has been passed again. This stops profits from prices seen in
   advance. A save made after the farthest month end you have passed never locks anything.
     - The game's own end-of-month autosave is written just before the month end is processed. Loading it locks
       trades for one hour, until that month end is over; the auto transfer of that month end buys as usual. The
       summary panel does not mention this hour. An autosave written while a lock was running is different: after
       loading it the auto transfer is refused too and the panel shows the lock, because a transfer rule made
       during a lock would buy at prices already seen.
     - While locked, the summary panel shows "Trade lock: another N hour(s) (stocks, futures)".
     - If you do not want this feature, switch it off: see "Switching features on and off" below.
     - A lock lasts at most 2 months from the load. If you went further ahead than that, economic growth, the
       industries' cycles and share prices all take another course from the load on. Futures settle against growth
       and the cycles, so what they pay changes too. Loading the same save again gives that same course.
     - Stock purchases of the auto transfer are skipped during a lock as well, except after an end-of-month
       autosave that was written with no lock running.
     - If you went ahead by mistake: set release=1 under [guard] in RealisticEconomy.ini and load. The record of
       that playthrough is cleared and the value goes back to 0 by itself.

   A month end that a load does not change ([guard] monthEndFixed, on by default)
   At the month end the game draws the month's economic growth, the industries' cycles and the new share prices from
   random numbers. Other things use up the same numbers: a search for a property, opening the "create a public
   company" window, an activity with a random payment. So if you load a save, do one of them and then pass the
   month end, the same month ends with other share prices. With shares or futures in hand you can repeat that until
   a month you like comes, and the trade lock does not notice, because nothing is traded. In a test save with
   shares worth $7.8 million one such try moved their worth at the month end by $50,000 to $240,000.
   The mod makes this part of the month end independent of how many random numbers were used up before it. The
   same month of the same playthrough ends with the same growth, the same cycles and the same share prices,
   whatever save you load and whatever you do first.
     - The listing price too. Within a month the window gives the same listing price for the same set of businesses
       however often you open it, and the listing gets the price the window showed. The game itself draws anew
       every time the window opens and once more at the listing, so the listing price was about 1% off what the
       window showed.
     - The properties of a new month too. When a month starts the game draws the lists of properties for sale and
       for rent anew, and that depended on how many random numbers had been used up before as well, so a load could
       be repeated until a property far under its value was in the list. Now the list of a month is the same
       whatever you do first. Every site is drawn by itself: a search for a property makes new sites, and the
       properties of the other sites stay as they are.
     - The offer a month brings for a property you have put up for sale, and the tenant's offer for one you let,
       are the same within a month as well.
     - The course changes in one case only: when you went further ahead than the lock's limit of 2 months and came
       back (see above).
     - Not stopped: buying shares, seeing them fall that month, and going back to the save from before the
       purchase. That only undoes the purchase, and after the load the lock keeps you from buying anything else
       until that month has passed again.
     - To switch it off: monthEndFixed=0 under [guard]. guard=0 switches it off as well.

4. Creating a public company (IPO)
   In the game itself a listing leaves you 25% of the shares and no cash. With the mod you keep 45% and sell the
   other 55% at the listing price, for cash.
     - The window in which you pick the businesses shows "Retained Shares (45%), the rest sells for $X".
     - That cash is taxable income of the year, like the price of a business you sell. The tax is worked out at
       the end of the next June and leaves your account in one sum three month ends later.
     - The listing price comes from the game's own share-price rule. For a business that makes a profit it is
       never below the value you put in (what the business would sell for).
     - The window's "company quality" letter tells how many times the value you put in (per share) the public
       pays: AAA above 3, AA 2.5, A 2, B 1.5, C 1.2, D above 1, otherwise E. A business reaches these when its
       yearly profit is above about 23%, 18%, 14%, 10%, 7.5% and 5% of that value.
     - After the listing, price and dividends are the game's own. The game pulls a listed company's earnings
       toward 5% of its equity and the price follows the earnings. So the value of your 45% comes down to about
       the company's equity within a year or so, and the higher the letter, the further it has to fall.
     - 45% is the most the game lets one holder own. [ipo] takes 5% to 45%.

5. A board seat for a large shareholder
   Every 20% of a listed company your household holds guarantees one of the five seats on its board; 40% or more
   guarantees two.
     - You have to nominate a household member in the board tab of the stock window in the month an election is
       open. No nomination, no seat.
     - With more nominees than guaranteed seats, a nominee who would be elected on the game's own score anyway is
       elected and uses up none of the guarantee. The guaranteed seats go to the others: sitting members first,
       then nominations in the order made. The rest compete with the game's own score. Example: 20% held, two
       nominees, one of them good enough to win a seat: both are seated.
     - A company's first election opens the month after its listing, then every 24 months. It is decided at the
       following month end.
     - A company you list yourself starts with its election notice on: the month whose summary says
       "... Board Of Directors election is happening now" is the month to nominate. The check box in the board
       tab switches the notice off. With the listing feature (4) switched off the notice is not set either; tick
       the box yourself then.
     - Sitting members are put up again by the game at the next election. The seat is guaranteed again if your
       holding still qualifies then.
     - A member's experience score is not changed. An inexperienced director makes the company's equity
       write-downs a little more likely, as the game's rules have it.
     - The game holds no board elections before the influence menu is unlocked.
     - Two nominees with the same score (a bug of the game, closed since 1.2.0): the game seats everybody whose
       score is one of the five best. Two scores that are equal to the last digit share one place, which makes six
       directors, and the game ends the moment that company's board tab is opened (it did in the test). Two members
       of your household without experience, nominated together, have equal scores. The mod lowers the score of
       the later one of the list by the least a number can be lowered. A save that has a board of six already is
       not repaired.

6. Ratios in the stock window
   The stock window says what a company's numbers mean. The game's price rules are not changed.
     - With a company selected, the amounts of its first tab carry a ratio. Shown without "Research Stock":
         Equity Per Share      $243.98 (PBR 1.29)        price / equity a share
         Earnings Per Share    $15.84 (PER 19.9)         price / earnings a share; "(-2.4% of equity)" for a loss
         Dividend Per Share    $4.79 (1.5% of the price) dividend yield
         Share Price           $392.18 (+17.9% in a month)   the price against the month before
     - Shown only for a company you have had researched at least once:
         Share Price           $392.18 (+17.9%, fair $519.40)   the fair price behind the change
         Agriculture           --- (-$2.17 a month)      an industry row, as of now and not of the research
     - The industry rows are part of the game's research result. The game shows the value of the month of the
       research as a run of + or - and leaves it until you research again. The mod puts today's value there and
       an amount behind it: "(-$2.17 a month)" means that this industry takes $2.17 off the company's yearly
       earnings a share at every month end; with a + it adds. Rising prices in an industry help some companies
       and hurt others (it differs from company to company).
     - Hovering over a company's icon in the list gives one line without selecting it: "PER 19.9, PBR 1.29,
       yield 1.5%". Under it comes "-2.5% on the month before" (today's price against the price the game wrote
       down a month ago; a company listed this month has none). A company in danger gets a warning line, a researched one the line "Fair $110.60 (-16%),
       industry headwind": the fair price is 16% under today's price, and "industry tailwind" or "industry
       headwind" says which way the industry rows push the earnings together. A small sum gets no word.
     - The order of the list: above the company list are five small sort buttons. Four of them sort another way.
         first    by value owned              as in the game (what you hold comes first)
         second   by market capitalisation    every company, the largest (price x shares) first
                                              (the game: by listing date, which put nothing in order)
         third    undervalued first           researched companies, the one furthest under its fair price first
                                              (the game: by company type)
         fourth   overvalued first            researched companies, the one furthest over its fair price first
                                              (the game: by market capitalisation)
         fifth    by change in a month        every company, the one that rose most first
                                              (the game: by earnings a share)
       The third and the fourth rank only the companies whose fair price is shown, the ones you have had
       researched. The others come under them in the order the game lists them; with no researched company the
       list stays as it is. With the fifth a company listed this month comes last, and the companies that fell
       most are on the last page. The hover text of a button says its order. A second click on a lit button goes
       back to the first order, as it always did. A row does not show the number it is ranked by; hover over the
       company for it. Off: [stocks] sortButtons=0 in the settings file (the game's four orders again).
     - A company's chart (the last tab of a company, the line-graph picture): the game draws the share price,
       the equity, the dividend and the earnings a share on one scale, so dividend and earnings lie flat on the
       bottom and the price is squeezed into the top. The mod opens the chart with the share price alone. The
       boxes on the right bring the others back, and the box for all of them ticks the four. Off: [stocks]
       chartPriceOnly=0.
     - The economy's chart ([stocks] chartEconomyOne, on by default): a click on the numbers left of the cash
       account, at the top of the screen, shows four small buttons. Three of them - economic growth, the central
       bank's rate, inflation - open the same chart in the game: seventeen lines (the three and the inflation
       rates of fourteen industries), in which nothing can be made out. The mod opens it with the line of the
       button you clicked alone. The boxes on the right bring the others, the box for all ticks the seventeen.
       The fourth button (the stock market) is under "What a chart opens with" below.
     - Three things for every chart ([stocks] chartFixes, on by default):
         · Colours: the game picks dark greens and browns by chance whenever a chart opens, and the lines are
           hard to tell apart. The mod hands out a fixed list (red, blue, green, orange, purple ...). The text
           of a box has its line's colour.
         · Ticks kept: a chart with more than eighteen lines has its boxes on pages (the arrows below them),
           and the game ticks every box again whenever the page is turned. With the mod what you unticked stays
           unticked, and the box for all is for the lines of every page, not only the page shown.
         · The time button at the top right of a chart goes 6 months -> 1 year -> 2 years -> 5 years -> the
           most -> 6 months (the game: 2 years -> 5 years -> the most).
     - The chart of all share prices (the line-graph button above the list; [stocks] chartIndex, on by default):
       the game draws a share of $16 and one of $700 on one scale, so the cheap ones lie flat and nothing can be
       compared. The mod shows every share against its price in the first month of the picture, as 100: 130 is
       30% up since then, 80 is 20% down. The title gets "(first month shown = 100)", and the numbers at the
       side and under the mouse are that index, not dollars. The time button moves the first month. A company
       listed inside the time shown is 100 in the month it was listed. chartIndex=0 gives the game's prices.
     - What a chart opens with ([stocks] chartOpenWith, on by default): four charts open with the lines that
       are read most. The boxes or the box for all bring the rest.
         · The household's wealth (the last tab of the finance window): net worth, cash and debt. The game draws
           nine lines on one scale (those and investments, real estate, stocks, business, items, savings).
         · The stock market (the fourth button at the economy's numbers): the growth over a year. The growth
           of the month jumps about and hides it.
         · All share prices: the companies you hold shares of. When you hold none, or when none of them has
           its box on the first page of boxes, all of them as before.
         · Growth, the central bank's rate, inflation: the last five years (the game: twenty). The time button
           changes it.
     - The width of the lines ([stocks] chartLineWidth, 2 by default): the game draws a line one pixel wide with
       soft edges, so it is paler than the colour of its name. The mod draws it as thick as the chart's axes.
       The number is half the width. 0 gives the game's line.
     - A value behind every name ([stocks] chartValues, on by default): the name next to a box is followed by
       its line's value in the last month, "Net Worth 49,099,094", "Share Price 283.25", so the numbers can be
       read without pointing at a line. In the chart of all share prices this is the price itself, also when
       the picture is the index.
     - What the mod does not put right in the charts: a chart with many lines draws only the lines of the page
       of boxes that is shown (eighteen; the game's limit). There are no lines across and no numbers between
       the lowest and the highest. An earlier stretch cannot be looked at by itself (it is always "the last N
       months").
     - The fair price is where the game pulls the price at every month end: a mix of the equity a share and 20
       times the earnings a share, with more weight on earnings the higher the return on equity. A month end
       closes about 20% of the gap between price and fair price, with noise of about 10% of the gap. Earnings
       themselves swing a lot from month to month, and the fair price moves with them.
     - A price under 0.75 times the equity a share (PBR 0.75) at a month end makes the game downsize the company
       (its equity a share is cut by about 20%) or have it issue new shares (your holding is diluted). If the
       price is under that line again at one of the next five month ends the company is bankrupt and its shares
       are worth nothing. During that time the equity row is red, "(PBR 0.81, bankruptcy watch)", and the hover
       text reads "Within 4 mo.: bankrupt if the price falls 7.4%".
     - Shares you hold in such a company are named in a line near the top of the monthly summary: "Shares held,
       warning: Name (price -0.5%: downsizing or new shares at month end)". The line comes when a fall of 10% or
       less reaches the 0.75 line, and in the six months after a downsizing or share issue when a fall of 25% or
       less does. The three most pressing companies are named, the rest counted ("and N more"). The game itself
       tells you only after it has happened.
     - The numbers are those of the moment you look. The risk row of the game's own "Research Stock" result
       ("Liquidity Risk") stays as it was in the month of the research.
     - The futures tab: the asset's inflation rate reads "4.30% (exp. 5.05%)". A quote assumes that today's
       rate stays as it is until the expiry, and a contract is settled at the price the asset really has then.
       The game moves every industry's rate each month by a fixed rule (toward economic growth, and back when
       the industry's prices have run away from prices overall). "Exp.", expected, is what the rate averages
       until the chosen expiry (6, 12 or 24 months) if it follows that rule. Above today's rate a purchase is
       ahead, below it a sale. Chance moves a rate by about one percentage point a month, so this is no
       certainty. The hover text of the number says the same.
     - The list of the futures tab ([stocks] futuresList, on by default): the name of every asset is followed
       by two numbers, "Office Supplies (+4.3% → +5.1%)". The first is its inflation rate now, the number the
       right-hand side shows once you click the asset; the one behind the arrow is what the rate is expected
       to average over the next 12 months. The second of the two sort buttons above the list sorts by "the
       12 months' expectation minus the rate now", the largest first: at the top the asset whose rate is
       expected to rise most (a purchase is ahead), at the bottom the one expected to fall most (a sale is
       ahead). The game's order there was by what a unit costs, which says nothing about a contract. The
       button's hover text says what it sorts by. The first button, by the value of your trades, is the
       game's own. The hover text of a row has the rate and what it is expected to average until 6, 12 and 24
       months, so you can compare assets without clicking through them. An asset with no industry icon under
       its name (one in the test save: cleaning supplies) has no industry to work the expectation out from:
       it shows the rate alone, "(+2.5%/yr)", and the order takes it for one that stays where it is. Off with
       futuresList=0.
     - Research kept fresh: the game's "Research Stock" takes five of your hours for one company, and its result
       stays that of the month it was done in. The mod can have the game do that research again whenever a new
       month starts.
         · A company with a member of your household on its board is researched again for nothing (on by
           default, [stocks] researchOnBoard).
         · Research subscription (off to begin with, switched in the game): a subscribed company is researched
           again every month, for "5 hours of a bank analyst at the standard wage" a company a month from your
           cash account (the game's research takes 5 of your hours, so it is that much of a professional's pay:
           about $335 a company in a save of 2035, rising with the game's wages; the hours are [stocks]
           researchFeeHours).
         · The switch: the list of companies on the left of the stock window.
             - A Shift-click on a company subscribes that one company; another Shift-click ends it. A company
               you hold no shares of can be subscribed too.
             - A Ctrl+Shift-click on any company subscribes every listed company, whether you hold its shares
               or not. Another Ctrl+Shift-click ends all of it, the companies you subscribed one by one too.
               While it is on, a company that is listed later is included.
             - Switching on works at once: the company is researched and the month's fee is taken then, and the
               result is in the list and on the company's page right away.
             - Switching off and on again within a month is paid once. A company that has this month's research
               already (one you subscribed and ended this month, or one you researched yourself with "Research
               Stock") costs nothing when you switch it on; its fee starts with the next month.
             - When your cash is less than what would be due at once, nothing is switched: no research, no
               fee, and the top of the screen says "Research subscription: not on, your cash is short of the
               fee ($6,406,080.00)".
             - Switching off: no fee from the next month. This month's fee is not given back and this month's
               research stays.
             - Whether a company is subscribed is written on the screen. In the list its name is followed by
               "subscribed" (see "The rows of the list" below), and the line "Last Researched" of its page
               reads like "Oct 35 (subscribed)". A company you sit on the board of has "board" and "(board:
               free)".
             - The page of a company the mod has researched this month (subscribed, or on whose board you sit)
               has no "Research Stock" button of the game's: it would take five of your hours for a result
               that is there already. The button is back when the subscription ends.
             - A fee once paid does not come back with an earlier save ([stocks] researchFeeKept, on). The
               subscription belongs to the playthrough; the result and the fee you paid are in the save. A save
               from before the fee therefore has the cash back while the fair prices you read still hold (share
               prices run the same course again). To close that, right after a load the mod researches again
               every company whose fee of this month was paid before, and takes that fee again - also when you
               ended the subscription before loading. The same at a month start that comes a second time: a
               company whose fee of that month was once paid is researched and paid for, subscribed or not.
               The monthly summary says "Research fee paid before: loading an earlier save does not bring it
               back. 24 companies paid for this month were researched again, fee $8,007.60", and a line passes
               at the top of the screen. Cash that is short becomes a debt, as with the month start's fee.
               Not asked for: after a rollback beyond the trade lock's cap (two months), which changes the course
               of share prices, the months paid before it (what you read no longer holds); and when the fee is
               set to 0. The mod remembers the last 24 months of a company.
             - A subscribed company that has no research of this month yet keeps the game's "Research Stock"
               button (a playthrough started with researchSubscription=1 before its first month start, or an
               earlier save loaded with researchFeeKept=0). The mod researches that company and takes the fee
               when the next month starts. To see the result sooner, research it with the button, or end the
               subscription and subscribe again (which researches and takes the fee at once).
             - Each click shows a line for a moment at the top of the screen, under the date, like "Research
               subscription: (the company) on. Researched now, fee $333.65 (every month)". The hover text of a
               company ends with two lines, where it stands and what to click ("Shift-click: subscribe to
               research", "Research subscribed (Shift-click: off)", "Research subscribed (every company)", "On
               the board: research is free", and "Ctrl+Shift-click: subscribe to all" or "Ctrl+Shift-click: all
               off").
             - A Shift-click also shows the company on the right, as a plain click does. The game's own
               Ctrl-click (add a shortcut) is as it was.
             - A company you sit on the board of is researched for nothing whatever the subscription; a
               Shift-click does not change that.
         · When a month changes every subscribed company is researched again and the summary panel gets a
           line, "Stock research subscription: 24 companies researched again, fee $8,052.00". Then the research
           is done and the fee charged whatever your cash is. What the cash does not cover becomes a debt of
           the game's, the kind it makes of wages or adverts you could not pay: no interest, due at once after
           three months. It is in the debts list of the finance window as "Financial Trading Fees", and the
           summary line ends with "your cash was short, so $84,128.41 of it is now a debt". If the cash is not
           there in the month it is due either, the game's rules take over: the game turns an unpaid debt into
           a new one of 120% with a deadline, and a household that misses that goes bankrupt. In a bankruptcy
           the game sells businesses and property under their market value and pays every debt, this one too,
           at the amount it stands at.
         · What is subscribed is kept by playthrough in RealisticEconomy.state, not in the save. [stocks]
           researchSubscription in the settings file is whether "every company" is on in a playthrough where
           you have never switched it with a Ctrl+Shift-click (0 by default: off). A playthrough that starts
           with 1 is researched from the next month start on.
         · The rows of the list ([stocks] rowNumbers, on by default): in the list on the left of the stock
           window a company's name is followed by numbers in brackets. "Gold (+5.7%)" says the share price is
           5.7% above last month's, and it changes every month. A company you have researched reads "Gold
           (+5.7%, fair +34%)": how far the fair price is above today's price (below zero: the price is above
           the fair price), and its name is blue. A subscribed company has "subscribed" at the end, one you
           sit on the board of "board". The company's name itself is not changed, only how this list shows
           it. The game draws a long row in a slightly smaller type. Off with rowNumbers=0.
         · The fee is deducted when the tax is worked out (it is booked like the game's own financial fees).
         · The fee is booked when the new month starts, after the month's receipts are in the cash account. The
           line "cash needed at this month end" does not have it.

7. Mistranslations
   Puts right texts of the game that say something else than the English original. Most are Korean. In other
   languages only what shows at a glance: the Spanish "Corregido" for a fixed interest rate becomes "Fijo", and the
   French line for an industry that is busting, which was the line for a booming one ("en plein essor"), says
   "en crise".
   Every language: the monthly summary's lines for a stolen and a recovered item start with the name of the
   home or the business premises the item was kept in. A household without a home (its lease ran out and no new
   home was taken) has no such name, and the game wrote "??:Custom Part stolen due to lack of floorspace" and
   ":Custom Part recovered by police". These now read "Custom Part stolen: no home or premises to keep it in" and
   "Custom Part recovered by police", in the game's language. While the household has no home, the items it kept
   at home are stolen month after month.
   Words that came out as they stand in the language file. The game puts a value into a text where the text has
   the value's name ("MONTHS"). Some language files spell a name differently or leave a brace out, and the game
   then shows the bare word. The mod writes these texts with the value in them:
     - Spanish: "MESm" for a number of months ("3m"), a pay demand that began with a brace, the name of a
       completed education ("EDUCACIÓN"), the second person of a compatibility line
     - German: the amount of a new loan ("GELD"), the months of a lease renewal ("MONATE"), the person of the line
       about friends turned down by the filter
     - French: the number of new shares of a capital raise, the months in the window that confirms a tenant's lease
     - Brazilian Portuguese: the number of education loans allowed, the address in a tenant's lease that ends
     - Japanese: the hours of an earlier qualification ("X"), the number in "maximum passions", the date somebody
       can join again
     - Simplified Chinese: the name of a newborn, the currency in two chart titles
     - Korean: the date in "cannot trade futures after a bankruptcy", the age in the title of the relationships
       window
   Japanese only: nine texts of the birthday window each stood one place too early (the birthday greeting had the
   sentence about health points, and so on). Each now has its own sentence.
   These were found by comparing every text of every language file with the English one. Not corrected: the
   Japanese text shown when a child comes of age, which names the age as "YEAR".
   Korean only: the four lines about a listed company that goes bankrupt, takes an emergency measure or is newly
   listed are written again (the game puts one particle behind every company name and translates word by word).
   Since 1.2.1 also:
     - Korean: three lines of a business's month that the game has in English (a pay demand, a member of staff who
       leaves for personal reasons, the heading "S-Con auto purchase:" of what "buy worn-out assets again" bought).
     - French, German, Chinese: that heading, which read "car purchase".
     - Japanese: the line of a bankrupt company's shares, which left out the number of shares and named the company
       twice.
     - German, Japanese, Chinese: the news of a new currency (a redenomination). The Japanese file had the Chinese
       sentence and the Chinese file the Japanese one, and the German and that Chinese sentence left a "VALUE"
       behind the number.
     - The title of the month's summary. Korean "10월, 2035 말" (the English order) becomes "2035년 10월 말", German
       "Beenden von Oktober" ("quit") "Ende Oktober 2035", French "Fin du Octobre" "Fin Octobre 2035". In
       Japanese and Chinese a second "月" followed the month's name ("10月月末", "十月月底"); now "10月末", "十月底".
   Since 1.3.0, in Korean, more than three hundred names and tips of the game that are wrong or read strangely:
   places ("학년 학교", "grade year school", becomes "초등학교", primary school; "라이브러리", the loanword, the plain
   "도서관"), the city's policies ("Defund Transportation" read "교통비 환불", "refund of transit fares"), jobs
   ("Dishwasher" read as the machine), status effects, the bank's actions ("Withdraw" read "철수", a retreat), the
   words for needs and preferences, a number of months that read as the name of a month. The game reads them so from
   its language files; the files on the disk are not changed, and a text the game itself puts right later is left
   as the game has it. The other languages have not had such a review.
   In the other languages, since 1.3.0, the words for how much a person likes something: Spanish and Portuguese
   wrote a like as "Como" ("as", "I eat"), French as "Comme" ("as"), and Japanese had the like and the love one step
   off ("楽しむ", to enjoy, for a like; "好き", like, for a love). Spanish "Añadir" ("to add") for an addiction and
   Portuguese "Não Gostei" ("I did not like it") for a dislike are put right as well.

8. Casino
   The four casino games (slots, roulette, blackjack, baccarat) become games of chance with a set stake.
         game        stake         wins
         Slots       $1,000        22%: 3 times the stake,  0.8%: 40 times
         Roulette    $10,000       44%: 2 times,            2%: 4 times
         Blackjack   $100,000      45%: 2 times,            2%: 2.5 times
         Baccarat    $1,000,000    46.5%: 2 times
     - "3 times" means you get three times the stake back: with a stake of $1,000 your cash goes up by $2,000.
       Without a win it goes down by the stake. Over many games 98% of the stakes come back at slots, 96%, 95%
       and 93% at the others.
     - The stake does not depend on your net worth (in the game itself the amounts grow with it). It does rise
       with the game's price level, like every other price: in a save of the year 2035 slots cost $1,857. The
       amount the list shows is the stake.
     - Each game can be played once a month by the whole household. After that the game shows it as "Already
       done recently" for the rest of the month. The chances are the first line of the game's hover text:
       "Monthly. Wins: 22%: 3x, 0.8%: 40x".
     - The money moves once, when the action (20 hours) ends. Nothing is taken at the start. An action given up
       half-way (right click on its entry in the queue) moves no money and has no result, and the game can
       still be played that month. In the game itself the result came in the first hour, so one could look at
       it, give the action up and play again at once.
     - A result belongs to "this playthrough, this game, this month". Saving just before the end and loading
       again and again gives the same result.
     - A game you have played is not undone by loading an older save. What the games after that save won or
       lost is put into your cash right after the load, with a line in the summary: "Casino: 2 game(s) played
       before this load still count (cash -$20,436.58)". Those months' games stay played.
     - If you do not want that rule, set keepResultsOnLoad=0 under [casino] in the settings file. Games played
       after the loaded save are then forgotten (a lost game can be undone by loading). Stakes and chances stay.
     - A save from before the mod with a casino game in the action queue: the first time the mod loads that
       save it takes the game out of the queue. The game's own way has no stake and gives the result when the
       game begins, which is not the mod's rule. The monthly summary says so: "A save from before the mod: ...
       2 casino game(s) taken out of the action queue ...", and the line across the top of the screen points
       at the summary.
         · A game that has not begun: it is gone from the queue. Queue it again if you want it; the mod's stake
           and rules apply then.
         · A game that had begun: its result is what the game gave when it began (it is in your cash already).
           You get the remaining hours back, and the game counts as played this month; next month it is free
           again.
         · It is taken out the way a right click on its picture in the queue takes it out. No money moves.
         · To leave the queue as the save has it, set switchesOffOnFirstLoad=0 under [business] in the settings
           file (the same setting as for the two switches above). A game that had begun then ends without
           money moving, with the line "Roulette: begun before the mod ...", and one that had not begun is
           played by the mod's rules.
         · The queue of a playthrough played with an earlier version of this mod is left as it is.
     - Winnings are not taxed and losses are not deductible. They do not count as income for the credit grade.

9. Businesses: what an hour of work costs with an employee
   When you hire for a business you own, the mod shows who does the same work for less, not who has the lowest wage.
     - Cost per effective hour = hourly wage / work efficiency. Someone at $64.84 an hour and 114% costs $56.88:
       what you pay that person for the work a 100% employee does in an hour.
     - In the hire window every candidate reads "Conduct Interview ($56.88 per effective hour)" and the list is
       in that order, cheapest first. The line above the list says so.
     - The hover text of a candidate's or an employee's wage icon has the same number and how it comes about.
     - The hover text of an offered contract's payout icon gets two lines: "+67.9% over standard cost", "$36.22
       left per hour": how much more the payout is than the standard cost of the work, and what is left for
       every hour worked. They are for telling which of several offers adds the most.
         · The standard cost is what the contract's work costs in a month at standard wages: the hours of the
           jobs it lists times each job's standard wage, the work it sets off in other jobs of the business
           (which the contract does not list), materials and utilities. Your own staff and their wages are not
           in it.
         · The game adds 65% to the labour and 35% to materials and utilities, then 0.23% for every month beyond
           24 and about 4% for every 10 points of reputation. So an offer heavy in materials has a lower
           percentage and a long one a higher.
         · The second line is the monthly payout less the standard cost, over the hours worked in that month. It
           can be the larger one where the percentage is lower (jobs with a high wage); when staff hours are
           short, look at this one.
     - A load does not change the candidates.
         · The game decides the candidates with its random numbers at the moment the hire window is first
           opened, so who comes and what they ask depends on what was done before (which is why loading a save
           and looking again gave other people). With the mod the candidates of a job in a month depend on the
           playthrough, the month, the business and the job alone: the same people whenever you look and
           whatever save you load ([business] candidatesFixed). They change when the month changes, as in the
           game.
     - Apart from that the candidates' skills and wages and the contracts themselves are not changed. The one
       thing changed in a contract is a line of the data that is wrong (see "The civil engineer of a construction
       contract" below).
     - Work efficiency alert: when the month changes (and right after a load) and a business loses work
       efficiency because assets are short, the summary panel gets a line: "Work efficiency down: S-Air (assets
       80%)". It has the same cause as a red number under 100% next to the furnishings icon of the business
       window (with several assets short at once that number reads lower than the line's).
         · "furnishings" applies to every job of the business (never below 50%), "assets" is the value of the
           worst job (never below 35%). Each multiplies the work efficiency of the staff.
         · They fall without an asset being lost: more staff, a new contract or more awareness raise the hours
           the game expects, and with them the assets it wants.
         · The three worst businesses are named, with " and 2 more" behind them when there are others. What to
           buy is in the game's hover text of that furnishings icon. Off with [business] efficiencyAlert=0.
     - A business type's month (since 1.5.0): in the window that picks the type of a new business, the hover text
       of a type's coin icon has under the game's "Estimated startup cost" a line such as "Mod: with all its
       customers, about $101,800.00 of sales and $68,300.00 of costs a month": what a business of that type takes
       in and pays out once all its customers come and its reputation is 100%, at today's prices in this city, to
       the nearest $100.
         · The numbers come from the game's own table of each type (data/statsFirm.txt: the hours of its work, its
           income, inventory, assets and utilities, rent). The sales go with the type's industry price index and the
           difficulty, the wages with this city's hourly wage of each job, the rest with the type's price index, the
           rent with the overall price level and the city's rent factor.
         · The sales are the ceiling with all the customers. A new business starts at an awareness of 25% and a
           reputation of 25%, so its first months earn much less. Against real businesses the number came out at
           0.8 to 1.1 times what they were paid.
         · The costs are the wages of the hours the work takes. Staff are paid their monthly hours whatever the
           work, so the real wages are higher (1.3 times in one business). The premises are taken as a rented one
           of the type's base size.
         · A type without sales of its own that lives on contracts (Game Studio, Security Contractor, Auditing
           Services, Building Construction) says "Mod: no sales of its own; it earns from contracts only."
         · Off with [business] typeEstimate=0.
     - Automatic management for a person you hire: the game's automatic staff management (the round-arrow icon
       at the top right of the staff tab) is switched on person by person, so somebody hired later is outside
       it until you click again. With the mod, a person hired through the hire window gets it at once when at
       least one employee of that business has it.
         · In a business where nobody has it the game is as it was: the new person comes in without it.
         · The switches of people already employed are not touched.
         · Off with [business] autoManageNewStaff=0.
     - The automatic management is a business's switch: a click on anybody's icon in the staff tab switches
       it for every employee of that business (in the game that takes a Shift-click; [business]
       autoManageWholeBusiness). Everything the mod does about staff applies to employees who have it.
     - Staff filled in (jobs that have employees under the automatic management; [business] autoHire): in the
       game people work their month's hours off early in the month, so a job that is short of people stands still
       from about a quarter of the month on, with work left. Eight times a month (every 91 game hours, the first in
       the month's first hour) the mod looks at each job's "work still to do this month" and "hours its people
       have left".
         · Right after a load and right after a business is handed over whole (since 1.5.1, [business]
           autoHireNow) it looks once more, a game hour later, without waiting for the next 91st hour. A save just
           before a month end, or a new business nobody works in, no longer leaves its work undone for up to 91
           hours. In the user's save (55 hours before a month end) a new business that did none of its month's work
           left 27% of it, and started the next month at a reputation of 18.8% instead of 4.3%.
         · The other jobs' work counted ahead (since 1.5.1, [business] autoHirePlan): in the game the month's work
           of the base job (for Electrical Services, the electricians) is drawn at the month end for the coming
           month, and the work of the other jobs (apprentices, a bookkeeper) comes by chance while the base job is
           worked. The mod counts that work still to come from the base job's customers' work still to do, in the
           shares of the business type's table (Electrical Services: apprentices 144 to electricians 480), so that
           the month's first look hires for the whole month (the user's save: 41 apprentice hours counted from 137
           electrician hours; 27 came). When less comes, the people to spare go at the month end by the rule below.
           The work contracts bring the other jobs, a type the table does not have and a business whose base job
           is not the table's are hired for when the work is there, as before.
         · When the work is more, by at least 8 hours and by more than 5% of the job's month ([business]
           autoHireShort), a candidate is hired, and when one is not enough the next one too, there and then,
           until the work has hands or the month's candidates are used up.
         · The measure is "what an hour of the missing work costs": a candidate's monthly wage over the hours of
           the missing work that person gets done. With 12 hours missing the lowest wage is picked, with 300 the
           best rate.
         · A hiring costs hireFeeHours a person (20 hours of a store manager). There is no limit to how many;
           the candidates the job has that month are the limit (five for a job in the test save).
         · A business that is short of assets: in the game an hour of a person gets less work done the less the
           business has of its assets (down to 35% for the assets of the job, down to 50% for the furnishings of
           the business). The mod counts the hands with that. Where a job is short of hands and of assets, the
           assets come first: they are bought there and then (see "Missing assets bought" below; only for a
           business that has the game's "buy worn-out assets again" on), and people are hired only when the
           work is still short of hands after that.
         · A job that has none at all of one of its assets is not worked by the game. Nobody is hired for such
           a job (a hired person would stand idle). The line "Work efficiency down" of the monthly summary names
           it: "S-Con (a job stands still: one of its assets is missing altogether)".
         · A person hired like this who is one too many in the next month (as under "A staff that fits the work"
           below) is let go at that month end, without the wait of three months and without severance.
         · Near a month end (since 1.5.1): the game pays a whole month's wage at the month end for a person hired
           at any hour of the month (an electrician hired 55 hours before a month end was paid the month's $4,120.05).
           So when fewer game hours are left than a candidate's hours of a month, nobody is hired by this rule; the
           shortage goes to the comparison of "The last few hours" below (the reputation the hours keep against what
           they cost). When the job's present people do not cover its month's work (the person is needed next month
           too), a hiring costs one month's wage there: the hiring and next month's wage would come at the next
           month start anyway. Otherwise two months' wages and the hiring cost. The first person of a base job in a
           new business is still hired: the missing assets need that person's hours. With autoCover=0 hiring is as
           before.
         · A job you have outsourced yourself and a job without an automatically managed employee are left alone. One
           exception (since 1.2.0): in a business handed over whole, a job that a signed contract brings work for
           and that has no employee at all gets its first person from the mod. The hire window of such a business
           is locked, so nobody else could hire there.
         · Seen with a save of 2035: one person each for five jobs at first, and no work left undone in any of
           the five businesses in the six months after (before, one business left a tenth of its work every
           month).
         · Off with [business] autoHire=0.
     - The last few hours (since 1.4.0, [business] autoCover): what is short by less than the rule above (under 8
       hours or no more than 5% of the job's month), what is short when no candidate is left, and the work that
       comes in after the month's last look (with 91 hours left).
         · At a month end the game takes reputation from a business for the share of its work left undone and gives
           a bonus for a month with none (14, "the monthly bonus for completed work"). What a business is paid goes
           with its reputation (0.7 times at 0%, 1.1 times at 100%).
         · The mod puts a price on reputation: what the customers paid last month and what the signed contracts pay
           a month, a point of reputation moving about 0.4% of that, counted over a year as the reputation fades.
           What the customers pay is the larger of what they paid last month and what the business type's table
           makes of the customers its awareness brings (since 1.5.1; the game draws the next month's customers'
           work from the awareness at the month end). A new business was paid nothing last month, so before, its
           reputation had a price of $0 and its last hours were always left undone.
         · When the hours can be covered for less than that, the cheapest way is taken: the game's outsourcing (only
           for a job the business may outsource; an hour costs three times the job's standard wage, and it does one
           hour a game hour) or a candidate (two months' wages and hireFeeHours; one month's wage when the job is
           short of people for next month too). When both cost more, or there is
           neither, the reputation is lost.
         · In the month's last look a job the business may outsource is outsourced when the reputation an hour keeps
           is more than the hour costs. The game outsources only once the job's people have fewer than 11 hours
           left, so it costs nothing while there is no such work. Jobs the mod outsourced are switched back at the
           next month start.
         · Seen with the user's save of 2035: S-Con's Builder Apprentices were 4 hours short (a job it may not
           outsource). A candidate for $3,563 (two months' wages and the hiring cost) kept reputation worth $16,819,
           the month ended with nothing undone and the reputation went from 68.4% to 70.1%. In a test with far too
           few plumbers 112 hours were outsourced, for $18,664.80.
         · Off with [business] autoCover=0.
     - A staff that fits the work (employees under the automatic management; [business] spareMonths, 3): just
       before a month end the mod looks at the hours each person did not work that month. Nothing is forecast.
         · In a job of two or more: when the others could have done a person's work in the hours they left
           unworked and would still have had a tenth of their month free, that person is one too many. When
           there is such a person three month ends in a row, the one whose hour of work costs most is let go
           after the month end. The month is paid; there is no severance.
         · Somebody alone in a job who leaves half the hours or more unworked ([business] idleShare) three
           month ends in a row is replaced, when the job's candidates have a person of fewer hours and a lower
           wage who gets a quarter more done than was done that month. The hiring costs hireFeeHours; letting
           the present person go costs nothing. A job's candidates are looked at once in three month ends
           (spareMonths) and two jobs a month end at most; the other jobs have their turn at the next (the game
           takes about a tenth of a second to make a job's list of candidates).
         · A contract signed or given up: the game sets a month's work at the month start, from the contracts
           signed by then. So the month in which you sign or give up keeps its work, and the next month has the
           change. After giving up a contract the work is less from the next month, and the people to spare leave
           by the rule above, three month ends later (their wages are paid until then; a lower spareMonths makes
           it sooner). After signing one the work is more from the next month start, and the first look of that
           month (its first hour) hires for a job that is short of hands ("Staff filled in" above).
         · At the end of a month in which a contract was signed nobody of that business is let go or replaced,
           also when that month end is the third in a row. The next month, which has the new contract's work, is
           measured first. Otherwise a person would be let go and the same job filled again a month later.
         · Nothing is written into the monthly summary; RealisticEconomy.log has it. Off with spareMonths=0.
     - The pay rise an automatically managed employee asks for: the game's management says yes to every one
       (about 24% each time). The mod answers instead, at the end of that month, just before the game would let
       the person go for want of an answer.
         · It looks at the job's candidates of that moment (the list of the hire window; made then when you did
           not open that window in the month).
         · First the work the job wants from that place: what the person did that month and what the job left
           undone (hours x work efficiency).
         · For every candidate, "what an hour of the wanted work costs": the monthly wage over the hours of that
           work the candidate gets done. Hours nobody needs do not count, so a candidate with many hours does
           not look cheap for them. Of the candidates who get at least 90% of the wanted work done, the cheapest
           is compared with the same number at the wage asked for. When the candidate is cheaper by more than 3%
           ([business] wageRefuseMargin), the rise is not given: the employee leaves, without severance as the
           game has it, and that candidate takes the place at once, with the automatic management on.
         · Otherwise the rise is given. The wage rises from the same month as with the game's own yes.
         · A person who was one too many that month (as above) gets no rise and no successor ([business]
           wageNoRiseWhenSpare). It does not apply to a job of one.
         · Cost of the hiring: the business pays "20 hours of a store manager at the standard wage" (about $645
           in a save of 2035, [business] hireFeeHours; by hand it is an interview of 20 of your own hours). It
           is booked as staff costs of that business, so profit and tax fall by it. The game's own replacement
           of a person who resigns for personal reasons is as it was (a fee of two monthly wages).
         · Nothing is written into the monthly summary. The answers are in RealisticEconomy.log.
         · An employee without the automatic management still asks you in a window, as in the game. Off with
           [business] wageDemandRule=0.
     - The price of a worn-out asset bought again (a fault of the game put right; [business]
       assetReplacementCharged): with a business's "buy worn-out assets again" on, the game puts a new unit in the
       place of an asset that has worn out. The game then does not take the price but pays it to you. The mod has
       the price charged, as when you buy by hand. assetReplacementCharged=0 leaves the game as it is.
     - The civil engineer of a construction contract (a fault of the game's data put right; [business]
       constructionContractFixed; since 1.2.0): the contract "Supply Construction Site" of a Building Construction
       business should come with 60 hours of a Civil Engineer next to its 180 hours of a Builder (the same contract
       gives an electrician's or a plumber's business 240 hours). In the game's data file that line hangs on a
       wrong number, so the civil engineer's work never came and was never paid. The mod has the game read the
       line as it should be; the file on the disk stays as it is. Offers made after that list the civil engineer's
       hours and pay for them (in the test save an offer of 177 hours a month became one of 236, and its payout
       over 36 months went from $5,502,134 to $5,719,165). A contract signed before stays as it was signed. A
       business without a civil engineer has to hire one before it takes such an offer (the mod does that for a
       business handed over whole). constructionContractFixed=0 leaves the game as it is.
     - Missing assets bought (only a business with the game's "buy worn-out assets again" on; [business]
       missingAssetsBought): a business wants so much of every kind of asset, by its jobs, its staff and the work
       it expects (coffee makers, parking spaces, an excavator). With less, its work efficiency drops. The game
       buys again only what has worn out. Right after a month has changed the mod buys what is missing too, and
       during the month as soon as it sees a job short of hands whose assets are short ("Staff filled in" above).
         · The switch is the game's own: the business's "buy worn-out assets again" (the switch that takes 20
           staff hours a month). The mod buys only for a business that has it on, and charges nothing itself.
           Those 20 hours are the game's: at a month start it takes them off the month of one employee of the
           base job, once for this box and "buy inventory" together (20 with both on, none with both off). The
           mod's hiring counts a job's people by the hours they have left, so those 20 are out of its sum too.
         · What it buys, where several wares bring the same asset (a coffee maker, a fridge and a billiards
           table; an electric plane and a fuel plane), in this order:
             1) a kind the business has already. You chose it.
             2) else a kind your other businesses have; of several the one they have most units of.
             3) else the ware with the lowest "cost of an hour of one missing unit": (the price over the ware's
                life + what it uses up in an hour of electricity, fuel and maintenance at the day's prices)
                over what a unit brings. What a unit brings beyond what is missing does not count (a ware
                that brings six where two are missing is paid for six). So of an electric and a fuel plane the
                one whose power is cheaper in this game is taken. This is worked out among the wares your
                cash covers.
             4) when your cash covers none of them, the ware that costs least for one of that asset.
           Ties go to the lower price for one of the asset, then to the less floor space. The price is the
           game's shop price, and the purchase is made by the function the game uses when it buys a worn-out
           asset again. If you want another kind, buy one unit of it yourself: from then on 1) follows it.
         · When you choose yourself (since 1.3.1): in the window "Purchase Assets" that the shop icon at the start of a business
           window's asset row opens, the hover text of an item has "Cost per 10hrs $0.97 (price ÷ lifetime +
           consumption)" above its "Activity:" line. That is the sum of 3) for that one ware (before it is
           divided by what a unit brings), in the ten hours of the game's own utility line; the game shows the
           price, the life and the utilities apart. In the test save a fridge came to $0.97 and a coffee maker to
           $2.26. It is in that window only, not in the windows that buy for a home or a person.
         · It buys even when your cash does not cover the price ([business] missingAssetsOnDebt, on by default).
           What is missing the game books as a debt, the kind it makes of wages you could not pay: no interest,
           due at once at the third month end. The game says so itself at the top of the screen ("...Capital
           Asset...").
         · Mind this: if the cash is not there in the month the debt is due, the game turns it into a debt of
           120%, and a household that misses that one goes bankrupt. A desk or a coffee maker is cheap, but one
           excavator was $240,433.74 and an aircraft costs more. If you do not want to buy on debt, set
           missingAssetsOnDebt=0: a unit is then bought only out of the cash that is left once "cash needed this
           month end" (the forecast of 2 above) is set aside, and what could not be bought is named as "Assets
           not bought: S-Con Excavator (not enough cash)". Before the first month end with the mod, what a month
           end takes is not known yet; with this setting nothing is bought during that month, only from the
           next month start on. An asset you did not want can be sold again with a right click on its icon.
         · It does not buy a unit that does not fit into the floor space (past the limit the game rolls for a
           burglary every month end), nor an asset that no ware of the shop brings.
         · When not everything can be bought, it starts with what fills most of what is missing for the least
           floor space. At most 12 units a business at one time.
         · Two places say what happened. The monthly summary: "Missing assets bought: S-Con Excavator x1
           ($240,433.74 in all, $2,482.15 of it as a debt)", and for what was not bought "Assets not bought:
           S-Con Excavator (no floor space)". The line across the top of the screen: "Missing assets bought: 1
           for $240,433.74 (debt $2,482.15, due in 3 months)". That line shows its messages one
           after the other, so at a month change it can come late; its hover text lists the last twenty at once
           (the game's own feature). What was not bought is named every month until it can be bought.
         · If you do not want any of this, switch the business's switch off or set missingAssetsBought=0.
         · An empty notice of the game gets a text: after the game has bought a worn-out asset again it shows two
           lines at the top of the screen, and the second has no text and comes empty. The mod puts "S-Con: a new
           one bought, $40.96" there.
         · Off with [business] missingAssetsBought=0.
     - Adverts handed to the mod (switched in the game for each business; off to begin with): adverts (the icons
       next to the megaphone in a business window) raise awareness, which brings work, and cost money while they
       are switched on.
         · The switch: Shift-click one of the advert icons in the business window. The mod then looks after the
           adverts of that business, and a line like "S-Con: the mod looks after the adverts now" shows for a
           moment at the top of the screen, under the date. Another Shift-click takes them back. A Shift-click
           does not switch the advert under it. Whether the mod has them shows in the colour of the icons ("How
           you see it" below) and at the end of an advert icon's hover text.
         · What the mod does: every 91 hours (eight times a month) the game adds the effect of the adverts that
           are on to the awareness and books their price. Just before that the mod looks at the awareness. Below
           103% ([business] advertsUpTo) it switches every paid advert of the business on, at 103% or more it
           switches them all off. When the month changes and the awareness drops, they come on again. The first
           switching after a hand-over also comes with the next of those 91 hours.
         · An advert that costs no money (one that takes your own time, like cold calling) is left alone.
         · Why 103%: when the month changes the game takes 3% of the awareness, and then 30% of what is still
           above 100%. Awareness up to 103% goes into the next month nearly whole; of what is raised above it a
           third is gone at once. To hold more, raise advertsUpTo (up to 1.3). The business below stopped at 107%
           with all three adverts on all the time, so above 1.07 the mod would keep them on always there.
         · A cheap advert and a dear one give the same for their money (the dear one costs twice as much and
           brings twice as much), so the mod switches them together.
         · Seen in a save of 2035, one business with three paid adverts: with all three on by hand the awareness
           just before the month end was 105%, 107%, 107% and the adverts cost about $19,500 a month. With the
           mod it was 103.5%, 103.6%; the adverts were on six of the eight times and cost about $14,600 a month,
           plus a fee of about $590.
         · The fee: for an advert that was on all month, 5 hours of a PR specialist at the standard wage (about
           $262 in a save of 2035, [business] advertsFeeHours); less for an advert that was on less. The amount
           is in the hover text of an advert icon too. It is taken just before the month end and booked as staff
           costs of that business. It is taken whatever your cash is; what the cash does not cover becomes a debt
           of the game's (no interest, due after three months). The mod does not switch adverts off for want of
           money. The summary panel's line about the cash this month end needs has last month's fee in it.
         · How you see it: the game shows an advert that is on bright and one that is off dark. While the mod
           has a business's adverts, the icons of its paid adverts are orange instead, all of them, on or off.
           They turn orange the moment you hand the business over and are orange whenever its window is opened;
           they go back to bright and dark when you take the adverts back. (Whether a single advert is on right
           now does not show while they are orange: the mod switches every 91 hours and an open window would
           not follow.)
         · A plain click on an orange icon switches nothing, and the top of the screen says "S-Con: the mod
           switches this advert (Shift-click to take the adverts back)". To switch an advert yourself, take
           them back first. An advert that costs nothing is not the mod's: its icon keeps the game's look and
           a click on it works as always.
         · That a business is handed over is kept in RealisticEconomy.state, not in the save: loading an older
           save leaves it as it is.
         · To take the feature out altogether: [business] advertsKeep=0.
     - Signing contracts handed to the mod (switched in the game for each business; off to begin with): when a
       month starts the game offers a business some contracts, and accepting one takes 10 of your hours. A
       contract whose work the business does not get done fails and costs reputation; giving one up costs
       reputation too.
         · The switch: Shift-click the Contracts icon of the business window, left of "Contracts (5/6)". A line
           at the top of the screen says "S-Con: the mod signs its contracts now", and the window of the offers
           opens as for a plain click. Another Shift-click takes the signing back ("S-Con: signing contracts is
           yours again").
         · What the mod does: right after a month has started it signs, for each business handed over, every one
           of that month's offers the business has a place for. A contract's work starts with the month after,
           and the mod's staff rules hire for it. There is no limit to how fast contracts are taken on. So that
           no contract fails, only an offer for which all of this holds is signed.
             1) The business is open and has a place for another contract (the game's own count; every kind of
                business has a limit).
             2) Its staff is under the game's automatic management and its "buy worn-out assets again" is on.
                More work needs more people and assets, and the mod's staff and asset rules are what brings
                them.
             3) The month before left no more of its work undone than 5% and 8 hours, the amount the mod's
                hiring lets pass ([business] autoHireShort). More than that means the hirings did not keep up,
                and no work is added on top.
             4) The offer pays more than its work costs at standard wages and prices (the number the mod shows
                in the hover text of the offer's payout icon is above 0).
             5) The floor space holds it: what is in use now, larger by as much as the work grows, fits the
                premises - with the contracts signed before it in the same month.
             6) Every job the offer lists has somebody to work it (since 1.2.0): an employee of that job under
                the automatic management, or the job is outsourced, or the business is handed over whole. Where
                only the signing is handed over the mod does not hire the first person of a job, so an offer
                that lists a job nobody works is not signed. To get such offers signed, hire one person for the
                job yourself.
           The offers that pass are signed from the one that pays most over its cost down, the longer one first
           where two pay alike. The signing is the game's own function, the one its "Accept Contract" ends in,
           so the result is that of a contract you took.
         · When a contract fails (the game's rule): not at once when a month's work is not done. The shares of
           work left undone add up month by month, and the contract fails when the sum passes a tenth of its
           months. A contract of 36 months bears several bad months, one of 6 months hardly one. That is why
           the longer offer goes first.
         · Hiring ahead: in the month a contract is signed, when next month's work looks like more than "what
           the job's people do in a month and what a month's candidates can add", the mod hires for what would
           be missing from this month's candidates. Candidates are new every month, so this uses two months'
           worth. Somebody hired ahead is paid for a month with little to do. Most of the time a month's
           candidates are enough and nobody is hired ahead. To switch it off: [business] autoHireAhead=0.
         · The mod never gives a contract up. That stays with you.
         · Where you are told: the monthly summary has "Contracts signed by the mod: S-Con 273 hours a month for
           36 months, $6,095,712.00 (fee $321.50)", and a line at the top of the screen says "The mod signed 1
           contract(s)". In a month in which nothing was signed for want of floor space that line says "S-Con:
           no floor space for this month's offers". When nothing is signed for another reason the screen says
           nothing; RealisticEconomy.log has, for every offer, why it was or was not signed.
         · The fee: for every contract signed, 10 hours of a store manager at the standard wage of the day
           (about $322 in a save of 2035, [business] contractFeeHours), booked as staff costs of the business.
           A month without a signing costs nothing.
         · A business that uses the game's influence operation (the first box at the top right of the business
           window: influence for the hours worked, paid for with reputation) is handled like any other. That
           switch turns hours of work into influence and takes reputation at every month end; it changes
           neither the staff nor the amount of work. With a lower reputation the offers made afterwards pay
           less (payout = amount x (0.7 + 0.4 x reputation)), and that lower payout is what the mod measures in
           4). In the test save the reputation went from 67% to 61% in three months with it on, and the same
           offer, two months in, paid 35.1% over its cost where it had paid 38.5% (still a gain, so the mod
           signed it).
         · That the signing is handed over is kept in RealisticEconomy.state, not in the save.
         · To take the feature out altogether: [business] contractsKeep=0. Then there is no "whole business"
           switch either.
     - A whole business handed to the mod (switched in the game for each business; off to begin with): staff,
       assets, adverts, signing contracts and floor space at once.
         · The switch: Ctrl+Shift-click one of the advert icons of the business window. The line at the top says
           "S-Con: the mod runs all of it now". Another Ctrl+Shift-click takes all of it back ("S-Con: all of it
           is yours again"). The hover text of an advert icon ends with how to click.
         · What handing over does: every employee of the business is put under the game's automatic management,
           "buy worn-out assets again" and "buy inventory" (since 1.5.0) are switched on, and the adverts and the
           signing are handed over exactly as by their own Shift-clicks. Taking it back is switching the whole
           thing off: the automatic management and "buy worn-out assets again" go off, also where you had them on
           before, and the adverts and the signing are yours again; "buy inventory" goes off only where the mod
           switched it on. The adverts stay as the mod last set them.
         · Inventory (since 1.5.0): the game does not work a job whose stock is out at all (it is not slowed, it is
           refused). So a business handed over whole keeps its stock with the game's "buy inventory": every game
           day the game buys, at list price, what is below 0.8 times what was used since the last purchase or
           below the item's default amount. The mod buys none itself.
         · Handing over costs nothing by itself. A hiring, an advert that is on and a contract signed cost what
           they cost when handed over one by one.
         · A business nobody works in yet (since 1.5.0): handed over whole, a business such as a new one is started
           by the mod. A job with work of the month that nobody did any of gets its first person by the hiring rule
           above. The first person of the base job (the electrician of Electrical Services) comes before the
           assets, because the game's "buy worn-out assets again" buys only with 20 hours of a base-job employee;
           the missing assets are bought right after that hire, and the game buys the stock from the next day.
             · A job a member of your household works gets nobody: one with hours done this month or the last and
               no employee, or with that work in a member's schedule. A job set only by a priority that the member
               has not worked yet this month is not seen, and the mod may hire one person for it.
             · While nobody works there the mod keeps the adverts handed to it off: the customers they bring would
               find nobody, and the work left undone costs reputation.
             · Seen in a copy of the user's save: before this, a new Electrical Services stayed empty for two
               months with only its adverts on, and its reputation fell from 25% to 1%. Now the first look hired two
               electricians and bought the assets ($169,237.87), and 3.8% of that month's work was left undone.
         · A save loaded later: the hand-over is kept with the playthrough, in RealisticEconomy.state, not in a
           save. So a business handed over whole is still handed over when you load an older save. Where that
           save has the automatic management or "buy worn-out assets again" off, the load switches them on again
           (before 1.1.1 they stayed off, and the lock of the business kept you from switching them on). The
           rule for a save from before the mod, which switches both off, leaves such a business out. What you
           handed over part by part, the adverts or the signing, is kept the same way: adverts handed over days
           ago are still the mod's when you load an old save, with their orange icons.
         · Where you see what is handed over: the monthly summary right after a load has a line like "Handed to
           the mod: S-Law (contracts), S-Bro (adverts), S-Con (all of it)". A business is listed with all of it,
           adverts, contracts, or "adverts, contracts". With nothing handed over there is no such line. When you
           hand something over or take it back in that month the line is put right at once, and with everything
           taken back it reads "Handed to the mod: nothing any more". The line goes with the month and comes
           again at the next load.
         · The lock: in a business handed over whole the clicks that change it do nothing. What you change by
           hand there the mod would put back at its next look (somebody let go is hired again, an asset sold is
           bought again), or its sums would go wrong. A locked click changes nothing, and the line at the top
           says "S-Con: locked (the mod runs it). Ctrl+Shift-click an advert icon takes it back".
             · Locked: the hire window of a job (the icon at the right of its row), the staff tab (where people
               are let go), the asset shop, the right click that sells an asset or an inventory item, a job's
               outsourcing box, the star of the floor space, the box "buy worn-out assets again", the month's
               offers (accepting a contract), the icon that gives a contract up. A plain click and a Shift-click
               on an advert icon do nothing either: a business handed over whole has no part to take back
               alone.
             · Still yours: every hover text, the ledger, the "?", the button "Open" (closing and opening) and
               the premises (a move), buying inventory and the box for it, the box of the influence operation,
               and the Ctrl+Shift-click that takes the business back.
             · One thing is not locked: the priority a member of your household has for working a job of that
               business (the "3!" icons). That button is the same part everywhere in the game, and priorities
               are set in a person's own windows too, so it cannot be locked in the business window alone. The
               mod staffs the jobs of a business handed over whole as if nobody of your household worked there:
               where a member did part of the work, the mod hires for that part and the member finds less to
               do. A job with its outsourcing on is left alone by the mod, so there a member works as before. A
               job that has no employee and that a member works gets no first person (since 1.5.0).
             · To switch the lock alone off: [business] lockHandedOver=0.
         · A business with only the game's switches on (the automatic staff management, "buy worn-out assets
           again") and not handed over whole is not locked. What you do by hand there:
             · you hire somebody: where at least one employee has the automatic management the new person gets
               it too, and when there is not enough work the mod lets the dearest person go after three months;
             · you let somebody go: the moment the job's work is more than its people's hours, the mod hires a
               candidate;
             · you buy an asset: it stays, and the mod buys that kind from then on;
             · you sell an asset: while a job needs it, the mod buys it again at the next month start. To be rid
               of it for good, switch "buy worn-out assets again" off;
             · you accept or give up a contract: taken as it is. The work changes from the next month on and
               the staff rules follow it;
             · you switch a job's outsourcing on: the mod hires nobody for that job.
         · More floor space is the fifth part, and only a business handed over whole has it. The star next to
           the floor space in a business window is the game's own switch: on, it doubles the premises at the end
           of the month. When the floor space is short - a missing asset does not fit, or an offered contract
           needs more room than there is and twice the room would do - the mod does this:
             · rented premises: it switches the star on. The rent rises by what the game charges. The summary
               says "Floor space (S-Con): it doubles at the end of this month, and the rent rises by $10,135.38
               a month. The business is short of floor space, so the mod switched the star on". When a contract
               was the reason, that contract is signed in the same month.
             · premises you own: it leaves the star alone, because making them larger takes a large sum at
               once. It says "Floor space (S-Car): short of it. The premises are owned, so the mod did not make
               them larger: that costs $5,065,041.79 at once. Switch the star on yourself if you want it" (once
               a month).
             · premises that are the largest of their kind: it says "Floor space (S-Air): short of it, and
               these premises cannot be made larger. The business has to move" (once a month).
           Those three are the lines of the monthly summary. The line across the top of the screen has each in
           short, "Floor space (S-Con): doubles at month end, rent up $10,135.38 a month" for example.
           To switch this off: [business] premisesGrow=0.
         · A business without an advert icon cannot be handed over this way. Use the staff and asset switches
           and the Contracts icon one by one there.
     - A business you close (the button "Open" of its window; a move to other premises takes a closed business):
       while it is closed the mod does nothing with it. There is no setting for this.
         · Nobody is hired, and at the month end nobody is counted as "without work" or let go (in a closed
           month nobody has work, so counting would send the wrong people away). A wage demand is met as the
           game itself meets it.
         · No asset is bought, no contract signed, the star is left alone. The paid adverts the mod has are
           switched off the moment you close.
         · When you close, the line at the top says "S-Con: closed. The mod's automation rests until you open it
           again"; when you open, "S-Con: open again. The mod's automation goes on" (only for a business the
           mod does something for). What was handed over stays handed over, and from the next look on (every 91
           hours) everything runs as before.
         · A business handed over whole, and so locked, can still be closed, opened and moved: those three are
           not locked.
         · Mind (the game's rule): a business closed with work of the month left undone loses a lot of
           reputation at the month end, 67% to 47% in the test save. Close when the work is done.

10. The game ending after many loads (a fault of the game itself)
   Load saves some dozens of times without leaving the game and the game suddenly ends, with or without the mod.
   Every load leaves about 13 MB of the game before it in memory, and the game is a 32-bit program that can use
   2 GB at most.
     - Two things stay behind. One is the data files (about 250 XML files): the game reads them again at every
       load and does not delete the ones before. The other is nine objects with the monthly income and expense
       records of the household, the businesses and the listed companies, which the game's clean-up of an ended
       game leaves out.
     - When a game ends (when you load another save), the mod deletes both with functions the game already has.
     - Test (the same save loaded again and again without a pause): what a load leaves went from about 13 MB
       to about 0.3 MB. Before the change the game ended in the 94th load; after it, 330 loads in a row went
       through and the game kept running.
     - That 0.3 MB still stays, so after a very long session with more than a thousand loads restart the game once. A larger
       save (more years played) leaves more.
     - The game's rules and the saves are not touched. If loading gives trouble, deleteLeftObjects=0 in [memory]
       switches off the part about the nine objects, and memory=0 in [features] all of it.

11. Property for sale under its value
   Of the properties you can buy, the one that would gain you most gets a line in the summary panel when the
   gain is large enough: "Property under its value: 867 Curzon Street, a gain of $72,918.62 after the purchase
   fees, price $1,808,845.41". It costs nothing.
     - The game asks for a property what its own formula makes it worth, moved by chance by about 4% up or down.
       So of two properties worth the same one is cheap and one is dear.
     - The "Estimated Value" on the game's screen keeps the first two digits only (a worth of $1,641,157 reads
       $1,600,000), so a cheap property is hard to tell there. The mod compares with the value before that.
     - The gain is the value less what buying costs: the price, 3% of the price (the game's purchase fee) and
       0.1% of the value.
     - A property is named only when that gain is both of these: at least 5% of the value ([property]
       alertBelowValue) and at least about $100,000 ([property] alertBelowAmount; the setting, 54000, is in
       dollars of the game's first prices and rises with the price level, which makes about $100,000 in 2035).
       A gain of 5% after the fees takes a price about 8% under the value, which one property in forty has.
       Lower the two settings to see lines more often; 0 switches that test off.
     - The line is written anew when the month changes (the game puts new properties on the market), after a
       save is loaded, and when a search of your character for property has ended: it then names the best of
       the list as it is. When nothing counts any more the month's line reads "none at the moment".
     - Selling costs a fee too (3% of what you paid), and what you saved at the purchase is taxable income in
       the year of the sale.
     - The list of properties is made anew every month. The properties are in the search window (key b), the tab
       with the house; a click on a row opens the place, where the address and the amounts are.
     - Nothing about prices or listings is changed. The mod only tells.

12. An education you completed is kept
   The game takes 1% a month off the "effective" number of every experience and education (degrees, diplomas,
   certificates). Jobs, educations and activities measure you against that number, so a degree counts for less
   every year: about 55% is left after five years, about 30% after ten (0.99 multiplied 60 and 120 times). Someone
   who does not take up the trade soon after the degree has to study the same thing again.
   With the mod the effective number of an education that a member of your household paid and studied for does
   not go below what that person gained of it. Work experience ("XP:Sales" and the like) decays every month as
   the game has it.
     - It is kept up to the highest amount that any job, education or activity asks of that education. Example:
       a law degree completed with 548, the lawyers' jobs ask 500: the effective number does not go below 500.
       What lies above 500 still decays by 1% a month.
     - When less was gained than that amount, what was gained stays. Example: a school diploma earned with 600,
       the highest amount asked is 650: it stays at 600.
     - An education nothing in the game asks for decays as the game has it.
     - What an education is comes from the game's data (the educations folder): school years, degrees, diplomas,
       trade school and short courses, 45 kinds.
     - A save from before the mod is put right at the first month end after it is loaded: the numbers of its
       educations go up to what was gained (up to the amount above). In the save this was tried with, a diploma
       that had decayed to 197 went back to 600 and a law degree from 353 to 500. The same save's sales
       experience (3733) was left alone and was 3696 a month later.
     - Members of the household only. Employees of your businesses and people outside the household are as the
       game has them.
     - The "?" in the experience window (the brain at the top left) says this under the game's own sentence.
     - Off: [features] experience=0. The numbers then decay as in the game again (numbers that were raised are
       not taken down).

13. Boxes in people's names (a fault of the game itself)
   A name is drawn with a box with a cross in it where a letter should be ("Sylwester Napiera□a"), with or
   without the mod. The game draws all text with one font file a language, people's names are written with the
   letters of many languages, and a font that has no picture for a letter draws its "no such letter" picture. The
   Korean font lacks 41 of the letters the names use (ł, ş, ğ, ž and others), the English font 18.
     - The mod takes that one letter from another font file of the game. The line's height and every other
       letter stay the language's font's. The order it looks in: the fixed-width Noto font (Latin, Greek,
       Cyrillic), the Korean, Japanese and Chinese Noto fonts, the Thai font, the English font.
     - The same fault made Korean lines kept in a save (older lines of the monthly summary) come out as boxes in
       an English game. They are drawn now.
     - A name in Arabic letters is written in Latin letters. Nine Arabic letters are in none of the game's font
       files, so there is no other font to take them from. Of the 81,568 names in the game's 108 lists, three are
       written with them, all in the list of Iranian men's first names. Each becomes the same name as that list
       has it in Latin letters: "Arash", "Alishah", "Mohammad".
         · A new person (an applicant, an employee, a rival, a baby, the people of a new game) gets the name in
           Latin letters when it is picked.
         · A person a save already has gets it when the save is loaded. The save file is not touched; when you
           save afterwards, the new name is saved.
     - No font file and no list of names of the game is changed or added. Off: [features] fonts=0.

14. Balance: a few of the game's numbers
   Some numbers of the game's economy that lean to one side are changed. No text is added to the screen. Every
   number is in [balance] of RealisticEconomy.ini, and a number that is the game's own leaves that item as it is
   in the game. All of it off: [features] balance=0. The game's data files are not changed (the game reads them
   with the numbers changed; a line that another mod has changed already is left alone).
     - Share trading fee 2% (the game: 1%). stockFee
         The stocks window shows what a share is worth, and the price moves a fifth of the way there every month.
         Buying what is below its worth and selling it when it is not earned 24 to 35% a year after a fee of 1%,
         in the price histories of real games. Buying and holding earns 8%, a certificate of deposit 7%. At 2% it
         earns 15 to 18% (at 3% less than holding). For somebody who buys and holds little changes (8.3% to 8.0%).
     - Futures fee: 10% of the margin (the game: 1%). futuresFee
         The margin is a twentieth of the contract, so of the contract it is 0.5% in place of 0.05%. The label
         under the buttons of the futures tab says so now ("fee 10% of margin"); the game's "10% trading fee"
         could be read as a tenth of the contract.
     - The savings account's rate: 0.8 times the central-bank rate (the game: 0.5). savingsRate
     - Interest on cash: at a month end the cash earns a twelfth of 0.5 times the central-bank rate (the game:
       nothing). cashInterest
         In the game money earns only in a savings account, and putting it in and taking it out each cost a visit
         to the bank, so small sums and money needed soon earned nothing. Now the three make a ladder: cash 0.5
         (no visit), a savings account 0.8 (two visits, the money can be taken out at any time), a certificate of
         deposit 0.8 plus one point (one visit, held for its term). The interest is in the game's list of income
         as "Interest" and counts for the tax, as a savings account's does. It is worked out from the cash just
         before the month end, so it is the same after a load.
     - A lease that is renewed can get cheaper. rentFollowsMarket
         At a renewal the game takes the larger of the market's rent and the rent you pay, so a rent only ever
         goes up. The mod takes the market's rent, for a home and for business premises alike. (The market's rent
         has a random part of about 4%, the game's own.)
     - A business you start has an awareness of 25% (the game: 0%). newBusinessAwareness
         Customers come by 5% + 95% of the awareness. A new business of the game works at a twentieth of its market
         until adverts have raised its awareness, while wages and rent run in full. 25% is where the game starts
         a new business's reputation. Only for a business made with "Get Business Permit" at City Hall.
     - Hydroponics Farm earns what the other business types do. hydroponics
         In the game's own table of the business types (data/statsFirm.txt) its income is 119% of its cost, and
         the other 46 types are at 136 to 142%. The income of an hour of its delivery driver is raised to make
         it 139.7%.
     - The degree Communications costs half ($90,000 to $45,000, before the city's and the price index's part).
       communicationsDegree
         The four jobs that ask for this degree pay $20.15 to $26.35 an hour.
     - Four degrees cost a year of the pay of the jobs they open (since 1.2.1; before the city's and the price
       index's part). degreePrices
         Computer Science $113,000 to $53,800, Art and Design $107,300 to $51,200, Science and Engineering $115,000
         to $63,000, Business and Finance $125,000 to $56,500. In the game the four cost 22 to 27 months of that
         pay, where 25 of the 30 courses that open jobs cost 13 months or less. The other courses keep their price.
     - Competition comes earlier: in a business type's market the firms count 1.25 times (since 1.3.0; the game:
       1). competitionWeight
         The game measures competition in each business type by the custom the city holds plus the share of that
         type's firms; the larger that sum, the more awareness the type's businesses lose every month. At 1.25 the
         firms' size at which competition sets in comes down to about four fifths of the game's. In one save the
         competition index (awareness lost a month, %) of Auditing, Law Firm and Hair Salon was +9, +7, +2 as in the
         game, +20, +15, +10 at 1.25 and +32, +23, +18 at 1.5. At 1.5 a large type loses a third of its awareness
         every month, more than advertising holds up, so 1.25 it is. 1 to 3 can be set; 1 is the game. Rivals and
         listed companies see the same competition. The custom the city holds shrinks faster too, and what it lost
         stays in the save (also without the mod). Competition among property types is reckoned apart and is not
         changed.
     - A business's monthly bonus for completed work goes down with the share left undone (since 1.4.0).
       reputationBonusFade
         At a month end the game takes 3% of a business's reputation (and 30% of what lies above 100%), takes 0.2
         times the share of the month's work left undone, and gives 3.6 points for a month with nothing left. The
         penalty goes with the share, but the bonus is gone for one hour: one hour left loses as much as 18% left
         costs in penalty. The mod lowers the 3.6 points with the share: at 0.02 a month with 1% left gets 1.8
         points and one with 2% or more none. The penalty stays the game's. The business window's hover text of the
         reputation lists the two as "completed work" and "incomplete work". Your businesses only; 0 = the game. In
         the user's save of 2035 S-Bro left 1 of 1,829 hours undone: 75.9% instead of the game's fall from 74.6% to
         72.4%.

15. Money into influence (since 1.3.0, for late in the game)
   Late in the game money piles up and there is little to spend it on. Influence is the points that move the city's
   votes on its policies. The game's "Donate To Political Group" ($100,000) gives about 100 points a month for 12
   months. The mod adds two larger donations at the same rate, below that one at a bank:
     - Fund A Think Tank: $1,000,000. About 1,000 influence points a month for 12 months.
     - Fund A Political Party: $10,000,000. About 10,000 a month for 12 months.
   The game's price level multiplies the price. As with the game's donation, only a person with the passion
   Lobbying can make them, and they take 10 hours. Each can be made once a year: the same donation made again
   within 12 months does not add its points, it only starts the 12 months again, so the money would be lost.
   Different donations add up.
   The game's own donation ("Donate To Political Group") could be made every month, but made again it only starts
   its 12 months again, so made monthly it threw away eleven twelfths of the money. The mod makes it once a year for
   the household as well; for 12 months after it the bank's row says "recently done".
   The city votes on its policies every 12 months (the game: 24). The next vote a save has planned already comes on
   its date; from that vote on the votes are 12 months apart.
   Off: [features] influence=0. One by one: [influence] bigDonations=0 (the two donations), voteEvery12Months=0 (the
   time between votes), donationOncePerYear=0 (the game's donation every month). The game's data files are not changed: the game reads them with the two donations added.

16. Personal Coaching (since 1.3.0)
   In the game a person's likes and dislikes (preferences) change only on a birthday, for points, and only four
   chosen by chance can be changed. The mod adds 18 activities at a hospital, below "Mental Therapy", one for each
   preference ("Personal Coaching: Working", "Personal Coaching: Exercising" and so on). You choose the preference.
     - $500,000 times the game's price level, and 20 hours. No happiness points are taken.
     - When it ends, the chosen preference moves one step towards a like: an allergy becomes a hate, a hate a
       dislike, a dislike a like. A love becomes a like and an addiction a love (both have a need to keep up). A like
       stays as it is.
     - From a dislike to a like the same price is taken once more at the end. The game's character editor counts that
       step twice as well.
     - A person changes one preference a year. After a change all 18 coachings of that person show as "Already done
       recently" for twelve months.
     - Nothing changes and the money comes back when the preference is a like already, when another preference of
       the person was changed the same year (two coachings booked before either ended), or when the cash is short of
       the second price. A coaching paid back is not blocked as done recently (after a change the same year it is
       blocked with the others).
     - What changed is shown in the ticker at the top and in the recent notes, for example "Coaching for (name):
       Exercising, Dislike → Like, $928,935.92 more paid".
   The change of a preference in the birthday window stays as in the game. Coaching does not count as a birthday
   change, so the points a change costs there stay as they were.
   Off: [features] coaching=0.

No window or button is added. Text is added to windows the game already has, and twenty activities to the game's
lists of a place (the two donations and the 18 of Personal Coaching).

The mod's own texts follow the language of the game, all eight: English, Korean, German, Spanish, French,
Brazilian Portuguese, Japanese and Simplified Chinese. Where the game has a word of its own for a thing (awareness,
floorspace, board of directors), the mod's texts use that word. This description exists in English and Korean.


Install
-------

1. Close the game.
2. Extract this zip into the game folder (the one that holds TGL2.exe). The game folder then has:
     RealisticEconomy_install.bat      the install file
     RealisticEconomy_uninstall.bat    the uninstall file
     RealisticEconomy (a folder)       the mod's files; the install file copies them from here
     RealisticEconomy_README_en.txt    this description (_ko is the Korean one)
   In Steam: right-click the game in the library, "Manage > Browse local files".
3. Double-click RealisticEconomy_install.bat. It does three things:
     - It copies the whole saves folder to saves_before_RealisticEconomy_1: your saves from before the install.
     - It copies three files next to TGL2.exe:
         version.dll             a public ASI loader; it loads .asi files when the game starts
         RealisticEconomy.asi    the mod
         RealisticEconomy.ini    its settings; a settings file that is already there is left as it is
     - If the game folder already has a different version.dll it stops and overwrites nothing: that file may be
       another mod's loader.
4. Start the game.

How to tell that the mod is installed:
     - The version in the lower right corner of the main menu reads "v1.03.22 + Realistic Economy (the mod's
       version)". Without the mod it reads "v1.03.22".
     - RealisticEconomy.log appears in the game folder, with the line "build pin: MATCH" and a "feature ... on"
       line per feature.

A newer release goes in the same way: extract it over the old one and run the install file again. When the mod
is already in, the saves are not copied a second time.

What the install file and the uninstall file say is in the language Windows is set to (the game's eight languages,
English for any other).

Without the batch file: copy the three files of the RealisticEconomy folder into the folder that holds TGL2.exe
yourself, after copying the saves folder to a safe place.


Uninstall
---------

Close the game and double-click RealisticEconomy_uninstall.bat. It deletes version.dll and RealisticEconomy.asi.
The game then runs as it did before the mod, and the main menu shows "v1.03.22" again.
     - The saves are not touched. Saves made with the mod load without it; what stays in them is listed below.
     - To go back to the saves from before the install: rename the saves folder (to saves_mod, say) and put a
       copy of saves_before_RealisticEconomy_1 in its place under the name saves. Every install into a game
       without the mod makes a new folder with the next number.
     - The settings and records (RealisticEconomy.ini, RealisticEconomy.state, RealisticEconomy.log) are kept:
       a later install goes on with them. Delete them if you do not need them.
     - A version.dll that is not the file this mod put there is not deleted.
     - The rest of the package (the two batch files, the two descriptions, the RealisticEconomy folder) does
       nothing to the game. Delete it if you will not install again.

By hand: close the game and delete version.dll and RealisticEconomy.asi.

What stays in a save:
     - Loans taken at a fixed rate while the mod was in use (mortgages, education loans) keep that rate.
     - Variable-rate loans return to the game's own rates.
     - The lines in the summary panel disappear at the next month end.
     - A course of share prices that the trade lock changed goes on as it is.
     - The fixed month end leaves nothing in a save. Without the mod a month end is drawn the game's own way again.
     - A company listed while the mod was in use, the cash it paid you and the tax on that cash stay as they are.
     - A member seated by the guarantee stays until the next election. The election notice of a company you
       listed stays switched on.
     - The casino goes back to the game's own way (amounts that grow with net worth, no limit on how often).
       What you won or lost while the mod was in use stays.


Settings
--------

Open RealisticEconomy.ini in a text editor; every number is explained there. Restart the game after a change. Only
release under [guard] is read at the next load.


Switching features on and off
-----------------------------

Every feature has a switch under [features] in RealisticEconomy.ini: 0 turns it off, 1 turns it on again. A
feature that is off leaves that part of the game alone. Keep only the ones you like.

     credit=0      no credit-grade rates: loan rates are the game's own again
     forecast=0    no "This month end: cash needed" line in the summary panel
     guard=0       no trade lock after a load: stock and futures trades are never refused. The fixed month end
                   goes with it: growth, share prices, a listing price and a new month's properties are drawn
                   the game's own way
     ipo=0         a listing works as in the game itself (you keep 25%, no cash)
     board=0       no guaranteed board seats
     stocks=0      nothing of 6 above: no ratios or fair price in the stock window, no research subscription, the
                   sort buttons, the charts and the futures list as in the game
     casino=0      the casino works as in the game itself
     business=0    nothing of 9 above: the hire window as in the game, no staff automation, no missing assets
                   bought, a worn-out asset bought again pays out as in the game, adverts are yours alone
     memory=0      nothing of an ended game is deleted; the game ends after some dozens of loads, as it does by itself
     property=0    no line about property for sale under its value
     experience=0  completed educations decay every month too, as in the game
     fonts=0       a letter the font does not have is a box again, as in the game
     balance=0     all of 14 above off: the fees, the interest, the renewal of a lease, a new business's
                   awareness and the two prices are the game's
     wording=0     no corrected texts

Any mix of them works (each feature alone and each one missing were run through two month ends). A few things to
know when you switch something off:
     - forecast=0 with [business] missingAssetsOnDebt=0: missing assets are bought out of all the cash there is,
       because the amount the month end will take is the forecast's.
     - business=0, or [business] advertsKeep=0, after you handed adverts to the mod: they stay as the mod last
       left them, which can be off. Take them back first (Shift-click), or click the advert icons afterwards.
     - business switched on later in a playthrough: the load at which it is on for the first time is the one that
       switches the two game switches off (see "A save from before the mod" at the top).
     - casino=0 while a game begun under the mod is under way: that game ends without a result, no money moves.
     - wording=0: lines already written into a month's summary stay as they were written.

The trade lock is a matter of taste. If you play by going back to earlier saves, set guard=0. To lift one lock and
keep the feature, use release=1 under [guard] (see 3 above). To keep the lock and let a month end be drawn the
game's own way, use monthEndFixed=0 under [guard] (see 3 above). To let a load undo casino games and keep the rest
of the casino feature, use keepResultsOnLoad=0 under [casino] (see 8 above).

What the mod does in your place, its switches and what it costs:
     - Six switches are inside the game.
         · Stock research subscription: in the stock window's list of companies, a Shift-click (that company) or
           a Ctrl+Shift-click (every listed company); "Research kept fresh" in 6 above. Off to begin with.
         · Assets: the business's "buy worn-out assets again" (the game's own). Where it is on, the mod buys
           missing assets too ("Missing assets bought" in 9 above).
         · Staff: the round-arrow icon at the top right of a business's staff tab (the game's automatic staff
           management). In that mode a click on anybody's icon switches it for every employee of the business.
           What the mod does about staff - answering a pay rise, letting go of people the work does not need, a
           person of fewer hours for a job of one, hiring for a job that is short of hours - applies only to
           employees and jobs that have it.
         · Adverts: a Shift-click on an advert icon in the business window ("Adverts handed to the mod" in 9
           above). It is a switch of that one business and off to begin with.
         · Signing contracts: a Shift-click on the Contracts icon in the business window ("Signing contracts
           handed to the mod" in 9 above). A switch of that one business, off to begin with.
         · A whole business: a Ctrl+Shift-click on an advert icon in the business window ("A whole business
           handed to the mod" in 9 above). It switches assets, staff, adverts and signing on at once, and has
           the mod make rented premises larger when the floor space runs short.
     - The rest is switched in RealisticEconomy.ini and takes effect when the game is started again.
         [business] autoManageNewStaff   the automatic management for a person you hire (free)
         [business] wageDemandRule       a pay rise answered by the job's candidates (when a candidate is put in:
                                         20 hours of a store manager, [business] hireFeeHours)
         [business] candidatesFixed      candidates that a load does not change (free)
         [guard] monthEndFixed           a month's growth, cycles and share prices, a listing price, and a new
                                         month's properties and offers that a load does not change (free)
         [stocks] researchOnBoard        the research of a company you sit on the board of, every month (free)
         [stocks] researchSubscription   whether the subscription for "every company" is on from the start (0
                                         by default; it is switched in the game; 5 hours of a bank analyst a
                                         company, [stocks] researchFeeHours)
         [business] spareMonths          people the work does not need let go, a person of fewer hours for a
                                         job of one (letting go is free, the hiring costs hireFeeHours; 0 = off)
         [business] autoHire             a candidate hired for a job with more work than hours (hireFeeHours
                                         each time)
         [business] autoHireNow          the same look right after a load and a whole hand-over (hireFeeHours
                                         each time)
         [business] autoHirePlan         the other jobs' work of the month counted ahead by the type's table
                                         (hireFeeHours each time)
         [business] autoManageWholeBusiness  the automatic-management icon for the whole business (free)
         [business] missingAssetsBought  missing assets bought (no charge of the mod's; the game's shop price)
         [business] missingAssetsOnDebt  what the cash does not cover of them becomes a debt of the game's (1 by
                                         default; 0 = bought only when the cash is there)
         [business] switchesOffOnFirstLoad  the asset and staff switches above are switched off, and a casino
                                         game is taken out of the action queue, when a save from before the
                                         mod is loaded (1 by default; 0 = left as the save has them)
         [business] advertsKeep          adverts can be handed to the mod (only a business handed over: 5 hours
                                         of a PR specialist for an advert that was on all month, [business]
                                         advertsFeeHours)
         [business] contractsKeep        signing contracts, and a whole business, can be handed to the mod (only
                                         a business handed over: 10 hours of a store manager for a contract
                                         signed, [business] contractFeeHours)
         [business] autoHireAhead        in the month a contract is signed, hiring ahead for work that a month's
                                         candidates will not staff (hireFeeHours a hiring)
         [business] lockHandedOver       the clicks that change a business handed over whole are locked (free)
         [business] premisesGrow         the star of rented premises switched on when a business handed over
                                         whole is short of floor space (no cost of the mod's; the rent rises by
                                         what the game charges)
         [features] property             the line about the property that would gain most (free)
       Not an automation, but in the same file: [stocks] sortButtons (four sort buttons of the stock list sort
       by capitalisation, by undervalued, by overvalued and by the change in a month; free, on), [stocks] chartPriceOnly (a
       company's chart opens with the share price alone; free, on), [stocks] chartEconomyOne, chartFixes,
       chartIndex, chartOpenWith, chartLineWidth and chartValues (the charts, 6 above), [stocks] rowNumbers
       (the change and the fair price in the rows of the
       stock list) and [stocks] futuresList (the rate and its expectation in the futures list, and the sort). All free and
       on.
     - A cost is "hours of the job that would do it, at the standard wage", so it rises with the game's wages and
       prices. All of them are deducted when the tax is worked out. A setting of 0 hours makes it free.


Files the mod writes
--------------------

     RealisticEconomy.log      what the mod did. Look here when something seems wrong.
     RealisticEconomy.state    what it remembers per playthrough: last month end's costs, the trade lock's record,
                               the casino games played.

The mod does not edit the game's files or the save files. Deleting RealisticEconomy.state also deletes the trade
lock's record and the casino's: this is a mod for players who want to keep a limit they set for themselves.


Good to know
------------

     - An antivirus may be suspicious of version.dll: it is a file next to the game that loads other code.
     - After a game update the mod switches itself off. It needs a new build for the new version.
     - Not checked in the game: the forecast in a month in which staff changed.
     - A month end passed with the "end month" buttons of the game's developer mode is not seen by the mod: it
       enters neither the forecast nor the trade lock's record.
     - Speed, measured on a save with five businesses and 66 employees: while time runs the mod's work does not
       show (under a thousandth of a second for a text on the screen). A month end takes the game about a second
       by itself, and the mod adds 0.05 to 0.5 seconds. Hiring for a job that is short of hands takes about a
       tenth of a second a job, so a month in which several jobs are short at once (the first month with the mod,
       the month after a large contract) can stop once for about half a second. To measure it yourself, set
       timing=1 under [trace] in RealisticEconomy.ini: at every month start RealisticEconomy.log gets a line a
       kind of the mod's work, with how often it ran and how long it took.


Licence
-------

The mod (RealisticEconomy.asi, RealisticEconomy.ini and these descriptions) is under the MIT licence: change it
and pass it on as you like. The full text is RealisticEconomy-LICENSE.txt in the RealisticEconomy_licenses folder
inside the RealisticEconomy folder.


Software by others
------------------

version.dll is Ultimate ASI Loader v9.7.4 by ThirteenAG (its release file dinput8.dll, renamed). MIT licence; the
full text is in the same RealisticEconomy_licenses folder.
https://github.com/ThirteenAG/Ultimate-ASI-Loader
