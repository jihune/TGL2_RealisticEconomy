#include "re_casino.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

#include "re_sites.h"

/* REQUEST.md [29], confirmed in [30]. Return = sum of chance x times: 0.98, 0.96, 0.95, 0.93. */
const re_casino_game RE_CASINO[RE_CASINO_GAMES] = {
    {RE_ID_SLOTS, 100000, 2, {{0.22, 3.0}, {0.008, 40.0}}},
    {RE_ID_ROULETTE, 1000000, 2, {{0.44, 2.0}, {0.02, 4.0}}},
    {RE_ID_BLACKJACK, 10000000, 2, {{0.45, 2.0}, {0.02, 2.5}}},
    {RE_ID_BACCARAT, 100000000, 1, {{0.465, 2.0}}},
};

const re_casino_game *re_casino_game_of(int id)
{
    for (int i = 0; i < RE_CASINO_GAMES; i++)
        if (RE_CASINO[i].id == id)
            return &RE_CASINO[i];
    return NULL;
}

/* splitmix64's finishing step */
static unsigned long long mix(unsigned long long x)
{
    x += 0x9E3779B97F4A7C15ull;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
    return x ^ (x >> 31);
}

double re_casino_unit(unsigned long long seed, int id, int month)
{
    unsigned long long x = mix(seed ^ mix(((unsigned long long)(unsigned)id << 32) | (unsigned)month));
    return (double)(x >> 11) / 9007199254740992.0; /* 53 bits */
}

double re_casino_times(const re_casino_game *game, unsigned long long seed, int month)
{
    double unit = re_casino_unit(seed, game->id, month), edge = 0.0;
    for (int i = 0; i < game->prizes; i++) {
        edge += game->prize[i].chance;
        if (unit < edge)
            return game->prize[i].times;
    }
    return 0.0;
}

long long re_casino_net(long long stake, double times)
{
    return llround((double)stake * times) - stake;
}

double re_casino_return(const re_casino_game *game)
{
    double sum = 0.0;
    for (int i = 0; i < game->prizes; i++)
        sum += game->prize[i].chance * game->prize[i].times;
    return sum;
}

static int is(const char *text, unsigned len, const char *word)
{
    return len == strlen(word) && memcmp(text, word, len) == 0;
}

int re_casino_attribute(int id, const char *section, unsigned section_len, const char *name, unsigned name_len, float *out)
{
    static const struct {
        const char *section, *name;
        float value;
    } FIXED[] = {
        {"action", "moneyVariance", 0.0f}, /* no draw around the amount, and no "~" in front of it */
        {"action", "interval", 1.0f},      /* months before the next one */
        {"action", "intervalGlobal", 1.0f}, /* counted for the household, not for the person */
        {"stats", "netWorthMod", 0.0f},     /* the amount does not grow with the household's wealth */
    };
    const re_casino_game *game = re_casino_game_of(id);
    if (game == NULL)
        return 0;
    if (is(section, section_len, "action") && is(name, name_len, "money")) {
        *out = -(float)game->stake;
        return 1;
    }
    for (size_t i = 0; i < sizeof FIXED / sizeof FIXED[0]; i++)
        if (is(section, section_len, FIXED[i].section) && is(name, name_len, FIXED[i].name)) {
            *out = FIXED[i].value;
            return 1;
        }
    return 0;
}

void *re_casino_attr_trampoline;
void *re_casino_str_free;
void *re_casino_money;
re_casino_end_fn re_casino_end;
re_casino_start_fn re_casino_start;

/* MSVC 14.x std::string, 32-bit: 16 bytes inline buffer or pointer, then size, then capacity. */
typedef struct {
    union {
        char buf[16];
        char *ptr;
    } u;
    unsigned size, capacity;
} msvc_string;

/* 1 = answered: `*out` holds the value and the three strings are destroyed, as the game's function would have left
 * them. 0 = not ours: nothing was touched and the game's function has to run. */
__attribute__((force_align_arg_pointer)) int re_casino_attr_enter(int id, msvc_string *type, msvc_string *section, msvc_string *name,
                                                                  float *out)
{
    typedef void(__thiscall *free_fn)(void *string);
    if (re_casino_str_free == NULL || section->size > 15 || name->size > 15) /* the names in question are shorter */
        return 0;
    const char *section_text = section->capacity >= 16 ? section->u.ptr : section->u.buf;
    const char *name_text = name->capacity >= 16 ? name->u.ptr : name->u.buf;
    if (!re_casino_attribute(id, section_text, section->size, name_text, name->size, out))
        return 0;
    ((free_fn)re_casino_str_free)(type);
    ((free_fn)re_casino_str_free)(section);
    ((free_fn)re_casino_str_free)(name);
    return 1;
}

__attribute__((force_align_arg_pointer)) void re_casino_end_enter(unsigned char *actions, unsigned char *record, unsigned char *object)
{
    re_casino_end_fn end = re_casino_end;
    if (end != NULL)
        end(actions, record, object);
}

__attribute__((force_align_arg_pointer)) void re_casino_start_enter(unsigned char *object)
{
    re_casino_start_fn start = re_casino_start;
    if (start != NULL)
        start(object);
}

#define TEXT_OF(x) #x
#define ASM(x) TEXT_OF(x) /* a define of re_casino.h or re_sites.h as it has to stand in the assembly */

/* The attribute function. Stack on entry: [esp] return address, [esp + 4] type, [esp + 0x1c] section, [esp + 0x34]
 * name. Any other object: edx is compared four times and the game's function runs untouched. A game: the C side is
 * asked; when it answers, the value goes into xmm0 and the function returns in the game's place (the caller pops the
 * strings' 0x48 bytes itself), otherwise ecx and edx are put back and the game's function runs. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_casino_attr_hook\n"
        "_re_casino_attr_hook:\n"
        "  cmp edx, " ASM(RE_ID_SLOTS) "\n"
        "  je 1f\n"
        "  cmp edx, " ASM(RE_ID_ROULETTE) "\n"
        "  je 1f\n"
        "  cmp edx, " ASM(RE_ID_BLACKJACK) "\n"
        "  je 1f\n"
        "  cmp edx, " ASM(RE_ID_BACCARAT) "\n"
        "  je 1f\n"
        "  jmp dword ptr [_re_casino_attr_trampoline]\n"
        "1:\n"
        "  push ecx\n"
        "  push edx\n"
        "  sub esp, 4\n" /* the answer */
        "  mov eax, esp\n"
        "  push eax\n"
        "  lea eax, [esp + 16 + 0x34]\n" /* 16 = the four words above the return address so far */
        "  push eax\n"
        "  lea eax, [esp + 20 + 0x1c]\n"
        "  push eax\n"
        "  lea eax, [esp + 24 + 4]\n"
        "  push eax\n"
        "  push edx\n"
        "  call _re_casino_attr_enter\n"
        "  add esp, 20\n"
        "  test eax, eax\n"
        "  jz 2f\n"
        "  movss xmm0, dword ptr [esp]\n"
        "  add esp, 12\n"
        "  ret\n"
        "2:\n"
        "  add esp, 4\n"
        "  pop edx\n"
        "  pop ecx\n"
        "  jmp dword ptr [_re_casino_attr_trampoline]\n"
        ".att_syntax prefix\n");

/* The action-money function, as its two callers reach it. Stack on entry: [esp] return address, [esp + 4] result,
 * [esp + 8] queue record, [esp + 0xc] activity object (id at +0xc), then the strings, [esp + 0x48] the phase.
 * eax is free at both call sites: it held the last pushed argument and is not read after the call. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _re_casino_money_hook\n"
        "_re_casino_money_hook:\n"
        "  mov eax, [esp + 0xc]\n"
        "  mov eax, [eax + " ASM(RE_OBJ_ID) "]\n"
        "  cmp eax, " ASM(RE_ID_SLOTS) "\n"
        "  je 1f\n"
        "  cmp eax, " ASM(RE_ID_ROULETTE) "\n"
        "  je 1f\n"
        "  cmp eax, " ASM(RE_ID_BLACKJACK) "\n"
        "  je 1f\n"
        "  cmp eax, " ASM(RE_ID_BACCARAT) "\n"
        "  je 1f\n"
        "  jmp dword ptr [_re_casino_money]\n"
        "1:\n"
        "  cmp byte ptr [esp + " ASM(RE_ACTION_MONEY_PHASE) "], 0\n"
        "  je 2f\n"
        "  mov dword ptr [esp + " ASM(RE_ACTION_MONEY_PHASE) "], 0\n" /* the first hour: nothing is booked */
        "  pushad\n"
        "  mov eax, [esp + 32 + 0xc]\n"
        "  push eax\n"
        "  call _re_casino_start_enter\n" /* ... and the start goes on record */
        "  add esp, 4\n"
        "  popad\n"
        "  jmp dword ptr [_re_casino_money]\n"
        "2:\n"
        "  pushad\n"
        "  mov eax, [esp + 32 + 0xc]\n"
        "  push eax\n"
        "  mov eax, [esp + 36 + 8]\n"
        "  push eax\n"
        "  push ecx\n"
        "  call _re_casino_end_enter\n"
        "  add esp, 12\n"
        "  popad\n"
        "  jmp dword ptr [_re_casino_money]\n"
        ".att_syntax prefix\n");
