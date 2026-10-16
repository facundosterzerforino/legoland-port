#ifndef PORT_SETUP_H
#define PORT_SETUP_H

/* [library:filesystem] The game's data folder: finding it, and building it from the LEGOLAND CD.
 *
 * The port plays from a complete install: the files the original's installer unpacked from the CD's main.z (an
 * InstallShield 3 archive: the game's base files, IMusic\, zbuffers\, strings\, RollerCoaster\) plus the folders
 * the original read from the CD itself (the movies into FMV\, speech\, the .res files into volumes\).
 * PortSetupInstall builds that from the CD (a drive, a mounted ISO or a copy of it), straight from an ISO image
 * file (ISO 9660 with Joliet names, read without mounting it), or copies an existing install. The UI is the launcher's (port_launcher.c); this part
 * is plain C on SDL3's file functions, for the other platforms too. */

/* What a folder or file is: */
enum {
    PORT_SETUP_NONE = 0, /* none of these */
    PORT_SETUP_CD = 1, /* the CD: main.z (the Czech CD: data\lego.pak), speech\, Legoland.res (beside it or in
                          * volumes\), the movies */
    PORT_SETUP_INSTALLED = 2, /* a complete install (volumes\Legoland.res, IMusic\, FMV\, speech\) */
    PORT_SETUP_IMAGE = 3 /* an image file of the CD (.iso, or a raw .bin) */
};
int PortSetupKind(const char *path);

/* The data folder is the current directory when it has the game, else the exe's folder when that has it (then it
 * becomes the current directory). 0 when neither has it. */
int PortSetupFindData(void);

/* The exe's folder, with a trailing separator. */
const char *PortSetupExeFolder(void);

typedef struct PortSetupProgress {
    /* bytes done of the total, and the file being written (relative to the target) */
    void (*report)(void *ctx, unsigned long long done, unsigned long long total, const char *file);
    int (*cancelled)(void *ctx); /* non-zero stops it */
    void *ctx;
} PortSetupProgress;

/* Builds the data folder target (created if needed) from source (a CD, its image or an install, PortSetupKind). Returns 1 on
 * success; else 0 with a message in error. Files already there are overwritten. */
int PortSetupInstall(const char *source, const char *target, const PortSetupProgress *progress, char *error,
    unsigned int error_size);

#endif
