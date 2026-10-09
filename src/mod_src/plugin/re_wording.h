/* Corrections for texts of the game that say something other than the English original. Most are Korean (the
 * language file reads like machine translation in places: "Fixed" became "수정됨", "Variable" became "변수"); a few
 * are in other languages, where the mistake shows at a glance.
 *
 * A second kind is a text whose language file names a value wrongly ("{{MES}}" where the game fills in "MONTHS") or
 * has the text of another key: the game then shows the bare word or the wrong sentence, in any language.
 *
 * A text is corrected only when what the game produced is exactly its own known wording, so a language file that
 * has been fixed by someone else is left alone. Pure string work: no game access here. */
#ifndef RE_WORDING_H
#define RE_WORDING_H

#include "re_text.h"

/* 0 = this key is not corrected in this language; otherwise the entry number + 1. `lang` is a column of re_lang.h.
 * `caller_va` is the address the text request returns to, at the image's preferred base: a few keys are corrected
 * for one window only. */
int re_wording_want(int lang, const char *key, unsigned len, unsigned caller_va);

/* `text` holds the finished text (`len` bytes, buffer of `cap`). Returns the new length; the same length and an
 * untouched buffer when the text is not the game's known wording. `call` are the names and values the game's call
 * came with, or NULL: a value the game's text lost (its name is misspelt in the language file) is taken from them. */
unsigned re_wording_apply(int entry, char *text, unsigned len, unsigned cap, const re_text_values *call);

/* 1 when the table has an entry for this language */
int re_wording_has(int lang);

/* for the test that compares the table with the game's language files */
int re_wording_count(void);
int re_wording_lang(int entry);
const char *re_wording_key(int entry);
const char *re_wording_game_text(int entry);

#endif
