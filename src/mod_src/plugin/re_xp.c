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
