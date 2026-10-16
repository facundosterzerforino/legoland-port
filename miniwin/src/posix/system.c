/* [library:system] Memory, time, strings, INI files, system information, the console and version information off
 * Windows. See posix.h. */
#include <ctype.h>
#include <errno.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#include "posix.h"

/* ---- memory: GMEM_MOVEABLE blocks don't move, so a handle is the block itself ---- */

HGLOBAL GlobalAlloc(UINT flags, SIZE_T size) {
    void *p = (flags & GMEM_ZEROINIT) != 0 ? calloc(1, size > 0 ? size : 1) : malloc(size > 0 ? size : 1);

    if (p == NULL) {
        PosixSetError(ERROR_NOT_ENOUGH_MEMORY);
    }
    return (HGLOBAL)p;
}

LPVOID GlobalLock(HGLOBAL handle) {
    return (LPVOID)handle;
}

BOOL GlobalUnlock(HGLOBAL handle) {
    (void)handle;
    PosixSetError(ERROR_SUCCESS);
    return FALSE; /* Windows: no lock left */
}

HGLOBAL GlobalFree(HGLOBAL handle) {
    free(handle);
    return NULL;
}

HLOCAL LocalAlloc(UINT flags, SIZE_T size) {
    return (HLOCAL)GlobalAlloc((flags & LMEM_ZEROINIT) != 0 ? GMEM_ZEROINIT : 0, size);
}

HLOCAL LocalFree(HLOCAL handle) {
    free(handle);
    return NULL;
}

/* readable: the kernel copies a byte from it without faulting */
BOOL IsBadReadPtr(const void *address, UINT_PTR size) {
    char byte;
    struct iovec local = {&byte, 1};
    struct iovec remote = {(void *)address, 1};

    if (size == 0) {
        return FALSE;
    }
    if (address == NULL || process_vm_readv(getpid(), &local, 1, &remote, 1, 0) != 1) {
        return TRUE;
    }
    remote.iov_base = (char *)address + size - 1;
    return process_vm_readv(getpid(), &local, 1, &remote, 1, 0) != 1;
}

SIZE_T VirtualQuery(LPCVOID address, PMEMORY_BASIC_INFORMATION info, SIZE_T size) {
    (void)address;
    (void)info;
    (void)size;
    PosixSetError(ERROR_NOT_SUPPORTED); /* only the Windows crash report walks memory */
    return 0;
}

void GlobalMemoryStatus(LPMEMORYSTATUS status) {
    struct sysinfo si;
    unsigned long long total = 0;
    unsigned long long avail = 0;

    if (sysinfo(&si) == 0) {
        total = (unsigned long long)si.totalram * si.mem_unit;
        avail = (unsigned long long)si.freeram * si.mem_unit;
    }
    memset(status, 0, sizeof(*status));
    status->dwLength = sizeof(*status);
    status->dwMemoryLoad = total > 0 ? (DWORD)(100 - avail * 100 / total) : 0;
    status->dwTotalPhys = (SIZE_T)total;
    status->dwAvailPhys = (SIZE_T)avail;
    status->dwTotalPageFile = (SIZE_T)total;
    status->dwAvailPageFile = (SIZE_T)avail;
    status->dwTotalVirtual = (SIZE_T)0x7ffe0000;
    status->dwAvailVirtual = (SIZE_T)0x7ffe0000;
}

void GetSystemInfo(LPSYSTEM_INFO info) {
    memset(info, 0, sizeof(*info));
    info->wProcessorArchitecture = PROCESSOR_ARCHITECTURE_AMD64;
    info->dwPageSize = (DWORD)sysconf(_SC_PAGESIZE);
    info->lpMinimumApplicationAddress = (LPVOID)0x10000;
    info->lpMaximumApplicationAddress = (LPVOID)0x7ffffffeffffULL;
    info->dwNumberOfProcessors = (DWORD)sysconf(_SC_NPROCESSORS_ONLN);
    info->dwAllocationGranularity = 0x10000;
}

/* ---- time ---- */

DWORD GetTickCount(void) {
    return (DWORD)PosixMilliseconds();
}

void Sleep(DWORD milliseconds) {
    struct timespec ts;

    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (long)(milliseconds % 1000) * 1000000;
    while (nanosleep(&ts, &ts) != 0 && errno == EINTR) {
    }
}

void GetSystemTimeAsFileTime(LPFILETIME out) {
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    PosixFileTime(ts.tv_sec, ts.tv_nsec, out);
}

static long long Seconds(const FILETIME *t) {
    unsigned long long v = ((unsigned long long)t->dwHighDateTime << 32) | t->dwLowDateTime;
    return (long long)(v / 10000000ULL) - 11644473600LL;
}

BOOL FileTimeToLocalFileTime(const FILETIME *utc, LPFILETIME local) {
    unsigned long long v = ((unsigned long long)utc->dwHighDateTime << 32) | utc->dwLowDateTime;
    time_t s = (time_t)Seconds(utc);
    struct tm tm;

    if (localtime_r(&s, &tm) == NULL) {
        return FALSE;
    }
    v += (unsigned long long)((long long)tm.tm_gmtoff * 10000000LL);
    local->dwLowDateTime = (DWORD)v;
    local->dwHighDateTime = (DWORD)(v >> 32);
    return TRUE;
}

BOOL FileTimeToDosDateTime(const FILETIME *t, LPWORD date, LPWORD time_out) {
    time_t s = (time_t)Seconds(t);
    struct tm tm;

    if (gmtime_r(&s, &tm) == NULL || tm.tm_year < 80 || tm.tm_year > 207) {
        PosixSetError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    *date = (WORD)(((tm.tm_year - 80) << 9) | ((tm.tm_mon + 1) << 5) | tm.tm_mday);
    *time_out = (WORD)((tm.tm_hour << 11) | (tm.tm_min << 5) | (tm.tm_sec / 2));
    return TRUE;
}

/* ---- strings ---- */

LPSTR lstrcpyA(LPSTR dst, LPCSTR src) {
    return strcpy(dst, src);
}

int lstrlenA(LPCSTR s) {
    return s != NULL ? (int)strlen(s) : 0;
}

int wvsprintfA(LPSTR out, LPCSTR format, va_list args) {
    /* wsprintf writes at most 1024 bytes */
    int n = vsnprintf(out, 1024, format, args);
    return n < 1024 ? n : 1023;
}

int wsprintfA(LPSTR out, LPCSTR format, ...) {
    va_list args;
    int n;

    va_start(args, format);
    n = wvsprintfA(out, format, args);
    va_end(args);
    return n;
}

int MulDiv(int a, int b, int c) {
    long long p = (long long)a * b;
    long long q;

    if (c == 0) {
        return -1;
    }
    /* rounded half away from zero, as Windows */
    if ((p < 0) != (c < 0)) {
        q = (p - c / 2) / c;
    } else {
        q = (p + c / 2) / c;
    }
    return q > 0x7fffffffLL || q < -0x7fffffffLL - 1 ? -1 : (int)q;
}

/* the ANSI code page is Windows-1252 here (the game's own text never goes through these) */
static const unsigned short cp1252[32] = {0x20ac, 0x81, 0x201a, 0x192, 0x201e, 0x2026, 0x2020, 0x2021, 0x2c6, 0x2030,
    0x160, 0x2039, 0x152, 0x8d, 0x17d, 0x8f, 0x90, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x2dc,
    0x2122, 0x161, 0x203a, 0x153, 0x9d, 0x17e, 0x178};

static int DecodeUtf8(const unsigned char *s, int len, int *used) {
    int c = s[0];
    int n = c < 0x80 ? 0 : c < 0xe0 ? 1
        : c < 0xf0                  ? 2
                                    : 3;
    int i;

    if (c >= 0x80 && c < 0xc0) {
        *used = 1;
        return 0xfffd;
    }
    if (n >= len) {
        *used = len;
        return 0xfffd;
    }
    c &= n == 0 ? 0x7f : n == 1 ? 0x1f
        : n == 2                ? 0x0f
                                : 0x07;
    for (i = 1; i <= n; i++) {
        if ((s[i] & 0xc0) != 0x80) {
            *used = i;
            return 0xfffd;
        }
        c = (c << 6) | (s[i] & 0x3f);
    }
    *used = n + 1;
    return c;
}

int MultiByteToWideChar(UINT code_page, DWORD flags, LPCCH src, int src_len, LPWSTR dst, int dst_len) {
    const unsigned char *s = (const unsigned char *)src;
    int len = src_len < 0 ? (int)strlen(src) + 1 : src_len;
    int i = 0;
    int out = 0;

    (void)flags;
    while (i < len) {
        int c;
        int used = 1;

        if (code_page == CP_UTF8) {
            c = DecodeUtf8(s + i, len - i, &used);
        } else {
            c = s[i] >= 0x80 && s[i] < 0xa0 ? cp1252[s[i] - 0x80] : s[i];
        }
        i += used;
        if (c > 0xffff) { /* a surrogate pair */
            if (dst_len > 0) {
                if (out + 2 > dst_len) {
                    PosixSetError(ERROR_INSUFFICIENT_BUFFER);
                    return 0;
                }
                dst[out] = (WCHAR)(0xd800 + ((c - 0x10000) >> 10));
                dst[out + 1] = (WCHAR)(0xdc00 + ((c - 0x10000) & 0x3ff));
            }
            out += 2;
            continue;
        }
        if (dst_len > 0) {
            if (out >= dst_len) {
                PosixSetError(ERROR_INSUFFICIENT_BUFFER);
                return 0;
            }
            dst[out] = (WCHAR)c;
        }
        out++;
    }
    return out;
}

int WideCharToMultiByte(UINT code_page, DWORD flags, LPCWCH src, int src_len, LPSTR dst, int dst_len,
    LPCCH default_char, LPBOOL used_default) {
    int len = src_len;
    int i;
    int out = 0;

    (void)flags;
    if (len < 0) {
        for (len = 0; src[len] != 0; len++) {
        }
        len++;
    }
    if (used_default != NULL) {
        *used_default = FALSE;
    }
    for (i = 0; i < len; i++) {
        unsigned char bytes[4];
        int n = 1;
        unsigned int c = src[i];

        if (code_page == CP_UTF8) {
            if (c >= 0xd800 && c < 0xdc00 && i + 1 < len && src[i + 1] >= 0xdc00 && src[i + 1] < 0xe000) {
                c = 0x10000 + ((c - 0xd800) << 10) + (src[i + 1] - 0xdc00);
                i++;
            }
            if (c < 0x80) {
                bytes[0] = (unsigned char)c;
            } else if (c < 0x800) {
                bytes[0] = (unsigned char)(0xc0 | (c >> 6));
                bytes[1] = (unsigned char)(0x80 | (c & 0x3f));
                n = 2;
            } else if (c < 0x10000) {
                bytes[0] = (unsigned char)(0xe0 | (c >> 12));
                bytes[1] = (unsigned char)(0x80 | ((c >> 6) & 0x3f));
                bytes[2] = (unsigned char)(0x80 | (c & 0x3f));
                n = 3;
            } else {
                bytes[0] = (unsigned char)(0xf0 | (c >> 18));
                bytes[1] = (unsigned char)(0x80 | ((c >> 12) & 0x3f));
                bytes[2] = (unsigned char)(0x80 | ((c >> 6) & 0x3f));
                bytes[3] = (unsigned char)(0x80 | (c & 0x3f));
                n = 4;
            }
        } else {
            int k;

            bytes[0] = (unsigned char)c;
            if (c >= 0x100 || (c >= 0x80 && c < 0xa0)) {
                for (k = 0; k < 32 && cp1252[k] != c; k++) {
                }
                if (k < 32) {
                    bytes[0] = (unsigned char)(0x80 + k);
                } else {
                    bytes[0] = default_char != NULL ? (unsigned char)*default_char : '?';
                    if (used_default != NULL) {
                        *used_default = TRUE;
                    }
                }
            }
        }
        if (dst_len > 0) {
            if (out + n > dst_len) {
                PosixSetError(ERROR_INSUFFICIENT_BUFFER);
                return 0;
            }
            memcpy(dst + out, bytes, (size_t)n);
        }
        out += n;
    }
    return out;
}

/* ---- INI files ---- */

struct Ini {
    char **lines;
    int count;
};

static void IniLoad(const char *file, struct Ini *ini) {
    char path[MAX_PATH];
    FILE *f = fopen(PosixPath(file, path, sizeof(path)), "rb");
    char line[4096];

    ini->lines = NULL;
    ini->count = 0;
    if (f == NULL) {
        return;
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        size_t n = strlen(line);
        while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r')) {
            line[--n] = '\0';
        }
        ini->lines = (char **)realloc(ini->lines, sizeof(char *) * (size_t)(ini->count + 1));
        ini->lines[ini->count++] = strdup(line);
    }
    fclose(f);
}

static void IniFree(struct Ini *ini) {
    int i;

    for (i = 0; i < ini->count; i++) {
        free(ini->lines[i]);
    }
    free(ini->lines);
}

static char *Trim(char *s) {
    char *e;

    while (*s == ' ' || *s == '\t') {
        s++;
    }
    e = s + strlen(s);
    while (e > s && (e[-1] == ' ' || e[-1] == '\t')) {
        *--e = '\0';
    }
    return s;
}

/* the section's name if line starts one ("[name]"), into out */
static int SectionName(const char *line, char *out, size_t size) {
    char copy[4096];
    char *s;
    char *end;

    snprintf(copy, sizeof(copy), "%s", line);
    s = Trim(copy);
    if (*s != '[' || (end = strchr(s, ']')) == NULL) {
        return 0;
    }
    *end = '\0';
    snprintf(out, size, "%s", Trim(s + 1));
    return 1;
}

/* the key and value of a "key=value" line */
static int KeyValue(const char *line, char *key, size_t key_size, char *value, size_t value_size) {
    char copy[4096];
    char *eq;
    char *v;
    size_t n;

    snprintf(copy, sizeof(copy), "%s", line);
    if ((eq = strchr(copy, '=')) == NULL) {
        return 0;
    }
    *eq = '\0';
    snprintf(key, key_size, "%s", Trim(copy));
    v = Trim(eq + 1);
    n = strlen(v);
    if (n >= 2 && (v[0] == '"' || v[0] == '\'') && v[n - 1] == v[0]) {
        v[n - 1] = '\0';
        v++;
    }
    snprintf(value, value_size, "%s", v);
    return key[0] != '\0';
}

/* copies a list of names, each NUL-terminated, with a final NUL, into out as Windows does */
static DWORD CopyList(char *list, size_t len, LPSTR out, DWORD size) {
    if (size < 2) {
        if (size == 1) {
            out[0] = '\0';
        }
        return 0;
    }
    if (len + 1 > size) {
        memcpy(out, list, size - 2);
        out[size - 2] = '\0';
        out[size - 1] = '\0';
        return size - 2;
    }
    memcpy(out, list, len);
    out[len] = '\0';
    return (DWORD)len;
}

DWORD GetPrivateProfileStringA(LPCSTR section, LPCSTR key, LPCSTR default_value, LPSTR out, DWORD size,
    LPCSTR file) {
    struct Ini ini;
    char current[512] = "";
    char k[512];
    char v[4096];
    char list[8192];
    size_t list_len = 0;
    int in_section = 0;
    int i;
    const char *found = NULL;
    DWORD n;

    IniLoad(file, &ini);
    for (i = 0; i < ini.count; i++) {
        if (SectionName(ini.lines[i], current, sizeof(current))) {
            in_section = section != NULL && strcasecmp(current, section) == 0;
            if (section == NULL && list_len + strlen(current) + 2 < sizeof(list)) {
                strcpy(list + list_len, current);
                list_len += strlen(current) + 1;
            }
            continue;
        }
        if (!in_section || !KeyValue(ini.lines[i], k, sizeof(k), v, sizeof(v))) {
            continue;
        }
        if (key == NULL) {
            if (list_len + strlen(k) + 2 < sizeof(list)) {
                strcpy(list + list_len, k);
                list_len += strlen(k) + 1;
            }
        } else if (found == NULL && strcasecmp(k, key) == 0) {
            found = v;
            break;
        }
    }
    IniFree(&ini);
    if (section == NULL || key == NULL) {
        return CopyList(list, list_len, out, size);
    }
    if (size == 0) {
        return 0;
    }
    if (found == NULL) {
        char d[4096];
        snprintf(d, sizeof(d), "%s", default_value != NULL ? default_value : "");
        snprintf(v, sizeof(v), "%s", Trim(d));
        found = v;
    }
    n = (DWORD)strlen(found);
    if (n >= size) {
        n = size - 1;
    }
    memmove(out, found, n);
    out[n] = '\0';
    return n;
}

UINT GetPrivateProfileIntA(LPCSTR section, LPCSTR key, INT default_value, LPCSTR file) {
    char value[64];

    if (GetPrivateProfileStringA(section, key, "", value, sizeof(value), file) == 0) {
        return (UINT)default_value;
    }
    return (UINT)strtol(value, NULL, 10);
}

BOOL WritePrivateProfileStringA(LPCSTR section, LPCSTR key, LPCSTR value, LPCSTR file) {
    struct Ini ini;
    char current[512];
    char k[512];
    char v[4096];
    char path[MAX_PATH];
    int start = -1; /* the section's header line */
    int end = -1; /* the line after its last key */
    int at = -1; /* the key's line */
    int i;
    FILE *f;

    if (section == NULL) {
        PosixSetError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    IniLoad(file, &ini);
    for (i = 0; i < ini.count; i++) {
        if (SectionName(ini.lines[i], current, sizeof(current))) {
            if (start >= 0) {
                break;
            }
            if (strcasecmp(current, section) == 0) {
                start = i;
                end = i + 1;
            }
            continue;
        }
        if (start >= 0) {
            if (KeyValue(ini.lines[i], k, sizeof(k), v, sizeof(v))) {
                end = i + 1;
                if (key != NULL && strcasecmp(k, key) == 0 && at < 0) {
                    at = i;
                }
            }
        }
    }
    f = fopen(PosixPath(file, path, sizeof(path)), "wb");
    if (f == NULL) {
        IniFree(&ini);
        PosixSetError(PosixErrorFromErrno(errno));
        return FALSE;
    }
    for (i = 0; i < ini.count; i++) {
        if (start >= 0 && key == NULL && i >= start && i < end) {
            continue; /* the section goes */
        }
        if (i == at) {
            if (value != NULL) {
                fprintf(f, "%s=%s\r\n", key, value);
            }
            continue;
        }
        fprintf(f, "%s\r\n", ini.lines[i]);
        if (i == end - 1 && at < 0 && key != NULL && value != NULL) {
            fprintf(f, "%s=%s\r\n", key, value);
        }
    }
    if (start < 0 && key != NULL && value != NULL) {
        fprintf(f, "[%s]\r\n%s=%s\r\n", section, key, value);
    }
    fclose(f);
    IniFree(&ini);
    return TRUE;
}

/* ---- the system ---- */

BOOL GetComputerNameA(LPSTR out, LPDWORD size) {
    char name[256];

    if (gethostname(name, sizeof(name)) != 0) {
        name[0] = '\0';
    }
    name[sizeof(name) - 1] = '\0';
    if (strlen(name) + 1 > *size) {
        *size = (DWORD)strlen(name) + 1;
        PosixSetError(ERROR_BUFFER_OVERFLOW);
        return FALSE;
    }
    strcpy(out, name);
    *size = (DWORD)strlen(name);
    return TRUE;
}

BOOL GetUserNameA(LPSTR out, LPDWORD size) {
    const char *name = getenv("USER");

    if (name == NULL) {
        name = "user";
    }
    if (strlen(name) + 1 > *size) {
        *size = (DWORD)strlen(name) + 1;
        PosixSetError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    strcpy(out, name);
    *size = (DWORD)strlen(name) + 1;
    return TRUE;
}

void OutputDebugStringA(LPCSTR text) {
    (void)text; /* for a debugger on Windows; the port's trace file has what matters */
}

UINT GetSystemWindowsDirectoryA(LPSTR out, UINT size) {
    (void)out;
    (void)size;
    PosixSetError(ERROR_PATH_NOT_FOUND); /* no Windows folder (nor its gm.dls) */
    return 0;
}

HMODULE LoadLibraryA(LPCSTR name) {
    (void)name;
    PosixSetError(ERROR_MOD_NOT_FOUND);
    return NULL;
}

HMODULE LoadLibraryExA(LPCSTR name, HANDLE file, DWORD flags) {
    (void)file;
    (void)flags;
    return LoadLibraryA(name);
}

BOOL FreeLibrary(HMODULE module) {
    (void)module;
    return TRUE;
}

BOOL SetThreadStackGuarantee(PULONG size) {
    (void)size;
    return TRUE;
}

BOOL SetProcessDPIAware(void) {
    return TRUE;
}

/* ---- the console: the terminal the program runs in ---- */

static PHANDLER_ROUTINE ctrl_handler;

BOOL AllocConsole(void) {
    return TRUE;
}

BOOL SetConsoleTitleA(LPCSTR title) {
    (void)title;
    return TRUE;
}

BOOL SetConsoleCtrlHandler(PHANDLER_ROUTINE handler, BOOL add) {
    ctrl_handler = add ? handler : NULL;
    return TRUE;
}

BOOL ReadConsoleA(HANDLE input, LPVOID buffer, DWORD size, LPDWORD read_size, PCONSOLE_READCONSOLE_CONTROL control) {
    ssize_t n;

    (void)input;
    (void)control;
    do {
        n = read(0, buffer, size);
    } while (n < 0 && errno == EINTR);
    if (n < 0) {
        PosixSetError(PosixErrorFromErrno(errno));
        return FALSE;
    }
    *read_size = (DWORD)n;
    return TRUE;
}

BOOL WriteConsoleA(HANDLE output, const void *buffer, DWORD size, LPDWORD written, LPVOID reserved) {
    ssize_t n;

    (void)output;
    (void)reserved;
    n = write(1, buffer, size);
    if (written != NULL) {
        *written = n > 0 ? (DWORD)n : 0;
    }
    return n >= 0;
}

/* ---- version information: read from the named PE file's RT_VERSION resource, as Windows does ---- */

static unsigned int U32(const unsigned char *p) {
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((unsigned int)p[3] << 24);
}

static unsigned int U16(const unsigned char *p) {
    return p[0] | (p[1] << 8);
}

/* the version resource's bytes, malloc'd, or NULL */
static unsigned char *ReadVersionResource(const char *file, unsigned int *size) {
    char path[MAX_PATH];
    FILE *f = fopen(PosixPath(file, path, sizeof(path)), "rb");
    unsigned char *image = NULL;
    unsigned char *result = NULL;
    long length;

    if (f == NULL) {
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) == 0 && (length = ftell(f)) > 0x40 && fseek(f, 0, SEEK_SET) == 0 &&
        (image = (unsigned char *)malloc((size_t)length)) != NULL && fread(image, 1, (size_t)length, f) == (size_t)length) {
        for (;;) {
            unsigned int pe = U32(image + 0x3c);
            unsigned int sections, opt_size, opt, rsrc_rva, s, i, level, dir, entry;
            unsigned int rsrc_raw = 0;
            unsigned int rsrc_va = 0;

            if (pe + 0x18 > (unsigned int)length || memcmp(image + pe, "PE\0\0", 4) != 0) {
                break;
            }
            sections = U16(image + pe + 6);
            opt_size = U16(image + pe + 0x14);
            opt = pe + 0x18;
            /* the data directories: after 0x60 (PE32) or 0x70 (PE32+) bytes; the resources are the third */
            rsrc_rva = U32(image + opt + (U16(image + opt) == 0x20b ? 0x70 : 0x60) + 2 * 8);
            for (s = 0; s < sections; s++) {
                const unsigned char *h = image + opt + opt_size + s * 40;
                unsigned int va = U32(h + 12);
                unsigned int vsize = U32(h + 8) > U32(h + 16) ? U32(h + 8) : U32(h + 16);
                if (rsrc_rva >= va && rsrc_rva < va + vsize) {
                    rsrc_va = va;
                    rsrc_raw = U32(h + 20);
                }
            }
            if (rsrc_va == 0) {
                break;
            }
            /* type 16 (RT_VERSION), then its first name and its first language */
            dir = rsrc_raw + (rsrc_rva - rsrc_va);
            for (level = 0; level < 3; level++) {
                unsigned int named = U16(image + dir + 12);
                unsigned int ids = U16(image + dir + 14);
                entry = 0;
                for (i = 0; i < named + ids; i++) {
                    const unsigned char *e = image + dir + 16 + i * 8;
                    if (level > 0 || U32(e) == 16 /* RT_VERSION */) {
                        entry = U32(e + 4);
                        break;
                    }
                }
                if (entry == 0) {
                    break;
                }
                if (level < 2) {
                    if ((entry & 0x80000000u) == 0) {
                        entry = 0;
                        break;
                    }
                    dir = rsrc_raw + (rsrc_rva - rsrc_va) + (entry & 0x7fffffffu);
                }
            }
            if (entry == 0 || (entry & 0x80000000u) != 0) {
                break;
            }
            {
                const unsigned char *data = image + rsrc_raw + (rsrc_rva - rsrc_va) + entry;
                unsigned int at = rsrc_raw + (U32(data) - rsrc_va);
                *size = U32(data + 4);
                if (at + *size <= (unsigned int)length && (result = (unsigned char *)malloc(*size)) != NULL) {
                    memcpy(result, image + at, *size);
                }
            }
            break;
        }
    }
    free(image);
    fclose(f);
    return result;
}

/* the buffer: the resource, then room for VerQueryValueA's ANSI copies of its strings */
DWORD GetFileVersionInfoSizeA(LPCSTR file, LPDWORD handle) {
    unsigned int size = 0;
    unsigned char *data = ReadVersionResource(file, &size);

    if (handle != NULL) {
        *handle = 0;
    }
    if (data == NULL) {
        PosixSetError(ERROR_RESOURCE_TYPE_NOT_FOUND);
        return 0;
    }
    free(data);
    return size * 2 + 4;
}

BOOL GetFileVersionInfoA(LPCSTR file, DWORD handle, DWORD size, LPVOID out) {
    unsigned int resource_size = 0;
    unsigned char *data = ReadVersionResource(file, &resource_size);

    (void)handle;
    if (data == NULL || size < resource_size * 2 + 4) {
        free(data);
        PosixSetError(data == NULL ? ERROR_RESOURCE_TYPE_NOT_FOUND : ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    memcpy((char *)out + 4, data, resource_size);
    memcpy(out, &resource_size, 4);
    free(data);
    return TRUE;
}

/* "\StringFileInfo\080904B0\ProductVersion": walks the VS_VERSIONINFO blocks (length, value length, type, a UTF-16
 * key, the value, the children, each 4-aligned); a string comes back converted, in the buffer's second half */
BOOL VerQueryValueA(LPCVOID block, LPCSTR sub_block, LPVOID *value, PUINT length) {
    const unsigned char *base = (const unsigned char *)block + 4;
    unsigned int total;
    unsigned int at = 0;
    unsigned int end;
    const char *want = sub_block;
    static char empty[1];

    memcpy(&total, block, 4);
    end = total;
    /* [port] a missing resource leaves an empty string rather than an unset pointer (GetProductVersion copies it) */
    *value = empty;
    if (length != NULL) {
        *length = 0;
    }
    while (*want == '\\') {
        want++;
    }
    for (;;) {
        unsigned int len = U16(base + at);
        unsigned int value_len = U16(base + at + 2);
        unsigned int type = U16(base + at + 4);
        unsigned int key = at + 6;
        unsigned int child;
        unsigned int val;
        char name[128];
        unsigned int n = 0;
        size_t part;

        while (U16(base + key + n * 2) != 0 && n < sizeof(name) - 1) {
            name[n] = (char)U16(base + key + n * 2);
            n++;
        }
        name[n] = '\0';
        val = (key + (n + 1) * 2 + 3) & ~3u;
        if (*want == '\0') {
            if (type == 1) {
                /* text, as ANSI, after the resource: at half the block's offset there (blocks don't overlap, and
                 * the characters take half their UTF-16 bytes) */
                char *ansi = (char *)block + 4 + total + at / 2;
                int chars = value_len > 0 ? WideCharToMultiByte(CP_ACP, 0, (LPCWCH)(base + val), (int)value_len, ansi,
                                                (int)(len / 2 + 1), NULL, NULL)
                                          : 0;
                ansi[chars] = '\0';
                *value = ansi;
            } else {
                *value = (void *)(base + val);
            }
            if (length != NULL) {
                *length = value_len;
            }
            return TRUE;
        }
        part = strcspn(want, "\\");
        child = (val + (type == 1 ? value_len * 2 : value_len) + 3) & ~3u;
        {
            unsigned int stop = at + len;
            int found = 0;

            while (child + 6 < stop && child < end) {
                unsigned int clen = U16(base + child);
                char cname[128];
                unsigned int m = 0;

                if (clen == 0) {
                    break;
                }
                while (U16(base + child + 6 + m * 2) != 0 && m < sizeof(cname) - 1) {
                    cname[m] = (char)U16(base + child + 6 + m * 2);
                    m++;
                }
                cname[m] = '\0';
                if (strlen(cname) == part && strncasecmp(cname, want, part) == 0) {
                    at = child;
                    end = child + clen;
                    found = 1;
                    break;
                }
                child = (child + clen + 3) & ~3u;
            }
            if (!found) {
                PosixSetError(ERROR_RESOURCE_TYPE_NOT_FOUND);
                return FALSE;
            }
        }
        want += part;
        while (*want == '\\') {
            want++;
        }
    }
}
