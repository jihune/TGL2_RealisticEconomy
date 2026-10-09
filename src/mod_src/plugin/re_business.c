#include "re_business.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

#include "re_asm.h"

long long re_business_work_cost(long long wage, int hours, double efficiency)
{
    if (wage <= 0 || hours <= 0 || !(efficiency > 0.0))
        return -1;
    return llround((double)wage / ((double)hours * efficiency));
}

int re_business_offer_value(const re_offer *offer, re_offer_value *value)
{
    double work = offer->hours + offer->induced;
    /* the labour inside M is the standard one without the policy part; the rest of M is wares and utilities */
    double paid_labour = offer->labour - offer->policy * work;
    if (offer->total <= 0 || offer->months <= 0 || !(offer->hours > 0.0) || !(offer->induced >= 0.0) || !(offer->labour > 0.0) ||
        !(paid_labour > 0.0) || !(offer->weighted > 0.0) || !(offer->x_labour > 0.0) || !(offer->x_inventory > 0.0) ||
        offer->x_inventory != offer->x_utilities)
        return 0;
    double materials = (offer->weighted - offer->x_labour * paid_labour) / offer->x_inventory;
    if (materials < 0.0) /* the game cuts every sum down to whole cents: work without materials comes out a little under 0 */
        materials = 0.0;
    double month = (double)offer->total / offer->months;
    value->materials = materials;
    value->cost = offer->labour + materials;
    value->premium = month / value->cost - 1.0;
    value->surplus = (month - value->cost) / work;
    return 1;
}

double re_business_need_cost(long long wage, double able, double need)
{
    double done = able < need ? able : need;
    return wage > 0 && done > 0.0 ? (double)wage / done : 0.0;
}

int re_business_wage_pick(const long long *wage, const int *hours, const double *efficiency, int n, int place_hours, double place_efficiency,
                          double need, double *cost)
{
    double place = place_hours * place_efficiency, wanted = need < RE_NEED_LEAST ? RE_NEED_LEAST : need;
    int pick = -1;
    for (int i = 0; i < n; i++) {
        double able = hours[i] * efficiency[i], c = re_business_need_cost(wage[i], able, wanted);
        if (!(c > 0.0) || able < RE_WAGE_PLACE_SHARE * (place < wanted ? place : wanted) || (pick >= 0 && c >= *cost))
            continue;
        pick = i;
        *cost = c;
    }
    return pick;
}

int re_business_fill_pick(const long long *wage, const int *hours, const double *efficiency, int n, double need, int clock, double *cost)
{
    int pick = -1;
    for (int i = 0; i < n; i++) {
        double c = re_business_need_cost(wage[i], (hours[i] < clock ? hours[i] : clock) * efficiency[i], need);
        if (!(c > 0.0) || (pick >= 0 && (c > *cost || (c == *cost && wage[i] >= wage[pick]))))
            continue;
        pick = i;
        *cost = c;
    }
    return pick;
}

int re_business_short(int given, int todo, double able, double share)
{
    double missing = (double)todo - able;
    return given > 0 && todo > 0 && missing >= RE_FILL_LEAST && missing > share * (double)given;
}

double re_business_asset_factor(double assets, double furnishings)
{
    return (assets < RE_ASSETS_FLOOR ? RE_ASSETS_FLOOR : assets) * (furnishings < RE_FURNISH_FLOOR ? RE_FURNISH_FLOOR : furnishings);
}

int re_business_smaller_pick(const long long *wage, const int *hours, const double *efficiency, int n, long long place_wage, int place_hours,
                             double did)
{
    int pick = -1;
    for (int i = 0; i < n; i++)
        if (wage[i] > 0 && hours[i] > 0 && hours[i] < place_hours && wage[i] < place_wage &&
            hours[i] * efficiency[i] >= RE_SMALLER_HEADROOM * did && (pick < 0 || wage[i] < wage[pick]))
            pick = i;
    return pick;
}

int re_business_wage_spare(double did, double others_left, double others_month, int mates)
{
    return mates > 0 && did >= 0.0 && others_left >= did + RE_SPARE_RESERVE * others_month;
}

double re_business_asset_hour(const re_asset_offer *offer, int missing)
{
    int counted = missing > 0 && missing < offer->power ? missing : offer->power;
    return ((offer->hours > 0.0 ? (double)offer->price / offer->hours : 0.0) + offer->running) / counted;
}

#define ASSET_ROOM_SLACK 0.001 /* the game compares floats; a unit that fits by less is taken to fit */

int re_business_asset_ware(const re_asset_offer *offer, int n, int missing, long long cash, double space)
{
    int pick = -1, pick_rank = 0, covered = 0, fits = 0;
    double pick_key = 0.0;
    for (int i = 0; i < n; i++)
        fits |= offer[i].power > 0 && offer[i].price > 0 && offer[i].space >= 0.0 && offer[i].space <= space + ASSET_ROOM_SLACK;
    for (int i = 0; i < n; i++)
        covered |= offer[i].power > 0 && offer[i].price > 0 && offer[i].price <= cash && (!fits || offer[i].space <= space + ASSET_ROOM_SLACK);
    for (int i = 0; i < n; i++) {
        const re_asset_offer *o = &offer[i];
        if (o->power <= 0 || o->price <= 0 || !(o->space >= 0.0) || (fits && o->space > space + ASSET_ROOM_SLACK))
            continue;
        double cost = (double)o->price / o->power, room = o->space / o->power;
        /* the rule that speaks for this ware, the higher the stronger, and what decides inside the rule (less is better) */
        int rank = o->owned ? 4 : o->others > 0 ? 3 : !covered ? 1 : o->price <= cash ? 2 : 0;
        double key = rank == 3 ? -(double)o->others : rank == 2 ? re_business_asset_hour(o, missing) : cost;
        if (rank == 0)
            continue; /* the cash covers another ware and not this one */
        if (pick >= 0) {
            const re_asset_offer *best = &offer[pick];
            double best_cost = (double)best->price / best->power, best_room = best->space / best->power;
            if (rank < pick_rank || (rank == pick_rank && (key > pick_key || (key == pick_key && (cost > best_cost ||
                                                                                               (cost == best_cost && room >= best_room))))))
                continue;
        }
        pick = i;
        pick_rank = rank;
        pick_key = key;
    }
    return pick;
}

int re_business_asset_why(const re_asset_want *want, double room, long long cash)
{
    if (want->owned >= want->wanted)
        return RE_ASSET_SERVED;
    if (want->unit.power <= 0 || want->unit.price <= 0)
        return RE_ASSET_NO_WARE;
    if (want->unit.space > room + ASSET_ROOM_SLACK)
        return RE_ASSET_NO_ROOM;
    return want->unit.price > cash ? RE_ASSET_NO_CASH : RE_ASSET_SERVED;
}

int re_business_asset_next(const re_asset_want *want, int n, double room, long long cash)
{
    int pick = -1;
    double best = 0.0;
    for (int i = 0; i < n; i++) {
        const re_asset_want *w = &want[i];
        if (w->owned >= w->wanted || w->wanted <= 0 || re_business_asset_why(w, room, cash) != RE_ASSET_SERVED)
            continue;
        int short_of = w->wanted - w->owned;
        double fills = (double)(w->unit.power < short_of ? w->unit.power : short_of) / w->wanted;
        double of_room = room > 0.0 ? w->unit.space / room : 0.0, of_cash = cash > 0 ? (double)w->unit.price / (double)cash : 0.0;
        double takes = of_room > of_cash ? of_room : of_cash;
        double worth = fills / (takes > 1e-9 ? takes : 1e-9);
        if (pick < 0 || worth > best) {
            pick = i;
            best = worth;
        }
    }
    return pick;
}

int re_business_wage_accept(long long demanded, int hours, double efficiency, double need, double fresh, double margin, re_wage_costs *costs)
{
    costs->keep = costs->fresh = 0.0;
    if (demanded <= 0 || hours <= 0 || !(efficiency > 0.0) || !(fresh > 0.0))
        return 1;
    costs->keep = re_business_need_cost(demanded, (double)hours * efficiency, need < RE_NEED_LEAST ? RE_NEED_LEAST : need);
    costs->fresh = fresh;
    return costs->keep <= costs->fresh * (1.0 + margin);
}

void re_business_order(const long long *cost, int n, int *order)
{
    for (int i = 0; i < n; i++) {
        int at = i; /* the new one moves up past every dearer one and every one without a number */
        while (at > 0 && cost[i] >= 0 && (cost[order[at - 1]] < 0 || cost[order[at - 1]] > cost[i])) {
            order[at] = order[at - 1];
            at--;
        }
        order[at] = i;
    }
}

/* FNV-1a, 64 bits: every byte is folded in and the sum multiplied by the prime */
static unsigned long long fnv(unsigned long long hash, const unsigned char *bytes, size_t count)
{
    for (size_t i = 0; i < count; i++)
        hash = (hash ^ bytes[i]) * 0x100000001b3ull;
    return hash;
}

static unsigned long long fnv_number(unsigned long long hash, int number)
{
    unsigned char bytes[4] = {(unsigned char)number, (unsigned char)(number >> 8), (unsigned char)(number >> 16), (unsigned char)(number >> 24)};
    return fnv(hash, bytes, sizeof bytes);
}

unsigned long long re_business_list_seed(const char *playthrough, int month, int firm, int job, int draw)
{
    unsigned long long hash = fnv(0xcbf29ce484222325ull, (const unsigned char *)playthrough, strlen(playthrough));
    return fnv_number(fnv_number(fnv_number(fnv_number(hash, month), firm), job), draw);
}

unsigned re_business_stream_seed(unsigned long long seed, int stream)
{
    /* the last bytes folded in move the high half of the sum most, so both halves go into the 32 bits */
    unsigned long long hash = fnv_number(seed, stream);
    return (unsigned)(hash >> 32) ^ (unsigned)hash;
}

static unsigned char **unit_engine(unsigned char *randgen, int i)
{
    return (unsigned char **)(randgen + (i < RE_RAND_STREAMS ? i * RE_RAND_UNIT_BYTES + RE_RAND_UNIT_ENGINE : RE_RANDGEN_SPARE));
}

void re_business_streams_seed(re_streams *keep, unsigned char *randgen, void *seed_fn, unsigned long long seed)
{
    typedef void(__thiscall *engine_seed_fn)(void *engine, const unsigned *seed);
    keep->randgen = randgen;
    memcpy(keep->unit, randgen, sizeof keep->unit);
    keep->spare = *unit_engine(randgen, RE_RAND_STREAMS);
    for (int i = 0; i < RE_RAND_ENGINES; i++)
        memcpy(keep->engine[i], *unit_engine(randgen, i), RE_RAND_ENGINE_BYTES);
    for (int i = 0; i < RE_RAND_STREAMS; i++) {
        unsigned stream_seed = re_business_stream_seed(seed, i);
        ((engine_seed_fn)seed_fn)(*unit_engine(randgen, i), &stream_seed);
    }
}

int re_business_streams_restore(const re_streams *keep, unsigned drawn[RE_RAND_STREAMS])
{
    unsigned char *randgen = keep->randgen;
    for (int i = 0; i < RE_RAND_STREAMS; i++) {
        unsigned *calls = (unsigned *)(randgen + i * RE_RAND_UNIT_BYTES + RE_RAND_UNIT_CALLS);
        drawn[i] = *calls - keep->unit[i][RE_RAND_UNIT_CALLS / 4];
        *calls = keep->unit[i][RE_RAND_UNIT_CALLS / 4];
    }
    for (int i = 0; i < RE_RAND_ENGINES; i++) {
        unsigned char *was = i < RE_RAND_STREAMS ? (unsigned char *)(size_t)keep->unit[i][RE_RAND_UNIT_ENGINE / 4] : keep->spare;
        if (*unit_engine(randgen, i) == was)
            memcpy(was, keep->engine[i], RE_RAND_ENGINE_BYTES);
    }
    /* Read again: what the game goes on with is what it had. An engine that is not where it was fails the first
     * test, so no pointer is followed here that was not followed when the streams were kept. */
    int same = memcmp(randgen, keep->unit, sizeof keep->unit) == 0 && *unit_engine(randgen, RE_RAND_STREAMS) == keep->spare;
    for (int i = 0; same && i < RE_RAND_ENGINES; i++)
        same = memcmp(*unit_engine(randgen, i), keep->engine[i], RE_RAND_ENGINE_BYTES) == 0;
    return same;
}

re_business_listing_fn re_business_listing;
re_business_listed_fn re_business_listed;
void *re_business_list;
re_business_wage_seed_fn re_business_wage_seed;
void *re_business_wage_ref;
re_business_line_fn re_business_hire_line;
void *re_business_assign, *re_business_name;
unsigned re_business_hire_return;
re_business_brand_fn re_business_brand;
void *re_business_brand_list;
void *re_brand_return, *re_brand_vector; /* the brand hook's own, between its two halves; the assembly below uses them */

__attribute__((force_align_arg_pointer)) void re_business_brand_after(unsigned char *const *vector, const unsigned *record)
{
    re_business_brand_fn fn = re_business_brand;
    if (fn != NULL)
        fn(vector, record);
}

static const unsigned char *g_row_staff; /* the candidate the row about to be made is for; NULL once used */

__attribute__((force_align_arg_pointer)) void re_business_before_list(void *firms, int firm, int job)
{
    re_business_listing_fn fn = re_business_listing;
    if (fn != NULL)
        fn(firms, firm, job);
}

__attribute__((force_align_arg_pointer)) void re_business_after_list(void *firms, unsigned char **vector)
{
    re_business_listed_fn fn = re_business_listed;
    if (fn != NULL)
        fn(firms, vector);
}

__attribute__((force_align_arg_pointer)) void re_business_wage_ref_enter(int *seed)
{
    re_business_wage_seed_fn fn = re_business_wage_seed;
    if (fn != NULL)
        fn(seed);
}

__attribute__((force_align_arg_pointer)) void re_business_row_enter(const unsigned char *staff)
{
    g_row_staff = staff;
}

/* `name` is the MSVC std::string the game's function has just filled: 16 bytes of text or a pointer, the length,
 * the room. */
__attribute__((force_align_arg_pointer)) void re_business_after_name(unsigned char *name, unsigned return_address)
{
    typedef void *(__thiscall *assign_fn)(void *self, const char *text, unsigned length);
    const unsigned char *staff = g_row_staff;
    re_business_line_fn fn = re_business_hire_line;
    g_row_staff = NULL;
    if (staff == NULL || fn == NULL || return_address != re_business_hire_return)
        return;
    unsigned size = *(const unsigned *)(name + 0x10), room = *(const unsigned *)(name + 0x14);
    char text[320];
    if (size == 0 || size > 100)
        return;
    memcpy(text, room >= 16 ? *(const char *const *)name : (const char *)name, size);
    unsigned added = fn(staff, text + size + 2, sizeof text - size - 3);
    if (added == 0)
        return;
    text[size] = ' ';
    text[size + 1] = '(';
    text[size + 2 + added] = ')';
    ((assign_fn)re_business_assign)(name, text, size + added + 3);
}

/* re_business_efficiency: the game's function takes its object in ecx and removes its two stack words; the registers
 * are put back whatever it does to them, and the float comes back the way C expects one.
 *
 * re_business_payout, re_business_length, re_business_data_number: the same for three more functions of the game.
 * The first builds its argument as the game does at 0x004f84ff: room on the stack, the copy constructor into it
 * (ecx = the room, one stack word that it removes), then the firm id; the payout function removes all of it. The
 * third builds its two strings as the game does at 0x004ebb2b: empty with room for 15 letters, then the game's
 * assign (ecx = the string, two stack words that it removes); the attribute's name is pushed first, so it is the
 * second argument.
 *
 * re_business_list_hook: in place of the hire list, ecx = firms and three stack words that the callee removes. The
 * first callback runs with every register put back behind it, then the game's function with the same arguments,
 * then the second callback; eax is what the game's function returned and every other register is as that function
 * left it.
 *
 * re_business_wage_ref_hook: in place of the reference wage of a new person, ecx = jobs and three stack words, the
 * last of them the seed. The C side gets the address of the seed; then the game's function runs with every
 * register and the stack as the caller left them. Behind `pushad` and the eight SSE registers the seed is 160
 * bytes and the return address and two arguments up.
 *
 * re_business_hire_row_hook: in place of a string assign (ecx = the string, two stack words), esi = the candidate.
 * That is noted; then the game's assign runs with every register and the stack as the caller left them.
 *
 * re_business_name_hook: in place of the name function inside the row maker, ecx = the object, one stack word (the
 * string to fill) that the callee removes; ebx = the row maker's argument base, its return address at [ebx + 4].
 * The game's function runs first, then the C side gets the string and that address.
 *
 * re_business_brand_hook: in place of the game's list function for the adverts of a business type, ecx = the vector
 * to fill and 0x68 bytes of arguments that the CALLER removes. The return address is put aside so that the game's
 * function finds its arguments where it expects them; behind it the C side gets the vector and the head of the
 * arguments, with every register as the game's function left it. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_business_efficiency\n"
        "_re_business_efficiency:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  sub esp, 4\n"
        "  pushad\n"
        "  mov ecx, [ebp + 12]\n"
        "  push dword ptr [ebp + 20]\n"
        "  push dword ptr [ebp + 16]\n"
        "  call dword ptr [ebp + 8]\n"
        "  movss [ebp - 4], xmm0\n"
        "  popad\n"
        "  fld dword ptr [ebp - 4]\n"
        "  mov esp, ebp\n"
        "  pop ebp\n"
        "  ret\n"
        ".globl _re_business_payout\n"
        "_re_business_payout:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  sub esp, 8\n"
        "  pushad\n"
        "  sub esp, [ebp + 28]\n"
        "  mov ecx, esp\n"
        "  push dword ptr [ebp + 24]\n"
        "  call dword ptr [ebp + 8]\n"
        "  push dword ptr [ebp + 20]\n"
        "  mov ecx, [ebp + 16]\n"
        "  call dword ptr [ebp + 12]\n"
        "  mov [ebp - 8], eax\n"
        "  mov [ebp - 4], edx\n"
        "  popad\n"
        "  mov eax, [ebp - 8]\n"
        "  mov edx, [ebp - 4]\n"
        "  mov esp, ebp\n"
        "  pop ebp\n"
        "  ret\n"
        ".globl _re_business_length\n"
        "_re_business_length:\n"
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
        ".globl _re_business_data_number\n"
        "_re_business_data_number:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  sub esp, 4\n"
        "  pushad\n"
        "  sub esp, 24\n"
        "  mov ecx, esp\n"
        "  mov byte ptr [ecx], 0\n"
        "  mov dword ptr [ecx + 16], 0\n"
        "  mov dword ptr [ecx + 20], 15\n"
        "  push dword ptr [ebp + 32]\n"
        "  push dword ptr [ebp + 28]\n"
        "  call dword ptr [ebp + 16]\n"
        "  sub esp, 24\n"
        "  mov ecx, esp\n"
        "  mov byte ptr [ecx], 0\n"
        "  mov dword ptr [ecx + 16], 0\n"
        "  mov dword ptr [ecx + 20], 15\n"
        "  push dword ptr [ebp + 24]\n"
        "  push dword ptr [ebp + 20]\n"
        "  call dword ptr [ebp + 16]\n"
        "  mov ecx, [ebp + 12]\n"
        "  call dword ptr [ebp + 8]\n"
        "  movss [ebp - 4], xmm0\n"
        "  lea esp, [ebp - 36]\n" /* a function that leaves the two strings to its caller is served too */
        "  popad\n"
        "  fld dword ptr [ebp - 4]\n"
        "  mov esp, ebp\n"
        "  pop ebp\n"
        "  ret\n"
        ".globl _re_business_list_hook\n"
        "_re_business_list_hook:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ecx\n" /* [ebp - 4] = firms */
        "  pushad\n"
        SAVE_XMM
        "  push dword ptr [ebp + 16]\n"
        "  push dword ptr [ebp + 12]\n"
        "  push dword ptr [ebp - 4]\n"
        "  call _re_business_before_list\n"
        "  add esp, 12\n"
        RESTORE_XMM
        "  popad\n"
        "  push dword ptr [ebp + 16]\n"
        "  push dword ptr [ebp + 12]\n"
        "  push dword ptr [ebp + 8]\n"
        "  call dword ptr [_re_business_list]\n"
        "  push eax\n"
        "  pushad\n"
        SAVE_XMM
        "  push dword ptr [ebp + 8]\n"
        "  push dword ptr [ebp - 4]\n"
        "  call _re_business_after_list\n"
        "  add esp, 8\n"
        RESTORE_XMM
        "  popad\n"
        "  pop eax\n"
        "  mov esp, ebp\n"
        "  pop ebp\n"
        "  ret 12\n"
        ".globl _re_business_wage_ref_hook\n"
        "_re_business_wage_ref_hook:\n"
        "  pushad\n"
        SAVE_XMM
        "  lea eax, [esp + 172]\n"
        "  push eax\n"
        "  call _re_business_wage_ref_enter\n"
        "  add esp, 4\n"
        RESTORE_XMM
        "  popad\n"
        "  jmp dword ptr [_re_business_wage_ref]\n"
        ".globl _re_business_hire_row_hook\n"
        "_re_business_hire_row_hook:\n"
        "  pushad\n"
        SAVE_XMM
        "  push esi\n"
        "  call _re_business_row_enter\n"
        "  add esp, 4\n"
        RESTORE_XMM
        "  popad\n"
        "  jmp dword ptr [_re_business_assign]\n"
        ".globl _re_business_name_hook\n"
        "_re_business_name_hook:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push dword ptr [ebp + 8]\n"
        "  call dword ptr [_re_business_name]\n"
        "  push eax\n"
        "  pushad\n"
        SAVE_XMM
        "  push dword ptr [ebx + 4]\n"
        "  push dword ptr [ebp + 8]\n"
        "  call _re_business_after_name\n"
        "  add esp, 8\n"
        RESTORE_XMM
        "  popad\n"
        "  pop eax\n"
        "  mov esp, ebp\n"
        "  pop ebp\n"
        "  ret 4\n"
        ".globl _re_business_brand_hook\n"
        "_re_business_brand_hook:\n"
        "  pop dword ptr [_re_brand_return]\n" /* the arguments are on top again, as the game's function wants them */
        "  mov [_re_brand_vector], ecx\n"
        "  call dword ptr [_re_business_brand_list]\n"
        "  pushad\n"
        SAVE_XMM
        "  lea eax, [esp + 160]\n" /* 128 (xmm) + 32 (pushad): the arguments the caller still has to remove */
        "  push eax\n"
        "  push dword ptr [_re_brand_vector]\n"
        "  call _re_business_brand_after\n"
        "  add esp, 8\n"
        RESTORE_XMM
        "  popad\n"
        "  jmp dword ptr [_re_brand_return]\n"
        ".att_syntax prefix\n");
