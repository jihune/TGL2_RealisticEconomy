/* Minimal patching helpers for a 32-bit process. Every patch checks the bytes it expects before writing. */
#ifndef RE_HOOK_H
#define RE_HOOK_H

#include <windows.h>

/* Replace the target of a 5-byte `call rel32` at `site`. The current target must be `expected_target`.
 * Returns 1 on success, 0 when the bytes differ or the page cannot be made writable. */
int re_patch_call(BYTE *site, const BYTE *expected_target, const void *new_target);

/* Replace the 4-byte absolute address an instruction reads from, at `at`. The current address must be `expected`. */
int re_patch_ptr(BYTE *at, const void *expected, const void *value);

/* Replace `count` bytes of an instruction at `at`. The current bytes must be `expected`. The replacement has to be
 * whole instructions of the same total length. */
int re_patch_bytes(BYTE *at, const BYTE *expected, const BYTE *replacement, SIZE_T count);

/* Redirect a function whose first five bytes are `expected5` to `hook`. On success `*trampoline` runs the
 * five displaced bytes and continues in the original. The five bytes must be position independent. */
int re_detour5(BYTE *fn, const BYTE expected5[5], const void *hook, void **trampoline);

/* Same for a function whose first whole instructions take `count` bytes (5..12): all of them are moved to the
 * trampoline and the bytes after the 5-byte jump become `nop`. */
int re_detour(BYTE *fn, const BYTE *expected, SIZE_T count, const void *hook, void **trampoline);

/* Read-only check used before touching anything: 1 when `count` bytes at `at` equal `expected`. */
int re_bytes_equal(const BYTE *at, const BYTE *expected, SIZE_T count);

/* 1 when `count` bytes at `p` are committed and readable. Used before dereferencing game pointers. */
int re_readable(const void *p, SIZE_T count);

#endif
