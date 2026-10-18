"""Do the plugin's ticker lines fit the ticker? Every line, in the eight languages, drawn with the game's fonts.

    python analysis/scripts/ticker_fit.py [--control] [--list]

The ticker is the line across the top of the game's screen. Its band holds about 960 pixels of text in a window of
1600x900; a longer line runs out of the band on both sides and, past 1160, under the panel at the left and off the
window (REQUEST.md [60]: the line about a save from before the mod was 1289 pixels in Korean and 2536 in German).
The long lines belong in the monthly summary, which breaks a line; the ticker gets a short one.

How a width is found: the text of mod_src/_build/dump_texts.exe (built by mod_src/build.ps1 from the plugin's own
table), its conversions filled with a business name of ten letters and an amount of thirteen, measured with the
font file the game uses for the language, and scaled by one whole ticker line of that font measured in the game
(SEEN, run980 to run983). The Korean line of run944, 1000 pixels in the game, comes out as 998.
LINES is every line re_main.c sends to the ticker, as the code puts it together. A new call of
re_game_post_message or re_game_post_urgent needs a row here: the count of those calls in re_main.c is compared
with SITES, and the check fails when it is another.

The last line: `ticker lines=<n> over_budget=<n> sites=<n> of <SITES> budget=<pixels> verdict=PASS|FAIL`, the exit
code 1 on FAIL. --control measures against 300 pixels, which most lines exceed: it has to end with FAIL.
--list writes every line's widest width.
"""

import pathlib
import re
import subprocess
import sys

from PIL import ImageFont

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUDGET = 960.0
SITES = 24
FONT = {
    "ko": "NotoSansKR-Regular.ttf",
    "jp": "NotoSansJP-Regular.ttf",
    "zh_CN": "NotoSansSC-Regular.ttf",
}
LATIN = "sofiapro-light.otf"
NAME, MONEY = "North Star", "$1,234,567.89"


def texts():
    header = (ROOT / "mod_src/plugin/re_lang.h").read_text(encoding="utf-8")
    names = [
        n
        for n in re.findall(r"^\s*(RE_MSG_[A-Z0-9_]+)\s*[,=]", header, re.MULTILINE)
        if n != "RE_MSG_COUNT"
    ]
    dump = subprocess.run(
        [str(ROOT / "mod_src/_build/dump_texts.exe")], capture_output=True, check=True
    ).stdout
    found = {}
    for line in dump.decode("utf-8").splitlines():
        number, lang, text = line.split("\t", 2)
        found.setdefault(names[int(number)], {})[lang] = text
    return found


TEXT = texts()
LANGS = list(TEXT["RE_MSG_ALL_KEPT"])
loaded = {}


def raw(lang, text):
    name = FONT.get(lang, LATIN)
    if name not in loaded:
        loaded[name] = ImageFont.truetype(str(ROOT / "fonts" / name), 100)
    return loaded[name].getlength(text)


def fill(text, samples):
    samples = list(samples)
    text = re.sub(r"%(lld|d|u)", "12", text)
    text = re.sub(r"%\.\d+f", "2.5", text)
    return re.sub(r"%s", lambda _: samples.pop(0) if samples else NAME, text).replace(
        "%%", "%"
    )


def t(name, lang, *samples):
    return fill(TEXT[name][lang], samples)


# one whole ticker line of each font as the game drew it at 1600x900: the text, its width in pixels
SEEN = {
    "ko": (t("RE_MSG_ALL_KEPT", "ko", "S-Con"), 228),
    "en": (t("RE_MSG_ALL_KEPT", "en", "S-Con"), 355),
    "jp": (t("RE_MSG_ALL_KEPT", "jp", "S-Con"), 236),
    "zh_CN": (t("RE_MSG_ALL_KEPT", "zh_CN", "S-Con"), 184),
}
SCALE = {lang: pixels / raw(lang, text) for lang, (text, pixels) in SEEN.items()}


def width(lang, text):
    return raw(lang, text) * SCALE.get(lang, SCALE["en"])


def premises(lang):
    return t("RE_MSG_PREMISES_PREFIX", lang, NAME)


def coaching(lang, name="RE_MSG_COACH_DONE"):
    # a person, a long preference name, the two words of the game's for its levels, from a dislike the second price;
    # for a preference at a like already its word, for a short cash the second price
    pref, before, after = (
        ("スピリチュアル", "大嫌い", "好き好き")
        if lang in FONT
        else ("Spirituality", "N'aime pas", "Le gusta")
    )
    if name == "RE_MSG_COACH_NONE":
        return t(name, lang, NAME, pref, after)
    if name == "RE_MSG_COACH_YEAR":
        return t(name, lang, NAME, pref)
    if name == "RE_MSG_COACH_CASH":
        return t(name, lang, NAME, pref, MONEY)
    return t(name, lang, NAME, pref, before, after, MONEY)


def widest(lang, texts):
    return max(texts, key=lambda text: raw(lang, text))


def game(lang):
    return widest(
        lang,
        [
            t("RE_MSG_CASINO_" + name, lang)
            for name in ("SLOTS", "ROULETTE", "BLACKJACK", "BACCARAT")
        ],
    )


def asset_short(lang):
    # the first asset that could not be bought and "and <n> more", for each of the three reasons
    return widest(
        lang,
        [
            t("RE_MSG_ASSET_SHORT_PREFIX", lang)
            + t("RE_MSG_ASSET_SHORT_" + why, lang, "", NAME, "Excavator")
            + t("RE_MSG_HELD_MORE", lang)
            for why in ("ROOM", "CASH", "NONE")
        ],
    )


LINES = {
    "research kept": lambda g: t("RE_MSG_RESEARCH_KEPT_TICK", g, MONEY),
    "research: on the board": lambda g: t("RE_MSG_RESEARCH_BOARD", g, NAME),
    "research: all have it": lambda g: t("RE_MSG_RESEARCH_ALL_HAS", g, NAME),
    "research: all off": lambda g: t("RE_MSG_RESEARCH_ALL_OFF", g, NAME),
    "research: one off": lambda g: t("RE_MSG_RESEARCH_ONE_OFF", g, NAME),
    "research: no cash": lambda g: t("RE_MSG_RESEARCH_NO_CASH", g, MONEY),
    "research: all on": lambda g: t("RE_MSG_RESEARCH_ALL_ON", g, MONEY),
    "research: one on": lambda g: t("RE_MSG_RESEARCH_ONE_ON", g, NAME, MONEY),
    "research: one on, researched already": lambda g: t(
        "RE_MSG_RESEARCH_ONE_ON_DONE", g, NAME, MONEY
    ),
    "trade lock refused": lambda g: t("RE_MSG_LOCK_REFUSED", g),
    "casino: played this month": lambda g: t("RE_MSG_CASINO_PLAYED", g, game(g)),
    "casino: begun before the mod": lambda g: t("RE_MSG_CASINO_BEFORE", g, game(g)),
    "casino: won": lambda g: t("RE_MSG_CASINO_WON", g, game(g), "2.5", MONEY),
    "casino: lost": lambda g: t("RE_MSG_CASINO_LOST", g, game(g), MONEY),
    "business locked": lambda g: t("RE_MSG_LOCKED", g, NAME),
    "adverts handed over": lambda g: t("RE_MSG_ADVERTS_KEPT", g, NAME),
    "adverts taken back": lambda g: t("RE_MSG_ADVERTS_BACK", g, NAME),
    "advert the mod switches": lambda g: t("RE_MSG_ADVERTS_LOCKED", g, NAME),
    "whole business handed over": lambda g: t("RE_MSG_ALL_KEPT", g, NAME),
    "whole business taken back": lambda g: t("RE_MSG_ALL_BACK", g, NAME),
    "contracts handed over": lambda g: t("RE_MSG_CONTRACTS_KEPT", g, NAME),
    "contracts taken back": lambda g: t("RE_MSG_CONTRACTS_BACK", g, NAME),
    "contracts: no floor space": lambda g: t("RE_MSG_CONTRACT_ROOM", g, NAME),
    "contracts signed": lambda g: t("RE_MSG_CONTRACT_TICK", g),
    "business closed": lambda g: t("RE_MSG_FIRM_CLOSED", g, NAME),
    "business opened": lambda g: t("RE_MSG_FIRM_OPENED", g, NAME),
    "premises grow": lambda g: t("RE_MSG_PREMISES_GROW_TICK", g, premises(g), MONEY),
    "premises the largest": lambda g: t("RE_MSG_PREMISES_MOST_TICK", g, premises(g)),
    "premises owned": lambda g: t("RE_MSG_PREMISES_OWNED_TICK", g, premises(g), MONEY),
    "assets bought": lambda g: t("RE_MSG_ASSET_TICK_BUY", g, MONEY),
    "assets bought on debt": lambda g: (
        t("RE_MSG_ASSET_TICK_BUY", g, MONEY) + t("RE_MSG_ASSET_TICK_DEBT", g, MONEY)
    ),
    "asset not bought": asset_short,
    "a save from before the mod": lambda g: t("RE_MSG_ADOPT_TICK", g),
    "coaching": coaching,
    "coaching: past a dislike": lambda g: coaching(g, "RE_MSG_COACH_CROSS"),
    "coaching: a like already": lambda g: coaching(g, "RE_MSG_COACH_NONE"),
    "coaching: one a year": lambda g: coaching(g, "RE_MSG_COACH_YEAR"),
    "coaching: cash short": lambda g: coaching(g, "RE_MSG_COACH_CASH"),
}


def main():
    budget = 300.0 if "--control" in sys.argv else BUDGET
    source = (ROOT / "mod_src/plugin/re_main.c").read_text(encoding="utf-8")
    sites = len(re.findall(r"re_game_post_(?:message|urgent)\(", source))
    over = 0
    for label, make in LINES.items():
        widths = {lang: width(lang, make(lang)) for lang in LANGS}
        wide = {lang: round(w) for lang, w in widths.items() if w > budget}
        over += bool(wide)
        widest = max(widths, key=widths.get)
        if wide and "--control" not in sys.argv:
            print(f"  over {budget:.0f}: {label}: {wide}; the widest: '{make(widest)}'")
        elif "--list" in sys.argv:
            print(f"  {label}: {widths[widest]:.0f} ({widest}), ko {widths['ko']:.0f}")
    if sites != SITES:
        print(
            f"  re_main.c sends to the ticker from {sites} places, this check knows {SITES}: bring LINES and SITES up to date"
        )
    ok = over == 0 and sites == SITES
    print(
        f"ticker lines={len(LINES)} over_budget={over} sites={sites} of {SITES} budget={budget:.0f} verdict={'PASS' if ok else 'FAIL'}"
    )
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
