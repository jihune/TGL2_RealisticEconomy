#ifndef RE_ASM_H
#define RE_ASM_H

/* Pieces for the hooks written in assembly (Intel syntax). The game is built with whole-program optimisation: a
 * caller may keep a value in a register that the usual convention lets a callee change, so a hook that runs code
 * of its own between two instructions of the game puts every register back, the eight SSE ones included. */
#define SAVE_XMM                                                                                                       \
    "  sub esp, 128\n"                                                                                                 \
    "  movdqu [esp], xmm0\n"                                                                                           \
    "  movdqu [esp + 16], xmm1\n"                                                                                      \
    "  movdqu [esp + 32], xmm2\n"                                                                                      \
    "  movdqu [esp + 48], xmm3\n"                                                                                      \
    "  movdqu [esp + 64], xmm4\n"                                                                                      \
    "  movdqu [esp + 80], xmm5\n"                                                                                      \
    "  movdqu [esp + 96], xmm6\n"                                                                                      \
    "  movdqu [esp + 112], xmm7\n"
#define RESTORE_XMM                                                                                                    \
    "  movdqu xmm0, [esp]\n"                                                                                           \
    "  movdqu xmm1, [esp + 16]\n"                                                                                      \
    "  movdqu xmm2, [esp + 32]\n"                                                                                      \
    "  movdqu xmm3, [esp + 48]\n"                                                                                      \
    "  movdqu xmm4, [esp + 64]\n"                                                                                      \
    "  movdqu xmm5, [esp + 80]\n"                                                                                      \
    "  movdqu xmm6, [esp + 96]\n"                                                                                      \
    "  movdqu xmm7, [esp + 112]\n"                                                                                     \
    "  add esp, 128\n"

#endif
