#include "re_board.h"

#include <stddef.h>
#include <string.h>

#include "re_asm.h"
#include "re_hook.h"
#include "re_sites.h"

re_board_scored_fn re_board_scored;
void *re_board_score, *re_board_score_free;

/* a std::list<int>: sentinel node pointer, size; a node is next, previous, id */
static int list_ids(const BYTE *list, int *out, int cap)
{
    if (!re_readable(list, 8))
        return -1;
    const BYTE *head = *(const BYTE *const *)list;
    int size = *(const int *)(list + 4), count = 0;
    if (size < 0 || size > cap || !re_readable(head, 12))
        return -1;
    for (const BYTE *node = *(const BYTE *const *)head; node != head; node = *(const BYTE *const *)node) {
        if (count == size || !re_readable(node, 12))
            return -1;
        out[count++] = *(const int *)(node + 8);
    }
    return count == size ? count : -1;
}

int re_board_nominees(const BYTE *company, int *out, int cap)
{
    return list_ids(company + RE_COMPANY_NOMINEES, out, cap);
}

int re_board_members(const BYTE *company, int *out, int cap)
{
    return list_ids(company + RE_COMPANY_BOARD_MEMBERS, out, cap);
}

int re_board_household(const BYTE *company, int *out, int cap)
{
    if (!re_readable(company + RE_COMPANY_PEOPLE, 4))
        return -1;
    const BYTE *people = *(const BYTE *const *)(company + RE_COMPANY_PEOPLE);
    if (!re_readable(people + RE_PEOPLE_HOUSEHOLD, 8))
        return -1;
    const int *begin = *(const int *const *)(people + RE_PEOPLE_HOUSEHOLD), *end = *(const int *const *)(people + RE_PEOPLE_HOUSEHOLD + 4);
    if (end < begin || end - begin > cap || (end != begin && !re_readable(begin, (SIZE_T)(end - begin) * sizeof *begin)))
        return -1;
    memcpy(out, begin, (size_t)(end - begin) * sizeof *begin);
    return (int)(end - begin);
}

int re_board_seats(float fraction, double per_seat)
{
    if (!(per_seat > 0.0) || !(fraction > 0.0f))
        return 0;
    double seats = (double)fraction / per_seat + 1e-6; /* 0.2f is a hair above 0.2, 0.4f a hair below 0.4 */
    return seats >= RE_BOARD_SEATS ? RE_BOARD_SEATS : (int)seats;
}

static int among(const int *ids, int count, int id)
{
    for (int i = 0; i < count; i++)
        if (ids[i] == id)
            return 1;
    return 0;
}

int re_board_place(const int *nominees, int count, const int *household, int members, int person)
{
    if (!among(household, members, person))
        return -1;
    int place = 0;
    for (int i = 0; i < count; i++) {
        if (nominees[i] == person)
            return place;
        place += among(household, members, nominees[i]);
    }
    return -1;
}

float re_board_total(float top, int place)
{
    return top - (float)place;
}

int re_board_totals(BYTE *company, const int *nominees, int count, int person, float own, float *totals)
{
    typedef void *(__thiscall *score_fn)(void *company, void *score, int person);
    typedef void(__thiscall *free_fn)(void *score);
    if (re_board_score == NULL || re_board_score_free == NULL)
        return 0;
    for (int i = 0; i < count; i++) {
        if (nominees[i] == person) {
            totals[i] = own;
            continue;
        }
        BYTE scratch[RE_SCORE_BYTES]; /* raw: the game's function constructs its result itself */
        ((score_fn)re_board_score)(company, scratch, nominees[i]);
        memcpy(&totals[i], scratch + RE_SCORE_TOTAL, sizeof totals[i]);
        ((free_fn)re_board_score_free)(scratch);
    }
    return 1;
}

int re_board_rank(const int *nominees, const float *totals, int count, const int *household, int members, int seats, int person)
{
    int place = re_board_place(nominees, count, household, members, person), own = 0;
    if (place < 0 || seats <= 0)
        return -1;
    for (int i = 0; i < count; i++)
        own += among(household, members, nominees[i]);
    if (own <= seats || totals == NULL)
        return place < seats ? place : -1;
    /* the lowest total that is elected with nothing forced: the fifth highest of the distinct ones */
    float cut = 0.0f;
    for (int k = 0; k < RE_BOARD_SEATS; k++) {
        int found = 0;
        float best = 0.0f;
        for (int i = 0; i < count; i++)
            if ((k == 0 || totals[i] < cut) && (!found || totals[i] > best)) {
                best = totals[i];
                found = 1;
            }
        if (!found)
            break;
        cut = best;
    }
    int rank = 0, given = 0;
    for (int i = 0; i < count; i++)
        if (among(household, members, nominees[i]) && totals[i] >= cut) {
            if (nominees[i] == person)
                return rank;
            rank++;
        }
    for (int i = 0; i < count && given < seats; i++)
        if (among(household, members, nominees[i]) && totals[i] < cut) {
            if (nominees[i] == person)
                return rank;
            rank++;
            given++;
        }
    return -1;
}

__attribute__((force_align_arg_pointer)) void re_board_after_score(BYTE *company, BYTE *score, int person, unsigned return_address)
{
    re_board_scored_fn fn = re_board_scored;
    if (fn != NULL)
        fn(company, score, person, return_address);
}

/* The score call has the company in ecx and two stack words that the callee removes: the address of the score and
 * the person. The game's function runs first with the same arguments; then the callback gets them and the address
 * the call returns to. When this returns, eax is what the game's function returned and every other register is as
 * that function left it.
 * re_board_fraction keeps the registers the compiler expects preserved, whatever the game's function does to them. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_board_score_hook\n"
        "_re_board_score_hook:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ecx\n" /* [ebp - 4] = the company */
        "  push dword ptr [ebp + 12]\n"
        "  push dword ptr [ebp + 8]\n"
        "  call dword ptr [_re_board_score]\n"
        "  push eax\n" /* [ebp - 8] = what the game's function returned */
        "  pushad\n"
        SAVE_XMM
        "  push dword ptr [ebp + 4]\n"
        "  push dword ptr [ebp + 12]\n"
        "  push dword ptr [ebp + 8]\n"
        "  push dword ptr [ebp - 4]\n"
        "  call _re_board_after_score\n"
        "  add esp, 16\n"
        RESTORE_XMM
        "  popad\n"
        "  pop eax\n"
        "  mov esp, ebp\n"
        "  pop ebp\n"
        "  ret 8\n"
        ".globl _re_board_fraction\n"
        "_re_board_fraction:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  sub esp, 4\n"
        "  pushad\n"
        "  mov ecx, [ebp + 12]\n"
        "  push dword ptr [ebp + 16]\n"
        "  call dword ptr [ebp + 8]\n"
        "  movss [ebp - 4], xmm0\n"
        "  popad\n"
        "  fld dword ptr [ebp - 4]\n"
        "  mov esp, ebp\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");
