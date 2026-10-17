#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "legoland.h"
#ifdef LEGOLAND_PORT
#include "debug.h"
#endif
#include "obj_instance.h"

#include "challenge.h"
#include "help.h"
#include "icon.h"
#include "interface.h"
#include "nerps.h"
#include "objectives.h"
#include "popupinfo.h"
#include "text.h"
#include "timer.h"
#include "tooltip.h"

#include "image_sprite.h"
#include "stream.h"
#ifdef LEGOLAND_PORT
#include "ext_actions.h"
#endif

// FUNCTION: LEGOLAND 0x0046ce20
void ClearAdvisorHelp(void) {
    if ((DAT_007fe040 & 0x3) != 0) {
        free((void *)AdvisorHelpText);
        FUN_004748a0((void *)1);
    }
    DAT_007fe040 &= ~0x3;
    FUN_00476000();
    SetNextAdvisorAnimState(0, 0);
    SpeechStop();
}

// FUNCTION: LEGOLAND 0x0046ce60
LEGO_EXPORT int DisplayAdvisorHelp(char *param_1, unsigned int param_2, unsigned int param_3) {
    if ((DAT_007fe040 & 1) != 0) {
        return 0;
    }
    DAT_007fe040 |= 1;
    AdvisorHelpText = (char *)malloc(strlen(param_1) + 1);
    strcpy(AdvisorHelpText, param_1);
#ifdef LEGOLAND_PORT
    ExtAction("advisor says \"%.300s\"", param_1); /* [port] action log (ext_actions.h) */
#endif
    AdvisorHelpStartTime = GetGameTimer();
    DAT_007fe044 = param_2;
    FUN_004748a0((void *)0);
    return 1;
}

// FUNCTION: LEGOLAND 0x0046cee0
unsigned int FUN_0046cee0(void) {
    unsigned int result = GetGameTimer() - DAT_007fe050;
    if ((int)result >= 0) {
        DAT_00668618 = 0;
        return 1;
    }
    if (DAT_00668724 != NULL) {
        if (((struct ObjectiveEvent *)DAT_00668724)->type == 0) {
            if (DAT_00668618 == 0) {
                return 0;
            }
        }
    } else {
        if (DAT_00668618 == 0) {
            return 0;
        }
    }
    DAT_00668618 = 0;
    return 1;
}

// FUNCTION: LEGOLAND 0x0046cf20
unsigned int FUN_0046cf20(void) {
    int diff = (int)(GetGameTimer() - AdvisorHelpStartTime);

    if (diff > 0x7530 || (DAT_007fe044 != 0 && DAT_00668724 != NULL && ((struct ObjectiveEvent *)DAT_00668724)->type != 0)) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046cf60
LEGO_EXPORT void ProcessInGameHelp(void) {
    int delta;

#ifdef LEGOLAND_PORT
    {
        /* [port] every 10 s: the level script's state (game timer, its last run, the pending events), to debug a
         * tutorial that doesn't go on (tutorial 4, web, 2026-10-17). Remove once found. */
        static DWORD last;
        static int lines;
        DWORD now = GetTickCount();

        if (now - last >= 10000 && lines < 300) {
            char types[160];
            int n = 0;
            struct ObjectiveEvent *ev;

            last = now;
            lines++;
            types[0] = 0;
            for (ev = ScriptEventList; ev != NULL && n < (int)sizeof(types) - 8; ev = ev->next) {
                n += snprintf(types + n, sizeof(types) - n, " %d%s", (int)ev->type, (ev->flags_10 & 0x80) ? "*" : "");
            }
            DebugTrace("objectives: game timer %u%s, script ran at %u, stopped %u, events%s", GetGameTimer(),
                GameTimerPaused != 0 ? " (paused)" : "", DAT_007fe054, IsScriptStopped(), types);
        }
    }
#endif
    PostPendingObjectiveHints();

    delta = (int)(GetGameTimer() - DAT_007fe054);
    if (delta > 0xc8) {
        RunLevelScript();
        DAT_007fe054 = GetGameTimer();
    }

    if ((DAT_007fe040 & 0x1) != 0) {
        if (FUN_0046cf20() != 0) {
            ClearAdvisorHelp();
            DAT_007fe050 = GetGameTimer();
        } else {
            BubbleHelp(DAT_004b9f78, AdvisorHelpText, 2);
            DAT_007fe050 = GetGameTimer();
        }
    } else {
        if (FUN_0046cee0() != 0) {
            ShowNextObjectiveMessage();
        }
    }

    RenderHelpIcons();
    UpdateAdvicePriority();
}

// FUNCTION: LEGOLAND 0x0046cff0
void FUN_0046cff0(void) {
    RECT rect;
    struct IconNode *icon;

    icon = (struct IconNode *)FocussedIconPtr;
    if (icon != NULL && (icon->flags & 0x2000) != 0 && DAT_00668954 == 0) {
        int y = MousePos.y;
        int x = MousePos.x;
        rect.left = x;
        rect.top = y - 10;
        rect.right = x;
        rect.bottom = y;
        HTBubbleHelp(&rect, icon->string, 2);
        icon = (struct IconNode *)FocussedIconPtr;
        if ((icon->flags & 0x1000) != 0) {
            FUN_0046d2f0((UINT_PTR)((struct Ride *)icon->field_8)->element->name);
        } else {
            FUN_0046d230(icon->string_id);
        }
    }
}

// FUNCTION: LEGOLAND 0x0046d080
LEGO_EXPORT void ProcessFrontEndHelp(void) {
    RECT rect;
    struct IconNode *icon;

    RenderHelpIcons();
    icon = (struct IconNode *)FocussedIconPtr;
    if (icon != NULL) {
        int y = MousePos.y;
        int x = MousePos.x;
        rect.left = x;
        rect.top = y - 10;
        rect.right = x;
        rect.bottom = y;
        if ((icon->flags & 0x2000) != 0) {
            HTBubbleHelp(&rect, icon->string, 2);
            icon = (struct IconNode *)FocussedIconPtr;
            if ((icon->flags & 0x1000) != 0) {
                FUN_0046d340(*(const char **)icon->field_8);
            } else {
                FUN_0046d230(icon->string_id);
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x0046d100
LEGO_EXPORT void KillHelp(void) { FUN_0046c5c0(); }

// FUNCTION: LEGOLAND 0x0046d110
void UpdateSpeechPlayback(void) {
    char buf[256];

    if (DAT_006687a8 == 0) {
        DAT_004b9f8c = 0xffffffff;
    }
    if (DAT_006687ac == 0 && DAT_006687b0 != 0) {
        DAT_006687b0 = DAT_006687b0 - 1;
        return;
    }
    if ((int)DAT_006687a4 < 3 && DAT_006687a8 != 0) {
        unsigned int prev = DAT_004b9f88;
        DAT_006687a8 = 0;
        if (prev != 0) {
            if (GetTickCount() - DAT_007fe920 < 500 && DAT_006687ac == 0) {
                DAT_006687b4 = 1;
                return;
            }
            DAT_006687b4 = 0;
            DAT_004b9f88 = 0;
            DAT_006687ac = 0;
            DAT_007fe920 = GetTickCount();
            SpeechCloseFile();
            if (DAT_004b9f8c != 0xffffffff) {
                switch (DAT_006687a4) {
                case 0:
                    // STRING: LEGOLAND 0x004ba858
                    sprintf(buf, "Text%04d.wav", (int)DAT_004b9f8c);
                    break;
                case 1:
                    // STRING: LEGOLAND 0x004ba868
                    sprintf(buf, "%s.wav", (const char *)DAT_004b9f8c);
                    break;
                case 2:
                    // STRING: LEGOLAND 0x004ba870
                    sprintf(buf, "%sz.wav", (const char *)DAT_004b9f8c);
                    break;
                }
                SpeechLoadWavFile(buf);
                SpeechPlay();
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x0046d230
void FUN_0046d230(unsigned int a1) {
    if (SpeechIsPlaying() == 0) {
        if (a1 == 0xffffffff) {
            return;
        }
        if (a1 != DAT_004b9f8c) {
            DAT_004b9f8c = a1;
            DAT_007fe920 = GetTickCount();
            DAT_006687a4 = 0;
            DAT_004b9f88 = 1;
            ClearAdvicePriority();
        }
    }
    DAT_006687a8 = 1;
}

// FUNCTION: LEGOLAND 0x0046d280
unsigned int PlaySpeech(unsigned int a1) {
    if (SpeechIsPlaying() == 0) {
        if (a1 != 0xffffffff && a1 != DAT_004b9f8c) {
            DAT_004b9f8c = a1;
            DAT_007fe920 = GetTickCount();
            DAT_006687a4 = 0;
            DAT_004b9f88 = 1;
            DAT_006687ac = 1;
            ClearAdvicePriority();
            DAT_006687a8 = 1;
            UpdateSpeechPlayback();
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046d2f0
void FUN_0046d2f0(UINT_PTR a1) {
    if (SpeechIsPlaying() != 0) {
        DAT_006687a8 = 1;
        return;
    }
    if (a1 != 0) {
        if (a1 != DAT_004b9f8c) {
            DAT_004b9f8c = a1;
            DAT_007fe920 = GetTickCount();
            DAT_006687a4 = 1;
            DAT_004b9f88 = 1;
        }
        DAT_006687a8 = 1;
    }
}
