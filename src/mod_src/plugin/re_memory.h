/* The memory the game loses at every load (analysis/notes/m20_load_leak.md).
 *
 * The game reads its data files, about 250 XML files, again at every start of a game, a load included. The object
 * that reads them (the loader) keeps every document in one of 16 lists. Its destructor (TGL2.exe 0x0080b560) copies
 * each list and empties the copy without deleting a document, so every load leaves the documents of the game before
 * it behind. The game is a 32-bit program with 2 GB of address space and ends with an access violation once that is
 * used up.
 *
 * The detour at the destructor's entry deletes the documents with the game's own function and empties the lists;
 * the destructor then finds nothing in them.
 */
#ifndef RE_MEMORY_H
#define RE_MEMORY_H

#include <windows.h>

typedef void (*re_memory_end_fn)(BYTE *loader);

extern void *re_memory_end_trampoline;   /* filled by re_detour5 */
extern re_memory_end_fn re_memory_ended; /* called with the loader before its destructor runs; NULL = nothing */

void re_memory_end_hook(void); /* entry for the detour; not callable from C */

/* `list` is a std::vector of pointers (first, end, end of storage). Every element that points at an object whose
 * table of virtual functions is `table` is deleted through the entry at `slot` bytes in that table (thiscall, one
 * argument: 1 = free the memory too), and the list is left empty with its storage. Other elements are left alone.
 * Returns how many were deleted, -1 for something that is not a list (nothing is touched then). */
int re_memory_release(BYTE *list, const void *table, unsigned slot);

#endif
