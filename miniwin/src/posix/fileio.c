/* [library:filesystem] Files, searches, folders and drives off Windows. Paths are Windows' (backslashes, any case):
 * each goes through miniwin's MiniwinPath first. See posix.h. */
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "miniwin/files.h"
#include "posix.h"

struct File {
    struct PosixObject base;
    int fd;
};

struct Find {
    struct PosixObject base;
    char folder[MAX_PATH];
    char **names;
    int count;
    int next;
};

static __thread DWORD last_error;

void PosixSetError(DWORD error) {
    last_error = error;
}

DWORD GetLastError(void) {
    return last_error;
}

DWORD PosixErrorFromErrno(int error) {
    switch (error) {
    case 0:
        return ERROR_SUCCESS;
    case ENOENT:
        return ERROR_FILE_NOT_FOUND;
    case ENOTDIR:
        return ERROR_PATH_NOT_FOUND;
    case EACCES:
    case EPERM:
    case EROFS:
        return ERROR_ACCESS_DENIED;
    case EEXIST:
        return ERROR_ALREADY_EXISTS;
    case ENOMEM:
        return ERROR_NOT_ENOUGH_MEMORY;
    case ENOSPC:
        return ERROR_DISK_FULL;
    case EBADF:
        return ERROR_INVALID_HANDLE;
    default:
        return ERROR_GEN_FAILURE;
    }
}

const char *PosixPath(const char *path, char *buf, size_t size) {
    return MiniwinPath(path, buf, size);
}

void PosixFileTime(long long seconds, long nanoseconds, FILETIME *out) {
    unsigned long long t = (unsigned long long)(seconds + 11644473600LL) * 10000000ULL + (unsigned long long)nanoseconds / 100;

    out->dwLowDateTime = (DWORD)t;
    out->dwHighDateTime = (DWORD)(t >> 32);
}

/* ---- files ---- */

HANDLE CreateFileA(LPCSTR name, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES security, DWORD disposition,
    DWORD flags, HANDLE template_file) {
    char path[MAX_PATH];
    struct File *f;
    int oflags;
    int fd;

    (void)share;
    (void)security;
    (void)template_file;
    if ((access & (GENERIC_READ | GENERIC_WRITE)) == (GENERIC_READ | GENERIC_WRITE)) {
        oflags = O_RDWR;
    } else if ((access & GENERIC_WRITE) != 0) {
        oflags = O_WRONLY;
    } else {
        oflags = O_RDONLY;
    }
    switch (disposition) {
    case CREATE_NEW:
        oflags |= O_CREAT | O_EXCL;
        break;
    case CREATE_ALWAYS:
        oflags |= O_CREAT | O_TRUNC;
        break;
    case OPEN_ALWAYS:
        oflags |= O_CREAT;
        break;
    case TRUNCATE_EXISTING:
        oflags |= O_TRUNC;
        break;
    case OPEN_EXISTING:
        break;
    default:
        PosixSetError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }
    if (strcmp(name, "CONIN$") == 0 || strcmp(name, "CONOUT$") == 0) {
        /* the console: the terminal's input or output */
        fd = dup(name[3] == 'I' ? 0 : 1);
        if (fd < 0) {
            PosixSetError(PosixErrorFromErrno(errno));
            return INVALID_HANDLE_VALUE;
        }
        PosixSetError(ERROR_SUCCESS);
    } else {
        /* Windows reports whether OPEN_ALWAYS and CREATE_ALWAYS found the file already there */
        struct stat st;
        int existed;

        PosixPath(name, path, sizeof(path));
        existed = stat(path, &st) == 0;

        fd = open(path, oflags | O_CLOEXEC, 0644);
        if (fd < 0) {
            PosixSetError(errno == EEXIST ? ERROR_FILE_EXISTS : PosixErrorFromErrno(errno));
            return INVALID_HANDLE_VALUE;
        }
        if ((flags & FILE_FLAG_WRITE_THROUGH) != 0) {
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_SYNC);
        }
        PosixSetError((disposition == OPEN_ALWAYS || disposition == CREATE_ALWAYS) && existed ? ERROR_ALREADY_EXISTS
                                                                                              : ERROR_SUCCESS);
    }
    f = (struct File *)PosixNewObject(POSIX_FILE, sizeof(struct File));
    if (f == NULL) {
        close(fd);
        PosixSetError(ERROR_NOT_ENOUGH_MEMORY);
        return INVALID_HANDLE_VALUE;
    }
    f->fd = fd;
    return (HANDLE)f;
}

BOOL ReadFile(HANDLE handle, LPVOID buffer, DWORD size, LPDWORD read_size, LPOVERLAPPED overlapped) {
    struct File *f = (struct File *)PosixObjectOf(handle, POSIX_FILE);
    size_t done = 0;

    (void)overlapped;
    if (read_size != NULL) {
        *read_size = 0;
    }
    if (f == NULL) {
        PosixSetError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    while (done < size) {
        ssize_t n = read(f->fd, (char *)buffer + done, size - done);
        if (n < 0 && errno == EINTR) {
            continue;
        }
        if (n < 0) {
            PosixSetError(PosixErrorFromErrno(errno));
            return FALSE;
        }
        if (n == 0) {
            break; /* the end: success, with fewer bytes */
        }
        done += (size_t)n;
    }
    if (read_size != NULL) {
        *read_size = (DWORD)done;
    }
    return TRUE;
}

BOOL WriteFile(HANDLE handle, LPCVOID buffer, DWORD size, LPDWORD written, LPOVERLAPPED overlapped) {
    struct File *f = (struct File *)PosixObjectOf(handle, POSIX_FILE);
    size_t done = 0;

    (void)overlapped;
    if (written != NULL) {
        *written = 0;
    }
    if (f == NULL) {
        PosixSetError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    while (done < size) {
        ssize_t n = write(f->fd, (const char *)buffer + done, size - done);
        if (n < 0 && errno == EINTR) {
            continue;
        }
        if (n < 0) {
            PosixSetError(PosixErrorFromErrno(errno));
            return FALSE;
        }
        done += (size_t)n;
    }
    if (written != NULL) {
        *written = (DWORD)done;
    }
    return TRUE;
}

DWORD SetFilePointer(HANDLE handle, LONG distance, PLONG distance_high, DWORD method) {
    struct File *f = (struct File *)PosixObjectOf(handle, POSIX_FILE);
    long long offset = distance;
    off_t at;

    if (f == NULL) {
        PosixSetError(ERROR_INVALID_HANDLE);
        return INVALID_SET_FILE_POINTER;
    }
    if (distance_high != NULL) {
        offset = (long long)(((unsigned long long)(unsigned int)*distance_high << 32) | (unsigned int)distance);
    }
    at = lseek(f->fd, (off_t)offset, method == FILE_BEGIN ? SEEK_SET : method == FILE_CURRENT ? SEEK_CUR
                                                                                              : SEEK_END);
    if (at < 0) {
        PosixSetError(errno == EINVAL ? ERROR_NEGATIVE_SEEK : PosixErrorFromErrno(errno));
        return INVALID_SET_FILE_POINTER;
    }
    if (distance_high != NULL) {
        *distance_high = (LONG)((unsigned long long)at >> 32);
    }
    PosixSetError(ERROR_SUCCESS);
    return (DWORD)at;
}

DWORD GetFileSize(HANDLE handle, LPDWORD size_high) {
    struct File *f = (struct File *)PosixObjectOf(handle, POSIX_FILE);
    struct stat st;

    if (f == NULL || fstat(f->fd, &st) != 0) {
        PosixSetError(ERROR_INVALID_HANDLE);
        return INVALID_FILE_SIZE;
    }
    if (size_high != NULL) {
        *size_high = (DWORD)((unsigned long long)st.st_size >> 32);
    }
    return (DWORD)st.st_size;
}

BOOL GetFileTime(HANDLE handle, LPFILETIME created, LPFILETIME accessed, LPFILETIME written) {
    struct File *f = (struct File *)PosixObjectOf(handle, POSIX_FILE);
    struct stat st;

    if (f == NULL || fstat(f->fd, &st) != 0) {
        PosixSetError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    if (created != NULL) {
        PosixFileTime(st.st_ctim.tv_sec, st.st_ctim.tv_nsec, created); /* the inode's change time: the nearest */
    }
    if (accessed != NULL) {
        PosixFileTime(st.st_atim.tv_sec, st.st_atim.tv_nsec, accessed);
    }
    if (written != NULL) {
        PosixFileTime(st.st_mtim.tv_sec, st.st_mtim.tv_nsec, written);
    }
    return TRUE;
}

BOOL CloseHandle(HANDLE handle) {
    struct PosixObject *o = (struct PosixObject *)handle;

    if (o == NULL || handle == INVALID_HANDLE_VALUE) {
        PosixSetError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    switch (o->kind) {
    case POSIX_FILE:
        close(((struct File *)o)->fd);
        o->kind = 0;
        free(o);
        return TRUE;
    case POSIX_EVENT:
    case POSIX_MUTEX:
    case POSIX_THREAD:
        return PosixCloseObject(o);
    default:
        PosixSetError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
}

/* ---- searches ---- */

static DWORD Attributes(const struct stat *st, const char *name) {
    DWORD a = S_ISDIR(st->st_mode) ? FILE_ATTRIBUTE_DIRECTORY : 0;

    if ((st->st_mode & S_IWUSR) == 0) {
        a |= FILE_ATTRIBUTE_READONLY;
    }
    if (name[0] == '.' && strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
        a |= FILE_ATTRIBUTE_HIDDEN;
    }
    return a != 0 ? a : FILE_ATTRIBUTE_NORMAL;
}

/* Windows' wildcards: "*.*" is every name, "*" too; the rest as fnmatch, ignoring case */
static int Matches(const char *pattern, const char *name) {
    if (strcmp(pattern, "*.*") == 0 || strcmp(pattern, "*") == 0) {
        return 1;
    }
    return fnmatch(pattern, name, FNM_CASEFOLD) == 0;
}

static int CompareNames(const void *a, const void *b) {
    const unsigned char *p = *(const unsigned char *const *)a;
    const unsigned char *q = *(const unsigned char *const *)b;

    while (*p != '\0' && toupper(*p) == toupper(*q)) {
        p++;
        q++;
    }
    return toupper(*p) - toupper(*q);
}

int PosixListFolder(const char *folder, const char *pattern, char ***names) {
    DIR *dir = opendir(folder);
    struct dirent *e;
    char **list = NULL;
    int count = 0;

    if (dir == NULL) {
        return -1;
    }
    while ((e = readdir(dir)) != NULL) {
        char **grown;

        if (!Matches(pattern, e->d_name)) {
            continue;
        }
        grown = (char **)realloc(list, sizeof(char *) * (size_t)(count + 1));
        if (grown == NULL) {
            break;
        }
        list = grown;
        list[count++] = strdup(e->d_name);
    }
    closedir(dir);
    qsort(list, (size_t)count, sizeof(char *), CompareNames);
    *names = list;
    return count;
}

void PosixFreeList(char **names, int count) {
    int i;

    for (i = 0; i < count; i++) {
        free(names[i]);
    }
    free(names);
}

static BOOL NextMatch(struct Find *find, LPWIN32_FIND_DATAA data) {
    while (find->next < find->count) {
        const char *name = find->names[find->next++];
        char full[MAX_PATH * 2];
        struct stat st;

        snprintf(full, sizeof(full), "%s/%s", find->folder, name);
        if (strlen(name) >= sizeof(data->cFileName) || stat(full, &st) != 0) {
            continue;
        }
        memset(data, 0, sizeof(*data));
        data->dwFileAttributes = Attributes(&st, name);
        PosixFileTime(st.st_ctim.tv_sec, st.st_ctim.tv_nsec, &data->ftCreationTime);
        PosixFileTime(st.st_atim.tv_sec, st.st_atim.tv_nsec, &data->ftLastAccessTime);
        PosixFileTime(st.st_mtim.tv_sec, st.st_mtim.tv_nsec, &data->ftLastWriteTime);
        data->nFileSizeHigh = (DWORD)((unsigned long long)st.st_size >> 32);
        data->nFileSizeLow = (DWORD)st.st_size;
        strcpy(data->cFileName, name);
        return TRUE;
    }
    PosixSetError(ERROR_NO_MORE_FILES);
    return FALSE;
}

/* splits a Windows search spec ("data\*.lls") into the folder, as on disk, and the pattern */
int PosixSplitSpec(const char *spec, char *folder, size_t folder_size, char *pattern, size_t pattern_size) {
    char copy[MAX_PATH];
    const char *slash;

    if (strlen(spec) >= sizeof(copy)) {
        return 0;
    }
    strcpy(copy, spec);
    slash = strrchr(copy, '\\');
    if (slash == NULL || (strrchr(copy, '/') != NULL && strrchr(copy, '/') > slash)) {
        slash = strrchr(copy, '/');
    }
    if (slash == NULL) {
        snprintf(folder, folder_size, ".");
        snprintf(pattern, pattern_size, "%s", copy);
    } else {
        snprintf(pattern, pattern_size, "%s", slash + 1);
        copy[slash - copy] = '\0';
        PosixPath(copy[0] != '\0' ? copy : "/", folder, folder_size);
    }
    return 1;
}

HANDLE FindFirstFileA(LPCSTR spec, LPWIN32_FIND_DATAA data) {
    struct Find *find = (struct Find *)PosixNewObject(POSIX_FIND, sizeof(struct Find));
    char pattern[MAX_PATH];

    if (find == NULL) {
        PosixSetError(ERROR_NOT_ENOUGH_MEMORY);
        return INVALID_HANDLE_VALUE;
    }
    if (!PosixSplitSpec(spec, find->folder, sizeof(find->folder), pattern, sizeof(pattern)) ||
        (find->count = PosixListFolder(find->folder, pattern, &find->names)) < 0) {
        PosixSetError(ERROR_PATH_NOT_FOUND);
        free(find);
        return INVALID_HANDLE_VALUE;
    }
    if (!NextMatch(find, data)) {
        PosixFreeList(find->names, find->count);
        free(find);
        PosixSetError(ERROR_FILE_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }
    return (HANDLE)find;
}

BOOL FindNextFileA(HANDLE handle, LPWIN32_FIND_DATAA data) {
    struct Find *find = (struct Find *)PosixObjectOf(handle, POSIX_FIND);

    if (find == NULL) {
        PosixSetError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    return NextMatch(find, data);
}

BOOL FindClose(HANDLE handle) {
    struct Find *find = (struct Find *)PosixObjectOf(handle, POSIX_FIND);

    if (find == NULL) {
        PosixSetError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    PosixFreeList(find->names, find->count);
    find->base.kind = 0;
    free(find);
    return TRUE;
}

/* ---- names, folders ---- */

DWORD GetFileAttributesA(LPCSTR name) {
    char path[MAX_PATH];
    struct stat st;
    const char *base;

    if (stat(PosixPath(name, path, sizeof(path)), &st) != 0) {
        PosixSetError(PosixErrorFromErrno(errno));
        return INVALID_FILE_ATTRIBUTES;
    }
    base = strrchr(path, '/');
    return Attributes(&st, base != NULL ? base + 1 : path);
}

BOOL CreateDirectoryA(LPCSTR name, LPSECURITY_ATTRIBUTES security) {
    char path[MAX_PATH];

    (void)security;
    if (mkdir(PosixPath(name, path, sizeof(path)), 0755) != 0) {
        PosixSetError(PosixErrorFromErrno(errno));
        return FALSE;
    }
    return TRUE;
}

BOOL MoveFileExA(LPCSTR from, LPCSTR to, DWORD flags) {
    char a[MAX_PATH];
    char b[MAX_PATH];
    struct stat st;

    PosixPath(from, a, sizeof(a));
    PosixPath(to, b, sizeof(b));
    if ((flags & MOVEFILE_REPLACE_EXISTING) == 0 && stat(b, &st) == 0) {
        PosixSetError(ERROR_ALREADY_EXISTS);
        return FALSE;
    }
    if (rename(a, b) != 0) {
        PosixSetError(PosixErrorFromErrno(errno));
        return FALSE;
    }
    return TRUE;
}

BOOL SetCurrentDirectoryA(LPCSTR name) {
    char path[MAX_PATH];

    if (chdir(PosixPath(name, path, sizeof(path))) != 0) {
        PosixSetError(PosixErrorFromErrno(errno));
        return FALSE;
    }
    return TRUE;
}

/* the module is the program: its path, and a stand-in instance handle */
static char module_tag;

HMODULE GetModuleHandleA(LPCSTR name) {
    if (name != NULL) {
        PosixSetError(ERROR_MOD_NOT_FOUND);
        return NULL;
    }
    return (HMODULE)&module_tag;
}

DWORD GetModuleFileNameA(HMODULE module, LPSTR out, DWORD size) {
    char path[MAX_PATH];
    ssize_t n;

    if (module != NULL && module != (HMODULE)&module_tag) {
        PosixSetError(ERROR_MOD_NOT_FOUND);
        return 0;
    }
    n = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (n < 0 || size == 0) {
        PosixSetError(ERROR_GEN_FAILURE);
        return 0;
    }
    path[n] = '\0';
    if ((DWORD)n >= size) {
        memcpy(out, path, size - 1);
        out[size - 1] = '\0';
        PosixSetError(ERROR_INSUFFICIENT_BUFFER);
        return size;
    }
    memcpy(out, path, (size_t)n + 1);
    return (DWORD)n;
}

/* ---- drives: none with letters here, so no CD drive either ---- */

DWORD GetLogicalDrives(void) {
    return 0;
}

UINT GetDriveTypeA(LPCSTR root) {
    (void)root;
    return DRIVE_NO_ROOT_DIR;
}

BOOL GetVolumeInformationA(LPCSTR root, LPSTR name, DWORD name_size, LPDWORD serial, LPDWORD max_component,
    LPDWORD flags, LPSTR fs_name, DWORD fs_name_size) {
    (void)root;
    (void)name;
    (void)name_size;
    (void)serial;
    (void)max_component;
    (void)flags;
    (void)fs_name;
    (void)fs_name_size;
    PosixSetError(ERROR_PATH_NOT_FOUND);
    return FALSE;
}

BOOL DeviceIoControl(HANDLE device, DWORD code, LPVOID in, DWORD in_size, LPVOID out, DWORD out_size,
    LPDWORD returned, LPOVERLAPPED overlapped) {
    (void)device;
    (void)code;
    (void)in;
    (void)in_size;
    (void)out;
    (void)out_size;
    (void)returned;
    (void)overlapped;
    PosixSetError(ERROR_NOT_SUPPORTED);
    return FALSE;
}
