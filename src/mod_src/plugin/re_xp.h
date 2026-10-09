/* An education that is not lost to time below what it is needed for (REQUEST.md [45], [48]; note m26).
 *
 * The game keeps, for a person and a kind of experience (a tag of work experience, or an education), a total of
 * what was ever gained and a second number that requirements are measured against. Every month the second number is
 * multiplied by 1 - xpDecayRate and rounded up. A job, an education or an activity asks for an amount of a kind:
 * `<xpReq id=".." amt=".." prob=".."/>` in the game's data. An education is an `<education>` element of the data;
 * its `<id>` is the kind it gives.
 *
 * The rule, for an education only: the second number does not go below what was ever gained, up to the most that
 * any requirement asks of that kind. Above that amount it decays as the game has it. An education no requirement
 * names, and all work experience, decay as the game has it.
 * This part is arithmetic and text only; the game's memory is read elsewhere. */
#ifndef RE_XP_H
#define RE_XP_H

typedef struct {
    int tag;  /* the kind of experience */
    int most; /* the highest amount a requirement of the data asks of it */
} re_xp_need;

/* Reads the requirements out of one XML text of the game's data: every `<xpReq .../>` outside a comment that has an
 * id and an amt. `need` keeps the highest amt of a tag; `*count` entries of `cap`. Returns how many requirements
 * the text had, -1 when the table is full. */
int re_xp_scan(const char *xml, unsigned len, re_xp_need *need, int cap, int *count);

/* Reads the educations out of one XML text of the game's data: the `<id>` of every `<education>` element outside a
 * comment. `tags` keeps each once; `*count` entries of `cap`. Returns how many educations the text had, -1 when the
 * table is full. */
int re_xp_educations(const char *xml, unsigned len, int *tags, int cap, int *count);

/* The highest amount a requirement asks of `tag`; 0 when none names it. */
int re_xp_most(const re_xp_need *need, int count, int tag);

/* What the number is after a month: `decayed` is what the game made of it, `gained` what was ever gained. */
long long re_xp_kept(long long decayed, long long gained, int most);

#endif
