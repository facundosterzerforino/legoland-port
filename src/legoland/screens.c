#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma intrinsic(strcpy, strlen)
#include "debug.h"
#include "globals.h"
#include "legoland.h"
#ifdef LEGOLAND_PORT
#include "ext_actions.h"
#endif

#include "bloke.h"
#include "bloke_ai.h"
#include "bricks.h"
#include "build.h"
#include "challenge.h"
#include "clipping.h"
#include "controller.h"
#include "draw.h"
#include "freeplay.h"
#include "game_util.h"
#include "gamemap.h"
#include "help.h"
#include "icon.h"
#include "input.h"
#include "interface.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "mapscreen.h"
#include "math.h"
#include "nerps.h"
#include "obj_instance.h"
#include "objclass.h"
#include "options.h"
#include "popupinfo.h"
#include "print_sprite.h"
#include "profile.h"
#include "profile_io.h"
#include "progress.h"
#include "savegame_ui.h"
#include "saveload.h"
#include "screens.h"
#include "sound_music.h"
#include "sound_sfx.h"
#include "string.h"
#include "text.h"
#include "tilemap.h"
#include "timer.h"
#include "title.h"
#include "tooltip.h"
#include "wndenv.h"
#include "worker.h"

struct ScreenConfig {
    unsigned short width;
    unsigned short height;
    unsigned char pad_4[0x28 - 0x4];
    unsigned int slot;
};

#include "image_sprite.h"
#include "stream.h"
#include "worker_mouse.h"
#ifdef LEGOLAND_PORT
#include "extensions.h"
#endif

// FUNCTION: LEGOLAND 0x004585c0
void CloseFrontEndScreen(void) {
    if (SPRITE_TitleScreenBk != NULL) {
        KillSprite(SPRITE_TitleScreenBk);
        SPRITE_TitleScreenBk = NULL;
    }
    RemoveIconGroup(7);
    if (DAT_0080ff80.unk4 != 0xffffffff) {
        switch (DAT_0080ff80.unk4) {
        case 0:
            FUN_0048d230();
            UpdateSoundVols();
            DeleteProfileList();
            KillListProfileSprite();
            break;
        case 3:
            CleanUpFreePlay();
            break;
        case 4:
            KillSaveScreenSprites();
            KillTitleScreenSprites();
            DeleteSavedGameList();
            break;
        default:
            break;
        }
        DAT_006687bc = 0;
        DAT_006687c0 = 0;
    }
}

// FUNCTION: LEGOLAND 0x00458640
LEGO_EXPORT void InitScreens(unsigned char param_1) {
    int id = (char)param_1;

    if (DAT_0080ff80.unk4 != (unsigned int)id) {
        SpeechCloseFile();
        DAT_006687b0 = 4;
        CloseFrontEndScreen();
        DAT_0080ff80.unk0 = 0;
        switch (id) {
        case 0:
            InitListProfiles();
            break;
        case 1:
            DeletePlayableSamples(0);
            InitTitleScreen();
            break;
        case 5:
            SamplesPaused = 1;
            PauseAllSamples();
            InitOptionScreen();
            break;
        case 4:
            InitSavedGameScreen();
            break;
        case 3:
            InitFreePlayScreen();
            break;
        case 6:
            DeletePlayableSamples(0);
            InitProgressScreen();
            break;
        case 7:
            SamplesPaused = 1;
            PauseAllSamples();
            InitIntervalScreen();
            break;
        case 8:
            InitCertificateScreen();
            break;
        case 9:
            InitAdvertScreen();
            break;
        default:
            break;
        }
        DAT_0080ff80.unk4 = id;
    }
    if (SPRITE_TitleScreenBk != 0) {
        RenderFrontEndScreen(param_1);
    }
}

// FUNCTION: LEGOLAND 0x00458740
void RenderFrontEndScreen(unsigned char param_1) {
    ResetHitInfo();
    PushRenderingStatusAndLockVideoSurface();
    PrintSprite(SPRITE_TitleScreenBk, 0, 0, 0, 0);
    if (DAT_00668e38 != 0) {
        RenderIcons2(7, 0xe, 0);
        if (DAT_0080ff80.unk8 == 6) {
            RenderIcons2(0x1c, 0x23, 0);
        }
    } else {
        RenderIcons();
    }
    switch (DAT_0080ff80.unk8) {
    case 0:
        PrintProfileDetails();
        break;
    case 4:
        PrintSavedGameDetails();
        break;
    case 1:
    case 5:
        PrintExitCheckBox();
        break;
    case 7:
        RenderIntervalScreen();
        break;
    case 8:
        RenderCertificateScreen();
        break;
    case 6:
        FUN_0048c100();
        break;
    default:
        break;
    }
    PlayPendingFrontEndSpeech();
    ProcessFrontEndHelp();
    UpdateFocussedIconPtr();
    PopRenderingStatus();
    if (FocussedIconPtr != 0) {
        SetPointer(6);
    }
    CheckFocussedIcon();
    RenderingComplete();
}

// FUNCTION: LEGOLAND 0x00458830
void GetProductVersion(char *param_1) {
    DWORD size;
    DWORD handle;
    char *value;
    char *buf;

    // STRING: LEGOLAND 0x004b912c
    size = GetFileVersionInfoSizeA("Legoland.exe", &handle);
    buf = (char *)malloc(size);
    GetFileVersionInfoA("Legoland.exe", handle, size, buf);
    // STRING: LEGOLAND 0x004b9104
    VerQueryValueA(buf, "\\StringFileInfo\\080904B0\\ProductVersion", (void **)&value, (UINT *)&size);
    strcpy(param_1, value);
    free(buf);
}

// FUNCTION: LEGOLAND 0x004588c0
void PrintTitleScreen1(void) {
    struct ScreenConfig *config;
    struct Sprite *sprite;

    config = (struct ScreenConfig *)lpConfig;
    ScreenRect.left = 0;
    ScreenRect.top = 0;
    ScreenRect.right = config->width;
    ScreenRect.bottom = config->height;

    // STRING: LEGOLAND 0x004b913c
    sprite = LoadSprite("TitleScreen1.lls", 0);
    PushRenderingStatusAndLockVideoSurface();
    PrintSprite(sprite, 0, 0, 0, 0);
    PopRenderingStatus();
    RenderingComplete();

    if (sprite != NULL) {
        KillSprite(sprite);
    }
}

#ifndef LEGOLAND_PORT /* [library:asm] the compiler's float-to-int helper, with its argument on the x87 stack; the
                       * port's compiler converts by itself and nothing calls this */
// FUNCTION: LEGOLAND 0x00458930
int _ftol() {
    __asm fistp dword ptr[DAT_00667c3c] __asm mov eax, [DAT_00667c3c]
}
#endif

// FUNCTION: LEGOLAND 0x00458940
void ResetEditState(void) {
    EditMode.unk0 = 0;
    EditMode.unk4 = 3;
    SetDefaultMouseHandlers();
    EditMode.unk8 = 0;
    DefaultCursor(&EditCursor);
    BuildCursorPtr(&EditCursor, 0x8f8, 0);
    GamePad = (GamePad & 0xffff0000) | ((((GamePad >> 8) & 0xeb) << 8) | ((GamePad & 0xff) | 0x20));
}

// FUNCTION: LEGOLAND 0x004589a0
void InitMouse(void) {
    unsigned int work[3];

    CONTROLLERBUFFER->prev_x = 0;
    CONTROLLERBUFFER->prev_y = 0;
    CONTROLLERBUFFER->x = 0;
    CONTROLLERBUFFER->y = 0;
    CONTROLLERBUFFER->delta_x = 0;
    CONTROLLERBUFFER->delta_y = 0;
    CONTROLLERBUFFER->x = lpConfig->screen_width >> 1;
    CONTROLLERBUFFER->y = lpConfig->screen_height >> 1;
    SystemParametersInfoA(3, 0, work, 0);
    CONTROLLERBUFFER->mouse_threshold1 = work[0];
    CONTROLLERBUFFER->mouse_threshold2 = work[1];
    CONTROLLERBUFFER->mouse_accel = work[2];
    SetPointer(5);
}

// FUNCTION: LEGOLAND 0x00458a50
void StartLevelGame(void) {
    char buf[0x34];

    if (MapLoaded == 0) {
        QueryClass = 0;
        CastlePlacedFlag = 0;
        PauseGameTimer();
        ResetGameTimer();
        MarkGameTimer();
        // STRING: LEGOLAND 0x004b9150
        sprintf(buf, "objlist%d.txt", lpConfig->level);
        SetBricksLimited(0);
        ResetMapAI();
        DAT_00667c4c = FUN_0047afb0(buf);
        SetBricksLimited(1);
        AllocBlokeCounters(lpConfig->max_blokes);
        ResetEditState();
        FUN_00489ee0();
        MapStats.level_end = 0;
        UpdateMenu();
        FUN_00490600(1);
        ShowIntervalScreen(DAT_0066861c, 0);
        MapLoaded = 1;
        ResumeGameTimer();
        FUN_0048a800();
    }
}

// FUNCTION: LEGOLAND 0x00458b20
void UnloadMap(void) {
    if (MapLoaded != 0) {
        FUN_00481170();
        DestroyIconGroup(0xd2);
        DelObjectList();
        FreeBlokeCounters();
        FreeAllObjectClassInstances();
        DAT_00667cd8 = 1;
        FUN_0046cb20();
        DAT_00667cd8 = 0;
        SetBricksLimited(0);
        FUN_00489ee0();
        DestroyAllBlokes();
        RemoveAllNewObjects();
        ResetInfoStruct();
        ClearBuildObjList();
        FUN_0049cfc0();
        UnloadGameInterface();
        SetBricksLimited(1);
        GamePad = (GamePad & 0xffff00ff) | (((GamePad >> 8) & 0xeb) << 8);
        MapLoaded = 0;
    }
}

// FUNCTION: LEGOLAND 0x00458bb0
void SetMapLoaded(unsigned int param_1) { MapLoaded = param_1; }

// FUNCTION: LEGOLAND 0x00458bc0
void FUN_00458bc0(void) {
    EditMode.unk4 = 2;
    DAT_0080ff80.unk4 = 0xffffffff;
    DAT_0080ff80.unk8 = 0;
}

// FUNCTION: LEGOLAND 0x00458be0
void FUN_00458be0(void) {
    int slot = ((struct ScreenConfig *)lpConfig)->slot;

    if (slot < 0xf) {
        CurrentProfile.flags[4 + slot] = 1;
    }
    FUN_0048a750();
    UpDateCurrentProfile();
}

// FUNCTION: LEGOLAND 0x00458c00
int FUN_00458c00(void) {
    char path[256];

    SpeechStreamUpdate();
    if (MapScreenOpenPending != 0) {
        PauseGameTimer();
        PauseAllSamples();
        DAT_006687bc = 0;
        GamePad = GamePad & 0xffffffdf;
        DAT_006687c0 = 0;
        InitMapScreen();
        DisableInfoPopUPIcons();
        DisableSidePanelIcons();
        MapScreenOpenPending = 0;
    } else if (MapScreenClosePending != 0) {
        ResumeAllSamples();
        DAT_006687bc = 0;
        GamePad = GamePad | 0x20;
        DAT_006687c0 = 0;
        KillMapScreen();
        SetDefaultMouseHandlers();
        EnableSidePanelIcons();
        MapScreenClosePending = 0;
        ResumeGameTimer();
    } else if (DAT_00667c64 != 0) {
        PauseGameTimer();
        ResetGameTimer();
        MarkGameTimer();
        DAT_006687bc = 0;
        DAT_006687c0 = 0;
        CastlePlacedFlag = 0;
        SamplesPaused = 1;
        PauseAllSamples();
        if (DAT_00667c80 != 0) {
            DeletePlayableSamples(0);
            sprintf(path, "%s\\%dsave%d.sav", "profiles", CurrentProfile.profile_slot, *(unsigned int *)&CurrentProfile.save_slot & 0xff);
            LoadWatchSprite(0, 0);
            LoadGame(path);
            DAT_00667c80 = 0;
            UnloadWatchSprite();
            InitGameInterface(0);
            SetDefaultMouseHandlers();
        } else {
            FUN_0046c5c0();
            StartLevelGame();
        }
        UpdateSoundVols();
        DAT_00667c64 = 0;
        MapStats.level_end = 0;
        EditMode.unk4 = 3;
        SetDefaultMouseHandlers();
        ResumeGameTimer();
    }

    switch (EditMode.unk4) {
    case 2:
        SetPointer(5);
        InitScreens(DAT_0080ff80.unk8 & 0xff);
        break;
    case 3:
        if (SamplesPaused != 0) {
            ResumeAllSamples();
            SamplesPaused = 0;
        }
        GameProc();
        break;
    case 1:
        RenderMapScreenFrame();
        break;
    case 0:
        return 0;
    default:
        break;
    }

    if (DAT_00667c64 == 0) {
        // STRING: LEGOLAND 0x004b9160
        DAT_00667c40 = "SFX";
        UpdateSound();
        ReadGameButtons();
        FrameCounter = FrameCounter + 1;
        if (MapStats.level_end != 0 && EditMode.unk4 == 3) {
            if (MapStats.field_3ac != 0) {
                SetPointer(0);
                PlayMovie(DAT_008100c0, 1, 1);
                MapStats.field_3ac = 0;
                SetPointer(5);
            }
            if (MapStats.level_end != 3) {
                if (MapStats.level_end == 1) {
                    ((struct ScreenConfig *)lpConfig)->slot += 1;
                }
                FUN_0048a750();
                UnloadMap();
                DAT_00668e38 = 1;
                EditMode.unk4 = 2;
                DAT_0080ff80.unk4 = 0xffffffff;
                DAT_0080ff80.unk8 = 6;
                UpdateSpeechPlayback();
                return 1;
            }
            DAT_00668e38 = 0;
            EditMode.unk4 = 2;
            DAT_0080ff80.unk4 = 0xffffffff;
            DAT_0080ff80.unk8 = 1;
            UnloadMap();
        }
        UpdateSpeechPlayback();
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00458ee0
void GameProc(void) {
    struct HitInfo hit = {1};
    RECT help_rect;
    struct HoverInfo saved;

    // STRING: LEGOLAND 0x004b91e4
    DAT_00667c40 = "AI";
    HandleRideAI();
    DoMapAI();
    ControlPeople();
    ControlWorkers();
    if (DAT_00668954 != 0) {
        CheckWorkerOnMouseStatus(0);
    }
    // STRING: LEGOLAND 0x004b91d4
    DAT_00667c40 = "ProcessStuff";
    ProcessBuildingTimes();
    ProcessDamage();
    saved.type = Hover.type;
    SetPointer(5);
    // STRING: LEGOLAND 0x004b91cc
    DAT_00667c40 = "Zoning";
    ResetHitInfo();
    // STRING: LEGOLAND 0x004b91c0
    DAT_00667c40 = "Rendering";
    PushRenderingStatusAndLockVideoSurface();
    RenderView();
    if ((saved.type & 0x100) != 0 || (GamePad & 0x1000) != 0) {
        FUN_00457a70();
    }
    saved = Hover;
    PrintSprite(InterfaceBgSprite, 0, 0, 0, &hit.type);
    FUN_0046f100(0x2c3);
    FUN_0046ee00();
    // STRING: LEGOLAND 0x004b91b0
    DAT_00667c40 = "In Game Help";
    ProcessInGameHelp();
    DrawPopUpInfo();
    RenderIcons2(0x2c3, 0, 0);
    FUN_0046cff0();
    if ((GamePad & 0x1000) != 0) {
        Hover = saved;
    }
    if ((Hover.type & 0x100) != 0 && (EditMode.unk0 == 0 || EditMode.unk0 == 2)) {
        help_rect.left = MousePos.x;
        help_rect.top = MousePos.y - 0xa;
        help_rect.right = MousePos.x;
        help_rect.bottom = MousePos.y;
        if (EditMode.unk0 == 0) {
            switch (Hover.type) {
            case 0x103:
                SetPointer(8);
                if (QueryClass != NULL) {
                    HTBubbleHelp(&help_rect, QueryClass->name, 2);
                    FUN_0046d340(QueryClass->element->name);
                }
                break;
            case 0x10a:
                HTBubbleHelp(&help_rect, GetString(0xd4), 2);
                FUN_0046d230(0xd4);
                SetPointer(8);
                break;
            case 0x10b:
                HTBubbleHelp(&help_rect, GetString(0xd2), 2);
                FUN_0046d230(0xd2);
                SetPointer(7);
                break;
            case 0x10c:
                HTBubbleHelp(&help_rect, GetString(0xd3), 2);
                FUN_0046d230(0xd3);
                SetPointer(7);
                break;
            case 0x10d:
                HTBubbleHelp(&help_rect, GetString(0x7e4), 2);
                FUN_0046d230(0x7e4);
                SetPointer(8);
                break;
            case 0x306:
                FUN_00455fc0(&help_rect, GetVisitorName(Hover.ptr), 2, FUN_00482cb0(Hover.ptr));
                SetPointer(8);
                FUN_00450a40(Hover.ptr);
                break;
            case 0x307:
                HTBubbleHelp(&help_rect, GetString(0x90), 2);
                FUN_0046d230(0x90);
                SetPointer(8);
                FUN_00450a40(Hover.ptr);
                break;
            case 0x308:
                HTBubbleHelp(&help_rect, GetString(0x92), 2);
                FUN_0046d230(0x92);
                SetPointer(8);
                FUN_00450a40(Hover.ptr);
                break;
            default:
                SetPointer(7);
                break;
            }
        } else {
            if (QueryClass != NULL) {
                HTBubbleHelp(&help_rect, QueryClass->name, 2);
                FUN_0046d340(QueryClass->element->name);
            }
        }
    }
    if (Hover.type == 5 && (MouseClickFlags & 2) != 0) {
        ClearAdvisorHelp();
        DAT_00667c48 = 1;
    }
    // STRING: LEGOLAND 0x004b91a4
    DAT_00667c40 = "Appraisals";
    CheckAppraisalDue();
    // STRING: LEGOLAND 0x004b9194
    DAT_00667c40 = "Appraisals Over";
    UpdateFocussedIconPtr();
    if ((GamePad & 0x1000) == 0 && DAT_00667c48 == 0) {
        if (FocussedIconPtr != NULL && DAT_00668954 == 0) {
            SetPointer(6);
        }
        CheckFocussedIcon();
    }
    if (DAT_00668954 != 0) {
        RenderWorkerOnMouse();
    }
    PopRenderingStatus();
    if (MapStats.show_capacity != 0) {
        if (IsLeftShiftDown() != 0 || IsRightShiftDown() != 0) {
            PrintCapacityStats();
        }
    }
    RenderingComplete();
    // STRING: LEGOLAND 0x004b9180
    DAT_00667c40 = "Exiting GameProc";
}

// FUNCTION: LEGOLAND 0x00459360
void RenderMapScreenFrame(void) {
    unsigned int var_4;
    struct HitInfo hit_info;
    RECT clipping;
    int ticks;

    var_4 = 1;
    hit_info.type = 0;
    hit_info.field_4 = 0;
    ResetHitInfo();
    PushRenderingStatusAndLockVideoSurface();
    DrawMapScreen();
    SetPointer(5);
    PrintSprite(InterfaceBgSprite, 0, 0, 0, &hit_info.type);
    FUN_0046ee00();
    RenderIcons();
    CheckFocussedIcon();
    if (Hover.type == 2) {
        SetPointer(6);
        UpdateFocussedIconPtr();
        PopRenderingStatus();
        RenderingComplete();
        return;
    }
    if ((Hover.type & 0x100) != 0) {
        RECT old_clipping = {0, 0x20, 0x280, 0x174};

        GetClipping(&clipping);
        SetClipping(&old_clipping);
        RenderMouseBounds();
        SetClipping(&clipping);
        if ((MouseClickFlags & var_4) != 0) {
            MapScreenSetScrollPos(&MousePos);
        }
        if ((MouseClickFlags & 2) != 0) {
            ticks = GetTicks();
            if (ticks - LastMapClickTicks < 0x1f4 && abs(MousePos.x - LastMapClickX) < 5 &&
                abs(MousePos.y - LastMapClickY) < 5) {
                MapScreenClosePending = var_4;
                EditMode.unk4 = PreMapScreenMode;
                PreMapScreenMode = var_4;
            }
            LastMapClickX = MousePos.x;
            LastMapClickY = MousePos.y;
            LastMapClickTicks = ticks;
        }
    }
    UpdateFocussedIconPtr();
    PopRenderingStatus();
    RenderingComplete();
}

// FUNCTION: LEGOLAND 0x004594e0
void FUN_004594e0(void) {
    switch (EditMode.unk4) {
    case 2:
        CloseFrontEndScreen();
        break;
    default:
        break;
    }
}

// FUNCTION: LEGOLAND 0x004594f0
void InitDefaultProfile(void) {
    int i;

    for (i = 0; i < 68; i = i + 1) {
        ((unsigned int *)&CurrentProfile)[i] = 0;
    }

    CurrentProfile.speech_volume = 0x4b;
    CurrentProfile.music_volume = 0x64;
    CurrentProfile.fx_volume = 0x4b;
}

// FUNCTION: LEGOLAND 0x00459520
void FUN_00459520(void) {
    HMODULE ir50;
    MSG msg;

    // STRING: LEGOLAND 0x004b7138
    HedgeObjectClass = ElemID("HEDGE")->data;
    InitDefaultProfile();
    InitSoundSystem();
    DebugTrace("boot: after InitSoundSystem()");
    SetMusicGrooveLevel(1);
    SuspendMusicThread();
    SetupControllers();
    DebugTrace("boot: after SetupControllers()");
    LLIDB_ClearOnLevel();
    InitMouse();
    DebugTrace("boot: after InitMouse()");
    SetPointer(0);
    ProcessSystemEvents();
    DebugTrace("boot: after ProcessSystemEvents()");
    // STRING: LEGOLAND 0x004b9200
#ifdef LEGOLAND_PORT
    PlayMovie("lmi.avi", ExtSkipLogo, 1); /* [port] extensions/: a click skips the logo (-no-skip-logo: original) */
#else
    PlayMovie("lmi.avi", 0, 1);
#endif
    DebugTrace("boot: after PlayMovie lmi.avi");
    PrintTitleScreen1();
    DebugTrace("boot: after PrintTitleScreen1()");
    ResumeMusicThread();
    LoadWatchSprite(0, 0);
    DebugTrace("boot: after LoadWatchSprite(0, 0)");

    DebugTrace("boot: waiting for lmi.avi, DAT_007988bc=%d", DAT_007988bc);
    while (DAT_007988bc == 0) {
        PeekMessageA(&msg, NULL, 0, 0, 0);
        Sleep(100);
        DrawWatchSprite();
    }

    LoadGBarSprites();
    DebugTrace("boot: after LoadGBarSprites()");
    LoadWorkerInterfaceGFX();
    LoadBubbleHelpGFX();
    InitialiseBlokes();
    DebugTrace("boot: after InitialiseBlokes()");
    InitGameMap();
    DebugTrace("boot: after InitGameMap()");
    SetPointer(0);
    // STRING: LEGOLAND 0x004b91f4
    PlayMovie("Intro.avi", 1, 0);
    DebugTrace("boot: after PlayMovie Intro.avi");
    FUN_00492ca0(0);
    SetPointer(5);
    FrameCounter = 0;
    Load_Interface_ControlIcons();
    DebugTrace("boot: after Load_Interface_ControlIcons()");
    DrawWatchSprite();
    Load_Interface_ThemeIcons();
    FreeTileSpace(0, 0x800);
    EditMode.unk4 = 3;
    LoadMapTiles();
    DebugTrace("boot: after LoadMapTiles()");
    DrawWatchSprite();
    InitMan();
    DebugTrace("boot: after InitMan()");
    DrawWatchSprite();
    CreateObjectClasses();
    DebugTrace("boot: after CreateObjectClasses()");
    DrawWatchSprite();
    FUN_00458bc0();
    DrawWatchSprite();
    // STRING: LEGOLAND 0x004b91e8
    ir50 = LoadLibraryA("Ir50_32.dll");
    DrawWatchSprite();
    LoadAdvisorAnims();
    DebugTrace("boot: after LoadAdvisorAnims()");
    DrawWatchSprite();
    UnloadWatchSprite();
    DebugTrace("boot: after UnloadWatchSprite()");
    ResumeMusicThread();
    DebugTrace("boot: entering main loop");
    while (FUN_00458c00() != 0) {
    }
    DebugTrace("boot: main loop ended");
    FUN_004594e0();
    SpeechCloseFile();
    if (SPRITE_TitleScreenBk != NULL) {
        KillSprite(SPRITE_TitleScreenBk);
        SPRITE_TitleScreenBk = NULL;
    }
    FreeControllers();
    UnloadGBarSprites();
    UnloadBubbleHelpGFX();
    FreeBlokePool();
    KillGameMap();
    UnLoad_Interface_ControlIcons();
    UnLoad_Interface_ThemeIcons();
    UnloadMapTiles();
    UnInitMan();
    FreeAdvisorAnims();
    FreeLibrary(ir50);
    FreeBlokeCounters();
    KillHelp();
    KillSoundSystem();
    KillInputSystem();
}

// FUNCTION: LEGOLAND 0x00459710
void FUN_00459710(char *s) {
    char buffer[0x80];
    char *semicolon;
    char *source;

    source = s;
    semicolon = strchr(source, ';');
    if (semicolon != NULL) {
        *semicolon = 0;
        strcpy(buffer, source);
        semicolon++;
        semicolon[-1] = ';';
        if (strlen(semicolon) != 0) {
            strcpy(DAT_008100c0, semicolon);
            MapStats.field_3ac = 1;
        }
    }
    FUN_00490600(0);
    if (LoadIntervalReport(buffer) != 0) {
        DAT_00668e38 = 1;
        EditMode.unk4 = 2;
        DAT_0080ff80.unk4 = 0xffffffff;
        DAT_0080ff80.unk8 = 7;
    }
}

// FUNCTION: LEGOLAND 0x004597e0
void FUN_004597e0(int param0, const char *param1) {
    char *buffer;

    if (param0) {
        buffer = MapStats.field_198;
    } else {
        buffer = MapStats.field_298;
    }

    if (param1) {
        strncpy(buffer, param1, 256);
        buffer[255] = 0;
    } else {
        buffer[0] = 0;
    }
}

// FUNCTION: LEGOLAND 0x00459820
void EndLevel(unsigned int a1) {
#ifdef LEGOLAND_PORT
    ExtAction("end the level (%u)", a1); /* [port] action log (ext_actions.h) */
#endif
    MapStats.level_end = a1;
    if (a1 == 1) {
        FUN_00459710(MapStats.field_198);
        return;
    }
    if (a1 == 2) {
        FUN_00459710(MapStats.field_298);
    }
}
