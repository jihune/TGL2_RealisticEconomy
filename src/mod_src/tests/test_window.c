/* A fixture for test runs, not a part of the mod (REQUEST.md [59], [61], note m41). mod_src/tools/game_run.ps1 puts it
 * beside the plugin as TestWindow.asi when it starts a run of the game copy and takes it away when the run stops;
 * the loader that loads the plugin loads it too. It is in no package.
 *
 * The game's window library (GLFW, inside libcocos2d.dll) creates the main window at Windows' default position on
 * the primary monitor, shows it, and asks for the foreground. From outside the process nothing gets in before the
 * window is on the screen: start-up information that names a position, a monitor or "hidden" is not honoured, and
 * a move from outside waits until the game answers messages. Measured with mod_src/tools/window_watch.py: the
 * window 0.8 s on the primary monitor; the taskbar up, because a refused request for the foreground makes the
 * taskbar button ask for attention; and in other starts the request granted, the keyboard gone to the game.
 * So it is done inside the process, by replacing four imports of libcocos2d.dll:
 *   - CreateWindowExW: the main window - class GLFW30, without a parent, larger than the library's helper window -
 *     is created at the position the environment variable TGL2_TEST_WINDOW names, "x,y" or "x,y,hidden";
 *   - ShowWindow, BringWindowToTop, SetForegroundWindow: the main window is shown without being activated and the
 *     two requests are answered with TRUE and not passed on;
 *   - the main window is never activated, whoever asks (a hook of its thread refuses), and Windows does not pick
 *     it when the user closes the window in front. So the game takes the keyboard from nobody, no taskbar button
 *     asks for attention, and no test breaks because the game gained and lost the keyboard in the middle of it.
 * With "hidden", for a window that is to be off every monitor:
 *   - the window gets an owner (a window of the fixture's that is never shown) and loses the style that forces a
 *     taskbar button: it has no taskbar button;
 *   - it keeps its position whoever moves it: Windows puts a window that is off every monitor back onto one when
 *     the monitors change.
 * The frame of the window stays what the library asked for, so the client area and every coordinate of a test are
 * as without the fixture. Without the variable nothing is replaced. TestWindow.log, beside the game, says what was
 * done. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static int g_x, g_y, g_hidden, g_created, g_refused;
static HWND g_owner, g_main;
static WNDPROC g_game_proc;

static void note(const char *format, ...)
{
    FILE *log = fopen("TestWindow.log", "a");
    va_list args;
    if (log == NULL)
        return;
    va_start(args, format);
    vfprintf(log, format, args);
    va_end(args);
    fputc('\n', log);
    fclose(log);
}

/* the main window stays where it was created */
static LRESULT CALLBACK pinned_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_WINDOWPOSCHANGING) {
        WINDOWPOS *pos = (WINDOWPOS *)lparam;
        if (!(pos->flags & SWP_NOMOVE)) {
            pos->x = g_x;
            pos->y = g_y;
        }
    }
    return CallWindowProcW(g_game_proc, hwnd, message, wparam, lparam);
}

/* the main window is not activated */
static LRESULT CALLBACK refuse_activation(int code, WPARAM wparam, LPARAM lparam)
{
    if (code == HCBT_ACTIVATE && g_main != NULL && (HWND)wparam == g_main) {
        if (g_refused++ == 0)
            note("the main window was to be activated: refused (the first time; later ones are not written)");
        return 1;
    }
    return CallNextHookEx(NULL, code, wparam, lparam);
}

static HWND WINAPI create_window(DWORD ex_style, LPCWSTR class_name, LPCWSTR title, DWORD style, int x, int y, int width, int height,
                                 HWND parent, HMENU menu, HINSTANCE instance, LPVOID param)
{
    int main_window = ((ULONG_PTR)class_name >> 16) != 0 && wcscmp(class_name, L"GLFW30") == 0 && parent == NULL && width > 64 && height > 64;
    if (!main_window)
        return CreateWindowExW(ex_style, class_name, title, style, x, y, width, height, parent, menu, instance, param);
    HHOOK hook = SetWindowsHookExW(WH_CBT, refuse_activation, NULL, GetCurrentThreadId());
    ex_style |= WS_EX_NOACTIVATE;
    if (g_hidden) {
        if (g_owner == NULL)
            g_owner = CreateWindowExW(WS_EX_TOOLWINDOW, L"STATIC", L"", WS_POPUP, 0, 0, 0, 0, NULL, NULL, instance, NULL);
        parent = g_owner;
        ex_style &= ~(DWORD)WS_EX_APPWINDOW;
    }
    g_main = CreateWindowExW(ex_style, class_name, title, style, g_x, g_y, width, height, parent, menu, instance, param);
    if (g_main != NULL && g_hidden)
        g_game_proc = (WNDPROC)SetWindowLongPtrW(g_main, GWLP_WNDPROC, (LONG_PTR)pinned_proc);
    note("the main window (%d of this process): asked for (%d,%d) %dx%d, created at (%d,%d)%s%s: %s", ++g_created, x, y, width, height, g_x, g_y,
         g_hidden ? g_owner != NULL ? ", owned by a window that is never shown, without the taskbar style" : ", the owner was NOT made" : "",
         hook == NULL ? ", the hook that refuses activation was NOT made" : g_hidden ? ", never activated, kept at its position" : ", never activated",
         g_main != NULL ? "done" : "NOT created");
    return g_main;
}

static BOOL WINAPI show_window(HWND hwnd, int how)
{
    if (hwnd == g_main && (how == SW_SHOW || how == SW_SHOWNORMAL || how == SW_RESTORE || how == SW_SHOWDEFAULT))
        how = SW_SHOWNA;
    return ShowWindow(hwnd, how);
}

static BOOL WINAPI not_passed_on(HWND hwnd)
{
    (void)hwnd;
    return TRUE;
}

/* The game's clock goes by frames: at its fastest speed an hour a frame. A frame ends in SwapBuffers, which waits for
 * the display's refresh, and with the display switched off (Windows does that after 90 minutes without a key or the
 * mouse here) the wait is about a second: a month end took more than ten minutes, and every test with one failed
 * (run1230 to run1237, note m41). So the wait is switched off the first time a frame is shown, and the frames are
 * held to 60 a second here, the pace of the display the tests were written on (without that the game makes about
 * 110, run1239, and its clock would run that much faster). TGL2_TEST_VSYNC=keep leaves both as they are (the
 * control). The frames of the first ten seconds are counted either way. */
static int g_vsync_keep;
static BOOL(WINAPI *g_swap)(HDC);

static BOOL WINAPI swap_buffers(HDC dc)
{
    typedef BOOL(WINAPI * interval_fn)(int);
    typedef PROC(WINAPI * address_fn)(LPCSTR);
    static DWORD first;
    static int frames, said;
    static LARGE_INTEGER rate, next;
    LARGE_INTEGER now;
    if (frames++ == 0) {
        HMODULE gl = GetModuleHandleW(L"opengl32.dll");
        address_fn address = gl != NULL ? (address_fn)(void *)GetProcAddress(gl, "wglGetProcAddress") : NULL;
        interval_fn interval = address != NULL ? (interval_fn)(void *)address("wglSwapIntervalEXT") : NULL;
        first = GetTickCount();
        note("the first frame: the wait for the display's refresh %s", g_vsync_keep        ? "is left as it is (TGL2_TEST_VSYNC=keep)"
                                                                      : interval == NULL  ? "could NOT be switched off (no wglSwapIntervalEXT)"
                                                                      : interval(0)       ? "is switched off"
                                                                                          : "could NOT be switched off (refused)");
    } else if (!said && GetTickCount() - first >= 10000) {
        said = 1;
        note("frames in the first %.1f seconds: %d", (GetTickCount() - first) / 1000.0, frames);
    }
    BOOL done = g_swap(dc);
    if (g_vsync_keep)
        return done;
    if (rate.QuadPart == 0) {
        QueryPerformanceFrequency(&rate);
        QueryPerformanceCounter(&next);
    }
    next.QuadPart += rate.QuadPart / 60;
    QueryPerformanceCounter(&now);
    if (now.QuadPart > next.QuadPart)
        next = now; /* a frame that took longer: the time is not made up */
    while (now.QuadPart < next.QuadPart) {
        Sleep((next.QuadPart - now.QuadPart) * 1000 / rate.QuadPart > 2 ? 1 : 0);
        QueryPerformanceCounter(&now);
    }
    return done;
}

/* The entries of a module's import table that hold `from` get `to`. Returns how many. */
static int replace_import(HMODULE module, HMODULE user, const char *name, void *to)
{
    BYTE *base = (BYTE *)module;
    IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)(base + ((IMAGE_DOS_HEADER *)base)->e_lfanew);
    IMAGE_DATA_DIRECTORY dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    void *from = (void *)GetProcAddress(user, name);
    int replaced = 0;
    if (dir.VirtualAddress == 0 || from == NULL)
        return 0;
    for (IMAGE_IMPORT_DESCRIPTOR *d = (IMAGE_IMPORT_DESCRIPTOR *)(base + dir.VirtualAddress); d->Name != 0; d++)
        for (void **slot = (void **)(base + d->FirstThunk); *slot != NULL; slot++) {
            DWORD old;
            if (*slot != from || !VirtualProtect(slot, sizeof *slot, PAGE_READWRITE, &old))
                continue;
            *slot = to;
            VirtualProtect(slot, sizeof *slot, old, &old);
            replaced++;
        }
    return replaced;
}

BOOL WINAPI DllMain(HINSTANCE self, DWORD reason, LPVOID reserved)
{
    char want[64], mode[16] = "";
    (void)self;
    (void)reserved;
    if (reason != DLL_PROCESS_ATTACH)
        return TRUE;
    DWORD length = GetEnvironmentVariableA("TGL2_TEST_WINDOW", want, sizeof want);
    if (length == 0 || length >= sizeof want || sscanf(want, "%d,%d,%15s", &g_x, &g_y, mode) < 2)
        return TRUE; /* no test run: nothing is replaced */
    g_hidden = strcmp(mode, "hidden") == 0;
    HMODULE library = GetModuleHandleW(L"libcocos2d.dll"), user = GetModuleHandleW(L"user32.dll");
    if (library == NULL || user == NULL) {
        note("TestWindow: TGL2_TEST_WINDOW=%s: libcocos2d.dll or user32.dll is NOT loaded yet, nothing replaced", want);
        return TRUE;
    }
    int created = replace_import(library, user, "CreateWindowExW", create_window), shown = replace_import(library, user, "ShowWindow", show_window);
    int top = replace_import(library, user, "BringWindowToTop", not_passed_on);
    int foreground = replace_import(library, user, "SetForegroundWindow", not_passed_on);
    char keep[8] = "";
    HMODULE gdi = GetModuleHandleW(L"gdi32.dll");
    g_vsync_keep = GetEnvironmentVariableA("TGL2_TEST_VSYNC", keep, sizeof keep) == 4 && strcmp(keep, "keep") == 0;
    g_swap = gdi != NULL ? (BOOL(WINAPI *)(HDC))(void *)GetProcAddress(gdi, "SwapBuffers") : NULL;
    int swapped = g_swap != NULL ? replace_import(library, gdi, "SwapBuffers", swap_buffers) : 0;
    note("TestWindow: TGL2_TEST_WINDOW=%s: imports of libcocos2d.dll replaced: CreateWindowExW %d, ShowWindow %d, BringWindowToTop %d, "
         "SetForegroundWindow %d%s, SwapBuffers %d",
         want, created, shown, top, foreground, created == 1 && shown == 1 && top == 1 && foreground == 1 ? "" : " (NOT one of each)",
         swapped);
    return TRUE;
}
