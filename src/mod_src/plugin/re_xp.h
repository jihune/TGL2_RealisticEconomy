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

/* What a job asks of a person (REQUEST.md [100], note b28): the `<xpReq>` of a `<job>`'s `<requires>`, and its
 * `<xpReqGroup>`, a `<tag>` of the data whose `<group>` lists more `<xpReq>` (data/misc/groupXPReq.xml). A missing
 * amt counts 1 and a missing prob 1.0, as the game reads them (FUN_0080a0c0). */
#define RE_XP_JOB_REQS 12 /* the data has at most 7 for a job and 10 for a group */
#define RE_XP_JOB_GAINS 8 /* the data has at most 5 for a job */
typedef struct {
    int tag;     /* the kind of experience, or the education */
    double amt;  /* how much of it counts in full */
    double prob; /* the share of the threshold it gives in full */
} re_xp_req;
typedef struct {
    int id;     /* the job, or the group */
    int group;  /* a job's group, 0 when it has none */
    int career; /* the career file it belongs to (one file data/jobs/career*.xml of the data), 0 when none */
    int count;
    re_xp_req req[RE_XP_JOB_REQS];
    int gains; /* the kinds of experience working the job gives, its `<xpGain>` */
    int gain[RE_XP_JOB_GAINS];
} re_xp_job;

/* Reads one XML text of the game's data: every `<job>` outside a comment with its `<id>`, what its `<requires>`
 * lists and the kinds its `<xpGain>` give into `jobs`, with `career` for the text's career file; every `<tag>` of the
 * category "XP Require Group" into `groups` (`*count` entries of `cap` each). Returns how many of either the text
 * had, -1 when a table is full. */
int re_xp_jobs_scan(const char *xml, unsigned len, int career, re_xp_job *jobs, int cap, int *count, re_xp_job *groups, int group_cap,
                    int *group_count);

/* The entry of a job or a group, NULL when the data has none. */
const re_xp_job *re_xp_job_of(const re_xp_job *list, int count, int id);

/* The share of the threshold a person reaches for a job: for each requirement of the job and of its group,
 * prob x min(1, the person's number / amt) (FUN_00434ad0 L180-L206); 1.0 or more is qualified. `have` gives the
 * person's number of a kind. */
double re_xp_job_share(const re_xp_job *job, const re_xp_job *groups, int group_count, long long (*have)(int tag, void *ctx), void *ctx);

/* The kinds of experience that jobs of every career file 1 .. `careers` give (work experience, 10505), into `out`;
 * how many. A requirement of such a kind joins no line of jobs to another. */
int re_xp_jobs_common(const re_xp_job *jobs, int count, int careers, int *out, int cap);

/* 1 when `next` is a job of the line `held` is on: a job of the same career file that asks, itself or by its group,
 * for a kind of experience working `held` gives, other than the `common` kinds (Criminal Lawyer and Business Lawyer
 * after Legal Intern, both asking for the law experience the intern gets; not Delivery Driver after Automotive
 * Apprentice, of the same file). */
int re_xp_job_follows(const re_xp_job *next, const re_xp_job *held, const re_xp_job *groups, int group_count, const int *common, int commons);

#endif
