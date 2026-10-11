/* Every text of the plugin in every language, for analysis/scripts/ticker_fit.py: one a line,
 * <name of the message as number>\t<language folder>\t<the text, a line break written as \n>. Built by
 * mod_src/build.ps1 into mod_src/_build/dump_texts.exe. */
#include <stdio.h>

#include "../plugin/re_lang.h"

int main(void)
{
    for (int msg = 0; msg < RE_MSG_COUNT; msg++)
        for (int lang = 0; lang < RE_LANG_COUNT; lang++) {
            printf("%d\t%s\t", msg, re_lang_folder(lang));
            for (const char *p = re_lang_text(lang, msg); *p != 0; p++)
                if (*p == '\n')
                    fputs("\\n", stdout);
                else
                    fputc(*p, stdout);
            fputc('\n', stdout);
        }
    return 0;
}
