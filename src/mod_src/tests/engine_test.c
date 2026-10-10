/* Tests the plugin's patching code without starting the game.
 *
 *   engine_test.exe <path to TGL2.exe> [--control]
 *
 * T1  maps TGL2.exe as data (no code of the game runs) and checks every patch site against the expected bytes.
 * T2  detours a stand-in function that has the game getter's prologue and calling convention, then calls it the
 *     way the game does and checks results, the stack pointer and the callee-saved registers.
 * T3  detours a stand-in with the cash function's 6-byte prologue and `ret 0x18`, with a sink that deliberately
 *     destroys xmm3, and checks the result, the registers, xmm3 and what the sink was shown.
 * T4  detours a stand-in for the translation function and checks the edited text, the caller it reports and that
 *     the caller's registers and frame reach both callbacks.
 * T5  redirects a `call rel32` to the education-loan hook and checks that the callee gets the record the callback
 *     changed, with registers, stack and xmm3 intact.
 * T6  re-points the two operands of a stand-in for the fixed-rate formula and checks the new result.
 * T7  checks every entry of the wording table against the game's Korean language file, and rewrites sample texts.
 * T8  puts the three trade hooks on stand-ins shaped like the game's buy function, futures function and sell handler,
 *     and checks that an allowed trade runs unchanged and a refused one does not run, with registers, stack and xmm3
 *     intact both ways. Then the two calls of a stand-in month end: the callback is told before the first and after
 *     the second, and both run as they did; and the one call of a stand-in month start, told on both sides.
 * T9  the load guard's arithmetic: no lock on the newest save, a lock up to the farthest month end, the cap, the
 *     stream move beyond the cap, and the same answer when the same save is loaded again.
 * T10 the listing: an instruction that reads a field of an object is re-pointed at a number of the plugin, and the
 *     two redirected calls of a stand-in listing function (frame laid out like the game's) hand the callbacks the
 *     frame's share counts and company, let them move the price, and leave registers, stack and xmm3 intact. The
 *     callback for the price steps' draws is told before the steps and after them, before the price is moved.
 * T11 the board: a stand-in election calls a stand-in score function (company in ecx, two stack words the callee
 *     removes); the redirected call lets the callback raise the totals of the household's first nominees, as many
 *     as the holding guarantees, with different values, and leaves the others, the registers, the stack and xmm3.
 * T12 a company's share: the ratios, the target of the game's monthly price step, the line under which the game
 *     takes emergency measures, and the lines written for the player in both languages.
 * T15 the casino: the table of the four games, results that depend on playthrough, game and month only, and the
 *     two hooks on stand-ins shaped like the game's attribute function and its action-money function.
 * --control breaks every expectation on purpose; the verdict must then be FAIL.
 */
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../plugin/re_board.h"
#include "../plugin/re_business.h"
#include "../plugin/re_casino.h"
#include "../plugin/re_font.h"
#include "../plugin/re_futures.h"
#include "../plugin/re_getter.h"
#include "../plugin/re_guard.h"
#include "../plugin/re_hook.h"
#include "../plugin/re_ipo.h"
#include "../plugin/re_lang.h"
#include "../plugin/re_loan.h"
#include "../plugin/re_memory.h"
#include "../plugin/re_sites.h"
#include "../plugin/re_stock.h"
#include "../plugin/re_text.h"
#include "../plugin/re_trace.h"
#include "../plugin/re_util.h"
#include "../plugin/re_wording.h"
#include "../plugin/re_xp.h"

/* re_util.c is not part of this test: what re_font.c would write into the plugin's log goes nowhere */
void re_log(const char *fmt, ...)
{
    (void)fmt;
}

typedef struct {
    char buf[16];
    unsigned size;
    unsigned capacity;
} msvc_string;

typedef struct {
    int pad[3];
    int id;      /* +0xc, where the game keeps the object id */
    float value; /* +0x10, what the stand-in returns */
} fake_object;

void fake_getter(void);
unsigned call_getter(void *fn, void *obj, const msvc_string *name, unsigned *intact);

/* Stand-in for 0x00807530: same first five bytes, object in ecx, std::string by value, callee "destroys" the
 * string, plain ret, float result in xmm0. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _fake_getter\n"
        "_fake_getter:\n"
        "  .byte 0x55, 0x8b, 0xec, 0x6a, 0xff\n" /* push ebp / mov ebp, esp / push -1 */
        "  add esp, 4\n"
        "  movss xmm0, [ecx + 0x10]\n"
        "  mov byte ptr [ebp + 8], 0\n"
        "  mov dword ptr [ebp + 24], 0\n"
        "  mov dword ptr [ebp + 28], 15\n"
        "  mov esp, ebp\n"
        "  pop ebp\n"
        "  ret\n"
        /* unsigned call_getter(fn, obj, name, intact): calls like the game (caller pops 24 bytes), returns the
         * float bits, sets *intact when ebx, esi, edi and esp came back unchanged */
        ".globl _call_getter\n"
        "_call_getter:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  sub esp, 24\n"
        "  mov esi, [ebp + 16]\n"
        "  mov edi, esp\n"
        "  mov ecx, 6\n"
        "  rep movsd\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, 0x33333333\n"
        "  mov ecx, [ebp + 12]\n"
        "  mov eax, [ebp + 8]\n"
        "  call eax\n"
        "  add esp, 24\n"
        "  xor edx, edx\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 12]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  mov edx, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 20]\n"
        "  mov [ecx], edx\n"
        "  movd eax, xmm0\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

void fake_money(void);
unsigned call_money(void *fn, void *self, const unsigned *args, unsigned *intact);

/* Stand-in for 0x00542920: same first six bytes, object in ecx, six stack words, `ret 0x18`.
 * Returns [ecx] + first argument + third argument. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _fake_money\n"
        "_fake_money:\n"
        "  .byte 0x53, 0x8b, 0xdc, 0x83, 0xec, 0x08\n" /* push ebx / mov ebx, esp / sub esp, 8 */
        "  mov eax, [ebx + 8]\n"
        "  add eax, [ebx + 16]\n"
        "  add eax, [ecx]\n"
        "  mov esp, ebx\n"
        "  pop ebx\n"
        "  ret 0x18\n"
        /* unsigned call_money(fn, self, args[6], intact): calls like the game, sets *intact when ebx, esi, edi,
         * esp and xmm3 came back unchanged */
        ".globl _call_money\n"
        "_call_money:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  mov esi, [ebp + 16]\n"
        "  push dword ptr [esi + 20]\n"
        "  push dword ptr [esi + 16]\n"
        "  push dword ptr [esi + 12]\n"
        "  push dword ptr [esi + 8]\n"
        "  push dword ptr [esi + 4]\n"
        "  push dword ptr [esi]\n"
        "  mov eax, 0x44444444\n"
        "  movd xmm3, eax\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, 0x33333333\n"
        "  mov ecx, [ebp + 12]\n"
        "  mov eax, [ebp + 8]\n"
        "  call eax\n"
        "  xor edx, edx\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 12]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  movd ecx, xmm3\n"
        "  cmp ecx, 0x44444444\n"
        "  jne 9f\n"
        "  mov edx, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 20]\n"
        "  mov [ecx], edx\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

void fake_translate(void);
unsigned call_translate(void *fn, void *self, msvc_string *out, const msvc_string *key);

/* Stand-in for 0x0081d7c0: same first six bytes, result string pointer plus eleven strings by value, `ret 0x10c`.
 * Writes "RATE" into the result. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _fake_translate\n"
        "_fake_translate:\n"
        "  .byte 0x53, 0x8b, 0xdc, 0x83, 0xec, 0x08\n" /* push ebx / mov ebx, esp / sub esp, 8 */
        "  mov eax, [ebx + 8]\n"
        "  mov dword ptr [eax], 0x45544152\n"
        "  mov byte ptr [eax + 4], 0\n"
        "  mov dword ptr [eax + 16], 4\n"
        "  mov dword ptr [eax + 20], 15\n"
        "  mov eax, 0x55555555\n" /* a return value the detour must not lose */
        "  mov esp, ebx\n"
        "  pop ebx\n"
        "  ret 0x10c\n"
        /* unsigned call_translate(fn, self, out, key): calls like the game; returns 1 when eax, ebx, esi, edi and
         * esp came back as expected */
        ".globl _call_translate\n"
        "_call_translate:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  sub esp, 264\n"
        "  mov edi, esp\n"
        "  mov ecx, 66\n"
        "  xor eax, eax\n"
        "  rep stosd\n"
        "  mov esi, [ebp + 20]\n"
        "  mov edi, esp\n"
        "  mov ecx, 6\n"
        "  rep movsd\n"
        "  push dword ptr [ebp + 16]\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, 0x33333333\n"
        "  mov ecx, [ebp + 12]\n"
        "  mov eax, [ebp + 8]\n"
        "  call eax\n"
        "  xor edx, edx\n"
        "  cmp eax, 0x55555555\n"
        "  jne 9f\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 12]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  mov edx, 1\n"
        "9:\n"
        "  mov eax, edx\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

void fake_row(void);
void call_row(void *fn, void *self, const msvc_string *label, const msvc_string *value, unsigned red, unsigned *intact);
msvc_string g_row_label, g_row_value;
unsigned g_row_red;

/* Stand-in for 0x007b6d30: same first five bytes, window in ecx, three words and two std::string by value,
 * `ret 0x3c`. Copies the red flag and the two strings out. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _fake_row\n"
        "_fake_row:\n"
        "  .byte 0x55, 0x8b, 0xec, 0x6a, 0xff\n" /* push ebp / mov ebp, esp / push -1 */
        "  push esi\n"
        "  push edi\n"
        "  mov eax, [ebp + 16]\n"
        "  mov [_g_row_red], eax\n"
        "  lea esi, [ebp + 20]\n"
        "  mov edi, offset _g_row_label\n"
        "  mov ecx, 6\n"
        "  rep movsd\n"
        "  lea esi, [ebp + 44]\n"
        "  mov edi, offset _g_row_value\n"
        "  mov ecx, 6\n"
        "  rep movsd\n"
        "  pop edi\n"
        "  pop esi\n"
        "  mov esp, ebp\n"
        "  pop ebp\n"
        "  ret 0x3c\n"
        /* call_row(fn, self, label, value, red, intact): calls like the game; *intact = 1 when ebx, esi, edi, esp and
         * xmm3 came back as they were */
        ".globl _call_row\n"
        "_call_row:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  sub esp, 48\n"
        "  mov esi, [ebp + 16]\n"
        "  mov edi, esp\n"
        "  mov ecx, 6\n"
        "  rep movsd\n"
        "  mov esi, [ebp + 20]\n"
        "  mov ecx, 6\n"
        "  rep movsd\n"
        "  push dword ptr [ebp + 24]\n"
        "  push 0x66666666\n"
        "  push 0x77777777\n"
        "  mov eax, 0x44444444\n"
        "  movd xmm3, eax\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, 0x33333333\n"
        "  mov ecx, [ebp + 12]\n"
        "  mov eax, [ebp + 8]\n"
        "  call eax\n"
        "  xor edx, edx\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 12]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  movd ecx, xmm3\n"
        "  cmp ecx, 0x44444444\n"
        "  jne 9f\n"
        "  mov edx, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 28]\n"
        "  mov [ecx], edx\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

void fake_add_debt(void);
void add_debt_site(void);
unsigned call_add_debt(void *self, const unsigned *record, unsigned *intact);

/* Stand-in for 0x004ce2b0: object in ecx, a 0x60-byte record by value, `ret 0x60`.
 * Returns [ecx] + the record's second word + its fourth word. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _fake_add_debt\n"
        "_fake_add_debt:\n"
        "  mov eax, [ecx]\n"
        "  add eax, [esp + 8]\n"
        "  add eax, [esp + 16]\n"
        "  ret 0x60\n"
        /* unsigned call_add_debt(self, record[24], intact): copies the record onto the stack and calls the stand-in
         * with a `call rel32` at add_debt_site, like the game's enrol routine; sets *intact when ebx, esi, edi,
         * esp and xmm3 came back unchanged */
        ".globl _call_add_debt\n"
        "_call_add_debt:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  sub esp, 0x60\n"
        "  mov esi, [ebp + 12]\n"
        "  mov edi, esp\n"
        "  mov ecx, 24\n"
        "  rep movsd\n"
        "  mov eax, 0x44444444\n"
        "  movd xmm3, eax\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, 0x33333333\n"
        "  mov ecx, [ebp + 8]\n"
        ".globl _add_debt_site\n"
        "_add_debt_site:\n"
        "  call _fake_add_debt\n"
        "  xor edx, edx\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 12]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  movd ecx, xmm3\n"
        "  cmp ecx, 0x44444444\n"
        "  jne 9f\n"
        "  mov edx, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 16]\n"
        "  mov [ecx], edx\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

static unsigned g_loan_value, g_loan_from;
static void *g_loan_manager;

static void test_loan_created(unsigned *record, void *debt_manager, unsigned return_address)
{
    g_loan_manager = debt_manager;
    g_loan_from = return_address;
    if (record[1] == 0)
        record[1] = g_loan_value;
    __asm__("pxor %xmm3, %xmm3");
}

extern const double fake_c11, fake_c003;
void fake_fixed_mul(void);
void fake_fixed_add(void);
float fake_fixed(float variable);

/* Stand-in for the game's fixed-rate formula: variable x [1.1] + [0.03], the same two instructions reading two
 * doubles through absolute addresses. */
__asm__(".intel_syntax noprefix\n"
        ".data\n"
        ".globl _fake_c11\n"
        "_fake_c11: .double 1.1\n"
        ".globl _fake_c003\n"
        "_fake_c003: .double 0.03\n"
        ".text\n"
        ".globl _fake_fixed\n"
        "_fake_fixed:\n"
        "  movss xmm0, [esp + 4]\n"
        "  cvtss2sd xmm0, xmm0\n"
        ".globl _fake_fixed_mul\n"
        "_fake_fixed_mul:\n"
        "  mulsd xmm0, qword ptr [_fake_c11]\n"
        ".globl _fake_fixed_add\n"
        "_fake_fixed_add:\n"
        "  addsd xmm0, qword ptr [_fake_c003]\n"
        "  cvtsd2ss xmm0, xmm0\n"
        "  movss [esp + 4], xmm0\n"
        "  fld dword ptr [esp + 4]\n"
        "  ret\n"
        ".att_syntax prefix\n");

unsigned g_trade_sum, g_trade_runs, g_step_runs;
void fake_buy(void);
void fake_futures(void);
void fake_sell_handler(void);
void fake_sell_step(void);
void fake_sell_site(void);
void fake_sell_skip(void);
void call_trade(void *fn, void *self, const unsigned *args, int count, unsigned *intact);

/* Stand-ins for the three places a trade is refused at. Each adds [ecx] + its first and last argument to g_trade_sum
 * and counts a run.
 * fake_buy: the first six bytes of 0x00535120, two stack arguments, `ret 8`.
 * fake_futures: the first five bytes of 0x0040ccf0, four stack arguments, `ret 16`.
 * fake_sell_handler: shaped like 0x007ba8d0 around its selling branch - nothing to sell goes straight to the way out;
 * otherwise the branch starts with a call that takes ecx only, and the sale follows. One stack argument, `ret 4`. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _fake_buy\n"
        "_fake_buy:\n"
        "  .byte 0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf8\n" /* push ebp / mov ebp, esp / and esp, -8 */
        "  mov eax, [ebp + 8]\n"
        "  add eax, [ebp + 12]\n"
        "  add eax, [ecx]\n"
        "  add dword ptr [_g_trade_sum], eax\n"
        "  inc dword ptr [_g_trade_runs]\n"
        "  mov esp, ebp\n"
        "  pop ebp\n"
        "  ret 8\n"
        ".globl _fake_futures\n"
        "_fake_futures:\n"
        "  .byte 0x55, 0x8b, 0xec, 0x6a, 0xff\n" /* push ebp / mov ebp, esp / push -1 */
        "  mov eax, [ebp + 8]\n"
        "  add eax, [ebp + 20]\n"
        "  add eax, [ecx]\n"
        "  add dword ptr [_g_trade_sum], eax\n"
        "  inc dword ptr [_g_trade_runs]\n"
        "  mov esp, ebp\n"
        "  pop ebp\n"
        "  ret 16\n"
        ".globl _fake_sell_step\n"
        "_fake_sell_step:\n"
        "  inc dword ptr [_g_step_runs]\n"
        "  ret\n"
        ".globl _fake_sell_handler\n"
        "_fake_sell_handler:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  mov edi, ecx\n"
        "  mov esi, [ebp + 8]\n"
        "  test esi, esi\n"
        "  je _fake_sell_skip\n"
        ".globl _fake_sell_site\n"
        "_fake_sell_site:\n"
        "  call _fake_sell_step\n"
        "  mov eax, [edi]\n"
        "  add eax, esi\n"
        "  add eax, esi\n"
        "  add dword ptr [_g_trade_sum], eax\n"
        "  inc dword ptr [_g_trade_runs]\n"
        ".globl _fake_sell_skip\n"
        "_fake_sell_skip:\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret 4\n"
        /* call_trade(fn, self, args, count, intact): pushes `count` arguments, calls like the game, sets *intact when
         * ebx, esi, edi and xmm3 came back unchanged and the callee took its arguments off the stack */
        ".globl _call_trade\n"
        "_call_trade:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  mov esi, [ebp + 16]\n"
        "  mov ecx, [ebp + 20]\n"
        "1:\n"
        "  test ecx, ecx\n"
        "  jz 2f\n"
        "  push dword ptr [esi + ecx * 4 - 4]\n"
        "  dec ecx\n"
        "  jmp 1b\n"
        "2:\n"
        "  mov eax, 0x44444444\n"
        "  movd xmm3, eax\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, 0x33333333\n"
        "  mov ecx, [ebp + 12]\n"
        "  mov eax, [ebp + 8]\n"
        "  call eax\n"
        "  xor edx, edx\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 12]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  movd ecx, xmm3\n"
        "  cmp ecx, 0x44444444\n"
        "  jne 9f\n"
        "  mov edx, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 24]\n"
        "  mov [ecx], edx\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

unsigned g_month_trail;
void fake_month(void);
void fake_economy(void);
void fake_stocks(void);
void fake_economy_site(void);
void fake_stocks_site(void);
void fake_property(void);
void fake_property_site(void);
void fake_site_data(void);
void fake_sale_site(void);
void fake_rent_site(void);
void fake_offers_site(void);

/* Stand-ins for what a month end draws. fake_month is shaped like the part of 0x00695020 that calls the economy's
 * month and then the listed companies': ecx only at both calls, and both functions end in a plain `ret`. Each of the
 * two writes its digit behind g_month_trail (the economy 1, the companies 2) and adds [ecx] to g_trade_sum; the
 * companies' function returns 7 in eax, which fake_month adds as well.
 * fake_property_site is the month-start routine's call of the property market's month start, shaped the same: the
 * function writes 7 behind the trail, adds [ecx] and returns 9, which the routine adds. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _fake_economy\n"
        "_fake_economy:\n"
        "  imul eax, dword ptr [_g_month_trail], 10\n"
        "  add eax, 1\n"
        "  mov dword ptr [_g_month_trail], eax\n"
        "  mov eax, [ecx]\n"
        "  add dword ptr [_g_trade_sum], eax\n"
        "  ret\n"
        ".globl _fake_stocks\n"
        "_fake_stocks:\n"
        "  imul eax, dword ptr [_g_month_trail], 10\n"
        "  add eax, 2\n"
        "  mov dword ptr [_g_month_trail], eax\n"
        "  mov eax, [ecx]\n"
        "  add dword ptr [_g_trade_sum], eax\n"
        "  mov eax, 7\n"
        "  ret\n"
        ".globl _fake_month\n"
        "_fake_month:\n"
        "  push esi\n"
        "  mov esi, ecx\n"
        ".globl _fake_economy_site\n"
        "_fake_economy_site:\n"
        "  call _fake_economy\n"
        "  mov ecx, esi\n"
        ".globl _fake_stocks_site\n"
        "_fake_stocks_site:\n"
        "  call _fake_stocks\n"
        "  add dword ptr [_g_trade_sum], eax\n"
        "  pop esi\n"
        "  ret\n"
        ".globl _fake_property\n"
        "_fake_property:\n"
        "  imul eax, dword ptr [_g_month_trail], 10\n"
        "  add eax, 7\n"
        "  mov dword ptr [_g_month_trail], eax\n"
        "  mov eax, [ecx]\n"
        "  add dword ptr [_g_trade_sum], eax\n"
        "  mov eax, 9\n"
        "  ret\n"
        ".globl _fake_property_site\n"
        "_fake_property_site:\n"
        "  call _fake_property\n"
        "  add dword ptr [_g_trade_sum], eax\n"
        "  ret\n"
        /* the first call for a site in the two walks, and the call before the offers: each of the three stand-in
         * call sites calls fake_site_data, which writes 8 behind the trail and adds [ecx] */
        ".globl _fake_site_data\n"
        "_fake_site_data:\n"
        "  imul eax, dword ptr [_g_month_trail], 10\n"
        "  add eax, 8\n"
        "  mov dword ptr [_g_month_trail], eax\n"
        "  mov eax, [ecx]\n"
        "  add dword ptr [_g_trade_sum], eax\n"
        "  ret\n"
        ".globl _fake_sale_site\n"
        "_fake_sale_site:\n"
        "  call _fake_site_data\n"
        "  ret\n"
        ".globl _fake_rent_site\n"
        "_fake_rent_site:\n"
        "  call _fake_site_data\n"
        "  ret\n"
        ".globl _fake_offers_site\n"
        "_fake_offers_site:\n"
        "  call _fake_site_data\n"
        "  ret\n"
        ".att_syntax prefix\n");

unsigned g_steps_runs, g_fee_runs, g_fee_tag;
void fake_keep(void);
unsigned call_keep(void *fn, void *object);
void fake_steps(void);
void fake_fee(void);
void fake_steps_site(void);
void fake_fee_site(void);
void fake_listing(void *finance, long long *out_price, const unsigned *values, void *company, unsigned *intact);

/* Stand-ins for the listing.
 * fake_keep: the bytes of 0x00535c50, `movss xmm0, [ecx + 0x298]` / `ret`; call_keep returns the float's bits.
 * fake_steps: like 0x0053cb50 - address of the price in ecx, five stack words the caller removes; sets the price to
 * half the offer price. fake_fee: like 0x00542920 - object in ecx, six stack words, `ret 0x18`; adds the amount's
 * low word to [ecx] and remembers the tag.
 * fake_listing(finance, out_price, values, company, intact): a frame laid out like FUN_005362a0's - all shares at
 * [ebp - 0x3f0], founder's shares at [ebp - 0x3c0], the company at [ebp - 0x3e0], the price at [ebp - 0x3e8] - that
 * calls the steps, copies the price out and into the company (+0x100), then charges a fee of -200.
 * values = {all shares, founder's shares, offer price, earnings a share}, int64 each. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _fake_keep\n"
        "_fake_keep:\n"
        "  .byte 0xf3, 0x0f, 0x10, 0x81, 0x98, 0x02, 0x00, 0x00\n"
        "  ret\n"
        ".globl _call_keep\n"
        "_call_keep:\n"
        "  mov ecx, [esp + 8]\n"
        "  call dword ptr [esp + 4]\n"
        "  movd eax, xmm0\n"
        "  ret\n"
        ".globl _fake_steps\n"
        "_fake_steps:\n"
        "  mov eax, [esp + 4]\n"
        "  shr eax, 1\n"
        "  mov [ecx], eax\n"
        "  mov dword ptr [ecx + 4], 0\n"
        "  inc dword ptr [_g_steps_runs]\n"
        "  ret\n"
        ".globl _fake_fee\n"
        "_fake_fee:\n"
        "  mov eax, [esp + 4]\n"
        "  add [ecx], eax\n"
        "  mov eax, [esp + 12]\n"
        "  mov dword ptr [_g_fee_tag], eax\n"
        "  inc dword ptr [_g_fee_runs]\n"
        "  ret 0x18\n"
        ".globl _fake_listing\n"
        "_fake_listing:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  sub esp, 0x440\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  mov esi, [ebp + 16]\n"
        "  mov eax, [esi]\n"
        "  mov [ebp - 0x3f0], eax\n"
        "  mov eax, [esi + 4]\n"
        "  mov [ebp - 0x3ec], eax\n"
        "  mov eax, [esi + 8]\n"
        "  mov [ebp - 0x3c0], eax\n"
        "  mov eax, [esi + 12]\n"
        "  mov [ebp - 0x3bc], eax\n"
        "  mov eax, [ebp + 20]\n"
        "  mov [ebp - 0x3e0], eax\n"
        "  mov eax, 0x44444444\n"
        "  movd xmm3, eax\n"
        "  mov ebx, 0x11111111\n"
        "  mov edi, 0x33333333\n"
        "  push 0x77\n"
        "  push dword ptr [esi + 28]\n"
        "  push dword ptr [esi + 24]\n"
        "  push dword ptr [esi + 20]\n"
        "  push dword ptr [esi + 16]\n"
        "  lea ecx, [ebp - 0x3e8]\n"
        ".globl _fake_steps_site\n"
        "_fake_steps_site:\n"
        "  call _fake_steps\n"
        "  add esp, 0x14\n"
        "  mov eax, [ebp + 12]\n"
        "  mov edx, [ebp - 0x3e8]\n"
        "  mov [eax], edx\n"
        "  mov ecx, [ebp - 0x3e4]\n"
        "  mov [eax + 4], ecx\n"
        "  mov eax, [ebp - 0x3e0]\n"
        "  mov [eax + 0x100], edx\n"
        "  mov [eax + 0x104], ecx\n"
        "  push 0\n"
        "  push 0\n"
        "  push 1\n"
        "  push 0x80f\n"
        "  push -1\n"
        "  push -200\n"
        "  mov ecx, [ebp + 8]\n"
        ".globl _fake_fee_site\n"
        "_fake_fee_site:\n"
        "  call _fake_fee\n"
        "  xor edx, edx\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, [ebp + 16]\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 0x44c]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  movd ecx, xmm3\n"
        "  cmp ecx, 0x44444444\n"
        "  jne 9f\n"
        "  mov edx, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 24]\n"
        "  mov [ecx], edx\n"
        "  lea esp, [ebp - 0x44c]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  mov esp, ebp\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

static long long g_listing_seen[4], g_listing_cash; /* all shares, founder's shares, offer, earnings: what the steps callback saw */
static const void *g_listing_company;
static void *g_listing_finance;
static int g_listing_floor;
static unsigned g_draws_seen[2]; /* how often the price steps had run when the callback was told: before them, after them */
static int g_draws_told, g_draws_told_at_done;

static void test_steps_draws(int drawn)
{
    g_draws_seen[drawn] = g_steps_runs;
    g_draws_told++;
    __asm__("pxor %xmm3, %xmm3");
}

static void test_steps_done(const BYTE *frame, long long *price, long long offer, long long earnings_per_share)
{
    g_draws_told_at_done = g_draws_told;
    g_listing_seen[0] = *(const long long *)(frame + RE_IPO_FRAME_SHARES);
    g_listing_seen[1] = *(const long long *)(frame + RE_IPO_FRAME_FOUNDER);
    g_listing_seen[2] = offer;
    g_listing_seen[3] = earnings_per_share;
    *price = re_ipo_floor(*price, offer, earnings_per_share, g_listing_floor);
    __asm__("pxor %xmm3, %xmm3");
}

static void test_before_fee(const BYTE *frame, void *finance)
{
    const BYTE *company = *(const BYTE *const *)(frame + RE_IPO_FRAME_COMPANY);
    g_listing_company = company;
    g_listing_finance = finance;
    g_listing_cash = re_ipo_cash(*(const long long *)(company + RE_COMPANY_SHARES), *(const long long *)(frame + RE_IPO_FRAME_FOUNDER),
                                 *(const long long *)(company + RE_COMPANY_PRICE));
    __asm__("pxor %xmm3, %xmm3");
}

unsigned g_score_runs, g_score_frees;
void fake_score(void);
void fake_score_free(void);
void fake_score_site(void);
void fake_ownership(void);
void fake_election(void *company, void *score, int person, unsigned *intact);

/* Stand-ins for the board.
 * fake_score: like 0x0053efa0 - company in ecx, (score, person) on the stack, `ret 8`, returns the score's address;
 * the total (+0x14) becomes the person's number and the first word the company it was called with.
 * fake_score_free: like 0x00414400 - the score in ecx, plain `ret`; it counts and spoils the total.
 * fake_ownership: like 0x00535010 - object in ecx, one stack word, `ret 4`, result in xmm0 (the float the object
 * starts with); it also overwrites ebx, esi and edi, which a C caller expects preserved.
 * fake_election(company, score, person, intact): calls the score the way 0x0053e87c does and reports whether eax
 * is the score's address and ebx, esi, edi, esp and xmm3 are what they were. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _fake_score\n"
        "_fake_score:\n"
        "  mov eax, [esp + 4]\n"
        "  cvtsi2ss xmm0, dword ptr [esp + 8]\n"
        "  movss [eax + 0x14], xmm0\n"
        "  mov [eax], ecx\n"
        "  inc dword ptr [_g_score_runs]\n"
        "  ret 8\n"
        ".globl _fake_score_free\n"
        "_fake_score_free:\n"
        "  mov dword ptr [ecx + 0x14], 0xffffffff\n"
        "  inc dword ptr [_g_score_frees]\n"
        "  ret\n"
        ".globl _fake_ownership\n"
        "_fake_ownership:\n"
        "  movss xmm0, [ecx]\n"
        "  mov ebx, 0xdead\n"
        "  mov esi, 0xdead\n"
        "  mov edi, 0xdead\n"
        "  ret 4\n"
        ".globl _fake_election\n"
        "_fake_election:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, 0x33333333\n"
        "  mov eax, 0x44444444\n"
        "  movd xmm3, eax\n"
        "  push dword ptr [ebp + 16]\n"
        "  mov ecx, [ebp + 8]\n"
        "  push dword ptr [ebp + 12]\n"
        ".globl _fake_score_site\n"
        "_fake_score_site:\n"
        "  call _fake_score\n"
        "  xor edx, edx\n"
        "  cmp eax, [ebp + 12]\n"
        "  jne 9f\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 12]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  movd ecx, xmm3\n"
        "  cmp ecx, 0x44444444\n"
        "  jne 9f\n"
        "  mov edx, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 20]\n"
        "  mov [ecx], edx\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

static double g_board_per_seat;
static const BYTE *g_board_company;
static unsigned g_board_return;

/* what the plugin's callback does, with the test's own numbers */
static void test_scored(BYTE *company, BYTE *score, int person, unsigned return_address)
{
    int nominees[8], household[8];
    float totals[8], own;
    int count = re_board_nominees(company, nominees, 8), members = re_board_household(company, household, 8);
    float fraction = re_board_fraction(fake_ownership, *(void **)(company + RE_COMPANY_MARKET), *(int *)(company + RE_COMPANY_ID));
    g_board_company = company;
    g_board_return = return_address;
    memcpy(&own, score + RE_SCORE_TOTAL, sizeof own);
    /* every other nominee is scored into a scratch score, and each of those scores is destroyed again */
    int known = re_board_place(nominees, count, household, members, person) >= 0 &&
                re_board_totals(company, nominees, count, person, own, totals);
    int rank = re_board_rank(nominees, known ? totals : NULL, count, household, members, re_board_seats(fraction, g_board_per_seat), person);
    if (rank >= 0)
        *(float *)(score + RE_SCORE_TOTAL) = re_board_total(1000.0f, rank);
    __asm__("pxor %xmm3, %xmm3");
}

static int g_block, g_blocked_calls, g_blocked_trade;

static int test_blocked(int trade)
{
    g_blocked_calls++;
    g_blocked_trade = trade;
    __asm__("pxor %xmm3, %xmm3");
    return g_block;
}

/* writes 3 behind the trail before the month end's draws and 4 after them, 5 and 6 for the property market's, 7
 * before the offers */
static void test_month_draws(int moment)
{
    g_month_trail = g_month_trail * 10 + 3 + (unsigned)moment;
    __asm__("pxor %xmm3, %xmm3");
}

static int g_site_rent;
static const unsigned char *g_site_node;

/* writes 9 behind the trail and remembers which walk and which node it was told */
static void test_site_draws(int rent, const unsigned char *node)
{
    g_month_trail = g_month_trail * 10 + 9;
    g_site_rent = rent;
    g_site_node = node;
    __asm__("pxor %xmm3, %xmm3");
}

static int close_to(float a, double b)
{
    double d = (double)a - b;
    return d < 1e-6 && d > -1e-6;
}

/* the game's std::string assign, for results that fit the inline buffer */
static void *__thiscall fake_assign(void *self, const char *text, unsigned len)
{
    msvc_string *s = (msvc_string *)self;
    memcpy(s->buf, text, len);
    s->buf[len] = 0;
    s->size = len;
    s->capacity = 15;
    return self;
}

static unsigned g_want_caller;

static int g_frame_arg, g_want_seen, g_edit_seen;
static const void *g_frame_out;

/* 1 when the registers are the ones call_translate sets before its call and its frame, which stands while the
 * callbacks run, holds `g_frame_out` at `g_frame_arg`: its third argument is at [ebp + 16] */
static int caller_seen(const re_text_regs *regs)
{
    return regs->ebx == 0x11111111 && regs->esi == 0x22222222 && regs->edi == 0x33333333 &&
           *(const void *const *)(UINT_PTR)(regs->ebp + (unsigned)g_frame_arg) == g_frame_out;
}

static int test_want(const char *key, unsigned len, unsigned caller, const re_text_regs *regs)
{
    g_want_caller = caller;
    g_want_seen = caller_seen(regs);
    return len == 7 && memcmp(key, "rateKey", 7) == 0;
}

static unsigned test_edit(int want, char *text, unsigned len, unsigned cap, const re_text_regs *regs, const re_text_values *values)
{
    (void)want;
    (void)cap;
    (void)values;
    g_edit_seen = caller_seen(regs);
    memcpy(text + len, " [A]", 4);
    return len + 4;
}

static unsigned g_row_caller;
static int g_row_seen;

static void test_row(unsigned caller, const re_text_regs *regs, void *label, void *value, unsigned *red)
{
    msvc_string *amount = (msvc_string *)value;
    (void)label;
    g_row_caller = caller;
    g_row_seen = regs->ebx == 0x11111111 && regs->esi == 0x22222222 && regs->edi == 0x33333333;
    memcpy(amount->buf + amount->size, " [B]", 5);
    amount->size += 4;
    *red = 1;
}

static int g_sink_calls;
static void *g_sink_self;
static unsigned g_sink_stack[7];

static void test_sink(void *self, const unsigned *stack)
{
    g_sink_calls++;
    g_sink_self = self;
    memcpy(g_sink_stack, stack, sizeof g_sink_stack);
    __asm__("pxor %xmm3, %xmm3"); /* a sink is free to use any xmm register; the detour has to put it back */
}

static int g_policy_calls;

static unsigned test_policy(int id, int attr, unsigned bits)
{
    float out = 0.0123f;
    g_policy_calls++;
    if (id == RE_ID_PERSONAL_LOAN && attr == RE_ATTR_MOD)
        memcpy(&bits, &out, 4);
    return bits;
}

static msvc_string inline_name(const char *text)
{
    msvc_string s;
    memset(&s, 0, sizeof s);
    s.size = (unsigned)strlen(text);
    s.capacity = 15;
    memcpy(s.buf, text, s.size);
    return s;
}

/* Stand-ins for the two futures hooks. The assembly below reads and writes these, so they are not static. */
unsigned g_rate_economy, g_copy_string, g_copy_source;
static unsigned g_fut_listed_calls, g_fut_value_calls;
static void *g_fut_listed_economy;
static const unsigned char *g_fut_listed_first, *g_fut_listed_end, *g_fut_value_window;
static unsigned char *g_fut_value_string;

void fake_rate_list(void);
void fake_rate_site(void);
unsigned call_rate_list(void *economy, const void *first, const void *end, unsigned *intact);
void fake_copy(void);
void fake_value_site(void);
unsigned call_value(void *string, const void *source, const void *window, unsigned *intact);

__asm__(".intel_syntax noprefix\n"
        ".text\n"
        /* Stand-in for 0x004d44a0: thiscall, three stack words which it removes; returns the first of them. */
        ".globl _fake_rate_list\n"
        "_fake_rate_list:\n"
        "  mov [_g_rate_economy], ecx\n"
        "  mov eax, [esp + 4]\n"
        "  ret 12\n"
        /* unsigned call_rate_list(economy, first, end, intact): calls like the game through `fake_rate_site`;
         * returns eax, sets *intact when ebx, esi, edi, xmm6 and esp came back */
        ".globl _call_rate_list\n"
        "_call_rate_list:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, 0x33333333\n"
        "  mov eax, 0x44444444\n"
        "  movd xmm6, eax\n"
        "  push 0\n"
        "  push dword ptr [ebp + 16]\n"
        "  push dword ptr [ebp + 12]\n"
        "  mov ecx, [ebp + 8]\n"
        ".globl _fake_rate_site\n"
        "_fake_rate_site:\n"
        "  call _fake_rate_list\n"
        "  xor edx, edx\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  movd ecx, xmm6\n"
        "  cmp ecx, 0x44444444\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 12]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  mov edx, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 20]\n"
        "  mov [ecx], edx\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        /* Stand-in for 0x00405500: thiscall, the source on the stack, which it removes; returns the new string. */
        ".globl _fake_copy\n"
        "_fake_copy:\n"
        "  mov [_g_copy_string], ecx\n"
        "  mov eax, [esp + 4]\n"
        "  mov [_g_copy_source], eax\n"
        "  mov eax, ecx\n"
        "  ret 4\n"
        /* unsigned call_value(string, source, window, intact): calls like the futures window, edi = the window,
         * through `fake_value_site`; returns eax, sets *intact when ebx, esi, edi (still the window), xmm6 and
         * esp came back */
        ".globl _call_value\n"
        "_call_value:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, [ebp + 16]\n"
        "  mov eax, 0x44444444\n"
        "  movd xmm6, eax\n"
        "  push dword ptr [ebp + 12]\n"
        "  mov ecx, [ebp + 8]\n"
        ".globl _fake_value_site\n"
        "_fake_value_site:\n"
        "  call _fake_copy\n"
        "  xor edx, edx\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, [ebp + 16]\n"
        "  jne 9f\n"
        "  movd ecx, xmm6\n"
        "  cmp ecx, 0x44444444\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 12]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  mov edx, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 20]\n"
        "  mov [ecx], edx\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

/* the callbacks spoil xmm6 the way the plugin's own code may: the hooks have to put it back */
static void spoil_xmm6(void)
{
    volatile float spoil = 3.5f;
    __asm__ volatile("movss %0, %%xmm6" : : "m"(spoil) : "xmm6");
}

static void test_fut_listed(void *economy, const unsigned char *first, const unsigned char *end)
{
    spoil_xmm6();
    g_fut_listed_calls++;
    g_fut_listed_economy = economy;
    g_fut_listed_first = first;
    g_fut_listed_end = end;
}

static void test_fut_value(unsigned char *string, const unsigned char *window)
{
    spoil_xmm6();
    g_fut_value_calls++;
    g_fut_value_string = string;
    g_fut_value_window = window;
}

/* Stand-ins for the business hooks. The assembly below reads and writes these, so they are not static. */
unsigned g_eff_jobs, g_eff_job, g_list_firms, g_list_firm, g_list_job, g_assign_string, g_assign_text, g_assign_length, g_name_object;
char g_assign_copy[128];
static unsigned g_listed_calls, g_line_calls;
static void *g_listed_firms;
static unsigned char **g_listed_vector;
static const unsigned char *g_line_subject;
static const char *g_line_text; /* what the callback writes; NULL = nothing */

void fake_efficiency(void);
void fake_list(void);
void fake_list_site(void);
unsigned call_list(void *firms, void *vector, int firm, int job, unsigned *intact);
void fake_line_assign(void);
void fake_hire_site(void);
unsigned call_line(void *string, const void *subject, const char *text, unsigned *intact);
void fake_name(void);
void fake_name_site(void);
unsigned call_name(void *object, void *string, unsigned return_address, unsigned *intact);

__asm__(".intel_syntax noprefix\n"
        ".text\n"
        /* Stand-in for 0x0051a4f0: thiscall, two stack words which it removes, the float at [second word] in xmm0.
         * It spoils every register a function of the game might. */
        ".globl _fake_efficiency\n"
        "_fake_efficiency:\n"
        "  mov [_g_eff_jobs], ecx\n"
        "  mov eax, [esp + 4]\n"
        "  mov [_g_eff_job], eax\n"
        "  mov eax, [esp + 8]\n"
        "  movss xmm0, [eax]\n"
        "  mov ebx, 0x0badbad0\n"
        "  mov esi, 0x0badbad1\n"
        "  mov edi, 0x0badbad2\n"
        "  ret 8\n"
        /* Stand-in for 0x004e9ca0: thiscall, three stack words which it removes; returns 0x77777777. */
        ".globl _fake_list\n"
        "_fake_list:\n"
        "  mov [_g_list_firms], ecx\n"
        "  mov eax, [esp + 8]\n"
        "  mov [_g_list_firm], eax\n"
        "  mov eax, [esp + 12]\n"
        "  mov [_g_list_job], eax\n"
        "  mov eax, 0x77777777\n"
        "  ret 12\n"
        /* unsigned call_list(firms, vector, firm, job, intact): calls like the hire tab, through `fake_list_site`,
         * the `call rel32` the test redirects; returns eax, sets *intact when ebx, esi, edi and esp came back */
        ".globl _call_list\n"
        "_call_list:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, 0x33333333\n"
        "  push dword ptr [ebp + 20]\n"
        "  push dword ptr [ebp + 16]\n"
        "  push dword ptr [ebp + 12]\n"
        "  mov ecx, [ebp + 8]\n"
        ".globl _fake_list_site\n"
        "_fake_list_site:\n"
        "  call _fake_list\n"
        "  xor edx, edx\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 12]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  mov edx, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 24]\n"
        "  mov [ecx], edx\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        /* Stand-in for 0x00405950: thiscall, the text and its length on the stack, which it removes; keeps a copy
         * of the text. */
        ".globl _fake_line_assign\n"
        "_fake_line_assign:\n"
        "  mov [_g_assign_string], ecx\n"
        "  mov eax, [esp + 4]\n"
        "  mov [_g_assign_text], eax\n"
        "  mov eax, [esp + 8]\n"
        "  mov [_g_assign_length], eax\n"
        "  push esi\n"
        "  push edi\n"
        "  push ecx\n"
        "  mov esi, [esp + 16]\n"
        "  mov edi, offset _g_assign_copy\n"
        "  mov ecx, [esp + 20]\n"
        "  cmp ecx, 127\n"
        "  jbe 1f\n"
        "  mov ecx, 127\n"
        "1:\n"
        "  rep movsb\n"
        "  mov byte ptr [edi], 0\n"
        "  pop ecx\n"
        "  pop edi\n"
        "  pop esi\n"
        "  mov eax, ecx\n"
        "  ret 8\n"
        /* unsigned call_line(string, subject, text, intact): calls like the hire loop, esi = the subject, through
         * `fake_hire_site`; returns eax, sets *intact when ebx, esi (still the subject), edi, xmm6 and esp came back */
        ".globl _call_line\n"
        "_call_line:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, [ebp + 12]\n"
        "  mov edi, 0x33333333\n"
        "  mov eax, 0x44444444\n"
        "  movd xmm6, eax\n"
        "  push 0\n"
        "  push dword ptr [ebp + 16]\n"
        "  mov ecx, [ebp + 8]\n"
        ".globl _fake_hire_site\n"
        "_fake_hire_site:\n"
        "  call _fake_line_assign\n"
        "  xor edx, edx\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, [ebp + 12]\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  movd ecx, xmm6\n"
        "  cmp ecx, 0x44444444\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 12]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  mov edx, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 20]\n"
        "  mov [ecx], edx\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        /* Stand-in for 0x00668f70: thiscall, the string to fill on the stack, which it removes; returns it. */
        ".globl _fake_name\n"
        "_fake_name:\n"
        "  mov [_g_name_object], ecx\n"
        "  mov eax, [esp + 4]\n"
        "  ret 4\n"
        /* unsigned call_name(object, string, return_address, intact): calls like the row maker, ebx = an argument
         * base with `return_address` at [ebx + 4], through `fake_name_site`; returns eax, sets *intact when ebx,
         * esi, edi, xmm6 and esp came back */
        ".globl _call_name\n"
        "_call_name:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  push dword ptr [ebp + 16]\n"
        "  push 0\n"
        "  mov ebx, esp\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, 0x33333333\n"
        "  mov eax, 0x44444444\n"
        "  movd xmm6, eax\n"
        "  push dword ptr [ebp + 12]\n"
        "  mov ecx, [ebp + 8]\n"
        ".globl _fake_name_site\n"
        "_fake_name_site:\n"
        "  call _fake_name\n"
        "  xor edx, edx\n"
        "  lea ecx, [ebp - 20]\n"
        "  cmp ebx, ecx\n"
        "  jne 9f\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  movd ecx, xmm6\n"
        "  cmp ecx, 0x44444444\n"
        "  jne 9f\n"
        "  mov edx, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 20]\n"
        "  mov [ecx], edx\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

static void test_listed(void *firms, unsigned char **vector)
{
    g_listed_calls++;
    g_listed_firms = firms;
    g_listed_vector = vector;
}

/* spoils xmm6 the way the plugin's own code may: the hook has to put it back */
static unsigned test_line(const unsigned char *subject, char *out, unsigned cap)
{
    volatile float spoil = 3.5f;
    __asm__ volatile("movss %0, %%xmm6" : : "m"(spoil) : "xmm6");
    g_line_calls++;
    g_line_subject = subject;
    if (g_line_text == NULL)
        return 0;
    snprintf(out, cap, "%s", g_line_text);
    return (unsigned)strlen(out);
}

/* T19: stand-ins for the functions of the game that say what an offer's work costs. The assembly writes these. */
unsigned g_offer_copies, g_payout_firms, g_payout_firm, g_payout_type, g_payout_last, g_length_firms;
unsigned g_number_data, g_number_element, g_number_element_length, g_number_name, g_number_name_length, g_number_empty;

void fake_contract_copy(void);
void fake_payout(void);
void fake_length(void);
void fake_text_assign(void);
void fake_data_number(void);

__asm__(".intel_syntax noprefix\n"
        ".text\n"
        /* Stand-in for 0x0043df40: thiscall, the record to copy on the stack, which it removes; copies its 0x50 bytes
         * and returns the copy. */
        ".globl _fake_contract_copy\n"
        "_fake_contract_copy:\n"
        "  push esi\n"
        "  push edi\n"
        "  mov eax, ecx\n"
        "  mov edi, ecx\n"
        "  mov esi, [esp + 12]\n"
        "  mov ecx, 20\n"
        "  rep movsd\n"
        "  inc dword ptr [_g_offer_copies]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  ret 4\n"
        /* Stand-in for 0x004eae90: thiscall, the firm id and the 0x50-byte record by value, all of which it removes.
         * It notes the record's first and last word, ruins the first word of its copy as a destructor might, returns
         * the record's third word (the months) under 0x12345 in edx:eax and spoils every register a function of the
         * game might. */
        ".globl _fake_payout\n"
        "_fake_payout:\n"
        "  mov [_g_payout_firms], ecx\n"
        "  mov eax, [esp + 4]\n"
        "  mov [_g_payout_firm], eax\n"
        "  mov eax, [esp + 8]\n"
        "  mov [_g_payout_type], eax\n"
        "  mov eax, [esp + 0x54]\n"
        "  mov [_g_payout_last], eax\n"
        "  mov dword ptr [esp + 8], 0\n"
        "  mov eax, [esp + 16]\n"
        "  mov edx, 0x12345\n"
        "  mov ebx, 0x0badbad0\n"
        "  mov esi, 0x0badbad1\n"
        "  mov edi, 0x0badbad2\n"
        "  ret 0x54\n"
        /* Stand-in for 0x004ec2c0: thiscall, the months on the stack, which it removes; the months as a float in xmm0. */
        ".globl _fake_length\n"
        "_fake_length:\n"
        "  mov [_g_length_firms], ecx\n"
        "  cvtsi2ss xmm0, dword ptr [esp + 4]\n"
        "  mov ebx, 0x0badbad0\n"
        "  mov esi, 0x0badbad1\n"
        "  mov edi, 0x0badbad2\n"
        "  ret 4\n"
        /* Stand-in for 0x00405950: thiscall, the text and its length on the stack, which it removes. Counts the
         * strings that came empty with room for 15 letters, and keeps a text the way a long one is kept: its address
         * and its length. */
        ".globl _fake_text_assign\n"
        "_fake_text_assign:\n"
        "  cmp byte ptr [ecx], 0\n"
        "  jne 1f\n"
        "  cmp dword ptr [ecx + 16], 0\n"
        "  jne 1f\n"
        "  cmp dword ptr [ecx + 20], 15\n"
        "  jne 1f\n"
        "  inc dword ptr [_g_number_empty]\n"
        "1:\n"
        "  mov eax, [esp + 4]\n"
        "  mov [ecx], eax\n"
        "  mov eax, [esp + 8]\n"
        "  mov [ecx + 16], eax\n"
        "  mov eax, ecx\n"
        "  ret 8\n"
        /* Stand-in for 0x00816c30: thiscall, two std::string by value, which it removes; notes the text and the
         * length of each, returns 1.65 in xmm0 and spoils the registers. */
        ".globl _fake_data_number\n"
        "_fake_data_number:\n"
        "  mov [_g_number_data], ecx\n"
        "  mov eax, [esp + 4]\n"
        "  mov [_g_number_element], eax\n"
        "  mov eax, [esp + 20]\n"
        "  mov [_g_number_element_length], eax\n"
        "  mov eax, [esp + 28]\n"
        "  mov [_g_number_name], eax\n"
        "  mov eax, [esp + 44]\n"
        "  mov [_g_number_name_length], eax\n"
        "  mov eax, 0x3fd33333\n"
        "  movd xmm0, eax\n"
        "  mov ebx, 0x0badbad0\n"
        "  mov esi, 0x0badbad1\n"
        "  mov edi, 0x0badbad2\n"
        "  ret 0x30\n"
        ".att_syntax prefix\n");

/* Stand-ins for the casino's two hooks. The assembly below reads and writes these, so they are not static. */
unsigned g_attr_runs, g_attr_seen_id, g_action_calls, g_action_phase, g_action_self;
static unsigned g_string_frees, g_end_calls;
static unsigned char *g_end_actions, *g_end_record, *g_end_object;

void fake_attribute(void);
unsigned call_attribute(void *fn, const float *value, int id, const msvc_string *three, unsigned *intact);
void fake_action_money(void);
void fake_action_site(void);
unsigned call_action(void *self, void *record, void *object, unsigned phase, unsigned *intact);

/* Stand-in for 0x00808b10: the same first five bytes, fastcall with the object's id in edx, three std::string by
 * value which it would destroy (counted as one run), plain ret, the float at [ecx] in xmm0. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _fake_attribute\n"
        "_fake_attribute:\n"
        "  .byte 0x55, 0x8b, 0xec, 0x6a, 0xff\n" /* push ebp / mov ebp, esp / push -1 */
        "  add esp, 4\n"
        "  movss xmm0, [ecx]\n"
        "  inc dword ptr [_g_attr_runs]\n"
        "  mov [_g_attr_seen_id], edx\n"
        "  mov esp, ebp\n"
        "  pop ebp\n"
        "  ret\n"
        /* unsigned call_attribute(fn, value, id, three, intact): calls like the game (the caller pops the 72 bytes of
         * the three strings), returns the float's bits, sets *intact when ebx, esi, edi and esp came back unchanged */
        ".globl _call_attribute\n"
        "_call_attribute:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  sub esp, 72\n"
        "  mov esi, [ebp + 20]\n"
        "  mov edi, esp\n"
        "  mov ecx, 18\n"
        "  rep movsd\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, 0x33333333\n"
        "  mov ecx, [ebp + 12]\n"
        "  mov edx, [ebp + 16]\n"
        "  mov eax, [ebp + 8]\n"
        "  call eax\n"
        "  add esp, 72\n"
        "  xor edx, edx\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 12]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  mov edx, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 24]\n"
        "  mov [ecx], edx\n"
        "  movd eax, xmm0\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        /* Stand-in for 0x00438440: thiscall, 0x4c bytes of arguments which it removes, the phase at [esp + 0x48];
         * returns its first argument. */
        ".globl _fake_action_money\n"
        "_fake_action_money:\n"
        "  mov eax, [esp + 0x48]\n"
        "  mov [_g_action_phase], eax\n"
        "  mov [_g_action_self], ecx\n"
        "  inc dword ptr [_g_action_calls]\n"
        "  mov eax, [esp + 4]\n"
        "  ret 0x4c\n"
        /* unsigned call_action(self, record, object, phase, intact): pushes what the game's two callers push and
         * calls through `fake_action_site`, the `call rel32` the test redirects */
        ".globl _call_action\n"
        "_call_action:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, 0x33333333\n"
        "  push 0\n"                    /* the afford flag */
        "  push dword ptr [ebp + 20]\n" /* the phase */
        "  sub esp, 0x38\n"             /* 8 bytes and the two strings */
        "  push dword ptr [ebp + 16]\n"
        "  push dword ptr [ebp + 12]\n"
        "  push 0x55555555\n" /* the result pointer */
        "  mov ecx, [ebp + 8]\n"
        ".globl _fake_action_site\n"
        "_fake_action_site:\n"
        "  call _fake_action_money\n"
        "  xor edx, edx\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 12]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  mov edx, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 24]\n"
        "  mov [ecx], edx\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

static void __thiscall fake_string_free(void *string)
{
    (void)string;
    g_string_frees++;
}

static void test_casino_end(unsigned char *actions, unsigned char *record, unsigned char *object)
{
    g_end_calls++;
    g_end_actions = actions;
    g_end_record = record;
    g_end_object = object;
    __asm__("pxor %xmm3, %xmm3");
}

static unsigned g_start_calls;
static unsigned char *g_start_object;

static void test_casino_start(unsigned char *object)
{
    g_start_calls++;
    g_start_object = object;
}

static int g_failures;

static void expect(const char *what, int id, float value, const msvc_string *name, float want, int want_calls_delta)
{
    fake_object obj = {{0, 0, 0}, id, value};
    unsigned intact = 0, bits;
    int before = g_policy_calls;
    float got;
    bits = call_getter(fake_getter, &obj, name, &intact);
    memcpy(&got, &bits, 4);
    int ok = got == want && intact == 1 && g_policy_calls - before == want_calls_delta;
    g_failures += !ok;
    printf("  T2 %-44s got=%g want=%g registers_and_stack_intact=%u policy_calls=%d/%d %s\n", what, got, want, intact,
           g_policy_calls - before, want_calls_delta, ok ? "ok" : "FAIL");
}

static void print_line(const char *line)
{
    printf("%s\n", line);
}

static int g_control;

static int same_number(double a, double b)
{
    double d = a - b;
    return d < 1e-9 && d > -1e-9;
}

/* one load in T9 */
static void guard_load(const char *what, re_guard_record *rec, long long now, long long cap, int shift_on, long long want_unlock,
                       int want_draws, int want_capped, int want_generation)
{
    re_guard_decision d;
    re_guard_on_load(rec, now, cap, shift_on, -1, &d);
    int ok = d.unlock_tick == want_unlock + g_control && d.draws == want_draws && d.capped == want_capped &&
             rec->generation == want_generation;
    g_failures += !ok;
    printf("  T9 %-50s unlock=%lld/%lld draws=%d/%d capped=%d/%d generation=%d/%d %s\n", what, d.unlock_tick, want_unlock + g_control,
           d.draws, want_draws, d.capped, want_capped, rec->generation, want_generation, ok ? "ok" : "FAIL");
}

/* a language file of the game, next to the executable; 0 when it cannot be read */
static size_t language_file(const wchar_t *exe, const char *folder, char *out, size_t cap)
{
    wchar_t file[MAX_PATH], name[64];
    wcsncpy(file, exe, MAX_PATH - 1);
    file[MAX_PATH - 1] = 0;
    wchar_t *slash = wcsrchr(file, L'\\') ? wcsrchr(file, L'\\') : wcsrchr(file, L'/');
    _snwprintf(name, 64, L"modsLanguages\\%hs\\baseTranslate.xml", folder);
    name[63] = 0;
    if (slash == NULL || (size_t)(slash - file) + wcslen(name) + 2 >= MAX_PATH)
        return 0;
    wcscpy(slash + 1, name);
    FILE *f = _wfopen(file, L"rb");
    if (f == NULL)
        return 0;
    size_t n = fread(out, 1, cap - 1, f);
    fclose(f);
    out[n] = 0;
    return n;
}

/* the conversions of a printf format one after the other, "%s|%+.0f|" for example; "%%" is none */
static void conversions(const char *format, char *out, size_t cap)
{
    size_t n = 0;
    for (const char *p = format; *p; p++) {
        if (*p != '%')
            continue;
        if (p[1] == '%') {
            p++;
            continue;
        }
        const char *end = p + 1;
        while (*end && strchr("diuxXsfc", *end) == NULL)
            end++;
        for (const char *c = p; *c && c <= end && n + 2 < cap; c++)
            out[n++] = *c;
        out[n++] = '|';
        if (*end == 0)
            break;
        p = end;
    }
    out[n] = 0;
}

static int begins(const char *text, const char *with)
{
    return strncmp(text, with, strlen(with)) == 0;
}

/* T18: stand-ins for an XML document and for the destructor of the object that keeps them */
static unsigned g_doc_ends, g_doc_flags, g_loader_calls;
unsigned g_loader_ended, g_loader_ran;
static BYTE *g_loader_seen;

static void *__thiscall fake_doc_end(void *self, unsigned flags)
{
    g_doc_ends++;
    g_doc_flags |= flags;
    ((unsigned *)self)[1] = 0xdead;
    return self;
}

static void test_loader_ended(BYTE *loader)
{
    g_loader_calls++;
    g_loader_seen = loader;
}

void fake_loader_end(void);
unsigned call_loader_end(void *loader);

/* fake_loader_end: the first five bytes of the game's destructor, ecx = the object, plain ret.
 * call_loader_end(loader): calls it like the game with every register set, returns 1 when all of them and two of
 * the SSE registers came back unchanged. */
__asm__(".intel_syntax noprefix\n"
        ".text\n"
        ".globl _fake_loader_end\n"
        "_fake_loader_end:\n"
        "  .byte 0x55, 0x8b, 0xec, 0x6a, 0xff\n" /* push ebp / mov ebp, esp / push -1 */
        "  add esp, 4\n"
        "  mov [_g_loader_ended], ecx\n"
        "  inc dword ptr [_g_loader_ran]\n"
        "  pop ebp\n"
        "  ret\n"
        ".globl _call_loader_end\n"
        "_call_loader_end:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  mov eax, 0x44444444\n"
        "  movd xmm0, eax\n"
        "  movd xmm7, eax\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, 0x33333333\n"
        "  mov edx, 0x55555555\n"
        "  mov ecx, [ebp + 8]\n"
        "  call _fake_loader_end\n"
        "  xor eax, eax\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  cmp edx, 0x55555555\n"
        "  jne 9f\n"
        "  cmp ecx, [ebp + 8]\n"
        "  jne 9f\n"
        "  movd ecx, xmm0\n"
        "  cmp ecx, 0x44444444\n"
        "  jne 9f\n"
        "  movd ecx, xmm7\n"
        "  cmp ecx, 0x44444444\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 12]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  mov eax, 1\n"
        "9:\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

/* T21: stand-ins for the game's random streams. An engine is laid out as the game's: 624 words of a 32-bit Mersenne
 * Twister and the place in them. */
typedef struct {
    unsigned word[(RE_RAND_ENGINE_BYTES - 4) / 4], at;
} test_engine;

static void test_engine_seed(test_engine *e, unsigned seed)
{
    e->word[0] = seed;
    for (unsigned i = 1; i < 624; i++)
        e->word[i] = 1812433253u * (e->word[i - 1] ^ (e->word[i - 1] >> 30)) + i;
    e->at = 624;
}

static unsigned test_engine_next(test_engine *e)
{
    if (e->at >= 624) {
        for (unsigned i = 0; i < 624; i++) {
            unsigned y = (e->word[i] & 0x80000000u) | (e->word[(i + 1) % 624] & 0x7fffffffu);
            e->word[i] = e->word[(i + 397) % 624] ^ (y >> 1) ^ (y & 1 ? 0x9908b0dfu : 0);
        }
        e->at = 0;
    }
    unsigned y = e->word[e->at++];
    y ^= y >> 11;
    y ^= (y << 7) & 0x9d2c5680u;
    y ^= (y << 15) & 0xefc60000u;
    return y ^ (y >> 18);
}

static test_engine g_engines[RE_RAND_ENGINES], g_engines_before[RE_RAND_ENGINES], g_engine_elsewhere;
static unsigned char g_randgen[RE_RANDGEN_SPARE + 4], g_randgen_before[RE_RANDGEN_SPARE + 4];
static unsigned g_seeded_engines, g_seeded_order_wrong, g_list_draws[10];
static re_streams g_streams_kept;

/* Stand-in for 0x00670a70: ecx = the engine, a pointer to the seed on the stack, which it removes. */
static void __thiscall fake_engine_seed(test_engine *e, const unsigned *seed)
{
    g_seeded_order_wrong += e != &g_engines[g_seeded_engines];
    g_seeded_engines++;
    test_engine_seed(e, *seed);
}

static unsigned *test_stream(int i)
{
    return (unsigned *)(g_randgen + i * RE_RAND_UNIT_BYTES);
}

static unsigned test_draw(int stream)
{
    unsigned *unit = test_stream(stream);
    unit[RE_RAND_UNIT_CALLS / 4]++;
    return test_engine_next((test_engine *)(UINT_PTR)unit[RE_RAND_UNIT_ENGINE / 4]);
}

/* Stand-in for the making of a list: seven draws from the job stream, one from the first, two from a stream no
 * note names, and the thirteenth engine written over. `spoil` 1 puts another engine in a stream's place, 2 changes
 * a stream's seed: what a restore must not call "as before". */
static void test_list_made(int spoil)
{
    int job = RE_RANDGEN_JOB / RE_RAND_UNIT_BYTES;
    for (int i = 0; i < 7; i++)
        g_list_draws[i] = test_draw(job);
    g_list_draws[7] = test_draw(0);
    g_list_draws[8] = test_draw(3);
    g_list_draws[9] = test_draw(3);
    test_engine_seed(&g_engines[RE_RAND_STREAMS], 0xdecade);
    if (spoil == 1)
        test_stream(5)[RE_RAND_UNIT_ENGINE / 4] = (unsigned)(UINT_PTR)&g_engine_elsewhere;
    if (spoil == 2)
        test_stream(2)[0]++;
}

/* the streams as a game has them in its tenth year: every engine at another place */
static void test_streams_set(void)
{
    for (int i = 0; i < RE_RAND_ENGINES; i++) {
        test_engine_seed(&g_engines[i], 1790088944u + (unsigned)i);
        for (int k = 0; k < 100 + 37 * i; k++)
            test_engine_next(&g_engines[i]);
        if (i < RE_RAND_STREAMS) {
            test_stream(i)[0] = 1790088944u;
            test_stream(i)[RE_RAND_UNIT_CALLS / 4] = 100u + 37u * (unsigned)i;
            test_stream(i)[RE_RAND_UNIT_ENGINE / 4] = (unsigned)(UINT_PTR)&g_engines[i];
        }
    }
    *(test_engine **)(g_randgen + RE_RANDGEN_SPARE) = &g_engines[RE_RAND_STREAMS];
    memcpy(g_engines_before, g_engines, sizeof g_engines);
    memcpy(g_randgen_before, g_randgen, sizeof g_randgen);
    g_seeded_engines = g_seeded_order_wrong = 0;
}

/* Stand-ins for the two hooks of T21. The assembly below reads and writes these, so they are not static. */
unsigned g_ref_jobs, g_ref_job, g_ref_skill, g_ref_seed;
static unsigned g_listing_calls, g_listing_firm, g_listing_job, g_listing_before_list, g_ref_calls;
static void *g_listing_firms;
static int g_ref_new_seed; /* what the callback puts in the seed's place; 0 = it leaves the seed alone */

void fake_wage_ref(void);
void fake_wage_ref_site(void);
unsigned long long call_wage_ref(void *jobs, int job, void *skill, int seed, unsigned *intact);

__asm__(".intel_syntax noprefix\n"
        ".text\n"
        /* Stand-in for 0x0051b480: thiscall, three stack words which it removes; returns 0x12345 : the seed it got. */
        ".globl _fake_wage_ref\n"
        "_fake_wage_ref:\n"
        "  mov [_g_ref_jobs], ecx\n"
        "  mov eax, [esp + 4]\n"
        "  mov [_g_ref_job], eax\n"
        "  mov eax, [esp + 8]\n"
        "  mov [_g_ref_skill], eax\n"
        "  mov eax, [esp + 12]\n"
        "  mov [_g_ref_seed], eax\n"
        "  mov edx, 0x12345\n"
        "  ret 12\n"
        /* unsigned long long call_wage_ref(jobs, job, skill, seed, intact): calls like the generator of a person,
         * through `fake_wage_ref_site`; returns edx:eax, sets *intact when ebx, esi, edi, xmm6 and esp came back */
        ".globl _call_wage_ref\n"
        "_call_wage_ref:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, 0x33333333\n"
        "  mov eax, 0x44444444\n"
        "  movd xmm6, eax\n"
        "  push dword ptr [ebp + 20]\n"
        "  push dword ptr [ebp + 16]\n"
        "  push dword ptr [ebp + 12]\n"
        "  mov ecx, [ebp + 8]\n"
        ".globl _fake_wage_ref_site\n"
        "_fake_wage_ref_site:\n"
        "  call _fake_wage_ref\n"
        "  push eax\n"
        "  xor eax, eax\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  movd ecx, xmm6\n"
        "  cmp ecx, 0x44444444\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 16]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  mov eax, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 24]\n"
        "  mov [ecx], eax\n"
        "  mov eax, [ebp - 16]\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

/* the callbacks spoil xmm6 the way the plugin's own code may: the hooks have to put it back */
static void test_listing(void *firms, int firm, int job)
{
    volatile float spoil = 3.5f;
    __asm__ volatile("movss %0, %%xmm6" : : "m"(spoil) : "xmm6");
    g_listing_calls++;
    g_listing_firms = firms;
    g_listing_firm = (unsigned)firm;
    g_listing_job = (unsigned)job;
    g_listing_before_list = g_list_firms == 0; /* the game's function has not run yet */
}

static void test_wage_seed(int *seed)
{
    volatile float spoil = 3.5f;
    __asm__ volatile("movss %0, %%xmm6" : : "m"(spoil) : "xmm6");
    g_ref_calls++;
    if (g_ref_new_seed != 0)
        *seed = g_ref_new_seed;
}

/* Stand-ins for the hook of T22. The assembly below reads and writes the first three, so they are not static. */
unsigned g_brand_vector, g_brand_first_word, g_brand_calls;
static unsigned g_brand_seen, g_brand_seen_word;
static unsigned char *const *g_brand_seen_vector;

void fake_brand_list(void);
void fake_brand_site(void);
void call_brand_list(void *vector, unsigned word, unsigned *intact);

__asm__(".intel_syntax noprefix\n"
        ".text\n"
        /* Stand-in for 0x00807f30: ecx = the vector, the arguments on the stack for the caller to remove; it returns
         * the vector in eax. Plain ret. */
        ".globl _fake_brand_list\n"
        "_fake_brand_list:\n"
        "  mov [_g_brand_vector], ecx\n"
        "  mov eax, [esp + 4]\n"
        "  mov [_g_brand_first_word], eax\n"
        "  inc dword ptr [_g_brand_calls]\n"
        "  mov eax, ecx\n"
        "  ret\n"
        /* void call_brand_list(vector, word, intact): 0x68 bytes of arguments whose first word is `word`, the call
         * through `fake_brand_site`, the arguments removed; sets *intact when eax is the vector and ebx, esi, edi,
         * xmm6 and esp came back */
        ".globl _call_brand_list\n"
        "_call_brand_list:\n"
        "  push ebp\n"
        "  mov ebp, esp\n"
        "  push ebx\n"
        "  push esi\n"
        "  push edi\n"
        "  mov ebx, 0x11111111\n"
        "  mov esi, 0x22222222\n"
        "  mov edi, 0x33333333\n"
        "  mov eax, 0x44444444\n"
        "  movd xmm6, eax\n"
        "  sub esp, 0x68\n"
        "  mov eax, [ebp + 12]\n"
        "  mov [esp], eax\n"
        "  mov ecx, [ebp + 8]\n"
        ".globl _fake_brand_site\n"
        "_fake_brand_site:\n"
        "  call _fake_brand_list\n"
        "  add esp, 0x68\n"
        "  mov edx, eax\n"
        "  xor eax, eax\n"
        "  cmp edx, [ebp + 8]\n"
        "  jne 9f\n"
        "  cmp ebx, 0x11111111\n"
        "  jne 9f\n"
        "  cmp esi, 0x22222222\n"
        "  jne 9f\n"
        "  cmp edi, 0x33333333\n"
        "  jne 9f\n"
        "  movd ecx, xmm6\n"
        "  cmp ecx, 0x44444444\n"
        "  jne 9f\n"
        "  lea ecx, [ebp - 12]\n"
        "  cmp esp, ecx\n"
        "  jne 9f\n"
        "  mov eax, 1\n"
        "9:\n"
        "  mov ecx, [ebp + 16]\n"
        "  mov [ecx], eax\n"
        "  lea esp, [ebp - 12]\n"
        "  pop edi\n"
        "  pop esi\n"
        "  pop ebx\n"
        "  pop ebp\n"
        "  ret\n"
        ".att_syntax prefix\n");

/* spoils xmm6 the way the plugin's own code may: the hook has to put it back */
static void test_brand(unsigned char *const *vector, const unsigned *record)
{
    volatile float spoil = 3.5f;
    __asm__ volatile("movss %0, %%xmm6" : : "m"(spoil) : "xmm6");
    g_brand_seen++;
    g_brand_seen_vector = vector;
    g_brand_seen_word = record[0];
}

int main(int argc, char **argv)
{
    int control = argc > 2 && strcmp(argv[2], "--control") == 0;
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX | SEM_NOGPFAULTERRORBOX);
    if (argc < 2) {
        printf("usage: engine_test <TGL2.exe> [--control]\n");
        return 2;
    }

    /* T1: the real image, mapped only */
    wchar_t path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, path, MAX_PATH);
    HMODULE image = LoadLibraryExW(path, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (image == NULL) {
        printf("T1 cannot map the executable, error %lu\n", GetLastError());
        g_failures++;
    } else {
        BYTE *base = (BYTE *)image;
        const IMAGE_NT_HEADERS32 *nt = (const IMAGE_NT_HEADERS32 *)(base + ((const IMAGE_DOS_HEADER *)base)->e_lfanew);
        int pin = nt->FileHeader.TimeDateStamp == RE_PIN_TIMESTAMP && nt->OptionalHeader.SizeOfImage == RE_PIN_SIZE_OF_IMAGE;
        int bad = 0;
        for (int group = 0; group < RE_GROUP_COUNT; group++)
            bad += re_sites_verify(control ? base + 1 : base, group, print_line);
        printf("T1 mapped at %p (preferred 0x%x) build_pin=%d site_mismatches=%d\n", (void *)base, RE_IMAGE_BASE, pin, bad);
        g_failures += !pin + (bad != 0);
        /* The site table compares instructions; the code reads frames and objects through defines of its own. Each
         * define against the displacement inside the instruction that shows it: `at` bytes into the site, 1 or 4 wide. */
        static const struct {
            const char *name;
            unsigned va;
            int at, width, value;
        } offsets[] = {
            {"stock tab: company slot", RE_VA_STOCK_TAB_COMPANY, 2, 4, RE_STOCK_TAB_FRAME_COMPANY},
            {"stock list: cursor into the company", RE_VA_STOCK_LIST_CURSOR, 2, 1, RE_STOCK_LIST_CURSOR_OFFSET},
            {"stock list: cursor slot", RE_VA_STOCK_LIST_CURSOR, 5, 4, RE_STOCK_LIST_FRAME_CURSOR},
            {"debt tooltip: record argument", RE_VA_DEBT_TIP_RECORD, 2, 1, RE_DEBT_TIP_FRAME_RECORD},
            {"company: price", RE_VA_COMPANY_PRICE, 2, 4, RE_COMPANY_PRICE},
            {"company: equity", RE_VA_COMPANY_EQUITY, 2, 4, RE_COMPANY_EQUITY},
            {"company: earnings", RE_VA_COMPANY_EARNINGS, 2, 4, RE_COMPANY_EARNINGS},
            {"company: dividend", RE_VA_COMPANY_DIVIDEND, 2, 4, RE_COMPANY_DIVIDEND},
            {"company: month of a measure", RE_VA_COMPANY_DESPERATE, 2, 1, RE_COMPANY_DESPERATE_MONTH},
            {"company: month of the research", RE_VA_COMPANY_RESEARCHED, 8, 1, RE_COMPANY_RESEARCH_MONTH},
            {"company: nominees", RE_VA_BOARD_NOMINEES, 2, 4, RE_COMPANY_NOMINEES},
            {"company: notice flag", RE_VA_BOARD_NOTICE, 2, 4, RE_COMPANY_BOARD_NOTICE},
            {"company: people", RE_VA_COMPANY_PEOPLE, 2, 4, RE_COMPANY_PEOPLE},
            {"company: id", RE_VA_COMPANY_ID, 2, 4, RE_COMPANY_ID},
            {"company: market", RE_VA_COMPANY_MARKET, 2, 1, RE_COMPANY_MARKET},
            {"people: household ids", RE_VA_HOUSEHOLD_IDS, 2, 4, RE_PEOPLE_HOUSEHOLD},
            {"market: companies", RE_VA_MARKET_COMPANIES, 2, 4, RE_MARKET_COMPANIES},
            {"market: company inside a node", RE_VA_MARKET_COMPANY, 2, 1, RE_MARKET_NODE_COMPANY},
            {"ipo frame: market", RE_VA_IPO_MARKET, 2, 4, RE_IPO_FRAME_MARKET},
            {"ipo frame: company", RE_VA_IPO_GRANT, 2, 4, RE_IPO_FRAME_COMPANY},
            {"ipo frame: founder's shares", RE_VA_IPO_GRANT_SHARES, 4, 4, RE_IPO_FRAME_FOUNDER},
            {"ipo frame: all shares, high half", RE_VA_IPO_ALL_SHARES, 2, 4, RE_IPO_FRAME_SHARES + 4},
            {"main: the UI object", RE_VA_POSTLOAD_PANEL, 2, 4, RE_MAIN_UI},
            {"UI: the summary panel", RE_VA_POSTLOAD_PANEL + 6, 2, 4, RE_UI_SUMMARY},
            {"summary panel: shown", RE_VA_SUMMARY_SHOWN, 2, 1, RE_SUMMARY_SHOWN},
            {"research block: company slot", RE_VA_COMPANY_RESEARCHED, 2, 4, RE_STOCK_RESEARCH_FRAME_COMPANY},
            {"industry: id inside an element", RE_VA_INDUSTRY_ID, 7, 1, RE_INDUSTRY_ID},
            {"industry: size of an element", RE_VA_INDUSTRY_NEXT, 2, 1, RE_INDUSTRY_BYTES},
            {"market: economy", RE_VA_MARKET_RATES, 2, 4, RE_MARKET_ECONOMY},
            {"economy: rates", RE_VA_MARKET_RATES + 6, 1, 4, RE_ECON_RATES},
            {"market: holdings", RE_VA_MARKET_HOLDINGS, 2, 4, RE_MARKET_HOLDINGS},
            {"header: company copy", RE_VA_COMPANY_COPY, 2, 4, RE_HEADER_FRAME_COMPANY},
            {"company: name", RE_VA_COMPANY_NAME, 2, 4, RE_HEADER_FRAME_COMPANY + RE_COMPANY_NAME},
            /* [ebp + 0x4c] after `push ebp`: four bytes further from esp at the entry */
            {"action money: phase", RE_VA_MONEY_PHASE, 2, 1, RE_ACTION_MONEY_PHASE + 4},
            {"actions: finance", RE_VA_ACTION_FINANCE, 2, 1, RE_ACTION_FINANCE},
            {"actions: prices", RE_VA_ACTION_PRICES, 2, 1, RE_ACTION_PRICES},
            {"actions: people", RE_VA_ACTION_PEOPLE, 2, 1, RE_ACTION_PEOPLE},
            {"queue record: count", RE_VA_RECORD_COUNT, 2, 4, RE_RECORD_COUNT},
            {"activity: id", RE_VA_ACTIVITY_ID, 2, 1, RE_OBJ_ID},
            {"main: people", RE_VA_MAIN_PEOPLE, 2, 4, RE_MAIN_PEOPLE},
            {"people: cooldowns", RE_VA_PEOPLE_COOLDOWNS, 2, 4, RE_PEOPLE_COOLDOWNS},
            {"business: the month's offers", RE_VA_FIRM_OFFERS, 2, 4, RE_FIRM_OFFERS},
            {"business: how many contracts are signed", RE_VA_FIRM_PLACES_SIGNED, 2, 4, RE_FIRM_SIGNED + 4},
            {"business: its reputation", RE_VA_FIRM_REPUTATION, 4, 4, RE_FIRM_REPUTATION},
            {"business: its influence operation", RE_VA_FIRM_INFLUENCE, 2, 4, RE_FIRM_INFLUENCE},
            {"business: open or closed, set", RE_VA_FIRM_OPEN_SET, 2, 1, RE_FIRM_OPEN},
            {"business: open or closed, asked", RE_VA_FIRM_OPEN_TEST, 2, 1, RE_FIRM_OPEN},
            {"UI: the business window", RE_VA_UI_FIRM_WINDOW, 2, 4, RE_UI_FIRM_WINDOW},
            {"property: the type to grow into, the click", RE_VA_HOUSE_GROW_CLICK, 2, 4, RE_HOUSE_GROW},
            {"property: the type to grow into, the month end", RE_VA_HOUSE_GROW_ASKED, 2, 4, RE_HOUSE_GROW},
            {"person: the queue of actions", RE_VA_PERSON_QUEUE, 2, 4, RE_PERSON_QUEUE},
            {"queue node: the first string", RE_VA_QUEUE_STRINGS + 24, 2, 1, RE_QUEUE_NODE_STRING},
            {"queue node: the last string", RE_VA_QUEUE_STRINGS, 2, 1, RE_QUEUE_NODE_STRING + 3 * 0x18},
            {"queue node: its size", RE_VA_QUEUE_NODE_BYTES, 1, 4, RE_QUEUE_NODE_BYTES},
            {"queue record: hours gone", RE_VA_RECORD_GONE, 2, 1, RE_RECORD_GONE},
            {"queue record: the activity", RE_VA_RECORD_ACTIVITY, 2, 1, RE_RECORD_ACTIVITY},
            {"person: itself", RE_VA_PERSON_SELF, 2, 4, RE_PERSON_SELF},
            {"person: its id", RE_VA_PERSON_SELF_ID, 2, 1, RE_PERSON_SELF_ID},
            {"person: the windows", RE_VA_PERSON_WINDOWS, 2, 4, RE_PERSON_WINDOWS},
            {"activity hover text: activity slot", RE_VA_HOVER_ACTIVITY, 2, 4, RE_HOVER_FRAME_ACTIVITY},
            {"main: firms", RE_VA_MAIN_FIRMS, 2, 4, RE_MAIN_FIRMS},
            {"firms: jobs", RE_VA_FIRMS_JOBS, 2, 1, RE_FIRMS_JOBS},
            {"staff record: size", RE_VA_HIRE_STEP, 2, 4, RE_STAFF_BYTES},
            {"staff record: id", RE_VA_HIRE_ID, 2, 1, RE_STAFF_ID},
            {"staff record: hours (card)", RE_VA_CARD_HOURS, 2, 4, RE_CARD_FRAME_STAFF + RE_STAFF_HOURS},
            {"staff record: wage (card)", RE_VA_CARD_WAGE, 2, 4, RE_CARD_FRAME_STAFF + RE_STAFF_WAGE},
            {"staff record: skill part (card)", RE_VA_CARD_SKILL, 2, 4, RE_CARD_FRAME_STAFF + RE_STAFF_SKILL},
            {"staff record: job (card)", RE_VA_CARD_JOB, 2, 4, RE_CARD_FRAME_STAFF + RE_STAFF_JOB},
            {"wage hover text: the record's wage", RE_VA_WAGE_TIP_WAGE, 2, 4, RE_WAGE_TIP_FRAME_STAFF + RE_STAFF_WAGE},
            {"payout hover text: contract slot", RE_VA_OFFER_TIP_CONTRACT, 2, 1, RE_OFFER_TIP_FRAME_CONTRACT},
            {"payout hover text: offered flag", RE_VA_OFFER_TIP_OFFERED, 2, 1, RE_OFFER_TIP_FRAME_OFFERED},
            {"contract: total payout", RE_VA_CONTRACT_TOTAL, 2, 1, RE_CONTRACT_TOTAL},
            {"contract: months", RE_VA_CONTRACT_MONTHS, 4, 1, RE_CONTRACT_MONTHS},
            {"contract: hours", RE_VA_CONTRACT_HOURS, 2, 1, RE_CONTRACT_HOURS},
            /* a jump's operand counts from the instruction after it */
            {"stock list: the second button goes on at", RE_VA_STOCK_SORT_LISTING, 2, 4, RE_VA_STOCK_SORT_ON - (RE_VA_STOCK_SORT_LISTING + 6)},
            {"firm: signed contracts", RE_VA_SIGNED_CONTRACTS, 2, 4, RE_FIRM_SIGNED},
            {"signed contract: months in its node", RE_VA_SIGNED_MONTHS, 2, 1, RE_SIGNED_NODE_CONTRACT + RE_CONTRACT_MONTHS},
            {"signed contract: start in its node", RE_VA_SIGNED_START, 2, 1, RE_SIGNED_NODE_CONTRACT + RE_CONTRACT_START},
            {"contract: size (room for a copy)", RE_VA_OFFER_ARGUMENT, 2, 1, RE_CONTRACT_BYTES},
            /* `ret 0x54`: the firm id and the contract */
            {"payout: what it removes", RE_VA_CONTRACT_PAYOUT_RET, 1, 1, 4 + RE_CONTRACT_BYTES},
            {"payout: the contract's type", RE_VA_PAYOUT_TYPE, 2, 1, RE_PAYOUT_FRAME_CONTRACT + RE_CONTRACT_TYPE},
            {"payout: the contract's months", RE_VA_PAYOUT_MONTHS, 2, 1, RE_PAYOUT_FRAME_CONTRACT + RE_CONTRACT_MONTHS},
            {"payout: the contract's hours", RE_VA_PAYOUT_HOURS, 2, 1, RE_PAYOUT_FRAME_CONTRACT + RE_CONTRACT_HOURS},
            {"payout hover text: firm slot", RE_VA_OFFER_TIP_FIRM, 2, 1, RE_OFFER_TIP_FRAME_FIRM},
            {"firm: id", RE_VA_FIRM_ID, 2, 1, RE_FIRM_ID},
            {"firms: data", RE_VA_FIRMS_DATA, 2, 1, RE_FIRMS_DATA},
            {"firms: settings", RE_VA_FIRMS_SETTINGS, 2, 1, RE_FIRMS_SETTINGS},
            {"settings: revenue factor", RE_VA_SETTINGS_REVENUE, 4, 1, RE_SETTINGS_REVENUE},
            {"firms: economy", RE_VA_FIRMS_ECONOMY, 2, 1, RE_FIRMS_ECONOMY},
            {"settings: key of the wage policy", RE_VA_WAGE_POLICY_KEY, 6, 4, RE_TAG_WAGE_POLICY},
            {"settings: map of factors", RE_VA_WAGE_POLICY_MAP, 2, 4, RE_POLICY_VALUES},
            {"data: specials", RE_VA_DATA_SPECIALS, 2, 4, RE_DATA_SPECIALS},
            {"specials: value behind a type", RE_VA_SPECIALS_TYPE_VALUE, 2, 1, RE_SPECIALS_ID_VALUE},
            {"specials: value behind a job", RE_VA_SPECIALS_JOB_VALUE, 2, 1, RE_SPECIALS_ID_VALUE},
            {"specials: value behind a group", RE_VA_SPECIALS_GROUP_VALUE, 2, 1, RE_SPECIALS_TEXT_VALUE},
            {"specials: value behind a name", RE_VA_SPECIALS_NAME_VALUE, 2, 1, RE_SPECIALS_TEXT_VALUE},
            {"induced job: chance", RE_VA_INDUCED_CHANCE, 4, 1, RE_INDUCED_CHANCE},
            {"induced job: job", RE_VA_INDUCED_JOB, 2, 1, RE_INDUCED_JOB},
            {"induced job: size of an element", RE_VA_INDUCED_NEXT, 2, 1, RE_INDUCED_BYTES},
            {"futures window: economy", RE_VA_FUT_ECONOMY, 2, 1, RE_FUT_WINDOW_ECONOMY},
            {"futures window: months", RE_VA_FUT_MONTHS, 2, 4, RE_FUT_WINDOW_MONTHS},
            {"economy: growth", RE_VA_ECON_GROWTH, 4, 4, RE_ECON_GROWTH},
            {"economy: index map", RE_VA_ECON_INDEX, 2, 4, RE_ECON_INDEX},
            {"economy: custom data", RE_VA_ECON_CUSTOM, 2, 1, RE_ECON_CUSTOM},
            {"custom data: policy map", RE_VA_ECON_CUSTOM + 3, 1, 4, RE_CUSTOM_POLICY},
            {"custom data: weights map", RE_VA_CUSTOM_WEIGHTS, 2, 4, RE_CUSTOM_WEIGHTS},
            {"industry list: id behind the share", RE_VA_TAG_ID, 2, 1, RE_TAG_ID - RE_TAG_SHARE},
            {"industry list: size of an element", RE_VA_TAG_NEXT, 2, 1, RE_TAG_BYTES},
            {"loader: first list of documents", RE_VA_LOADER_END_FIRST, 2, 1, RE_LOADER_DOCS},
            {"loader: second list", RE_VA_LOADER_END_SECOND, 2, 1, RE_LOADER_DOCS + RE_LOADER_DOCS_STEP},
            {"loader: last list of a folder", RE_VA_LOADER_END_LAST, 2, 4,
             RE_LOADER_DOCS + (RE_LOADER_DOC_LISTS - 1) * RE_LOADER_DOCS_STEP},
            {"loader: list of the language files", RE_VA_LOADER_END_TEXT, 2, 4, RE_LOADER_TEXT_DOCS},
            {"document: its deleting destructor", RE_VA_LOADER_END_DOC, 4, 1, RE_XML_DOC_END_SLOT},
            {"company: board members", RE_VA_BOARD_MEMBERS, 2, 4, RE_COMPANY_BOARD_MEMBERS},
            {"firms: own businesses", RE_VA_FIRMS_OWN, 2, 4, RE_FIRMS_OWN},
            {"own businesses: record in its node", RE_VA_FIRMS_OWN_RECORD, 2, 1, RE_FIRMS_NODE_FIRM},
            {"firm: type", RE_VA_FIRM_TYPE, 2, 1, RE_FIRM_TYPE},
            {"firm: premises", RE_VA_FIRM_PREMISES, 2, 1, RE_FIRM_PREMISES},
            {"firm: name", RE_VA_FIRM_NAME, 2, 1, RE_FIRM_NAME},
            {"firm: furnishings factor", RE_VA_FIRM_FURNISH, 4, 4, RE_FIRM_FURNISH},
            {"firm: assets by job", RE_VA_FIRM_ASSETS, 2, 1, RE_FIRM_ASSETS},
            {"brand list: the advert in an element", RE_VA_BRAND_ADVERT, 2, 1, RE_ADVERT_ID},
            {"brand (window): what the caller removes", RE_VA_BRAND_WINDOW_ARGS, 2, 1, 0x68},
            {"advert switch: what it removes", RE_VA_ADVERT_SWITCH_RET, 1, 1, 12},
            {"firm desired: what it removes", RE_VA_FIRM_DESIRED_RET, 1, 1, 8},
            {"firm: owned powers", RE_VA_FIRM_OWNED, 1, 4, RE_FIRM_OWNED},
            {"map erase: what it removes", RE_VA_MAP_ERASE_RET, 1, 1, 12},
            {"ware info: what it removes", RE_VA_WARE_INFO_RET, 1, 1, 4 + 4 + 0x18},
            {"tag info: what it removes", RE_VA_TAG_INFO_RET, 1, 1, 4 + 4 + 0x18},
            {"info name: what it removes", RE_VA_INFO_NAME_RET, 1, 1, 4},
            {"ware price: what it removes", RE_VA_WARE_PRICE_RET, 1, 1, 4 + 0x18 + 0x18},
            {"firm: prices", RE_VA_FIRM_PRICES, 2, 4, RE_FIRM_PRICES},
            {"firm: data", RE_VA_FIRM_DATA, 2, 4, RE_FIRM_DATA},
            {"data: lists", RE_VA_DATA_LISTS, 2, 4, RE_DATA_LISTS},
            {"space used: what it removes", RE_VA_SPACE_USED_RET, 1, 1, 4},
            {"firm: space", RE_VA_FIRM_SPACE, 2, 4, RE_FIRM_SPACE},
            {"firm: houses", RE_VA_FIRM_HOUSES, 2, 4, RE_FIRM_HOUSES},
            {"houses: own", RE_VA_HOUSES_OWN, 2, 4, RE_HOUSES_OWN},
            {"houses: other", RE_VA_HOUSES_OTHER, 2, 4, RE_HOUSES_OTHER},
            {"post-message: what it removes", RE_VA_POST_MESSAGE_RET, 1, 1, 4 + 3 * 0x18},
            {"advert on: what it removes", RE_VA_ADVERT_ON_RET, 1, 1, 8},
            {"advert on: what it removes without the business", RE_VA_ADVERT_ON_RET_NONE, 1, 1, 8},
            {"advert price: what it removes", RE_VA_ADVERT_PRICE_RET, 1, 1, 8},
            {"ui: the hire window", RE_VA_UI_HIRE_WINDOW, 2, 4, RE_UI_HIRE_WINDOW},
            {"firm: adverts", RE_VA_FIRM_ADVERTS, 2, 4, RE_FIRM_ADVERTS},
            {"firm: adverts, end", RE_VA_FIRM_ADVERTS_END, 2, 4, RE_FIRM_ADVERTS + 4},
            {"advert: size", RE_VA_ADVERT_NEXT, 2, 1, RE_ADVERT_BYTES},
            {"firm: awareness", RE_VA_FIRM_AWARENESS, 4, 4, RE_FIRM_AWARENESS},
            {"firm: advert costs", RE_VA_FIRM_ADVERT_COSTS, 2, 4, RE_FIRM_ADVERT_COSTS},
            {"advert costs: amount in its node", RE_VA_ADVERT_COST_VALUE, 2, 1, RE_ADVERT_COST_VALUE},
            {"firm: jobs", RE_VA_FIRM_JOBS, 2, 4, RE_FIRM_JOBS},
            {"job: hours added", RE_VA_JOB_OVERTIME, 2, 1, RE_JOB_NODE_OVERTIME},
            {"job: hours done", RE_VA_JOB_WORKED, 2, 1, RE_JOB_NODE_WORKED},
            {"job: hours given", RE_VA_JOB_ALLOWED, 2, 1, RE_JOB_NODE_ALLOWED},
            {"job: outsourced", RE_VA_JOB_OUTSOURCED, 2, 1, RE_JOB_NODE_OUTSOURCED},
            {"job: its kind", RE_VA_JOB_KIND, 2, 1, RE_JOB_NODE_KIND},
            {"firm tick: the firm in its node", RE_VA_FIRM_TICK_FIRM, 2, 1, RE_FIRMS_NODE_FIRM},
            /* `ret 0xc`: the firm, the job and the candidate */
            {"hire: what it removes", RE_VA_HIRE_RET, 1, 1, 12},
            {"advert tip: the window's firm", RE_VA_ADVERT_TIP_FIRM, 2, 4, RE_ADVERT_WINDOW_FIRM},
            {"advert window: the sprite in the frame", RE_VA_ADVERT_LOOK_SPRITE, 2, 4, RE_ADVERT_BUILD_FRAME_SPRITE},
            {"firm: stockpile", RE_VA_FIRM_STOCKPILE, 2, 4, RE_FIRM_STOCKPILE},
            {"asset node: item type", RE_VA_ASSET_NODE_TYPE, 2, 1, RE_ASSET_NODE_TYPE},
            {"firm: auto asset", RE_VA_FIRM_AUTO_ASSET, 2, 4, RE_FIRM_AUTO_ASSET},
            {"firm: supply hours taken", RE_VA_FIRM_SUPPLY_HOURS, 2, 4, RE_FIRM_SUPPLY_HOURS},
            {"firms: staff", RE_VA_FIRMS_STAFF, 2, 1, RE_FIRMS_STAFF},
            {"staff: by firm", RE_VA_STAFF_BY_FIRM, 2, 4, RE_STAFF_BY_FIRM},
            {"staff by firm: list in its node", RE_VA_STAFF_BY_FIRM_LIST, 2, 1, RE_STAFF_BY_FIRM_LIST},
            {"staff record: wage (set wage)", RE_VA_SET_WAGE_WAGE, 2, 4, RE_STAFF_WAGE},
            {"staff record: demand (set wage)", RE_VA_SET_WAGE_DEMAND, 2, 4, RE_STAFF_DEMAND},
            {"staff record: demand (left open)", RE_VA_DEMAND_STORE, 2, 4, RE_STAFF_DEMAND},
            {"staff record: demand, high half", RE_VA_DEMAND_OPEN_HIGH, 2, 4, RE_STAFF_DEMAND + 4},
            {"staff record: demand, low half", RE_VA_DEMAND_OPEN_LOW, 2, 4, RE_STAFF_DEMAND},
            {"staff record: id (demand accepted)", RE_VA_WAGE_ACCEPT_ID, 2, 1, RE_STAFF_ID},
            /* `ret 0x10`: the firm id, the staff id and the two halves of the wage, which the wrapper takes in its place */
            {"set wage: what it removes", RE_VA_STAFF_SET_WAGE_RET, 1, 1, 16},
            {"staff record: start month", RE_VA_HIRE_START, 2, 4, RE_STAFF_START},
            {"staff record: hours left", RE_VA_STAFF_REFILL, 2, 4, RE_STAFF_LEFT},
            {"staff record: size (list end)", RE_VA_STAFF_LIST_END_STEP, 2, 4, RE_STAFF_BYTES},
            {"staff record: the person (card)", RE_VA_CARD_PERSON, 2, 1, RE_CARD_FRAME_STAFF + RE_STAFF_FIRST_NAME},
            /* the length of a std::string is 0x10 into it */
            {"staff record: first name's length", RE_VA_NAME_FIRST, 2, 1, RE_STAFF_FIRST_NAME + 0x10},
            {"staff record: last name", RE_VA_NAME_LAST, 2, 1, RE_STAFF_LAST_NAME},
            {"recruiter's fee: the staff tag", RE_VA_RECRUIT_FEE_TAG, 1, 4, RE_TAG_STAFF},
            {"firms: candidates", RE_VA_FIRMS_CANDIDATES, 2, 4, RE_FIRMS_CANDIDATES},
            {"candidates: lists of a firm in its node", RE_VA_CANDIDATES_JOBS, 2, 1, RE_CANDIDATES_JOBS},
            {"candidates: lists of a firm (month end)", RE_VA_MONTHEND_JOBS, 2, 1, RE_CANDIDATES_JOBS},
            /* a node: 0x10, the key, then {head, count} or {first, end, room} */
            {"candidates: size of a firm's node", RE_VA_MONTHEND_FIRM_NODE, 1, 1, RE_CANDIDATES_JOBS + 8},
            {"candidates: size of a job's node", RE_VA_STAFF_TREE_NODE, 1, 1, RE_STAFF_BY_FIRM_LIST + 12},
            {"new node: where its list is", RE_VA_STAFF_LIST_NEW, 2, 1, RE_STAFF_BY_FIRM_LIST},
            {"map of lists: the key of a node", RE_VA_STAFF_LIST_KEY, 2, 1, RE_STAFF_BY_FIRM_KEY},
            {"streams: size of an engine", RE_VA_RANDGEN_ENGINE_NEW, 1, 4, RE_RAND_ENGINE_BYTES},
            {"engine: the place behind its words", RE_VA_RAND_SEED_PLACE, 2, 4, RE_RAND_ENGINE_BYTES - 4},
            {"engine: how many words", RE_VA_RAND_SEED_WORDS, 6, 4, (RE_RAND_ENGINE_BYTES - 4) / 4},
            {"streams: size of one", RE_VA_RANDGEN_SECOND, 2, 1, RE_RAND_UNIT_BYTES},
            {"streams: the last of them", RE_VA_RANDGEN_LAST, 2, 4, (RE_RAND_STREAMS - 1) * RE_RAND_UNIT_BYTES},
            {"streams: the thirteenth engine", RE_VA_RANDGEN_SPARE, 2, 4, RE_RANDGEN_SPARE},
            {"streams: the thirteenth engine (wage)", RE_VA_WAGE_REF_ENGINE, 2, 4, RE_RANDGEN_SPARE},
            {"stream: its engine", RE_VA_RAND_UNIT_ENGINE, 2, 1, RE_RAND_UNIT_ENGINE},
            {"stream: its calls", RE_VA_RAND_UNIT_CALLS, 2, 1, RE_RAND_UNIT_CALLS},
            {"firms: streams", RE_VA_FIRMS_RANDGEN, 2, 1, RE_FIRMS_RANDGEN},
            {"streams: the job stream", RE_VA_RANDGEN_JOB, 2, 1, RE_RANDGEN_JOB},
            {"jobs: streams", RE_VA_JOBS_RANDGEN, 2, 1, RE_JOBS_RANDGEN},
            {"firms: the object of the ages", RE_VA_FIRMS_AGES, 2, 1, RE_FIRMS_AGES},
            {"ages: streams", RE_VA_AGES_RANDGEN, 2, 1, RE_AGES_RANDGEN},
            {"reference wage: the id as its seed", RE_VA_WAGE_REF_ID, 2, 1, RE_STAFF_ID},
            /* `ret 0xc`: the job, the skill part and the seed, which the hook reaches 12 bytes above the return address */
            {"reference wage: what it removes", RE_VA_WAGE_REF_RET, 1, 1, 12},
            {"hire window: keys held", RE_VA_STAFF_TAB_KEYS, 2, 1, RE_WINDOW_KEYS},
            {"keys: left Shift", RE_VA_STAFF_TAB_SHIFT, 3, 4, RE_KEY_SHIFT},
            {"keys: right Shift", RE_VA_STAFF_TAB_SHIFT_RIGHT, 3, 4, RE_KEY_SHIFT_RIGHT},
            {"market: price history", RE_VA_MARKET_HISTORY, 2, 4, RE_MARKET_HISTORY},
            {"price history: the price's tag", RE_VA_HISTORY_PRICE, 1, 4, RE_STAT_PRICE},
            {"stock window: keys held", RE_VA_STOCK_ROW_KEYS, 2, 1, RE_WINDOW_KEYS},
            {"keys: left Ctrl", RE_VA_STOCK_ROW_CTRL, 3, 4, RE_KEY_CTRL},
            {"keys: right Ctrl", RE_VA_STOCK_ROW_CTRL_RIGHT, 3, 4, RE_KEY_CTRL_RIGHT},
            {"key held: what it removes", RE_VA_KEY_HELD_RET, 1, 1, 4},
            {"stock show: what it removes", RE_VA_STOCK_SHOW_RET, 1, 1, 4},
            {"research button: the company's id", RE_VA_RESEARCH_BUTTON_COMPANY, 2, 4, RE_COMPANY_ID},
            {"list item: the name in the company's copy", RE_VA_STOCK_ITEM_NAME, 2, 4, RE_STOCK_ITEM_FRAME_NAME},
            {"chart: the colours", RE_VA_CHART_COLOURS, 2, 4, RE_CHART_COLOURS},
            {"chart: a colour in its node", RE_VA_COLOUR_NODE_VALUE, 2, 1, RE_COLOUR_NODE_VALUE},
            {"chart drawing: the month in the frame", RE_VA_CHART_DRAW_MONTH, 2, 4, RE_CHART_DRAW_FRAME_MONTH},
            {"chart drawing: the chart in the frame", RE_VA_CHART_DRAW_CHART, 2, 4, RE_CHART_DRAW_FRAME_CHART},
            {"chart time: the months shown", RE_VA_CHART_SHOWN, 2, 4, RE_CHART_SHOWN},
            {"chart time: the most months", RE_VA_CHART_TIME_STEP + 22, 2, 4, RE_CHART_MOST},
            {"chart: its statement", RE_VA_CHART_STATEMENT, 2, 4, RE_CHART_STATEMENT},
            {"chart box: the series in the frame", RE_VA_CHART_BOX_SERIES, 2, 4, RE_CHART_BOX_FRAME_SERIES},
            {"chart box: the chart in the frame", RE_VA_CHART_BOX_CHART, 2, 4, RE_CHART_BOX_FRAME_CHART},
            /* `ret 4`: the key's address */
            {"map<int, ...> at: what it removes", RE_VA_MAP_INT_AT_RET, 1, 1, 4},
            {"experience: what requirements are measured against", RE_VA_XP_EFFECTIVE, 2, 1, RE_XP_EFFECTIVE},
            {"experience: what was gained", RE_VA_XP_GAINED, 2, 1, RE_XP_GAINED},
            {"person: the experience", RE_VA_XP_STATEMENT, 2, 4, RE_PERSON_XP},
            /* `ret 8`: the source and one more word */
            {"map copy: what it removes", RE_VA_MAP_COPY_RET, 1, 1, 8},
            /* `ret 4`: the ware */
            {"ware rate: what it removes", RE_VA_WARE_RATE_RET, 1, 1, 4},
            {"futures list: the row's ware in the frame", RE_VA_FUT_LIST_ASSET, 2, 1, RE_FUT_LIST_FRAME_ASSET},
            {"futures item: the ware", RE_VA_FUT_ITEM_ASSET, 2, 1, RE_FUT_ITEM_FRAME_ASSET},
            {"item: its title", RE_VA_ITEM_TITLE, 2, 1, RE_ITEM_TITLE},
            {"node: the slot of its colour", RE_VA_NODE_COLOUR, 2, 4, RE_NODE_COLOUR},
            {"item handed on: what it removes", RE_VA_ITEM_MADE_RET, 1, 1, 0x10},
            {"company: the size of a copy", RE_VA_COMPANY_STEP, 2, 4, RE_COMPANY_SIZE},
            {"company copy: what it removes", RE_VA_COMPANY_CLONE_RET, 1, 1, 4},
            {"company assign: what it removes", RE_VA_COMPANY_ASSIGN_RET, 1, 1, 4},
            {"chart: the all flag", RE_VA_CHART_ALL, 3, 4, RE_CHART_ALL},
            {"chart: the boxes", RE_VA_CHART_BOXES, 2, 4, RE_CHART_BOXES},
            {"chart: what is ticked", RE_VA_CHART_TICKS, 2, 4, RE_CHART_TICKS},
            {"tick: the value in its node", RE_VA_TICK_NODE_VALUE, 2, 1, RE_TICK_NODE_VALUE},
            {"chart: its kind", RE_VA_CHART_KIND, 2, 4, RE_CHART_KIND},
            {"chart tick: what it removes", RE_VA_CHART_TICK_RET, 1, 1, 8},
            {"scene: real estate", RE_VA_MAIN_RESTATE, 2, 4, RE_MAIN_RESTATE},
            {"real estate: for sale", RE_VA_RESTATE_FOR_SALE, 2, 4, RE_RESTATE_FOR_SALE},
            {"for sale: record in its node", RE_VA_FOR_SALE_RECORD, 2, 1, RE_FOR_SALE_NODE_RECORD},
            {"for sale: size of a record", RE_VA_FOR_SALE_BYTES, 3, 4, RE_HOUSE_BYTES},
            {"house: state", RE_VA_HOUSE_STATE, 2, 1, RE_HOUSE_STATE},
            {"house: the state for sale", RE_VA_HOUSE_STATE, 5, 1, RE_HOUSE_FOR_SALE},
            {"house: asking price", RE_VA_HOUSE_ASKING, 2, 1, RE_HOUSE_ASKING},
            {"house: address", RE_VA_HOUSE_ADDRESS, 2, 1, RE_HOUSE_ADDRESS},
            {"scene: pfm", RE_VA_PFM_STORE, 2, 4, RE_MAIN_PFM},
            {"scene: economy", RE_VA_ECONOMY_STORE, 2, 4, RE_MAIN_ECONOMY},
            {"scene: restate", RE_VA_RESTATE_STORE, 2, 4, RE_MAIN_RESTATE},
            {"scene: firms", RE_VA_FIRMS_STORE, 2, 4, RE_MAIN_FIRMS},
            {"scene: market", RE_VA_MARKET_STORE, 2, 4, RE_MAIN_MARKET},
            {"scene: relations", RE_VA_RELATIONS_STORE, 2, 4, RE_MAIN_RELATIONS},
            {"scene: rich", RE_VA_RICH_STORE, 2, 4, RE_MAIN_RICH},
            {"scene: moods", RE_VA_MOODS_STORE, 2, 4, RE_MAIN_MOODS},
            {"scene: achieve", RE_VA_ACHIEVE_STORE, 2, 4, RE_MAIN_ACHIEVE},
        };
        int offsets_wrong = 0;
        for (int i = 0; i < (int)(sizeof offsets / sizeof offsets[0]); i++) {
            const BYTE *at = base + (offsets[i].va - RE_IMAGE_BASE) + offsets[i].at;
            int found = offsets[i].width == 1 ? *(const signed char *)at : *(const int *)at;
            if (found != offsets[i].value + control) {
                offsets_wrong++;
                printf("  T1 offset %-36s the game 0x%x, the plugin 0x%x\n", offsets[i].name, (unsigned)found, (unsigned)offsets[i].value);
            }
        }
        printf("T1 frame and object offsets against the instructions that show them: wrong=%d of %d\n", offsets_wrong,
               (int)(sizeof offsets / sizeof offsets[0]));
        g_failures += offsets_wrong != 0;
        /* the three `call [slot]` that make an advert icon's colour call through the slot the plugin names: an
         * address inside the instruction, so it has moved with the image */
        static const unsigned colour[3] = {RE_VA_ADVERT_LOOK_ON, RE_VA_ADVERT_LOOK_OFF, RE_VA_ADVERT_LOOK_BUILD};
        int slots_wrong = 0;
        for (int i = 0; i < 3; i++)
            slots_wrong += *(BYTE **)(base + (colour[i] - RE_IMAGE_BASE) + 2) != base + (RE_VA_COLOUR3_NEW - RE_IMAGE_BASE) + 4 * control;
        printf("T1 the slot of an advert icon's three colour calls: wrong=%d of 3 %s\n", slots_wrong, slots_wrong == 0 ? "ok" : "FAIL");
        g_failures += slots_wrong != 0;
        /* the same for a chart's lines: the call of drawLine, and the slot of drawSegment that stands in for it */
        int line_slots_wrong = (*(BYTE **)(base + (RE_VA_CHART_LINE - RE_IMAGE_BASE) + 2) != base + (RE_VA_DRAW_LINE_SLOT - RE_IMAGE_BASE) + 4 * control) +
                               (*(BYTE **)(base + (RE_VA_CHART_AXIS - RE_IMAGE_BASE) + 2) != base + (RE_VA_DRAW_SEGMENT_SLOT - RE_IMAGE_BASE) + 4 * control);
        printf("T1 the slots of a chart's line and of its axes: wrong=%d of 2 %s\n", line_slots_wrong, line_slots_wrong == 0 ? "ok" : "FAIL");
        g_failures += line_slots_wrong != 0;
        /* the entry of the documents' table that the plugin calls is the function the sites describe */
        int doc_end = *(BYTE **)(base + (RE_VA_XML_DOC_TABLE - RE_IMAGE_BASE) + RE_XML_DOC_END_SLOT + 4 * control) ==
                      base + (RE_VA_XML_DOC_END - RE_IMAGE_BASE);
        printf("T1 the XML document's table has its deleting destructor where the plugin calls it: %d\n", doc_end);
        g_failures += !doc_end;
    }

    /* T2: the detour itself */
    msvc_string mod = inline_name("interestMod"), xer = inline_name("interestXer"), other = inline_name("instrument");
    msvc_string heap = inline_name("x");
    heap.size = 20;
    heap.capacity = 31; /* a string that would live on the heap: must never be inspected */
    expect("before the detour, personal loan interestMod", RE_ID_PERSONAL_LOAN, 0.10f, &mod, 0.10f, 0);
    re_getter_policy = control ? NULL : test_policy;
    int installed = re_detour5((BYTE *)fake_getter, RE_GETTER_PROLOGUE, re_getter_hook, &re_getter_trampoline);
    printf("  T2 detour installed=%d\n", installed);
    g_failures += !installed;
    expect("other object (fast path)", 5, 7.5f, &mod, 7.5f, 0);
    expect("personal loan interestMod (overridden)", RE_ID_PERSONAL_LOAN, 0.10f, &mod, 0.0123f, 1);
    expect("personal loan interestXer (seen, unchanged)", RE_ID_PERSONAL_LOAN, 1.5f, &xer, 1.5f, 1);
    expect("mortgage interestMod (seen, unchanged)", RE_ID_MORTGAGE, 0.02f, &mod, 0.02f, 1);
    expect("personal loan, other attribute", RE_ID_PERSONAL_LOAN, 1.0f, &other, 1.0f, 0);
    expect("mortgage, long (heap) name", RE_ID_MORTGAGE, 3.0f, &heap, 3.0f, 0);
    for (int i = 0; i < 100000; i++) { /* the fast path is the hot one in the game */
        fake_object obj = {{0, 0, 0}, i, 1.0f};
        unsigned intact = 0;
        call_getter(fake_getter, &obj, &mod, &intact);
        if (!intact) {
            g_failures++;
            printf("  T2 repeated call %d damaged registers or stack\n", i);
            break;
        }
    }

    /* T3: the observing detour on a 6-byte prologue */
    unsigned self_value[1] = {1000}, intact = 0;
    const unsigned args[6] = {7, 0xffffffffu, 71111, 1, 0, 0}; /* amount low word 7, high word -1, tag 71111 */
    unsigned want = 1000 + 7 + 71111;
    unsigned got = call_money(fake_money, self_value, args, &intact);
    int ok = got == want && intact == 1;
    g_failures += !ok;
    printf("  T3 before the detour                           got=%u want=%u intact=%u %s\n", got, want, intact, ok ? "ok" : "FAIL");
    re_trace_money_sink = test_sink;
    installed = re_detour((BYTE *)fake_money, RE_CHANGE_MONEY_PROLOGUE, sizeof RE_CHANGE_MONEY_PROLOGUE, re_trace_money_hook,
                          &re_trace_money_trampoline);
    printf("  T3 detour installed=%d\n", installed);
    g_failures += !installed;
    intact = 0;
    got = call_money(fake_money, self_value, args, &intact);
    unsigned want_tag = control ? 71112 : 71111;
    ok = got == want && intact == 1 && g_sink_calls == 1 && g_sink_self == (void *)self_value && g_sink_stack[0] != 0 &&
         g_sink_stack[1] == 7 && g_sink_stack[2] == 0xffffffffu && g_sink_stack[3] == want_tag;
    g_failures += !ok;
    printf("  T3 with the detour                             got=%u want=%u intact=%u sink_calls=%d sink_tag=%u/%u %s\n", got, want,
           intact, g_sink_calls, g_sink_stack[3], want_tag, ok ? "ok" : "FAIL");
    for (int i = 0; i < 100000; i++) {
        intact = 0;
        if (call_money(fake_money, self_value, args, &intact) != want || !intact) {
            g_failures++;
            printf("  T3 repeated call %d damaged the result, registers, stack or xmm3\n", i);
            break;
        }
    }

    /* T4: the text detour - edits the result of a wanted key after the original returned, leaves other keys alone */
    msvc_string rate_key = inline_name("rateKey"), other_key = inline_name("otherKey"), result;
    int text_self = 0;
    memset(&result, 0, sizeof result);
    result.capacity = 15;
    re_text_want = test_want;
    re_text_edit = test_edit;
    re_text_assign = fake_assign;
    installed = re_detour((BYTE *)fake_translate, RE_CHANGE_MONEY_PROLOGUE, sizeof RE_CHANGE_MONEY_PROLOGUE, re_text_hook,
                          &re_text_trampoline);
    printf("  T4 detour installed=%d\n", installed);
    g_failures += !installed;
    const char *want_wanted = control ? "RATE" : "RATE [A]";
    g_frame_arg = control ? 12 : 16;
    g_frame_out = &result;
    unsigned sane = call_translate(fake_translate, &text_self, &result, &rate_key);
    unsigned from = g_want_caller - (unsigned)(UINT_PTR)call_translate; /* the caller the detour reports is this stand-in */
    ok = sane == 1 && strcmp(result.buf, want_wanted) == 0 && result.size == strlen(want_wanted) && from < 128;
    g_failures += !ok;
    printf("  T4 wanted key                                  text='%s' want='%s' registers_stack_and_result_intact=%u "
           "caller_offset=%u %s\n",
           result.buf, want_wanted, sane, from, ok ? "ok" : "FAIL");
    ok = g_want_seen && g_edit_seen;
    g_failures += !ok;
    printf("  T4 the caller's registers and frame            seen_when_asked=%d seen_when_edited=%d %s\n", g_want_seen, g_edit_seen,
           ok ? "ok" : "FAIL");
    sane = call_translate(fake_translate, &text_self, &result, &other_key);
    ok = sane == 1 && strcmp(result.buf, "RATE") == 0;
    g_failures += !ok;
    printf("  T4 other key                                   text='%s' want='RATE' registers_stack_and_result_intact=%u %s\n", result.buf,
           sane, ok ? "ok" : "FAIL");
    for (int i = 0; i < 100000; i++) {
        if (call_translate(fake_translate, &text_self, &result, i % 2 ? &rate_key : &other_key) != 1 ||
            strcmp(result.buf, i % 2 ? "RATE [A]" : "RATE") != 0) {
            g_failures++;
            printf("  T4 repeated call %d went wrong\n", i);
            break;
        }
    }

    /* T5: the redirected call that stores a new education loan - the callback sees the record the callee then gets */
    unsigned record[24] = {1, 0, 0, 71126}, loan_self[1] = {1000};
    intact = 0;
    got = call_add_debt(loan_self, record, &intact);
    want = 1000 + 0 + 71126;
    ok = got == want && intact == 1;
    g_failures += !ok;
    printf("  T5 before the redirect                         got=%u want=%u intact=%u %s\n", got, want, intact, ok ? "ok" : "FAIL");
    re_loan_add_debt = fake_add_debt;
    re_loan_created = test_loan_created;
    g_loan_value = control ? 4999 : 5000;
    installed = re_patch_call((BYTE *)add_debt_site, (BYTE *)fake_add_debt, re_loan_call_hook);
    printf("  T5 call redirected=%d\n", installed);
    g_failures += !installed;
    intact = 0;
    got = call_add_debt(loan_self, record, &intact);
    want = 1000 + 5000 + 71126;
    /* the caller's own copy is not what gets changed; the callback is told the object and the place the call returns to */
    ok = got == want && intact == 1 && record[1] == 0 && g_loan_manager == (void *)loan_self &&
         g_loan_from == (unsigned)(UINT_PTR)add_debt_site + 5;
    g_failures += !ok;
    printf("  T5 with the redirect                           got=%u want=%u intact=%u manager_seen=%d return_address_seen=%d %s\n", got,
           want, intact, g_loan_manager == (void *)loan_self, g_loan_from == (unsigned)(UINT_PTR)add_debt_site + 5, ok ? "ok" : "FAIL");
    record[1] = 7; /* a loan that already has a rate is left alone by this callback */
    got = call_add_debt(loan_self, record, &intact);
    want = 1000 + 7 + 71126;
    ok = got == want && intact == 1;
    g_failures += !ok;
    printf("  T5 record with a rate already                  got=%u want=%u intact=%u %s\n", got, want, intact, ok ? "ok" : "FAIL");

    /* T6: the operands of the fixed-rate formula point at other numbers afterwards */
    static double factor = 1.0, premium = 0.01;
    static const BYTE mul_form[4] = {0xf2, 0x0f, 0x59, 0x05}, add_form[4] = {0xf2, 0x0f, 0x58, 0x05};
    float fixed = fake_fixed(0.069f);
    ok = close_to(fixed, 0.069 * 1.1 + 0.03) && re_bytes_equal((BYTE *)fake_fixed_mul, mul_form, 4) &&
         re_bytes_equal((BYTE *)fake_fixed_add, add_form, 4);
    g_failures += !ok;
    printf("  T6 before the patch                            fixed=%.6f want=%.6f same_instructions_as_the_game %s\n", fixed,
           0.069 * 1.1 + 0.03, ok ? "ok" : "FAIL");
    int refused = !re_patch_ptr((BYTE *)fake_fixed_mul + 4, &fake_c003, &factor); /* wrong expected address */
    installed = re_patch_ptr((BYTE *)fake_fixed_mul + 4, &fake_c11, &factor) &&
                re_patch_ptr((BYTE *)fake_fixed_add + 4, &fake_c003, &premium);
    fixed = fake_fixed(0.069f);
    double want_fixed = control ? 0.069 * 1.1 + 0.03 : 0.069 + 0.01;
    ok = refused && installed && close_to(fixed, want_fixed);
    g_failures += !ok;
    printf("  T6 with the patch                              fixed=%.6f want=%.6f wrong_address_refused=%d patched=%d %s\n", fixed,
           want_fixed, refused, installed, ok ? "ok" : "FAIL");

    /* T7: the wording table - every entry is the game's own text in its language, and a finished text is rewritten
     * with its values kept */
    static char xml[2000000];
    size_t xml_len = 0;
    int in_file = 0, files = 0, file_lang = -1;
    for (int i = 0; i < re_wording_count(); i++) {
        if (re_wording_lang(i) != file_lang) {
            /* the control looks in the English file, where none of the corrected texts can be */
            file_lang = re_wording_lang(i);
            xml_len = language_file(path, control ? "en" : re_lang_folder(file_lang), xml, sizeof xml);
            files += xml_len > 0;
        }
        char element[600];
        snprintf(element, sizeof element, "<%s>%s</%s>", re_wording_key(i), re_wording_game_text(i), re_wording_key(i));
        int found = xml_len > 0 && strstr(xml, element) != NULL;
        in_file += found;
        if (!found && !control)
            printf("  T7 entry %s (%s): the game's language file does not have this text\n", re_wording_key(i),
                   re_lang_folder(re_wording_lang(i)));
    }
    ok = in_file == re_wording_count() && in_file > 0;
    g_failures += !ok;
    printf("  T7 table against the game's language files (%d read, the last of %u bytes)  entries=%d found=%d %s\n", files,
           (unsigned)xml_len, re_wording_count(), in_file, ok ? "ok" : "FAIL");

    static const struct {
        int lang;
        const char *key, *game, *better;
        unsigned from;
    } samples[] = {
        {RE_LANG_KO, "interestRateFixed", "수정됨", "고정", 0x0061bc4eu},
        {RE_LANG_KO, "interestRateVariable", "변수", "변동", 0x0061b5ddu},
        {RE_LANG_KO, "cannotAffordDebtAdded", "임금을 구매할 수 없습니다. 부채가 $13,720.67에 추가되었습니다.",
         "돈이 모자라 임금 미납. 부채 $13,720.67 추가", 0x00542b00u},
        {RE_LANG_KO, "cannotAffordPenalty", "감당할 수 없는 개인 대출! 총 $20,968.21에 20% 페널티가 부과된 부채가 추가되었습니다.",
         "돈이 모자라 개인 대출 미납! 20% 페널티를 더한 부채 $20,968.21 추가", 0x004ce400u},
        {RE_LANG_KO, "interestRateFixed", "Fixed", "Fixed", 0x0061bc4eu}, /* not the game's Korean wording: left alone */
        {RE_LANG_KO, "depositTitle", "예금", "계약금", 0x00631400u},      /* the mortgage window: the down payment */
        {RE_LANG_KO, "depositTitle", "예금", "예금", 0x00636b00u},        /* a savings window: a deposit is a deposit */
        {RE_LANG_EN, "interestRateFixed", "수정됨", "수정됨", 0x0061bc4eu}, /* the Korean entries are for Korean only */
        {RE_LANG_ES, "interestRateFixed", "Corregido", "Fijo", 0x0061bc4eu},
        {RE_LANG_DE, "interestRateFixed", "Corregido", "Corregido", 0x0061bc4eu}, /* and the Spanish one for Spanish */
        {RE_LANG_FR, "industryBust", "L'industrie Tech est en plein essor !", "L'industrie Tech est en crise !", 0x00542b00u},
        {RE_LANG_FR, "industryBoom", "L'industrie Tech est en plein essor !", "L'industrie Tech est en plein essor !", 0x00542b00u},
        /* a theft and a recovery: with the name of the place, with the "??" the game writes for a household without
         * a home, and with no name at all */
        {RE_LANG_KO, "itemStolen", "S-Con:커스텀 부품이(가) 바닥 공간 부족으로 도난당함", "S-Con: 커스텀 부품 도난 (바닥 공간 부족)", 0x005582d0u},
        {RE_LANG_KO, "itemStolen", "??:커스텀 부품이(가) 바닥 공간 부족으로 도난당함", "커스텀 부품 도난 (집이나 사업장 없음)", 0x005582d0u},
        {RE_LANG_KO, "itemRecovered", "S-Con:도구 및 장비이(가) 경찰에 의해 회수됨", "S-Con: 도난당한 도구 및 장비 경찰이 되찾음", 0x0055c100u},
        {RE_LANG_KO, "itemRecovered", ":도구 및 장비이(가) 경찰에 의해 회수됨", "도난당한 도구 및 장비 경찰이 되찾음", 0x0055c100u},
        /* a listed company ends, takes an emergency measure, is listed */
        {RE_LANG_KO, "stockBankruptWorthless", "Holmes 아케이드이 파산했습니다! 귀하의 24746 주식은 이제 무가치합니다.",
         "Holmes 아케이드 파산: 가진 주식 24746주는 가치가 없어졌습니다", 0x00530000u},
        {RE_LANG_KO, "stockCapitalRaiseDesperate",
         "Gold 별미이 파산을 피하기 위해 1,200,000 주식을 발행했습니다! 귀하의 지분율이 37.5% 희석되었습니다.",
         "Gold 별미: 파산을 피하려고 새 주식 1,200,000주 발행. 내 지분율이 37.5% 줄었습니다", 0x00530000u},
        {RE_LANG_KO, "stockDownsizeDesperate", "Gold 별미이 파산을 피하기 위해 사업 규모를 축소합니다! 주당 지분이 20.0% 감소했습니다.",
         "Gold 별미: 파산을 피하려고 사업 규모 축소. 주당 자본이 20.0% 줄었습니다", 0x00530000u},
        {RE_LANG_KO, "stockListing", "Panesar 랜드이 주식 시장에 상장했습니다!", "Panesar 랜드: 주식 시장에 새로 상장", 0x00530000u},
        {RE_LANG_EN, "itemStolen", "S-Con:Custom Part stolen due to lack of floorspace", "S-Con:Custom Part stolen due to lack of floorspace",
         0x005582d0u},
        {RE_LANG_EN, "itemStolen", "??:Custom Part stolen due to lack of floorspace", "Custom Part stolen: no home or premises to keep it in",
         0x005582d0u},
        {RE_LANG_EN, "itemRecovered", ":Tools And Equipment recovered by police", "Tools And Equipment recovered by police", 0x0055c100u},
    };
    int samples_ok = 0, sample_count = (int)(sizeof samples / sizeof samples[0]);
    for (int i = 0; i < sample_count; i++) {
        char buffer[512];
        unsigned len = (unsigned)strlen(samples[i].game);
        memcpy(buffer, samples[i].game, len + 1);
        int entry = re_wording_want(samples[i].lang, samples[i].key, (unsigned)strlen(samples[i].key), samples[i].from);
        if (entry)
            len = re_wording_apply(entry - 1, buffer, len, sizeof buffer, NULL);
        const char *want_text = control && i == 0 ? "수정" : samples[i].better;
        int same = len == strlen(want_text) && memcmp(buffer, want_text, len) == 0;
        samples_ok += same;
        if (!same)
            printf("  T7 sample %d (%s): not the expected text\n", i, samples[i].key);
    }
    ok = samples_ok == sample_count;
    g_failures += !ok;
    printf("  T7 finished texts rewritten                    samples=%d as_expected=%d %s\n", sample_count, samples_ok, ok ? "ok" : "FAIL");

    /* T7: a text whose language file misspells a name, leaves a brace out or has another key's sentence. The game's
     * text has lost the value; it comes from the names and values of the call. `game` is what the game shows. */
    static const struct {
        int lang;
        const char *key, *game, *better;
        int whole;
        const char *pairs[RE_TEXT_PAIRS][2];
    } called[] = {
        {RE_LANG_ES, "timeMonthsTranslate", "MESm", "7m", 1, {{"MONTHS", "7"}}},
        {RE_LANG_ES, "timeMonthsTranslate", "7 meses", "7 meses", 1, {{"MONTHS", "7"}}}, /* not the file's wording: left alone */
        {RE_LANG_ES, "timeMonthsTranslate", "MESm", "MESm", 0, {{"MONTHS", "7"}}},       /* a value did not fit: left alone */
        {RE_LANG_ES, "employeeWageDemand", "(S-Con, Constructor) ¡{Ana Ruiz exige un aumento de sueldo!",
         "(S-Con, Constructor) ¡Ana Ruiz exige un aumento de sueldo!", 1,
         {{"FIRMNAME", "S-Con"}, {"JOBTITLE", "Constructor"}, {"STAFFNAME", "Ana Ruiz"}}},
        {RE_LANG_ES, "educationCompleted", "¡Ana Ruiz ha completado EDUCACIÓN con un 81% de nota!",
         "¡Ana Ruiz ha completado Derecho con un 81% de nota!", 1, {{"PERSONNAME", "Ana Ruiz"}, {"EDUCATION", "Derecho"}, {"PERCENT", "81"}}},
        {RE_LANG_DE, "loanAdded", "Privatkredit für GELD hinzugefügt", "Privatkredit für $5,000.00 hinzugefügt", 1,
         {{"LOANTYPE", "Privatkredit"}, {"MONEY", "$5,000.00"}}},
        {RE_LANG_DE, "rejectedMeetingsDescription", "PERSONNAME: 3 potenzieller Freunde, die aufgrund der Filtereinstellungen abgelehnt wurden",
         "Mina Park: 3 mögliche Freunde wegen der Filtereinstellungen abgelehnt", 1, {{"PERSONAME", "Mina Park"}, {"NUMBER", "3"}}},
        {RE_LANG_DE, "itemStolen", "S-Con: Custom Part wegen Platzmangels gestohlen", "S-Con: Custom Part wegen Platzmangels gestohlen", 1,
         {{"LOCATIONNAME", "S-Con"}, {"ITEMNAME", "Custom Part"}}},
        {RE_LANG_DE, "itemStolen", "??: Custom Part wegen Platzmangels gestohlen",
         "Custom Part gestohlen: weder Zuhause noch Geschäftsräume zum Aufbewahren", 1, {{"LOCATIONNAME", "??"}, {"ITEMNAME", "Custom Part"}}},
        {RE_LANG_ES, "itemStolen", "??:Custom Part robado por falta de suelo", "Custom Part robado: no hay casa ni local donde guardarlo", 1,
         {{"LOCATIONNAME", "??"}, {"ITEMNAME", "Custom Part"}}},
        {RE_LANG_KO, "kinEditRelationshipsTitle", "Mina Park (연령월) 관계", "Mina Park (31y 2m) 관계", 1,
         {{"PERSONNAME", "Mina Park"}, {"AGEYEARSMONTHS", "31y 2m"}}},
        {RE_LANG_KO, "futuresDeclinedBankruptcy", "최근 파산으로 인해 선물을 거래할 수 없습니다. 파산은 데이터스트링에 만료됩니다.",
         "최근 파산 때문에 선물을 거래할 수 없습니다. 파산 기록은 2037년 3월에 없어집니다.", 1, {{"DATESTRING", "2037년 3월"}}},
        {RE_LANG_JP, "happyBirthdayPerson", "健康ポイントでこの変更を有効にする。", "お誕生日おめでとうございます、Aki！", 1, {{"PERSONNAME", "Aki"}}},
        {RE_LANG_JP, "preferenceChangeNote", "お誕生日おめでとうございます、Aki！", "Akiは現在、音楽に対して好きと感じている", 1,
         {{"PERSONNAME", "Aki"}, {"FEELING", "好き"}, {"PREFERENCE", "音楽"}}},
        {RE_LANG_JP, "birthdayLowPointsTip", "思春期の気分の変動を引き起こした：", "無効 - 健康ポイントまたは幸福ポイントが十分でない。", 1, {{NULL, NULL}}},
        {RE_LANG_JP, "passionMaximum", "キャラクターあたりの最大情熱数", "情熱はキャラクター1人につき最大3個", 1, {{"NUMBER", "3"}}},
        {RE_LANG_FR, "tenantConfirmLease",
         "Confirmer le bail du locataire ?\r\n12 Rue A\r\nEstimation du marché : $900\r\nLoyer proposé : $950\r\nDurée du bail : RENTMONTHES mois",
         "Confirmer le bail du locataire ?\n12 Rue A\nEstimation du marché : $900\nLoyer proposé : $950\nDurée du bail : 12 mois", 1,
         {{"HOUSEADDRESS", "12 Rue A"}, {"MARKETRENT", "$900"}, {"RENTPRICE", "$950"}, {"RENTMONTHS", "12"}}},
        {RE_LANG_FR, "tenantConfirmLease",
         "Confirmer le bail du locataire ?\n12 Rue A\nEstimation du marché : $900\nLoyer proposé : $950\nDurée du bail : RENTMONTHES mois",
         "Confirmer le bail du locataire ?\n12 Rue A\nEstimation du marché : $900\nLoyer proposé : $950\nDurée du bail : 12 mois", 1,
         {{"HOUSEADDRESS", "12 Rue A"}, {"MARKETRENT", "$900"}, {"RENTPRICE", "$950"}, {"RENTMONTHS", "12"}}},
        {RE_LANG_PT_BR, "tenantLeaseExpired", "{O contrato de aluguel do locatário expira este mês",
         "12 Rua A: o contrato de aluguel do locatário expira este mês", 1, {{"HOUSEADDRESS", "12 Rua A"}}},
        {RE_LANG_ZH_CN, "isBorn", "名字 出生！", "Li Wei 出生了！", 1, {{"NAME", "Li Wei"}}},
        {RE_LANG_ZH_CN, "itemRecovered", "：Custom Part被警方追回", "Custom Part被警方追回", 1, {{"LOCATIONNAME", ""}, {"ITEMNAME", "Custom Part"}}},
    };
    int called_ok = 0, called_count = (int)(sizeof called / sizeof called[0]);
    for (int i = 0; i < called_count; i++) {
        char buffer[512];
        re_text_values values = {0, called[i].whole, {{0}}, {{0}}};
        for (int p = 0; p < RE_TEXT_PAIRS && called[i].pairs[p][0] != NULL; p++, values.count++) {
            snprintf(values.name[p], sizeof values.name[p], "%s", called[i].pairs[p][0]);
            snprintf(values.value[p], sizeof values.value[p], "%s", called[i].pairs[p][1]);
        }
        unsigned len = (unsigned)strlen(called[i].game);
        memcpy(buffer, called[i].game, len + 1);
        int entry = re_wording_want(called[i].lang, called[i].key, (unsigned)strlen(called[i].key), 0);
        if (entry)
            len = re_wording_apply(entry - 1, buffer, len, sizeof buffer, &values);
        const char *want_text = control && i == 0 ? "MESm" : called[i].better; /* the control: as if the call's values went unused */
        int same = len == strlen(want_text) && memcmp(buffer, want_text, len) == 0;
        called_ok += same;
        if (!same)
            printf("  T7 call sample %d (%s): not the expected text: '%.*s'\n", i, called[i].key, (int)len, buffer);
    }
    ok = called_ok == called_count;
    g_failures += !ok;
    printf("  T7 texts rewritten with the call's values      samples=%d as_expected=%d %s\n", called_count, called_ok, ok ? "ok" : "FAIL");

    /* T8: the trade hooks - an allowed trade runs as it did without them, a refused one does not run at all */
    static const unsigned buy_args[2] = {5, 7}, sell_args[1] = {4}, futures_args[4] = {3, 0, 0, 9}, nothing[1] = {0};
    static unsigned market[1] = {1000};
    const struct {
        const char *name;
        void *fn;
        const unsigned *args;
        int count;
        unsigned sum;
    } trades[3] = {{"buy", fake_buy, buy_args, 2, 1012}, {"sell", fake_sell_handler, sell_args, 1, 1008}, {"futures", fake_futures, futures_args, 4, 1012}};
    static const char *const phases[3] = {"before the hooks", "hooks allow", "hooks refuse"};
    for (int phase = 0; phase < 3; phase++) {
        if (phase == 1) {
            re_guard_blocked = test_blocked;
            re_guard_sell_step = fake_sell_step;
            re_guard_sell_skip = fake_sell_skip;
            installed = re_detour((BYTE *)fake_buy, RE_STOCK_BUY_PROLOGUE, sizeof RE_STOCK_BUY_PROLOGUE, re_guard_buy_hook,
                                  &re_guard_buy_trampoline) &&
                        re_patch_call((BYTE *)fake_sell_site, (BYTE *)fake_sell_step, re_guard_sell_hook) &&
                        re_detour5((BYTE *)fake_futures, RE_GETTER_PROLOGUE, re_guard_futures_hook, &re_guard_futures_trampoline);
            printf("  T8 hooks installed=%d\n", installed);
            g_failures += !installed;
        }
        g_block = phase == 2 && !control;
        unsigned runs_want = phase < 2;
        for (int i = 0; i < 3; i++) {
            unsigned runs = g_trade_runs, steps = g_step_runs;
            int asked = g_blocked_calls;
            g_trade_sum = 0;
            intact = 0;
            call_trade(trades[i].fn, market, trades[i].args, trades[i].count, &intact);
            ok = intact == 1 && g_trade_runs - runs == runs_want && g_trade_sum == (runs_want ? trades[i].sum : 0) &&
                 g_blocked_calls - asked == (phase > 0) && (phase == 0 || g_blocked_trade == i) &&
                 (i != RE_TRADE_SELL || g_step_runs - steps == runs_want);
            g_failures += !ok;
            printf("  T8 %-8s %-18s                    ran=%u/%u sum=%u asked=%d intact=%u %s\n", trades[i].name, phases[phase],
                   g_trade_runs - runs, runs_want, g_trade_sum, g_blocked_calls - asked, intact, ok ? "ok" : "FAIL");
        }
    }
    unsigned runs = g_trade_runs;
    int asked = g_blocked_calls;
    intact = 0;
    call_trade(fake_sell_handler, market, nothing, 1, &intact); /* a click with nothing to sell never reaches the hook */
    ok = intact == 1 && g_trade_runs == runs && g_blocked_calls == asked;
    g_failures += !ok;
    printf("  T8 sell     nothing to sell                       ran=%u asked=%d intact=%u %s\n", g_trade_runs - runs,
           g_blocked_calls - asked, intact, ok ? "ok" : "FAIL");
    for (int i = 0; i < 100000; i++) {
        g_block = i & 1;
        runs = g_trade_runs;
        intact = 0;
        call_trade(trades[i % 3].fn, market, trades[i % 3].args, trades[i % 3].count, &intact);
        if (!intact || g_trade_runs - runs != (unsigned)!g_block) {
            g_failures++;
            printf("  T8 repeated call %d went wrong\n", i);
            break;
        }
    }
    /* What a month end draws: the callback is told before the economy's month and after the listed companies', both
     * get their ecx, and what the second returns reaches the routine. The control: as if the callback were never set. */
    for (int phase = 0; phase < 2; phase++) {
        if (phase == 1) {
            re_guard_economy_month = fake_economy;
            re_guard_stocks_month = fake_stocks;
            re_guard_property_month = fake_property;
            re_guard_site_data = re_guard_own_values = fake_site_data;
            re_guard_month_draws = control ? NULL : test_month_draws;
            re_guard_site_draws = control ? NULL : test_site_draws;
            installed = re_patch_call((BYTE *)fake_economy_site, (BYTE *)fake_economy, re_guard_economy_hook) &&
                        re_patch_call((BYTE *)fake_stocks_site, (BYTE *)fake_stocks, re_guard_stocks_hook) &&
                        re_patch_call((BYTE *)fake_property_site, (BYTE *)fake_property, re_guard_property_hook) &&
                        re_patch_call((BYTE *)fake_sale_site, (BYTE *)fake_site_data, re_guard_sale_site_hook) &&
                        re_patch_call((BYTE *)fake_rent_site, (BYTE *)fake_site_data, re_guard_rent_site_hook) &&
                        re_patch_call((BYTE *)fake_offers_site, (BYTE *)fake_site_data, re_guard_offers_hook);
            printf("  T8 month hooks installed=%d\n", installed);
            g_failures += !installed;
        }
        unsigned trail_want = phase ? 3124 : 12;
        g_month_trail = g_trade_sum = 0;
        intact = 0;
        call_trade(fake_month, market, nothing, 0, &intact);
        ok = intact == 1 && g_month_trail == trail_want && g_trade_sum == 2007;
        g_failures += !ok;
        printf("  T8 month    %-18s                    order=%u/%u sum=%u/2007 intact=%u %s\n", phase ? "hooks in" : phases[0],
               g_month_trail, trail_want, g_trade_sum, intact, ok ? "ok" : "FAIL");
        /* the property market's month start: told before and after the one call */
        trail_want = phase ? 576 : 7;
        g_month_trail = g_trade_sum = 0;
        intact = 0;
        call_trade(fake_property_site, market, nothing, 0, &intact);
        ok = intact == 1 && g_month_trail == trail_want && g_trade_sum == 1009;
        g_failures += !ok;
        printf("  T8 property %-18s                    order=%u/%u sum=%u/1009 intact=%u %s\n", phase ? "hooks in" : phases[0],
               g_month_trail, trail_want, g_trade_sum, intact, ok ? "ok" : "FAIL");
        /* inside it: a site of the walk for sale (node in esi, which call_trade sets to 0x22222222), one of the walk
         * for rent (edi, 0x33333333), and the call before the offers */
        static const struct {
            const char *name;
            void *site;
            unsigned trail, node;
            int rent;
        } inside[3] = {{"site for sale", fake_sale_site, 98, 0x22222222u, 0},
                       {"site for rent", fake_rent_site, 98, 0x33333333u, 1},
                       {"before the offers", fake_offers_site, 78, 0, 0}};
        for (int i = 0; i < 3; i++) {
            g_month_trail = g_trade_sum = 0;
            g_site_rent = -1;
            g_site_node = NULL;
            intact = 0;
            call_trade(inside[i].site, market, nothing, 0, &intact);
            int told = !phase || i == 2 || (g_site_rent == inside[i].rent && g_site_node == (const unsigned char *)(UINT_PTR)inside[i].node);
            trail_want = phase ? inside[i].trail : 8;
            ok = intact == 1 && g_month_trail == trail_want && g_trade_sum == 1000 && told;
            g_failures += !ok;
            printf("  T8 %-17s %-18s               order=%u/%u sum=%u/1000 node_and_walk_as_told=%d intact=%u %s\n", inside[i].name,
                   phase ? "hooks in" : phases[0], g_month_trail, trail_want, g_trade_sum, told, intact, ok ? "ok" : "FAIL");
        }
    }

    /* T9: the load guard's arithmetic. A month has 728 hours; the routine that ends month n runs at hour 728 n - 1,
     * and a save carrying that hour was made before it. */
    const long long cap = 2 * 728;
    re_guard_record rec = {-1, -1, 0, -1, -1}, flat = {3639, -1, 0, -1, -1}, uncapped = {3639, -1, 0, -1, -1};
    g_control = control;
    guard_load("first load of a playthrough", &rec, 1000, cap, 1, 0, 0, 0, 0);
    re_guard_on_month_end(&rec, 727);
    re_guard_on_month_end(&rec, 1455);
    guard_load("newest save, after the month end", &rec, 1456, cap, 1, 0, 0, 0, 0);
    guard_load("save from the hour before that month end", &rec, 1455, cap, 1, 1456, 0, 0, 0);
    guard_load("save from earlier in that month", &rec, 1000, cap, 1, 1456, 0, 0, 0);
    /* a save from the very hour of the last month end (the game's autosave) is told apart from an earlier one */
    re_guard_decision same_hour, earlier, newest, under_lock, unknown, before_rollback, next_month;
    re_guard_record copy = {1455, -1, 0, -1, -1};
    re_guard_on_load(&copy, 1455, cap, 1, -1, &same_hour);
    re_guard_on_load(&copy, 1456, cap, 1, -1, &newest);
    re_guard_on_load(&copy, 1000, cap, 1, -1, &earlier); /* a rollback: the lock covers the hours 1000 to 1455 */
    ok = same_hour.month_end_pending == !control && !earlier.month_end_pending && !newest.month_end_pending;
    g_failures += !ok;
    printf("  T9 month end still to run in the save: same hour=%d, earlier=%d, newest=%d %s\n", same_hour.month_end_pending,
           earlier.month_end_pending, newest.month_end_pending, ok ? "ok" : "FAIL");
    /* The same hour again after that rollback. A save written under the lock carries the lock's line, and its standing
     * orders are not let through. The autosave the game wrote at that hour before the rollback has no such line and
     * is let through as before. A lock line in a save from outside the lock's hours changes nothing. */
    re_guard_on_load(&copy, 1455, cap, 1, 1, &under_lock);
    re_guard_on_load(&copy, 1455, cap, 1, -1, &unknown);
    re_guard_on_load(&copy, 1455, cap, 1, 0, &before_rollback);
    re_guard_on_month_end(&copy, 1455);
    re_guard_on_month_end(&copy, 2183);
    re_guard_on_load(&copy, 2183, cap, 1, 1, &next_month);
    ok = under_lock.month_end_pending == control && !unknown.month_end_pending && before_rollback.month_end_pending == !control &&
         under_lock.unlock_tick == 1456 && copy.lock_from == 1000 && copy.lock_until == 1456 && next_month.month_end_pending;
    g_failures += !ok;
    printf("  T9 ... after a rollback (lock %lld to %lld): save with the lock's line=%d, line not readable=%d, save without the "
           "line=%d, the next month's autosave=%d %s\n",
           copy.lock_from, copy.lock_until, under_lock.month_end_pending, unknown.month_end_pending, before_rollback.month_end_pending,
           next_month.month_end_pending, ok ? "ok" : "FAIL");
    re_guard_on_month_end(&rec, 1000); /* an earlier month end, passed again, moves nothing */
    re_guard_on_month_end(&rec, 2183);
    re_guard_on_month_end(&rec, 2911);
    re_guard_on_month_end(&rec, 3639);
    guard_load("three months back: cut to the cap, stream moved", &rec, 1000, cap, 1, 2456, 1, 1, 1);
    guard_load("the same save again: same lock, same move", &rec, 1000, cap, 1, 2456, 1, 0, 1);
    re_guard_on_month_end(&rec, 2911);
    guard_load("save from the stretch that was seen", &rec, 3000, cap, 1, 0, 1, 0, 1);
    guard_load("save from beyond what was seen", &rec, 4000, cap, 1, 0, 0, 0, 1);
    re_guard_on_month_end(&rec, 4367);
    guard_load("beyond the cap a second time: next generation", &rec, 1000, cap, 1, 2456, 2, 1, 2);
    ok = rec.farthest == 2455 && rec.shift_until == 4367;
    g_failures += !ok;
    printf("  T9 record after that                                  farthest=%lld/2455 shift_until=%lld/4367 %s\n", rec.farthest,
           rec.shift_until, ok ? "ok" : "FAIL");
    guard_load("beyond the cap with the stream move switched off", &flat, 1000, cap, 0, 2456, 0, 1, 0);
    guard_load("no cap: locked up to the farthest month end", &uncapped, 1000, 0, 1, 3640, 0, 0, 0);

    /* T10: the listing. First the fraction: an instruction reading object + 0x298 is re-pointed at a float here. */
    static BYTE keeper[0x2a0], listed[0x140];
    static float keep = 0.45f;
    static const BYTE from_getter[5] = {0x81, 0x98, 0x02, 0x00, 0x00}, not_there[5] = {0x86, 0x98, 0x02, 0x00, 0x00};
    BYTE absolute[5] = {0x05};
    const float saved_fraction = 0.25f, *keep_at = &keep;
    float fraction;
    memcpy(keeper + 0x298, &saved_fraction, 4);
    memcpy(absolute + 1, &keep_at, 4);
    unsigned fraction_bits = call_keep(fake_keep, keeper);
    memcpy(&fraction, &fraction_bits, 4);
    refused = !re_patch_bytes((BYTE *)fake_keep + 3, not_there, absolute, 5); /* wrong expected bytes */
    installed = re_patch_bytes((BYTE *)fake_keep + 3, from_getter, absolute, 5);
    float want_fraction = control ? 0.25f : 0.45f, patched_fraction;
    fraction_bits = call_keep(fake_keep, keeper);
    memcpy(&patched_fraction, &fraction_bits, 4);
    ok = fraction == 0.25f && refused && installed && patched_fraction == want_fraction;
    g_failures += !ok;
    printf("  T10 fraction read                               before=%g after=%g want=%g wrong_bytes_refused=%d patched=%d %s\n",
           fraction, patched_fraction, want_fraction, refused, installed, ok ? "ok" : "FAIL");

    /* then the two calls: 100,000 shares, the founder keeps 45,000, offer price 5,000, the stand-in halves the price */
    static unsigned finance[1] = {1000};
    static const unsigned earning[8] = {100000, 0, 45000, 0, 5000, 0, 77, 0};
    static const unsigned losing[8] = {100000, 0, 45000, 0, 5000, 0, 0xffffffffu, 0xffffffffu};
    const long long all_shares = 100000;
    long long listing_price = 0;
    memcpy(listed + RE_COMPANY_SHARES, &all_shares, 8);
    intact = 0;
    fake_listing(finance, &listing_price, earning, listed, &intact);
    ok = intact == 1 && listing_price == 2500 && finance[0] == 800 && g_steps_runs == 1 && g_fee_runs == 1 && g_fee_tag == 0x80f;
    g_failures += !ok;
    printf("  T10 listing before the redirects                price=%lld fee_charged=%u intact=%u %s\n", listing_price,
           1000 - finance[0], intact, ok ? "ok" : "FAIL");
    re_ipo_steps = fake_steps;
    re_ipo_change_money = fake_fee;
    re_ipo_steps_done = test_steps_done;
    re_ipo_before_fee = test_before_fee;
    installed = re_patch_call((BYTE *)fake_steps_site, (BYTE *)fake_steps, re_ipo_steps_hook) &&
                re_patch_call((BYTE *)fake_fee_site, (BYTE *)fake_fee, re_ipo_fee_hook);
    printf("  T10 calls redirected=%d\n", installed);
    g_failures += !installed;
    g_listing_floor = !control;
    re_ipo_steps_draws = control ? NULL : test_steps_draws; /* the control: as if nothing were told about the steps' draws */
    intact = 0;
    fake_listing(finance, &listing_price, earning, listed, &intact);
    ok = intact == 1 && listing_price == 5000 && finance[0] == 600 && g_steps_runs == 2 && g_fee_runs == 2 &&
         g_listing_seen[0] == 100000 && g_listing_seen[1] == 45000 && g_listing_seen[2] == 5000 && g_listing_seen[3] == 77 &&
         g_listing_company == (const void *)listed && g_listing_finance == (void *)finance && g_listing_cash == 55000LL * 5000;
    g_failures += !ok;
    printf("  T10 earning business: price held at the offer   price=%lld/5000 cash=%lld/%lld frame_seen=%d company_seen=%d intact=%u %s\n",
           listing_price, g_listing_cash, 55000LL * 5000, g_listing_seen[0] == 100000 && g_listing_seen[1] == 45000,
           g_listing_company == (const void *)listed, intact, ok ? "ok" : "FAIL");
    ok = g_draws_seen[0] == 1 && g_draws_seen[1] == 2 && g_draws_told_at_done == 2;
    g_failures += !ok;
    printf("  T10 the steps' draws: told before and after     steps_run_then=%u,%u/1,2 told_before_the_price_callback=%d/2 %s\n",
           g_draws_seen[0], g_draws_seen[1], g_draws_told_at_done, ok ? "ok" : "FAIL");
    intact = 0;
    fake_listing(finance, &listing_price, losing, listed, &intact);
    ok = intact == 1 && listing_price == 2500 && finance[0] == 400 && g_listing_seen[3] == -1 && g_listing_cash == 55000LL * 2500;
    g_failures += !ok;
    printf("  T10 losing business: the market's price         price=%lld/2500 cash=%lld/%lld intact=%u %s\n", listing_price,
           g_listing_cash, 55000LL * 2500, intact, ok ? "ok" : "FAIL");
    ok = re_ipo_cash(100000, 45000, 24768) == 1362240000LL && re_ipo_cash(45000, 100000, 24768) == 0 &&
         re_ipo_cash(100000, 45000, -5) == 0 && re_ipo_floor(90, 100, 1, 1) == 100 && re_ipo_floor(90, 100, 1, 0) == 90 &&
         re_ipo_floor(90, 100, 0, 1) == 90 && re_ipo_floor(120, 100, 1, 1) == 120;
    g_failures += !ok;
    printf("  T10 cash and floor arithmetic                   %s\n", ok ? "ok" : "FAIL");

    /* the quality letter: the limits the re-pointed comparisons read, and the letters they give. The control keeps
     * the game's own limits, under which every multiple above 1.15 is AAA. */
    static const double above[RE_IPO_LETTERS] = {3.0, 2.5, 2.0, 1.5, 1.2, 1.0}, unordered[RE_IPO_LETTERS] = {3.0, 2.5, 2.5, 1.5, 1.2, 1.0};
    static const float multiples[9] = {3.01f, 3.0f, 2.6f, 2.14f, 1.7f, 1.5f, 1.3f, 1.1f, 1.0f};
    static const int want_letter[9] = {0, 1, 1, 2, 3, 4, 4, 5, 6};
    static const re_ipo_grade own = {1.0f, 0.15, 0.1f, 0.05f, -0.05f, -0.1f};
    re_ipo_grade grade = {0}, untouched = {0};
    int letters_wrong = 0;
    ok = re_ipo_grade_limits(above, &grade) && !re_ipo_grade_limits(unordered, &untouched) && untouched.base == 0.0f &&
         grade.base == 1.5f && grade.aaa == 1.5;
    for (int i = 0; i < 9; i++)
        letters_wrong += re_ipo_grade_index(control ? &own : &grade, multiples[i]) != want_letter[i];
    ok = ok && letters_wrong == 0;
    g_failures += !ok;
    printf("  T10 quality letter limits                       base=%g aaa=%g unordered_refused=%d letters_wrong=%d of 9 %s\n",
           grade.base, grade.aaa, untouched.base == 0.0f, letters_wrong, ok ? "ok" : "FAIL");
    for (int i = 0; i < 100000; i++) {
        intact = 0;
        fake_listing(finance, &listing_price, i & 1 ? losing : earning, listed, &intact);
        if (!intact || listing_price != (i & 1 || control ? 2500 : 5000)) {
            g_failures++;
            printf("  T10 repeated call %d went wrong\n", i);
            break;
        }
    }

    /* T11: the board. A company with five nominees - 501, 7, 502, 9, 8 - of which 7, 8 and 9 (and 6, who is not
     * nominated) are the household's; the household holds 45%, good for two seats at one seat per 20%. */
    static BYTE firm[0x190], people[0x380];
    static struct node {
        struct node *next, *previous;
        int person;
    } nodes[6]; /* [0] is the sentinel */
    static const int named[5] = {501, 7, 502, 9, 8};
    static const int family[4] = {6, 7, 8, 9};
    static float held = 0.45f;
    static BYTE score[RE_SCORE_BYTES];
    for (int i = 0; i < 6; i++) {
        nodes[i].next = &nodes[(i + 1) % 6];
        nodes[i].previous = &nodes[(i + 5) % 6];
        nodes[i].person = i ? named[i - 1] : -1;
    }
    const void *list = &nodes[0], *family_begin = family, *family_end = family + 4, *people_at = people, *stock_market = &held;
    const int five = 5, firm_id = 77;
    memcpy(firm + RE_COMPANY_NOMINEES, &list, 4);
    memcpy(firm + RE_COMPANY_NOMINEES + 4, &five, 4);
    memcpy(firm + RE_COMPANY_PEOPLE, &people_at, 4);
    memcpy(firm + RE_COMPANY_MARKET, &stock_market, 4);
    memcpy(firm + RE_COMPANY_ID, &firm_id, 4);
    memcpy(people + RE_PEOPLE_HOUSEHOLD, &family_begin, 4);
    memcpy(people + RE_PEOPLE_HOUSEHOLD + 4, &family_end, 4);
    int read_nominees[8], read_family[8];
    int n_named = re_board_nominees(firm, read_nominees, 8), n_family = re_board_household(firm, read_family, 8);
    ok = n_named == 5 && memcmp(read_nominees, named, sizeof named) == 0 && n_family == 4 && memcmp(read_family, family, sizeof family) == 0 &&
         re_board_nominees(firm, read_nominees, 4) == -1; /* does not fit */
    nodes[3].next = NULL;
    ok = ok && re_board_nominees(firm, read_nominees, 8) == -1; /* a broken list is refused, not followed */
    nodes[3].next = &nodes[4];
    g_failures += !ok;
    printf("  T11 nominee list and household read               nominees=%d household=%d %s\n", n_named, n_family, ok ? "ok" : "FAIL");
    ok = re_board_place(named, 5, family, 4, 7) == 0 && re_board_place(named, 5, family, 4, 9) == 1 &&
         re_board_place(named, 5, family, 4, 8) == 2 && re_board_place(named, 5, family, 4, 501) == -1 &&
         re_board_place(named, 5, family, 4, 6) == -1 && re_board_seats(0.45f, 0.2) == 2 && re_board_seats(0.2f, 0.2) == 1 &&
         re_board_seats(0.4f, 0.2) == 2 && re_board_seats(0.199f, 0.2) == 0 && re_board_seats(2.0f, 0.2) == RE_BOARD_SEATS &&
         re_board_seats(0.45f, 0.0) == 0 && re_board_seats(0.0f, 0.2) == 0 && re_board_total(1000.0f, 0) != re_board_total(1000.0f, 1) &&
         re_board_fraction(fake_ownership, &held, 77) == 0.45f;
    g_failures += !ok;
    printf("  T11 place, seats, totals, the game's fraction     %s\n", ok ? "ok" : "FAIL");
    /* Whose total is forced. Nine candidates: six others with 9, 8, 7, 6, 5, 4 and the household's 7, 9, 8 in the
     * order of the list. With nothing forced the five seats go to the totals 10, 9, 8, 7, 6. */
    static const int field[9] = {601, 7, 602, 603, 9, 604, 605, 8, 606};
    static const float strong_first[9] = {9, 10, 8, 7, 1, 6, 5, 0.5f, 4};  /* 7 is elected anyway */
    static const float nobody[9] = {9, 3, 8, 7, 1, 6, 5, 0.5f, 4};         /* none of the three is */
    static const float tied[9] = {9, 6, 8, 7, 6, 10, 5, 0.5f, 4};          /* 7 and 9 share the fifth place */
    static const float few[9] = {2, 1, 2, 2, 1, 2, 2, 1, 2};               /* two distinct totals: everyone is elected */
#define RANK(totals, seats, person) re_board_rank(field, totals, 9, family, 4, seats, person)
    int ranks_wrong =
        (RANK(strong_first, 1, 7) != 0) + (RANK(strong_first, 1, 9) != 1 + control) + (RANK(strong_first, 1, 8) != -1) + /* 7 uses no seat */
        (RANK(strong_first, 2, 8) != 2) + (RANK(nobody, 1, 7) != 0) + (RANK(nobody, 1, 9) != -1) + (RANK(nobody, 2, 9) != 1) +
        (RANK(nobody, 2, 8) != -1) + (RANK(tied, 1, 7) != 0) + (RANK(tied, 1, 9) != 1) + (RANK(tied, 1, 8) != 2) +
        (RANK(few, 1, 7) != 0) + (RANK(few, 1, 9) != 1) + (RANK(few, 1, 8) != 2) +
        (RANK(strong_first, 3, 8) != 2) + (RANK(NULL, 3, 8) != 2) + /* no more nominees than seats: all of them, by the list */
        (RANK(NULL, 1, 7) != 0) + (RANK(NULL, 1, 9) != -1) +        /* totals not known: the first of the list */
        (RANK(strong_first, 0, 7) != -1) + (RANK(strong_first, 2, 601) != -1) + (RANK(strong_first, 2, 6) != -1);
#undef RANK
    ok = ranks_wrong == 0;
    g_failures += !ok;
    printf("  T11 who is forced: elected anyway first, then the guarantee    ranks_wrong=%d of 21 %s\n", ranks_wrong, ok ? "ok" : "FAIL");
    /* asking for the other totals: one scratch score per other nominee, each destroyed, nothing of the caller's */
    float field_totals[9];
    unsigned runs_before = g_score_runs, frees_before = g_score_frees;
    re_board_score = fake_score;
    int asked_ok = !re_board_totals(firm, field, 9, 9, 77.0f, field_totals); /* refused without the destructor */
    re_board_score_free = fake_score_free;
    asked_ok = asked_ok && re_board_totals(firm, field, 9, 9, 77.0f, field_totals);
    for (int i = 0; i < 9; i++)
        asked_ok = asked_ok && field_totals[i] == (field[i] == 9 ? 77.0f : (float)field[i]);
    ok = asked_ok && g_score_runs - runs_before == 8 && g_score_frees - frees_before == 8u + (unsigned)control;
    g_failures += !ok;
    printf("  T11 the other nominees' totals asked for          scored=%u destroyed=%u %s\n", g_score_runs - runs_before,
           g_score_frees - frees_before, ok ? "ok" : "FAIL");
    re_board_score_free = fake_score_free;
    g_score_runs = g_score_frees = 0;
    intact = 0;
    fake_election(firm, score, 7, &intact);
    float natural;
    memcpy(&natural, score + RE_SCORE_TOTAL, 4);
    ok = intact == 1 && natural == 7.0f && g_score_runs == 1 && *(BYTE **)score == firm;
    re_board_score = fake_score;
    re_board_scored = test_scored;
    installed = re_patch_call((BYTE *)fake_score_site, (BYTE *)fake_score, re_board_score_hook);
    ok = ok && installed;
    g_failures += !ok;
    printf("  T11 election before the redirect                  total=%g intact=%u redirected=%d %s\n", natural, intact, installed,
           ok ? "ok" : "FAIL");
    g_board_per_seat = control ? 0.5 : 0.2; /* the control's rule gives 45% no seat */
    static const int voters[5] = {7, 9, 8, 501, 6};
    /* five nominees for five seats: the household's three are elected anyway, so all three keep their seats */
    static const float want_total[5] = {1000.0f, 999.0f, 998.0f, 501.0f, 6.0f};
    int totals_wrong = 0, not_intact = 0;
    for (int i = 0; i < 5; i++) {
        float total;
        intact = 0;
        fake_election(firm, score, voters[i], &intact);
        memcpy(&total, score + RE_SCORE_TOTAL, 4);
        totals_wrong += total != want_total[i];
        not_intact += intact != 1;
    }
    /* six calls of the game's own; every score the callback asked for besides was destroyed */
    ok = totals_wrong == 0 && not_intact == 0 && g_score_runs - g_score_frees == 6 && g_score_frees == 12 && g_board_company == firm &&
         g_board_return == (unsigned)((BYTE *)fake_score_site + 5);
    g_failures += !ok;
    printf("  T11 through the hook, 45%% of the company           totals_wrong=%d of 5 not_intact=%d asked_and_destroyed=%u "
           "company_seen=%d return_seen=%d %s\n",
           totals_wrong, not_intact, g_score_frees, g_board_company == firm, g_board_return == (unsigned)((BYTE *)fake_score_site + 5),
           ok ? "ok" : "FAIL");
    for (int i = 0; i < 100000; i++) {
        intact = 0;
        fake_election(firm, score, voters[i % 5], &intact);
        if (!intact) {
            g_failures++;
            printf("  T11 repeated call %d went wrong\n", i);
            break;
        }
    }

    /* T12: what a company's numbers say about its share */
    BYTE traded[RE_COMPANY_ID + 4] = {0};
    long long fields[4] = {13099, 266, 532, 12004}; /* run 19's company, thirteen months after its listing; the dividend is made up */
    memcpy(traded + RE_COMPANY_PRICE, &fields[0], 8);
    memcpy(traded + RE_COMPANY_DIVIDEND, &fields[1], 8);
    memcpy(traded + RE_COMPANY_EARNINGS, &fields[2], 8);
    memcpy(traded + RE_COMPANY_EQUITY, &fields[3], 8);
    *(int *)(traded + RE_COMPANY_RESEARCH_MONTH) = 150;
    *(int *)(traded + RE_COMPANY_DESPERATE_MONTH) = 197;
    re_stock share;
    re_stock_view view;
    re_stock_read(traded, &share);
    re_stock_view_of(&share, 200, &view);
    /* weight 0.8 - 3 x 532 / 12004 = 0.66704; target 0.66704 x 12004 + 0.33296 x 20 x 532 = 11549.8 */
    ok = share.research_month == 150 && view.has_per && view.has_pbr && same_number(view.per, 13099.0 / 532.0) &&
         same_number(view.pbr, 13099.0 / 12004.0) && same_number(view.roe, 532.0 / 12004.0) && same_number(view.yield, 266.0 / 13099.0) &&
         view.target == 11550 + g_control && same_number(view.gap, (11550.0 - 13099.0) / 13099.0) && !view.below_line &&
         same_number(view.to_line, (13099.0 - 9003.0) / 13099.0) && view.months_since == 3;
    g_failures += !ok;
    printf("  T12 ratios and target of a healthy company        per=%.2f pbr=%.3f roe=%.4f target=%lld/%d gap=%.4f to_line=%.4f %s\n",
           view.per, view.pbr, view.roe, view.target, 11550 + g_control, view.gap, view.to_line, ok ? "ok" : "FAIL");
    char told[600];
    const char *want_told = "PER 24.6, PBR 1.09, 배당 2.0%\n@RED@3개월 안에 주가 -31.3%면 파산@@\n"
                            "적정 $115.50 (-12%), 업종 호조";
    char calm[600], against[600];
    unsigned told_len = re_stock_text(&view, 1, "$115.50", 1, 0.0123, 1, told, sizeof told);
    /* the industries get a word from half a sign (0.25% of the equity a month) either way, none below */
    re_stock_text(&view, 1, "$115.50", 1, 0.0024, 0, calm, sizeof calm);
    re_stock_text(&view, 0, "$115.50", 1, -0.0026, 0, against, sizeof against);
    ok = told_len == strlen(want_told) + (unsigned)g_control && strcmp(told, want_told) == 0 &&
         strcmp(calm, "PER 24.6, PBR 1.09, 배당 2.0%\n3개월 안에 주가 -31.3%면 파산\n적정 $115.50 (-12%)") == 0 &&
         strcmp(against, "PER 24.6, PBR 1.09, yield 2.0%\nWithin 3 mo.: bankrupt if the price falls 31.3%\n"
                         "Fair $115.50 (-12%), industry headwind") == 0 &&
         re_stock_text(&view, 1, "$115.50", 1, 0.0123, 1, told, 40) == 0;
    g_failures += !ok;
    printf("  T12 the hover lines for that company              length=%u/%u too_small_refused=%d %s\n", told_len,
           (unsigned)strlen(want_told) + (unsigned)g_control, told[0] == 0, ok ? "ok" : "FAIL");
    static const char *const want_rows[4] = {" (적정 $115.50)", " (PBR 1.09, 파산 주의)", " (PER 24.6)",
                                             " (주가의 2.0%)"};
    int rows_wrong = 0;
    for (int row = 0; row < 4; row++) {
        int red = -1;
        unsigned n = re_stock_row(&view, row, 1, "$115.50", &red, told, sizeof told);
        rows_wrong += n != strlen(want_rows[row]) || strcmp(told, want_rows[row]) != 0 ||
                      red != ((row == RE_STOCK_ROW_EQUITY) ^ g_control);
    }
    int no_red = -1;
    ok = rows_wrong == 0 && re_stock_row(&view, RE_STOCK_ROW_PRICE, 1, NULL, &no_red, told, sizeof told) == 0 && no_red == 0 &&
         re_stock_row(&view, RE_STOCK_ROW_EARNINGS, 1, NULL, &no_red, told, 8) == 0;
    g_failures += !ok;
    printf("  T12 what goes behind its four amounts             rows_wrong=%d of 4 %s\n", rows_wrong, ok ? "ok" : "FAIL");
    /* with the price of the month before known: a line of its own in the hover text, and behind the price of the
     * tab with the fair price or alone */
    view.has_change = 1;
    view.change = 0.0234;
    char change_ko[600], change_en[600], change_row[200], change_alone[200];
    re_stock_text(&view, 1, "$115.50", 1, 0.0024, 0, change_ko, sizeof change_ko);
    view.change = -0.0021;
    re_stock_text(&view, 0, NULL, 0, 0.0, 0, change_en, sizeof change_en);
    re_stock_row(&view, RE_STOCK_ROW_PRICE, 1, "$115.50", &no_red, change_row, sizeof change_row);
    re_stock_row(&view, RE_STOCK_ROW_PRICE, 1, NULL, &no_red, change_alone, sizeof change_alone);
    ok = strcmp(change_ko, g_control ? "" : "PER 24.6, PBR 1.09, 배당 2.0%\n전월 대비 +2.3%\n3개월 안에 주가 -31.3%면 파산\n적정 $115.50 (-12%)") == 0 &&
         strcmp(change_en, "PER 24.6, PBR 1.09, yield 2.0%\n-0.2% on the month before\nWithin 3 mo.: bankrupt if the price falls 31.3%") == 0 &&
         strcmp(change_row, " (-0.2%, 적정 $115.50)") == 0 && strcmp(change_alone, " (전월 -0.2%)") == 0;
    g_failures += !ok;
    printf("  T12 the price against the month before            %s\n", ok ? "ok" : "FAIL");
    view.has_change = 0;
    {
        /* the order of the list: researched, gap, has_change, change, capitalisation. Rows 0 and 3 are equal by
         * change and neither is researched, rows 1 and 4 have one gap: equal rows keep their order. Row 2 has no
         * price of the month before and goes last by change; the two that are not researched go last by value, as
         * they stood. By capitalisation every row is ranked, the largest first; rows 2 and 5 are equal. */
        static const re_stock_rank rows[] = {{0, 0.90, 1, 0.05, 3e11}, {1, 0.30, 1, -0.02, 9e12}, {1, -0.10, 0, 0.77, 5e10},
                                             {0, -0.90, 1, 0.05, 7e11}, {1, 0.30, 1, 0.09, 1e9}, {1, 0.80, 1, 0.0, 5e10}};
        static const int wanted[4][6] = {{5, 1, 4, 2, 0, 3}, {2, 1, 4, 5, 0, 3}, {4, 0, 3, 5, 1, 2}, {1, 3, 0, 2, 5, 4}};
        int orders_wrong = 0, order[6], one[1] = {7};
        for (int by = RE_STOCK_BY_CHEAP; by <= RE_STOCK_BY_CAP; by++) {
            re_stock_order(rows, 6, by, order);
            int wrong = order[0] != wanted[by][0] + g_control;
            for (int i = 1; i < 6; i++)
                wrong |= order[i] != wanted[by][i];
            if (wrong)
                printf("  T12 order %d: %d %d %d %d %d %d, wanted %d %d %d %d %d %d\n", by, order[0], order[1], order[2], order[3], order[4],
                       order[5], wanted[by][0] + g_control, wanted[by][1], wanted[by][2], wanted[by][3], wanted[by][4], wanted[by][5]);
            orders_wrong += wrong;
        }
        re_stock_order(rows, 1, RE_STOCK_BY_CHEAP, one);
        ok = orders_wrong == 0 && one[0] == 0;
        g_failures += !ok;
        printf("  T12 the list by value, change and capitalisation  orders_wrong=%d of 4 %s\n", orders_wrong, ok ? "ok" : "FAIL");
    }
    {
        /* the months a research fee was paid for ([48]): a month is in the record once paid; a month left out is not;
         * a month paid later for an earlier month (after a rollback) is; 24 months are kept; a month paid in another
         * course of share prices says nothing */
        long long r = 0;
        int paid_wrong = re_stock_paid_has(r, 0, 130);
        r = re_stock_paid_add(r, 0, 130);
        paid_wrong += !re_stock_paid_has(r, 0, 130) + re_stock_paid_has(r, 0, 129) + re_stock_paid_has(r, 0, 131) + re_stock_paid_has(r, 1, 130);
        r = re_stock_paid_add(r, 0, 132);
        paid_wrong += !re_stock_paid_has(r, 0, 130) + re_stock_paid_has(r, 0, 131) + !re_stock_paid_has(r, 0, 132);
        r = re_stock_paid_add(r, 0, 131);
        paid_wrong += !re_stock_paid_has(r, 0, 130) + !re_stock_paid_has(r, 0, 131) + !re_stock_paid_has(r, 0, 132);
        r = re_stock_paid_add(r, 0, 132 + RE_STOCK_PAID_MONTHS - 1);
        paid_wrong += !re_stock_paid_has(r, 0, 132) + re_stock_paid_has(r, 0, 131) + re_stock_paid_has(r, 0, 130) + !re_stock_paid_has(r, 0, 155);
        r = re_stock_paid_add(r, 0, 195);
        paid_wrong += !re_stock_paid_has(r, 0, 195) + re_stock_paid_has(r, 0, 155);
        r = re_stock_paid_add(r, 1, 196);
        paid_wrong += (re_stock_paid_has(r, 1, 196) != 1 - g_control) + re_stock_paid_has(r, 1, 195) + re_stock_paid_has(r, 0, 196);
        ok = paid_wrong == 0;
        g_failures += !ok;
        printf("  T12 the months a research fee was paid for         paid_wrong=%d of 20 %s\n", paid_wrong, ok ? "ok" : "FAIL");
    }
    {
        /* behind a company's name in a row of the list: the change, the fair price of a researched company, who
         * keeps the research fresh; nothing where there is nothing to say */
        re_stock_view item;
        char plain[80], full[80], board[80], none[80] = "x", small[6];
        memset(&item, 0, sizeof item);
        item.has_change = 1;
        item.change = 0.0249;
        item.has_pbr = 1;
        item.gap = 0.318;
        unsigned plain_len = re_stock_item(&item, 0, RE_STOCK_KEPT_NOT, 1, plain, sizeof plain);
        re_stock_item(&item, 1, RE_STOCK_KEPT_SUBSCRIBED, 1, full, sizeof full);
        unsigned small_len = re_stock_item(&item, 1, RE_STOCK_KEPT_SUBSCRIBED, 1, small, sizeof small);
        item.has_change = 0;
        re_stock_item(&item, 0, RE_STOCK_KEPT_BOARD, 0, board, sizeof board);
        unsigned none_len = re_stock_item(&item, 0, RE_STOCK_KEPT_NOT, 1, none, sizeof none);
        ok = strcmp(plain, g_control ? "" : " (+2.5%)") == 0 && plain_len == strlen(" (+2.5%)") &&
             strcmp(full, " (+2.5%, 적정 +32%, 구독)") == 0 && strcmp(board, " (board)") == 0 && none_len == 0 && small_len == 0;
        if (!ok)
            printf("  T12 row: '%s' '%s' '%s' none=%u small=%u\n", plain, full, board, none_len, small_len);
        g_failures += !ok;
        printf("  T12 behind a company's name in the list            %s\n", ok ? "ok" : "FAIL");
    }
    static const struct {
        const char *what;
        re_stock s;
        int now, below, has_per, has_pbr;
        long long target;
        const char *korean, *english, *equity_row, *earnings_row;
        int red;
        const char *held; /* in the month's warning about shares held; empty = not named there */
        int level;
    } cases[] = {
        {"under the line, no measure before", {700, 0, -50, 1000, 0, -99}, 200, 1, 0, 1, 600,
         "PER 적자, PBR 0.70, 배당 0.0%\nPBR 0.75 미만: 월말 축소·증자",
         "PER: loss, PBR 0.70, yield 0.0%\nPBR < 0.75: downsizing or new shares at month end",
         " (PBR 0.70, 월말 축소·증자)", " (자본의 -5.0%)", 1, "가나다 (월말 축소·증자)", RE_STOCK_MEASURE_DUE},
        {"under the line after a measure", {700, 0, -50, 1000, 0, 199}, 200, 1, 0, 1, 600,
         "PER 적자, PBR 0.70, 배당 0.0%\nPBR 0.75 미만: 월말에 파산",
         "PER: loss, PBR 0.70, yield 0.0%\nPBR < 0.75: bankrupt at month end",
         " (PBR 0.70, 월말 파산)", " (자본의 -5.0%)", 1, "가나다 (월말 파산)", RE_STOCK_BANKRUPT_DUE},
        /* weight 0.65, target 0.65 x 1000 + 0.35 x 20 x 50 = 1000 */
        {"close to the line", {810, 40, 50, 1000, 0, -99}, 200, 0, 1, 1, 1000,
         "PER 16.2, PBR 0.81, 배당 4.9%\n주가 -7.4%면 월말 축소·증자",
         "PER 16.2, PBR 0.81, yield 4.9%\nPrice -7.4%: downsizing or new shares",
         " (PBR 0.81)", " (PER 16.2)", 0, "가나다 (주가 -7.4%면 월말 규모 축소·증자)", RE_STOCK_CLOSE},
        {"close to the line after a measure", {810, 40, 50, 1000, 0, 198}, 200, 0, 1, 1, 1000,
         "PER 16.2, PBR 0.81, 배당 4.9%\n4개월 안에 주가 -7.4%면 파산",
         "PER 16.2, PBR 0.81, yield 4.9%\nWithin 4 mo.: bankrupt if the price falls 7.4%",
         " (PBR 0.81, 파산 주의)", " (PER 16.2)", 1, "가나다 (주가 -7.4%면 월말 파산)", RE_STOCK_WATCH},
        /* the hover text names the line from a quarter of the price away, the month's warning only from a tenth */
        {"a sixth of the price above the line", {900, 0, 50, 1000, 0, -99}, 200, 0, 1, 1, 1000,
         "PER 18.0, PBR 0.90, 배당 0.0%\n주가 -16.7%면 월말 축소·증자",
         "PER 18.0, PBR 0.90, yield 0.0%\nPrice -16.7%: downsizing or new shares",
         " (PBR 0.90)", " (PER 18.0)", 0, "", RE_STOCK_CALM},
        {"equity gone", {500, 0, -100, -20, 0, -99}, 200, 1, 0, 0, 0,
         "PER 적자, 배당 0.0%\n월말 축소·증자",
         "PER: loss, yield 0.0%\ndownsizing or new shares at month end",
         " (월말 축소·증자)", "", 1, "가나다 (월말 축소·증자)", RE_STOCK_MEASURE_DUE},
    };
    int cases_wrong = 0;
    for (int i = 0; i < (int)(sizeof cases / sizeof cases[0]); i++) {
        char ko[600], en[600], equity_row[160], earnings_row[160], held[160] = "";
        int red = -1, other = -1, level = -1;
        re_stock_view_of(&cases[i].s, cases[i].now, &view);
        re_stock_text(&view, 1, NULL, 0, 0.0, 0, ko, sizeof ko);
        re_stock_text(&view, 0, NULL, 0, 0.0, 0, en, sizeof en);
        re_stock_row(&view, RE_STOCK_ROW_EQUITY, 1, NULL, &red, equity_row, sizeof equity_row);
        earnings_row[0] = 0;
        re_stock_row(&view, RE_STOCK_ROW_EARNINGS, 1, NULL, &other, earnings_row, sizeof earnings_row);
        unsigned held_len = re_stock_held(&view, 1, "가나다", &level, held, sizeof held);
        int wrong = view.below_line != (cases[i].below ^ g_control) || view.has_per != cases[i].has_per ||
                    view.has_pbr != cases[i].has_pbr || view.target != cases[i].target || strcmp(ko, cases[i].korean) != 0 ||
                    strcmp(en, cases[i].english) != 0 || strcmp(equity_row, cases[i].equity_row) != 0 || red != cases[i].red ||
                    strcmp(earnings_row, cases[i].earnings_row) != 0 || other != 0 || held_len != strlen(cases[i].held) ||
                    strcmp(held, cases[i].held) != 0 || level != cases[i].level;
        if (wrong)
            printf("  T12 case '%s': below=%d target=%lld red=%d level=%d\n    %s\n    %s\n    %s | %s | %s\n", cases[i].what,
                   view.below_line, view.target, red, level, ko, en, equity_row, earnings_row, held);
        cases_wrong += wrong;
    }
    ok = cases_wrong == 0;
    g_failures += !ok;
    printf("  T12 the line, a measure before, no equity         cases_wrong=%d of %d %s\n", cases_wrong,
           (int)(sizeof cases / sizeof cases[0]), ok ? "ok" : "FAIL");
    /* what an industry adds: rate x weight x 0.025, one sign for every started half per cent, at most ten signs;
     * the amount behind them is the caller's (the term times the equity a share, as the game writes amounts) */
    static const struct {
        double rate, weight;
        const char *row;
    } industries[] = {
        {0.03, 8.0, "++ (월 +$1.23)"},
        {-0.13, 10.0, "------- (월 +$1.23)"},
        {0.5, 8.0, "++++++++++ (월 +$1.23)"},
        {0.002, 2.0, "+ (월 +$1.23)"},
        {0.0, 8.0, ""},
    };
    int industries_wrong = 0;
    for (int i = 0; i < (int)(sizeof industries / sizeof industries[0]); i++) {
        char row[48] = "";
        double term = re_stock_industry_term(industries[i].rate, industries[i].weight);
        unsigned n = re_stock_industry_row(term + g_control, 1, "+$1.23", row, sizeof row);
        int wrong = n != strlen(industries[i].row) || strcmp(row, industries[i].row) != 0 ||
                    !same_number(term, industries[i].rate * industries[i].weight * 0.025);
        if (wrong)
            printf("  T12 industry %g x %g: '%s', wanted '%s'\n", industries[i].rate, industries[i].weight, row, industries[i].row);
        industries_wrong += wrong;
    }
    char small_row[8], bare_row[48], english_row[48];
    re_stock_industry_row(-0.006, 1, NULL, bare_row, sizeof bare_row); /* a company without equity: the signs alone */
    re_stock_industry_row(-0.006, 0, "-$2.17", english_row, sizeof english_row);
    ok = industries_wrong == 0 && strcmp(bare_row, "--") == 0 && strcmp(english_row, "-- (-$2.17 a month)") == 0 &&
         re_stock_industry_row(0.006, 1, "+$1.23", small_row, sizeof small_row) == 0;
    g_failures += !ok;
    printf("  T12 what an industry adds to the earnings         rows_wrong=%d of %d %s\n", industries_wrong,
           (int)(sizeof industries / sizeof industries[0]), ok ? "ok" : "FAIL");

    /* T13: the row detour - the callback changes the value and the red flag before the row is built */
    msvc_string row_label = inline_name("Equity"), row_value = inline_name("$243.98");
    intact = 0;
    call_row(fake_row, &text_self, &row_label, &row_value, 0, &intact);
    ok = intact == 1 && strcmp(g_row_value.buf, "$243.98") == 0 && g_row_red == 0;
    g_failures += !ok;
    printf("  T13 row before the detour                         value='%s' red=%u intact=%u %s\n", g_row_value.buf, g_row_red, intact,
           ok ? "ok" : "FAIL");
    re_text_row = test_row;
    installed = re_detour5((BYTE *)fake_row, RE_GETTER_PROLOGUE, re_text_row_hook, &re_text_row_trampoline);
    printf("  T13 detour installed=%d\n", installed);
    g_failures += !installed;
    intact = 0;
    call_row(fake_row, &text_self, &row_label, &row_value, 0, &intact);
    const char *want_row = control ? "$243.98" : "$243.98 [B]";
    unsigned row_from = g_row_caller - (unsigned)(UINT_PTR)call_row;
    ok = intact == 1 && strcmp(g_row_value.buf, want_row) == 0 && g_row_value.size == strlen(want_row) && g_row_red == 1 &&
         strcmp(g_row_label.buf, "Equity") == 0 && g_row_seen == 1 && row_from < 128;
    g_failures += !ok;
    printf("  T13 with the detour                               value='%s' want='%s' red=%u label='%s' registers_seen=%d "
           "caller_offset=%u intact=%u %s\n",
           g_row_value.buf, want_row, g_row_red, g_row_label.buf, g_row_seen, row_from, intact, ok ? "ok" : "FAIL");
    for (int i = 0; i < 100000; i++) {
        intact = 0;
        call_row(fake_row, &text_self, &row_label, &row_value, 0, &intact);
        if (intact != 1 || strcmp(g_row_value.buf, "$243.98 [B]") != 0) {
            g_failures++;
            printf("  T13 repeated call %d went wrong\n", i);
            break;
        }
    }

    /* T14: the plugin's texts in the game's languages. Every language has every text, in one place (the table of
     * re_lang.c or the language's file, not both); a translation takes the conversions of the English text in the
     * same order; the lines of the summary panel begin the way the code looks for them; no other lock line looks
     * like the line of a running lock, in any language; the longest hover text fits its buffer. */
    int own_texts = 0, conversions_wrong = 0, prefixes_wrong = 0, too_long = 0, places_wrong = 0;
    for (int msg = 0; msg < RE_MSG_COUNT; msg++) {
        char english[64], other[64];
        conversions(re_lang_text(RE_LANG_EN, msg), english, sizeof english);
        if (!re_lang_has(RE_LANG_EN, msg) || re_lang_text(RE_LANG_EN, msg)[0] == 0) {
            conversions_wrong++;
            printf("  T14 text %d has no English\n", msg);
        }
        for (int lang = 1; lang < RE_LANG_COUNT; lang++) {
            int places = re_lang_places(lang, msg);
            if (places != (control ? 2 : 1)) { /* the control: asked for two places, every text counts as wrong */
                places_wrong++;
                if (!control)
                    printf("  T14 text %d is in %d places for '%s' (the table, the language's file): 1 is right\n", msg, places,
                           re_lang_folder(lang));
            }
            if (!re_lang_has(lang, msg))
                continue;
            own_texts++;
            conversions(re_lang_text(lang, msg), other, sizeof other);
            if (strcmp(english, other) != 0) {
                conversions_wrong++;
                printf("  T14 text %d in '%s' takes %s, the English %s\n", msg, re_lang_folder(lang), other, english);
            }
        }
    }
    {
        /* the control: a translation that swaps two conversions has to count as wrong, and "none" as a running lock */
        char straight[64], swapped[64];
        conversions("%s of %d, 100%%", straight, sizeof straight);
        conversions(control ? "100%%: %s von %d" : "%d von %s", swapped, sizeof swapped);
        conversions_wrong += strcmp(straight, swapped) == 0;
        prefixes_wrong += begins("Trade lock: none", control ? "Trade lock" : "Trade lock: another ");
    }
    for (int lang = 0; lang < RE_LANG_COUNT; lang++) {
        char runs[200], none[200], over[200], grade[200], skipped[200], loans[200], cash[200], hover[600], row[160];
        const char *lock = re_lang_text(lang, RE_MSG_LOCK_PREFIX), *start = re_lang_text(lang, RE_MSG_LOCK_RUNS);
        const char *forecast = re_lang_text(lang, RE_MSG_FORECAST_PREFIX), *transfer = re_lang_text(lang, RE_MSG_AUTO_PREFIX);
        snprintf(runs, sizeof runs, re_lang_text(lang, RE_MSG_LOCK_RUNS_REST), start, 1456LL);
        snprintf(none, sizeof none, re_lang_text(lang, RE_MSG_LOCK_NONE), lock);
        snprintf(over, sizeof over, re_lang_text(lang, RE_MSG_LOCK_OVER), lock);
        snprintf(grade, sizeof grade, re_lang_text(lang, RE_MSG_GRADE_CHANGE), "AAA", "BBB");
        snprintf(skipped, sizeof skipped, re_lang_text(lang, RE_MSG_AUTO_SKIPPED), transfer, 3);
        snprintf(loans, sizeof loans, re_lang_text(lang, RE_MSG_FORECAST_LOANS), forecast, "$1,234,567.89");
        snprintf(cash, sizeof cash, re_lang_text(lang, RE_MSG_FORECAST_CASH), forecast, "$1,234,567.89");
        char back[200];
        const char *casino = re_lang_text(lang, RE_MSG_CASINO_BACK_PREFIX);
        snprintf(back, sizeof back, re_lang_text(lang, RE_MSG_CASINO_BACK), casino, 3, "-$1,234,567.89");
        int bad = !begins(start, lock) || !begins(runs, start) || !begins(none, lock) || !begins(over, lock) ||
                  !begins(grade, re_lang_text(lang, RE_MSG_GRADE_PREFIX)) || !begins(skipped, transfer) || !begins(loans, forecast) ||
                  !begins(cash, forecast) || !begins(back, casino);
        for (int other = 0; other < RE_LANG_COUNT; other++) {
            const char *marker = re_lang_text(other, RE_MSG_LOCK_RUNS);
            bad += begins(none, marker) || begins(over, marker) || (other != lang && begins(runs, marker));
        }
        if (bad)
            printf("  T14 '%s': a line of the summary panel does not begin as the code expects\n", re_lang_folder(lang));
        prefixes_wrong += bad != 0;
        /* the longest hover text: after an emergency measure, with a price target; and the longest row */
        re_stock_view worst = {1, 1, 1, 123.4, 0.81, -0.065, 0.015, 123456789, -0.16, 0, 0.07, 2, 1, -0.1234}, under = worst;
        under.below_line = 1;
        int red = 0;
        unsigned hover_len = re_stock_text(&worst, lang, "$1,234,567.89", 1, -0.0325, 1, hover, sizeof hover);
        unsigned row_len = re_stock_row(&under, RE_STOCK_ROW_EQUITY, lang, NULL, &red, row, sizeof row - 60);
        if (hover_len == 0 || row_len == 0) {
            too_long++;
            printf("  T14 '%s': hover text %u bytes, equity row %u bytes (0 = did not fit)\n", re_lang_folder(lang), hover_len, row_len);
        }
    }
    ok = conversions_wrong == 0 && prefixes_wrong == 0 && too_long == 0 && places_wrong == 0 && own_texts > RE_MSG_COUNT;
    g_failures += !ok;
    printf("  T14 the plugin's texts: languages=%d texts=%d translations=%d places_wrong=%d conversions_wrong=%d "
           "panel_lines_wrong=%d too_long=%d %s\n",
           RE_LANG_COUNT, RE_MSG_COUNT, own_texts, places_wrong, conversions_wrong, prefixes_wrong, too_long, ok ? "ok" : "FAIL");

    /* T15: the casino. The table and the attributes the plugin answers for it; the result of a month is the same
     * whenever it is asked and comes with the table's chances over many months; the attribute detour answers for
     * the four games only and returns in the game's place with the strings destroyed once; the wrapper of the two
     * money calls turns the first hour into "nothing booked" and hands the last hour to the callback. */
    {
        static const double want_return[RE_CASINO_GAMES] = {0.98, 0.96, 0.95, 0.93};
        static const float want_money[RE_CASINO_GAMES] = {-100000.0f, -1000000.0f, -10000000.0f, -100000000.0f};
        int table_wrong = 0;
        for (int g = 0; g < RE_CASINO_GAMES; g++) {
            const re_casino_game *game = &RE_CASINO[g];
            float money = 0.0f, other = -1.0f;
            int id = game->id;
            table_wrong += re_casino_game_of(id) != game || !same_number(re_casino_return(game), want_return[g] + g_control) ||
                           !re_casino_attribute(id, "action", 6, "money", 5, &money) || money != want_money[g];
            table_wrong += !re_casino_attribute(id, "action", 6, "moneyVariance", 13, &other) || other != 0.0f;
            table_wrong += !re_casino_attribute(id, "action", 6, "interval", 8, &other) || other != 1.0f;
            table_wrong += !re_casino_attribute(id, "action", 6, "intervalGlobal", 14, &other) || other != 1.0f;
            table_wrong += !re_casino_attribute(id, "stats", 5, "netWorthMod", 11, &other) || other != 0.0f;
            /* not the plugin's: another attribute, the right name in the wrong section, another object */
            table_wrong += re_casino_attribute(id, "action", 6, "hours", 5, &other) || re_casino_attribute(id, "stats", 5, "money", 5, &other) ||
                           re_casino_attribute(id + 1, "action", 6, "money", 5, &other);
        }
        ok = table_wrong == 0 && re_casino_game_of(RE_ID_PERSONAL_LOAN) == NULL && re_casino_net(100000, 3.0) == 200000 &&
             re_casino_net(100000, 0.0) == -100000 && re_casino_net(10000000, 2.5) == 15000000;
        g_failures += !ok;
        printf("  T15 the four games and their attributes            wrong=%d of %d %s\n", table_wrong, RE_CASINO_GAMES, ok ? "ok" : "FAIL");

        enum { MONTHS = 200000 };
        const unsigned long long seed = 0x0123456789abcdefull;
        int draws_wrong = 0;
        for (int g = 0; g < RE_CASINO_GAMES; g++) {
            const re_casino_game *game = &RE_CASINO[g];
            int hits[2] = {0, 0};
            double paid = 0.0;
            for (int month = 0; month < MONTHS; month++) {
                double times = re_casino_times(game, seed, month);
                paid += times;
                for (int i = 0; i < game->prizes; i++)
                    hits[i] += times == game->prize[i].times;
            }
            for (int i = 0; i < game->prizes; i++) {
                double mean = MONTHS * game->prize[i].chance, sd = sqrt(mean * (1.0 - game->prize[i].chance));
                draws_wrong += fabs(hits[i] - mean) > 4.0 * sd;
            }
            draws_wrong += fabs(paid / MONTHS - re_casino_return(game)) > 0.04;
            printf("  T15 game %d over %d months: prizes %d and %d times (expected %.0f and %.0f), %.4f of the stakes back (table %.2f)\n",
                   game->id, MONTHS, hits[0], hits[1], MONTHS * game->prize[0].chance, game->prizes > 1 ? MONTHS * game->prize[1].chance : 0.0,
                   paid / MONTHS, re_casino_return(game));
        }
        int same = 0, follows = 0;
        for (int month = 0; month < 1000; month++) {
            double unit = re_casino_unit(seed, RE_ID_SLOTS, month);
            same += unit == re_casino_unit(seed, RE_ID_SLOTS, month) && unit >= 0.0 && unit < 1.0;
            follows += unit == re_casino_unit(seed + 1, RE_ID_SLOTS, month) || unit == re_casino_unit(seed, RE_ID_SLOTS, month + 1) ||
                       unit == re_casino_unit(seed, RE_ID_ROULETTE, month);
        }
        ok = draws_wrong == 0 && same == 1000 + g_control && follows == 0;
        g_failures += !ok;
        printf("  T15 results by playthrough, game and month         outside_four_sd=%d same_again=%d of 1000 follows_another=%d %s\n", draws_wrong,
               same, follows, ok ? "ok" : "FAIL");

        static const struct {
            int id;
            const char *section, *name;
            float want;
            unsigned runs, frees; /* the stand-in ran; strings the plugin destroyed in its place */
        } reads[] = {
            {RE_ID_PERSONAL_LOAN, "action", "money", 7.5f, 1, 0},  {RE_ID_SLOTS, "action", "hours", 7.5f, 1, 0},
            {RE_ID_SLOTS, "action", "money", -100000.0f, 0, 3},    {RE_ID_ROULETTE, "action", "interval", 1.0f, 0, 3},
            {RE_ID_BLACKJACK, "stats", "netWorthMod", 0.0f, 0, 3}, {RE_ID_BACCARAT, "action", "money", -100000000.0f, 0, 3},
            {RE_ID_BACCARAT, "stats", "active", 7.5f, 1, 0},
        };
        const float stand_in = 7.5f;
        msvc_string three[3];
        int reads_wrong = 0;
        re_casino_str_free = (void *)fake_string_free;
        installed = re_detour5((BYTE *)fake_attribute, RE_GETTER_PROLOGUE, re_casino_attr_hook, &re_casino_attr_trampoline);
        printf("  T15 attribute detour installed=%d\n", installed);
        g_failures += !installed;
        for (int round = 0; round < 20000 && reads_wrong == 0; round++)
            for (int i = 0; i < (int)(sizeof reads / sizeof reads[0]); i++) {
                unsigned intact = 0, bits;
                float got;
                three[0] = inline_name("activity");
                three[1] = inline_name(reads[i].section);
                three[2] = inline_name(reads[i].name);
                g_attr_runs = g_string_frees = g_attr_seen_id = 0;
                bits = call_attribute(fake_attribute, &stand_in, reads[i].id, three, &intact);
                memcpy(&got, &bits, 4);
                int wrong = got != reads[i].want + (float)control || intact != 1 || g_attr_runs != reads[i].runs ||
                            g_string_frees != reads[i].frees || (reads[i].runs && g_attr_seen_id != (unsigned)reads[i].id);
                if (wrong)
                    printf("  T15 read %d (%d %s.%s), round %d: got=%g want=%g intact=%u stand_in_runs=%u frees=%u\n", i, reads[i].id,
                           reads[i].section, reads[i].name, round, got, reads[i].want, intact, g_attr_runs, g_string_frees);
                reads_wrong += wrong;
            }
        ok = installed && reads_wrong == 0;
        g_failures += !ok;
        printf("  T15 attributes through the detour                  reads_wrong=%d of %d kinds %s\n", reads_wrong,
               (int)(sizeof reads / sizeof reads[0]), ok ? "ok" : "FAIL");

        fake_object game_object = {{0, 0, 0}, RE_ID_ROULETTE, 0.0f}, other_object = {{0, 0, 0}, RE_ID_PERSONAL_LOAN, 0.0f};
        unsigned char actions[4], record[4];
        static const struct {
            int game;
            unsigned phase, want_phase, want_end;
        } calls[] = {{0, 1, 1, 0}, {0, 0, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 1}};
        int calls_wrong = 0;
        re_casino_money = (void *)fake_action_money;
        re_casino_end = test_casino_end;
        re_casino_start = test_casino_start;
        installed = re_patch_call((BYTE *)fake_action_site, (const BYTE *)fake_action_money, re_casino_money_hook);
        printf("  T15 money call redirected=%d\n", installed);
        g_failures += !installed;
        for (int round = 0; round < 20000 && calls_wrong == 0; round++)
            for (int i = 0; i < (int)(sizeof calls / sizeof calls[0]); i++) {
                fake_object *object = calls[i].game ? &game_object : &other_object;
                unsigned intact = 0;
                g_action_calls = g_end_calls = g_start_calls = 0;
                g_action_phase = 99;
                unsigned result = call_action(actions, record, object, calls[i].phase, &intact);
                /* the start of a game goes on record in its first hour and nowhere else */
                unsigned want_start = calls[i].game && calls[i].phase;
                int wrong = result != 0x55555555 || intact != 1 || g_action_calls != 1 || g_action_phase != calls[i].want_phase + (unsigned)control ||
                            g_start_calls != want_start || (want_start && g_start_object != (unsigned char *)object) ||
                            g_end_calls != calls[i].want_end || g_action_self != (unsigned)(UINT_PTR)actions ||
                            (calls[i].want_end &&
                             (g_end_actions != actions || g_end_record != record || g_end_object != (unsigned char *)object));
                if (wrong)
                    printf("  T15 call %d, round %d: result=0x%x intact=%u calls=%u phase=%u end_calls=%u\n", i, round, result, intact,
                           g_action_calls, g_action_phase, g_end_calls);
                calls_wrong += wrong;
            }
        ok = installed && calls_wrong == 0;
        g_failures += !ok;
        printf("  T15 the first and the last hour of a game          calls_wrong=%d of %d kinds %s\n", calls_wrong,
               (int)(sizeof calls / sizeof calls[0]), ok ? "ok" : "FAIL");
    }

    /* T16: a business. The cost of an hour of work and the order of a list; the call of the game's efficiency function; the
     * hire list's hook hands the vector on after the game's function; the name on a candidate's row gets the
     * callback's text behind it, a row made from anywhere else keeps its name, and every register stays as it was. */
    {
        /* the four candidates of run 57: wage a month, hours, efficiency as the cards show them */
        static const struct {
            long long wage;
            int hours;
            double efficiency;
            long long want;
        } staff[] = {{1037490, 160, 1.14, 5688}, {952032, 160, 1.22, 4877}, {1025796, 160, 1.18, 5433}, {469531, 69, 1.15, 5917},
                     {1037490, 0, 1.14, -1},     {1037490, 160, 0.0, -1}};
        int numbers_wrong = 0;
        long long cost[4];
        for (int i = 0; i < (int)(sizeof staff / sizeof staff[0]); i++) {
            long long got = re_business_work_cost(staff[i].wage, staff[i].hours, staff[i].efficiency);
            if (i < 4)
                cost[i] = got;
            numbers_wrong += got != staff[i].want + (control && i == 0);
        }
        static const long long ties[3] = {5, 5, 3}, unknown[3] = {-1, 7, 3};
        int order[4], tie_order[3], unknown_order[3];
        re_business_order(cost, 4, order);
        re_business_order(ties, 3, tie_order);
        re_business_order(unknown, 3, unknown_order);
        int order_ok = order[0] == 1 && order[1] == 2 && order[2] == 0 && order[3] == 3 && tie_order[0] == 2 && tie_order[1] == 0 &&
                       tie_order[2] == 1 && unknown_order[0] == 2 && unknown_order[1] == 1 && unknown_order[2] == 0;
        int ok = numbers_wrong == 0 && order_ok;
        g_failures += !ok;
        printf("  T16 an hour of work: cost and order                numbers_wrong=%d order=%d,%d,%d,%d %s\n", numbers_wrong, order[0],
               order[1], order[2], order[3], ok ? "ok" : "FAIL");

        float skill = 1.14f;
        float efficiency = re_business_efficiency((void *)fake_efficiency, (void *)0x1234, 77, &skill);
        ok = efficiency == skill && g_eff_jobs == 0x1234 && g_eff_job == 77;
        g_failures += !ok;
        printf("  T16 the efficiency call                            got=%.2f jobs=0x%x job=%u %s\n", efficiency, g_eff_jobs, g_eff_job,
               ok ? "ok" : "FAIL");

        unsigned char *vector[3] = {0};
        unsigned intact = 0;
        re_business_list = (void *)fake_list;
        re_business_listed = test_listed;
        int installed = re_patch_call((BYTE *)fake_list_site, (const BYTE *)fake_list, re_business_list_hook);
        unsigned result = installed ? call_list((void *)0x5678, vector, 12, 34, &intact) : 0;
        ok = installed && result == 0x77777777 && intact == 1 && g_list_firms == 0x5678 && g_list_firm == 12 && g_list_job == 34 &&
             g_listed_calls == 1 && g_listed_firms == (void *)0x5678 && g_listed_vector == vector;
        g_failures += !ok;
        printf("  T16 the hire list's hook                           redirected=%d result=0x%x intact=%u callback=%u %s\n", installed, result,
               intact, g_listed_calls, ok ? "ok" : "FAIL");

        static const char empty[] = "";
        unsigned char string[24], subject[8];
        re_business_assign = (void *)fake_line_assign;
        re_business_name = (void *)fake_name;
        re_business_hire_line = test_line;
        re_business_hire_return = 0x00627bdb;
        installed = re_patch_call((BYTE *)fake_hire_site, (const BYTE *)fake_line_assign, re_business_hire_row_hook) &&
                    re_patch_call((BYTE *)fake_name_site, (const BYTE *)fake_name, re_business_name_hook);
        /* a candidate's row with a text and one without, a row made from elsewhere after a candidate was noted, a
         * name fetched with no candidate noted */
        static const struct {
            int noted, text;
            unsigned from;
            const char *want; /* what the game's assign is given for the name; NULL = the name is left alone */
        } rows[] = {{1, 1, 0x00627bdb, "Interview (a line of text)"}, {1, 0, 0x00627bdb, NULL}, {1, 1, 0x005dd359, NULL}, {0, 1, 0x00627bdb, NULL}};
        int rows_wrong = 0;
        for (int round = 0; installed && round < 20000 && rows_wrong == 0; round++)
            for (int i = 0; i < (int)(sizeof rows / sizeof rows[0]); i++) {
                msvc_string title = inline_name("Interview");
                unsigned line_ok = 1, name_intact = 0;
                g_line_text = rows[i].text ? "a line of text" : NULL;
                g_line_calls = 0;
                g_assign_copy[0] = 0;
                if (rows[i].noted) { /* the assign in front of the row runs as the game wrote it */
                    result = call_line(string, subject, empty, &intact);
                    line_ok = intact == 1 && result == (unsigned)(UINT_PTR)string && g_assign_string == (unsigned)(UINT_PTR)string &&
                              g_assign_text == (unsigned)(UINT_PTR)empty && g_assign_length == 0;
                }
                g_assign_string = g_assign_length = 0;
                result = call_name((void *)0x4321, &title, rows[i].from, &name_intact);
                int renamed = g_assign_string == (unsigned)(UINT_PTR)&title;
                unsigned asked = rows[i].noted && rows[i].from == 0x00627bdb;
                int wrong = !line_ok || name_intact != 1 || result != (unsigned)(UINT_PTR)&title || g_name_object != 0x4321 || g_line_calls != asked ||
                            (asked && g_line_subject != subject) ||
                            (rows[i].want != NULL ? !renamed || strcmp(g_assign_copy, rows[i].want) != 0 ||
                                                        g_assign_length != strlen(rows[i].want) + (unsigned)control
                                                  : renamed);
                if (wrong)
                    printf("  T16 row %d, round %d: line_ok=%u intact=%u callback=%u renamed=%d name='%s' length=%u\n", i, round, line_ok, name_intact,
                           g_line_calls, renamed, g_assign_copy, g_assign_length);
                rows_wrong += wrong;
            }
        ok = installed && rows_wrong == 0;
        g_failures += !ok;
        printf("  T16 the name on a candidate's row                  redirected=%d rows_wrong=%d of %d kinds %s\n", installed, rows_wrong,
               (int)(sizeof rows / sizeof rows[0]), ok ? "ok" : "FAIL");
    }

    /* T17: futures. One month end of the rule against numbers worked out by hand (two industries, one of them
     * above the overall price level with a rising rate, one below it with a falling rate and a policy term), and
     * the expected rate of an asset that follows the first industry by half. The two hooks hand on what they are
     * given and leave every register as it was. */
    {
        re_fut_economy e = {2, {{10150, 0.10, 1.2, 0.6, 0.0}, {10160, -0.02, 0.9, 0.4, 0.001}}, 0.05, 1.0, 0.03};
        static const re_fut_tag half[1] = {{10150, 0.5}};
        double level = re_futures_level(&e, half, 1), expected = re_futures_expected_rate(&e, half, 1, 1);
        re_futures_step(&e);
        static const double want[7] = {0.0852727, 1.2085273, -0.0178947, 0.8986579, 0.0440057, 1.0036671, 0.066515};
        const double got[7] = {e.industry[0].rate, e.industry[0].index, e.industry[1].rate, e.industry[1].index,
                               e.overall_rate,     e.overall_index,     expected};
        int rule_wrong = fabs(level - 1.1) > 1e-9;
        for (int i = 0; i < 7; i++)
            rule_wrong += fabs(got[i] - want[i] - (control && i == 0 ? 0.01 : 0.0)) > 2e-6;
        int ok = rule_wrong == 0;
        g_failures += !ok;
        printf("  T17 a month end of the rates by the rule           numbers_wrong=%d rate=%.7f overall=%.7f expected=%.6f %s\n", rule_wrong,
               got[0], got[4], got[6], ok ? "ok" : "FAIL");

        unsigned char list[2 * RE_TAG_BYTES], string[24], source[24], window[8];
        unsigned intact = 0;
        int hooks_wrong = 0;
        re_futures_list = (void *)fake_rate_list;
        re_futures_listed = test_fut_listed;
        re_futures_copy = (void *)fake_copy;
        re_futures_value = test_fut_value;
        int installed = re_patch_call((BYTE *)fake_rate_site, (const BYTE *)fake_rate_list, re_futures_list_hook) &&
                        re_patch_call((BYTE *)fake_value_site, (const BYTE *)fake_copy, re_futures_value_hook);
        for (int round = 0; installed && round < 20000 && hooks_wrong == 0; round++) {
            g_fut_listed_calls = g_fut_value_calls = 0;
            unsigned result = call_rate_list((void *)0x2468, list, list + sizeof list, &intact);
            int wrong = result != (unsigned)(UINT_PTR)list || intact != 1 || g_rate_economy != 0x2468 ||
                        g_fut_listed_calls != 1 + (unsigned)control || g_fut_listed_economy != (void *)0x2468 || g_fut_listed_first != list ||
                        g_fut_listed_end != list + sizeof list;
            intact = 0;
            result = call_value(string, source, window, &intact);
            wrong += result != (unsigned)(UINT_PTR)string || intact != 1 || g_copy_string != (unsigned)(UINT_PTR)string ||
                     g_copy_source != (unsigned)(UINT_PTR)source || g_fut_value_calls != 1 || g_fut_value_string != string ||
                     g_fut_value_window != window;
            if (wrong)
                printf("  T17 round %d: listed=%u value=%u intact=%u\n", round, g_fut_listed_calls, g_fut_value_calls, intact);
            hooks_wrong += wrong;
        }
        ok = installed && hooks_wrong == 0;
        g_failures += !ok;
        printf("  T17 the two futures hooks                          redirected=%d hooks_wrong=%d %s\n", installed, hooks_wrong, ok ? "ok" : "FAIL");
    }

    /* T18: memory. A list of five: three documents, an empty place and an object of another kind. The three are
     * deleted through the entry of their table with the flag 1, the list is empty afterwards and keeps its storage,
     * the other object is not touched; something that is not a list is left as it is. The detour of the loader's
     * destructor hands the loader on, runs the destructor and leaves every register as it was. */
    {
        void *table[17] = {0}, *other_table[17] = {0};
        table[RE_XML_DOC_END_SLOT / 4] = (void *)fake_doc_end;
        other_table[RE_XML_DOC_END_SLOT / 4] = (void *)fake_doc_end;
        void *doc[3][2] = {{table, 0}, {table, 0}, {table, 0}}, *other[2] = {other_table, 0};
        void *items[5] = {doc[0], NULL, doc[1], other, doc[2]};
        void **list[3] = {items, items + 5, items + 5}, **backwards[3] = {items + 5, items, items + 5};
        int deleted = re_memory_release((BYTE *)list, table, RE_XML_DOC_END_SLOT);
        int refused = re_memory_release((BYTE *)backwards, table, RE_XML_DOC_END_SLOT);
        int ok = deleted == 3 + control && refused == -1 && g_doc_ends == 3 && g_doc_flags == 1 && list[0] == items && list[1] == items &&
                 list[2] == items + 5 && backwards[1] == items && doc[0][1] == (void *)0xdead && doc[1][1] == (void *)0xdead &&
                 doc[2][1] == (void *)0xdead && other[1] == NULL;
        g_failures += !ok;
        printf("  T18 the documents of a list                        deleted=%d not_a_list=%d calls=%u flags=%u other_touched=%d %s\n",
               deleted, refused, g_doc_ends, g_doc_flags, other[1] != NULL, ok ? "ok" : "FAIL");

        re_memory_ended = test_loader_ended;
        int installed = re_detour5((BYTE *)fake_loader_end, RE_GETTER_PROLOGUE, re_memory_end_hook, &re_memory_end_trampoline);
        int rounds_wrong = 0;
        for (unsigned round = 1; installed && round <= 20000 && rounds_wrong == 0; round++) {
            BYTE *loader = (BYTE *)(UINT_PTR)(0x1000 * round);
            unsigned intact = call_loader_end(loader);
            rounds_wrong += intact != 1 || g_loader_calls != round || g_loader_seen != loader || g_loader_ran != round ||
                            g_loader_ended != (unsigned)(UINT_PTR)loader;
            if (rounds_wrong)
                printf("  T18 round %u: intact=%u callback=%u destructor=%u\n", round, intact, g_loader_calls, g_loader_ran);
        }
        ok = installed && rounds_wrong == 0;
        g_failures += !ok;
        printf("  T18 the detour of the loader's destructor          installed=%d rounds_wrong=%d %s\n", installed, rounds_wrong,
               ok ? "ok" : "FAIL");
    }

    /* T19: an offer against the standard cost of its work. The six rows of note b18 "Q4" and its first row under a
     * wage policy, each at 24 and at 48 months; work without materials whose weighted cost came out a little short;
     * offers that cannot be valued. Then the three calls of the game's functions on stand-ins: what each is handed,
     * what comes back, the contract left as it was. */
    {
        /* Cents per listed hour of the main job: labour with what the job sets off, inventory, utilities; before
         * them the listed hours of a month and the hours set off per listed hour, behind them what a wage policy
         * adds to an hour of work. Then the premium in per cent at 24 and at 48 months (the length adds 5.5% there)
         * and the cents left per hour of work at 24. The note's table has the premiums with one decimal; its 46.2
         * for the second row is this 46.147 rounded twice. */
        static const struct {
            double hours, set_off, labour, inventory, utilities, policy, want24, want48, want_left;
        } rows[] = {{360, 0.05, 2295.00, 25.00, 1169.20, 0, 54.732, 63.243, 1818.78}, /* Trucking Company, delivery driver */
                    {360, 0.15, 2785.00, 75.00, 4635.60, 0, 46.147, 54.185, 3007.79}, /* the same, truck driver */
                    {400, 0.30, 4025.00, 325.00, 1.30, 0, 62.750, 71.702, 2100.35},   /* Law Firm, criminal lawyer */
                    {400, 0.30, 5625.00, 325.00, 1.30, 0, 63.355, 72.340, 2900.35},   /* the same, business lawyer */
                    {360, 0.30, 2772.50, 650.00, 31.20, 0, 59.083, 67.832, 1569.65},  /* Auto-Repair Shop */
                    {480, 0.00, 3050.00, 0.00, 0.00, 0, 65.000, 74.075, 1982.50},     /* Fitness Centre */
                    /* the first row with the wage factor at 1.1: a dollar more for every hour of work */
                    {360, 0.05, 2295.00, 25.00, 1169.20, 100, 50.212, 58.474, 1718.78}};
        int rows_wrong = 0;
        double first_premium = 0.0;
        for (int i = 0; i < (int)(sizeof rows / sizeof rows[0]); i++)
            for (int late = 0; late < 2; late++) {
                double month = rows[i].hours * (1.65 * rows[i].labour + 1.35 * (rows[i].inventory + rows[i].utilities));
                double work = rows[i].hours * (1.0 + rows[i].set_off);
                int months = late ? 48 : 24;
                re_offer offer = {llround(month * months * (late ? 1.055 : 1.0)), months, rows[i].hours * rows[i].labour + rows[i].policy * work,
                                  rows[i].hours, rows[i].hours * rows[i].set_off, month, rows[i].policy, 1.65, 1.35, 1.35};
                re_offer_value value = {0};
                double want = (late ? rows[i].want48 : rows[i].want24) + (control && i == 0 ? 1.0 : 0.0);
                int wrong = !re_business_offer_value(&offer, &value) || fabs(100.0 * value.premium - want) > 0.001 ||
                            (!late && fabs(value.surplus - rows[i].want_left) > 0.005) ||
                            fabs(value.materials - rows[i].hours * (rows[i].inventory + rows[i].utilities)) > 0.01 ||
                            fabs(value.cost - offer.labour - value.materials) > 1e-6;
                if (wrong)
                    printf("  T19 row %d, %d months: premium %.3f%% (expected %.3f), %.2f cents left per hour of work, materials %.2f\n", i,
                           months, 100.0 * value.premium, want, value.surplus, value.materials);
                rows_wrong += wrong;
                if (i == 0 && !late)
                    first_premium = 100.0 * value.premium;
            }
        /* 480 hours of 3050 cents, the weighted cost three cents under 1.65 times that: no materials, not negative ones */
        re_offer bare = {24 * 2415597, 24, 480 * 3050.0, 480, 0, 1.65 * 480 * 3050.0 - 3.0, 0, 1.65, 1.35, 1.35}, bad;
        re_offer_value value = {0};
        int short_ok = re_business_offer_value(&bare, &value) && value.materials == 0.0 && fabs(100.0 * value.premium - 65.0) < 0.001;
        int refused = 0;
        bad = bare, bad.x_utilities = 1.30; /* a mod with two different markups */
        refused += !re_business_offer_value(&bad, &value);
        bad = bare, bad.months = 0;
        refused += !re_business_offer_value(&bad, &value);
        bad = bare, bad.policy = 3050.0; /* a policy that takes the whole wage */
        refused += !re_business_offer_value(&bad, &value);
        bad = bare, bad.weighted = 0.0; /* the game's payout function answered nothing */
        refused += !re_business_offer_value(&bad, &value);
        int ok = rows_wrong == 0 && short_ok && refused == 4;
        g_failures += !ok;
        printf("  T19 an offer against its standard cost             rows_wrong=%d of %d first=%+.3f%% no_materials=%d refused=%d of 4 %s\n",
               rows_wrong, 2 * (int)(sizeof rows / sizeof rows[0]), first_premium, short_ok, refused, ok ? "ok" : "FAIL");

        static const char element[] = "business", name[] = "contractJobHoursPayoutXer";
        unsigned contract[RE_CONTRACT_BYTES / 4];
        for (unsigned i = 0; i < RE_CONTRACT_BYTES / 4; i++)
            contract[i] = 0x01010101u * (i + 1);
        contract[RE_CONTRACT_MONTHS / 4] = 42;
        unsigned first_word = contract[0], last_word = contract[RE_CONTRACT_BYTES / 4 - 1];
        int calls_wrong = 0;
        for (int round = 0; round < 20000 && calls_wrong == 0; round++) {
            g_offer_copies = g_number_empty = 0;
            long long paid = re_business_payout((void *)fake_contract_copy, (void *)fake_payout, (void *)0x5678, 12 + round, contract,
                                                sizeof contract);
            float length = re_business_length((void *)fake_length, (void *)0x9abc, 24 + round % 60);
            float number = re_business_data_number((void *)fake_data_number, (void *)0x2468, (void *)fake_text_assign, element,
                                                   sizeof element - 1, name, sizeof name - 1);
            int wrong = paid != (((long long)0x12345 << 32) | 42) || g_offer_copies != 1 || g_payout_firms != 0x5678 ||
                        g_payout_firm != 12u + (unsigned)round + (unsigned)control || g_payout_type != first_word ||
                        g_payout_last != last_word || contract[0] != first_word || length != (float)(24 + round % 60) ||
                        g_length_firms != 0x9abc || number != 1.65f || g_number_data != 0x2468 || g_number_empty != 2 ||
                        g_number_element != (unsigned)(UINT_PTR)element || g_number_element_length != sizeof element - 1 ||
                        g_number_name != (unsigned)(UINT_PTR)name || g_number_name_length != sizeof name - 1;
            if (wrong)
                printf("  T19 round %d: paid=0x%llx copies=%u firm=%u length=%.1f number=%.2f empty=%u element_length=%u name_length=%u\n", round,
                       (unsigned long long)paid, g_offer_copies, g_payout_firm, length, number, g_number_empty, g_number_element_length,
                       g_number_name_length);
            calls_wrong += wrong;
        }
        ok = calls_wrong == 0;
        g_failures += !ok;
        printf("  T19 the game's payout, length and data number      calls_wrong=%d %s\n", calls_wrong, ok ? "ok" : "FAIL");
    }

    /* T20: the wage demand of an automatically managed employee (note b20). The candidate for the place: of those who
     * get at least nine tenths of the person's work done in a month, the cheapest hour's worth of work. Then the
     * answer: yes unless that candidate is cheaper by more than the margin; yes too when a number is missing. */
    {
        /* the person: 160 hours at 110%, 176 hours of work a month; the place needs 158.4 */
        static const long long wage[] = {334114, 386617, 696334, 821844, 700000, 0};
        static const int hours[] = {48, 84, 160, 160, 160, 160};
        static const double efficiency[] = {0.8832, 1.0701, 0.9496, 1.0338, 0.0, 1.2};
        /* The job wants the person's whole month, 176 hours of work. An hour of it: 7881 (too few hours), 4301 (too
         * few), 4583 (152 hours of work: too few), 4969 (821844 over the 165.4 hours that candidate gets done), none,
         * none */
        double cost = -1.0, none_cost = -1.0;
        int pick = re_business_wage_pick(wage, hours, efficiency, 6, 160, 1.1, 176.0, &cost) ^ (control ? 1 : 0);
        int nobody = re_business_wage_pick(wage, hours, efficiency, 3, 160, 1.1, 176.0, &none_cost);
        /* for a person of 80 hours at 100% the part-timer of 84 hours is enough and the cheapest: 386617 over the 80
         * hours that are wanted, not over the 89.9 that candidate could do */
        double part_cost = -1.0;
        int part = re_business_wage_pick(wage, hours, efficiency, 6, 80, 1.0, 80.0, &part_cost);
        /* Seen in the game (run 118): a person of 48 hours at 108.2% asks for 188586 and did the whole month, 51.9
         * hours of work; the candidate has 88 hours at 112.35% for 334316. With nothing left undone in the job the
         * candidate's hour of the wanted work is 334316 / 51.9 = 6437 against 3631: the rise. With 60 hours of work
         * left undone in the job the place is wanted for 111.9: the candidate does 98.9 of it, 3381 an hour against
         * 3631 for the 51.9 the person does: refused. */
        static const long long seen_wage[] = {334316};
        static const int seen_hours[] = {88};
        static const double seen_efficiency[] = {1.1235};
        double seen_cost = -1.0, busy_cost = -1.0, did = 48 * 1.082;
        re_wage_costs seen = {-1.0, -1.0}, busy = {-1.0, -1.0};
        int seen_pick = re_business_wage_pick(seen_wage, seen_hours, seen_efficiency, 1, 48, 1.082, did, &seen_cost);
        int seen_rise = re_business_wage_accept(188586, 48, 1.082, did, seen_cost, 0.03, &seen);
        int busy_pick = re_business_wage_pick(seen_wage, seen_hours, seen_efficiency, 1, 48, 1.082, did + 60.0, &busy_cost);
        int busy_rise = re_business_wage_accept(188586, 48, 1.082, did + 60.0, busy_cost, 0.03, &busy);
        /* a person who did nothing in a job that left nothing undone: the wages themselves are compared */
        double idle_cost = -1.0;
        int idle_pick = re_business_wage_pick(wage, hours, efficiency, 6, 160, 1.1, 0.0, &idle_cost);
        int picks_wrong = (pick != 3) + (fabs(cost - 821844.0 / (160 * 1.0338)) > 1e-6) + (nobody != -1) + (part != 1) +
                          (fabs(part_cost - 386617.0 / 80.0) > 1e-6) + (seen_pick != 0) + (fabs(seen_cost - 334316.0 / did) > 1e-6) +
                          (seen_rise != 1) + (busy_pick != 0) + (fabs(busy_cost - 334316.0 / (88 * 1.1235)) > 1e-6) + (busy_rise != 0) +
                          (fabs(busy.keep - 188586.0 / did) > 1e-6) + (idle_pick != 0) + (idle_cost != 334114.0);

        static const struct {
            long long demanded;
            double fresh, margin;
            int want;
        } cases[] = {{880000, 4969.0, 0.03, 1},  /* 5000.0 an hour of work against 4969 x 1.03 = 5118.1: the rise is given */
                     {880000, 4969.0, 0.0, 0},   /* without a margin the candidate is cheaper */
                     {915200, 4969.0, 0.03, 0},  /* 5200.0: the candidate is cheaper by more than 3% */
                     {880000, 4800.0, 0.03, 0},  /* against 4944.0 */
                     {880000, 5000.0, 0.03, 1},  /* the same cost: the rise */
                     {880000, 9000.0, 0.03, 1}};
        int cases_wrong = 0;
        for (int i = 0; i < (int)(sizeof cases / sizeof cases[0]); i++) {
            re_wage_costs costs = {-1.0, -1.0};
            int got = re_business_wage_accept(cases[i].demanded, 160, 1.1, 176.0, cases[i].fresh, cases[i].margin, &costs);
            double keep = (double)cases[i].demanded / 176.0;
            int wrong = got != cases[i].want || fabs(costs.keep - keep) > 1e-6 || costs.fresh != cases[i].fresh;
            if (wrong)
                printf("  T20 case %d: asked %lld, the candidate %.1f, margin %.2f: accept=%d (expected %d), kept %.2f (expected %.2f)\n", i,
                       cases[i].demanded, cases[i].fresh, cases[i].margin, got, cases[i].want, costs.keep, keep);
            cases_wrong += wrong;
        }
        re_wage_costs costs = {-1.0, -1.0};
        int blind = 0;
        blind += re_business_wage_accept(880000, 0, 1.1, 176.0, 4000.0, 0.03, &costs) && costs.keep == 0.0 && costs.fresh == 0.0; /* no hours */
        blind += re_business_wage_accept(880000, 160, 0.0, 176.0, 4000.0, 0.03, &costs);                                         /* no efficiency */
        blind += re_business_wage_accept(880000, 160, 1.1, 176.0, 0.0, 0.03, &costs);                                            /* no candidate */
        blind += re_business_wage_accept(0, 160, 1.1, 176.0, 4000.0, 0.03, &costs);                                              /* no demand */
        /* Staff filled in during the month. Twelve hours of work without hands: the lowest wage that gets them done,
         * the part-timer of 48 hours (334114 / 12). Three hundred: the best rate of what a person adds, the 84 hours
         * at 107% (386617 / 89.9 = 4301). With 30 game hours left in the month nobody does more than 30 hours: of
         * 300 missing the 84-hour person does 32.1 (12044 an hour), which is still the cheapest. A candidate without
         * a wage or without efficiency is never picked; no candidates, nobody. Short or not, at 5% of the job's
         * month: of a month of 215 hours, 13 still to do with hands for none is short (6%); 100 to do with hands for
         * 96 is not (4 missing, under the least); of a month of 1000, 40 missing is not (4%), 51 is; a job that was
         * given nothing is never short. */
        double few_cost = -1.0, many_cost = -1.0, late_cost = -1.0, no_cost = -1.0;
        int few = re_business_fill_pick(wage, hours, efficiency, 6, 12.0, 600, &few_cost) ^ (control ? 1 : 0);
        int many = re_business_fill_pick(wage, hours, efficiency, 6, 300.0, 600, &many_cost);
        int late = re_business_fill_pick(wage, hours, efficiency, 6, 300.0, 30, &late_cost);
        int fills_wrong = (few != 0) + (fabs(few_cost - 334114.0 / 12.0) > 1e-6) + (many != 1) + (fabs(many_cost - 386617.0 / (84 * 1.0701)) > 1e-6) +
                          (late != 1) + (fabs(late_cost - 386617.0 / (30 * 1.0701)) > 1e-6) +
                          (re_business_fill_pick(wage + 4, hours + 4, efficiency + 4, 2, 300.0, 600, &no_cost) != -1) +
                          (re_business_fill_pick(wage, hours, efficiency, 0, 300.0, 600, &no_cost) != -1) +
                          (re_business_short(215, 13, 0.0, 0.05) != 1) + (re_business_short(100, 100, 96.0, 0.05) != 0) +
                          (re_business_short(1000, 1000, 960.0, 0.05) != 0) + (re_business_short(1000, 1000, 949.0, 0.05) != 1) +
                          (re_business_short(0, 0, 0.0, 0.05) != 0);
        /* what missing assets leave of an hour of work: all of it when nothing is short; a job's assets at 0.8 with
         * whole furnishings leave 0.8; a job with next to none of its assets counts for 0.35, and with no furnishings
         * either for half of that */
        fills_wrong += (fabs(re_business_asset_factor(1.0, 1.0) - 1.0) > 1e-9) + (fabs(re_business_asset_factor(0.8, 1.0) - 0.8) > 1e-9) +
                       (fabs(re_business_asset_factor(0.1, 1.0) - 0.35) > 1e-9) + (fabs(re_business_asset_factor(0.0, 0.0) - 0.175) > 1e-9);
        /* a person the job did not need: 150 hours' worth of work done, the two others get 320 done in a month and left
         * 190 unworked - the work and a tenth of their month, 32, to spare. Not with 180 left (two short), not in a
         * job of one. A month without any work still needs the others to have their tenth free. */
        int spare_wrong = (re_business_wage_spare(150.0, 190.0, 320.0, 2) != 1) + (re_business_wage_spare(150.0, 180.0, 320.0, 2) != 0) +
                          (re_business_wage_spare(0.0, 100.0, 320.0, 0) != 0) + (re_business_wage_spare(0.0, 33.0, 320.0, 1) != 1) +
                          (re_business_wage_spare(0.0, 31.0, 320.0, 1) != 0);
        /* a person alone in a job, 160 hours for 700000 cents, who did 40 hours' worth of work: the candidates above
         * are two part-timers (48 hours for 334114, 84 hours for 386617) and four of 160 hours. A quarter more than
         * the work done is 50 hours' worth; the first part-timer gets 42.4 done, the second 89.9: the second. With
         * 20 hours' worth done the cheaper first one is enough. Nobody when the present wage is lower than theirs,
         * and nobody when the present person has no more hours than they. */
        int smaller_wrong = (re_business_smaller_pick(wage, hours, efficiency, 6, 700000, 160, 40.0) != 1) +
                            (re_business_smaller_pick(wage, hours, efficiency, 6, 700000, 160, 20.0) != 0) +
                            (re_business_smaller_pick(wage, hours, efficiency, 6, 300000, 160, 20.0) != -1) +
                            (re_business_smaller_pick(wage, hours, efficiency, 6, 700000, 48, 20.0) != -1);
        int ok = picks_wrong == 0 && cases_wrong == 0 && blind == 4 && spare_wrong == 0 && smaller_wrong == 0 && fills_wrong == 0;
        g_failures += !ok;
        printf("  T20 a wage demand against the job's candidates     picks_wrong=%d of 14 cases_wrong=%d of %d nothing_to_compare=%d of 4 "
               "spare_wrong=%d of 5 smaller_wrong=%d of 4 fills_wrong=%d of 17 %s\n",
               picks_wrong, cases_wrong, (int)(sizeof cases / sizeof cases[0]), blind, spare_wrong, smaller_wrong, fills_wrong,
               ok ? "ok" : "FAIL");
    }

    /* T21: candidates that a load does not change (note b21). What a list is drawn from: the same for the same
     * list, another for another month, business, job, draw or playthrough, and one value worked out apart from
     * this code. The game's streams around the making of a list: seeded for the list, and afterwards what they
     * were. The two hooks. */
    {
        static const char play[] = "Tue Sep 22 23:55:44 20264912879";
        unsigned long long seed = re_business_list_seed(play, 24322, 17332, 30111, 0);
        int seeds_wrong = (seed != 0xd8aaa42459ca3b28ull + (unsigned)control) + (seed != re_business_list_seed(play, 24322, 17332, 30111, 0)) +
                          (seed == re_business_list_seed(play, 24323, 17332, 30111, 0)) + (seed == re_business_list_seed(play, 24322, 17333, 30111, 0)) +
                          (seed == re_business_list_seed(play, 24322, 17332, 30112, 0)) + (seed == re_business_list_seed(play, 24322, 17332, 30111, 1)) +
                          (seed == re_business_list_seed("Tue Sep 22 23:55:44 20264912870", 24322, 17332, 30111, 0)) +
                          (re_business_stream_seed(seed, 8) != 0xdabb7762u) + (re_business_stream_seed(seed, 0) != 0x88460b2du);
        /* every stream of a list, and the first reference wages behind them, get a seed of their own */
        for (int a = 0; a < RE_RAND_STREAMS + 6; a++)
            for (int b = a + 1; b < RE_RAND_STREAMS + 6; b++)
                seeds_wrong += re_business_stream_seed(seed, a) == re_business_stream_seed(seed, b);
        int ok = seeds_wrong == 0;
        g_failures += !ok;
        printf("  T21 what a list is drawn from                         seed=%016llx seeds_wrong=%d %s\n", seed, seeds_wrong, ok ? "ok" : "FAIL");

        /* the stand-in engine is the generator the game uses: the first number of the standard seed */
        test_engine known;
        test_engine_seed(&known, 5489);
        unsigned first = test_engine_next(&known), drawn[RE_RAND_STREAMS];
        test_streams_set();
        re_business_streams_seed(&g_streams_kept, g_randgen, (void *)fake_engine_seed, seed);
        test_list_made(0);
        int same = re_business_streams_restore(&g_streams_kept, drawn);
        /* what the list drew is what engines seeded for this list give, stream by stream */
        int job = RE_RANDGEN_JOB / RE_RAND_UNIT_BYTES, draws_wrong = 0, counts_wrong = 0;
        test_engine expect;
        test_engine_seed(&expect, re_business_stream_seed(seed, job));
        for (int i = 0; i < 7; i++)
            draws_wrong += g_list_draws[i] != test_engine_next(&expect) + (control && i == 0);
        test_engine_seed(&expect, re_business_stream_seed(seed, 0));
        draws_wrong += g_list_draws[7] != test_engine_next(&expect);
        test_engine_seed(&expect, re_business_stream_seed(seed, 3));
        draws_wrong += g_list_draws[8] != test_engine_next(&expect);
        draws_wrong += g_list_draws[9] != test_engine_next(&expect);
        for (int i = 0; i < RE_RAND_STREAMS; i++)
            counts_wrong += drawn[i] != (i == job ? 7u : i == 0 ? 1u : i == 3 ? 2u : 0u);
        if (control)
            g_engines_before[job].word[100] ^= 1; /* one bit of one engine not put back has to show */
        int back = memcmp(g_engines, g_engines_before, sizeof g_engines) == 0 && memcmp(g_randgen, g_randgen_before, sizeof g_randgen) == 0;
        ok = first == 3499211612u && same == 1 && back && draws_wrong == 0 && counts_wrong == 0 && g_seeded_engines == RE_RAND_STREAMS &&
             g_seeded_order_wrong == 0;
        g_failures += !ok;
        printf("  T21 the streams around the making of a list           engine=%u seeded=%u draws_wrong=%d of 10 counts_wrong=%d as_before=%d "
               "bytes_back=%d %s\n",
               first, g_seeded_engines, draws_wrong, counts_wrong, same, back, ok ? "ok" : "FAIL");

        /* the answer "as before" is not a constant: another engine in a stream's place, or another seed, is told;
         * the engine that is not the game's any more is not written to */
        test_streams_set();
        test_engine_seed(&g_engine_elsewhere, 7);
        test_engine elsewhere_before = g_engine_elsewhere;
        re_business_streams_seed(&g_streams_kept, g_randgen, (void *)fake_engine_seed, seed);
        test_list_made(1);
        int moved = re_business_streams_restore(&g_streams_kept, drawn);
        int left_alone = memcmp(&g_engine_elsewhere, &elsewhere_before, sizeof elsewhere_before) == 0;
        test_streams_set();
        re_business_streams_seed(&g_streams_kept, g_randgen, (void *)fake_engine_seed, seed);
        test_list_made(2);
        int reseeded = re_business_streams_restore(&g_streams_kept, drawn);
        ok = moved == 0 && left_alone && reseeded == control;
        g_failures += !ok;
        printf("  T21 streams that are not what they were are told      engine_moved=%d left_alone=%d seed_changed=%d %s\n", moved, left_alone,
               reseeded, ok ? "ok" : "FAIL");

        /* the hire list's hook with a callback before the game's function, and the hook of the reference wage:
         * the seed replaced or left, everything else handed on, every register as it was */
        unsigned char *vector[3] = {0};
        unsigned intact = 0, hooks_wrong = 0;
        re_business_list = (void *)fake_list;
        re_business_listing = test_listing;
        re_business_listed = test_listed;
        g_list_firms = g_listed_calls = 0;
        unsigned result = call_list((void *)0x5678, vector, 12, 34, &intact);
        hooks_wrong += result != 0x77777777 || intact != 1 || g_listing_calls != 1 || g_listing_firms != (void *)0x5678 || g_listing_firm != 12 ||
                       g_listing_job != 34 + (unsigned)control || !g_listing_before_list || g_list_firms != 0x5678 || g_listed_calls != 1;
        re_business_listing = NULL;
        re_business_wage_ref = (void *)fake_wage_ref;
        re_business_wage_seed = test_wage_seed;
        int installed = re_patch_call((BYTE *)fake_wage_ref_site, (const BYTE *)fake_wage_ref, re_business_wage_ref_hook);
        for (int round = 0; installed && round < 20000 && hooks_wrong == 0; round++) {
            int given = 31000 + round, want = round & 1 ? 0x7654321 + round : given;
            g_ref_new_seed = round & 1 ? want : 0;
            g_ref_calls = 0;
            unsigned long long wage = call_wage_ref((void *)0x2468, 30111, (void *)0x1357, given, &intact);
            hooks_wrong += wage != ((0x12345ull << 32) | (unsigned)want) || intact != 1 || g_ref_calls != 1 || g_ref_jobs != 0x2468 ||
                           g_ref_job != 30111 || g_ref_skill != 0x1357 || g_ref_seed != (unsigned)want;
        }
        ok = installed && hooks_wrong == 0;
        g_failures += !ok;
        printf("  T21 the list's hook with its first callback, the reference wage's hook   redirected=%d hooks_wrong=%u %s\n", installed,
               hooks_wrong, ok ? "ok" : "FAIL");
    }

    /* T22: the adverts of a business type (note m22). The hook around the game's list function for a type's
     * adverts: the game's function runs first, with the vector in ecx and the arguments where it expects them; then
     * the callback gets the vector and the head of the arguments; eax and the registers come back, the arguments
     * are still the caller's to remove. */
    {
        unsigned intact = 0, hook_wrong = 0;
        re_business_brand_list = (void *)fake_brand_list;
        re_business_brand = test_brand;
        int installed = re_patch_call((BYTE *)fake_brand_site, (const BYTE *)fake_brand_list, re_business_brand_hook);
        for (int round = 0; installed && round < 20000 && hook_wrong == 0; round++) {
            unsigned vector = 0x9000u + (unsigned)round, word = 0xabc00000u + (unsigned)round;
            g_brand_calls = g_brand_seen = g_brand_vector = g_brand_first_word = g_brand_seen_word = 0;
            g_brand_seen_vector = NULL;
            call_brand_list((void *)(UINT_PTR)vector, word, &intact);
            hook_wrong += intact != 1 || g_brand_calls != 1 || g_brand_vector != vector || g_brand_first_word != word ||
                          g_brand_seen != 1 + (unsigned)control || g_brand_seen_vector != (unsigned char *const *)(UINT_PTR)vector ||
                          g_brand_seen_word != word;
        }
        int ok = installed && hook_wrong == 0;
        g_failures += !ok;
        printf("  T22 the hook of a business type's adverts            redirected=%d hook_wrong=%u %s\n", installed, hook_wrong,
               ok ? "ok" : "FAIL");
    }

    /* T23: an asset a business is short of (note b23). The ware for a tag, where the data says nothing of a ware's
     * life or of what it uses up: the cheaper for one of the tag; one the
     * business has before a cheaper one; without a power or a price none; at the same cost the smaller. The want
     * served next: a unit that does not fit into the room is passed over; with room for both the unit that fills
     * more of its want for the room it takes goes first; with too little cash nothing, and the reason is the cash;
     * no ware, no room and "has enough" are told apart. */
    {
        const long long rich = 1000000000;
        const re_asset_offer cheap = {2, 1.0, 60000, 0, 0, 0.0, 0.0}, large = {4, 4.0, 200000, 0, 0, 0.0, 0.0};
        const re_asset_offer had = {4, 4.0, 200000, 1, 0, 0.0, 0.0}, none = {0, 1.0, 60000, 0, 0, 0.0, 0.0};
        const re_asset_offer unpriced = {2, 1.0, 0, 0, 0, 0.0, 0.0}, wide = {2, 2.0, 60000, 0, 0, 0.0, 0.0};
        const re_asset_offer two[2] = {cheap, large}, with_had[2] = {cheap, had}, useless[2] = {none, unpriced}, same[2] = {wide, cheap};
        int ware_wrong = (re_business_asset_ware(two, 2, 1, rich, 1e9) != 0 + control) + (re_business_asset_ware(with_had, 2, 1, rich, 1e9) != 1) +
                         (re_business_asset_ware(useless, 2, 1, rich, 1e9) != -1) + (re_business_asset_ware(same, 2, 1, rich, 1e9) != 1) +
                         (re_business_asset_ware(two, 0, 1, rich, 1e9) != -1);
        /* The wares of the game's data (analysis/scripts/asset_choices.py), cents. Two planes that differ only in what
         * they burn: the one whose hour is cheaper at the day's prices. Three cars for two missing units of a trunk -
         * an old one (life 3000 hours, 10.80 an hour), a small one (12000, 10.20), an electric one (12000, 6.20): an
         * hour of a unit costs 6.40, 5.86 and 4.30, so the electric one; with cash for the two cheaper ones the small
         * one; with cash for none the lowest price. A kind the household's other businesses have goes before all
         * that. A lorry that brings six against one that brings two: with two missing the small one, with six the
         * large one. */
        const re_asset_offer plane_e = {2, 0.0, 10500000, 0, 0, 28800.0, 6000.0}, plane_f = {2, 0.0, 10500000, 0, 0, 28800.0, 7200.0};
        const re_asset_offer planes[2] = {plane_f, plane_e}, planes_dear_power[2] = {plane_e, {2, 0.0, 10500000, 0, 0, 28800.0, 5000.0}};
        const re_asset_offer old_car = {2, 0.0, 600000, 0, 0, 3000.0, 1080.0}, small_car = {2, 0.0, 1820000, 0, 0, 12000.0, 1020.0};
        const re_asset_offer electric_car = {2, 0.0, 2880000, 0, 0, 12000.0, 620.0};
        const re_asset_offer cars[3] = {old_car, small_car, electric_car};
        re_asset_offer fleet[3] = {old_car, small_car, electric_car};
        const re_asset_offer lorries[2] = {{6, 0.0, 11200000, 0, 0, 36000.0, 7972.0}, {2, 0.0, 5500000, 0, 0, 18000.0, 4632.0}};
        fleet[0].others = 3;
        ware_wrong += (re_business_asset_ware(planes, 2, 1, rich, 1e9) != 1) + (re_business_asset_ware(planes_dear_power, 2, 1, rich, 1e9) != 1) +
                      (re_business_asset_ware(cars, 3, 2, rich, 1e9) != 2) + (re_business_asset_ware(cars, 3, 2, 2000000, 1e9) != 1) +
                      (re_business_asset_ware(cars, 3, 2, 100000, 1e9) != 0) + (re_business_asset_ware(fleet, 3, 2, rich, 1e9) != 0) +
                      (re_business_asset_ware(lorries, 2, 2, rich, 1e9) != 1) + (re_business_asset_ware(lorries, 2, 6, rich, 1e9) != 0) +
                      (fabs(re_business_asset_hour(&electric_car, 2) - 430.0) > 1e-9);
        /* Floor space. A table that brings twelve on 16.7 square metres against a fridge that brings two on 1.68, ten
         * missing: the table by the cost of an hour while it fits; with six square metres left the fridge, which
         * fits; with one left neither fits and the choice is as if both did. */
        const re_asset_offer rooms[2] = {{12, 16.7, 580815, 0, 0, 14400.0, 0.0}, {2, 1.68, 108335, 0, 0, 78624.0, 8.3}};
        ware_wrong += (re_business_asset_ware(rooms, 2, 10, rich, 100.0) != 0) + (re_business_asset_ware(rooms, 2, 10, rich, 6.0) != 1) +
                      (re_business_asset_ware(rooms, 2, 10, rich, 1.0) != 0);
        re_asset_want want[2] = {{1, 0, {2, 40.0, 100, 0, 0, 0.0, 0.0}}, {10, 8, {2, 1.0, 100, 0, 0, 0.0, 0.0}}};
        int next_wrong = (re_business_asset_next(want, 2, 30.0, 1000000) != 1) + (re_business_asset_next(want, 2, 100.0, 1000000) != 1);
        want[1].owned = 10;
        next_wrong += (re_business_asset_next(want, 2, 100.0, 1000000) != 0) + (re_business_asset_next(want, 2, 100.0, 99) != -1) +
                      (re_business_asset_why(&want[0], 100.0, 99) != RE_ASSET_NO_CASH) +
                      (re_business_asset_why(&want[0], 30.0, 1000000) != RE_ASSET_NO_ROOM) +
                      (re_business_asset_why(&want[1], 0.0, 0) != RE_ASSET_SERVED);
        want[0].unit.power = 0;
        next_wrong += (re_business_asset_why(&want[0], 100.0, 1000000) != RE_ASSET_NO_WARE) + (re_business_asset_next(want, 2, 100.0, 1000000) != -1);
        int ok = ware_wrong == 0 && next_wrong == 0;
        g_failures += !ok;
        printf("  T23 an asset a business is short of                  ware_wrong=%d of 17 next_wrong=%d of 9 %s\n", ware_wrong, next_wrong,
               ok ? "ok" : "FAIL");
    }

    /* T24: experience kept (note m26). The requirements are read out of the data's text: the highest amount a kind,
     * nothing from a comment, a group's reference and a requirement without an amount passed over. Then what a
     * month leaves of a number: never below what was gained, up to that highest amount. */
    {
        static const char data[] = "<xml>\n<!-- <xpReq id=\"10505\" amt=\"99999\"/> a comment is not data -->\n"
                                   "<job><requires>\n<xpReqGroup id=\"83509\"/>\n<xpReq id=\"10505\" amt=\"2000\" prob=\"0.5\"/>\n"
                                   "<xpReq id=\"45036\" amt=\"650\"/>\n</requires></job>\n"
                                   "<job><requires><xpReq  amt=\"8000\"\tid=\"10505\"/><xpReq id=\"10515\"/></requires></job>\n</xml>";
        re_xp_need need[8], tiny[1];
        int count = 0, tiny_count = 0, found = re_xp_scan(data, sizeof data - 1, need, 8, &count);
        int scan_wrong = (found != 3) + (count != 2) + (re_xp_most(need, count, 10505) != 8000 + control) + (re_xp_most(need, count, 45036) != 650) +
                         (re_xp_most(need, count, 10515) != 0) + (re_xp_most(need, count, 83509) != 0) +
                         (re_xp_scan(data, sizeof data - 1, tiny, 1, &tiny_count) != -1);
        static const struct {
            long long decayed, gained;
            int most;
            long long want;
        } months[] = {
            {4950, 5000, 8000, 5000},  /* below the most a requirement asks: nothing is lost */
            {7920, 12000, 8000, 8000}, /* above it: held at that amount */
            {9900, 12000, 8000, 9900}, /* still above it: the game's decay stands */
            {495, 500, 650, 500},      /* an education of 500 against a requirement of 650 */
            {99, 100, 0, 99},          /* a kind no requirement names: the game's decay */
            {300, 0, 8000, 300},       /* nothing on record as gained: left as it is */
            {3000, 2000, 8000, 3000},  /* more than was gained on record: left as it is */
        };
        int kept_wrong = 0, month_count = (int)(sizeof months / sizeof months[0]);
        for (int i = 0; i < month_count; i++)
            kept_wrong += re_xp_kept(months[i].decayed, months[i].gained, months[i].most) != months[i].want + (control && i == 1);
        /* which kinds are educations ([48]): the id of every <education> element, not a comment's, not the data's
         * template (00000), not the id of another kind of element, each once */
        static const char schools[] = "<xml>\n<!-- <education><id>45999</id></education> -->\n<education>\n\t<id>00000</id>\n</education>\n"
                                      "<education>\n\t<!-- before the id: <id>45998</id> -->\n\t<id>45161</id>\n"
                                      "\t<special><actionStudy id=\"45161\"/></special>\n</education>\n"
                                      "<education><id>45036</id></education><job><id>31321</id></job>\n"
                                      "<education><id>45161</id></education>\n</xml>";
        int edu[4], edu_count = 0, one[1], one_count = 0, edu_found = re_xp_educations(schools, sizeof schools - 1, edu, 4, &edu_count);
        int edu_wrong = (edu_found != 3) + (edu_count != 2 + control) + (edu_count < 1 || edu[0] != 45161) + (edu_count < 2 || edu[1] != 45036) +
                        (re_xp_educations(schools, sizeof schools - 1, one, 1, &one_count) != -1);
        int ok = scan_wrong == 0 && kept_wrong == 0 && edu_wrong == 0;
        g_failures += !ok;
        printf("  T24 an education kept                                scan_wrong=%d kept_wrong=%d of %d educations_wrong=%d %s\n", scan_wrong,
               kept_wrong, month_count, edu_wrong, ok ? "ok" : "FAIL");
    }

    /* T25: which letters a font has (note m29). Two tables of characters written out here, one of each kind the
     * game's fonts use: segments of the basic plane (a letter whose glyph number comes out 0 has no picture) and
     * groups of any plane (only what is below U+10000 is kept). Then the game's own font files next to the
     * executable: the Korean font has Hangul and lacks the Polish l with a stroke, the fixed-width Korean font
     * lacks the e with an acute, the two fonts the missing letters are taken from have both letters. */
    {
        static const BYTE segments[] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0c, /* one table: Windows, basic plane */
                                        0x00, 0x04, 0x00, 0x28, 0x00, 0x00, 0x00, 0x06, 0x00, 0x04, 0x00, 0x01, 0x00, 0x02,
                                        0x00, 0x43, 0x00, 0x50, 0xff, 0xff, 0x00, 0x00, /* ends, the pad */
                                        0x00, 0x41, 0x00, 0x50, 0xff, 0xff,             /* starts: A to C, P, the closing one */
                                        0xff, 0xc0, 0xff, 0xb0, 0x00, 0x01,             /* what is added: A is glyph 1, P is glyph 0 */
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        static const BYTE groups[] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x0c, /* one table: Windows, all planes */
                                      0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
                                      0x00, 0x00, 0x01, 0x42, 0x00, 0x00, 0x01, 0x44, 0x00, 0x00, 0x00, 0x05,  /* U+0142 to U+0144 */
                                      0x00, 0x01, 0xf6, 0x00, 0x00, 0x01, 0xf6, 0x01, 0x00, 0x00, 0x00, 0x09}; /* two above the plane */
        static unsigned char bits[RE_FONT_BMP_BYTES];
#define HAS(code) ((bits[(code) >> 3] >> ((code)&7)) & 1)
        int wrong = 0, files_wrong = 0, files = 0, counts[4] = {0, 0, 0, 0};
        wrong += re_font_cmap(segments, sizeof segments, bits) != 3 + control;
        wrong += !HAS(0x41) + !HAS(0x43) + HAS(0x44) + HAS(0x50) + HAS(0xffff);
        wrong += re_font_cmap(groups, sizeof groups, bits) != 3;
        wrong += !HAS(0x142) + !HAS(0x144) + HAS(0x141) + HAS(0x145);
        wrong += re_font_cmap(groups, 15, bits) != -1; /* cut short */
        static const struct {
            const wchar_t *name;
            int hangul, l_stroke, e_acute;
        } fonts[] = {
            {L"fonts\\NotoSansKR-Regular.ttf", 1, 0, 1},
            {L"fonts\\NanumGothicCoding-Regular.ttf", 1, 1, 0},
            {L"fonts\\NotoSansMono_SemiCondensed-Regular.ttf", 0, 1, 1},
            {L"fonts\\sofiapro-light.otf", 0, 1, 1},
        };
        for (size_t i = 0; i < sizeof fonts / sizeof fonts[0]; i++) {
            wchar_t file[MAX_PATH];
            wcsncpy(file, path, MAX_PATH - 1);
            file[MAX_PATH - 1] = 0;
            wchar_t *slash = wcsrchr(file, L'\\') ? wcsrchr(file, L'\\') : wcsrchr(file, L'/');
            if (slash == NULL || (size_t)(slash - file) + wcslen(fonts[i].name) + 2 >= MAX_PATH) {
                files_wrong++;
                continue;
            }
            wcscpy(slash + 1, fonts[i].name);
            int found = re_font_file(file, bits);
            counts[i] = found;
            files += found > 0;
            files_wrong += found <= 0 || HAS(0xac00) != fonts[i].hangul || HAS(0x142) != fonts[i].l_stroke || HAS(0xe9) != fonts[i].e_acute ||
                           !HAS(0x41);
        }
#undef HAS
        int ok = wrong == 0 && files_wrong == 0;
        g_failures += !ok;
        /* the four counts are fontTools' for the same files (note m29 has the command) */
        printf("  T25 the letters a font has                           tables_wrong=%d font_files_wrong=%d of %d read, code points below "
               "U+10000: %d, %d, %d, %d %s\n",
               wrong, files_wrong, files, counts[0], counts[1], counts[2], counts[3], ok ? "ok" : "FAIL");
    }

    /* T26: a name no font file can write (REQUEST.md [48]). The three lines of the game's lists of names that are in
     * Arabic letters come out in Latin letters, alone and inside a longer text; a text without them is left. Then
     * every line of every list of extraData\namesPeople next to the executable, as the plugin hands it on: none may
     * keep a letter that no file of fonts\ has. The control adds one name in such letters that the plugin does not
     * know. */
    {
        static const struct {
            const char *text, *want;
        } cases[] = {
            {"Arash (\xd8\xa2\xd8\xb1\xd8\xb4)", "Arash"},
            {"\xd8\xb9\xd8\xa7\xd9\x84\xdb\x8c\xd8\xb4\xd8\xa7\xd9\x87", "Alishah"},
            {"\xef\xb7\xb4", "Mohammad"},
            {"\xef\xb7\xb4 Hosseini", "Mohammad Hosseini"},
            {"Sylwester Napiera\xc5\x82"
             "a",
             NULL},                                        /* a letter a font has */
            {"\xea\xb9\x80\xec\xb2\xa0\xec\x88\x98", NULL}, /* Hangul: bytes above 0xd8 too */
            {"\xd8\xb9\xd9\x84\xdb\x8c", NULL},             /* Arabic letters, none of the three names */
            {"", NULL},
        };
        char out[64];
        int wrong = 0;
        for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
            int n = re_font_name(cases[i].text, (unsigned)strlen(cases[i].text), out, sizeof out);
            wrong += cases[i].want == NULL ? n != 0 : n != (int)strlen(cases[i].want) || strcmp(out, cases[i].want) != 0;
        }
        wrong += re_font_name(cases[3].text, (unsigned)strlen(cases[3].text), out, 8) != -1; /* no room for it */

        static unsigned char any[RE_FONT_BMP_BYTES], one[RE_FONT_BMP_BYTES];
        static char list[1 << 20];
        int fonts = 0, lists = 0, lines = 0, changed = 0, left = 0;
        wchar_t file[MAX_PATH];
        wcsncpy(file, path, MAX_PATH - 1);
        file[MAX_PATH - 1] = 0;
        wchar_t *slash = wcsrchr(file, L'\\') ? wcsrchr(file, L'\\') : wcsrchr(file, L'/');
        WIN32_FIND_DATAW found;
        HANDLE find = INVALID_HANDLE_VALUE;
        if (slash != NULL && (size_t)(slash - file) + 40 < MAX_PATH) {
            wcscpy(slash + 1, L"fonts\\*");
            find = FindFirstFileW(file, &found);
        }
        if (find != INVALID_HANDLE_VALUE) {
            do {
                if ((size_t)(slash - file) + wcslen(found.cFileName) + 40 >= MAX_PATH)
                    continue;
                wcscpy(slash + 1, L"fonts\\");
                wcscat(file, found.cFileName);
                if (re_font_file(file, one) > 0) { /* a folder or another kind of file is not read */
                    fonts++;
                    for (int b = 0; b < RE_FONT_BMP_BYTES; b++)
                        any[b] |= one[b];
                }
            } while (FindNextFileW(find, &found));
            FindClose(find);
            wcscpy(slash + 1, L"extraData\\namesPeople\\*.txt");
            find = FindFirstFileW(file, &found);
        }
        if (find != INVALID_HANDLE_VALUE) {
            do {
                if ((size_t)(slash - file) + wcslen(found.cFileName) + 40 >= MAX_PATH)
                    continue;
                wcscpy(slash + 1, L"extraData\\namesPeople\\");
                wcscat(file, found.cFileName);
                FILE *f = _wfopen(file, L"rb");
                if (f == NULL)
                    continue;
                size_t size = fread(list, 1, sizeof list - 64, f);
                fclose(f);
                if (control && lists == 0) /* a line no list has */
                    size += (size_t)snprintf(list + size, 64, "\n%s\n", cases[6].text);
                lists++;
                for (size_t at = 0; at < size;) {
                    size_t end = at;
                    while (end < size && list[end] != '\n' && list[end] != '\r')
                        end++;
                    if (end > at) {
                        int n = end - at < sizeof out ? re_font_name(list + at, (unsigned)(end - at), out, sizeof out) : 0;
                        const unsigned char *name = (const unsigned char *)(n > 0 ? out : list + at);
                        size_t count = n > 0 ? (size_t)n : end - at, none = 0;
                        for (size_t i = 0; i < count;) { /* UTF-8: one code point at a time */
                            unsigned code = name[i], more = code < 0x80 ? 0 : code < 0xe0 ? 1 : code < 0xf0 ? 2 : 3;
                            code &= more == 0 ? 0x7f : 0x3fu >> more;
                            for (i++; more > 0 && i < count; more--, i++)
                                code = code << 6 | (name[i] & 0x3f);
                            none += code > 0x7e && (code > 0xffff || !((any[code >> 3] >> (code & 7)) & 1));
                        }
                        lines++;
                        changed += n > 0;
                        left += none > 0;
                    }
                    at = end + 1;
                }
            } while (FindNextFileW(find, &found));
            FindClose(find);
        }
        int ok = wrong == 0 && fonts > 0 && lists > 0 && changed == 3 && left == 0;
        g_failures += !ok;
        printf("  T26 a name no font can write                         cases_wrong=%d; %d list(s) of names, %d line(s), %d font file(s): "
               "%d line(s) put in Latin letters, %d line(s) still with a letter no font has %s\n",
               wrong, lists, lines, fonts, changed, left, ok ? "ok" : "FAIL");
    }

    printf("failures=%d control=%d verdict=%s\n", g_failures, control, g_failures == 0 ? "PASS" : "FAIL");
    return g_failures != 0;
}
