#include "re_util.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static HANDLE g_log = INVALID_HANDLE_VALUE;
static wchar_t g_ini[MAX_PATH], g_state[MAX_PATH];
static char g_language[16] = "en";

static void read_language(void)
{
    wchar_t path[MAX_PATH];
    DWORD n = GetModuleFileNameW(NULL, path, MAX_PATH);
    wchar_t *slash = n && n < MAX_PATH ? wcsrchr(path, L'\\') : NULL;
    if (slash == NULL || (slash - path) + 20 >= MAX_PATH)
        return;
    wcscpy(slash + 1, L"data\\options.xml");
    HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (file == INVALID_HANDLE_VALUE)
        return;
    char text[8192];
    DWORD got = 0;
    ReadFile(file, text, sizeof text - 1, &got, NULL);
    CloseHandle(file);
    text[got] = 0;
    const char *at = strstr(text, "<languageFolder>");
    if (at == NULL)
        return;
    at += 16;
    size_t len = strcspn(at, "<");
    if (len > 0 && len < sizeof g_language) {
        memcpy(g_language, at, len);
        g_language[len] = 0;
    }
}

void re_util_init(HINSTANCE self)
{
    wchar_t path[MAX_PATH];
    DWORD n = GetModuleFileNameW(self, path, MAX_PATH);
    if (n == 0 || n >= MAX_PATH - 6)
        return;
    wchar_t *dot = wcsrchr(path, L'.');
    if (dot == NULL)
        return;
    wcscpy(dot, L".ini");
    wcscpy(g_ini, path);
    wcscpy(dot, L".state");
    wcscpy(g_state, path);
    wcscpy(dot, L".log");
    g_log = CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    read_language();
}

const char *re_language(void)
{
    return g_language;
}

long long re_state_get(const char *section, const char *key, long long fallback)
{
    wchar_t ws[96], wk[64], wv[64];
    if (g_state[0] == 0 || section[0] == 0)
        return fallback;
    MultiByteToWideChar(CP_UTF8, 0, section, -1, ws, 96);
    MultiByteToWideChar(CP_UTF8, 0, key, -1, wk, 64);
    if (GetPrivateProfileStringW(ws, wk, L"", wv, 64, g_state) == 0)
        return fallback;
    return _wcstoi64(wv, NULL, 10);
}

void re_state_set(const char *section, const char *key, long long value)
{
    wchar_t ws[96], wk[64], wv[64];
    if (g_state[0] == 0 || section[0] == 0)
        return;
    MultiByteToWideChar(CP_UTF8, 0, section, -1, ws, 96);
    MultiByteToWideChar(CP_UTF8, 0, key, -1, wk, 64);
    _snwprintf(wv, 64, L"%I64d", value);
    WritePrivateProfileStringW(ws, wk, wv, g_state);
}

void re_log(const char *fmt, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof buf - 2, fmt, ap);
    va_end(ap);
    if (g_log == INVALID_HANDLE_VALUE || n < 0)
        return;
    if (n > (int)sizeof buf - 2)
        n = sizeof buf - 2;
    buf[n++] = '\n';
    DWORD written;
    /* One write a line and no buffer of the plugin's own: the line is the system's from here on, and a process that
     * is killed (a test run's end) or that crashes loses none of it. No FlushFileBuffers: that waits for the disk
     * after every line and keeps nothing those two cases need. Where it was measured it cost little (note m30:
     * the plugin's part of a load 148 ms with it and 69 without, of a month start 73 and 64). */
    WriteFile(g_log, buf, (DWORD)n, &written, NULL);
}

static int ini_text(const char *section, const char *key, char *out, int size)
{
    wchar_t ws[64], wk[64], wv[512];
    out[0] = 0;
    if (g_ini[0] == 0)
        return 0;
    MultiByteToWideChar(CP_UTF8, 0, section, -1, ws, 64);
    MultiByteToWideChar(CP_UTF8, 0, key, -1, wk, 64);
    if (GetPrivateProfileStringW(ws, wk, L"", wv, 512, g_ini) == 0)
        return 0;
    return WideCharToMultiByte(CP_UTF8, 0, wv, -1, out, size, NULL, NULL) > 1;
}

int re_ini_int(const char *section, const char *key, int fallback)
{
    char text[64];
    return ini_text(section, key, text, sizeof text) ? atoi(text) : fallback;
}

void re_ini_set_int(const char *section, const char *key, int value)
{
    wchar_t ws[64], wk[64], wv[32];
    if (g_ini[0] == 0)
        return;
    MultiByteToWideChar(CP_UTF8, 0, section, -1, ws, 64);
    MultiByteToWideChar(CP_UTF8, 0, key, -1, wk, 64);
    _snwprintf(wv, 32, L"%d", value);
    WritePrivateProfileStringW(ws, wk, wv, g_ini);
}

void re_ini_doubles(const char *section, const char *key, double *out, int count)
{
    char text[512];
    if (!ini_text(section, key, text, sizeof text))
        return;
    char *at = text;
    for (int i = 0; i < count && *at; i++) {
        char *end;
        double v = strtod(at, &end);
        if (end == at)
            break;
        out[i] = v;
        at = *end == ',' ? end + 1 : end;
    }
}

int re_ini_ints(const char *section, const char *key, const char *fallback, int *out, int max)
{
    char text[512];
    if (!ini_text(section, key, text, sizeof text)) {
        strncpy(text, fallback, sizeof text - 1);
        text[sizeof text - 1] = 0;
    }
    int n = 0;
    char *at = text;
    while (n < max && *at) {
        char *end;
        long v = strtol(at, &end, 10);
        if (end == at)
            break;
        out[n++] = (int)v;
        at = *end == ',' ? end + 1 : end;
    }
    return n;
}

static int data_walk(wchar_t *path, size_t at, int depth, re_data_each each, void *ctx)
{
    WIN32_FIND_DATAW found;
    int files = 0;
    if (at + 3 >= MAX_PATH)
        return 0;
    wcscpy(path + at, L"\\*");
    HANDLE find = FindFirstFileW(path, &found);
    if (find == INVALID_HANDLE_VALUE)
        return 0;
    do {
        size_t n = wcslen(found.cFileName);
        if (found.cFileName[0] == L'.' || at + 1 + n + 3 >= MAX_PATH)
            continue;
        path[at] = L'\\';
        wcscpy(path + at + 1, found.cFileName);
        if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (depth < 4)
                files += data_walk(path, at + 1 + n, depth + 1, each, ctx);
        } else if (n > 4 && _wcsicmp(found.cFileName + n - 4, L".xml") == 0 && found.nFileSizeHigh == 0 && found.nFileSizeLow < (8u << 20)) {
            HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
            if (file == INVALID_HANDLE_VALUE)
                continue;
            char *text = (char *)malloc(found.nFileSizeLow + 1);
            DWORD got = 0;
            if (text != NULL && ReadFile(file, text, found.nFileSizeLow, &got, NULL)) {
                text[got] = 0;
                each(text, got, ctx);
                files++;
            }
            free(text);
            CloseHandle(file);
        }
    } while (FindNextFileW(find, &found));
    FindClose(find);
    return files;
}

int re_data_files(re_data_each each, void *ctx)
{
    wchar_t path[MAX_PATH];
    DWORD n = GetModuleFileNameW(NULL, path, MAX_PATH);
    wchar_t *slash = n && n < MAX_PATH ? wcsrchr(path, L'\\') : NULL;
    if (slash == NULL || (slash - path) + 8 >= MAX_PATH)
        return -1;
    wcscpy(slash + 1, L"data");
    return data_walk(path, (size_t)(slash + 5 - path), 0, each, ctx);
}
