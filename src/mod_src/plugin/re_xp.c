#include "re_xp.h"

#include <stdlib.h>
#include <string.h>

static const char *find(const char *from, const char *end, const char *needle)
{
    size_t n = strlen(needle);
    for (; from + n <= end; from++)
        if (memcmp(from, needle, n) == 0)
            return from;
    return NULL;
}

/* the value of ` name="..."` inside [from, end); NULL when the attribute is not there */
static const char *attribute(const char *from, const char *end, const char *name)
{
    size_t n = strlen(name);
    for (const char *p = from; p + n + 2 <= end; p++)
        if ((p[-1] == ' ' || p[-1] == '\t' || p[-1] == '\n' || p[-1] == '\r') && memcmp(p, name, n) == 0 && p[n] == '=' && p[n + 1] == '"')
            return p + n + 2;
    return NULL;
}

int re_xp_scan(const char *xml, unsigned len, re_xp_need *need, int cap, int *count)
{
    const char *p = xml, *end = xml + len;
    int found = 0;
    while (p < end) {
        const char *comment = find(p, end, "<!--"), *tag = find(p, end, "<xpReq");
        if (tag == NULL)
            break;
        if (comment != NULL && comment < tag) { /* what a comment holds is not data */
            const char *close = find(comment + 4, end, "-->");
            if (close == NULL)
                break;
            p = close + 3;
            continue;
        }
        const char *close = find(tag, end, ">");
        if (close == NULL)
            break;
        p = close + 1;
        if (tag[6] != ' ' && tag[6] != '\t' && tag[6] != '\n' && tag[6] != '\r')
            continue; /* <xpReqGroup .../> */
        const char *id = attribute(tag + 6, close, "id"), *amt = attribute(tag + 6, close, "amt");
        if (id == NULL || amt == NULL)
            continue;
        int which = atoi(id), most = (int)(atof(amt) + 0.5), at = 0;
        if (which <= 0 || most <= 0)
            continue;
        found++;
        while (at < *count && need[at].tag != which)
            at++;
        if (at == *count) {
            if (at == cap)
                return -1;
            need[at].tag = which;
            need[at].most = 0;
            (*count)++;
        }
        if (most > need[at].most)
            need[at].most = most;
    }
    return found;
}

int re_xp_educations(const char *xml, unsigned len, int *tags, int cap, int *count)
{
    const char *p = xml, *end = xml + len;
    int found = 0;
    while (p < end) {
        const char *comment = find(p, end, "<!--"), *open = find(p, end, "<education>");
        if (open == NULL)
            break;
        if (comment != NULL && comment < open) {
            const char *close = find(comment + 4, end, "-->");
            if (close == NULL)
                break;
            p = close + 3;
            continue;
        }
        const char *stop = find(open, end, "</education>"), *id = NULL;
        if (stop == NULL)
            break;
        p = stop + 12;
        for (const char *q = open + 11; q < stop && id == NULL;) { /* the element's first <id> that is no comment's */
            const char *inner = find(q, stop, "<!--"), *mark = find(q, stop, "<id>");
            if (mark == NULL)
                break;
            if (inner == NULL || mark < inner) {
                id = mark + 4;
                break;
            }
            const char *close = find(inner + 4, stop, "-->");
            if (close == NULL)
                break;
            q = close + 3;
        }
        int which = id != NULL ? atoi(id) : 0, at = 0;
        if (which <= 0)
            continue; /* the template of the data has 00000 */
        found++;
        while (at < *count && tags[at] != which)
            at++;
        if (at == *count) {
            if (at == cap)
                return -1;
            tags[(*count)++] = which;
        }
    }
    return found;
}

int re_xp_most(const re_xp_need *need, int count, int tag)
{
    for (int i = 0; i < count; i++)
        if (need[i].tag == tag)
            return need[i].most;
    return 0;
}

long long re_xp_kept(long long decayed, long long gained, int most)
{
    long long floor = gained < most ? gained : most;
    return decayed > floor ? decayed : floor;
}

/* the first `needle` in [from, end) that is no comment's; NULL when there is none */
static const char *find_data(const char *from, const char *end, const char *needle)
{
    while (from < end) {
        const char *comment = find(from, end, "<!--"), *hit = find(from, end, needle);
        if (hit == NULL || comment == NULL || hit < comment)
            return hit;
        const char *close = find(comment + 4, end, "-->");
        if (close == NULL)
            return NULL;
        from = close + 3;
    }
    return NULL;
}

/* the `<xpReq>` and the `<xpReqGroup>` in [from, end) into `job`; -1 when it has more than the entry takes */
static int reqs_read(const char *from, const char *end, re_xp_job *job)
{
    for (const char *tag; (tag = find_data(from, end, "<xpReq")) != NULL;) {
        const char *close = find(tag, end, ">");
        if (close == NULL)
            break;
        from = close + 1;
        if (tag[6] == 'G') {
            const char *id = attribute(tag + 11, close, "id");
            job->group = id != NULL ? atoi(id) : 0;
            continue;
        }
        const char *id = attribute(tag + 6, close, "id"), *amt = attribute(tag + 6, close, "amt"), *prob = attribute(tag + 6, close, "prob");
        int which = id != NULL ? atoi(id) : 0;
        if (which <= 0)
            continue;
        if (job->count == RE_XP_JOB_REQS)
            return -1;
        double most = amt != NULL ? atof(amt) : 0.0, part = prob != NULL ? atof(prob) : 0.0;
        job->req[job->count++] = (re_xp_req){which, most != 0.0 ? most : 1.0, part != 0.0 ? part : 1.0};
    }
    return 0;
}

/* `entry` into the list, in place of an earlier one of the same id (a later file of the data overrides) */
static int entry_keep(re_xp_job *list, int cap, int *count, const re_xp_job *entry)
{
    int at = 0;
    while (at < *count && list[at].id != entry->id)
        at++;
    if (at == cap)
        return -1;
    list[at] = *entry;
    *count += at == *count;
    return 0;
}

int re_xp_jobs_scan(const char *xml, unsigned len, int career, re_xp_job *jobs, int cap, int *count, re_xp_job *groups, int group_cap,
                    int *group_count)
{
    const char *end = xml + len;
    int found = 0;
    for (const char *p = xml, *open; (open = find_data(p, end, "<job>")) != NULL;) {
        const char *stop = find(open, end, "</job>"), *id = stop != NULL ? find_data(open, stop, "<id>") : NULL;
        if (stop == NULL)
            break;
        p = stop + 6;
        re_xp_job job = {id != NULL ? atoi(id + 4) : 0, 0, career, 0, {{0, 0.0, 0.0}}, 0, {0}};
        const char *list = find_data(open, stop, "<requires>"), *done = list != NULL ? find(list, stop, "</requires>") : NULL;
        if (job.id <= 0)
            continue;
        for (const char *gain = open; job.gains < RE_XP_JOB_GAINS && (gain = find_data(gain, stop, "<xpGain")) != NULL;) {
            const char *close = find(gain, stop, ">"), *kind = close != NULL ? attribute(gain + 7, close, "id") : NULL;
            if (close == NULL)
                break;
            gain = close + 1;
            if (kind != NULL && atoi(kind) > 0)
                job.gain[job.gains++] = atoi(kind);
        }
        if ((done != NULL && reqs_read(list, done, &job) < 0) || entry_keep(jobs, cap, count, &job) < 0)
            return -1;
        found++;
    }
    for (const char *p = xml, *open; (open = find_data(p, end, "<tag>")) != NULL;) {
        const char *stop = find(open, end, "</tag>"), *id = stop != NULL ? find_data(open, stop, "<id>") : NULL;
        if (stop == NULL)
            break;
        p = stop + 6;
        re_xp_job group = {id != NULL ? atoi(id + 4) : 0, 0, 0, 0, {{0, 0.0, 0.0}}, 0, {0}};
        const char *list = find_data(open, stop, "<group>"), *done = list != NULL ? find(list, stop, "</group>") : NULL;
        if (group.id <= 0 || done == NULL || find_data(open, stop, "\"XP Require Group\"") == NULL)
            continue;
        if (reqs_read(list, done, &group) < 0 || entry_keep(groups, group_cap, group_count, &group) < 0)
            return -1;
        found++;
    }
    return found;
}

const re_xp_job *re_xp_job_of(const re_xp_job *list, int count, int id)
{
    for (int i = 0; i < count; i++)
        if (list[i].id == id)
            return &list[i];
    return NULL;
}

double re_xp_job_share(const re_xp_job *job, const re_xp_job *groups, int group_count, long long (*have)(int tag, void *ctx), void *ctx)
{
    const re_xp_job *part[2] = {job, job->group > 0 ? re_xp_job_of(groups, group_count, job->group) : NULL};
    double share = 0.0;
    int asked = 0;
    for (int k = 0; k < 2; k++)
        for (int i = 0; part[k] != NULL && i < part[k]->count; i++) {
            const re_xp_req *r = &part[k]->req[i];
            double of = (double)have(r->tag, ctx) / r->amt;
            share += r->prob * (of > 1.0 ? 1.0 : of > 0.0 ? of : 0.0);
            asked++;
        }
    return asked > 0 ? share : 1.0; /* a job that asks nothing: the game's flag of a requirement is never set (L167, L216) */
}

static int gives(const re_xp_job *job, int tag)
{
    for (int g = 0; g < job->gains; g++)
        if (job->gain[g] == tag)
            return 1;
    return 0;
}

int re_xp_jobs_common(const re_xp_job *jobs, int count, int careers, int *out, int cap)
{
    int n = 0;
    for (int i = 0; i < count; i++)
        for (int g = 0; g < jobs[i].gains; g++) {
            int tag = jobs[i].gain[g], every = careers > 0;
            for (int k = 0; k < n && every; k++)
                every = out[k] != tag; /* counted already */
            for (int c = 1; c <= careers && every; c++) {
                int some = 0;
                for (int j = 0; j < count && !some; j++)
                    some = jobs[j].career == c && gives(&jobs[j], tag);
                every = some;
            }
            if (every && n < cap)
                out[n++] = tag;
        }
    return n;
}

int re_xp_job_follows(const re_xp_job *next, const re_xp_job *held, const re_xp_job *groups, int group_count, const int *common, int commons)
{
    const re_xp_job *part[2] = {next, next->group > 0 ? re_xp_job_of(groups, group_count, next->group) : NULL};
    if (next->career == 0 || next->career != held->career)
        return 0;
    for (int k = 0; k < 2; k++)
        for (int i = 0; part[k] != NULL && i < part[k]->count; i++) {
            int tag = part[k]->req[i].tag, everyone = 0;
            for (int c = 0; c < commons; c++)
                everyone |= common[c] == tag;
            if (!everyone && gives(held, tag))
                return 1;
        }
    return 0;
}
