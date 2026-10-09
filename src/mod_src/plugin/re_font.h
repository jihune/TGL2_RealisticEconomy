/* A letter the game's font does not have (REQUEST.md [47], note m29).
 *
 * The game draws its texts with one font file a language (fonts/NotoSansKR-Regular.ttf for Korean) and its lists in
 * fixed-width type with another (fonts/NanumGothicCoding-Regular.ttf). The names of people come from name lists of
 * many countries (extraData/namesPeople), and a save keeps lines in the language it was played in. A letter the
 * font of the moment has no picture for is drawn as that font's "missing" picture, a box with a cross.
 *
 * The drawing is libcocos2d.dll's (cocos2d-x 3.17): FontFreeType::getGlyphBitmap hands the picture of one letter to
 * the atlas of a font, and asks FreeType for it whether the font has the letter or not. Here that function is
 * stood in for: a letter that the font's own file does not list is asked of another of the game's font files, at
 * the same size and with the same outline, and that picture goes into the atlas. Which letters a file has is read
 * from the file's own table of them (cmap). */
#ifndef RE_FONT_H
#define RE_FONT_H

#include <windows.h>

#define RE_FONT_BMP_BYTES 8192 /* one bit a code point below U+10000 */

/* The code points below U+10000 that an OpenType table of characters (cmap) gives a picture: bits set in `bits`.
 * Returns how many, -1 when the table is not understood. */
int re_font_cmap(const BYTE *cmap, unsigned len, unsigned char bits[RE_FONT_BMP_BYTES]);

/* The same for a font file (.ttf, .otf): only its directory and that table are read. -1 when it cannot be read. */
int re_font_file(const wchar_t *path, unsigned char bits[RE_FONT_BMP_BYTES]);

/* Stands in for the two functions of libcocos2d.dll. `trace` logs every letter taken from another font, else the
 * first few. Returns 1 when both are in place, 0 when the library or the functions are not as expected (nothing is
 * changed then), -1 when the library is not loaded yet. */
int re_font_install(int trace);

/* A name no font file of the game can write (REQUEST.md [48]). Three lines of the game's lists of names are in
 * Arabic letters, and none of the font files has one of those; the list they are in has the same three names in
 * Latin letters too. Writes `text` to `out` with each of the three in Latin letters, wherever in the text it is,
 * and a 0 behind it. Returns the new length; 0 when the text has none of them or nothing of it would be left (`out`
 * is not to be used then), -1 when `out` is too small. */
int re_font_name(const char *text, unsigned len, char *out, unsigned cap);

#endif
