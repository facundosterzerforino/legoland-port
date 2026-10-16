/* [library:filesystem] The first-run setup window (Windows): when the game's data isn't found, it asks for the
 * LEGOLAND CD (or its ISO image, or a folder) and builds the data folder next to the exe with port_setup.c.
 *
 * Its texts come in the game's languages (setup_texts below, UTF-8), Windows' display language first, and a
 * drop-down to change it; the window is Unicode, so every language shows on any Windows. Paths are UTF-8, as
 * port_setup.c (SDL) takes them. */
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <stdio.h>
#include <string.h>

#include "port_setup.h"
#include "port_setup_win.h"
#include "port_trace.h"

#define ID_INSTALL 201
#define ID_CHOOSE 202
#define ID_SOURCE 203
#define ID_PROGRESS 204
#define ID_FILE 205
#define ID_CHOOSE_ISO 206
#define ID_LANGUAGE 207
#define TIMER_PROGRESS 1
#define NEEDED_MB 450 /* the data folder is about 400 MB */
#define PATH_SIZE 1024

enum {
    T_INTRO,
    T_NONE,
    T_FROM, /* "Install from: %s (%s)" */
    T_KIND_CD,
    T_KIND_IMAGE,
    T_KIND_INSTALLED,
    T_INSTALL,
    T_CHOOSE_ISO,
    T_CHOOSE_FOLDER,
    T_QUIT,
    T_CANCEL,
    T_COPYING,
    T_FOLDER_TITLE,
    T_ISO_TITLE,
    T_ISO_FILTER,
    T_ALL_FILES,
    T_NOT_FOLDER,
    T_NOT_ISO,
    T_NOT_WRITABLE, /* %s: the folder */
    T_NO_SPACE, /* %d MB, %s the folder */
    T_FAILED, /* %s: the reason (port_setup.c's, in English) */
    T_LANGUAGE,
    T_COUNT
};

typedef struct SetupLanguage {
    const char *name; /* in itself */
    WORD primary; /* LANG_* of Windows' display language */
    const char *text[T_COUNT];
} SetupLanguage;

static const SetupLanguage setup_texts[] = {
    {"English",
        LANG_ENGLISH,
        {"LEGOLAND needs its game files from the CD. Insert the LEGOLAND CD, or choose its ISO image (no need to "
         "mount it), or a folder with a copy of the CD or with LEGOLAND installed.\n\nThe files (about 400 MB) are "
         "copied next to this program, once.",
            "No LEGOLAND CD found. Insert it, or choose its ISO image or a folder.", "Install from: %s (%s)",
            "the LEGOLAND CD", "an image of the LEGOLAND CD", "an installed LEGOLAND", "Install", "Choose ISO...",
            "Choose folder...", "Quit", "Cancel", "Copying the game's files...",
            "Choose the LEGOLAND CD, a copy of it, or a folder with LEGOLAND installed:",
            "Choose the LEGOLAND CD's ISO image", "CD images (*.iso, *.bin)", "All files (*.*)",
            "That isn't the LEGOLAND CD or a LEGOLAND folder.\n\nThe CD has main.z, Legoland.res and the folder "
            "Speech; an installed LEGOLAND has IMusic, FMV, speech and volumes. For an ISO image, use Choose ISO.",
            "That isn't an image of the LEGOLAND CD.\n\nThe CD's image is an .iso (or .bin) file with main.z, "
            "Legoland.res and the folder Speech in it.",
            "The game's files go next to this program, in\n%s\nbut that folder can't be written to.\n\nMove the "
            "LEGOLAND folder somewhere you can write, such as Documents, and start it again.",
            "The game's files need about %d MB, and the disk with\n%s\nhas less free space.",
            "The game's files couldn't be copied.\n\n%s", "Language:"}},
    {"Español",
        LANG_SPANISH,
        {"LEGOLAND necesita los archivos del juego del CD. Inserta el CD de LEGOLAND, o elige su imagen ISO (no "
         "hace falta montarla), o una carpeta con una copia del CD o con LEGOLAND instalado.\n\nLos archivos (unos "
         "400 MB) se copian junto a este programa, una sola vez.",
            "No se encontró el CD de LEGOLAND. Insértalo, o elige su imagen ISO o una carpeta.",
            "Instalar desde: %s (%s)", "el CD de LEGOLAND", "una imagen del CD de LEGOLAND",
            "un LEGOLAND instalado", "Instalar", "Elegir ISO...", "Elegir carpeta...", "Salir", "Cancelar",
            "Copiando los archivos del juego...",
            "Elige el CD de LEGOLAND, una copia o una carpeta con LEGOLAND instalado:",
            "Elige la imagen ISO del CD de LEGOLAND", "Imágenes de CD (*.iso, *.bin)", "Todos los archivos (*.*)",
            "Eso no es el CD de LEGOLAND ni una carpeta de LEGOLAND.\n\nEl CD tiene main.z, Legoland.res y la "
            "carpeta Speech; un LEGOLAND instalado tiene IMusic, FMV, speech y volumes. Para una imagen ISO, usa "
            "Elegir ISO.",
            "Ese archivo no es una imagen del CD de LEGOLAND.\n\nLa imagen del CD es un archivo .iso (o .bin) que "
            "contiene main.z, Legoland.res y la carpeta Speech.",
            "Los archivos del juego van junto a este programa, en\n%s\npero no se puede escribir en esa carpeta."
            "\n\nMueve la carpeta de LEGOLAND a un lugar donde puedas escribir, como Documentos, y vuelve a "
            "iniciarlo.",
            "Los archivos del juego necesitan unos %d MB, y el disco de\n%s\ntiene menos espacio libre.",
            "No se pudieron copiar los archivos del juego.\n\n%s", "Idioma:"}},
    {"Deutsch",
        LANG_GERMAN,
        {"LEGOLAND braucht seine Spieldateien von der CD. Lege die LEGOLAND-CD ein, oder wähle ihr ISO-Abbild (es "
         "muss nicht eingebunden werden) oder einen Ordner mit einer Kopie der CD oder mit installiertem "
         "LEGOLAND.\n\nDie Dateien (etwa 400 MB) werden einmalig neben dieses Programm kopiert.",
            "Keine LEGOLAND-CD gefunden. Lege sie ein, oder wähle ihr ISO-Abbild oder einen Ordner.",
            "Installieren von: %s (%s)", "die LEGOLAND-CD", "ein Abbild der LEGOLAND-CD",
            "ein installiertes LEGOLAND", "Installieren", "ISO wählen...", "Ordner wählen...", "Beenden",
            "Abbrechen", "Die Spieldateien werden kopiert...",
            "Wähle die LEGOLAND-CD, eine Kopie davon oder einen Ordner mit installiertem LEGOLAND:",
            "Wähle das ISO-Abbild der LEGOLAND-CD", "CD-Abbilder (*.iso, *.bin)", "Alle Dateien (*.*)",
            "Das ist weder die LEGOLAND-CD noch ein LEGOLAND-Ordner.\n\nDie CD enthält main.z, Legoland.res und "
            "den Ordner Speech; ein installiertes LEGOLAND hat IMusic, FMV, speech und volumes. Für ein "
            "ISO-Abbild nimm „ISO wählen“.",
            "Das ist kein Abbild der LEGOLAND-CD.\n\nDas Abbild der CD ist eine .iso-Datei (oder .bin-Datei) mit "
            "main.z, Legoland.res und dem Ordner Speech.",
            "Die Spieldateien kommen neben dieses Programm, nach\n%s\naber in diesen Ordner kann nicht geschrieben "
            "werden.\n\nVerschiebe den LEGOLAND-Ordner an einen Ort, an dem du schreiben kannst, zum Beispiel "
            "Dokumente, und starte es erneut.",
            "Die Spieldateien brauchen etwa %d MB, und auf dem Laufwerk von\n%s\nist weniger Platz frei.",
            "Die Spieldateien konnten nicht kopiert werden.\n\n%s", "Sprache:"}},
    {"Français",
        LANG_FRENCH,
        {"LEGOLAND a besoin des fichiers du jeu présents sur le CD. Insérez le CD de LEGOLAND, ou choisissez son "
         "image ISO (inutile de la monter), ou un dossier contenant une copie du CD ou LEGOLAND installé.\n\nLes "
         "fichiers (environ 400 Mo) sont copiés une seule fois à côté de ce programme.",
            "CD de LEGOLAND introuvable. Insérez-le, ou choisissez son image ISO ou un dossier.",
            "Installer depuis : %s (%s)", "le CD de LEGOLAND", "une image du CD de LEGOLAND",
            "un LEGOLAND installé", "Installer", "Choisir l'ISO...", "Choisir dossier...", "Quitter",
            "Annuler", "Copie des fichiers du jeu...",
            "Choisissez le CD de LEGOLAND, une copie, ou un dossier où LEGOLAND est installé :",
            "Choisissez l'image ISO du CD de LEGOLAND", "Images de CD (*.iso, *.bin)", "Tous les fichiers (*.*)",
            "Ce n'est ni le CD de LEGOLAND ni un dossier de LEGOLAND.\n\nLe CD contient main.z, Legoland.res et le "
            "dossier Speech ; un LEGOLAND installé contient IMusic, FMV, speech et volumes. Pour une image ISO, "
            "utilisez « Choisir l'ISO ».",
            "Ce n'est pas une image du CD de LEGOLAND.\n\nL'image du CD est un fichier .iso (ou .bin) contenant "
            "main.z, Legoland.res et le dossier Speech.",
            "Les fichiers du jeu sont copiés à côté de ce programme, dans\n%s\nmais ce dossier n'est pas "
            "accessible en écriture.\n\nDéplacez le dossier LEGOLAND à un endroit où vous pouvez écrire, comme "
            "Documents, puis relancez-le.",
            "Les fichiers du jeu occupent environ %d Mo, et le disque de\n%s\na moins d'espace libre.",
            "Les fichiers du jeu n'ont pas pu être copiés.\n\n%s", "Langue :"}},
    {"Italiano",
        LANG_ITALIAN,
        {"LEGOLAND ha bisogno dei file del gioco dal CD. Inserisci il CD di LEGOLAND, oppure scegli la sua "
         "immagine ISO (non serve montarla) o una cartella con una copia del CD o con LEGOLAND installato.\n\nI "
         "file (circa 400 MB) vengono copiati una sola volta accanto a questo programma.",
            "CD di LEGOLAND non trovato. Inseriscilo, oppure scegli la sua immagine ISO o una cartella.",
            "Installa da: %s (%s)", "il CD di LEGOLAND", "un'immagine del CD di LEGOLAND",
            "un LEGOLAND installato", "Installa", "Scegli ISO...", "Scegli cartella...", "Esci", "Annulla",
            "Copia dei file del gioco...",
            "Scegli il CD di LEGOLAND, una sua copia o una cartella con LEGOLAND installato:",
            "Scegli l'immagine ISO del CD di LEGOLAND", "Immagini CD (*.iso, *.bin)", "Tutti i file (*.*)",
            "Questo non è il CD di LEGOLAND né una cartella di LEGOLAND.\n\nIl CD contiene main.z, Legoland.res e "
            "la cartella Speech; un LEGOLAND installato contiene IMusic, FMV, speech e volumes. Per un'immagine "
            "ISO, usa Scegli ISO.",
            "Questo file non è un'immagine del CD di LEGOLAND.\n\nL'immagine del CD è un file .iso (o .bin) che "
            "contiene main.z, Legoland.res e la cartella Speech.",
            "I file del gioco vanno accanto a questo programma, in\n%s\nma in quella cartella non si può "
            "scrivere.\n\nSposta la cartella di LEGOLAND in un posto dove puoi scrivere, come Documenti, e "
            "avvialo di nuovo.",
            "I file del gioco richiedono circa %d MB, e il disco di\n%s\nha meno spazio libero.",
            "Non è stato possibile copiare i file del gioco.\n\n%s", "Lingua:"}},
    {"Nederlands",
        LANG_DUTCH,
        {"LEGOLAND heeft de spelbestanden van de cd nodig. Plaats de LEGOLAND-cd, of kies het ISO-bestand ervan "
         "(koppelen is niet nodig), of een map met een kopie van de cd of met LEGOLAND geïnstalleerd.\n\nDe "
         "bestanden (ongeveer 400 MB) worden één keer naast dit programma gekopieerd.",
            "Geen LEGOLAND-cd gevonden. Plaats de cd, of kies het ISO-bestand of een map.",
            "Installeren vanaf: %s (%s)", "de LEGOLAND-cd", "een image van de LEGOLAND-cd",
            "een geïnstalleerde LEGOLAND", "Installeren", "ISO kiezen...", "Map kiezen...", "Afsluiten",
            "Annuleren", "De spelbestanden worden gekopieerd...",
            "Kies de LEGOLAND-cd, een kopie ervan of een map met een geïnstalleerde LEGOLAND:",
            "Kies het ISO-bestand van de LEGOLAND-cd", "Cd-images (*.iso, *.bin)", "Alle bestanden (*.*)",
            "Dat is niet de LEGOLAND-cd en ook geen LEGOLAND-map.\n\nDe cd bevat main.z, Legoland.res en de map "
            "Speech; een geïnstalleerde LEGOLAND heeft IMusic, FMV, speech en volumes. Gebruik voor een "
            "ISO-bestand ‘ISO kiezen’.",
            "Dat is geen image van de LEGOLAND-cd.\n\nDe image van de cd is een .iso-bestand (of .bin-bestand) "
            "met main.z, Legoland.res en de map Speech.",
            "De spelbestanden komen naast dit programma, in\n%s\nmaar in die map kan niet worden geschreven.\n\n"
            "Verplaats de LEGOLAND-map naar een plek waar je kunt schrijven, zoals Documenten, en start het "
            "opnieuw.",
            "De spelbestanden hebben ongeveer %d MB nodig, en de schijf met\n%s\nheeft minder vrije ruimte.",
            "De spelbestanden konden niet worden gekopieerd.\n\n%s", "Taal:"}},
    {"Dansk",
        LANG_DANISH,
        {"LEGOLAND skal bruge spilfilerne fra cd'en. Indsæt LEGOLAND-cd'en, eller vælg dens ISO-fil (den behøver "
         "ikke at blive monteret) eller en mappe med en kopi af cd'en eller med LEGOLAND installeret.\n\nFilerne "
         "(ca. 400 MB) kopieres én gang ved siden af dette program.",
            "Ingen LEGOLAND-cd fundet. Indsæt den, eller vælg dens ISO-fil eller en mappe.",
            "Installer fra: %s (%s)", "LEGOLAND-cd'en", "en kopi af LEGOLAND-cd'en", "et installeret LEGOLAND",
            "Installer", "Vælg ISO...", "Vælg mappe...", "Afslut", "Annuller", "Spilfilerne kopieres...",
            "Vælg LEGOLAND-cd'en, en kopi af den eller en mappe med LEGOLAND installeret:",
            "Vælg ISO-filen med LEGOLAND-cd'en", "Cd-images (*.iso, *.bin)", "Alle filer (*.*)",
            "Det er hverken LEGOLAND-cd'en eller en LEGOLAND-mappe.\n\nCd'en har main.z, Legoland.res og mappen "
            "Speech; et installeret LEGOLAND har IMusic, FMV, speech og volumes. Brug \"Vælg ISO\" til en ISO-fil.",
            "Det er ikke en kopi af LEGOLAND-cd'en.\n\nCd'ens kopi er en .iso-fil (eller .bin-fil) med main.z, "
            "Legoland.res og mappen Speech.",
            "Spilfilerne lægges ved siden af dette program, i\n%s\nmen der kan ikke skrives i den mappe.\n\nFlyt "
            "LEGOLAND-mappen et sted hen, hvor du kan skrive, f.eks. Dokumenter, og start det igen.",
            "Spilfilerne fylder ca. %d MB, og drevet med\n%s\nhar mindre ledig plads.",
            "Spilfilerne kunne ikke kopieres.\n\n%s", "Sprog:"}},
    {"Svenska",
        LANG_SWEDISH,
        {"LEGOLAND behöver spelfilerna från cd-skivan. Sätt i LEGOLAND-skivan, eller välj dess ISO-fil (den "
         "behöver inte monteras) eller en mapp med en kopia av skivan eller med LEGOLAND installerat.\n\nFilerna "
         "(cirka 400 MB) kopieras en gång bredvid det här programmet.",
            "Ingen LEGOLAND-skiva hittades. Sätt i den, eller välj dess ISO-fil eller en mapp.",
            "Installera från: %s (%s)", "LEGOLAND-skivan", "en avbild av LEGOLAND-skivan",
            "ett installerat LEGOLAND", "Installera", "Välj ISO...", "Välj mapp...", "Avsluta", "Avbryt",
            "Spelfilerna kopieras...",
            "Välj LEGOLAND-skivan, en kopia av den eller en mapp med LEGOLAND installerat:",
            "Välj ISO-filen med LEGOLAND-skivan", "Cd-avbilder (*.iso, *.bin)", "Alla filer (*.*)",
            "Det är varken LEGOLAND-skivan eller en LEGOLAND-mapp.\n\nSkivan har main.z, Legoland.res och mappen "
            "Speech; ett installerat LEGOLAND har IMusic, FMV, speech och volumes. Använd \"Välj ISO\" för en "
            "ISO-fil.",
            "Det är ingen avbild av LEGOLAND-skivan.\n\nSkivans avbild är en .iso-fil (eller .bin-fil) med "
            "main.z, Legoland.res och mappen Speech.",
            "Spelfilerna hamnar bredvid det här programmet, i\n%s\nmen det går inte att skriva i den mappen.\n\n"
            "Flytta LEGOLAND-mappen till ett ställe där du kan skriva, till exempel Dokument, och starta det igen.",
            "Spelfilerna behöver cirka %d MB, och enheten med\n%s\nhar mindre ledigt utrymme.",
            "Spelfilerna kunde inte kopieras.\n\n%s", "Språk:"}},
    {"Norsk",
        LANG_NORWEGIAN,
        {"LEGOLAND trenger spillfilene fra CD-en. Sett inn LEGOLAND-CD-en, eller velg ISO-filen (den trenger ikke "
         "å monteres) eller en mappe med en kopi av CD-en eller med LEGOLAND installert.\n\nFilene (omtrent "
         "400 MB) kopieres én gang ved siden av dette programmet.",
            "Fant ingen LEGOLAND-CD. Sett den inn, eller velg ISO-filen eller en mappe.",
            "Installer fra: %s (%s)", "LEGOLAND-CD-en", "et bilde av LEGOLAND-CD-en", "et installert LEGOLAND",
            "Installer", "Velg ISO...", "Velg mappe...", "Avslutt", "Avbryt", "Spillfilene kopieres...",
            "Velg LEGOLAND-CD-en, en kopi av den eller en mappe med LEGOLAND installert:",
            "Velg ISO-filen med LEGOLAND-CD-en", "CD-bilder (*.iso, *.bin)", "Alle filer (*.*)",
            "Det er verken LEGOLAND-CD-en eller en LEGOLAND-mappe.\n\nCD-en har main.z, Legoland.res og mappen "
            "Speech; et installert LEGOLAND har IMusic, FMV, speech og volumes. Bruk «Velg ISO» for en ISO-fil.",
            "Det er ikke et bilde av LEGOLAND-CD-en.\n\nCD-bildet er en .iso-fil (eller .bin-fil) med main.z, "
            "Legoland.res og mappen Speech.",
            "Spillfilene legges ved siden av dette programmet, i\n%s\nmen det går ikke an å skrive i den mappen."
            "\n\nFlytt LEGOLAND-mappen til et sted der du kan skrive, for eksempel Dokumenter, og start det på "
            "nytt.",
            "Spillfilene trenger omtrent %d MB, og disken med\n%s\nhar mindre ledig plass.",
            "Spillfilene kunne ikke kopieres.\n\n%s", "Språk:"}},
    {"Čeština",
        LANG_CZECH,
        {"LEGOLAND potřebuje herní soubory z CD. Vložte CD LEGOLAND, nebo vyberte jeho obraz ISO (není třeba ho "
         "připojovat), nebo složku s kopií CD či s nainstalovaným LEGOLANDem.\n\nSoubory (asi 400 MB) se jednou "
         "zkopírují vedle tohoto programu.",
            "CD LEGOLAND nebylo nalezeno. Vložte ho, nebo vyberte jeho obraz ISO nebo složku.",
            "Instalovat z: %s (%s)", "CD LEGOLAND", "obraz CD LEGOLAND", "nainstalovaný LEGOLAND", "Instalovat",
            "Vybrat ISO...", "Vybrat složku...", "Konec", "Zrušit", "Kopírují se herní soubory...",
            "Vyberte CD LEGOLAND, jeho kopii nebo složku s nainstalovaným LEGOLANDem:",
            "Vyberte obraz ISO CD LEGOLAND", "Obrazy CD (*.iso, *.bin)", "Všechny soubory (*.*)",
            "To není CD LEGOLAND ani složka LEGOLAND.\n\nCD obsahuje main.z, Legoland.res a složku Speech; "
            "nainstalovaný LEGOLAND má IMusic, FMV, speech a volumes. Pro obraz ISO použijte „Vybrat ISO“.",
            "To není obraz CD LEGOLAND.\n\nObraz CD je soubor .iso (nebo .bin) s main.z, Legoland.res a složkou "
            "Speech.",
            "Herní soubory patří vedle tohoto programu, do\n%s\nale do této složky nelze zapisovat.\n\nPřesuňte "
            "složku LEGOLAND někam, kam můžete zapisovat, například do Dokumentů, a spusťte ho znovu.",
            "Herní soubory potřebují asi %d MB a na disku se složkou\n%s\nje méně volného místa.",
            "Herní soubory se nepodařilo zkopírovat.\n\n%s", "Jazyk:"}},
};
#define LANGUAGES ((int)(sizeof(setup_texts) / sizeof(setup_texts[0])))

static HWND window;
static HWND intro_text;
static HWND source_text;
static HWND language_label;
static HWND language_box;
static HWND install_button;
static HWND choose_button;
static HWND iso_button;
static HWND quit_button;
static HWND progress_bar;
static HWND file_text;
static int dpi = 96;
static int finished; /* 1 installed, -1 quit */
static int language;
static char source[PATH_SIZE]; /* UTF-8 */

/* the install, on its own thread */
static CRITICAL_SECTION lock;
static HANDLE worker;
static volatile LONG cancel;
static unsigned long long progress_done, progress_total;
static char progress_file[PATH_SIZE];
static int result;
static char error[512];

static int S(int v) {
    return MulDiv(v, dpi, 96);
}

static const char *T(int id) {
    return setup_texts[language].text[id];
}

/* UTF-8 to UTF-16, in one of a few rotating buffers (enough for one call's arguments) */
static const WCHAR *Wide(const char *utf8) {
    static WCHAR buffers[4][2048];
    static int next;
    WCHAR *w = buffers[next];

    next = (next + 1) % 4;
    if (MultiByteToWideChar(CP_UTF8, 0, utf8, -1, w, 2048) == 0) {
        w[0] = 0;
    }
    return w;
}

static void Utf8(const WCHAR *w, char *out, int size) {
    if (WideCharToMultiByte(CP_UTF8, 0, w, -1, out, size, NULL, NULL) == 0) {
        out[0] = '\0';
    }
}

static void Message(const char *text) {
    MessageBoxW(window, Wide(text), L"LEGOLAND", MB_OK | MB_ICONWARNING);
}

static void ShowSource(void) {
    char text[PATH_SIZE + 256];

    if (source[0] == '\0') {
        SetWindowTextW(source_text, Wide(T(T_NONE)));
    } else {
        int kind = PortSetupKind(source);

        snprintf(text, sizeof(text), T(T_FROM), source,
            T(kind == PORT_SETUP_CD            ? T_KIND_CD
                    : kind == PORT_SETUP_IMAGE ? T_KIND_IMAGE
                                               : T_KIND_INSTALLED));
        SetWindowTextW(source_text, Wide(text));
    }
    EnableWindow(install_button, source[0] != '\0' && worker == NULL);
}

/* every text of the window in the chosen language */
static void ShowTexts(void) {
    SetWindowTextW(intro_text, Wide(T(T_INTRO)));
    SetWindowTextW(language_label, Wide(T(T_LANGUAGE)));
    SetWindowTextW(install_button, Wide(T(T_INSTALL)));
    SetWindowTextW(iso_button, Wide(T(T_CHOOSE_ISO)));
    SetWindowTextW(choose_button, Wide(T(T_CHOOSE_FOLDER)));
    SetWindowTextW(quit_button, Wide(T(worker != NULL ? T_CANCEL : T_QUIT)));
    if (worker != NULL) {
        SetWindowTextW(source_text, Wide(T(T_COPYING)));
    } else {
        ShowSource();
    }
}

/* Windows' display language, if the setup has it; else English */
static int SystemLanguage(void) {
    WORD primary = PRIMARYLANGID(GetUserDefaultUILanguage());
    int i;

    for (i = 0; i < LANGUAGES; i++) {
        if (setup_texts[i].primary == primary) {
            return i;
        }
    }
    return 0;
}

/* a CD drive (a mounted ISO is one too) with the LEGOLAND CD */
static void FindCd(void) {
    DWORD drives = GetLogicalDrives();
    char root[4] = "A:\\";
    int i;

    source[0] = '\0';
    for (i = 0; i < 26; i++) {
        if (drives & (1u << i)) {
            root[0] = (char)('A' + i);
            if (GetDriveTypeA(root) == DRIVE_CDROM && PortSetupKind(root) == PORT_SETUP_CD) {
                strcpy(source, root);
                return;
            }
        }
    }
}

static void ChooseFolder(void) {
    BROWSEINFOW bi;
    LPITEMIDLIST item;
    WCHAR wpath[MAX_PATH];
    WCHAR title[512];
    char path[PATH_SIZE];

    lstrcpynW(title, Wide(T(T_FOLDER_TITLE)), 512);
    memset(&bi, 0, sizeof(bi));
    bi.hwndOwner = window;
    bi.lpszTitle = title;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    item = SHBrowseForFolderW(&bi);
    if (item == NULL) {
        return;
    }
    if (SHGetPathFromIDListW(item, wpath)) {
        Utf8(wpath, path, sizeof(path));
        if (PortSetupKind(path) == PORT_SETUP_NONE) {
            Message(T(T_NOT_FOLDER));
        } else {
            strcpy(source, path);
        }
    }
    CoTaskMemFree(item);
    ShowSource();
}

/* an image file of the CD, read as it is (no need to mount it) */
static void ChooseIso(void) {
    OPENFILENAMEW ofn;
    WCHAR wpath[MAX_PATH] = L"";
    WCHAR filter[512];
    WCHAR title[256];
    char path[PATH_SIZE];
    int n;

    /* "CD images (*.iso, *.bin)\0*.iso;*.bin\0All files (*.*)\0*.*\0\0" */
    lstrcpynW(filter, Wide(T(T_ISO_FILTER)), 200);
    n = lstrlenW(filter) + 1;
    lstrcpyW(filter + n, L"*.iso;*.bin");
    n += lstrlenW(filter + n) + 1;
    lstrcpynW(filter + n, Wide(T(T_ALL_FILES)), 200);
    n += lstrlenW(filter + n) + 1;
    lstrcpyW(filter + n, L"*.*");
    n += lstrlenW(filter + n) + 1;
    filter[n] = 0;
    lstrcpynW(title, Wide(T(T_ISO_TITLE)), 256);
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = window;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = wpath;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameW(&ofn)) {
        return;
    }
    Utf8(wpath, path, sizeof(path));
    if (PortSetupKind(path) != PORT_SETUP_IMAGE) {
        Message(T(T_NOT_ISO));
    } else {
        strcpy(source, path);
    }
    ShowSource();
}

static void Report(void *ctx, unsigned long long done, unsigned long long total, const char *file) {
    (void)ctx;
    EnterCriticalSection(&lock);
    progress_done = done;
    progress_total = total;
    strncpy(progress_file, file, sizeof(progress_file) - 1);
    LeaveCriticalSection(&lock);
}

static int Cancelled(void *ctx) {
    (void)ctx;
    return cancel != 0;
}

static DWORD WINAPI Work(LPVOID param) {
    PortSetupProgress progress = {Report, Cancelled, NULL};
    char target[PATH_SIZE];

    (void)param;
    strncpy(target, PortSetupExeFolder(), sizeof(target) - 1);
    result = PortSetupInstall(source, target, &progress, error, sizeof(error));
    return 0;
}

/* the exe's folder can take the data: writable, with room */
static int TargetUsable(void) {
    char text[PATH_SIZE + 512];
    WCHAR folder[MAX_PATH];
    WCHAR test[MAX_PATH + 32];
    ULARGE_INTEGER free_bytes;
    HANDLE h;

    lstrcpynW(folder, Wide(PortSetupExeFolder()), MAX_PATH);
    lstrcpyW(test, folder);
    lstrcatW(test, L"setup-test.tmp");
    h = CreateFileW(test, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        snprintf(text, sizeof(text), T(T_NOT_WRITABLE), PortSetupExeFolder());
        Message(text);
        return 0;
    }
    CloseHandle(h);
    if (GetDiskFreeSpaceExW(folder, &free_bytes, NULL, NULL) && free_bytes.QuadPart < (ULONGLONG)NEEDED_MB << 20) {
        snprintf(text, sizeof(text), T(T_NO_SPACE), NEEDED_MB, PortSetupExeFolder());
        Message(text);
        return 0;
    }
    return 1;
}

static void StartInstall(void) {
    if (!TargetUsable()) {
        return;
    }
    PortTrace("setup: installing from %s into %s", source, PortSetupExeFolder());
    cancel = 0;
    progress_done = progress_total = 0;
    progress_file[0] = '\0';
    EnableWindow(install_button, FALSE);
    EnableWindow(choose_button, FALSE);
    EnableWindow(iso_button, FALSE);
    ShowWindow(progress_bar, SW_SHOW);
    ShowWindow(file_text, SW_SHOW);
    worker = CreateThread(NULL, 0, Work, NULL, 0, NULL);
    ShowTexts();
    SetTimer(window, TIMER_PROGRESS, 100, NULL);
}

static void Poll(void) {
    char text[PATH_SIZE + 600];
    unsigned long long done, total;

    EnterCriticalSection(&lock);
    done = progress_done;
    total = progress_total;
    snprintf(text, sizeof(text), "%s", progress_file);
    LeaveCriticalSection(&lock);
    SendMessageW(progress_bar, PBM_SETPOS, total ? (WPARAM)(done * 1000 / total) : 0, 0);
    SetWindowTextW(file_text, Wide(text));
    if (WaitForSingleObject(worker, 0) != WAIT_OBJECT_0) {
        return;
    }
    KillTimer(window, TIMER_PROGRESS);
    CloseHandle(worker);
    worker = NULL;
    if (result) {
        PortTrace("setup: done");
        finished = 1;
        return;
    }
    PortTrace("setup: failed: %s", error);
    if (!cancel) {
        snprintf(text, sizeof(text), T(T_FAILED), error);
        Message(text);
    }
    ShowWindow(progress_bar, SW_HIDE);
    ShowWindow(file_text, SW_HIDE);
    EnableWindow(choose_button, TRUE);
    EnableWindow(iso_button, TRUE);
    ShowTexts();
}

static LRESULT CALLBACK SetupProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_COMMAND:
        switch (LOWORD(wparam)) {
        case ID_INSTALL:
            StartInstall();
            return 0;
        case ID_CHOOSE:
            ChooseFolder();
            return 0;
        case ID_CHOOSE_ISO:
            ChooseIso();
            return 0;
        case ID_LANGUAGE:
            if (HIWORD(wparam) == CBN_SELCHANGE) {
                int chosen = (int)SendMessageW(language_box, CB_GETCURSEL, 0, 0);

                if (chosen >= 0 && chosen < LANGUAGES) {
                    language = chosen;
                    ShowTexts();
                }
            }
            return 0;
        case IDCANCEL:
            if (worker != NULL) {
                cancel = 1;
            } else {
                finished = -1;
            }
            return 0;
        }
        break;
    case WM_TIMER:
        if (wparam == TIMER_PROGRESS) {
            Poll();
        }
        return 0;
    case WM_DEVICECHANGE:
        /* a CD inserted or an ISO mounted while the window is open (a chosen image stays chosen) */
        if (worker == NULL && PortSetupKind(source) != PORT_SETUP_IMAGE) {
            FindCd();
            ShowSource();
        }
        break;
    case WM_CLOSE:
        if (worker != NULL) {
            cancel = 1;
        } else {
            finished = -1;
        }
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

static HWND Control(const WCHAR *cls, DWORD style, int x, int y, int w, int h, int id, HFONT font) {
    HWND hwnd = CreateWindowExW(0, cls, L"", WS_CHILD | WS_VISIBLE | style, S(x), S(y), S(w), S(h), window,
        (HMENU)(INT_PTR)id, GetModuleHandleW(NULL), NULL);
    SendMessageW(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
    return hwnd;
}

int PortSetupRun(HINSTANCE instance) {
    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES};
    NONCLIENTMETRICSW metrics;
    WNDCLASSEXW wc;
    HFONT font;
    RECT frame;
    MSG msg;
    DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    HDC screen = GetDC(NULL);
    int i;

    if (screen != NULL) {
        dpi = GetDeviceCaps(screen, LOGPIXELSY);
        ReleaseDC(NULL, screen);
    }
    InitCommonControlsEx(&icc);
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); /* the folder dialog */
    InitializeCriticalSection(&lock);
    memset(&metrics, 0, sizeof(metrics));
    metrics.cbSize = sizeof(metrics);
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0);
    font = CreateFontIndirectW(&metrics.lfMessageFont);

    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = SetupProc;
    wc.hInstance = instance;
    wc.hIcon = LoadIconW(instance, (LPCWSTR)0x65);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"LEGOLANDSETUP";
    RegisterClassExW(&wc);

    SetRect(&frame, 0, 0, S(480), S(250));
    AdjustWindowRect(&frame, style, FALSE);
    window = CreateWindowExW(WS_EX_DLGMODALFRAME, L"LEGOLANDSETUP", L"LEGOLAND", style, CW_USEDEFAULT,
        CW_USEDEFAULT, frame.right - frame.left, frame.bottom - frame.top, NULL, NULL, instance, NULL);
    if (window == NULL) {
        DeleteObject(font);
        return 0;
    }
    {
        RECT work;
        if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0)) {
            SetWindowPos(window, NULL, work.left + (work.right - work.left - (frame.right - frame.left)) / 2,
                work.top + (work.bottom - work.top - (frame.bottom - frame.top)) / 2, 0, 0,
                SWP_NOSIZE | SWP_NOZORDER);
        }
    }
    intro_text = Control(L"STATIC", 0, 16, 14, 448, 80, -1, font);
    source_text = Control(L"STATIC", SS_PATHELLIPSIS, 16, 102, 448, 18, ID_SOURCE, font);
    progress_bar = Control(PROGRESS_CLASSW, 0, 16, 126, 448, 18, ID_PROGRESS, font);
    SendMessageW(progress_bar, PBM_SETRANGE32, 0, 1000);
    file_text = Control(L"STATIC", SS_PATHELLIPSIS, 16, 148, 448, 18, ID_FILE, font);
    ShowWindow(progress_bar, SW_HIDE);
    ShowWindow(file_text, SW_HIDE);
    language_label = Control(L"STATIC", SS_RIGHT, 16, 179, 260, 18, -1, font);
    language_box = Control(L"COMBOBOX", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 288, 176, 176, 240,
        ID_LANGUAGE, font);
    language = SystemLanguage();
    for (i = 0; i < LANGUAGES; i++) {
        SendMessageW(language_box, CB_ADDSTRING, 0, (LPARAM)Wide(setup_texts[i].name));
    }
    SendMessageW(language_box, CB_SETCURSEL, (WPARAM)language, 0);
    install_button = Control(L"BUTTON", BS_DEFPUSHBUTTON | WS_GROUP | WS_TABSTOP, 16, 208, 109, 28, ID_INSTALL, font);
    iso_button = Control(L"BUTTON", WS_TABSTOP, 129, 208, 109, 28, ID_CHOOSE_ISO, font);
    choose_button = Control(L"BUTTON", WS_TABSTOP, 242, 208, 109, 28, ID_CHOOSE, font);
    quit_button = Control(L"BUTTON", WS_TABSTOP, 355, 208, 109, 28, IDCANCEL, font);
    FindCd();
    ShowTexts();
    ShowWindow(window, SW_SHOW);
    SetForegroundWindow(window);

    finished = 0;
    while (finished == 0 && GetMessageW(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessageW(window, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    DestroyWindow(window);
    window = NULL;
    UnregisterClassW(L"LEGOLANDSETUP", instance);
    DeleteObject(font);
    DeleteCriticalSection(&lock);
    CoUninitialize();
    return finished == 1 && PortSetupFindData();
}
