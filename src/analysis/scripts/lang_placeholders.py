"""The placeholders of the game's own texts, language by language.

    python analysis/scripts/lang_placeholders.py [--terms WORD ...] [game folder]

How the game fills a text (FUN_0081d7c0): it takes the text of the key, removes every "{{" and "}}", and then
replaces each name it was called with ("LOCATIONNAME") by its value. So "{{LOCATIONNAME}}:{{ITEMNAME}} stolen"
and "LOCATIONNAME: ITEMNAME gestohlen" both work; what does not is a name that is not in the text as the
English has it ("{{MES}}m" for "{{MONTHS}}m" shows "MESm") and a brace that is left over ("{STAFFNAME}}").

This lists every key of a language whose text, braces removed, lacks a name of the English text or keeps a
brace, and ends with one line of counts. The control: the Spanish `timeMonthsTranslate` is such a key and
the German `itemStolen` (names without braces) is not; `control=1` says the listing has the one and not the
other.

With --terms it prints, for every short English text that has one of the words, the key and the text in
every language: the game's own word for a thing, for the translation of the mod's texts.
"""

import re
import sys
from pathlib import Path

ENTRY = re.compile(r"<([A-Za-z0-9_]+)>([^<]*)</\1>")  # a leaf, not a section
PLACE = re.compile(r"\{\{([A-Za-z0-9_]+)\}\}")


def texts(folder: Path) -> dict[str, str]:
    raw = (folder / "baseTranslate.xml").read_bytes().decode("utf-8", errors="replace")
    out: dict[str, str] = {}
    for key, text in ENTRY.findall(raw):
        out.setdefault(key, text)
    return out


def main() -> int:
    args = sys.argv[1:]
    terms: list[str] = []
    if args and args[0] == "--terms":
        args = args[1:]
        while args and not Path(args[0]).is_dir():
            terms.append(args.pop(0))
    game = Path(args[0]) if args else Path(__file__).resolve().parents[2]
    folders = sorted(
        p
        for p in (game / "modsLanguages").iterdir()
        if (p / "baseTranslate.xml").is_file()
    )
    by_lang = {p.name: texts(p) for p in folders}
    english = by_lang["en"]

    if terms:
        shown = 0
        for key, text in english.items():
            if not any(t.lower() in text.lower() for t in terms) or len(text) > 36:
                continue
            shown += 1
            print(f"{key}")
            for lang, table in by_lang.items():
                print(f"  {lang:6} {table.get(key, '(missing)')}")
        print(f"terms={len(terms)} keys_shown={shown}")
        return 0

    wrong = missing = 0
    seen: set[tuple[str, str]] = set()
    for lang, table in by_lang.items():
        if lang == "en":
            continue
        for key, text in english.items():
            names = sorted(set(PLACE.findall(text)))
            if not names:
                continue
            if key not in table:
                missing += 1
                print(f"{lang:6} {key}: missing")
                continue
            bare = table[key].replace("{{", "").replace("}}", "")
            lacks = [n for n in names if n not in bare]
            brace = "{" in bare or "}" in bare
            if lacks or brace:
                wrong += 1
                seen.add((lang, key))
                why = (f"lacks {lacks}" if lacks else "") + (
                    " a brace is left" if brace else ""
                )
                print(
                    f"{lang:6} {key}: {why.strip()}: {table[key]!r} (English {text!r})"
                )
    with_places = sum(1 for t in english.values() if PLACE.search(t))
    control = int(
        ("es", "timeMonthsTranslate") in seen and ("de", "itemStolen") not in seen
    )
    print(
        f"languages={len(by_lang)} english_texts={len(english)} with_placeholders={with_places} "
        f"wrong={wrong} missing={missing} control={control}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
