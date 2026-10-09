/* The casino as games of fixed stake and pure chance (notes b15 and m17, REQUEST.md [29] and [30]).
 *
 * The four games are activities of 20 hours. The game itself draws an amount around a part of the household's net
 * worth in the first hour and books it at once. Here instead: a stake per game that does not grow with wealth, one
 * game a month per household and game, and the result booked by the plugin when the action ends, so that an action
 * given up half-way has no result. What a game pays is decided by the playthrough, the game and the month alone:
 * loading a save and playing the same month again gives the same result.
 *
 * Two hooks do it. A detour on the function every float attribute of an object is read through answers the five
 * attributes that make a game a fixed-stake one. A wrapper around the two calls that book an action's money turns
 * the first-hour booking into nothing and hands the last hour to the plugin.
 */
#ifndef RE_CASINO_H
#define RE_CASINO_H

#define RE_ID_SLOTS 75711
#define RE_ID_ROULETTE 75716
#define RE_ID_BLACKJACK 75721
#define RE_ID_BACCARAT 75726
#define RE_TAG_GAMBLING 2191 /* the finance tag the game books these activities under */

typedef struct {
    double chance, times; /* with this chance the game pays this many times the stake */
} re_casino_prize;

typedef struct {
    int id;
    long long stake; /* cents, before the game's price index */
    int prizes;
    re_casino_prize prize[2]; /* a game pays one of them or nothing; the commoner one first */
} re_casino_game;

#define RE_CASINO_GAMES 4
extern const re_casino_game RE_CASINO[RE_CASINO_GAMES];

/* NULL for an activity that is not one of the four games. */
const re_casino_game *re_casino_game_of(int id);

/* A number in [0, 1) that depends on the playthrough's seed, the game and the month and on nothing else. */
double re_casino_unit(unsigned long long seed, int id, int month);
/* What the game of that month pays, as a multiple of the stake; 0 = nothing. */
double re_casino_times(const re_casino_game *game, unsigned long long seed, int month);
/* Prize minus stake, in cents. */
long long re_casino_net(long long stake, double times);
/* The part of the stakes that comes back over many games. */
double re_casino_return(const re_casino_game *game);

/* The plugin's answer for a float attribute of a game: action.money (minus the stake), action.moneyVariance 0,
 * action.interval 1, action.intervalGlobal 1, stats.netWorthMod 0. Returns 0 for any other attribute or object. */
int re_casino_attribute(int id, const char *section, unsigned section_len, const char *name, unsigned name_len, float *out);

/* Detour for the attribute function (0x00808b10, fastcall: ecx, edx = object id; three std::string by value, which
 * the callee destroys; plain ret; float in xmm0). Other objects pass with two registers compared and nothing else. */
extern void *re_casino_attr_trampoline; /* filled by re_detour5 */
extern void *re_casino_str_free;        /* the game's std::string destructor (thiscall) */
void re_casino_attr_hook(void);         /* entry for the detour; not callable from C */

/* Wrapper for the two calls of the action-money function (0x00438440, thiscall: ecx = the person's actions; result,
 * queue record, activity object, two strings, 8 bytes, phase, afford flag; ret 0x4c). For a game: in the first hour
 * the phase is turned to "income", which books nothing for a cost; in the last hour `re_casino_end` runs first. The
 * game's function then runs as it would, and books nothing either. */
typedef void (*re_casino_end_fn)(unsigned char *actions, unsigned char *record, unsigned char *object);
extern void *re_casino_money; /* the game's function */
extern re_casino_end_fn re_casino_end;
/* `re_casino_start` sees the first hour of a game: a game whose first hour the plugin did not see was begun under the
 * game's own rules, which book the result right then (a save from before the mod). */
typedef void (*re_casino_start_fn)(unsigned char *object);
extern re_casino_start_fn re_casino_start;
void re_casino_start_enter(unsigned char *object);
void re_casino_money_hook(void); /* target for re_patch_call; not callable from C */

#endif
