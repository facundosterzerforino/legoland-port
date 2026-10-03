#include "globals.h"
#include <dinput.h> /* this TU defines the DInput data symbols (c_dfDI*, GUID_Sys*, MouseState) */
#include "bloke.h"
#include "icon.h"
#include "interface.h"
#include "math.h"
#include "objclass.h"
#include "popupinfo.h"

// GLOBAL: LEGOLAND 0x004ab444
float FLOAT_004ab444;

// GLOBAL: LEGOLAND 0x004ab550
float FLOAT_004ab550;

// GLOBAL: LEGOLAND 0x004ab5f0
GUID IID_IDirectMusicComposer = {0xd2ac28bf, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004ab640
GUID IID_IDirectMusicPerformance = {0x07d43d03, 0x6523, 0x11d2, {0x87, 0x1d, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004ab6a0
GUID IID_IDirectMusicLoader = {0x2ffaaca2, 0x5dca, 0x11d2, {0xaf, 0xa6, 0x00, 0xaa, 0x00, 0x24, 0xd8, 0xb6}};

// GLOBAL: LEGOLAND 0x004ab7b0
GUID GUID_Download = {0xd2ac28a7, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004ab880
GUID GUID_NOTIFICATION_MEASUREANDBEAT = {0xd2ac289a, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004ab8a0
GUID GUID_NOTIFICATION_SEGMENT = {0xd2ac2899, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004ab900
GUID CLSID_DirectMusicLoader = {0xd2ac2892, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004ab920
GUID CLSID_DirectMusicComposer = {0xd2ac2890, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004aba00
GUID CLSID_DirectMusicPerformance = {0xd2ac2881, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004acfd0
GUID NullGuid = {0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

// GLOBAL: LEGOLAND 0x004b4860
struct Point DAT_004b4860;

// GLOBAL: LEGOLAND 0x004c2b00
struct FlumeShape DAT_004c2b00;

// GLOBAL: LEGOLAND 0x004c2b30
struct FlumeDims DAT_004c2b30[4];

// GLOBAL: LEGOLAND 0x004c2b58
struct FlumeShape DAT_004c2b58;

// GLOBAL: LEGOLAND 0x004c2b78
struct FlumeDims DAT_004c2b78[4];

// GLOBAL: LEGOLAND 0x004c2bc0
struct FlumeShape DAT_004c2bc0;

// GLOBAL: LEGOLAND 0x004c2bc8
struct FlumeDims DAT_004c2bc8[4];

// GLOBAL: LEGOLAND 0x004c2be8
struct FlumeShape DAT_004c2be8;

// GLOBAL: LEGOLAND 0x004c2c08
struct FlumeShape DAT_004c2c08;

// GLOBAL: LEGOLAND 0x004c2c10
struct FlumeShape DAT_004c2c10;

// GLOBAL: LEGOLAND 0x004c8d58
struct FlumeDims DAT_004c8d58[2];

// GLOBAL: LEGOLAND 0x004cbde8
struct FlumeDims DAT_004cbde8[4];

// GLOBAL: LEGOLAND 0x004cbe38
struct FlumeDims DAT_004cbe38[2];

// GLOBAL: LEGOLAND 0x004d8bb8
int DAT_004d8bb8[1024][4];

// GLOBAL: LEGOLAND 0x004dcbb8
float DAT_004dcbb8[3];

// GLOBAL: LEGOLAND 0x0066b638
struct ColorLutEntry ColorLut[256];

// GLOBAL: LEGOLAND 0x004ab468
float FLOAT_004ab468;

// GLOBAL: LEGOLAND 0x004ab390
float FLOAT_004ab390;

// GLOBAL: LEGOLAND 0x004ab398
double DOUBLE_004ab398;

// GLOBAL: LEGOLAND 0x004ab3dc
float DAT_004ab3dc;

// GLOBAL: LEGOLAND 0x004ab3e0
float DAT_004ab3e0;

// GLOBAL: LEGOLAND 0x004ab3e4
float DAT_004ab3e4;

// GLOBAL: LEGOLAND 0x004ab3e8
float DAT_004ab3e8;

// GLOBAL: LEGOLAND 0x004ab3ec
float DAT_004ab3ec;

// GLOBAL: LEGOLAND 0x004ab3f0
float DAT_004ab3f0;

// GLOBAL: LEGOLAND 0x004ab3f4
float DAT_004ab3f4;

// GLOBAL: LEGOLAND 0x004ab3f8
float DAT_004ab3f8;

// GLOBAL: LEGOLAND 0x004ab3fc
float DAT_004ab3fc;

// GLOBAL: LEGOLAND 0x004ab478
double DOUBLE_004ab478;

// GLOBAL: LEGOLAND 0x004ab4a0
float DAT_004ab4a0;

// GLOBAL: LEGOLAND 0x004ab4a8
double DAT_004ab4a8;

// GLOBAL: LEGOLAND 0x004ab4b0
double DAT_004ab4b0;

// GLOBAL: LEGOLAND 0x004ab430
float DAT_004ab430;

// GLOBAL: LEGOLAND 0x004ab43c
float FLOAT_004ab43c;

// GLOBAL: LEGOLAND 0x004ab418
double DAT_004ab418;

// GLOBAL: LEGOLAND 0x004ab44c
float FLOAT_004ab44c;

// GLOBAL: LEGOLAND 0x004ab454
float FLOAT_004ab454;

// GLOBAL: LEGOLAND 0x004ab458
float FLOAT_004ab458;

// GLOBAL: LEGOLAND 0x004ab45c
float FLOAT_004ab45c;

// GLOBAL: LEGOLAND 0x004ab4bc
float DAT_004ab4bc;

// GLOBAL: LEGOLAND 0x004ab4c0
float DAT_004ab4c0;

// GLOBAL: LEGOLAND 0x004ab4c8
double DAT_004ab4c8;

// GLOBAL: LEGOLAND 0x004ab518
float DAT_004ab518;

// GLOBAL: LEGOLAND 0x004ab528
float DAT_004ab528;

// GLOBAL: LEGOLAND 0x004ab52c
float FLOAT_004ab52c;

// GLOBAL: LEGOLAND 0x004ab530
double DOUBLE_004ab530;

// GLOBAL: LEGOLAND 0x004ab538
double DOUBLE_004ab538;

// GLOBAL: LEGOLAND 0x004ab558
double DOUBLE_004ab558;

#ifndef LEGOLAND_PORT
/* [library:input] In the port these two tables come from dinput.lib (which is where the original got them);
 * the decomp's empty definitions only provide the symbols for the matching link. */
// GLOBAL: LEGOLAND 0x004ab560
const DIDATAFORMAT c_dfDIKeyboard;

// GLOBAL: LEGOLAND 0x004ab578
const DIDATAFORMAT c_dfDIMouse;
#endif

// GLOBAL: LEGOLAND 0x004ab5e0
GUID IID_IDirectMusicBand = {0xd2ac28c0, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004ab600
GUID IID_IDirectMusicChordMap = {0xd2ac28be, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004ab610
GUID IID_IDirectMusicStyle = {0xd2ac28bd, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004ab670
GUID IID_IDirectMusicSegment = {0xf96029a2, 0x4282, 0x11d2, {0x87, 0x17, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004ab6d0
GUID GUID_PerfMasterGrooveLevel = {0xd2ac28b2, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004ab8b0
GUID GUID_DirectMusicAllTypes = {0xd2ac2893, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004ab8e0
GUID DirectMusicBandClassGuid = {0x79ba9e00, 0xb6ee, 0x11d1, {0x86, 0xbe, 0x00, 0xc0, 0x4f, 0xbf, 0x8f, 0xef}};

// GLOBAL: LEGOLAND 0x004ab930
GUID DirectMusicChordMapClassGuid = {0xd2ac288f, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004ab980
GUID CLSID_DirectMusicStyle = {0xd2ac288a, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

// GLOBAL: LEGOLAND 0x004ab9f0
GUID CLSID_DirectMusicSegment = {0xd2ac2882, 0xb39b, 0x11d1, {0x87, 0x04, 0x00, 0x60, 0x08, 0x93, 0xb1, 0xbd}};

#ifndef LEGOLAND_PORT /* [library:input] the port takes this GUID from dinput.lib */
// GLOBAL: LEGOLAND 0x004ac090
const GUID GUID_SysKeyboard = {0x6f1d2b61, 0xd5a0, 0x11cf, {0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
#endif

#ifndef LEGOLAND_PORT /* [library:input] the port takes this GUID from dinput.lib */
// GLOBAL: LEGOLAND 0x004ac0a0
const GUID GUID_SysMouse = {0x6f1d2b60, 0xd5a0, 0x11cf, {0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
#endif

// GLOBAL: LEGOLAND 0x004acf80
GUID IID_IDirectDraw2_Guid = {0xb3a6f3e0, 0x2b43, 0x11cf, {0xa2, 0xde, 0x00, 0xaa, 0x00, 0xb9, 0x33, 0x56}};

// GLOBAL: LEGOLAND 0x004b4034
int DAT_004b4034[16];

// GLOBAL: LEGOLAND 0x004b40a4
unsigned int DAT_004b40a4[4];

// GLOBAL: LEGOLAND 0x004b40b4
int DAT_004b40b4[4];

// GLOBAL: LEGOLAND 0x004b40c8
unsigned char Catapult_SFX[0x70];

// GLOBAL: LEGOLAND 0x004b4140
unsigned char Helicopter_SFX[0x70];

// GLOBAL: LEGOLAND 0x004b43f8
unsigned char DRIVING_SCHOOL_SFX[0xa8];

// GLOBAL: LEGOLAND 0x004b4440
unsigned int DAT_004b4440[5];

// GLOBAL: LEGOLAND 0x004b4458
unsigned int DAT_004b4458[5];

// GLOBAL: LEGOLAND 0x004b4470
unsigned int DAT_004b4470[5];

// GLOBAL: LEGOLAND 0x004b4580
struct FortRect DAT_004b4580 = {-1, -3, 3, 3};

// GLOBAL: LEGOLAND 0x004b4688
unsigned char JOUST_SFX[12];

// GLOBAL: LEGOLAND 0x004b4728
unsigned int LogFlumeFootprint;

// GLOBAL: LEGOLAND 0x004b472c
unsigned int DAT_004b472c;

// GLOBAL: LEGOLAND 0x004b4730
unsigned int DAT_004b4730;

// GLOBAL: LEGOLAND 0x004b4734
unsigned int DAT_004b4734;
// 0x008003e8 is EditCursor.field_1828 — see struct Cursor in gamemap.h.

// GLOBAL: LEGOLAND 0x004b473c
int DAT_004b473c[4];

// GLOBAL: LEGOLAND 0x004b474c
int DAT_004b474c[2];

// GLOBAL: LEGOLAND 0x004b4754
int DAT_004b4754[4];

// GLOBAL: LEGOLAND 0x004b4764
int DAT_004b4764;

// GLOBAL: LEGOLAND 0x004b47f8
struct Sprite *DAT_004b47f8;

// GLOBAL: LEGOLAND 0x004b4804
struct Sprite *DAT_004b4804;

// GLOBAL: LEGOLAND 0x004b4808
struct FlumeTemplate DAT_004b4808;

// GLOBAL: LEGOLAND 0x004b4818
struct Sprite *DAT_004b4818;

// GLOBAL: LEGOLAND 0x004b4824
struct Sprite *DAT_004b4824;

// GLOBAL: LEGOLAND 0x004b4828
struct FlumeTemplate DAT_004b4828;

// GLOBAL: LEGOLAND 0x004b4838
struct Sprite *DAT_004b4838;

// GLOBAL: LEGOLAND 0x004b4850
struct Sprite *DAT_004b4850;

// GLOBAL: LEGOLAND 0x004b4858
struct FlumeTemplate DAT_004b4858;

// GLOBAL: LEGOLAND 0x004b4bd0
unsigned char DAT_004b4bd0[0x14];

// GLOBAL: LEGOLAND 0x004b4bf0
unsigned int DAT_004b4bf0[5];

// GLOBAL: LEGOLAND 0x004b4c04
int DAT_004b4c04;

// GLOBAL: LEGOLAND 0x004b4c08
unsigned short DAT_004b4c08[15][4];

// GLOBAL: LEGOLAND 0x004b4cb8
struct FXItem SAFARI_SFX[1];

// GLOBAL: LEGOLAND 0x004b4cc4
int DAT_004b4cc4[8] = {0x42, 0x42, 0x2f, 0x2f, 0x50, 0x50, 0x30, 0x30};

// GLOBAL: LEGOLAND 0x004b4ce4
int DAT_004b4ce4[8] = {0x3f, 0x3f, 0x3f, 0x3f, 0x30, 0x30, 0x20, 0x20};

// GLOBAL: LEGOLAND 0x004b4d88
struct FXItem SpiderRide_SFX[1];

// GLOBAL: LEGOLAND 0x004b4d94
struct SpiderBnv SpiderBnvInfo = {"manbox??", {0, 0x20, 0x1c, 0x14, 0x18, 0x0c, 0x10, 0x14, 0x0c, 0x14, 0x18, 0x1c, 0x21, 0x24, 0x21, 0x24}, {0x20, 0x14, 0x18, 0x20, 0x18, 0x20, 0x20, 0x18, 0x20, 0x14, 0x18, 0x10, 0x0c, 0x08, 0x0c, 0x12, 0x14}};

// GLOBAL: LEGOLAND 0x004b4e20
Point DAT_004b4e20;

// GLOBAL: LEGOLAND 0x004b4f08
char *DAT_004b4f08[4] = {
    // STRING: LEGOLAND 0x004b4f44
    "manbox01",
    // STRING: LEGOLAND 0x004b4f38
    "manbox02",
    // STRING: LEGOLAND 0x004b4f2c
    "manbox03",
    // STRING: LEGOLAND 0x004b4f20
    "manbox04",
};

// GLOBAL: LEGOLAND 0x004b4f18
signed char DAT_004b4f18[4] = {0x40, 0x45, 0x3f, 0x3c};

// GLOBAL: LEGOLAND 0x004b4f1c
signed char DAT_004b4f1c[4] = {0x51, 0x53, 0x53, 0x50};

// GLOBAL: LEGOLAND 0x004b4fa8
unsigned char WATERWORKS_SFX[0x24];

// GLOBAL: LEGOLAND 0x004b5118
struct BoatDirStep BoatingSchoolDirSteps[4];

// GLOBAL: LEGOLAND 0x004b5158
struct BoatArc DAT_004b5158[4];

// GLOBAL: LEGOLAND 0x004b5198
struct BoatArc DAT_004b5198[4];

// GLOBAL: LEGOLAND 0x004b51d8
int DAT_004b51d8[0x80];

// GLOBAL: LEGOLAND 0x004b5260
struct Footprint DAT_004b5260;

// GLOBAL: LEGOLAND 0x004b5278
struct Footprint DAT_004b5278;

// GLOBAL: LEGOLAND 0x004b5290
struct Point DAT_004b5290[6];

// GLOBAL: LEGOLAND 0x004b52c0
unsigned char PTR_s_Boat_Noise_wav[0x18];

// GLOBAL: LEGOLAND 0x004b53c0
struct Footprint DAT_004b53c0;

// GLOBAL: LEGOLAND 0x004b53d4
unsigned char DAT_004b53d4[0x190];

// GLOBAL: LEGOLAND 0x004b5570
unsigned int DAT_004b5570[5] = {0, 0, 2, 2, 0};

// GLOBAL: LEGOLAND 0x004b5584
short DAT_004b5584[4][2] = {{0, -2}, {2, 0}, {0, 2}, {-2, 0}};

// GLOBAL: LEGOLAND 0x004b55f4
unsigned int DAT_004b55f4;

// GLOBAL: LEGOLAND 0x004b5608
unsigned int DAT_004b5608;

// GLOBAL: LEGOLAND 0x004b560c
int DAT_004b560c;

// GLOBAL: LEGOLAND 0x004b5b20
void *DAT_004b5b20;

// GLOBAL: LEGOLAND 0x004b5b24
unsigned short *DAT_004b5b24;

// GLOBAL: LEGOLAND 0x004b5b28
int DAT_004b5b28;

// GLOBAL: LEGOLAND 0x004b5b4c
int DAT_004b5b4c;

// GLOBAL: LEGOLAND 0x004b5b50
int DAT_004b5b50;

// GLOBAL: LEGOLAND 0x004b59e8
char SitLoGirlSitName[16] = "sit.logirlsit";

// GLOBAL: LEGOLAND 0x004bc0ec
char LanguageName[36] = "english";

// GLOBAL: LEGOLAND 0x004b59f8
char SitLoManSitName[] = "sit.lomansit";

// GLOBAL: LEGOLAND 0x004b5abc
float DAT_004b5abc[4][4];

// GLOBAL: LEGOLAND 0x004b5b58
unsigned short DAT_004b5b58;

// GLOBAL: LEGOLAND 0x004b5b5a
unsigned short DAT_004b5b5a;

// GLOBAL: LEGOLAND 0x004b5b60
unsigned short DAT_004b5b60;

// GLOBAL: LEGOLAND 0x004b5b62
unsigned short DAT_004b5b62;

// GLOBAL: LEGOLAND 0x004b5b3c
struct RecBuf *DAT_004b5b3c;

// GLOBAL: LEGOLAND 0x004b5988
struct BlokeInfo DAT_004b5988;

// GLOBAL: LEGOLAND 0x004b5964
unsigned int DAT_004b5964[2] = {0x191919, 0xf11a22};

// GLOBAL: LEGOLAND 0x004b596c
const char *DAT_004b596c[2] = {
    // STRING: LEGOLAND 0x004b59d8
    "Chest visitor1",
    // STRING: LEGOLAND 0x004b59d0
    "Face01"};

// GLOBAL: LEGOLAND 0x004b5974
unsigned int DAT_004b5974[2] = {0xf11a22, 0x8b4a};

// GLOBAL: LEGOLAND 0x004b597c
const char *DAT_004b597c[2] = {
    // STRING: LEGOLAND 0x004b59c0
    "Chest girly2",
    // STRING: LEGOLAND 0x004b59d0
    "Face01"};

// GLOBAL: LEGOLAND 0x004b5c1c
struct FMat4 DAT_004b5c1c[2];

// GLOBAL: LEGOLAND 0x004b5c9c
unsigned int DAT_004b5c9c;

// GLOBAL: LEGOLAND 0x004b5ca0
unsigned int DAT_004b5ca0;

// GLOBAL: LEGOLAND 0x004b5ca4
unsigned int DAT_004b5ca4;

// GLOBAL: LEGOLAND 0x004b5ca8
unsigned int DAT_004b5ca8;

// GLOBAL: LEGOLAND 0x004b5cac
float DAT_004b5cac;

// GLOBAL: LEGOLAND 0x004b5cb0
unsigned int DAT_004b5cb0;

// GLOBAL: LEGOLAND 0x004b5cb4
unsigned int DAT_004b5cb4;

// GLOBAL: LEGOLAND 0x004b5cb8
unsigned int DAT_004b5cb8;

// GLOBAL: LEGOLAND 0x004b5cbc
float DAT_004b5cbc;

// GLOBAL: LEGOLAND 0x004b5cc0
unsigned int DAT_004b5cc0;

// GLOBAL: LEGOLAND 0x004b5cc4
unsigned int DAT_004b5cc4;

// GLOBAL: LEGOLAND 0x004b5cc8
unsigned int DAT_004b5cc8;

// GLOBAL: LEGOLAND 0x004b5cf4
char RollerCoasterSavePath[32] = "RollerCoaster\\RollerCoaster.sav";

// GLOBAL: LEGOLAND 0x004b5d20
const unsigned char DAT_004b5d20[1];

// GLOBAL: LEGOLAND 0x004b5d58
const unsigned char DAT_004b5d58[1];

// GLOBAL: LEGOLAND 0x004b5d90
const unsigned char DAT_004b5d90[1];

// GLOBAL: LEGOLAND 0x004b5dc8
const unsigned char DAT_004b5dc8[1];

// GLOBAL: LEGOLAND 0x004b5df0
unsigned int DAT_004b5df0[4] = {0x427ff0, 0x427f70, 1, 0};

// GLOBAL: LEGOLAND 0x004b5e00
float DAT_004b5e00[4][3] = {{1.0f, 0.0f, 0.0f}, {2.0f, 1.0f, 0.0f}, {1.0f, 2.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};

// GLOBAL: LEGOLAND 0x004b5e30
float DAT_004b5e30[4][3] = {{0.5f, 0.0f, 0.0f}, {-0.5f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}, {0.0f, -0.5f, 0.0f}};

// GLOBAL: LEGOLAND 0x004b5e60
float DAT_004b5e60[4][3] = {{1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};

// GLOBAL: LEGOLAND 0x004b5e90
float DAT_004b5e90[4][3] = {{0.0f, 0.0f, 0.0f}, {2.0f, 0.0f, 0.0f}, {2.0f, 2.0f, 0.0f}, {0.0f, 2.0f, 0.0f}};

// GLOBAL: LEGOLAND 0x004b5ec0
int DAT_004b5ec0[4] = {0, 2, 1, 3};

// GLOBAL: LEGOLAND 0x004b5ee0
int DAT_004b5ee0[5] = {-4, -2, 0, 2, 4};

// GLOBAL: LEGOLAND 0x004b5ef4
unsigned int DAT_004b5ef4[4][2] = {{0, 3}, {2, 3}, {2, 1}, {0, 1}};

// GLOBAL: LEGOLAND 0x004b5f14
unsigned int DAT_004b5f14[4][2] = {{0x3fc00000, 0x3f000000}, {0x3f000000, 0x3fc00000}, {0x3fc00000, 0x3f000000}, {0x3f000000, 0x3fc00000}};

// GLOBAL: LEGOLAND 0x004b5f60
struct TexMesh DAT_004b5f60 = {0, 0, 0, 0, (struct MeshVert *)DAT_006139c8, DAT_006148b8, (unsigned int (*)[3])DAT_00612708};

// GLOBAL: LEGOLAND 0x004b6150
unsigned int DAT_004b6150[12];

// GLOBAL: LEGOLAND 0x004b61e0
struct CastleFloatEnt DAT_004b61e0[8];

// GLOBAL: LEGOLAND 0x004b62f0
unsigned int DAT_004b62f0;

// GLOBAL: LEGOLAND 0x004b6300
unsigned int DAT_004b6300;

// GLOBAL: LEGOLAND 0x004b6398
float DAT_004b6398[4][3] = {{20.0f, 0.0f, 0.0f}, {40.0f, 20.0f, 0.0f}, {20.0f, 40.0f, 0.0f}, {0.0f, 20.0f, 0.0f}};

// GLOBAL: LEGOLAND 0x004b63c8
float DAT_004b63c8[4][3] = {{10.0f, 0.0f, 0.0f}, {0.0f, 10.0f, 0.0f}, {-10.0f, 0.0f, 0.0f}, {0.0f, -10.0f, 0.0f}};
// GLOBAL: LEGOLAND 0x004b6408
int DAT_004b6408[3] = {0, 2, 1};

// GLOBAL: LEGOLAND 0x004ab490
float FLOAT_004ab490;

// GLOBAL: LEGOLAND 0x004ab494
float FLOAT_004ab494;

// GLOBAL: LEGOLAND 0x004b64d4
char DAT_004b64d4[4];

// GLOBAL: LEGOLAND 0x004b64d8
struct FXItem CAROUSSEL_SFX[2];

// GLOBAL: LEGOLAND 0x004b65c0
int DAT_004b65c0[8] = {0, 4, 0, 3, 0, 2, 0, 1};

// GLOBAL: LEGOLAND 0x004b6638
char DAT_004b6638[4] = ".";

// GLOBAL: LEGOLAND 0x004b6654
int ENTRANCE_DEST_RIGHT[2] = {0x978, 0xd78};

// GLOBAL: LEGOLAND 0x004b665c
int ENTRANCE_DEST_LEFT[2] = {0x878, 0xc78};

// GLOBAL: LEGOLAND 0x004b6668
unsigned char ENTRANCE_SFX[0x1c];

// GLOBAL: LEGOLAND 0x004b66e8
unsigned char RESTAURANT_SFX[0xc];

// GLOBAL: LEGOLAND 0x004b66f4
int DAT_004b66f4[15 * 6];

// GLOBAL: LEGOLAND 0x004b685c
int DAT_004b685c;

// GLOBAL: LEGOLAND 0x004b6860
int DAT_004b6860[0x20];

// GLOBAL: LEGOLAND 0x004b68e0
int DAT_004b68e0[0x22];

// GLOBAL: LEGOLAND 0x004b6968
unsigned char OCTOPUS_SFX[0x24];

// GLOBAL: LEGOLAND 0x004b6990
int DAT_004b6990[0x29];

// GLOBAL: LEGOLAND 0x004b6a34
int DAT_004b6a34[4];

// GLOBAL: LEGOLAND 0x004b6a44
int DAT_004b6a44[0xd];

// GLOBAL: LEGOLAND 0x004b6a78
int DAT_004b6a78[0x30];

// GLOBAL: LEGOLAND 0x004b6b38
int DAT_004b6b38[0x2c];

// GLOBAL: LEGOLAND 0x004b6be8
int DAT_004b6be8[0x5c];

// GLOBAL: LEGOLAND 0x004b6d58
unsigned char DAT_004b6d58[0x120];

// GLOBAL: LEGOLAND 0x004b7260
struct Footprint DAT_004b7260;

// GLOBAL: LEGOLAND 0x004b7278
struct Footprint JungleCruiseStartFootprint;

// GLOBAL: LEGOLAND 0x004b7288
struct Footprint *DAT_004b7288;

// GLOBAL: LEGOLAND 0x004b7148
struct BoatDirStep DAT_004b7148[4];

// GLOBAL: LEGOLAND 0x004b7188
struct BoatArc DAT_004b7188[4];

// GLOBAL: LEGOLAND 0x004b71c8
struct BoatArc DAT_004b71c8[4];

// GLOBAL: LEGOLAND 0x004b7290
struct Point DAT_004b7290[5];

// GLOBAL: LEGOLAND 0x004b7230
struct Footprint DAT_004b7230;

// GLOBAL: LEGOLAND 0x004b7248
struct Footprint DAT_004b7248;

// GLOBAL: LEGOLAND 0x004b72e4
unsigned char DAT_004b72e4[0x190];

// GLOBAL: LEGOLAND 0x004b7478
struct Footprint DAT_004b7478;

// GLOBAL: LEGOLAND 0x004b7618
struct FXItem SPACE_TOWER_SFX[1];

// GLOBAL: LEGOLAND 0x004b76b8
unsigned char DAT_004b76b8[16];

// GLOBAL: LEGOLAND 0x004b7750
unsigned char DAT_004b7750[16];

// GLOBAL: LEGOLAND 0x004b7758
struct SpaceTowerLayout DAT_004b7758[8];

// GLOBAL: LEGOLAND 0x004b7798
struct SpaceTowerSeatData DAT_004b7798[4];

// GLOBAL: LEGOLAND 0x004b77e8
struct Point DAT_004b77e8[8];

// GLOBAL: LEGOLAND 0x004b78b4
char BoxBlokeBnvName[] = "BoxBloke??";

// GLOBAL: LEGOLAND 0x004b79bc
char DAT_004b79bc[] = "manbox??";

// GLOBAL: LEGOLAND 0x004b79d0
unsigned char DAT_004b79d0[0x18];

// GLOBAL: LEGOLAND 0x004b7abc
unsigned short DAT_004b7abc;

// GLOBAL: LEGOLAND 0x004b7ac0
unsigned char BlokeColours[0x18];

// GLOBAL: LEGOLAND 0x004b7d70
unsigned int DAT_004b7d70;

// GLOBAL: LEGOLAND 0x004b7d74
unsigned int DAT_004b7d74;

// GLOBAL: LEGOLAND 0x004b7d78
unsigned int DAT_004b7d78;

// GLOBAL: LEGOLAND 0x004b7d7e
unsigned short DAT_004b7d7e;

// GLOBAL: LEGOLAND 0x004b7d84
unsigned int DAT_004b7d84;

// GLOBAL: LEGOLAND 0x004b7e9c
char *DAT_004b7e9c[22]; /* keyword table for FindStringNoCase */

// GLOBAL: LEGOLAND 0x004b81c0
// STRING: LEGOLAND 0x004b8260
char *GraphicsPath = ".\\graphics\\";

// GLOBAL: LEGOLAND 0x004b81c4
char *GraphicsPathPrefix = ".\\graphics\\";

// GLOBAL: LEGOLAND 0x004b81c8
// STRING: LEGOLAND 0x004b824c
char *GraphicsSmallPath = ".\\graphics\\small\\";

// GLOBAL: LEGOLAND 0x004b81cc
// STRING: LEGOLAND 0x004b8238
char *MasksPath = ".\\graphics\\masks\\";

// GLOBAL: LEGOLAND 0x004b81d0
// STRING: LEGOLAND 0x004b8220
char *MasksSmallPath = ".\\graphics\\masks\\small\\";

// GLOBAL: LEGOLAND 0x004b81d4
// STRING: LEGOLAND 0x004b820c
char *IconsPath = ".\\graphics\\icons\\";

// GLOBAL: LEGOLAND 0x004b81d8
// STRING: LEGOLAND 0x004b81f8
char *ModelsPath = ".\\graphics\\models\\";

// GLOBAL: LEGOLAND 0x004b8318
struct Point DAT_004b8318; /* offset of the park entrance walk-in start */

// GLOBAL: LEGOLAND 0x004b8320
struct Point DAT_004b8320; /* park exit (goal of leaving blokes) */

// GLOBAL: LEGOLAND 0x004b8328
struct Point DAT_004b8328; /* step past the exit */

// GLOBAL: LEGOLAND 0x004b8334
int DAT_004b8334[4] = {0}; /* bloke tiredness thresholds */

// GLOBAL: LEGOLAND 0x004b8344
char DAT_004b8344 = 0;

// GLOBAL: LEGOLAND 0x004b8348
char *PTR_DAT_004b8348[8] = {0};

// GLOBAL: LEGOLAND 0x004b8368
void (*PTR_Bloke_DoNothing_004b8368[26])(struct Bloke *) = {0};

// GLOBAL: LEGOLAND 0x004b85c4
HANDLE DAT_004b85c4 = INVALID_HANDLE_VALUE;

// GLOBAL: LEGOLAND 0x004b8710
unsigned char FountainSFX[0x40];

// GLOBAL: LEGOLAND 0x004b8750
unsigned char PowerStationSFX[0x18];

// GLOBAL: LEGOLAND 0x004b8768
struct FXItem DINO_SFX[5];

// GLOBAL: LEGOLAND 0x004b87a8
struct FXItem MONEY_SFX[2];

// GLOBAL: LEGOLAND 0x004b8bbc
unsigned char PercentSFormat[1];

// GLOBAL: LEGOLAND 0x004b90f8
unsigned int BrickCount;

// GLOBAL: LEGOLAND 0x004b90fc
unsigned int UnlimitedBricks;

// GLOBAL: LEGOLAND 0x004b9210
int DAT_004b9210;

// GLOBAL: LEGOLAND 0x004b9214
int DAT_004b9214;

// GLOBAL: LEGOLAND 0x004b9218
int DAT_004b9218;

// GLOBAL: LEGOLAND 0x004b921c
int DAT_004b921c;

// GLOBAL: LEGOLAND 0x004b9220
LEGO_EXPORT unsigned int BGFullUpdate;

// GLOBAL: LEGOLAND 0x004b9228
struct FXItem GameFX[FX_COUNT];

// GLOBAL: LEGOLAND 0x004b9550
unsigned char DAT_004b9550[8];

// GLOBAL: LEGOLAND 0x004b9558
unsigned int DAT_004b9558[9] = {
    0x01ce7000,
    0x00e73800,
    0x00739c00,
    0x000e7380,
    0x000739c0,
    0x00039ce0,
    0x0000739c,
    0x000039ce,
    0x00001ce7,
};

// GLOBAL: LEGOLAND 0x004b957c
int DAT_004b957c[9] = {-2, -1, 0, -2, -1, 0, -2, -1, 0};

// GLOBAL: LEGOLAND 0x004b95a0
int DAT_004b95a0[9] = {-2, -2, -2, -1, -1, -1, 0, 0, 0};

// GLOBAL: LEGOLAND 0x004b95c4
unsigned char DAT_004b95c4[8];

// GLOBAL: LEGOLAND 0x004b95cc
unsigned char DAT_004b95cc[8];

// GLOBAL: LEGOLAND 0x004b95d4
unsigned char DAT_004b95d4[8];

// GLOBAL: LEGOLAND 0x004b95dc
unsigned char DAT_004b95dc[8];

// GLOBAL: LEGOLAND 0x004b95e8
int DAT_004b95e8;

// GLOBAL: LEGOLAND 0x004b95ec
int DAT_004b95ec;

// GLOBAL: LEGOLAND 0x004b95f0
struct DeferredSprite *PTR_DAT_004b95f0;

// GLOBAL: LEGOLAND 0x004b95f4
int DAT_004b95f4;

// GLOBAL: LEGOLAND 0x004b95f8
int DAT_004b95f8;

// GLOBAL: LEGOLAND 0x004b95fc
int DAT_004b95fc;

// GLOBAL: LEGOLAND 0x004b9600
int DAT_004b9600;

// GLOBAL: LEGOLAND 0x004b9604
int DAT_004b9604;

// GLOBAL: LEGOLAND 0x004b9608
int DAT_004b9608;

// GLOBAL: LEGOLAND 0x004b960c
int DAT_004b960c;

// GLOBAL: LEGOLAND 0x004b9610
int DAT_004b9610;

// GLOBAL: LEGOLAND 0x004b9ca4
int (*BlitFrameFunc)(void);

// GLOBAL: LEGOLAND 0x004b9ca8
unsigned int OverrideFrame;

// GLOBAL: LEGOLAND 0x004b9e5c
unsigned int DAT_004b9e5c[71];

// GLOBAL: LEGOLAND 0x004b9f78
int DAT_004b9f78[4] = {0x1f4, 0x190, 0x280, 0x1e0};

// GLOBAL: LEGOLAND 0x004b9f88
unsigned int DAT_004b9f88;

// GLOBAL: LEGOLAND 0x004b9f8c
unsigned int DAT_004b9f8c;

// GLOBAL: LEGOLAND 0x004ba87c
void *PTR_some_list_head_004ba87c;

// GLOBAL: LEGOLAND 0x004ba880
void *PTR_DAT_004ba880;

// GLOBAL: LEGOLAND 0x004ba884
unsigned int DAT_004ba884 = 0xff000000;

// GLOBAL: LEGOLAND 0x004ba8e0
struct InfoTimedEntry DAT_004ba8e0[17];

// GLOBAL: LEGOLAND 0x004ba9ac
unsigned int DAT_004ba9ac[234];

// GLOBAL: LEGOLAND 0x004bad54
int mouse_granularity;

// GLOBAL: LEGOLAND 0x004bad58
struct KeyMapping DAT_004bad58[0x3c];

// GLOBAL: LEGOLAND 0x004bafa8
unsigned int DAT_004bafa8[20];

// GLOBAL: LEGOLAND 0x004baff8
unsigned int DAT_004baff8;

// GLOBAL: LEGOLAND 0x004baffc
char DAT_004baffc[4][0x14];

// GLOBAL: LEGOLAND 0x004bb04c
int DAT_004bb04c[18];

// GLOBAL: LEGOLAND 0x004bb094
unsigned int DAT_004bb094;

// GLOBAL: LEGOLAND 0x004bb098
unsigned int DAT_004bb098;

// GLOBAL: LEGOLAND 0x004bb09c
unsigned int DAT_004bb09c;

// GLOBAL: LEGOLAND 0x004bb0a0
unsigned int DAT_004bb0a0;

// GLOBAL: LEGOLAND 0x004bb0a4
struct TrackElemPair DAT_004bb0a4[29];

// GLOBAL: LEGOLAND 0x004bb18c
unsigned int DAT_004bb18c[4];

// GLOBAL: LEGOLAND 0x004bb4dc
float DAT_004bb4dc;

// GLOBAL: LEGOLAND 0x004bb4e0
BITMAPINFOHEADER DAT_004bb4e0;

// GLOBAL: LEGOLAND 0x004bb598
struct Point DAT_004bb598;

// GLOBAL: LEGOLAND 0x004bb5a0
int DAT_004bb5a0;

// GLOBAL: LEGOLAND 0x004bb5a4
int DAT_004bb5a4;

// GLOBAL: LEGOLAND 0x004bb5ac
unsigned int DAT_004bb5ac;

// GLOBAL: LEGOLAND 0x004bb5b0
unsigned int ScriptConditionActive;

// GLOBAL: LEGOLAND 0x004bb5b4
char *DAT_004bb5b4[4]; /* keyword table for FindStringNoCase */

// GLOBAL: LEGOLAND 0x004bb5c4
char *DAT_004bb5c4[5]; /* keyword table for FindStringNoCase */

// GLOBAL: LEGOLAND 0x004bb5d8
char *DAT_004bb5d8[2]; /* keyword table for FindStringNoCase */

// GLOBAL: LEGOLAND 0x004bb5e0
char *DAT_004bb5e0[5]; /* keyword table for FindStringNoCase */

// GLOBAL: LEGOLAND 0x004bb5f4
char *DAT_004bb5f4[12]; /* keyword table for FindStringNoCase */

// GLOBAL: LEGOLAND 0x004bb624
char *DAT_004bb624[25]; /* keyword table for FindStringNoCase */

// GLOBAL: LEGOLAND 0x004bb688
char *DAT_004bb688[13]; /* keyword table for FindStringNoCase */

// GLOBAL: LEGOLAND 0x004bb6bc
char *DAT_004bb6bc[6]; /* keyword table for FindStringNoCase */

// GLOBAL: LEGOLAND 0x004bb6d4
char *DAT_004bb6d4[9]; /* keyword table for FindStringNoCase */

// GLOBAL: LEGOLAND 0x004bb6f8
struct ScriptCommand DAT_004bb6f8[0x5d];

// GLOBAL: LEGOLAND 0x004bcba4
const char *ResourceFileNames[3] = {
    // STRING: LEGOLAND 0x004bcc18
    "Legoland.res",
    // STRING: LEGOLAND 0x004bcc08
    "Graphics2.res",
    // STRING: LEGOLAND 0x004bcbf8
    "Graphics1.res",
};

// GLOBAL: LEGOLAND 0x004bcbf4
LEGO_EXPORT struct LegoConfig *lpConfig;

// GLOBAL: LEGOLAND 0x004bcec0
struct Point DAT_004bcec0;

// GLOBAL: LEGOLAND 0x004bcecc
char *PTR_s_Aaron_004bcecc[0x53];

// GLOBAL: LEGOLAND 0x004bd018
char *PTR_s_Abbie_004bd018[0x5a];

// GLOBAL: LEGOLAND 0x004bd180
char *PTR_s_Adams_004bd180[0x6b];

// GLOBAL: LEGOLAND 0x004bd32c
short DAT_004bd32c[8][2] = {
    {-181, -181},
    {0, -256},
    {181, -181},
    {256, 0},
    {181, 181},
    {0, 256},
    {-181, 181},
    {-256, 0},
};

// GLOBAL: LEGOLAND 0x004bd34c
BlokeAction PTR_FUN_004bd34c[16] = {
    LogBlokeRethinking,
    FUN_004838c0,
    FUN_00483ef0,
    FUN_00484090,
    FUN_00483d10,
    FUN_004838e0,
    FUN_00484220,
    FUN_004845d0,
    FUN_00484630,
    FUN_00484790,
    FUN_00483e20,
    FUN_00484470,
    FUN_00484520,
    FUN_004848e0,
    FUN_00483d90,
    FUN_00484350,
};

// GLOBAL: LEGOLAND 0x004bdd00
struct HoverInfo Hover;

// GLOBAL: LEGOLAND 0x004bdea0
LEGO_EXPORT RECT SPRITE_ClipRect;

// GLOBAL: LEGOLAND 0x004beb80
struct ProgressTables ProgressScreenTables;

// [port:rewrite] 0x4bed40..0x4bef9c holds the progress and tutorial screens' sprite names ("Appraisal_Yes.lls",
// "Pro_Egypt_Unlit.lls", ... "TutorialBK.lls"), which ProgressScreenTables and the screen code point into. The
// decomp declares 0x4bed40 and 0x4bed44 as unsigned ints only because the original's loops compare against
// those addresses (one past ProgressScreenTables.tutorials, see progress.c). As 4-byte globals gen_data cut
// every one of these names short ("Appraisa"), so the port sizes 0x4bed40 to the whole string pool and leaves
// DAT_004bed44 out.
// GLOBAL: LEGOLAND 0x004bed40
char DAT_004bed40[0x25c];

// GLOBAL: LEGOLAND 0x004bef9c
unsigned int DAT_004bef9c;

// GLOBAL: LEGOLAND 0x004bf670
unsigned int DAT_004bf670;

// GLOBAL: LEGOLAND 0x004bf774
unsigned int MusicEnabled;

// GLOBAL: LEGOLAND 0x004bf778
unsigned int MusicState;

// GLOBAL: LEGOLAND 0x004bff28
const int DAT_004bff28[12][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {0, 2}, {0, -2},
    {2, 0}, {-2, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

// GLOBAL: LEGOLAND 0x004c10d4
void *DAT_004c10d4;

// GLOBAL: LEGOLAND 0x004c10dc
struct Ride *CastleLevelRide;

// GLOBAL: LEGOLAND 0x004c10e4
struct Sprite *CastleLevelMatteSprite;

// GLOBAL: LEGOLAND 0x004c10e8
struct RenderItemNode *DAT_004c10e8;

// GLOBAL: LEGOLAND 0x004c10f0
void *DAT_004c10f0;

// GLOBAL: LEGOLAND 0x004c10f4
void *ActiveCatapultRide;

// GLOBAL: LEGOLAND 0x004c1100
struct RideSpriteInfo DAT_004c1100;

// GLOBAL: LEGOLAND 0x004c1118
struct CatapultNode *CatapultNodeList;

// GLOBAL: LEGOLAND 0x004c1120
struct Sprite *CopterBaseMatteSprite;

// GLOBAL: LEGOLAND 0x004c1124
int CopterQueueTables[5];

// GLOBAL: LEGOLAND 0x004c1138
void *CopterModelLayers;

// GLOBAL: LEGOLAND 0x004c113c
struct Sprite *CopterModelSprites[10];

// GLOBAL: LEGOLAND 0x004c1164
struct RenderItemNode **DAT_004c1164;

// GLOBAL: LEGOLAND 0x004c1168
struct RenderItemNode **DAT_004c1168;

// GLOBAL: LEGOLAND 0x004c1170
struct RideSpriteInfo DAT_004c1170;

// GLOBAL: LEGOLAND 0x004c1188
struct RenderItemNode **DAT_004c1188;

// GLOBAL: LEGOLAND 0x004c1190
struct RenderItemNode **DAT_004c1190;

// GLOBAL: LEGOLAND 0x004c1194
struct RenderItemNode **DAT_004c1194;

// GLOBAL: LEGOLAND 0x004c1198
void *ActiveCopterRide;

// GLOBAL: LEGOLAND 0x004c119c
struct RenderItemNode *DAT_004c119c;

// GLOBAL: LEGOLAND 0x004c11a0
struct RenderItemNode *DAT_004c11a0;

// GLOBAL: LEGOLAND 0x004c11a4
struct RenderItemNode *DAT_004c11a4;

// GLOBAL: LEGOLAND 0x004c11a8
struct RenderItemNode *DAT_004c11a8;

// GLOBAL: LEGOLAND 0x004c11ac
struct RenderItemNode *DAT_004c11ac;

// GLOBAL: LEGOLAND 0x004c11b0
struct RenderItemNode *DAT_004c11b0;

// GLOBAL: LEGOLAND 0x004c11b4
struct CopterNode *CopterNodeList;

// GLOBAL: LEGOLAND 0x004c11bc
void *DrivingSchoolCountList;

// GLOBAL: LEGOLAND 0x004c11c0
int DAT_004c11c0;

// GLOBAL: LEGOLAND 0x004c11c4
struct RideQueueEntry *DAT_004c11c4;

// GLOBAL: LEGOLAND 0x004c11c8
struct RideQueueEntry *DAT_004c11c8;

// GLOBAL: LEGOLAND 0x004c11cc
struct Sprite *FortMaskSprite;

// GLOBAL: LEGOLAND 0x004c11d8
struct Sprite *FortLayer;

// GLOBAL: LEGOLAND 0x004c11dc
struct Ride *FortRide;

// GLOBAL: LEGOLAND 0x004c11e0
struct RenderItemNode *DAT_004c11e0;

// GLOBAL: LEGOLAND 0x004c11e4
struct Sprite *DAT_004c11e4;

// GLOBAL: LEGOLAND 0x004c11e8
struct Sprite *GoldRushLayer;

// GLOBAL: LEGOLAND 0x004c11f0
void *GoldRushRide;

// GLOBAL: LEGOLAND 0x004c11f4
struct Sprite *GoldMaskSprite;

// GLOBAL: LEGOLAND 0x004c11f8
struct Sprite *GoldWashMatte1Sprite;

// GLOBAL: LEGOLAND 0x004c11fc
struct Sprite *GoldWashSprite;

// GLOBAL: LEGOLAND 0x004c1200
struct Sprite *GoldWashMatte2Sprite;

// GLOBAL: LEGOLAND 0x004c1204
struct GoldNode *GoldWashList;

// GLOBAL: LEGOLAND 0x004c1208
struct RenderItemNode *GoldRushBlokeRenderList;

// GLOBAL: LEGOLAND 0x004c1210
struct Sprite *ZJoustSprite;

// GLOBAL: LEGOLAND 0x004c1214
unsigned int JoustLayer;

// GLOBAL: LEGOLAND 0x004c1218
void *JoustRideBnv;

// GLOBAL: LEGOLAND 0x004c121c
unsigned int JoustRide;

// GLOBAL: LEGOLAND 0x004c1228
struct RideSpriteInfo DAT_004c1228;

// GLOBAL: LEGOLAND 0x004c123c
void *DAT_004c123c[1];

// GLOBAL: LEGOLAND 0x004c1240
struct Sprite *DAT_004c1240;

// GLOBAL: LEGOLAND 0x004c1244
struct Sprite *JoustFMaskSprite;

// GLOBAL: LEGOLAND 0x004c1248
struct Sprite *JoustSpecRMSprite;

// GLOBAL: LEGOLAND 0x004c124c
struct Sprite *JoustSpecLMSprite;

// GLOBAL: LEGOLAND 0x004c1250
struct JoustNode *JoustNodeList;

// GLOBAL: LEGOLAND 0x004c1258
struct Sprite *LogFlumeTunelMSprite;

// GLOBAL: LEGOLAND 0x004c1260
struct Cursor DAT_004c1260;

// GLOBAL: LEGOLAND 0x004c2a94
struct Sprite *LogFlumeHup1M1Sprite;

// GLOBAL: LEGOLAND 0x004c2a98
struct Sprite *LogFlumeHup1M2Sprite;

// GLOBAL: LEGOLAND 0x004c2aa8
struct Footprint DAT_004c2aa8;

// GLOBAL: LEGOLAND 0x004c2aa0
struct CursorSource *DAT_004c2aa0;

// GLOBAL: LEGOLAND 0x004c2abc
struct Sprite *LogFlumeTrackSprites[10];

// GLOBAL: LEGOLAND 0x004c2ae4
struct Sprite *DAT_004c2ae4;

// GLOBAL: LEGOLAND 0x004c2ae8
void *DAT_004c2ae8;

// GLOBAL: LEGOLAND 0x004c2aec
struct Sprite *LogFlumeDrop2MSprite;

// GLOBAL: LEGOLAND 0x004c2af0
struct Sprite *LogFlumeDrop1MSprite;

// GLOBAL: LEGOLAND 0x004c2af4
unsigned int DAT_004c2af4;

// GLOBAL: LEGOLAND 0x004c2af8
void *DAT_004c2af8;

// GLOBAL: LEGOLAND 0x004c2afc
unsigned int LogFlumeImageListId;

// GLOBAL: LEGOLAND 0x004c2b0c
struct CursorSource *DAT_004c2b0c;

// GLOBAL: LEGOLAND 0x004c2b10
unsigned int LogFlumeTrackEndyListId;

// GLOBAL: LEGOLAND 0x004c2b50
void *DAT_004c2b50;

// GLOBAL: LEGOLAND 0x004c2b60
struct CursorSource *DAT_004c2b60;

// GLOBAL: LEGOLAND 0x004c2b64
struct Sprite *LogFlumeSplashSprite;

// GLOBAL: LEGOLAND 0x004c2b68
struct SpriteSet *LogFlumeImageList;

// GLOBAL: LEGOLAND 0x004c2b6c
struct Sprite *LogFlumeFc1M1Sprite;

// GLOBAL: LEGOLAND 0x004c2b70
struct Sprite *LogFlumeFc1M2Sprite;

// GLOBAL: LEGOLAND 0x004c2b98
unsigned int DAT_004c2b98;

// GLOBAL: LEGOLAND 0x004c2b9c
struct Ride *LogFlumeEntranceRide;

// GLOBAL: LEGOLAND 0x004c2c18
struct Cursor DAT_004c2c18;

// GLOBAL: LEGOLAND 0x004c2ba0
void *DAT_004c2ba0;

// GLOBAL: LEGOLAND 0x004c2bf0
struct CursorSource *DAT_004c2bf0;

// GLOBAL: LEGOLAND 0x004c445c
struct CursorSource *DAT_004c445c;

// GLOBAL: LEGOLAND 0x004c4460
void *DAT_004c4460;

// GLOBAL: LEGOLAND 0x004c4468
struct Cursor DAT_004c4468;

// GLOBAL: LEGOLAND 0x004c5ca0
struct Cursor DAT_004c5ca0;

// GLOBAL: LEGOLAND 0x004c74f4
struct Element *DAT_004c74f4;

// GLOBAL: LEGOLAND 0x004c74f8
struct Cursor LogFlumeCursor;

// GLOBAL: LEGOLAND 0x004c74d4
struct CursorSource *DAT_004c74d4;

// GLOBAL: LEGOLAND 0x004c74d8
struct RideSpriteInfo DAT_004c74d8;

// GLOBAL: LEGOLAND 0x004c8d2c
struct Sprite *LogFlumeFc2M1Sprite;

// GLOBAL: LEGOLAND 0x004c8d38
struct Footprint DAT_004c8d38;

// GLOBAL: LEGOLAND 0x004c8d4c
void *DAT_004c8d4c;

// GLOBAL: LEGOLAND 0x004c8d50
unsigned int DAT_004c8d50;

// GLOBAL: LEGOLAND 0x004c8d54
struct Sprite *LogFlumeEntranceLayer;

// GLOBAL: LEGOLAND 0x004c8d78
struct Cursor DAT_004c8d78;

// GLOBAL: LEGOLAND 0x004c8d68
struct Sprite *LogFlumeFc3M3Sprite;

// GLOBAL: LEGOLAND 0x004c8d6c
struct CursorSource *DAT_004c8d6c;

// GLOBAL: LEGOLAND 0x004c8d70
struct Sprite *LogFlumeFc4MSprite;

// GLOBAL: LEGOLAND 0x004c8d74
struct RenderItemNode *DAT_004c8d74;

// GLOBAL: LEGOLAND 0x004ca5ac
struct RenderItemNode *DAT_004ca5ac;

// GLOBAL: LEGOLAND 0x004ca5b0
struct Cursor DAT_004ca5b0;

// GLOBAL: LEGOLAND 0x004cbe08
struct Sprite *LogFlumeFc3M2Sprite;

// GLOBAL: LEGOLAND 0x004cbe0c
struct Sprite *LogFlumeFc3M1Sprite;

// GLOBAL: LEGOLAND 0x004cbe10
void *DAT_004cbe10;

// GLOBAL: LEGOLAND 0x004cbe14
struct Sprite *LogFlumeCsaw2MSprite;

// GLOBAL: LEGOLAND 0x004cbe18
struct CursorSource *DAT_004cbe18;

// GLOBAL: LEGOLAND 0x004cbe1c
struct Sprite *LogFlumeFc1M3Sprite;

// GLOBAL: LEGOLAND 0x004cbe20
unsigned int DAT_004cbe20;

// GLOBAL: LEGOLAND 0x004cbe24
unsigned int DAT_004cbe24;

// GLOBAL: LEGOLAND 0x004cbe28
unsigned int DAT_004cbe28;

// GLOBAL: LEGOLAND 0x004cbe2c
unsigned int DAT_004cbe2c;

// GLOBAL: LEGOLAND 0x004cbe30
struct Ride *LogFlumeTrackRide;

// GLOBAL: LEGOLAND 0x004cbe48
void *DAT_004cbe48;

// GLOBAL: LEGOLAND 0x004cbe4c
struct Sprite *LogFlumeEnta3MSprite;

// GLOBAL: LEGOLAND 0x004cbe50
struct SpriteSet *LogFlumeTrackEndyList;

// GLOBAL: LEGOLAND 0x004cbe58
struct RideSpriteInfo DAT_004cbe58;

// GLOBAL: LEGOLAND 0x004cbe70
struct RenderItemNode *DAT_004cbe70;

// GLOBAL: LEGOLAND 0x004cbe74
struct Sprite *LogFlumeBarrelSprite;

// GLOBAL: LEGOLAND 0x004cbe78
struct Sprite *LogFlumeBarrelMSprite;

// GLOBAL: LEGOLAND 0x004cbe7c
struct Sprite *LogFlumeBarrel1Sprite;

// GLOBAL: LEGOLAND 0x004cbe80
struct Sprite *LogFlumeBarrelMatteSprite;

// GLOBAL: LEGOLAND 0x004cbe84
struct FlumeEntry *FlumeEntryList;

// GLOBAL: LEGOLAND 0x004cbe88
struct Sprite *LogFlumeEnta1Matte2Sprite;

// GLOBAL: LEGOLAND 0x004cbe8c
struct Sprite *LogFlumeSignSprite;

// GLOBAL: LEGOLAND 0x004cbe90
struct Sprite *LogFlumeEntrance2Sprite;

// GLOBAL: LEGOLAND 0x004cbe94
struct Sprite *LogFlumeEntrance3Sprite;

// GLOBAL: LEGOLAND 0x004cbe98
struct Sprite *LogFlumeEntrance1Sprite;

// GLOBAL: LEGOLAND 0x004cbe9c
unsigned int DrivingSchoolPumpsRide;

// GLOBAL: LEGOLAND 0x004cbea4
void *PumpList;

// GLOBAL: LEGOLAND 0x004cbeac
struct RideQueueEntry *DAT_004cbeac;

// GLOBAL: LEGOLAND 0x004cbeb0
int DAT_004cbeb0;

// GLOBAL: LEGOLAND 0x004cbeb4
int DAT_004cbeb4;

// GLOBAL: LEGOLAND 0x004cbeb8
unsigned int DAT_004cbeb8;

// GLOBAL: LEGOLAND 0x004cbec4
struct SafariOwner *SafariRide;

// GLOBAL: LEGOLAND 0x004cbed0
struct RideSpriteInfo DAT_004cbed0;

// GLOBAL: LEGOLAND 0x004cbec0
void *SafariOffBNV;

// GLOBAL: LEGOLAND 0x004cbec8
struct Sprite *SafariLayer;

// GLOBAL: LEGOLAND 0x004cbecc
struct RenderItemNode *DAT_004cbecc;

// GLOBAL: LEGOLAND 0x004cbee8
int DAT_004cbee8;

// GLOBAL: LEGOLAND 0x004cbeec
int DAT_004cbeec;

// GLOBAL: LEGOLAND 0x004cbef4
void *SafariRunBNV;

// GLOBAL: LEGOLAND 0x007988d0
void *DAT_007988d0[600];

// GLOBAL: LEGOLAND 0x00799230
struct DirectMusicSegment *MusicSegments[35];

// GLOBAL: LEGOLAND 0x00799c1c
unsigned char *DAT_00799c1c[35];

// GLOBAL: LEGOLAND 0x0079a608
int DAT_0079a608[35];

// GLOBAL: LEGOLAND 0x0079a69c
void *DMusicNotificationEvent;

// GLOBAL: LEGOLAND 0x0079a6b0
int DAT_0079a6b0;

// GLOBAL: LEGOLAND 0x007fe9a4
int DAT_007fe9a4;

// GLOBAL: LEGOLAND 0x007fea14
int DAT_007fea14;

// GLOBAL: LEGOLAND 0x007fea1c
int DAT_007fea1c;

// GLOBAL: LEGOLAND 0x007fea4c
struct DrawClipOrigin SpriteClipOrigin;

// GLOBAL: LEGOLAND 0x007febac
struct DrawClipSize DrawClipExtent;

// GLOBAL: LEGOLAND 0x0082c66c
struct Sprite *ZSafariSprite;

// GLOBAL: LEGOLAND 0x0082c670
struct Point DAT_0082c670;

// GLOBAL: LEGOLAND 0x004cbef8
void *DAT_004cbef8;

// GLOBAL: LEGOLAND 0x004cbefc
void *DAT_004cbefc;

// GLOBAL: LEGOLAND 0x004cbf00
void *DAT_004cbf00;

// GLOBAL: LEGOLAND 0x004cbf04
void *DAT_004cbf04[1];

// GLOBAL: LEGOLAND 0x004cbf08
struct Sprite *DAT_004cbf08;

// GLOBAL: LEGOLAND 0x004cbf0c
struct SafariNode *SafariNodeList;

// GLOBAL: LEGOLAND 0x004cbf10
void *SpiderRunBinV;

// GLOBAL: LEGOLAND 0x004cbf14
struct Sprite *SpiderHutMask1Sprite;

// GLOBAL: LEGOLAND 0x004cbf18
void *SpiderOffBinV;

// GLOBAL: LEGOLAND 0x004cbf1c
struct Sprite *SpiderHutMask2Sprite;

// GLOBAL: LEGOLAND 0x004cbf20
unsigned int DAT_004cbf20;

// GLOBAL: LEGOLAND 0x004cbf24
void *SpiderOnBinV;

// GLOBAL: LEGOLAND 0x004cbf28
struct Sprite *SpiderRideLayer;

// GLOBAL: LEGOLAND 0x0082c660
Point DAT_0082c660;

// GLOBAL: LEGOLAND 0x004cbf30
void *DAT_004cbf30[2];

// GLOBAL: LEGOLAND 0x004cbf38
void *DAT_004cbf38[2];

// GLOBAL: LEGOLAND 0x004cbf40
struct RideSpriteInfo DAT_004cbf40;

// GLOBAL: LEGOLAND 0x004cbf58
struct SpiderNode *SpiderNodeList;

// GLOBAL: LEGOLAND 0x004cbf5c
struct Ride *TempleBuildingRide;

// GLOBAL: LEGOLAND 0x004cbf64
void *TempleLayer;

// GLOBAL: LEGOLAND 0x004cbf68
struct Sprite *TempleMatte1Sprite;

// GLOBAL: LEGOLAND 0x004cbf6c
struct Sprite *TempleMatte2Sprite;

// GLOBAL: LEGOLAND 0x004cbf70
struct RenderItemNode *DAT_004cbf70;

// GLOBAL: LEGOLAND 0x004cbf7c
struct SlideCar *DAT_004cbf7c;

// GLOBAL: LEGOLAND 0x004cbf78
struct Sprite *TempSlideMatteSprite;

// GLOBAL: LEGOLAND 0x004cbf80
struct SlideTrack *DAT_004cbf80;

// GLOBAL: LEGOLAND 0x004cbf98
struct RideSpriteInfo DAT_004cbf98;

// GLOBAL: LEGOLAND 0x004cbf84
struct RenderItemNode *DAT_004cbf84;

// GLOBAL: LEGOLAND 0x004cbf88
unsigned int DAT_004cbf88;

// GLOBAL: LEGOLAND 0x004cbf8c
unsigned int DAT_004cbf8c;

// GLOBAL: LEGOLAND 0x004cbfb8
unsigned int DAT_004cbfb8[3];

// GLOBAL: LEGOLAND 0x004cbfc4
void *TempSlideBinV;

// GLOBAL: LEGOLAND 0x004cbfc8
int DAT_004cbfc8;

// GLOBAL: LEGOLAND 0x004cbfcc
int DAT_004cbfcc[1];

// GLOBAL: LEGOLAND 0x004cbfd0
struct Sprite *ZTempSlideSprite;

// GLOBAL: LEGOLAND 0x004cbfd4
struct SlideNode *SlideNodeList;

// GLOBAL: LEGOLAND 0x004cbfd8
struct Sprite *TopwaterSprite;

// GLOBAL: LEGOLAND 0x004cbfdc
struct WaterContext *ElephantFountainRide;

// GLOBAL: LEGOLAND 0x004cbfe0
struct WaterSub *ShowerLayer;

// GLOBAL: LEGOLAND 0x004cbfe4
unsigned int DAT_004cbfe4;

// GLOBAL: LEGOLAND 0x004cbfe8
unsigned int WaterWorksImageListHandle;

// GLOBAL: LEGOLAND 0x004cbfec
unsigned int ElephantFountainLayer;

// GLOBAL: LEGOLAND 0x004cbff0
struct RideSpriteInfo DAT_004cbff0;

// GLOBAL: LEGOLAND 0x004cc008
struct WaterContext *WaterBlockRide;

// GLOBAL: LEGOLAND 0x004cc014
struct Sprite *ShowerSprite;

// GLOBAL: LEGOLAND 0x004cc018
void *WaterWorksImageListData;

// GLOBAL: LEGOLAND 0x004cc01c
unsigned int DAT_004cc01c;

// GLOBAL: LEGOLAND 0x004cc020
struct Sprite *WwElsquirtSprite;

// GLOBAL: LEGOLAND 0x004cc024
struct WaterContext *ShowerRide;

// GLOBAL: LEGOLAND 0x004cc028
unsigned int WaterWorksSfxRefCount;

// GLOBAL: LEGOLAND 0x004cc02c
struct WaterNode *WaterNodeList;

// GLOBAL: LEGOLAND 0x004cc030
struct WaterNode *DAT_004cc030;

// GLOBAL: LEGOLAND 0x004cc034
struct WaterNode *ElephantFountainList;
// 0x007fffc4, 0x007fffd4, 0x008003f0 are EditCursor fields (field_1404,
// field_1414[5], field_1830) — see struct Cursor in gamemap.h.

struct InterfaceIconNode;
struct InterfaceProfileObj;
struct InterfaceListNode;
struct InterfaceQueryNode;
struct InterfaceEventNode;
struct InterfaceResearchNode;

// GLOBAL: LEGOLAND 0x004cc03c
struct BoatRide *BoatRideList;

// GLOBAL: LEGOLAND 0x004cc048
struct Footprint DAT_004cc048;

// GLOBAL: LEGOLAND 0x004cc060
struct Footprint BoatingSchoolStartFootprint;

// GLOBAL: LEGOLAND 0x004cc070
int *DAT_004cc070;

// GLOBAL: LEGOLAND 0x004cc074
struct BoatRideNode *BoatRideNodeList;

// GLOBAL: LEGOLAND 0x004cc078
struct Footprint BoatingSchoolFootprint;

// GLOBAL: LEGOLAND 0x004cc088
int *DAT_004cc088;

// GLOBAL: LEGOLAND 0x004cc08c
unsigned int BoatingSchoolAnimTick;

// GLOBAL: LEGOLAND 0x004cc090
struct Cursor DAT_004cc090[4];

// GLOBAL: LEGOLAND 0x004d2164
struct MermaidNode *MermaidList;

// GLOBAL: LEGOLAND 0x004d2168
struct Cursor DAT_004d2168[4];

// GLOBAL: LEGOLAND 0x004d823c
struct PathNode *BoatPathList;

// GLOBAL: LEGOLAND 0x004d8240
struct PathNode *DAT_004d8240;

// GLOBAL: LEGOLAND 0x004d8244
struct PathNode *DAT_004d8244;

// GLOBAL: LEGOLAND 0x004d8250
struct SprEnt DAT_004d8250;

// GLOBAL: LEGOLAND 0x004d8268
unsigned int DAT_004d8268;

// GLOBAL: LEGOLAND 0x004d8270
unsigned int DAT_004d8270;

// GLOBAL: LEGOLAND 0x004d829c
float DAT_004d829c[64];

// GLOBAL: LEGOLAND 0x004d83c0
unsigned int DAT_004d83c0;

// GLOBAL: LEGOLAND 0x004d88f4
unsigned char DAT_004d88f4[0x80];

// GLOBAL: LEGOLAND 0x004d8974
unsigned char DAT_004d8974[0x40];

// GLOBAL: LEGOLAND 0x004d89c4
void *DAT_004d89c4;

// GLOBAL: LEGOLAND 0x004d89c8
unsigned int LtxFileTable[30];

// GLOBAL: LEGOLAND 0x004d8a40
unsigned int LmsFileTable[31];

// GLOBAL: LEGOLAND 0x004d8abc
unsigned int LfmFileTable[30];

// GLOBAL: LEGOLAND 0x004d8b34
unsigned int LfmFileSizes[30];

// GLOBAL: LEGOLAND 0x004d8bac
unsigned int RollercoasterLpt;

// GLOBAL: LEGOLAND 0x004d8bb0
char DAT_004d8bb0[0x100];

// GLOBAL: LEGOLAND 0x004dcbd0
void *DAT_004dcbd0[10];

// GLOBAL: LEGOLAND 0x004dcbf8
unsigned int DAT_004dcbf8;

// GLOBAL: LEGOLAND 0x004dcc00
unsigned char DAT_004dcc00[2520];

// GLOBAL: LEGOLAND 0x004dd5d8
unsigned int DAT_004dd5d8;

// GLOBAL: LEGOLAND 0x004dd5e0
void *DAT_004dd5e0[24];

// GLOBAL: LEGOLAND 0x004dd758
unsigned int CoasterTxtFile;

// GLOBAL: LEGOLAND 0x004dd760
char DAT_004dd760[256];

// GLOBAL: LEGOLAND 0x004dd860
unsigned int CoasterObjFile;

// GLOBAL: LEGOLAND 0x004dd868
unsigned int DAT_004dd868;

// GLOBAL: LEGOLAND 0x004dd86c
unsigned int DAT_004dd86c;

// GLOBAL: LEGOLAND 0x004dd870
char DAT_004dd870[0x6000];

// GLOBAL: LEGOLAND 0x0060f908
int DAT_0060f908;

// GLOBAL: LEGOLAND 0x0060f90c
int DAT_0060f90c;

// GLOBAL: LEGOLAND 0x004e3870
char DAT_004e3870[16];

// GLOBAL: LEGOLAND 0x0060f914
struct LSub DAT_0060f914[20];

// GLOBAL: LEGOLAND 0x006102f8
struct CastlePathObj DAT_006102f8[3];

// GLOBAL: LEGOLAND 0x00610a04
unsigned int DAT_00610a04;

// GLOBAL: LEGOLAND 0x00610a08
unsigned int DAT_00610a08;

// GLOBAL: LEGOLAND 0x00610a18
struct RenderItemNode *DAT_00610a18;

// GLOBAL: LEGOLAND 0x00610a20
float sqrtf_table[128];

// GLOBAL: LEGOLAND 0x00610c40
float invsqrtf_table[128];

// GLOBAL: LEGOLAND 0x00610e44
float invsqrtf_exp_table[256];

// GLOBAL: LEGOLAND 0x00611244
float sqrtf_exp_table[256];

// GLOBAL: LEGOLAND 0x00611648
unsigned int DAT_00611648 = 0;

// GLOBAL: LEGOLAND 0x0061164c
float DAT_0061164c = 0;

// GLOBAL: LEGOLAND 0x00611650
float DAT_00611650 = 0;

// GLOBAL: LEGOLAND 0x00611654
float DAT_00611654 = 0;

// GLOBAL: LEGOLAND 0x00611658
Vector3 DAT_00611658[4] = {0};

// GLOBAL: LEGOLAND 0x00611688
struct KeyValuePair DAT_00611688[11];

// GLOBAL: LEGOLAND 0x006116e0
Vector3 DAT_006116e0[4] = {0};

// GLOBAL: LEGOLAND 0x00611710
unsigned int DAT_00611710[16] = {0};

// GLOBAL: LEGOLAND 0x00611750
Vector3 DAT_00611750[4] = {0};

// GLOBAL: LEGOLAND 0x00611780
unsigned char DAT_00611780[0x40] = {0};

// GLOBAL: LEGOLAND 0x006117c0
Vector3 DAT_006117c0[34] = {0};

// GLOBAL: LEGOLAND 0x00611958
unsigned int DAT_00611958;

// GLOBAL: LEGOLAND 0x00612178
int DAT_00612178[20];

// GLOBAL: LEGOLAND 0x006122a0
float DAT_006122a0[90][3];

// GLOBAL: LEGOLAND 0x00614858
struct CastleFloatEnt DAT_00614858[8];

// GLOBAL: LEGOLAND 0x00616180
struct Cursor DAT_00616180[8];

// GLOBAL: LEGOLAND 0x00622320
struct Cursor JungleCruiseCursors[4];

// GLOBAL: LEGOLAND 0x006159c8
int DAT_006159c8[90][4];

// GLOBAL: LEGOLAND 0x00615f6c
unsigned int DAT_00615f6c;

// GLOBAL: LEGOLAND 0x00615f70
unsigned int DAT_00615f70[5];

// GLOBAL: LEGOLAND 0x00615f80
void *DAT_00615f80;

// GLOBAL: LEGOLAND 0x00615ff0
int DAT_00615ff0;

// GLOBAL: LEGOLAND 0x00615ff4
float DAT_00615ff4;

// GLOBAL: LEGOLAND 0x004b5f80
struct CastleFloatEnt DAT_004b5f80[3][4];

// GLOBAL: LEGOLAND 0x00612210
struct CastleFloatEnt DAT_00612210[3][4];

// GLOBAL: LEGOLAND 0x00615fc4
int DAT_00615fc4;

// GLOBAL: LEGOLAND 0x00615fc8
int DAT_00615fc8;

// GLOBAL: LEGOLAND 0x00615fcc
int DAT_00615fcc;

// GLOBAL: LEGOLAND 0x00615fd0
float DAT_00615fd0;

// GLOBAL: LEGOLAND 0x00615fd8
float DAT_00615fd8;

// GLOBAL: LEGOLAND 0x00615fdc
float DAT_00615fdc;

// GLOBAL: LEGOLAND 0x00615fe0
float DAT_00615fe0;

// GLOBAL: LEGOLAND 0x00615fe4
int DAT_00615fe4;

// GLOBAL: LEGOLAND 0x00615fe8
int DAT_00615fe8;

// GLOBAL: LEGOLAND 0x00615fec
int DAT_00615fec;

// GLOBAL: LEGOLAND 0x00615f8c
float *DAT_00615f8c;

// GLOBAL: LEGOLAND 0x004b63fc
int (*DAT_004b63fc)(float (*fn)(unsigned int), float lo, float hi, float *out);

// GLOBAL: LEGOLAND 0x00615f84
void *DAT_00615f84;

// GLOBAL: LEGOLAND 0x00615f90
int DAT_00615f90;

// GLOBAL: LEGOLAND 0x00615fd4
float DAT_00615fd4;

// GLOBAL: LEGOLAND 0x00615f98
unsigned int DAT_00615f98;

// GLOBAL: LEGOLAND 0x00616000
unsigned int CoasterTrainWheelLms;

// GLOBAL: LEGOLAND 0x00616004
unsigned int CoasterTrainWheelLfm;

// GLOBAL: LEGOLAND 0x00616010
struct BinVFile *BalloonzBinV;

// GLOBAL: LEGOLAND 0x00616018
struct BinVFile *DAT_00616018[4];

// GLOBAL: LEGOLAND 0x0061603c
struct Sprite *DAT_0061603c[1];

// GLOBAL: LEGOLAND 0x00616040
struct Sprite *DAT_00616040;

// GLOBAL: LEGOLAND 0x00616044
struct Sprite *BalloonzLayer;

// GLOBAL: LEGOLAND 0x00616048
struct Sprite *BallBaseM1Sprite;

// GLOBAL: LEGOLAND 0x0061604c
struct Sprite *BallBaseM2Sprite;

// GLOBAL: LEGOLAND 0x00616050
struct Sprite *BallBaseM3Sprite;

// GLOBAL: LEGOLAND 0x00616054
struct Sprite *BZRedCarM1Sprite;

// GLOBAL: LEGOLAND 0x00616058
struct Sprite *BZGreenCarM1Sprite;

// GLOBAL: LEGOLAND 0x0061605c
struct Sprite *BZBlueCarM1Sprite;

// GLOBAL: LEGOLAND 0x00616060
struct BalloonNode *BalloonNodeList;

// GLOBAL: LEGOLAND 0x00616068
void *CarouselLayer;

// GLOBAL: LEGOLAND 0x0061606c
struct Sprite *CarouselEntranceMatteSprite;

// GLOBAL: LEGOLAND 0x00616070
struct Sprite *CarouselEntranceMatte2Sprite;

// GLOBAL: LEGOLAND 0x00616078
int DAT_00616078;

// GLOBAL: LEGOLAND 0x0061607c
int DAT_0061607c;

// GLOBAL: LEGOLAND 0x00616080
void *CarouselOnBinV;

// GLOBAL: LEGOLAND 0x00616084
void *CarouselOffBinV;

// GLOBAL: LEGOLAND 0x0061608c
void *CarouselBinV;

// GLOBAL: LEGOLAND 0x00616090
void *DAT_00616090;

// GLOBAL: LEGOLAND 0x00616094
void *DAT_00616094;

// GLOBAL: LEGOLAND 0x00616098
void *DAT_00616098;

// GLOBAL: LEGOLAND 0x006160b8
struct Sprite *ZCarouselSprite;

// GLOBAL: LEGOLAND 0x006160bc
struct CarouselRide *DAT_006160bc;

// GLOBAL: LEGOLAND 0x006160c0
struct Sprite *DAT_006160c0;

// GLOBAL: LEGOLAND 0x006160c4
struct CarouselNode *CarouselNodeList;

// GLOBAL: LEGOLAND 0x006160c8
unsigned int DAT_006160c8;

// GLOBAL: LEGOLAND 0x006160cc
unsigned int DAT_006160cc;

// GLOBAL: LEGOLAND 0x006160d0
unsigned int EarthSlideRide;

// GLOBAL: LEGOLAND 0x006160d4
struct RinData *EarthSlideRin;

// GLOBAL: LEGOLAND 0x006160d8
struct Sprite *EarthSlideEntranceMatteSprite;

// GLOBAL: LEGOLAND 0x006160e0
struct Sprite *EarthSlideEntranceMatte2Sprite;

// GLOBAL: LEGOLAND 0x006160e4
struct Position *EarthPos;

// GLOBAL: LEGOLAND 0x006160e8
struct EarthNode *EarthNodeHead;

// GLOBAL: LEGOLAND 0x006160ec
struct RenderItemNode *BlokeRenderList;

// GLOBAL: LEGOLAND 0x006160f4
struct Ride *EntranceRide;

// GLOBAL: LEGOLAND 0x006160f0
struct Sprite *EntranceLayer;

// GLOBAL: LEGOLAND 0x00616110
int DAT_00616110;

// GLOBAL: LEGOLAND 0x006160fc
struct Sprite *EntranceMatte1Sprite;

// GLOBAL: LEGOLAND 0x00616100
struct Sprite *EntranceMatte2Sprite;

// GLOBAL: LEGOLAND 0x00616104
struct Sprite *EntranceMatte3Sprite;

// GLOBAL: LEGOLAND 0x00616108
struct Sprite *EntranceMatte4Sprite;

// GLOBAL: LEGOLAND 0x0061610c
struct Sprite *Booth1Sprite;

// GLOBAL: LEGOLAND 0x00616118
unsigned int EateryLayerOwner;

// GLOBAL: LEGOLAND 0x00616120
struct RideSpriteInfo DAT_00616120;

// GLOBAL: LEGOLAND 0x0061613c
unsigned int BrollyImagesHandle;

// GLOBAL: LEGOLAND 0x00616140
struct BrollyData *BrollyImagesData;

// GLOBAL: LEGOLAND 0x00616144
struct BrollyNode *DAT_00616144;

// GLOBAL: LEGOLAND 0x00616148
struct SaveBlock *SaveBlockList;

// GLOBAL: LEGOLAND 0x0061614c
unsigned int HedgeImagesData;

// GLOBAL: LEGOLAND 0x00616150
unsigned int FlowerImagesHandle;

// GLOBAL: LEGOLAND 0x00616158
unsigned int FlowerImagesData;

// GLOBAL: LEGOLAND 0x0061615c
unsigned int HedgeImagesHandle;

// GLOBAL: LEGOLAND 0x00616164
struct JungleRide *JungleRideList;

// GLOBAL: LEGOLAND 0x00629c2c
struct JungleObj *DAT_00629c2c;

// GLOBAL: LEGOLAND 0x00629c30
struct JungleFish *JungleFishList;

// GLOBAL: LEGOLAND 0x00629c34
struct JungleObj *DAT_00629c34;

// GLOBAL: LEGOLAND 0x00629c3c
struct JungleScore *JungleScoreList;

// GLOBAL: LEGOLAND 0x00629c40
struct Footprint JungleCruiseFootprint;

// GLOBAL: LEGOLAND 0x00629c50
struct Footprint *DAT_00629c50;

// GLOBAL: LEGOLAND 0x00629c54
int JungleCruiseStep;

// GLOBAL: LEGOLAND 0x00629c58
struct Cursor DAT_00629c58[4];

// GLOBAL: LEGOLAND 0x0062fd2c
struct JunglePath *JunglePathList;

// GLOBAL: LEGOLAND 0x0062fd30
struct JunglePath *DAT_0062fd30;

// GLOBAL: LEGOLAND 0x0062fd34
struct JunglePath *JungleBfsNextFrontier;

// GLOBAL: LEGOLAND 0x0062fd3c
struct JailCell *JailCellList;

// GLOBAL: LEGOLAND 0x0062fd40
void *DAT_0062fd40;

// GLOBAL: LEGOLAND 0x0062fd48
struct RideSpriteInfo DAT_0062fd48;

// GLOBAL: LEGOLAND 0x0062fd60
void *SpaceTowerLayers;

// GLOBAL: LEGOLAND 0x0062fd64
struct Sprite *SpaceTowerSeatMatteSprites[4];

// GLOBAL: LEGOLAND 0x0062fd74
void *SpaceTowerRideObj;

// GLOBAL: LEGOLAND 0x0062fd7c
struct Sprite *SpaceTowerMatte2Sprite;

// GLOBAL: LEGOLAND 0x0062fd80
struct Sprite *SpaceTowerMatte1Sprite;

// GLOBAL: LEGOLAND 0x0062fd88
struct Point DAT_0062fd88[4];

// GLOBAL: LEGOLAND 0x0062fda8
struct SpaceTowerCar *SpaceTowerCarList;

// GLOBAL: LEGOLAND 0x0062fdb0
struct RideSpriteInfo DAT_0062fdb0;

// GLOBAL: LEGOLAND 0x0062fdc8
void *BoxBlokesOffBNV;

// GLOBAL: LEGOLAND 0x0062fdcc
struct Sprite *SpinningBarrelsEntranceMatte2Sprite;

// GLOBAL: LEGOLAND 0x0062fdd0
struct Sprite *SpinningBarrelsEntranceMatteSprite;

// GLOBAL: LEGOLAND 0x0062fde4
struct Ride *SpinningBarrelsRide;

// GLOBAL: LEGOLAND 0x0062fde0
struct Sprite *SpinningBarrelsLayer;

// GLOBAL: LEGOLAND 0x0062fdd8
struct Point DAT_0062fdd8;

// GLOBAL: LEGOLAND 0x0062fde8
void *SpinningBarrelsBNV;

// GLOBAL: LEGOLAND 0x0062fdf0
void *DAT_0062fdf0[3];

// GLOBAL: LEGOLAND 0x0062fdfc
void *BoxBlokesOn1BNV;

// GLOBAL: LEGOLAND 0x0062fe00
void *DAT_0062fe00[1];

// GLOBAL: LEGOLAND 0x0062fe04
struct Sprite *ZSpinningBarrelsSprite;

// GLOBAL: LEGOLAND 0x0062fe08
struct BarrelNode *SpinningBarrelList;

// GLOBAL: LEGOLAND 0x0062fe48
struct RideLayer *PottingShedLayer;

// GLOBAL: LEGOLAND 0x0062fe4c
struct Sprite *GShedMatteSprite;

// GLOBAL: LEGOLAND 0x0062fe50
struct RideLayer *MechanicsHutLayer;

// GLOBAL: LEGOLAND 0x0062fe54
struct Sprite *MechHutMaskSprite;

// GLOBAL: LEGOLAND 0x0062fe58
struct Ride *PlaneRide;

// GLOBAL: LEGOLAND 0x0062fe60
struct RideSpriteInfo DAT_0062fe60;

// GLOBAL: LEGOLAND 0x0062fe78
void *Zoomer0ffBinV;

// GLOBAL: LEGOLAND 0x0062fe7c
struct Sprite *PlaneRideLayer;

// GLOBAL: LEGOLAND 0x0062fe98
struct Sprite *DAT_0062fe98;

// GLOBAL: LEGOLAND 0x0062fe84
void *DAT_0062fe84[3];

// GLOBAL: LEGOLAND 0x0062fe90
void *ZoomerideBinV;

// GLOBAL: LEGOLAND 0x0062fe94
void *Zoomer0nBinV;

// GLOBAL: LEGOLAND 0x0062fe9c
struct PlaneRideNode *PlaneRideNodeList;

// GLOBAL: LEGOLAND 0x0062fea0
int DialogListScrollY;

// GLOBAL: LEGOLAND 0x0062fea4
int DAT_0062fea4;

// GLOBAL: LEGOLAND 0x0062fea8
int *DAT_0062fea8;

// GLOBAL: LEGOLAND 0x0062feac
void *AltWomanFileData;

// GLOBAL: LEGOLAND 0x0062feb0
void *GeoffMeshes[2];

// GLOBAL: LEGOLAND 0x0062feb8
int DAT_0062feb8[1];

// GLOBAL: LEGOLAND 0x0062febc
void *ManMeshes[6];

// GLOBAL: LEGOLAND 0x0062fed4
void *WomanMeshes[6];

// GLOBAL: LEGOLAND 0x0062feec
unsigned int DAT_0062feec[1];

// GLOBAL: LEGOLAND 0x0062fef0
unsigned int DAT_0062fef0;

// GLOBAL: LEGOLAND 0x0062fef4
void *TracyWalkMesh;

// GLOBAL: LEGOLAND 0x0062fef8
int *DAT_0062fef8;

// GLOBAL: LEGOLAND 0x00630100
void *AltManFileData;

// GLOBAL: LEGOLAND 0x00630108
unsigned int DAT_00630108[0x2000];

// GLOBAL: LEGOLAND 0x00638218
unsigned int DAT_00638218[80];

// GLOBAL: LEGOLAND 0x00638358
unsigned short DAT_00638358;

// GLOBAL: LEGOLAND 0x0064cd8c
int *DAT_0064cd8c;

// GLOBAL: LEGOLAND 0x00655a38
int *DAT_00655a38;

// GLOBAL: LEGOLAND 0x00655a3c
void *PersonListHead;

// GLOBAL: LEGOLAND 0x00655a4c
unsigned int RenderItemCount;

// GLOBAL: LEGOLAND 0x00655a50
unsigned int RenderItem2Count;

// GLOBAL: LEGOLAND 0x00665e8c
unsigned int DAT_00665e8c;

// GLOBAL: LEGOLAND 0x00665eec
int DAT_00665eec;

// GLOBAL: LEGOLAND 0x00665f48
unsigned int DAT_00665f48;

// GLOBAL: LEGOLAND 0x00665f5c
void *DAT_00665f5c;

// GLOBAL: LEGOLAND 0x00665f60
void *DAT_00665f60;

// GLOBAL: LEGOLAND 0x00665f64
unsigned int DAT_00665f64;

// GLOBAL: LEGOLAND 0x00665f68
unsigned int DAT_00665f68;

// GLOBAL: LEGOLAND 0x00665f6c
unsigned int DAT_00665f6c;

// GLOBAL: LEGOLAND 0x00665fe8
unsigned int DAT_00665fe8;

// GLOBAL: LEGOLAND 0x00665fec
unsigned int DAT_00665fec;

// GLOBAL: LEGOLAND 0x00665ff8
unsigned int ReportFlags;

// GLOBAL: LEGOLAND 0x00665ffc
unsigned int DAT_00665ffc;

// GLOBAL: LEGOLAND 0x00666000
unsigned int DAT_00666000;

// GLOBAL: LEGOLAND 0x00666004
unsigned int DAT_00666004;

// GLOBAL: LEGOLAND 0x00666008
unsigned int DAT_00666008;

// GLOBAL: LEGOLAND 0x0066600c
unsigned int DAT_0066600c;

// GLOBAL: LEGOLAND 0x00666010
unsigned int DAT_00666010;

// GLOBAL: LEGOLAND 0x00666014
unsigned int DAT_00666014;

// GLOBAL: LEGOLAND 0x00666018
unsigned int DAT_00666018;

// GLOBAL: LEGOLAND 0x0066601c
unsigned int DAT_0066601c;

// GLOBAL: LEGOLAND 0x00666020
unsigned int DAT_00666020;

// GLOBAL: LEGOLAND 0x00666024
unsigned int DAT_00666024;

// GLOBAL: LEGOLAND 0x00666028
unsigned int DAT_00666028;

// GLOBAL: LEGOLAND 0x0066602c
unsigned int DAT_0066602c;

// GLOBAL: LEGOLAND 0x00666030
unsigned int DAT_00666030;

// GLOBAL: LEGOLAND 0x00666034
unsigned int DAT_00666034;

// GLOBAL: LEGOLAND 0x00666038
unsigned int DAT_00666038;

// GLOBAL: LEGOLAND 0x0066603c
unsigned int DAT_0066603c;

// GLOBAL: LEGOLAND 0x00666040
unsigned int DAT_00666040;

// GLOBAL: LEGOLAND 0x00666044
unsigned int DAT_00666044;

// GLOBAL: LEGOLAND 0x00666048
unsigned int DAT_00666048;

// GLOBAL: LEGOLAND 0x0066604c
unsigned int DAT_0066604c;

// GLOBAL: LEGOLAND 0x00666050
unsigned int DAT_00666050;

// GLOBAL: LEGOLAND 0x00666054
unsigned int DAT_00666054;

// GLOBAL: LEGOLAND 0x00666058
unsigned int DAT_00666058;

// GLOBAL: LEGOLAND 0x0066605c
unsigned int DAT_0066605c;

// GLOBAL: LEGOLAND 0x00666060
unsigned int DAT_00666060;

// GLOBAL: LEGOLAND 0x00666064
unsigned int DAT_00666064;

// GLOBAL: LEGOLAND 0x00666068
unsigned int DAT_00666068;

// GLOBAL: LEGOLAND 0x0066606c
unsigned int DAT_0066606c;

// GLOBAL: LEGOLAND 0x00666070
unsigned int DAT_00666070;

// GLOBAL: LEGOLAND 0x00666074
unsigned int DAT_00666074;

// GLOBAL: LEGOLAND 0x00666078
unsigned int DAT_00666078;

// GLOBAL: LEGOLAND 0x0066607c
unsigned int DAT_0066607c;

// GLOBAL: LEGOLAND 0x00666080
unsigned int DAT_00666080;

// GLOBAL: LEGOLAND 0x00666084
unsigned int DAT_00666084;

// GLOBAL: LEGOLAND 0x00666088
unsigned int DAT_00666088;

// GLOBAL: LEGOLAND 0x0066608c
unsigned int DAT_0066608c;

// GLOBAL: LEGOLAND 0x00666090
unsigned int DAT_00666090;

// GLOBAL: LEGOLAND 0x00666094
unsigned int DAT_00666094;

// GLOBAL: LEGOLAND 0x00666098
unsigned int AppraisalDeadline;

// GLOBAL: LEGOLAND 0x0066609c
int DAT_0066609c;

// GLOBAL: LEGOLAND 0x006660a0
int DAT_006660a0;

// GLOBAL: LEGOLAND 0x006660a4
int DAT_006660a4;

// GLOBAL: LEGOLAND 0x006660a8
struct IconNode *DAT_006660a8;

// GLOBAL: LEGOLAND 0x006660ac
struct IconNode *DAT_006660ac;

// GLOBAL: LEGOLAND 0x006660b0
char DAT_006660b0[256];

// GLOBAL: LEGOLAND 0x006661bc
int DAT_006661bc;

// GLOBAL: LEGOLAND 0x006661c0
struct Element *SharkCafeBrollyElem;

// GLOBAL: LEGOLAND 0x006661c4
struct Element *Entrance1Elem;

// GLOBAL: LEGOLAND 0x006661c8
int DAT_006661c8;

// GLOBAL: LEGOLAND 0x006661cc
char DAT_006661cc[8][100];

// GLOBAL: LEGOLAND 0x006664ec
int DAT_006664ec;

// GLOBAL: LEGOLAND 0x006664f8
struct BuildObj BuildObjArray[256] = {0};

// GLOBAL: LEGOLAND 0x006670f8
int BuildObjCount = 0;

// GLOBAL: LEGOLAND 0x006670fc
unsigned int ButtonRepeatDelay;

// GLOBAL: LEGOLAND 0x00667104
unsigned int ControllersInitialized;

// GLOBAL: LEGOLAND 0x00667108
unsigned int DAT_00667108;

// GLOBAL: LEGOLAND 0x0066710c
unsigned int ButtonRepeatLastTick;

// GLOBAL: LEGOLAND 0x00667114
unsigned int FountainSFXRefCount;

// GLOBAL: LEGOLAND 0x00667118
unsigned int PowerStationSFXRefCount;

// GLOBAL: LEGOLAND 0x0066711c
unsigned int DinoSFXRefCount;

// GLOBAL: LEGOLAND 0x00667120
unsigned int MoneySFXLoadCount;

// GLOBAL: LEGOLAND 0x00667128
char LogBuffer[512];

// GLOBAL: LEGOLAND 0x0066752c
char ProductVersionString[0x88];

// GLOBAL: LEGOLAND 0x006675b4
unsigned int BubbleHelpGFXLoaded;

// GLOBAL: LEGOLAND 0x006675b8
volatile int TextCellCount;

// GLOBAL: LEGOLAND 0x006675c0
struct TextCell TextCells[50];

// GLOBAL: LEGOLAND 0x00667c00
int MapWorldMinX;

// GLOBAL: LEGOLAND 0x00667c04
int DAT_00667c04;

// GLOBAL: LEGOLAND 0x00667c08
int DAT_00667c08;

// GLOBAL: LEGOLAND 0x00667c0c
int DAT_00667c0c;

// GLOBAL: LEGOLAND 0x00667c14
short DAT_00667c14;

// GLOBAL: LEGOLAND 0x00667c16
short DAT_00667c16;

// GLOBAL: LEGOLAND 0x00667c18
int MapWorldHeight;

// GLOBAL: LEGOLAND 0x00667c1c
int MapWorldWidth;

// GLOBAL: LEGOLAND 0x00667c20
int MapCenterOffsetY;

// GLOBAL: LEGOLAND 0x00667c10
unsigned int DAT_00667c10;

// GLOBAL: LEGOLAND 0x00667c28
unsigned int DAT_00667c28;

// GLOBAL: LEGOLAND 0x00667c2c
struct Sprite *FullMapSprite;

// GLOBAL: LEGOLAND 0x00667c30
unsigned int DAT_00667c30;

// GLOBAL: LEGOLAND 0x00667c34
struct Sprite *MapSpannerSprite;

// GLOBAL: LEGOLAND 0x00667c3c
int DAT_00667c3c;

// GLOBAL: LEGOLAND 0x00667c40
const char *DAT_00667c40;

// GLOBAL: LEGOLAND 0x00667c48
unsigned int DAT_00667c48;

// GLOBAL: LEGOLAND 0x00667c4c
int DAT_00667c4c;

// GLOBAL: LEGOLAND 0x00667c54
LEGO_EXPORT TileId QueryObj;

// GLOBAL: LEGOLAND 0x00667c58
LEGO_EXPORT struct ObjClass *QueryClass;

// GLOBAL: LEGOLAND 0x00667c5c
unsigned int DAT_00667c5c;

// GLOBAL: LEGOLAND 0x00667c60
unsigned int DAT_00667c60;

// GLOBAL: LEGOLAND 0x00667c68
unsigned int DAT_00667c68;

// GLOBAL: LEGOLAND 0x00667c70
long DAT_00667c70;

// GLOBAL: LEGOLAND 0x00667c74
long DAT_00667c74;

// GLOBAL: LEGOLAND 0x00667c64
unsigned int DAT_00667c64;

// GLOBAL: LEGOLAND 0x00667c78
unsigned int SamplesPaused;

// GLOBAL: LEGOLAND 0x00667c7c
unsigned int MapLoaded;

// GLOBAL: LEGOLAND 0x00667c80
unsigned int DAT_00667c80;

// GLOBAL: LEGOLAND 0x00667c88
struct Sprite *Arrow02Sprite;

// GLOBAL: LEGOLAND 0x00667c8c
struct Sprite *Arrow01Sprite;

// GLOBAL: LEGOLAND 0x00667c90
struct Sprite *Arrow04Sprite;

// GLOBAL: LEGOLAND 0x00667c94
struct Sprite *Arrow03Sprite;

// GLOBAL: LEGOLAND 0x00667c9c
void *GameMapRawBlock;

// GLOBAL: LEGOLAND 0x00667ca0
unsigned int LoadInProgress;

// GLOBAL: LEGOLAND 0x00667ca4
unsigned int DAT_00667ca4;

// GLOBAL: LEGOLAND 0x00667ca8
LEGO_EXPORT void *OverlayList;

// GLOBAL: LEGOLAND 0x00667cac
LEGO_EXPORT unsigned int OverlayILF;

// GLOBAL: LEGOLAND 0x00667cb0
void *BridgesData;

// GLOBAL: LEGOLAND 0x00667cb4
LEGO_EXPORT int ScrollX;

// GLOBAL: LEGOLAND 0x00667cb8
LEGO_EXPORT int ScrollY;

// GLOBAL: LEGOLAND 0x00667cbc
LEGO_EXPORT int ScrollSpeedX;

// GLOBAL: LEGOLAND 0x00667cc0
LEGO_EXPORT int ScrollSpeedY;

// GLOBAL: LEGOLAND 0x00667cc4
int DAT_00667cc4;

// GLOBAL: LEGOLAND 0x00667cd0
int DAT_00667cd0;

// GLOBAL: LEGOLAND 0x00667cd4
int DAT_00667cd4;

// GLOBAL: LEGOLAND 0x00667cd8
unsigned int DAT_00667cd8;

// GLOBAL: LEGOLAND 0x00667cdc
unsigned int DAT_00667cdc;

// GLOBAL: LEGOLAND 0x00667ce0
int DAT_00667ce0;

// GLOBAL: LEGOLAND 0x00667ce4
int DAT_00667ce4;

// GLOBAL: LEGOLAND 0x00667ce8
int DAT_00667ce8;

// GLOBAL: LEGOLAND 0x00667cec
int DAT_00667cec;

// GLOBAL: LEGOLAND 0x00667cf0
int DAT_00667cf0;

// GLOBAL: LEGOLAND 0x00667cf4
int DAT_00667cf4;

// GLOBAL: LEGOLAND 0x00667cf8
int DAT_00667cf8;

// GLOBAL: LEGOLAND 0x00667cfc
int DAT_00667cfc;

// GLOBAL: LEGOLAND 0x00667d00
int DAT_00667d00;

// GLOBAL: LEGOLAND 0x00667d04
int DAT_00667d04;

// GLOBAL: LEGOLAND 0x00667d08
int DAT_00667d08;

// GLOBAL: LEGOLAND 0x00667d0c
int DAT_00667d0c;

// GLOBAL: LEGOLAND 0x00667d10
unsigned int DAT_00667d10;

// GLOBAL: LEGOLAND 0x00667d3c
int DAT_00667d3c;

// GLOBAL: LEGOLAND 0x00667d40
unsigned int DAT_00667d40;

// GLOBAL: LEGOLAND 0x00667d44
int DAT_00667d44;

// GLOBAL: LEGOLAND 0x00667d48
unsigned int DAT_00667d48;

// GLOBAL: LEGOLAND 0x00667d4c
unsigned int DAT_00667d4c;

// GLOBAL: LEGOLAND 0x00667d50
unsigned int MapDataLoaded;

// GLOBAL: LEGOLAND 0x00667d54
unsigned int DAT_00667d54;

// GLOBAL: LEGOLAND 0x00667d58
int DAT_00667d58;

// GLOBAL: LEGOLAND 0x00667d60
int LastWatchDrawTicks;

// GLOBAL: LEGOLAND 0x00667d68
unsigned int LastRenderingCompleteTick;

// GLOBAL: LEGOLAND 0x00667d6c
int WinDebugMode;

// GLOBAL: LEGOLAND 0x00667d70
LEGO_EXPORT struct DDRAWENV DDRAWENV;

// GLOBAL: LEGOLAND 0x00668070
LPDIRECTDRAWSURFACE PrimarySurface;

// GLOBAL: LEGOLAND 0x00668074
LPDIRECTDRAWSURFACE DAT_00668074;

// GLOBAL: LEGOLAND 0x00668078
LPDIRECTDRAWSURFACE OffscreenSurface;

// GLOBAL: LEGOLAND 0x0066807c
LPDIRECTDRAWSURFACE renderEngine;

// GLOBAL: LEGOLAND 0x00668080
LPDIRECTDRAWCLIPPER DDrawClipper;

// GLOBAL: LEGOLAND 0x00668084
unsigned int DDPalette;

// GLOBAL: LEGOLAND 0x00668088
unsigned int DisplayPixelFormat;

// GLOBAL: LEGOLAND 0x0066808c
HFONT LegoFont20Bold;

// GLOBAL: LEGOLAND 0x00668090
HFONT LegoFont24Bold;

// GLOBAL: LEGOLAND 0x00668094
HFONT LegoFont18SemiBold;

// GLOBAL: LEGOLAND 0x00668098
HFONT LegoFont28Normal;

// GLOBAL: LEGOLAND 0x0066809c
DDSURFACEDESC CurrentSurfaceDesc;

// GLOBAL: LEGOLAND 0x00668108
RECT DAT_00668108;

// GLOBAL: LEGOLAND 0x00668118
int renderEngineTargetIdx;

// GLOBAL: LEGOLAND 0x0066811c
LPDIRECTDRAWSURFACE renderEngineTargets[10];

// GLOBAL: LEGOLAND 0x00668144
int VideoSurfaceLocked;

// GLOBAL: LEGOLAND 0x00668148
struct Sprite *DAT_00668148;

// GLOBAL: LEGOLAND 0x0066814c
unsigned int DAT_0066814c;

// GLOBAL: LEGOLAND 0x00668164
int DAT_00668164[32];

// GLOBAL: LEGOLAND 0x006681e4
int RenderingStatusStackDepth;

// GLOBAL: LEGOLAND 0x006681e8
unsigned int OverridePalette;

// GLOBAL: LEGOLAND 0x006681ec
void *DAT_006681ec;

// GLOBAL: LEGOLAND 0x006681f0
unsigned int FpsWindowStartTicks;

// GLOBAL: LEGOLAND 0x006681f4
unsigned int LastFrameTicks;

// GLOBAL: LEGOLAND 0x006681f8
int FramesThisSecond;

// GLOBAL: LEGOLAND 0x006681fc
LEGO_EXPORT unsigned int LastFrameMS;

// GLOBAL: LEGOLAND 0x00668200
unsigned int LastPresentTicks;

// GLOBAL: LEGOLAND 0x00668204
unsigned int WatchActive;

// GLOBAL: LEGOLAND 0x00668208
struct Sprite *WatchSprite;

// GLOBAL: LEGOLAND 0x0066820c
char DAT_0066820c[0x404];

// GLOBAL: LEGOLAND 0x00668610
unsigned int DAT_00668610;

// GLOBAL: LEGOLAND 0x00668614
unsigned int DAT_00668614;

// GLOBAL: LEGOLAND 0x00668618
unsigned int DAT_00668618;

// GLOBAL: LEGOLAND 0x0066861c
char DAT_0066861c[128];

// GLOBAL: LEGOLAND 0x0066869c
char DAT_0066869c[128];

// GLOBAL: LEGOLAND 0x0066871c
unsigned int DAT_0066871c;

// GLOBAL: LEGOLAND 0x00668720
unsigned int ScriptStringCount;

// GLOBAL: LEGOLAND 0x00668724
struct ObjectiveEvent *DAT_00668724;

// GLOBAL: LEGOLAND 0x00668728
struct ObjectiveEvent *ObjectiveEventList;

// GLOBAL: LEGOLAND 0x0066872c
unsigned int DAT_0066872c[0x15];

// GLOBAL: LEGOLAND 0x00668780
unsigned int DAT_00668780;

// GLOBAL: LEGOLAND 0x00668784
void *DAT_00668784;

// GLOBAL: LEGOLAND 0x00668788
unsigned int DAT_00668788;

// GLOBAL: LEGOLAND 0x0066878c
int DAT_0066878c;

// GLOBAL: LEGOLAND 0x00668790
unsigned int DAT_00668790;

// GLOBAL: LEGOLAND 0x00668794
unsigned int DAT_00668794;

// GLOBAL: LEGOLAND 0x00668798
void *DAT_00668798;

// GLOBAL: LEGOLAND 0x0066879c
unsigned int DAT_0066879c;

// GLOBAL: LEGOLAND 0x006687a0
unsigned int ScriptLoadErrorCount;

// GLOBAL: LEGOLAND 0x006687a4
unsigned int DAT_006687a4;

// GLOBAL: LEGOLAND 0x006687a8
unsigned int DAT_006687a8;

// GLOBAL: LEGOLAND 0x006687ac
unsigned int DAT_006687ac;

// GLOBAL: LEGOLAND 0x006687b0
unsigned int DAT_006687b0;

// GLOBAL: LEGOLAND 0x006687b4
unsigned int DAT_006687b4;

// GLOBAL: LEGOLAND 0x006687bc
unsigned int DAT_006687bc;

// GLOBAL: LEGOLAND 0x006687c0
unsigned int DAT_006687c0;

// GLOBAL: LEGOLAND 0x006687c8
struct IconNode *IconListHead;

// GLOBAL: LEGOLAND 0x006687cc
struct IconNode *DAT_006687cc;

// GLOBAL: LEGOLAND 0x006687d0
LEGO_EXPORT void *FocussedIconPtr;

// GLOBAL: LEGOLAND 0x00668828
struct Sprite *GBarFrameSprite;

// GLOBAL: LEGOLAND 0x0066882c
struct Sprite *IfSideBUpSprite;

// GLOBAL: LEGOLAND 0x00668830
struct Sprite *IfSideBDownSprite;

// GLOBAL: LEGOLAND 0x00668834
struct Sprite *IfSidebar1Sprite;

// GLOBAL: LEGOLAND 0x00668840
short DAT_00668840;

// GLOBAL: LEGOLAND 0x0066884c
short DAT_0066884c;

// GLOBAL: LEGOLAND 0x00668858
struct IconNode DAT_00668858;

// GLOBAL: LEGOLAND 0x006688a8
void *DAT_006688a8;

// GLOBAL: LEGOLAND 0x006688ac
void *DAT_006688ac;

// GLOBAL: LEGOLAND 0x006688b0
void *DAT_006688b0;

// GLOBAL: LEGOLAND 0x006688b4
unsigned int LastScrollIconTick;

// GLOBAL: LEGOLAND 0x006688b8
unsigned int DAT_006688b8;

// GLOBAL: LEGOLAND 0x006688bc
int PowerDemandBarPos;

// GLOBAL: LEGOLAND 0x006688c0
int EnergyBarSupplyWidth;

// GLOBAL: LEGOLAND 0x006688c4
int DAT_006688c4;

// GLOBAL: LEGOLAND 0x006688c8
unsigned int RenderIconsLastTicks;

// GLOBAL: LEGOLAND 0x006688cc
unsigned int DAT_006688cc;

// GLOBAL: LEGOLAND 0x006688d0
unsigned int GBarSpritesLoaded;

// GLOBAL: LEGOLAND 0x006688d4
void *DAT_006688d4;

// GLOBAL: LEGOLAND 0x006688d8
void *ActiveIndicators;

// GLOBAL: LEGOLAND 0x006688e0
struct Sprite *PuBgMainSprite;

// GLOBAL: LEGOLAND 0x006688e4
struct Sprite *PuBgCentreTopSprite;

// GLOBAL: LEGOLAND 0x006688e8
struct Sprite *PuBgRightTopSprite;

// GLOBAL: LEGOLAND 0x006688ec
struct Sprite *PuBgLeftMidSprite;

// GLOBAL: LEGOLAND 0x006688f0
struct Sprite *PuBgCentreMidSprite;

// GLOBAL: LEGOLAND 0x006688f4
struct Sprite *PuBgRightMidSprite;

// GLOBAL: LEGOLAND 0x006688f8
struct Sprite *PuBgLeftBtmSprite;

// GLOBAL: LEGOLAND 0x006688fc
struct Sprite *PuBgCentreBtmSprite;

// GLOBAL: LEGOLAND 0x00668900
struct Sprite *PuBgRightBtmSprite;

// GLOBAL: LEGOLAND 0x00668904
struct Sprite *CBBGLeftSprite;

// GLOBAL: LEGOLAND 0x00668908
struct Sprite *CBBGCentreSprite;

// GLOBAL: LEGOLAND 0x0066890c
struct Sprite *CBBGRightSprite;

// GLOBAL: LEGOLAND 0x00668910
struct Sprite *NewPopMockSprite;

// GLOBAL: LEGOLAND 0x00668914
struct Sprite *PuDeleteObjectOnSprite;

// GLOBAL: LEGOLAND 0x00668918
struct Sprite *PuDeleteObjectSprite;

// GLOBAL: LEGOLAND 0x0066891c
struct Sprite *PuClosePopUpOnSprite;

// GLOBAL: LEGOLAND 0x00668920
struct Sprite *PuClosePopUpSprite;

// GLOBAL: LEGOLAND 0x00668924
struct Sprite *NextIconOnSprite;

// GLOBAL: LEGOLAND 0x00668928
struct Sprite *NextIconSprite;

// GLOBAL: LEGOLAND 0x0066892c
struct Sprite *PrevIconOnSprite;

// GLOBAL: LEGOLAND 0x00668930
struct Sprite *PrevIconSprite;

// GLOBAL: LEGOLAND 0x00668934
struct Sprite *PUOKOnSprite;

// GLOBAL: LEGOLAND 0x00668938
struct Sprite *PUOKSprite;

// GLOBAL: LEGOLAND 0x0066893c
struct Sprite *CBCloseSprite;

// GLOBAL: LEGOLAND 0x00668940
struct Sprite *CBCloseOnSprite;

// GLOBAL: LEGOLAND 0x00668944
struct Sprite *PuAddGardenerOnSprite;

// GLOBAL: LEGOLAND 0x00668948
struct Sprite *PuAddGardenerSprite;

// GLOBAL: LEGOLAND 0x0066894c
struct Sprite *PuAddMechanicsOnSprite;

// GLOBAL: LEGOLAND 0x00668950
struct Sprite *PuAddMechanicsSprite;

// GLOBAL: LEGOLAND 0x00668954
unsigned int DAT_00668954;

// GLOBAL: LEGOLAND 0x00668958
struct Sprite *PopUpSpritesLoaded;

// GLOBAL: LEGOLAND 0x0066895c
int DAT_0066895c;

// GLOBAL: LEGOLAND 0x00668960
unsigned int DAT_00668960;

// GLOBAL: LEGOLAND 0x00668964
int DAT_00668964;

// GLOBAL: LEGOLAND 0x00668968
char DAT_00668968[0x400];

// GLOBAL: LEGOLAND 0x00668d68
unsigned int DAT_00668d68;

// GLOBAL: LEGOLAND 0x00668d78
DIMOUSESTATE MouseState;

// GLOBAL: LEGOLAND 0x00668d88
void *dinput;

// GLOBAL: LEGOLAND 0x00668d8c
void *dinput_keyboard;

// GLOBAL: LEGOLAND 0x00668d90
void *dintput_mouse;

// GLOBAL: LEGOLAND 0x00668d94
char CheatKeyBuffer[0x14];

// GLOBAL: LEGOLAND 0x00668da8
unsigned char DAT_00668da8[0x3b];

// GLOBAL: LEGOLAND 0x00668de4
unsigned char DAT_00668de4[0x3b];

// GLOBAL: LEGOLAND 0x00668e20
unsigned int DAT_00668e20[4];

// GLOBAL: LEGOLAND 0x00668e34
unsigned int DAT_00668e34;

// GLOBAL: LEGOLAND 0x00668e38
unsigned int DAT_00668e38;

// GLOBAL: LEGOLAND 0x00668e3c
void *DAT_00668e3c;

// GLOBAL: LEGOLAND 0x00668e40
struct InterfaceListNode *DAT_00668e40;

// GLOBAL: LEGOLAND 0x00668e44
int DAT_00668e44[8];

// GLOBAL: LEGOLAND 0x00668e64
unsigned char DAT_00668e64;

// GLOBAL: LEGOLAND 0x00668e68
struct Sprite *InterfaceBgSprite;

// GLOBAL: LEGOLAND 0x00668e6c
struct Sprite *NoEnergySprite;

// GLOBAL: LEGOLAND 0x00668e70
struct Sprite *BarPointerSprite;

// GLOBAL: LEGOLAND 0x00668e74
struct Sprite *AttractHighlightOnSprite;

// GLOBAL: LEGOLAND 0x00668e78
struct Sprite *AttractHighlightOffSprite;

// GLOBAL: LEGOLAND 0x00668e7c
struct Sprite *AttractNewOffSprite;

// GLOBAL: LEGOLAND 0x00668e80
struct Sprite *AttractNewOnSprite;

// GLOBAL: LEGOLAND 0x00668e84
struct Sprite *SideScrollDownLitSprite;

// GLOBAL: LEGOLAND 0x00668e88
struct Sprite *SideScrollUpLitSprite;

// GLOBAL: LEGOLAND 0x00668e8c
struct Sprite *LinkMiddleSprite;

// GLOBAL: LEGOLAND 0x00668e90
struct Sprite *LinkBottomSprite;

// GLOBAL: LEGOLAND 0x00668e94
struct Sprite *BriefIcon2Sprite;

// GLOBAL: LEGOLAND 0x00668e98
struct Sprite *BriefIconSprite;

// GLOBAL: LEGOLAND 0x00668e9c
void *DAT_00668e9c;

// GLOBAL: LEGOLAND 0x00668ea0
struct Sprite *ScriptEndSprite;

// GLOBAL: LEGOLAND 0x00668ea4
unsigned int DAT_00668ea4;

// GLOBAL: LEGOLAND 0x00668eb0
unsigned int SelectedThemeIcon;

// GLOBAL: LEGOLAND 0x00668eb4
unsigned int DAT_00668eb4;

// GLOBAL: LEGOLAND 0x00668eb8
unsigned int ScriptEndIcon;

// GLOBAL: LEGOLAND 0x00668ebc
unsigned int DAT_00668ebc;

// GLOBAL: LEGOLAND 0x00668ec0
unsigned int DAT_00668ec0;

// GLOBAL: LEGOLAND 0x00668ed8
struct InterfaceResearchNode *ResearchList;

// GLOBAL: LEGOLAND 0x00668ee0
int DAT_00668ee0;

// GLOBAL: LEGOLAND 0x00668ee8
unsigned int AviAcmStreamHeader[0x15];

// GLOBAL: LEGOLAND 0x00668f3c
unsigned int AviSoundBytesPerFrame;

// GLOBAL: LEGOLAND 0x00668f44
int AcmStreamOpenResult;

// GLOBAL: LEGOLAND 0x00668f48
void *AviSoundBuffer;

// GLOBAL: LEGOLAND 0x00668f4c
unsigned int DAT_00668f4c;

// GLOBAL: LEGOLAND 0x00668f50
unsigned int DAT_00668f50;

// GLOBAL: LEGOLAND 0x00668f54
int DAT_00668f54;

// GLOBAL: LEGOLAND 0x00668f58
unsigned int DAT_00668f58;

// GLOBAL: LEGOLAND 0x00668f5c
void *AviAcmStream;

// GLOBAL: LEGOLAND 0x00668f60
int AviAudioSamplePos;

// GLOBAL: LEGOLAND 0x00668f64
unsigned int AviAudioSampleSize;

// GLOBAL: LEGOLAND 0x00668f70
WAVEFORMATEX AviPcmFormat;

// GLOBAL: LEGOLAND 0x00668f84
void *AviAudioStream;

// GLOBAL: LEGOLAND 0x00668f88
int DAT_00668f88;

// GLOBAL: LEGOLAND 0x00668f8c
void *AviAcmDstBase;

// GLOBAL: LEGOLAND 0x00668f90
unsigned int DAT_00668f90;

// GLOBAL: LEGOLAND 0x00668f9c
int DAT_00668f9c;

// GLOBAL: LEGOLAND 0x00668fa0
unsigned int DAT_00668fa0;

// GLOBAL: LEGOLAND 0x00668f98
unsigned int AviOpenCount;

// GLOBAL: LEGOLAND 0x00668fa4
unsigned int DAT_00668fa4;

// GLOBAL: LEGOLAND 0x00668fac
unsigned int PerfCounterState;

// GLOBAL: LEGOLAND 0x00668fb0
unsigned int DAT_00668fb0;

// GLOBAL: LEGOLAND 0x00668fb4
unsigned int DAT_00668fb4;

// GLOBAL: LEGOLAND 0x00668fb8
struct MoviePool *DAT_00668fb8;

// GLOBAL: LEGOLAND 0x00668fc0
void *DAT_00668fc0;

// GLOBAL: LEGOLAND 0x00668fc4
struct InterfaceQueryNode *DAT_00668fc4;

// GLOBAL: LEGOLAND 0x00668fcc
int DAT_00668fcc;

// GLOBAL: LEGOLAND 0x00668fd0
char DAT_00668fd0[128];

// GLOBAL: LEGOLAND 0x00669050
unsigned char CurrentObjectiveEventFlags;

// GLOBAL: LEGOLAND 0x00669054
unsigned int DAT_00669054;

// GLOBAL: LEGOLAND 0x00669058
char DAT_00669058[0x40];

// GLOBAL: LEGOLAND 0x00669098
unsigned int DAT_00669098;

// GLOBAL: LEGOLAND 0x006691a0
unsigned int LLIDB_Capacity;

// GLOBAL: LEGOLAND 0x006691a4
unsigned int LLIDB_ElementCount;

// GLOBAL: LEGOLAND 0x006691a8
struct Element **LLIDB_Pages;

// GLOBAL: LEGOLAND 0x006691ac
struct LLSNode *LLSPlayList;

// GLOBAL: LEGOLAND 0x006691b0
int SaveFileHandle;

// GLOBAL: LEGOLAND 0x006691b4
int SavedElementCount;

// GLOBAL: LEGOLAND 0x006691bc
int MeasuredBlockStarts[16];

// GLOBAL: LEGOLAND 0x006691fc
int MeasuredBlockDepth;

// GLOBAL: LEGOLAND 0x00669200
struct Element **SavedElementTable;

// GLOBAL: LEGOLAND 0x00669204
unsigned int GameTimerMark;

// GLOBAL: LEGOLAND 0x00669208
void *g_hInstance;

// GLOBAL: LEGOLAND 0x0066920c
int g_nCmdShow;

// GLOBAL: LEGOLAND 0x00669210
HWND WndEnvHwnd;

// GLOBAL: LEGOLAND 0x00669238
unsigned int PauseGameTimerResult;

// GLOBAL: LEGOLAND 0x00669240
LEGO_EXPORT struct Ride *ObjectClassList;

// GLOBAL: LEGOLAND 0x00669244
struct LibraryNode *ObjectLibraryHead;

// GLOBAL: LEGOLAND 0x00669248
void *ClassRideList;

// GLOBAL: LEGOLAND 0x0066924c
unsigned int DAT_0066924c;

// GLOBAL: LEGOLAND 0x00669250
int DAT_00669250;

// GLOBAL: LEGOLAND 0x00669254
unsigned int DAT_00669254;

// GLOBAL: LEGOLAND 0x00669258
unsigned int DAT_00669258[1152];

// GLOBAL: LEGOLAND 0x0066a45c
void *DAT_0066a45c[1020];

// GLOBAL: LEGOLAND 0x0066b44c
void *DAT_0066b44c;

// GLOBAL: LEGOLAND 0x0066b450
struct DirNode *DirSearchNodeList;

// GLOBAL: LEGOLAND 0x0066b454
struct DirNode *DAT_0066b454;

// GLOBAL: LEGOLAND 0x0066b458
struct DirNode *DAT_0066b458;

// GLOBAL: LEGOLAND 0x0066b460
struct Point Entrance1Point; /* park entrance tile */

// GLOBAL: LEGOLAND 0x0066b468
int LastPathUpdateTime;

// GLOBAL: LEGOLAND 0x0066b46c
unsigned int PathUpdateNeeded;

// GLOBAL: LEGOLAND 0x0066b470
char VisitorNameBuffer[0x104];

// GLOBAL: LEGOLAND 0x0066b574
LEGO_EXPORT struct Bloke *FirstBloke;

// GLOBAL: LEGOLAND 0x0066b57c
struct Bloke *BlokePool;

// GLOBAL: LEGOLAND 0x0066b580
int DirPickCounts[9];

// GLOBAL: LEGOLAND 0x0066b5a4
struct SortNode *PrintListHead;

// GLOBAL: LEGOLAND 0x0066b5a8
unsigned int PrintListBytesUsed;

// GLOBAL: LEGOLAND 0x0066b5ac
unsigned int DAT_0066b5ac;

// GLOBAL: LEGOLAND 0x0066b5b0
DDSURFACEDESC DAT_0066b5b0;

// GLOBAL: LEGOLAND 0x0066b61c
unsigned int DAT_0066b61c;

// GLOBAL: LEGOLAND 0x0066b620
int DAT_0066b620;

// GLOBAL: LEGOLAND 0x0066b624
int DAT_0066b624;

// GLOBAL: LEGOLAND 0x0066b628
int DAT_0066b628;

// GLOBAL: LEGOLAND 0x0066b62c
int DAT_0066b62c;

// GLOBAL: LEGOLAND 0x0066b630
void *DAT_0066b630;

// GLOBAL: LEGOLAND 0x0066be40
unsigned int DAT_0066be40;

// GLOBAL: LEGOLAND 0x0066be44
unsigned int DAT_0066be44;

// GLOBAL: LEGOLAND 0x0066be48
unsigned int DAT_0066be48;

// GLOBAL: LEGOLAND 0x0066be4c
unsigned int DAT_0066be4c;

// GLOBAL: LEGOLAND 0x0066be50
struct Image *DAT_0066be50;

// GLOBAL: LEGOLAND 0x0066be54
unsigned short DAT_0066be54[0x4b002];

// GLOBAL: LEGOLAND 0x00701e58
unsigned int DAT_00701e58;

// GLOBAL: LEGOLAND 0x00701e5c
unsigned char *DAT_00701e5c;

// GLOBAL: LEGOLAND 0x00701e60
unsigned int DAT_00701e60;

// GLOBAL: LEGOLAND 0x00701e64
struct Sprite *DAT_00701e64;

// GLOBAL: LEGOLAND 0x00701e68
unsigned short DAT_00701e68[0x4b000];

// GLOBAL: LEGOLAND 0x00797e68
unsigned int DAT_00797e68;

// GLOBAL: LEGOLAND 0x00797e6c
void *DAT_00797e6c;

// GLOBAL: LEGOLAND 0x00797e70
float DAT_00797e70[100];

// GLOBAL: LEGOLAND 0x00798000
int DAT_00798000[100];

// GLOBAL: LEGOLAND 0x00798190
void *DAT_00798190[256];

// GLOBAL: LEGOLAND 0x00798590
void *DAT_00798590;

// GLOBAL: LEGOLAND 0x00798598
DDSURFACEDESC DAT_00798598;

// GLOBAL: LEGOLAND 0x00798608
RECT DAT_00798608;

// GLOBAL: LEGOLAND 0x0079861c
LPDIRECTDRAWSURFACE DAT_0079861c;

// GLOBAL: LEGOLAND 0x00798620
int DAT_00798620;

// GLOBAL: LEGOLAND 0x00798624
void *MasterDirList;

// GLOBAL: LEGOLAND 0x00798628
void *MasterVolList;

// GLOBAL: LEGOLAND 0x0079862c
void *DAT_0079862c;

// GLOBAL: LEGOLAND 0x00798630
unsigned int SavedClipLeft;

// GLOBAL: LEGOLAND 0x00798634
unsigned int SavedClipTop;

// GLOBAL: LEGOLAND 0x00798638
unsigned int SavedClipRight;

// GLOBAL: LEGOLAND 0x0079863c
unsigned int SavedClipBottom;

// GLOBAL: LEGOLAND 0x00798648
unsigned int DAT_00798648;

// GLOBAL: LEGOLAND 0x0079864c
struct IconNode *DAT_0079864c;

// GLOBAL: LEGOLAND 0x00798650
unsigned int DAT_00798650;

// GLOBAL: LEGOLAND 0x00798660
unsigned int DAT_00798660;

// GLOBAL: LEGOLAND 0x00798664
unsigned int DAT_00798664;

// GLOBAL: LEGOLAND 0x0079866c
unsigned long DAT_0079866c;

// GLOBAL: LEGOLAND 0x00798668
unsigned int DAT_00798668;

// GLOBAL: LEGOLAND 0x00798674
struct Sprite *PuOkOnSprite;

// GLOBAL: LEGOLAND 0x00798678
struct Sprite *PuOkSprite;

// GLOBAL: LEGOLAND 0x0079867c
struct Sprite *PopUpCloseSprite;

// GLOBAL: LEGOLAND 0x00798680
struct Sprite *PuCloseOnSprite;

// GLOBAL: LEGOLAND 0x00798684
struct Sprite *ClosePopUpSprite;

// GLOBAL: LEGOLAND 0x00798688
struct Sprite *ClosePopUpOnSprite;

// GLOBAL: LEGOLAND 0x0079868c
struct Sprite *RegDeleteOnSprite;

// GLOBAL: LEGOLAND 0x00798690
struct Sprite *RegDeleteSprite;

// GLOBAL: LEGOLAND 0x00798694
struct Sprite *RegProfileOff1Sprite;

// GLOBAL: LEGOLAND 0x00798698
struct Sprite *RegProfileOff2Sprite;

// GLOBAL: LEGOLAND 0x0079869c
struct Sprite *RegProfileOff3Sprite;

// GLOBAL: LEGOLAND 0x007986a0
struct Sprite *RegProfileOff4Sprite;

// GLOBAL: LEGOLAND 0x007986a4
struct Sprite *RegProfileOff5Sprite;

// GLOBAL: LEGOLAND 0x007986a8
struct Sprite *RegProfileOff6Sprite;

// GLOBAL: LEGOLAND 0x007986ac
struct Sprite *RegProfileOff7Sprite;

// GLOBAL: LEGOLAND 0x007986b0
struct Sprite *RegProfileOff8Sprite;

// GLOBAL: LEGOLAND 0x007986b4
struct Sprite *RegProfileOnSprite;

// GLOBAL: LEGOLAND 0x007986b8
struct Sprite *DAT_007986b8;

// GLOBAL: LEGOLAND 0x007986bc
struct Sprite *RegDiffPopUpSprite;

// GLOBAL: LEGOLAND 0x007986c0
struct Sprite *RegEasyOnSprite;

// GLOBAL: LEGOLAND 0x007986c4
struct Sprite *RegEasyOffSprite;

// GLOBAL: LEGOLAND 0x007986c8
struct Sprite *RegMidOnSprite;

// GLOBAL: LEGOLAND 0x007986cc
struct Sprite *RegMidOffSprite;

// GLOBAL: LEGOLAND 0x007986d0
struct Sprite *RegHardOnSprite;

// GLOBAL: LEGOLAND 0x007986d4
struct Sprite *RegHardOffSprite;

// GLOBAL: LEGOLAND 0x007986d8
struct IconNode *PopUpOkIcon;

// GLOBAL: LEGOLAND 0x007986dc
struct IconNode *PopUpCloseIcon;

// GLOBAL: LEGOLAND 0x007986e0
unsigned int AcceptIcon;

// GLOBAL: LEGOLAND 0x007986e4
unsigned int DeletePopUpShown;

// GLOBAL: LEGOLAND 0x007986e8
unsigned int NewProfilePopUpShown;

// GLOBAL: LEGOLAND 0x007986f0
unsigned int DAT_007986f0;

// GLOBAL: LEGOLAND 0x007986f4
unsigned int DAT_007986f4;

// GLOBAL: LEGOLAND 0x007986f8
unsigned int DAT_007986f8;

// GLOBAL: LEGOLAND 0x00798700
unsigned int DAT_00798700;

// GLOBAL: LEGOLAND 0x00798704
struct Sprite *RegSaveSlotOnSprite;

// GLOBAL: LEGOLAND 0x00798708
struct Sprite *RegSaveOff1Sprite;

// GLOBAL: LEGOLAND 0x0079870c
struct Sprite *RegSaveOff2Sprite;

// GLOBAL: LEGOLAND 0x00798710
struct Sprite *RegSaveOff3Sprite;

// GLOBAL: LEGOLAND 0x00798714
struct Sprite *RegSaveOff4Sprite;

// GLOBAL: LEGOLAND 0x00798718
struct Sprite *RegSaveOff5Sprite;

// GLOBAL: LEGOLAND 0x0079871c
struct Sprite *RegSaveOff6Sprite;

// GLOBAL: LEGOLAND 0x00798720
struct Sprite *RegSaveOff7Sprite;

// GLOBAL: LEGOLAND 0x00798724
struct Sprite *RegSaveOff8Sprite;

// GLOBAL: LEGOLAND 0x00798728
struct Sprite *SaveTypeNormalSprite;

// GLOBAL: LEGOLAND 0x0079872c
struct Sprite *SaveTypeFreeSprite;

// GLOBAL: LEGOLAND 0x00798730
struct Sprite *RegCornerMaskSprite;

// GLOBAL: LEGOLAND 0x00798734
void *SavedGameList;

// GLOBAL: LEGOLAND 0x00798738
int DAT_00798738;

// GLOBAL: LEGOLAND 0x0079873c
unsigned int DAT_0079873c;

// GLOBAL: LEGOLAND 0x00798740
unsigned int DAT_00798740;

// GLOBAL: LEGOLAND 0x00798748
struct IconNode *SpeechVolumeMarkerIcon;

// GLOBAL: LEGOLAND 0x0079874c
struct IconNode *FxVolumeMarkerIcon;

// GLOBAL: LEGOLAND 0x00798750
struct IconNode *MusicVolumeMarkerIcon;

// GLOBAL: LEGOLAND 0x00798754
unsigned int DAT_00798754;

// GLOBAL: LEGOLAND 0x00798764
struct Sprite *PrintInfoSprite;

// GLOBAL: LEGOLAND 0x00798768
int CertificatePrintResultTimer;

// GLOBAL: LEGOLAND 0x0079876c
unsigned int DAT_0079876c;

// GLOBAL: LEGOLAND 0x00798770
unsigned int DAT_00798770;

// GLOBAL: LEGOLAND 0x00798777
char DAT_00798777;

// GLOBAL: LEGOLAND 0x00798778
char DAT_00798778[0x100];

// GLOBAL: LEGOLAND 0x00798878
unsigned int DAT_00798878;

// GLOBAL: LEGOLAND 0x0079887c
unsigned int DAT_0079887c;

// GLOBAL: LEGOLAND 0x00798880
unsigned int DAT_00798880;

// GLOBAL: LEGOLAND 0x00798884
unsigned int DAT_00798884;

// GLOBAL: LEGOLAND 0x00798888
unsigned int DAT_00798888;

// GLOBAL: LEGOLAND 0x00798890
void *ProfileListHead;

// GLOBAL: LEGOLAND 0x00798894
int DAT_00798894;

// GLOBAL: LEGOLAND 0x00798898
unsigned int AviLockBytes1;

// GLOBAL: LEGOLAND 0x0079889c
unsigned int AviLockBytes2;

// GLOBAL: LEGOLAND 0x007988a0
unsigned int DAT_007988a0;

// GLOBAL: LEGOLAND 0x007988a4
void *AviLockPtr1;

// GLOBAL: LEGOLAND 0x007988a8
void *AviLockPtr2;

// GLOBAL: LEGOLAND 0x007988b0
void *SoundHwnd;

// GLOBAL: LEGOLAND 0x007988bc
unsigned int DAT_007988bc;

// GLOBAL: LEGOLAND 0x007988c0
unsigned int SoundAvailable;

// GLOBAL: LEGOLAND 0x007988c4
unsigned int SfxMuted;

// GLOBAL: LEGOLAND 0x007988c8
unsigned int DAT_007988c8;

// GLOBAL: LEGOLAND 0x007988cc
void *SampleListHead;

// GLOBAL: LEGOLAND 0x0079a694
LEGO_EXPORT unsigned int DMusicInitialised;

// GLOBAL: LEGOLAND 0x0079a698
void *MusicThread;

// GLOBAL: LEGOLAND 0x0079a6a0
void *MusicCommandEvent;

// GLOBAL: LEGOLAND 0x0079a6a4
unsigned int MusicCommand;

// GLOBAL: LEGOLAND 0x0079a6a8
int NextMusicTheme;

// GLOBAL: LEGOLAND 0x0079a6ac
int CurrentMusicTheme;

// GLOBAL: LEGOLAND 0x0079a7d0
unsigned int SpeechVolume;

// GLOBAL: LEGOLAND 0x0079a7d8
int SpeechRawReadPos;

// GLOBAL: LEGOLAND 0x0079a7dc
int SpeechRawWritePos;

// GLOBAL: LEGOLAND 0x0079a7e0
unsigned int SpeechChunkIndex;

// GLOBAL: LEGOLAND 0x0079a7e4
unsigned int SpeechChunkByteCounts[20];

// GLOBAL: LEGOLAND 0x0079a834
int SpeechPcmReadPos;

// GLOBAL: LEGOLAND 0x0079a838
int SpeechPcmWritePos;

// GLOBAL: LEGOLAND 0x0079a83c
unsigned int SpeechFlags;

// GLOBAL: LEGOLAND 0x0079a840
int SpeechPageIndex;

// GLOBAL: LEGOLAND 0x0079a844
int DAT_0079a844;

// GLOBAL: LEGOLAND 0x0079a848
void *SpeechSoundBuffer;

// GLOBAL: LEGOLAND 0x0079a84c
unsigned int SpeechState;

// GLOBAL: LEGOLAND 0x0079a850
void *strings[10];

// GLOBAL: LEGOLAND 0x0079a878
unsigned int DAT_0079a878[6];

// GLOBAL: LEGOLAND 0x0079a890
unsigned int GameTimerPaused;

// GLOBAL: LEGOLAND 0x0079a894
unsigned int GameTimerPausedTicks;

// GLOBAL: LEGOLAND 0x0079a898
unsigned int DAT_0079a898;

// GLOBAL: LEGOLAND 0x0079a89c
unsigned int GameTimerPausedFrame;

// GLOBAL: LEGOLAND 0x0079a8a0
unsigned int DAT_0079a8a0;

// GLOBAL: LEGOLAND 0x0079a8a8
LEGO_EXPORT struct Bloke *GardenerList;

// GLOBAL: LEGOLAND 0x0079a8ac
LEGO_EXPORT struct Bloke *MechanicList;

// GLOBAL: LEGOLAND 0x0079a8b0
struct WorkOrder *GardenerOrderHead;

// GLOBAL: LEGOLAND 0x0079a8b4
struct WorkOrder *GardenerOrderTail;

// GLOBAL: LEGOLAND 0x0079a8b8
int DAT_0079a8b8;

// GLOBAL: LEGOLAND 0x0079a8bc
int GardenerCount;

// GLOBAL: LEGOLAND 0x0079a8c0
struct WorkOrder *MechanicOrderHead;

// GLOBAL: LEGOLAND 0x0079a8c4
struct WorkOrder *MechanicOrderTail;

// GLOBAL: LEGOLAND 0x0079a8c8
int DAT_0079a8c8;

// GLOBAL: LEGOLAND 0x0079a8cc
int MechanicCount;

// GLOBAL: LEGOLAND 0x0079a8d0
unsigned int CastlePlacedFlag;

// GLOBAL: LEGOLAND 0x0079a8d4
struct RepairOrder *RepairOrderList;

// GLOBAL: LEGOLAND 0x0079abfc
struct Element *NormalPathTilesElement;

// GLOBAL: LEGOLAND 0x0079ac04
unsigned int SpeechDataSize;

// GLOBAL: LEGOLAND 0x0079ac08
unsigned char *SpeechAcmDstBuffer;

// GLOBAL: LEGOLAND 0x0079ac0c
void *SpeechAcmSrcBuffer;

// GLOBAL: LEGOLAND 0x0079ac20
unsigned char SpeechRawBuffer[0x10000];

// GLOBAL: LEGOLAND 0x007aac24
int SpeechConvertedUsed;

// GLOBAL: LEGOLAND 0x007aac40
struct AcmHdr SpeechAcmHeader;

// GLOBAL: LEGOLAND 0x007aaca0
unsigned char SpeechPcmBuffer[0x20000];

// GLOBAL: LEGOLAND 0x007caca0
int SpeechAcmSrcSize;

// GLOBAL: LEGOLAND 0x007caca4
int SpeechConvertedSize;

// GLOBAL: LEGOLAND 0x007caca8
unsigned int SpeechFileHandle;

// GLOBAL: LEGOLAND 0x007cacac
unsigned int SpeechBytesRemaining;

// GLOBAL: LEGOLAND 0x007cacb0
WAVEFORMATEX *SpeechSourceFormat;

// GLOBAL: LEGOLAND 0x007cacb8
void *SpeechAcmStream;

// GLOBAL: LEGOLAND 0x007cacc0
WAVEFORMATEX SpeechPcmFormat;

// GLOBAL: LEGOLAND 0x007cacb4
unsigned int SpeechDataOffset;

// GLOBAL: LEGOLAND 0x007cacd4
LEGO_EXPORT unsigned int FrameNumber;

// GLOBAL: LEGOLAND 0x007cacd8
struct DirectMusicLoader *DMusicLoader;

// GLOBAL: LEGOLAND 0x007cacdc
struct DirectMusicPerformance *DMusicPerformance;

// GLOBAL: LEGOLAND 0x007cace0
unsigned int DSoundCaps[0x18];

// GLOBAL: LEGOLAND 0x007cad40
void *DSound;

// GLOBAL: LEGOLAND 0x007cad44
struct DirectMusicComposer *DMusicComposer;

// GLOBAL: LEGOLAND 0x007cad48
unsigned int MusicThreadId;

// GLOBAL: LEGOLAND 0x007cad4c
void *DMusicSoundBuffer;

// GLOBAL: LEGOLAND 0x007cad60
struct ProfileData TempProfile;

// GLOBAL: LEGOLAND 0x007cae80
char DAT_007cae80[0x100];

// GLOBAL: LEGOLAND 0x007caf80
struct Sprite *RepHint1Sprite;

// GLOBAL: LEGOLAND 0x007cafa0
char *DAT_007cafa0[104]; /* text lines (ReadResourceLines, up to 100) */

// GLOBAL: LEGOLAND 0x007cb140
char *DAT_007cb140[32]; /* text lines (ReadResourceLines, up to 32) */

// GLOBAL: LEGOLAND 0x007cb1c0
struct IconNode *RepHint1Icon;

// GLOBAL: LEGOLAND 0x007cb1c4
struct Sprite *RepHint2Sprite;

// GLOBAL: LEGOLAND 0x007cb1e0
char DAT_007cb1e0[0x100];

// GLOBAL: LEGOLAND 0x007cb2e0
struct IconNode *DAT_007cb2e0;

// GLOBAL: LEGOLAND 0x007cb2e4
struct IconNode *DAT_007cb2e4;

// GLOBAL: LEGOLAND 0x007cb2f0
char DAT_007cb2f0[0x10];

// GLOBAL: LEGOLAND 0x007cb300
char DAT_007cb300[0xc];

// GLOBAL: LEGOLAND 0x007cb30c
char DAT_007cb30c[4];

// GLOBAL: LEGOLAND 0x007cb310
unsigned int DAT_007cb310;

// GLOBAL: LEGOLAND 0x007cb314
char DAT_007cb314;

// GLOBAL: LEGOLAND 0x007cb315
char DAT_007cb315;

// GLOBAL: LEGOLAND 0x007cb318
unsigned int DAT_007cb318;

// GLOBAL: LEGOLAND 0x007cb31c
char DAT_007cb31c;

// GLOBAL: LEGOLAND 0x007cb320
unsigned int DAT_007cb320;

// GLOBAL: LEGOLAND 0x007cb324
unsigned int DAT_007cb324;

// GLOBAL: LEGOLAND 0x007cb328
unsigned int LoadMode;

// GLOBAL: LEGOLAND 0x007cb340
char DAT_007cb340[0x20];

// GLOBAL: LEGOLAND 0x007cb360
struct IconNode *DeleteIcon;

// GLOBAL: LEGOLAND 0x007cb380
unsigned int DAT_007cb380[5];

// GLOBAL: LEGOLAND 0x007cb394
unsigned int DAT_007cb394;

// GLOBAL: LEGOLAND 0x007cb398
struct Sprite *FreePlayTickSprite;

// GLOBAL: LEGOLAND 0x007cb39c
struct PanelNode *DAT_007cb39c;

// GLOBAL: LEGOLAND 0x007cb3a0
unsigned int DAT_007cb3a0;

// GLOBAL: LEGOLAND 0x007cb3a4
struct PanelNode *DAT_007cb3a4;

// GLOBAL: LEGOLAND 0x007cb3a8
struct Sprite *FreePlayDown4Sprite;

// GLOBAL: LEGOLAND 0x007cb3ac
struct Sprite *FreePlayDown2Sprite;

// GLOBAL: LEGOLAND 0x007cb3b0
struct Sprite *FreePlayDown3Sprite;

// GLOBAL: LEGOLAND 0x007cb3b4
struct Sprite *FreePlayDown1Sprite;

// GLOBAL: LEGOLAND 0x007cb3b8
struct PanelNode *DAT_007cb3b8;

// GLOBAL: LEGOLAND 0x007cb3bc
struct Element *DAT_007cb3bc;

// GLOBAL: LEGOLAND 0x007cb3c0
struct Sprite *FreePlayUp2Sprite;

// GLOBAL: LEGOLAND 0x007cb3c4
struct Sprite *FreePlayUp1Sprite;

// GLOBAL: LEGOLAND 0x007cb3c8
struct Sprite *FreePlayUp4Sprite;

// GLOBAL: LEGOLAND 0x007cb3cc
struct Sprite *FreePlayUp3Sprite;

// GLOBAL: LEGOLAND 0x007cb3d0
struct PanelNode *DAT_007cb3d0;

// GLOBAL: LEGOLAND 0x007cb3d4
struct Sprite *FreePlayCoverSprite;

// GLOBAL: LEGOLAND 0x007cb3e0
struct ObjTableEntry ObjInstanceTable[128];

// GLOBAL: LEGOLAND 0x007cb5e0
struct ObjTableEntry DAT_007cb5e0;

// GLOBAL: LEGOLAND 0x007cb600
unsigned char PrintListPool[0x32000];

// GLOBAL: LEGOLAND 0x007fd600
struct SortNode *SortCursor;

// GLOBAL: LEGOLAND 0x007fd610
struct LibraryNode DAT_007fd610;

// GLOBAL: LEGOLAND 0x007fd620
LEGO_EXPORT void *NewObjectPtr;

// GLOBAL: LEGOLAND 0x007fd624
void *PathControlObject;

// GLOBAL: LEGOLAND 0x007fd630
unsigned int MidiTimerId;

// GLOBAL: LEGOLAND 0x007fd634
void *CurrentMidiFile;

// GLOBAL: LEGOLAND 0x007fd638
unsigned int MidiOutHandle;

// GLOBAL: LEGOLAND 0x007fd640
struct ResVolume *ResourceVolumes[4];

// GLOBAL: LEGOLAND 0x007fd660
unsigned int DAT_007fd660[256];

// GLOBAL: LEGOLAND 0x007fda60
struct BlokeSave BlokeSaveBuffer;

// GLOBAL: LEGOLAND 0x007fdb84
int DAT_007fdb84;

// GLOBAL: LEGOLAND 0x007fdb88
unsigned int SelectElementMask;

// GLOBAL: LEGOLAND 0x007fdba0
char DAT_007fdba0[0x100];

// GLOBAL: LEGOLAND 0x007fdca0
int SelectedElementIndex;

// GLOBAL: LEGOLAND 0x007fdca4
unsigned int DAT_007fdca4;

// GLOBAL: LEGOLAND 0x007fdca8
float PerfCounterScale;

// GLOBAL: LEGOLAND 0x007fdcc0
struct Sprite *LegolandThemeOnSprite;

// GLOBAL: LEGOLAND 0x007fdcc4
struct Sprite *WesternThemeOnSprite;

// GLOBAL: LEGOLAND 0x007fdcc8
struct Sprite *CastleThemeOnSprite;

// GLOBAL: LEGOLAND 0x007fdccc
struct Sprite *AdventurersThemeOnSprite;

// GLOBAL: LEGOLAND 0x007fdcd0
struct Sprite *IfPathIconPressedSprite;

// GLOBAL: LEGOLAND 0x007fdcd4
struct Sprite *IfQueryIconPressedSprite;

// GLOBAL: LEGOLAND 0x007fdcd8
struct Sprite *IfEraserIconPressedSprite;

// GLOBAL: LEGOLAND 0x007fdcdc
struct Sprite *IfMapIconPressedSprite;

// GLOBAL: LEGOLAND 0x007fdce0
struct Sprite *IfOptionsIconPressedSprite;

// GLOBAL: LEGOLAND 0x007fdd00
unsigned int ButtonFlashStates[9];

// GLOBAL: LEGOLAND 0x007fdd40
struct Sprite *LegolandThemeOffSprite;

// GLOBAL: LEGOLAND 0x007fdd44
struct Sprite *WesternThemeOffSprite;

// GLOBAL: LEGOLAND 0x007fdd48
struct Sprite *CastleThemeOffSprite;

// GLOBAL: LEGOLAND 0x007fdd4c
struct Sprite *AdventurersThemeOffSprite;

// GLOBAL: LEGOLAND 0x007fdd50
struct Sprite *IfPathIconSprite;

// GLOBAL: LEGOLAND 0x007fdd54
struct Sprite *IfQueryiconSprite;

// GLOBAL: LEGOLAND 0x007fdd58
struct Sprite *IfEraserIconSprite;

// GLOBAL: LEGOLAND 0x007fdd5c
struct Sprite *IfMapiconSprite;

// GLOBAL: LEGOLAND 0x007fdd60
struct Sprite *IfOptionsIconSprite;

// GLOBAL: LEGOLAND 0x007fdd70
struct InterfaceProfileObj *DAT_007fdd70[4];

// GLOBAL: LEGOLAND 0x007fdd80
unsigned char DAT_007fdd80;

// GLOBAL: LEGOLAND 0x007fdd84
unsigned int DAT_007fdd84;

// GLOBAL: LEGOLAND 0x007fdd88
unsigned int DAT_007fdd88;

// GLOBAL: LEGOLAND 0x007fdd8c
unsigned char DAT_007fdd8c;

// GLOBAL: LEGOLAND 0x007fdda0
unsigned char KeyboardState[256];

// GLOBAL: LEGOLAND 0x007fdea4
struct IconNode *AddMechanicsIcon;

// GLOBAL: LEGOLAND 0x007fdea8
struct IconNode *PopUpInfoOkIcon;

// GLOBAL: LEGOLAND 0x007fdeac
struct Sprite *IFullSprite;

// GLOBAL: LEGOLAND 0x007fdeb0
struct Sprite *ObjectNoRepair1Sprite;

// GLOBAL: LEGOLAND 0x007fdec0
struct HoverInfo DAT_007fdec0;

// GLOBAL: LEGOLAND 0x007fdecc
int PopUpInfoX;

// GLOBAL: LEGOLAND 0x007fded0
int PopUpInfoY;

// GLOBAL: LEGOLAND 0x007fded4
struct NewObjTable NewObjects;

// GLOBAL: LEGOLAND 0x007fdf7c
unsigned int DAT_007fdf7c;

// GLOBAL: LEGOLAND 0x007fdf80
struct InfoObjData *DAT_007fdf80;

// GLOBAL: LEGOLAND 0x007fdf84
unsigned char *DAT_007fdf84;

// GLOBAL: LEGOLAND 0x007fdf88
unsigned short DAT_007fdf88;

// GLOBAL: LEGOLAND 0x007fdf8c
void *DAT_007fdf8c;

// GLOBAL: LEGOLAND 0x007fdf90
unsigned int DAT_007fdf90;

// GLOBAL: LEGOLAND 0x007fdf94
unsigned int DAT_007fdf94;

// GLOBAL: LEGOLAND 0x007fdf98
unsigned int DAT_007fdf98;

// GLOBAL: LEGOLAND 0x007fdf9c
int DAT_007fdf9c;

// GLOBAL: LEGOLAND 0x007fdfa0
unsigned int DAT_007fdfa0;

// GLOBAL: LEGOLAND 0x007fdfa4
unsigned int DAT_007fdfa4;

// GLOBAL: LEGOLAND 0x007fdfa8
unsigned int DAT_007fdfa8;

// GLOBAL: LEGOLAND 0x007fdfac
unsigned char PopupInfoLineCount;

// GLOBAL: LEGOLAND 0x007fdfb0
unsigned int PottingShedHandle;

// GLOBAL: LEGOLAND 0x007fdfb4
unsigned int MechanicsHutHandle;

// GLOBAL: LEGOLAND 0x007fdfb8
unsigned int PathControlHandle;

// GLOBAL: LEGOLAND 0x007fdfbc
unsigned int Entrance1Handle;

// GLOBAL: LEGOLAND 0x007fdfc0
struct IconNode *ClosePopUpIcon;

// GLOBAL: LEGOLAND 0x007fdfc4
struct IconNode *NextPopUpIcon;

// GLOBAL: LEGOLAND 0x007fdfc8
struct Sprite *ISadSprite;

// GLOBAL: LEGOLAND 0x007fdfcc
struct IconNode *DAT_007fdfcc;

// GLOBAL: LEGOLAND 0x007fdfd0
struct Sprite *IHungrySprite;

// GLOBAL: LEGOLAND 0x007fdfd8
struct IconNode *PuCornerMaskIcon;

// GLOBAL: LEGOLAND 0x007fdfdc
struct IconNode *DeleteObjectIcon;

// GLOBAL: LEGOLAND 0x007fdfe0
struct IconNode *AddGardenerIcon;

// GLOBAL: LEGOLAND 0x007fdfe4
struct Sprite *INormSprite;

// GLOBAL: LEGOLAND 0x007fdfe8
struct IconNode *PrevPopUpIcon;

// GLOBAL: LEGOLAND 0x007fdff0
struct Bloke *WorkerOnMouse;

// GLOBAL: LEGOLAND 0x007fdff4
int WorkerOldX;

// GLOBAL: LEGOLAND 0x007fdff8
int WorkerOldY;

// GLOBAL: LEGOLAND 0x007fdffc
unsigned int WorkerOnMouseType;

// GLOBAL: LEGOLAND 0x007fe000
struct IconNode *CBCloseIcon;

// GLOBAL: LEGOLAND 0x007fe004
struct Sprite *ObjectRepairOkSprite;

// GLOBAL: LEGOLAND 0x007fe008
struct Sprite *IPeckishSprite;

// GLOBAL: LEGOLAND 0x007fe010
int DAT_007fe010;

// GLOBAL: LEGOLAND 0x007fe014
int DAT_007fe014;

// GLOBAL: LEGOLAND 0x007fe018
struct Sprite *IHappySprite;

// GLOBAL: LEGOLAND 0x007fe020
RECT ScreenRect;

// GLOBAL: LEGOLAND 0x007fe040
unsigned int DAT_007fe040;

// GLOBAL: LEGOLAND 0x007fe044
unsigned int DAT_007fe044;

// GLOBAL: LEGOLAND 0x007fe048
char *AdvisorHelpText;

// GLOBAL: LEGOLAND 0x007fe04c
unsigned int AdvisorHelpStartTime;

// GLOBAL: LEGOLAND 0x007fe050
unsigned int DAT_007fe050;

// GLOBAL: LEGOLAND 0x007fe054
unsigned int DAT_007fe054;

// GLOBAL: LEGOLAND 0x007fe114
unsigned char LegolandCommonThemeCount;

// GLOBAL: LEGOLAND 0x007fe115
unsigned char WesternThemeCount;

// GLOBAL: LEGOLAND 0x007fe116
unsigned char CastleThemeCount;

// GLOBAL: LEGOLAND 0x007fe117
unsigned char AdventurersThemeCount;

// GLOBAL: LEGOLAND 0x007fe120
unsigned int ScriptStringTable[512];

// GLOBAL: LEGOLAND 0x007fe920
unsigned int DAT_007fe920;

// GLOBAL: LEGOLAND 0x007fe930
unsigned char ObjectiveCounters[10];

// GLOBAL: LEGOLAND 0x007fe994
unsigned int DAT_007fe994;

// GLOBAL: LEGOLAND 0x007fe998
unsigned int DAT_007fe998;

// GLOBAL: LEGOLAND 0x007fe9a8
unsigned int DAT_007fe9a8;

// GLOBAL: LEGOLAND 0x007fe9c0
struct Sprite *PointerSprites[9];

// GLOBAL: LEGOLAND 0x007fea30
RECT WatchRect;

// GLOBAL: LEGOLAND 0x007fea44
unsigned int StoredTransparentColour;

// GLOBAL: LEGOLAND 0x007fea48
LEGO_EXPORT unsigned int FramesPerSecond;

// GLOBAL: LEGOLAND 0x007feb14
unsigned int DAT_007feb14;

// GLOBAL: LEGOLAND 0x007febb8
unsigned short DAT_007febb8;

// GLOBAL: LEGOLAND 0x007febc0
LEGO_EXPORT struct Cursor EditCursor;

// GLOBAL: LEGOLAND 0x008003f4
int LevelMapHandle;

// GLOBAL: LEGOLAND 0x008003f8
unsigned int DAT_008003f8;

// GLOBAL: LEGOLAND 0x00800400
LEGO_EXPORT RECT ObjectPartArray[256];

// GLOBAL: LEGOLAND 0x00801400
LEGO_EXPORT struct MapElement **GameMap;

// GLOBAL: LEGOLAND 0x00801404
void *BridgesHandle;

// GLOBAL: LEGOLAND 0x00801408
unsigned int DAT_00801408;

// GLOBAL: LEGOLAND 0x0080140c
void *LevelTileMapHandle;

// GLOBAL: LEGOLAND 0x00801410
void *OverlayILFHandle;

// GLOBAL: LEGOLAND 0x00801420
struct DeferredSprite DAT_00801420[100];

// GLOBAL: LEGOLAND 0x00801a60
int DAT_00801a60;

// GLOBAL: LEGOLAND 0x00801a64
int DAT_00801a64;

// GLOBAL: LEGOLAND 0x00801a68
void *DAT_00801a68;

// GLOBAL: LEGOLAND 0x00801a6c
int *DAT_00801a6c;

// GLOBAL: LEGOLAND 0x00801a70
void *DAT_00801a70;

// GLOBAL: LEGOLAND 0x00801a74
int DAT_00801a74;

// GLOBAL: LEGOLAND 0x00801a80
struct MapRect DAT_00801a80[10];

// GLOBAL: LEGOLAND 0x00801b20
unsigned int DAT_00801b20;

// GLOBAL: LEGOLAND 0x00801b24
LEGO_EXPORT unsigned int ObjectPartCount;

// GLOBAL: LEGOLAND 0x00801b28
int DAT_00801b28;

// GLOBAL: LEGOLAND 0x00801b40
int ObjectPartKey[256];

// GLOBAL: LEGOLAND 0x00801f40
LEGO_EXPORT struct TileSpriteEntry TileSpriteInfo[2048];

// GLOBAL: LEGOLAND 0x00805f40
int DAT_00805f40;

// GLOBAL: LEGOLAND 0x00805f44
int DAT_00805f44;

// GLOBAL: LEGOLAND 0x00805f48
unsigned int DAT_00805f48;

// GLOBAL: LEGOLAND 0x00805f60
LEGO_EXPORT struct Sprite *TileSpriteArray[2048];

// GLOBAL: LEGOLAND 0x00807f60
struct MapRenderOrderEntry MapRenderOrderList[4096];

// GLOBAL: LEGOLAND 0x0080ff60
unsigned int DAT_0080ff60;

// GLOBAL: LEGOLAND 0x0080ff64
struct Element *CastleObjElem;

// GLOBAL: LEGOLAND 0x0080ff68
unsigned int DAT_0080ff68;

// GLOBAL: LEGOLAND 0x0080ff6c
void *DAT_0080ff6c;

// GLOBAL: LEGOLAND 0x0080ff70
unsigned int DAT_0080ff70;

// GLOBAL: LEGOLAND 0x0080ff74
LEGO_EXPORT unsigned int NEWFLC_AutoPlay;

// GLOBAL: LEGOLAND 0x0080ff78
LEGO_EXPORT unsigned char NEWFLC_PauseType;

// GLOBAL: LEGOLAND 0x0080ff80
struct ScreenMode DAT_0080ff80;

// GLOBAL: LEGOLAND 0x0080ffa0
struct ScreenState CurrentProfile;

// GLOBAL: LEGOLAND 0x008100c0
char DAT_008100c0[0x80];

// GLOBAL: LEGOLAND 0x00810140
unsigned int DAT_00810140;

// GLOBAL: LEGOLAND 0x00810144
unsigned int DAT_00810144;

// GLOBAL: LEGOLAND 0x00810148
struct Sprite *SPRITE_TitleScreenBk;

// GLOBAL: LEGOLAND 0x0081014c
LEGO_EXPORT unsigned char NEWFLC_ID[20];

// GLOBAL: LEGOLAND 0x00810160
LEGO_EXPORT struct Cursor QueryCursor;

// GLOBAL: LEGOLAND 0x008119a0
LEGO_EXPORT unsigned int NEWFLC_CheckDuplicate;

// GLOBAL: LEGOLAND 0x008119a4
unsigned int FrameCounter;

// GLOBAL: LEGOLAND 0x008119a8
LEGO_EXPORT unsigned int NEWFLC_BuffSize;

// GLOBAL: LEGOLAND 0x008119ac
LEGO_EXPORT unsigned short NEWFLC_Repeat;

// GLOBAL: LEGOLAND 0x008119b0
LEGO_EXPORT struct EditState EditMode;

// GLOBAL: LEGOLAND 0x008119bc
unsigned int DAT_008119bc;

// GLOBAL: LEGOLAND 0x008119c0
struct MapMarker DAT_008119c0[31][32];

// GLOBAL: LEGOLAND 0x008138c0
struct MapMarker DAT_008138c0[32];

// GLOBAL: LEGOLAND 0x008139c0
int MapViewHeight;

// GLOBAL: LEGOLAND 0x008139c4
int MapViewWidth;

// GLOBAL: LEGOLAND 0x008139c8
int MapViewX;

// GLOBAL: LEGOLAND 0x008139cc
int MapViewY;

// GLOBAL: LEGOLAND 0x008139e0
struct Sprite *DAT_008139e0;

// GLOBAL: LEGOLAND 0x008139e4
struct Sprite *MiHungrySprite;

// GLOBAL: LEGOLAND 0x008139e8
struct Sprite *MiHappySprite;

// GLOBAL: LEGOLAND 0x008139ec
struct Sprite *MiSadSprite;

// GLOBAL: LEGOLAND 0x008139f0
struct Sprite *MiHomeSprite;

// GLOBAL: LEGOLAND 0x008139f4
struct Sprite *MiEatSprite;

// GLOBAL: LEGOLAND 0x008139f8
struct Sprite *GreatSprite;

// GLOBAL: LEGOLAND 0x008139fc
struct Sprite *PoorSprite;

// GLOBAL: LEGOLAND 0x00813a00
struct Sprite *FavouriteSprite;

// GLOBAL: LEGOLAND 0x00813a04
struct Sprite *OpinionSprite;

// GLOBAL: LEGOLAND 0x00813a08
struct Sprite *MiBoredSprite;

// GLOBAL: LEGOLAND 0x00813a0c
void *SpeechBubbleData;

// GLOBAL: LEGOLAND 0x00813a10
unsigned int DebugAllocatedBytes;

// GLOBAL: LEGOLAND 0x00813a18
unsigned long DAT_00813a18;

// GLOBAL: LEGOLAND 0x00813a2c
int DAT_00813a2c;

// GLOBAL: LEGOLAND 0x00813a34
unsigned short DAT_00813a34;

// GLOBAL: LEGOLAND 0x00813a38
unsigned int DAT_00813a38;

// GLOBAL: LEGOLAND 0x00813a3c
unsigned int DAT_00813a3c;

// GLOBAL: LEGOLAND 0x00813a40
LEGO_EXPORT unsigned int GamePad;

// GLOBAL: LEGOLAND 0x00813a44
struct Point MousePos;

// GLOBAL: LEGOLAND 0x00813a4c
unsigned int DAT_00813a4c;

// GLOBAL: LEGOLAND 0x00813a50
unsigned int DAT_00813a50;

// GLOBAL: LEGOLAND 0x00813a54
unsigned int DAT_00813a54;

// GLOBAL: LEGOLAND 0x00813a5c
unsigned int DAT_00813a5c;

// GLOBAL: LEGOLAND 0x00813a60
unsigned int DAT_00813a60;

// GLOBAL: LEGOLAND 0x00813a64
unsigned int MouseTileX;

// GLOBAL: LEGOLAND 0x00813a68
unsigned int MouseTileY;

// GLOBAL: LEGOLAND 0x00813a6c
unsigned int FootprintWidth;

// GLOBAL: LEGOLAND 0x00813a70
unsigned int FootprintHeight;

// GLOBAL: LEGOLAND 0x00813a74
unsigned int DAT_00813a74;

// GLOBAL: LEGOLAND 0x00813a78
unsigned int DAT_00813a78;

// GLOBAL: LEGOLAND 0x00813a7c
unsigned int DAT_00813a7c;

// GLOBAL: LEGOLAND 0x00813a80
unsigned int DAT_00813a80;

// GLOBAL: LEGOLAND 0x00813a84
unsigned int DAT_00813a84;

// GLOBAL: LEGOLAND 0x00813a88
unsigned int DAT_00813a88;

// GLOBAL: LEGOLAND 0x00813a8c
unsigned int DAT_00813a8c;

// GLOBAL: LEGOLAND 0x00813a90
unsigned int DAT_00813a90;

// GLOBAL: LEGOLAND 0x00813a94
unsigned int DAT_00813a94;

// GLOBAL: LEGOLAND 0x00813a98
unsigned int DAT_00813a98;

// GLOBAL: LEGOLAND 0x00813a9c
unsigned int DAT_00813a9c;

// GLOBAL: LEGOLAND 0x00813aa0
unsigned int DAT_00813aa0;

// GLOBAL: LEGOLAND 0x00813aa4
unsigned int DAT_00813aa4;

// GLOBAL: LEGOLAND 0x00813aa8
unsigned int DAT_00813aa8;

// GLOBAL: LEGOLAND 0x00813aac
unsigned int DAT_00813aac;

// GLOBAL: LEGOLAND 0x00813ab0
unsigned int DAT_00813ab0;

// GLOBAL: LEGOLAND 0x00813ab4
unsigned int DAT_00813ab4;

// GLOBAL: LEGOLAND 0x00813ab8
unsigned int DAT_00813ab8;

// GLOBAL: LEGOLAND 0x00813abc
unsigned int DAT_00813abc;

// GLOBAL: LEGOLAND 0x00813ac0
unsigned int DAT_00813ac0;

// GLOBAL: LEGOLAND 0x00813ac4
unsigned int DAT_00813ac4;

// GLOBAL: LEGOLAND 0x00813ac8
unsigned int DAT_00813ac8;

// GLOBAL: LEGOLAND 0x00813acc
unsigned int DAT_00813acc;

// GLOBAL: LEGOLAND 0x00813ad0
unsigned int DAT_00813ad0;

// GLOBAL: LEGOLAND 0x00813ad4
unsigned int DAT_00813ad4;

// GLOBAL: LEGOLAND 0x00813ad8
unsigned int DAT_00813ad8;

// GLOBAL: LEGOLAND 0x00813adc
unsigned int DAT_00813adc;

// GLOBAL: LEGOLAND 0x00813ae0
unsigned int DAT_00813ae0;

// GLOBAL: LEGOLAND 0x00813af0
int DAT_00813af0;

// GLOBAL: LEGOLAND 0x00813af4
int DAT_00813af4;

// GLOBAL: LEGOLAND 0x00813af8
int DAT_00813af8;

// GLOBAL: LEGOLAND 0x00813afc
int DAT_00813afc;

// GLOBAL: LEGOLAND 0x00813b00
LEGO_EXPORT struct CtrlBuffer *CONTROLLERBUFFER;

// GLOBAL: LEGOLAND 0x00813b04
char CdDrivePath[4];

// GLOBAL: LEGOLAND 0x00813b08
unsigned int DAT_00813b08;

// GLOBAL: LEGOLAND 0x00813b20
unsigned char DAT_00813b20[0x300];

// GLOBAL: LEGOLAND 0x00813e20
unsigned short DAT_00813e20[256];

// GLOBAL: LEGOLAND 0x00814020
unsigned char ColourLookupTable[0x8000];

// GLOBAL: LEGOLAND 0x0081c028
struct Sprite *AppBarSprite;

// GLOBAL: LEGOLAND 0x0081c02c
struct Sprite *NextPageSprite;

// GLOBAL: LEGOLAND 0x0081c030
struct Sprite *AppBarMarkerSprite;

// GLOBAL: LEGOLAND 0x0081c034
struct Sprite *NextPageLitSprite;

// GLOBAL: LEGOLAND 0x0081c038
unsigned int DAT_0081c038;

// GLOBAL: LEGOLAND 0x0081c040
struct Sprite *DAT_0081c040[5];

// GLOBAL: LEGOLAND 0x0081c054
struct Sprite *DAT_0081c054[5];

// GLOBAL: LEGOLAND 0x0081c068
struct Sprite *DAT_0081c068[5];

// GLOBAL: LEGOLAND 0x0081c07c
unsigned int DAT_0081c07c;

// GLOBAL: LEGOLAND 0x0081c080
struct Sprite *PreviousPageSprite;

// GLOBAL: LEGOLAND 0x0081c084
struct Sprite *PreviousPageLitSprite;

// GLOBAL: LEGOLAND 0x0081c088
unsigned int DAT_0081c088;

// GLOBAL: LEGOLAND 0x0081c08c
void *AdBlinkAnim;

// GLOBAL: LEGOLAND 0x0081c090
void *AdWobbleAnim;

// GLOBAL: LEGOLAND 0x0081c094
void *AdLRAnim;

// GLOBAL: LEGOLAND 0x0081c098
void *AdPhoneDownAnim;

// GLOBAL: LEGOLAND 0x0081c09c
unsigned int DAT_0081c09c;

// GLOBAL: LEGOLAND 0x0081c0a0
void *AdPhoneAnim;

// GLOBAL: LEGOLAND 0x0081c0a4
void *AdPhoneGestureAnim;

// GLOBAL: LEGOLAND 0x0081c0c0
int DAT_0081c0c0[0x200];

// GLOBAL: LEGOLAND 0x0081c4c0
unsigned int DAT_0081c4c0[0x100];

// GLOBAL: LEGOLAND 0x0081c8c0
void *VisitorLocData;

// GLOBAL: LEGOLAND 0x0081c8c4
void *TracyLocData;

// GLOBAL: LEGOLAND 0x0081c8c8
void *GeoffLocData;

// GLOBAL: LEGOLAND 0x0081c8cc
void *DAT_0081c8cc;

// GLOBAL: LEGOLAND 0x0081c8d0
unsigned int ViewportLeft;

// GLOBAL: LEGOLAND 0x0081c8d4
unsigned int ViewportTop;

// GLOBAL: LEGOLAND 0x0081c8d8
unsigned int ViewportRight;

// GLOBAL: LEGOLAND 0x0081c8dc
unsigned int ViewportBottom;

// GLOBAL: LEGOLAND 0x0081c8e0
char DAT_0081c8e0[512];

// GLOBAL: LEGOLAND 0x0081cae0
struct Sprite *ZoomerSprite;

// GLOBAL: LEGOLAND 0x0081cae8
int DAT_0081cae8;

// GLOBAL: LEGOLAND 0x0081caec
int DAT_0081caec;

// GLOBAL: LEGOLAND 0x0081caf0
struct Ride *PottingShedRide;

// GLOBAL: LEGOLAND 0x0081caf4
struct Ride *MechanicsHutRide;

// GLOBAL: LEGOLAND 0x0081cb00
struct Sprite *SaloonMatte1Sprite;

// GLOBAL: LEGOLAND 0x0081cb04
struct Sprite *SaloonMatte2Sprite;

// GLOBAL: LEGOLAND 0x0081cb08
struct Sprite *GStoreMatteSprite;

// GLOBAL: LEGOLAND 0x0081cb0c
struct Sprite *JailCellMaskSprite;

// GLOBAL: LEGOLAND 0x0081cb10
struct Building *DAT_0081cb10;

// GLOBAL: LEGOLAND 0x0081cb14
struct Building *SheriffBuilding;

// GLOBAL: LEGOLAND 0x0081cb18
struct Sprite *LegoShop1MatteSprite;

// GLOBAL: LEGOLAND 0x0081cb1c
struct Building *SaloonBuilding;

// GLOBAL: LEGOLAND 0x0081cb20
struct Sprite *LegoShop2MatteSprite;

// GLOBAL: LEGOLAND 0x0081cb24
struct Sprite *GStoreMatte2Sprite;

// GLOBAL: LEGOLAND 0x0081cb28
struct Sprite *ExplorersInstituteMatteSprite;

// GLOBAL: LEGOLAND 0x0081cb2c
struct Building *BankBuilding;

// GLOBAL: LEGOLAND 0x0081cb30
struct Building *GeneralStoreBuilding;

// GLOBAL: LEGOLAND 0x0081cb34
struct Sprite *BankMatteSprite;

// GLOBAL: LEGOLAND 0x0081cb38
struct Sprite *SherifshutMatteSprite;

// GLOBAL: LEGOLAND 0x0081cb3c
struct Building *LegoShop1Building;

// GLOBAL: LEGOLAND 0x0081cb40
struct Building *LegoMediaShopBuilding;

// GLOBAL: LEGOLAND 0x0081cb44
struct Building *ExplorersInstituteBuilding;

// GLOBAL: LEGOLAND 0x0081cb48
struct Sprite *LegMediaShopMask1Sprite;

// GLOBAL: LEGOLAND 0x0081cb4c
struct Building *LegoShop2Building;

// GLOBAL: LEGOLAND 0x0081cb50
struct Sprite *LegMediaShopMask2Sprite;

// GLOBAL: LEGOLAND 0x0081cb54
struct Ride *DAT_0081cb54;

// GLOBAL: LEGOLAND 0x0081cb58
struct TileMap *BoatingSchoolTileMap;

// GLOBAL: LEGOLAND 0x0081cb5c
struct Sprite *JungMaskSprite;

// GLOBAL: LEGOLAND 0x0081cb60
struct Ride *JungleCruiseRide;

// GLOBAL: LEGOLAND 0x0081cb64
struct Ride *DAT_0081cb64;

// GLOBAL: LEGOLAND 0x0081cb68
struct Sprite *BrijMaskSprite;

// GLOBAL: LEGOLAND 0x0081cb6c
struct Sprite *MFish2Sprite;

// GLOBAL: LEGOLAND 0x0081cb70
struct Ride *MonkeyTreeRide;

// GLOBAL: LEGOLAND 0x0081cb74
struct Ride *MFish2Ride;

// GLOBAL: LEGOLAND 0x0081cb80
struct Point DAT_0081cb80[3][16];

// GLOBAL: LEGOLAND 0x0081cd00
struct SpriteSet *JungleCruiseBoats;

// GLOBAL: LEGOLAND 0x0081cd04
void *DAT_0081cd04;

// GLOBAL: LEGOLAND 0x0081cd08
void *HedgeObjectClass;

// GLOBAL: LEGOLAND 0x0081cd0c
unsigned int CastleBBQRide;

// GLOBAL: LEGOLAND 0x0081cd10
unsigned int CastleBBQLayer;

// GLOBAL: LEGOLAND 0x0081cd14
unsigned int FoodcartDrinkRide;

// GLOBAL: LEGOLAND 0x0081cd18
struct EateryFX *SharkCafeRide;

// GLOBAL: LEGOLAND 0x0081cd1c
struct EateryFX *OctopusCafeRide;

// GLOBAL: LEGOLAND 0x0081cd20
struct Sprite *R2TowermSprite;

// GLOBAL: LEGOLAND 0x0081cd28
struct Sprite *RestMaskMainSprite;

// GLOBAL: LEGOLAND 0x0081cd2c
unsigned int EateryFxInner;

// GLOBAL: LEGOLAND 0x0081cd30
unsigned int Restaurant2Ride;

// GLOBAL: LEGOLAND 0x0081cd34
struct Sprite *R2FdoormSprite;

// GLOBAL: LEGOLAND 0x0081cd38
struct EateryFX *DAT_0081cd38;

// GLOBAL: LEGOLAND 0x0081cd3c
unsigned int FoodcartFoodRide;

// GLOBAL: LEGOLAND 0x0081cd40
unsigned int Restaurant1Ride;

// GLOBAL: LEGOLAND 0x0081cd44
struct EateryFX *ChuckWagonRide;

// GLOBAL: LEGOLAND 0x0081cd48
struct Sprite *R2Fdoorm1Sprite;

// GLOBAL: LEGOLAND 0x0081cd60
struct Sprite *OctTentASprite;

// GLOBAL: LEGOLAND 0x0081cd64
struct Sprite *OctTentBSprite;

// GLOBAL: LEGOLAND 0x0081cd68
struct Sprite *OctTentCSprite;

// GLOBAL: LEGOLAND 0x0081cd6c
struct Sprite *OctTentDSprite;

// GLOBAL: LEGOLAND 0x0081cd70
struct Sprite *OctTentESprite;

// GLOBAL: LEGOLAND 0x0081cd74
struct Sprite *OctTentFSprite;

// GLOBAL: LEGOLAND 0x0081cd78
struct Sprite *OctTentGSprite;

// GLOBAL: LEGOLAND 0x0081cd7c
struct Sprite *OctTentHSprite;

// GLOBAL: LEGOLAND 0x0081cd80
struct Sprite *OctoKioskSprite;

// GLOBAL: LEGOLAND 0x0081cd84
struct Sprite *R2BdoormSprite;

// GLOBAL: LEGOLAND 0x0081cd88
struct Sprite *RestMaskLevel1Sprite;

// GLOBAL: LEGOLAND 0x0081cd8c
struct Sprite *RestMaskLevel1aaSprite;

// GLOBAL: LEGOLAND 0x0081cd90
struct Sprite *RestMaskLevel3Sprite;

// GLOBAL: LEGOLAND 0x0081cd94
struct Sprite *RestMaskLevel2Sprite;

// GLOBAL: LEGOLAND 0x0081cda0
struct Sprite *OctTabAASprite;

// GLOBAL: LEGOLAND 0x0081cda4
struct Sprite *OctTabABSprite;

// GLOBAL: LEGOLAND 0x0081cda8
struct Sprite *OctTabBASprite;

// GLOBAL: LEGOLAND 0x0081cdac
struct Sprite *OctTabBBSprite;

// GLOBAL: LEGOLAND 0x0081cdb0
struct Sprite *OctTabCASprite;

// GLOBAL: LEGOLAND 0x0081cdb4
struct Sprite *OctTabCBSprite;

// GLOBAL: LEGOLAND 0x0081cdb8
struct Sprite *OctTabDASprite;

// GLOBAL: LEGOLAND 0x0081cdbc
struct Sprite *OctTabDBSprite;

// GLOBAL: LEGOLAND 0x0081cdc0
struct Sprite *OctTabEASprite;

// GLOBAL: LEGOLAND 0x0081cdc4
struct Sprite *OctTabEBSprite;

// GLOBAL: LEGOLAND 0x0081cdc8
struct Sprite *OctTabFASprite;

// GLOBAL: LEGOLAND 0x0081cdcc
struct Sprite *OctTabFBSprite;

// GLOBAL: LEGOLAND 0x0081cdd0
struct Sprite *OctTabGASprite;

// GLOBAL: LEGOLAND 0x0081cdd4
struct Sprite *OctTabGBSprite;

// GLOBAL: LEGOLAND 0x0081cdd8
struct Sprite *OctTabHASprite;

// GLOBAL: LEGOLAND 0x0081cddc
struct Sprite *OctTabHBSprite;

// GLOBAL: LEGOLAND 0x0081cde0
struct EateryFX *FoodcartIcecreamRide;

// GLOBAL: LEGOLAND 0x0081cde4
struct Ride *BalloonzRide;

// GLOBAL: LEGOLAND 0x0081cde8
struct Sprite *ZBalloon2Sprite;

// GLOBAL: LEGOLAND 0x0081cdec
void *DAT_0081cdec;

// GLOBAL: LEGOLAND 0x0081ce00
struct Cursor DAT_0081ce00[8];

// GLOBAL: LEGOLAND 0x00828fe0
unsigned int DAT_00828fe0[2];

// GLOBAL: LEGOLAND 0x00829980
void *BasicTilesData;

// GLOBAL: LEGOLAND 0x00829990
float DAT_00829990[3];

// GLOBAL: LEGOLAND 0x0082999c
float DAT_0082999c;

// GLOBAL: LEGOLAND 0x008299a0
float DAT_008299a0[3];

// GLOBAL: LEGOLAND 0x008299bc
struct FMat4 DAT_008299bc;

// GLOBAL: LEGOLAND 0x008299fc
struct FMat4 DAT_008299fc;

// GLOBAL: LEGOLAND 0x008299ac
int DAT_008299ac;

// GLOBAL: LEGOLAND 0x008299b0
int DAT_008299b0;

// GLOBAL: LEGOLAND 0x008299b4
int DAT_008299b4;

// GLOBAL: LEGOLAND 0x008299b8
int DAT_008299b8;

// GLOBAL: LEGOLAND 0x00829a3c
struct ListLink DAT_00829a3c;

// GLOBAL: LEGOLAND 0x00829a58
void (*DAT_00829a58)(void);

// GLOBAL: LEGOLAND 0x00829a5c
void (*DAT_00829a5c)(void);

// GLOBAL: LEGOLAND 0x00829a60
float DAT_00829a60;

// GLOBAL: LEGOLAND 0x00829a64
unsigned int DAT_00829a64;

// GLOBAL: LEGOLAND 0x00829a80
struct EditFootPrint DAT_00829a80;

// GLOBAL: LEGOLAND 0x00829abc
struct Element *DAT_00829abc;

// GLOBAL: LEGOLAND 0x00829ae0
unsigned int DAT_00829ae0;

// GLOBAL: LEGOLAND 0x00829ae4
unsigned int DAT_00829ae4;
// GLOBAL: LEGOLAND 0x00829ae8
short DAT_00829ae8[2];

// GLOBAL: LEGOLAND 0x00829af8
unsigned int DAT_00829af8[3];

// GLOBAL: LEGOLAND 0x00829b04
unsigned int DAT_00829b04[3];

// GLOBAL: LEGOLAND 0x00829b0c
unsigned int DAT_00829b0c;

// GLOBAL: LEGOLAND 0x00829aec
unsigned int DAT_00829aec;

// GLOBAL: LEGOLAND 0x00829af0
unsigned int DAT_00829af0;

// GLOBAL: LEGOLAND 0x00829af4
unsigned int DAT_00829af4;

// GLOBAL: LEGOLAND 0x004b5b48
unsigned int DAT_004b5b48;

// GLOBAL: LEGOLAND 0x00829b88
unsigned int DAT_00829b88;

// GLOBAL: LEGOLAND 0x00829b8c
unsigned int DAT_00829b8c;

// GLOBAL: LEGOLAND 0x00829ba0
unsigned int DAT_00829ba0;

// GLOBAL: LEGOLAND 0x00829ba4
unsigned int DAT_00829ba4;

// GLOBAL: LEGOLAND 0x00829b90
short DAT_00829b90[4][2];

// GLOBAL: LEGOLAND 0x00829ba8
short DAT_00829ba8[4][2];

// GLOBAL: LEGOLAND 0x00829bec
void *DAT_00829bec;

// GLOBAL: LEGOLAND 0x00829bf0
void *DAT_00829bf0;

// GLOBAL: LEGOLAND 0x00829bf4
void *DAT_00829bf4;

// GLOBAL: LEGOLAND 0x00829bf8
unsigned int DAT_00829bf8;

// GLOBAL: LEGOLAND 0x00829bfc
unsigned int DAT_00829bfc;

// GLOBAL: LEGOLAND 0x00829c00
int DAT_00829c00;

// GLOBAL: LEGOLAND 0x00829c04
struct Sprite *CastleMatteSprite;

// GLOBAL: LEGOLAND 0x00829c08
unsigned int DAT_00829c08;

// GLOBAL: LEGOLAND 0x00829c34
int DAT_00829c34;

// GLOBAL: LEGOLAND 0x00829c54
char *DAT_00829c54;

// GLOBAL: LEGOLAND 0x00829c60
void *DAT_00829c60[1024];

// GLOBAL: LEGOLAND 0x0082ac60
void *DAT_0082ac60[48];

// GLOBAL: LEGOLAND 0x0082ad20
unsigned char CastleDispatchTable[0x90];

// GLOBAL: LEGOLAND 0x0082adb0
unsigned int DAT_0082adb0[6];

// GLOBAL: LEGOLAND 0x0082add0
unsigned int CoasterTrainHeadCarLms;

// GLOBAL: LEGOLAND 0x0082add4
unsigned int CoasterTrainMidCarLms;

// GLOBAL: LEGOLAND 0x0082add8
unsigned int CoasterTrainTailCarLms;

// GLOBAL: LEGOLAND 0x0082ade0
unsigned int CoasterTrainHeadCarLfm;

// GLOBAL: LEGOLAND 0x0082ade4
unsigned int CoasterTrainMidCarLfm;

// GLOBAL: LEGOLAND 0x0082ade8
unsigned int CoasterTrainTailCarLfm;

// GLOBAL: LEGOLAND 0x0082adec
unsigned int DAT_0082adec;

// GLOBAL: LEGOLAND 0x0082adf0
struct Ride *BoatingSchoolWaterRide;

// GLOBAL: LEGOLAND 0x0082adf4
struct TileMap *BoatingSchoolTileMapping;

// GLOBAL: LEGOLAND 0x0082adf8
struct Ride *BoatingSchoolMermaidRide;

// GLOBAL: LEGOLAND 0x0082adfc
struct Sprite *BoatingSchoolHullMaskSprite;

// GLOBAL: LEGOLAND 0x0082ae00
void *DAT_0082ae00;

// GLOBAL: LEGOLAND 0x0082ae20
struct Cursor DAT_0082ae20;

// GLOBAL: LEGOLAND 0x0082c654
struct Sprite *BoatingSchoolRailmSprite;

// GLOBAL: LEGOLAND 0x0082c658
struct Ride *BoatingSchoolRide;

// GLOBAL: LEGOLAND 0x0082c65c
struct SpriteSet *BoatingSchoolBoats;

// GLOBAL: LEGOLAND 0x0082c668
struct Sprite *ZSpiderSprite;

// GLOBAL: LEGOLAND 0x0082c678
void *ZebraCrossingRide;

// GLOBAL: LEGOLAND 0x0082c67c
struct DSMapEntry *DSchoolMappingData;

// GLOBAL: LEGOLAND 0x0082c680
struct RoadLights *DrivingSchoolLightsData;

// GLOBAL: LEGOLAND 0x0082c684
void *DAT_0082c684;

// GLOBAL: LEGOLAND 0x0082c688
void *DAT_0082c688;

// GLOBAL: LEGOLAND 0x0082c690
unsigned short *DSchoolRedPalette;

// GLOBAL: LEGOLAND 0x0082c694
struct DSCursorSource *DrivingSchoolRide;

// GLOBAL: LEGOLAND 0x0082c6b8
unsigned short *DSchoolYellowPalette;

// GLOBAL: LEGOLAND 0x0082c6bc
unsigned short *DSchoolBluePalette;

// GLOBAL: LEGOLAND 0x0082c6c0
struct Sprite *DSchoolMatteSprite;

// GLOBAL: LEGOLAND 0x0082c6e0
struct Cursor DAT_0082c6e0;

// GLOBAL: LEGOLAND 0x0082df20
struct Cursor DAT_0082df20;

// GLOBAL: LEGOLAND 0x0082f760
struct Cursor DAT_0082f760;

// 0x00830b64/68/74, 0x00830f88/90 are DAT_0082f760 cursor fields.
// 0x00811564, 0x00811568, 0x00811988, 0x00811574 are QueryCursor fields
// (field_1404, field_1408, field_1828, field_1414[5]) — see struct Cursor.
// 0x007fffc8 is EditCursor.field_1408 — see struct Cursor in gamemap.h.

// GLOBAL: LEGOLAND 0x00830f94
struct Sprite *DSCarSprite;

// GLOBAL: LEGOLAND 0x00830f9c
struct DriveTable *DSchoolBlueCarData;

// GLOBAL: LEGOLAND 0x00830f98
struct Position *CoptersPos;

// GLOBAL: LEGOLAND 0x00830fc0
LEGO_EXPORT struct Cursor PathCursor;

// GLOBAL: LEGOLAND 0x00832800
LEGO_EXPORT struct MapStats MapStats;

// GLOBAL: LEGOLAND 0x00832bf0
LEGO_EXPORT void *PathSprite;

// GLOBAL: LEGOLAND 0x0082c6a0
struct RideSpriteInfo RideSpriteInfoBuffer;

// GLOBAL: LEGOLAND 0x00616028
struct RideSpriteInfo DAT_00616028;

// GLOBAL: LEGOLAND 0x006160a0
struct RideSpriteInfo DAT_006160a0;

// GLOBAL: LEGOLAND 0x0062fe30
struct RideSpriteInfo DAT_0062fe30;

// GLOBAL: LEGOLAND 0x0062fe10
struct RideSpriteInfo DAT_0062fe10;

// GLOBAL: LEGOLAND 0x004ab404
float FLOAT_004ab404;

// GLOBAL: LEGOLAND 0x004b55fc
struct PlaneSet *DAT_004b55fc;

// GLOBAL: LEGOLAND 0x004d83b4
struct RingHost *DAT_004d83b4;

// GLOBAL: LEGOLAND 0x004d83a0
unsigned int DAT_004d83a0[5];

// GLOBAL: LEGOLAND 0x004b5634
float FLOAT_004b5634;

// GLOBAL: LEGOLAND 0x004b559c
float DAT_004b559c[3];

// GLOBAL: LEGOLAND 0x004b55a8
float FLOAT_004b55a8;

// GLOBAL: LEGOLAND 0x004b4cac
char DAT_004b4cac[] = "manbox??";

// GLOBAL: LEGOLAND 0x00641000
int DAT_00641000;

// GLOBAL: LEGOLAND 0x00641004
int DAT_00641004[3001];

// GLOBAL: LEGOLAND 0x00643ee8
int DAT_00643ee8[3001][3];

// GLOBAL: LEGOLAND 0x0063810c
int DAT_0063810c;

// GLOBAL: LEGOLAND 0x0064cd90
int DAT_0064cd90;

// GLOBAL: LEGOLAND 0x0064cd88
int DAT_0064cd88;

// GLOBAL: LEGOLAND 0x0063835c
int DAT_0063835c;

// GLOBAL: LEGOLAND 0x00638110
int DAT_00638110;

// GLOBAL: LEGOLAND 0x00638108
int DAT_00638108;

// GLOBAL: LEGOLAND 0x004b5600
int **DAT_004b5600[2];

// GLOBAL: LEGOLAND 0x004b5610
float DAT_004b5610;

// GLOBAL: LEGOLAND 0x004b5614
float DAT_004b5614;

// GLOBAL: LEGOLAND 0x004b5618
float DAT_004b5618[3];

// GLOBAL: LEGOLAND 0x004b5624
float DAT_004b5624;

// GLOBAL: LEGOLAND 0x004b5628
float DAT_004b5628[3];

// GLOBAL: LEGOLAND 0x004d83c4
int DAT_004d83c4[0x100];

// GLOBAL: LEGOLAND 0x004d87c4
int *DAT_004d87c4;

// GLOBAL: LEGOLAND 0x004d884c
int *DAT_004d884c[32];

// GLOBAL: LEGOLAND 0x004d88cc
unsigned int *DAT_004d88cc[5];

// GLOBAL: LEGOLAND 0x004b5660
float DAT_004b5660[4][2];

// GLOBAL: LEGOLAND 0x004dd75c
unsigned int DAT_004dd75c;

// GLOBAL: LEGOLAND 0x004dd864
unsigned int DAT_004dd864;

// GLOBAL: LEGOLAND 0x004dd644
struct Struct1e40 *DAT_004dd644;

// GLOBAL: LEGOLAND 0x004dd648
struct Struct1e40 *DAT_004dd648;

// GLOBAL: LEGOLAND 0x004dd64c
float *DAT_004dd64c;

// GLOBAL: LEGOLAND 0x004dd650
int DAT_004dd650;

// GLOBAL: LEGOLAND 0x004ab438
float FLOAT_004ab438;

// GLOBAL: LEGOLAND 0x00610804
int DAT_00610804[64];

// GLOBAL: LEGOLAND 0x00610500
int DAT_00610500[64];

// GLOBAL: LEGOLAND 0x00610400
int DAT_00610400[64];

// GLOBAL: LEGOLAND 0x00610704
int DAT_00610704[64];

// GLOBAL: LEGOLAND 0x00610604
int DAT_00610604[64];

// GLOBAL: LEGOLAND 0x006100f8
int DAT_006100f8[64];

// GLOBAL: LEGOLAND 0x0060fdf8
int DAT_0060fdf8[64];

// GLOBAL: LEGOLAND 0x0060fef8
int DAT_0060fef8[64];

// GLOBAL: LEGOLAND 0x0060fbf8
int DAT_0060fbf8[64];

// GLOBAL: LEGOLAND 0x006101f8
int DAT_006101f8[64];

// GLOBAL: LEGOLAND 0x0060fcf8
int DAT_0060fcf8[64];

// GLOBAL: LEGOLAND 0x00610904
int DAT_00610904[64];

// GLOBAL: LEGOLAND 0x00610a10
unsigned int DAT_00610a10;

// GLOBAL: LEGOLAND 0x0060f900
int DAT_0060f900;

// GLOBAL: LEGOLAND 0x0060f904
int DAT_0060f904;

// GLOBAL: LEGOLAND 0x00579878
char DAT_00579878[0x80];

// GLOBAL: LEGOLAND 0x0060f8fc
int DAT_0060f8fc;

// GLOBAL: LEGOLAND 0x00611644
int DAT_00611644;

// GLOBAL: LEGOLAND 0x00615f68
int DAT_00615f68;

// GLOBAL: LEGOLAND 0x004dcbc8
int DAT_004dcbc8;

// GLOBAL: LEGOLAND 0x004d83bc
int DAT_004d83bc;

// GLOBAL: LEGOLAND 0x0060f910
int DAT_0060f910;

// GLOBAL: LEGOLAND 0x006121c8
float DAT_006121c8[6][3];

// GLOBAL: LEGOLAND 0x006126d8
float DAT_006126d8[6][2];

// GLOBAL: LEGOLAND 0x00612708
unsigned int DAT_00612708[30][36];

// GLOBAL: LEGOLAND 0x006137e8
float DAT_006137e8[3][4][3];

// GLOBAL: LEGOLAND 0x00613878
unsigned int DAT_00613878[36];

// GLOBAL: LEGOLAND 0x00613908
int DAT_00613908[24][2];

// GLOBAL: LEGOLAND 0x006139c8
int DAT_006139c8[31][6][5];

// GLOBAL: LEGOLAND 0x006148b8
int DAT_006148b8[546][2];

// GLOBAL: LEGOLAND 0x00667528
int ExceptionReportStarted;

// GLOBAL: LEGOLAND 0x004b8a88
int DAT_004b8a88 = 0x10;

// GLOBAL: LEGOLAND 0x004b8a8c
int DAT_004b8a8c = 0x800;

// GLOBAL: LEGOLAND 0x004b8a90
int DAT_004b8a90 = 8;
