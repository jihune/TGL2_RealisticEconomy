/* Detour for the game's translation function (TGL2.exe 0x0081d7c0): every text a window shows is fetched through it.
 *
 * Convention of the original: ecx = text manager, stack = pointer to the result std::string, then the key and ten
 * replacement values as std::string by value (11 x 24 bytes), `ret 0x10c`.
 *
 * The detour asks `re_text_want` about the key. For a wanted key it lets the original run, then hands the finished
 * text to `re_text_edit`, which may change it. Other keys pass through untouched. This is how text is added to
 * windows the game already has, without creating any window.
 */
#ifndef RE_TEXT_H
#define RE_TEXT_H

/* The registers of the window code at its call, in the order `pushad` stores them. The translation function has not
 * run its prologue yet, so ebp is the caller's frame: what the caller keeps there (the company a row is about, the
 * debt a tooltip is about) can be read from it. */
typedef struct {
    unsigned edi, esi, ebp, esp, ebx, edx, ecx, eax;
} re_text_regs;

/* What the game's call wants put into the text: the ten strings behind the key are five names ("LOCATIONNAME")
 * each with its value. The original removes every "{{" and "}}" from the key's text and then replaces each name by
 * its value, in this order. `count` pairs have a name; `whole` is 0 when a name or a value did not fit its buffer
 * here and that pair was left out. */
#define RE_TEXT_PAIRS 5
typedef struct {
    int count, whole;
    char name[RE_TEXT_PAIRS][32];
    char value[RE_TEXT_PAIRS][128];
} re_text_values;

/* Non-zero = edit this key's text; the value is handed to re_text_edit. `caller` is the address the game's call
 * returns to, which tells the windows apart. Must be cheap: it runs for every text. */
typedef int (*re_text_want_fn)(const char *key, unsigned len, unsigned caller, const re_text_regs *regs);
/* `text` holds `len` bytes in a buffer of `cap`; returns the new length. A text left as it was is not written back.
 * `regs` are the ones the wanted call came with; the caller's frame is still standing. `values` are the names and
 * values that call came with. */
typedef unsigned (*re_text_edit_fn)(int want, char *text, unsigned len, unsigned cap, const re_text_regs *regs,
                                    const re_text_values *values);

extern void *re_text_trampoline; /* filled by re_detour */
extern re_text_want_fn re_text_want;
extern re_text_edit_fn re_text_edit;
extern void *re_text_assign; /* the game's std::string assign(ptr, len): thiscall, two stack arguments */

void re_text_hook(void); /* entry for the detour; not callable from C */

/* Detour for the function that builds one row "label, value" of a tab of the stocks window (0x007b6d30): ecx = the
 * window, stack = layer, position, a flag that paints the row red, then the label and the value as std::string by
 * value (`ret 0x3c`). `re_text_row` sees the two strings and the flag before the game uses them and may change them
 * in place: the strings through `re_text_assign`, the flag by writing 0 or 1. `caller` and `regs` as above. */
typedef void (*re_text_row_fn)(unsigned caller, const re_text_regs *regs, void *label, void *value, unsigned *red);
extern void *re_text_row_trampoline;
extern re_text_row_fn re_text_row;
void re_text_row_hook(void);

#endif
