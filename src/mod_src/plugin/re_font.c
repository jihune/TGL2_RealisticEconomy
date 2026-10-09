#include "re_font.h"

#include <stdio.h>
#include <string.h>

#include "re_hook.h"
#include "re_util.h"

/* ------------------------------------------------------------- the table of characters of a font file */

static unsigned be16(const BYTE *p)
{
    return (unsigned)p[0] << 8 | p[1];
}

static unsigned be32(const BYTE *p)
{
    return (unsigned)p[0] << 24 | (unsigned)p[1] << 16 | (unsigned)p[2] << 8 | p[3];
}

static int set_bit(unsigned char *bits, unsigned code)
{
    unsigned char mask = (unsigned char)(1u << (code & 7));
    if (bits[code >> 3] & mask)
        return 0;
    bits[code >> 3] |= mask;
    return 1;
}

/* format 4: segments of the basic plane; a glyph number of 0 is "no picture" */
static int cmap_segments(const BYTE *sub, unsigned len, unsigned char *bits)
{
    if (len < 16)
        return -1;
    unsigned count = be16(sub + 6) / 2, found = 0;
    if (16 + count * 8 > len)
        return -1;
    const BYTE *ends = sub + 14, *starts = ends + count * 2 + 2, *deltas = starts + count * 2, *offsets = deltas + count * 2;
    for (unsigned i = 0; i < count; i++) {
        unsigned first = be16(starts + i * 2), last = be16(ends + i * 2), delta = be16(deltas + i * 2), offset = be16(offsets + i * 2);
        if (first > last)
            continue;
        for (unsigned code = first; code <= last && code < 0xffff; code++) {
            unsigned glyph;
            if (offset == 0)
                glyph = (code + delta) & 0xffff;
            else {
                const BYTE *at = offsets + i * 2 + offset + (code - first) * 2;
                if (at + 2 > sub + len)
                    return -1;
                glyph = be16(at);
                glyph = glyph ? (glyph + delta) & 0xffff : 0;
            }
            if (glyph != 0)
                found += (unsigned)set_bit(bits, code);
        }
    }
    return (int)found;
}

/* format 12: groups of code points of any plane */
static int cmap_groups(const BYTE *sub, unsigned len, unsigned char *bits)
{
    if (len < 16)
        return -1;
    unsigned count = be32(sub + 12), found = 0;
    if (count > (len - 16) / 12)
        return -1;
    for (unsigned i = 0; i < count; i++) {
        const BYTE *group = sub + 16 + i * 12;
        unsigned first = be32(group), last = be32(group + 4), glyph = be32(group + 8);
        for (unsigned code = first; code <= last && code < 0x10000; code++)
            if (glyph + (code - first) != 0)
                found += (unsigned)set_bit(bits, code);
    }
    return (int)found;
}

int re_font_cmap(const BYTE *cmap, unsigned len, unsigned char bits[RE_FONT_BMP_BYTES])
{
    memset(bits, 0, RE_FONT_BMP_BYTES);
    if (len < 4)
        return -1;
    unsigned tables = be16(cmap + 2), best = 0, best_rank = 0;
    if (4 + tables * 8 > len)
        return -1;
    /* Unicode tables only: the full one of Windows (3, 10) or of Unicode (0, 4 and up), else the basic plane's */
    for (unsigned i = 0; i < tables; i++) {
        const BYTE *record = cmap + 4 + i * 8;
        unsigned platform = be16(record), encoding = be16(record + 2), offset = be32(record + 4), rank = 0;
        if (offset + 4 > len)
            continue;
        unsigned format = be16(cmap + offset);
        if (format == 12 && ((platform == 3 && encoding == 10) || platform == 0))
            rank = 3;
        else if (format == 4 && ((platform == 3 && encoding == 1) || platform == 0))
            rank = 2;
        if (rank > best_rank) {
            best_rank = rank;
            best = offset;
        }
    }
    if (best_rank == 0)
        return -1;
    return best_rank == 3 ? cmap_groups(cmap + best, len - best, bits) : cmap_segments(cmap + best, len - best, bits);
}

static int read_at(HANDLE file, unsigned offset, BYTE *out, unsigned count)
{
    DWORD got = 0;
    return SetFilePointer(file, (LONG)offset, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER && ReadFile(file, out, count, &got, NULL) &&
           got == count;
}

int re_font_file(const wchar_t *path, unsigned char bits[RE_FONT_BMP_BYTES])
{
    BYTE head[12], record[16], *table = NULL;
    int found = -1;
    HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
        return -1;
    if (read_at(file, 0, head, sizeof head)) {
        unsigned tables = be16(head + 4);
        for (unsigned i = 0; i < tables && i < 256 && table == NULL; i++) {
            if (!read_at(file, 12 + i * 16, record, sizeof record))
                break;
            if (memcmp(record, "cmap", 4) != 0)
                continue;
            unsigned offset = be32(record + 8), len = be32(record + 12);
            if (len == 0 || len > 16u * 1024 * 1024)
                break;
            table = (BYTE *)HeapAlloc(GetProcessHeap(), 0, len);
            if (table != NULL && read_at(file, offset, table, len))
                found = re_font_cmap(table, len, bits);
        }
    }
    if (table != NULL)
        HeapFree(GetProcessHeap(), 0, table);
    CloseHandle(file);
    return found;
}

/* ------------------------------------------------------------- a name no font file can write */

/* The lines of extraData/namesPeople with letters that none of the game's font files has: three of the 680 lines of
 * IRFirstNamesM.txt (engine test T26 reads every list and every font file and finds no other). That list has
 * "Arash", "Alishah" and "Mohammad" as lines of their own, so a person keeps a name of the same list. */
static const struct {
    const char *with, *plain;
} NAME[] = {
    {" (\xd8\xa2\xd8\xb1\xd8\xb4)", ""}, /* the line "Arash (...)": U+0622 U+0631 U+0634 in brackets behind the name */
    {"\xd8\xb9\xd8\xa7\xd9\x84\xdb\x8c\xd8\xb4\xd8\xa7\xd9\x87", "Alishah"}, /* U+0639 0627 0644 06CC 0634 0627 0647 */
    {"\xef\xb7\xb4", "Mohammad"},                                             /* U+FDF4: the name as one letter */
};
#define NAMES (sizeof NAME / sizeof NAME[0])

int re_font_name(const char *text, unsigned len, char *out, unsigned cap)
{
    unsigned at = 0, n = 0;
    int changed = 0;
    while (at < len && (unsigned char)text[at] < 0xd8) /* each of the three has a byte of 0xd8 or more */
        at++;
    if (at == len)
        return 0;
    for (at = 0; at < len;) {
        size_t i = 0, with = 1;
        for (; i < NAMES; i++) {
            with = strlen(NAME[i].with);
            if (len - at >= with && memcmp(text + at, NAME[i].with, with) == 0)
                break;
        }
        const char *from = i < NAMES ? NAME[i].plain : text + at;
        size_t add = i < NAMES ? strlen(NAME[i].plain) : 1;
        if (n + add >= cap)
            return -1;
        memcpy(out + n, from, add);
        n += (unsigned)add;
        at += i < NAMES ? (unsigned)with : 1;
        changed += i < NAMES;
    }
    out[n] = 0;
    return changed ? (int)n : 0;
}

/* ------------------------------------------------------------- the stand-ins in libcocos2d.dll */

/* cocos2d::FontFreeType, 0x68 bytes: the font's file name as it was asked for, whether its pictures are distance
 * fields, the width of its outline. `0x102d3a97 cmp byte ptr [esi + 0x3c], 0` in getGlyphBitmap is the second;
 * the name is the std::string in front of it and the outline the float after it (the class's members in the
 * order of cocos2d-x 3.17's CCFontFreeType.h; the exported getOutlineSize reads the float). */
#define FONT_BYTES 0x68
#define FONT_NAME 0x24
#define FONT_DISTANCE_FIELD 0x3c
#define FONT_OUTLINE 0x40

/* a std::string of the Visual C++ library: the text in place while it is shorter than 16, else behind a pointer */
typedef struct {
    union {
        char in_place[16];
        const char *elsewhere;
    } text;
    unsigned size, room;
} cpp_string;

typedef unsigned char *(__thiscall *glyph_fn)(void *font, unsigned low, unsigned high, long *width, long *height, void *rect, int *advance);
typedef void *(__cdecl *create_fn)(const cpp_string *name, float size, int glyphs, const char *custom, int distance_field, float outline);
typedef void(__thiscall *retain_fn)(void *object);

static void *g_glyph_trampoline, *g_create_trampoline, *g_retain;
static int g_trace, g_lines, g_taken;

/* The other font files a letter is looked for in, in this order: fixed-width Noto for Latin with marks, Greek and
 * Cyrillic (its weight is the Korean, Japanese and Chinese fonts'), those three for their own scripts when the
 * language's font is another, Thai, and the English font last. */
static const char *const OTHER[] = {
    "fonts/NotoSansMono_SemiCondensed-Regular.ttf", "fonts/NotoSansKR-Regular.ttf",       "fonts/NotoSansJP-Regular.ttf",
    "fonts/NotoSansSC-Regular.ttf",                 "fonts/PlaypenSansThai-Regular.ttf", "fonts/sofiapro-light.otf",
};
#define OTHERS ((int)(sizeof OTHER / sizeof OTHER[0]))

/* which letters a font file has; the files the game names and the six above */
#define FILES 24
static struct {
    char name[96];
    int state; /* 1 read, -1 not readable */
    unsigned char bits[RE_FONT_BMP_BYTES];
} g_file[FILES];
static int g_files;

static int same_name(const char *a, const char *b)
{
    for (; *a && *b; a++, b++) {
        char x = *a == '\\' ? '/' : *a, y = *b == '\\' ? '/' : *b;
        if ((x | 0x20) != (y | 0x20) && x != y)
            return 0;
    }
    return *a == *b;
}

/* the letters of a font file by the name the game asked for it, relative to the game's folder; NULL when unknown */
static const unsigned char *letters_of(const char *name)
{
    int at = 0;
    while (at < g_files && !same_name(g_file[at].name, name))
        at++;
    if (at == g_files) {
        wchar_t path[MAX_PATH], wide[96];
        if (g_files == FILES || strlen(name) >= sizeof g_file[0].name)
            return NULL;
        g_files++;
        snprintf(g_file[at].name, sizeof g_file[at].name, "%s", name);
        g_file[at].state = -1;
        DWORD n = GetModuleFileNameW(NULL, path, MAX_PATH);
        wchar_t *slash = n && n < MAX_PATH ? wcsrchr(path, L'\\') : NULL;
        int absolute = name[0] == '/' || name[0] == '\\' || (name[0] && name[1] == ':');
        if (MultiByteToWideChar(CP_UTF8, 0, name, -1, wide, 96) > 0 && (absolute || (slash != NULL && (slash - path) + 100 < MAX_PATH))) {
            if (absolute)
                wcscpy(path, wide);
            else
                wcscpy(slash + 1, wide);
            int found = re_font_file(path, g_file[at].bits);
            g_file[at].state = found > 0 ? 1 : -1;
            re_log("font: %s has pictures for %d code point(s) below U+10000%s", name, found,
                   found > 0 ? "" : ": its table of characters is not read, its letters are left to it");
        }
    }
    return g_file[at].state == 1 ? g_file[at].bits : NULL;
}

/* what a font object was made with: the size is nowhere in the object */
#define MADE 96
static struct {
    const void *font;
    float size;
} g_made[MADE];
static int g_made_next;

static void made_note(const void *font, float size)
{
    int at = 0;
    while (at < MADE && g_made[at].font != font)
        at++;
    if (at == MADE) {
        at = g_made_next;
        g_made_next = (g_made_next + 1) % MADE;
    }
    g_made[at].font = font;
    g_made[at].size = size;
}

static float made_size(const void *font)
{
    for (int at = 0; at < MADE; at++)
        if (g_made[at].font == font)
            return g_made[at].size;
    return 0.0f;
}

/* the font objects made here: one a file, size, kind and outline, kept for good */
#define SPARES 48
static struct {
    int other, distance_field;
    float size, outline;
    void *font;
} g_spare[SPARES];
static int g_spares;

static void *spare_font(int other, float size, int distance_field, float outline)
{
    for (int i = 0; i < g_spares; i++)
        if (g_spare[i].other == other && g_spare[i].distance_field == distance_field && g_spare[i].size == size &&
            g_spare[i].outline == outline)
            return g_spare[i].font;
    if (g_spares == SPARES)
        return NULL;
    cpp_string name;
    name.text.elsewhere = OTHER[other];
    name.size = (unsigned)strlen(OTHER[other]);
    name.room = name.size > 31 ? name.size : 31; /* 16 or more: the text is behind the pointer; the callee only reads it */
    void *font = ((create_fn)g_create_trampoline)(&name, size, 0, NULL, distance_field, outline);
    if (font != NULL)
        ((retain_fn)g_retain)(font); /* create hands out an object that is let go at the end of the frame */
    g_spare[g_spares].other = other;
    g_spare[g_spares].distance_field = distance_field;
    g_spare[g_spares].size = size;
    g_spare[g_spares].outline = outline;
    g_spare[g_spares++].font = font; /* NULL too: a file that cannot be made into a font is not tried again */
    re_log("font: %s at size %.1f, outline %.1f%s: %s", OTHER[other], size, outline, distance_field ? ", distance field" : "",
           font != NULL ? "made, for the letters other fonts do not have" : "COULD NOT be made");
    return font;
}

/* the font object to ask for `code` in place of `font`, NULL when `font` has the letter or nothing better is known */
static void *other_font(const void *font, unsigned code)
{
    const BYTE *f = (const BYTE *)font;
    if (!re_readable(f, FONT_BYTES))
        return NULL;
    const cpp_string *name = (const cpp_string *)(const void *)(f + FONT_NAME);
    const char *file = name->room >= 16 ? name->text.elsewhere : name->text.in_place;
    if (name->size == 0 || name->size >= 96 || !re_readable(file, name->size + 1) || file[name->size] != 0)
        return NULL;
    const unsigned char *own = letters_of(file);
    if (own == NULL || (own[code >> 3] & (1u << (code & 7))))
        return NULL;
    float size = made_size(font), outline = *(const float *)(const void *)(f + FONT_OUTLINE);
    if (!(size > 0.0f))
        return NULL;
    for (int i = 0; i < OTHERS; i++) {
        if (same_name(OTHER[i], file))
            continue;
        const unsigned char *has = letters_of(OTHER[i]);
        if (has == NULL || !(has[code >> 3] & (1u << (code & 7))))
            continue;
        void *spare = spare_font(i, size, f[FONT_DISTANCE_FIELD] != 0, outline);
        if (spare == NULL)
            continue;
        g_taken++;
        if (g_lines < (g_trace ? 600 : 24)) {
            g_lines++;
            re_log("font: U+%04X is not in %s (size %.1f): its picture is taken from %s", code, file, size, OTHER[i]);
        }
        return spare;
    }
    return NULL;
}

__attribute__((force_align_arg_pointer)) static unsigned char *__fastcall glyph_wrapper(void *font, void *unused, unsigned low, unsigned high,
                                                                                         long *width, long *height, void *rect, int *advance)
{
    static int inside; /* a font made here is asked as it is */
    (void)unused;
    void *other = NULL;
    if (!inside && high == 0 && low > 0x7e && low < 0xffff) {
        inside = 1;
        other = other_font(font, low);
        inside = 0;
    }
    return ((glyph_fn)g_glyph_trampoline)(other != NULL ? other : font, low, high, width, height, rect, advance);
}

__attribute__((force_align_arg_pointer)) static void *__cdecl create_wrapper(const cpp_string *name, float size, int glyphs,
                                                                             const char *custom, int distance_field, float outline)
{
    void *font = ((create_fn)g_create_trampoline)(name, size, glyphs, custom, distance_field, outline);
    if (font != NULL)
        made_note(font, size);
    return font;
}

int re_font_install(int trace)
{
    /* `push ebp / mov ebp, esp / sub esp, 0x64` and `push 4 / mov eax, <the frame's handler>`: whole instructions,
     * and the second's address is the loader's already */
    static const BYTE glyph_entry[6] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x64}, create_entry[3] = {0x6a, 0x04, 0xb8};
    HMODULE cocos = GetModuleHandleW(L"libcocos2d.dll");
    if (cocos == NULL)
        return -1;
    BYTE *glyph = (BYTE *)(UINT_PTR)GetProcAddress(cocos, "?getGlyphBitmap@FontFreeType@cocos2d@@QAEPAE_KAAJ1AAVRect@2@AAH@Z");
    BYTE *create = (BYTE *)(UINT_PTR)GetProcAddress(
        cocos, "?create@FontFreeType@cocos2d@@SAPAV12@ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@MW4GlyphCollection@2@PBD_NM@Z");
    g_retain = (void *)(UINT_PTR)GetProcAddress(cocos, "?retain@Ref@cocos2d@@QAEXXZ");
    g_trace = trace;
    if (glyph == NULL || create == NULL || g_retain == NULL || !re_bytes_equal(glyph, glyph_entry, sizeof glyph_entry) ||
        !re_bytes_equal(create, create_entry, sizeof create_entry)) {
        re_log("font: libcocos2d.dll is not the one this was written for (getGlyphBitmap %p, create %p, retain %p, or their first bytes): "
               "NOT installed",
               (void *)glyph, (void *)create, g_retain);
        return 0;
    }
    BYTE moved[7];
    memcpy(moved, create, sizeof moved);
    int made = re_detour(create, moved, sizeof moved, create_wrapper, &g_create_trampoline);
    int drawn = made && re_detour(glyph, glyph_entry, sizeof glyph_entry, glyph_wrapper, &g_glyph_trampoline);
    re_log("font: a letter a font does not have is taken from another of the game's fonts: the making of a font %s, the picture of a "
           "letter %s",
           made ? "redirected" : "NOT redirected", drawn ? "redirected" : "NOT redirected");
    return made && drawn;
}
