/* Log file and settings file of the plugin. Both sit next to the .asi and carry its name. */
#ifndef RE_UTIL_H
#define RE_UTIL_H

#include <windows.h>

/* Opens <plugin>.log for appending and remembers <plugin>.ini. Safe to call once from DllMain. */
void re_util_init(HINSTANCE self);

/* One line, written at once: a test run ends by killing the process, so nothing may sit in a buffer of the plugin's.
 * The write is not followed to the disk (note m30). */
void re_log(const char *fmt, ...) __attribute__((format(gnu_printf, 1, 2)));

/* Settings. A missing file or key gives the fallback. */
int re_ini_int(const char *section, const char *key, int fallback);
/* Reads a comma separated list of numbers into out[0..count-1]; entries that are missing keep their fallback. */
void re_ini_doubles(const char *section, const char *key, double *out, int count);
/* Reads a comma separated list of integers; returns how many were read (at most max). */
int re_ini_ints(const char *section, const char *key, const char *fallback, int *out, int max);
/* Writes one whole number into the settings file; the rest of the file stays as it is. */
void re_ini_set_int(const char *section, const char *key, int value);

/* What the plugin remembers between sessions, in <plugin>.state: one section per playthrough, whole numbers only.
 * Nothing of this goes into a save file. */
long long re_state_get(const char *section, const char *key, long long fallback);
void re_state_set(const char *section, const char *key, long long value);

/* The game's language folder ("ko", "en", ...), read once from data/options.xml next to the game executable. */
const char *re_language(void);

/* Hands every .xml file under the game's data folder to `each`, as text that ends with a zero. Returns the files
 * read, -1 when the folder cannot be named. Mods' XML files are not looked at. */
typedef void (*re_data_each)(const char *text, unsigned len, void *ctx);
int re_data_files(re_data_each each, void *ctx);

#endif
