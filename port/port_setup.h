#ifndef PORT_SETUP_H
#define PORT_SETUP_H

/* [library:filesystem] The game's data folder: finding it, and building it from the LEGOLAND CD.
 *
 * The port plays from a complete install: the files the original's installer unpacked from the CD's main.z (an
 * InstallShield 3 archive: the game's base files, IMusic\, zbuffers\, strings\, RollerCoaster\) plus the folders
 * the original read from the CD itself (FMV\, speech\, volumes\). PortSetupInstall builds that from the CD (or a
 * mounted ISO, or a copy of it), or copies an existing install. The UI is the launcher's (port_launcher.c); this part
 * is plain C on SDL3's file functions, for the other platforms too. */

/* What a folder is: */
enum {
    PORT_SETUP_NONE = 0, /* neither */
    PORT_SETUP_CD = 1, /* the CD: main.z, FMV\, speech\, volumes\ */
    PORT_SETUP_INSTALLED = 2 /* a complete install (volumes\Legoland.res, IMusic\, FMV\, speech\) */
};
int PortSetupKind(const char *folder);

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

/* Builds the data folder target (created if needed) from source (a CD or an install, PortSetupKind). Returns 1 on
 * success; else 0 with a message in error. Files already there are overwritten. */
int PortSetupInstall(const char *source, const char *target, const PortSetupProgress *progress, char *error,
    unsigned int error_size);

#endif
