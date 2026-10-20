"""Do the plugin's hover texts keep to their lines? (DECISIONS.md D-120: what the mod adds to one place is two lines
at most in Korean; another language may take one line more.)

    python analysis/scripts/tip_fit.py [--control] [--list]

A hover text of the game is drawn in a monospaced font. In Korean, Japanese and Chinese it is broken letter by
letter; in the captures of the game a Korean line holds 18.0 to 19.5 em, a Hangul letter being 1 em and a Latin
letter, a digit, a space or a punctuation mark half of one (run1100, the staff switch: 16 lines; run1173, the box for
worn-out assets). In the other languages it is broken at a space: a word that does not fit goes to the next line, and
lines of 37 and 38 letters were seen while a 39th did not fit (run1415 to run1421, REQUEST.md [72]; counting whole
ems alone had passed a Spanish and a German text that took four lines in the game). So a text is counted in em, each
paragraph apart, a line is taken as 18 em, and outside the three languages the words are laid out as the game does:
a text that passes here has its lines in the game, one that is half an em over may still have them.

PLACES is every place the mod adds a hover text to, as re_main.c puts the text together with the settings as they are
shipped, with the lines it may take in Korean. A message of re_lang.h whose name has TIP or HOVER in it and is in no
place makes the check fail: a new hover text needs a row here.

The last line: `hover texts places=<n> over=<n> unplaced=<n> line=<em> verdict=PASS|FAIL`, the exit code 1 on FAIL.
--control counts a line as 6 em, which most texts exceed: it has to end with FAIL. --list writes every place.
"""

import math
import pathlib
import re
import subprocess
import sys
import unicodedata

ROOT = pathlib.Path(__file__).resolve().parents[2]
LINE = 18.0
MORE = 1  # the lines another language may take beyond the Korean ones
BY_LETTER = ("ko", "jp", "zh_CN")  # broken letter by letter; the other languages at a space
NAME, MONEY = "North Star", "$1,234.56"
# an advert's month of the mod's care (README: five hours of a PR specialist): $262.08 in the test save of 2035; the
# check takes a price of nine letters, what the same fee is after some more decades of the game's inflation
FEE = "$1,262.08"
# a business type's month at today's prices, to the nearest $100: a Medical Clinic's sales after ten years of the
# game's inflation (note e21, $135,165)
ESTIMATE = "$135,200.00"


def texts():
    header = (ROOT / "mod_src/plugin/re_lang.h").read_text(encoding="utf-8")
    names = [n for n in re.findall(r"^\s*(RE_MSG_[A-Z0-9_]+)\s*[,=]", header, re.MULTILINE) if n != "RE_MSG_COUNT"]
    dump = subprocess.run([str(ROOT / "mod_src/_build/dump_texts.exe")], capture_output=True, check=True).stdout
    found = {}
    for line in dump.decode("utf-8").splitlines():
        number, lang, text = line.split("\t", 2)
        found.setdefault(names[int(number)], {})[lang] = text.replace("\\n", "\n")
    return found


TEXT = texts()
LANGS = list(TEXT["RE_MSG_XP_TIP"])
used = set()


def t(name, lang, *samples):
    used.add(name)
    samples = list(samples)
    text = re.sub(r"%\+?(lld|d|u)", "12", TEXT[name][lang])
    # a number as the conversion writes it: "%.0f" is an awareness like 103, the others rates like 12.50
    text = re.sub(r"%\+?\.(\d)f", lambda m: "103" if m.group(1) == "0" else f"{12.5:.{m.group(1)}f}", text)
    return re.sub(r"%s", lambda _: samples.pop(0) if samples else NAME, text).replace("%%", "%")


def em(text):
    return sum(1.0 if unicodedata.east_asian_width(c) in "WF" else 0.5 for c in text)


def words(part, line):
    """the lines of a paragraph that is broken at its spaces"""
    count, used = 1, 0.0
    for word in part.split(" "):
        if used and used + 0.5 + em(word) > line:
            count, used = count + 1, em(word)
        else:
            used += (0.5 if used else 0.0) + em(word)
    return count


def lines(text, line, lang):
    parts = text.strip("\n").split("\n")
    if lang in BY_LETTER:
        return sum(max(1, math.ceil(em(part) / line)) for part in parts)
    return sum(words(part, line) for part in parts)


# the place: (the lines it may take in Korean, the text as the plugin makes it)
PLACES = {
    "the staff switch (automatic management)": (
        2,
        lambda g: t("RE_MSG_MOD", g) + t("RE_MSG_AUTO_TIP_ALL", g) + t("RE_MSG_AUTO_TIP", g) + t("RE_MSG_AUTO_TIP_STAFF", g),
    ),
    "an advert icon": (2, lambda g: t("RE_MSG_ADVERTS_TIP", g, FEE) + t("RE_MSG_ADVERTS_TIP_ALL", g)),
    "an advert icon, the mod has the adverts": (2, lambda g: t("RE_MSG_ADVERTS_TIP_ON", g, FEE)),
    "an advert icon, the mod has the whole business": (2, lambda g: t("RE_MSG_ADVERTS_TIP_WHOLE", g)),
    "the experience window's question mark": (2, lambda g: t("RE_MSG_XP_TIP", g)),
    "the box for worn-out assets, on debt": (2, lambda g: t("RE_MSG_ASSET_TIP_DEBT", g)),
    "the box for worn-out assets, cash only": (2, lambda g: t("RE_MSG_ASSET_TIP_CASH", g)),
    "the futures tab's expected rate": (2, lambda g: t("RE_MSG_FUT_TIP", g)),
    "a row of the futures list": (2, lambda g: t("RE_MSG_FUT_ROW_TIP", g)),
    "a wage icon of the hire tab": (2, lambda g: t("RE_MSG_STAFF_TIP", g, MONEY)),
    "an item of a business's purchase window": (2, lambda g: t("RE_MSG_WARE_HOUR", g, MONEY)),
    "a hand-over's place before the household's first listing": (2, lambda g: t("RE_MSG_AUTO_CLOSED_TIP", g)),
    "a business type's estimate": (2, lambda g: t("RE_MSG_TYPE_MONTH_TIP", g, ESTIMATE, ESTIMATE)),
    "a business type's estimate, contracts only": (2, lambda g: t("RE_MSG_TYPE_CONTRACTS_TIP", g)),
}


def main():
    line = 6.0 if "--control" in sys.argv else LINE
    over = 0
    for label, (allowed, make) in PLACES.items():
        try:
            counts = {lang: lines(make(lang), line, lang) for lang in LANGS}
        except KeyError as missing:  # dump_texts.exe is of another build than re_lang.h
            print(f"  {label}: the message {missing} is not among the built texts")
            over += 1
            continue
        wrong = {lang: n for lang, n in counts.items() if n > allowed + (MORE if lang != "ko" else 0)}
        over += bool(wrong)
        if wrong and "--control" not in sys.argv:
            worst = max(wrong, key=wrong.get)
            print(f"  over {allowed} line(s) (+{MORE} outside Korean): {label}: {wrong}; '{make(worst).strip()}' is {em(make(worst).strip()):.1f} em")
        elif "--list" in sys.argv:
            print(f"  {label}: ko {counts['ko']} line(s), {em(make('ko').strip()):.1f} em; the most {max(counts.values())} ({max(counts, key=counts.get)})")
    if "--list" in sys.argv:
        for name in sorted(OTHER & set(TEXT)):
            text = t(name, "ko").strip()
            print(f"  (other) {name}: ko {em(text):.1f} em: '{text}'")
    unplaced = sorted(n for n in TEXT if re.search(r"TIP|HOVER", n) and n not in used and n not in OTHER)
    if unplaced:
        print(f"  in no place of this check: {', '.join(unplaced)}")
    ok = over == 0 and not unplaced
    print(f"hover texts places={len(PLACES)} over={over} unplaced={len(unplaced)} line={line:.0f} verdict={'PASS' if ok else 'FAIL'}")
    return 0 if ok else 1


# hover texts that are pieces of one line each, put together by what the row is: the list of shares (D-044: one line,
# a second with a warning, a third for a researched share), the casino's chances (D-045), the two lines of a contract
# offer (D-055), the switch of the research subscription. --list writes their widths.
OTHER = {
    "RE_MSG_HOVER_YIELD", "RE_MSG_HOVER_UNDER", "RE_MSG_HOVER_BANKRUPT", "RE_MSG_HOVER_MEASURE", "RE_MSG_HOVER_WATCH",
    "RE_MSG_HOVER_NEAR", "RE_MSG_HOVER_FAIR", "RE_MSG_HOVER_TAILWIND", "RE_MSG_HOVER_HEADWIND", "RE_MSG_HOVER_CHANGE",
    "RE_MSG_HOVER_SUB_OFF", "RE_MSG_HOVER_SUB_ON", "RE_MSG_HOVER_SUB_ALL", "RE_MSG_HOVER_SUB_BOARD", "RE_MSG_HOVER_ALL_ON",
    "RE_MSG_HOVER_ALL_OFF", "RE_MSG_CASINO_TIP", "RE_MSG_CASINO_TIP_PRIZE", "RE_MSG_OFFER_TIP",
}  # fmt: skip

if __name__ == "__main__":
    sys.exit(main())
