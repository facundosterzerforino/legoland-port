#pragma once

#include <windows.h>
#include <ddraw.h>

#include "gamemap.h"
#include "legoland.h"
#include "math.h"

struct Bloke;
struct BlokeSex0;
struct BlokeInfo {
    unsigned int sex;
    unsigned int leg;
    unsigned int arm;
    char chest[0x14];
    char face[0x14];
    struct BlokeSex0 *bloke;
};
struct RecBuf;

/* DirectInput stays private to the input subsystem (input.c) + its definitions
 * (globals.c) — both include <dinput.h> directly. globals.h only needs an opaque
 * handle to the one DInput-typed global below, so it does NOT drag <dinput.h>
 * across every TU that includes globals.h. */
struct _DIMOUSESTATE;
struct CtrlBuffer;

// ---------------------------------------------------------------------------
// Forward declarations for typed/instance globals declared below. Full struct
// definitions live in the owning translation units; these globals only need an
// incomplete type for their pointer/extern declarations. (struct Cursor's full
// layout comes from gamemap.h, used by the EditCursor / QueryCursor instances.)
// ---------------------------------------------------------------------------
struct Sprite;
struct SampleDef;
/* One entry of the game's sound-effect list (GameFX), loaded by Load_FXList. */
struct FXItem {
    /* 0x00 */ char *name; /* file name under .\sfx\ */
    /* 0x04 */ unsigned char pad_4[0x8 - 0x4];
    /* 0x08 */ struct SampleDef *sample;
};
/* Indices into GameFX, named after the .wav each entry loads. */
enum GameFXIndex {
    FX_FLOWERS, /* Flowers.wav */
    FX_DRILL, /* RabOld\Drill.wav */
    FX_PUNCH, /* RabOld\Punch4.wav */
    FX_BUILDING1, /* Building 01.wav */
    FX_BUILDING2, /* Building 02.wav */
    FX_BUILDING3, /* Building 03.wav */
    FX_HAMMER, /* RabOld\Hammer.wav */
    FX_DRILLING, /* Drilling.wav */
    FX_BUILDING4, /* Building 04.wav */
    FX_CLICK1, /* Click01.wav */
    FX_CLICK2, /* Click02.wav */
    FX_CLICK3, /* Click03.wav */
    FX_BUTTON4, /* Button04.wav */
    FX_BUTTON14, /* Button14.wav */
    FX_RASP, /* Rasp1.wav */
    FX_GARDENER_LIFT, /* gardener lift up3.wav */
    FX_INVENTORY_IN, /* inventory slide away.wav */
    FX_INVENTORY_OUT, /* inventory slide out.wav */
    FX_MECHANIC_LIFT, /* mechanic lift2.wav */
    FX_THEME_CLICK, /* theme button click3.wav */
    FX_GARDENER_DROP, /* alroight then gardener put down.wav */
    FX_MECHANIC_DROP, /* tut tut for mechanic put down.wav */
    FX_WARNING, /* warning01.wav */
    FX_COUNT
};
struct FlumeDims {
    int field1;
    int field2;
};

struct FlumeShape {
    int count;
    struct FlumeDims *pts;
};

struct FlumeTemplate {
    int count;
    void *table;
};
struct BinVFile;
struct BalloonNode;
struct Ride;
struct Image;
struct IconNode;
struct Point;
struct DirNode;
struct CursorSource;
struct Building;
struct JailCell;
struct FXSpriteList;
struct ResVolume;
struct RideQueueEntry;
struct ObjectiveEvent;
struct MapElement;
struct WaterContext;
struct WaterSub;
struct WaterNode;
struct EateryFX;
struct BrollyData;
struct BrollyNode;
struct BlokeNode;
struct SaveBlock;
struct JungleRide;
struct JungleCursor;
struct JungleFish;
struct JungleScore;
struct JungleObj;
struct JunglePath;
struct BoatRide;
struct RideNode;
struct PathNode;
struct BoatRideNode;
struct MermaidNode;
struct Ride;
struct ObjClass;
struct RideLayer;
struct CatapultNode;
struct Position;
struct RinData;
struct CopterNode;
struct RenderItemNode;
struct GoldNode;
struct CarouselNode;
struct CarouselRide;
struct EarthNode;
struct JoustNode;
struct InterfaceIconNode;
struct IconNode;
struct InterfaceProfileObj;
struct InterfaceListNode;
struct MoviePool;
struct PanelNode;
struct InterfaceQueryNode;
struct InterfaceEventNode;
struct InterfaceResearchNode;
struct InfoObjData;
struct Element;
struct SortNode;
struct DSCursorSource;
struct SpaceTowerCar;

// Full definitions needed by typed globals defined in globals.c.
struct SpaceTowerLayout {
    /* 0x00 */ unsigned short field_0;
    /* 0x02 */ unsigned char pad_2[2];
    /* 0x04 */ void *anim_layout;
};

struct SpaceTowerSeatData {
    /* 0x00 */ struct Point inner;
    /* 0x08 */ struct Point outer;
    /* 0x10 */ int direction;
};

/* An object under construction; on disk the class is saved as its LLIDB index. */
struct BuildObj {
    union {
        struct Ride *ride;
        int index;
    };
    TileId coords;
    unsigned short pad;
    int elapsed;
};
typedef struct BuildObj BuildObj;

typedef struct MapElement MapElement;
struct MapElement {
    /* 0x00 */ struct Element *field_0; /* class of the object on this tile */
    /* 0x04 */ union {
        struct {
            unsigned char field_4;
            unsigned char field_5;
        };
        TileId anchor; /* tile of the object's origin */
    };
    /* 0x06 */ TileId next; /* anchor tile of the next object on the map */
    /* 0x08 */ unsigned short field_8;
    /* 0x0a */ unsigned short field_a;
    /* 0x0c */ unsigned short flags;
    /* 0x0e */ unsigned char pad_e[0x10 - 0xe];
    /* 0x10 */ unsigned char field_10;
    /* 0x11 */ unsigned char durability_level;
    /* 0x12 */ unsigned short field_12;
};

struct FMat4 {
    float m[4][4];
};

struct LegoConfig {
    /* 0x00 */ unsigned short screen_width;
    /* 0x02 */ unsigned short screen_height;
    /* 0x04 */ unsigned short scroll_border_x;
    /* 0x06 */ unsigned short scroll_border_y;
    /* 0x08 */ unsigned short scroll_accel_x;
    /* 0x0a */ unsigned short scroll_accel_y;
    /* 0x0c */ unsigned short scroll_max_x;
    /* 0x0e */ unsigned short scroll_max_y;
    /* 0x10 */ unsigned short view_width;
    /* 0x12 */ unsigned short view_height;
    /* 0x14 */ unsigned short width;
    /* 0x16 */ unsigned short height;
    /* 0x18 */ unsigned short field_18;
    /* 0x1a */ unsigned short max_blokes;
    /* 0x1c */ unsigned char field_1c;
    /* 0x1d */ unsigned char pad_1d[1];
    /* 0x1e */ unsigned short field_1e;
    /* 0x20 */ unsigned short view_x;
    /* 0x22 */ unsigned short view_y;
    /* 0x24 */ unsigned short field_24;
    /* 0x26 */ unsigned char pad_26[2];
    /* 0x28 */ unsigned int level;
    /* 0x2c */ unsigned int field_2c;
    /* 0x30 */ unsigned int field_30;
    /* 0x34 */ unsigned int mechanics_enabled;
    /* 0x38 */ unsigned int gardeners_enabled;
    /* 0x3c */ int repair_orders_enabled;
    /* 0x40 */ unsigned int field_40;
};

struct ObjTableEntry {
    unsigned short key;
    unsigned short value;
};

struct ColorLutEntry {
    unsigned short shifted;
    unsigned short scaled;
    unsigned short value;
    unsigned short pad;
};

struct FreePlaySpriteSlot {
    struct Sprite *sprite0;
    struct Sprite *sprite1;
    unsigned char pad_8[0x14];
};

struct ProgressEntry {
    /* 0x00 */ const char *name0;
    /* 0x04 */ const char *name1;
    /* 0x08 */ int id;
    /* 0x0c */ int x;
    /* 0x10 */ int y;
    /* 0x14 */ struct Sprite *sprite0;
    /* 0x18 */ struct Sprite *sprite1;
};

/* 0x4beb80: progress-screen levels, then the tutorial-screen entries */
struct ProgressTables {
    /* 0x000 */ struct ProgressEntry levels[10];
    /* 0x118 */ int last_clicked;
    /* 0x11c */ int pad_11c;
    /* 0x120 */ struct ProgressEntry tutorials[5];
    /* 0x1ac */ unsigned char tail[0x14];
};

struct InfoTimedEntry {
    /* 0x00 */ unsigned int sample;
    /* 0x04 */ int interval;
    /* 0x08 */ int last_time;
};

/* Boating school: one straight step per direction (dx, dy per frame; ox, oy start offset). */
struct BoatDirStep {
    /* 0x00 */ int dx;
    /* 0x04 */ int dy;
    /* 0x08 */ int ox;
    /* 0x0c */ int oy;
};

/* Boating school: one quarter-circle turn (angle a0 -> a1 around cx, cy). */
struct BoatArc {
    /* 0x00 */ float a0;
    /* 0x04 */ float a1;
    /* 0x08 */ float cx;
    /* 0x0c */ float cy;
};

struct MapRect {
    /* 0x00 */ int x0;
    /* 0x04 */ int y0;
    /* 0x08 */ int x1;
    /* 0x0c */ int y1;
};
typedef struct MapRect MapRect;

struct TextCell {
    /* 0x00 */ int width;
    /* 0x04 */ int height;
    /* 0x08 */ unsigned int format;
    /* 0x0c */ char *name;
    /* 0x10 */ unsigned int bg_color;
    /* 0x14 */ unsigned int text_color;
    /* 0x18 */ int font;
    /* 0x1c */ struct Sprite *sprite;
};

/* A tile set: sprites plus per-tile-type callbacks. */
struct FXSpriteList {
    /* 0x00 */ int base;
    /* 0x04 */ unsigned char pad_4[0xc - 0x4];
    /* 0x0c */ int *sprite_ids;
    /* 0x10 */ unsigned char pad_10[0x18 - 0x10];
    /* 0x18 */ unsigned char (*get_rf_flags)(int x, int y);
    /* 0x1c */ void (*on_enter)(struct Point pos);
    /* 0x20 */ void (*on_leave)(int x, int y);
};
typedef struct FXSpriteList FXSpriteList;

struct TileSpriteEntry {
    struct FXSpriteList *src;
    unsigned short sprite;
    unsigned short pad_6;
};

struct DeferredSprite {
    struct Sprite *sprite;
    unsigned int x;
    unsigned int y;
    unsigned int flags;
};

struct ListLink {
    unsigned char pad_0[0x14];
    struct ListLink *var_14;
    struct ListLink *var_18;
};

struct CastleFloatEnt {
    float a;
    float b;
    float c;
};

/* The textured mesh FUN_004234e0 draws (the roller-coaster track tube is built into DAT_004b5f60). */
struct MeshVert {
    int x;
    int y;
    int z;
    unsigned int codes; /* clip code, as FUN_00426250 computes it */
    int u;
};

struct TexMesh {
    /* 0x00 */ int palette;
    /* 0x04 */ int pad_4;
    /* 0x08 */ int corner_count;
    /* 0x0c */ int face_count;
    /* 0x10 */ struct MeshVert *verts;
    /* 0x14 */ int (*corners)[2]; /* vertex pairs */
    /* 0x18 */ unsigned int (*faces)[3]; /* corner indices; the top bit picks the pair's second vertex */
};

struct Int16Pair {
    short x;
    short y;
};

/* castle track-segment list node (0x60f914 table entries; also embedded in castle nodes) */
struct LSub {
    /* 0x00 */ int r[4];
    /* 0x10 */ struct LSub *next;
    /* 0x14 */ struct Int16Pair xy;
    /* 0x18 */ unsigned int k;
    /* 0x1c */ float a;
    /* 0x20 */ float b;
};

/* 0x6102f8: three 0x58-byte ride-path objects (doubly linked via next/prev) */
struct CastlePathObj {
    unsigned int f0[0x10];
    float f40;
    float f44;
    float f48;
    void **f4c;
    struct CastlePathObj *next;
    struct CastlePathObj *prev;
};

/* 0x7fded4: newly unlocked objects shown by the "new objects" popup */
/* 0x7fea4c / 0x7febac: clip rectangle used by the software sprite renderer */
struct DrawClipOrigin {
    int top;
    int left;
};

struct DrawClipSize {
    int height;
    int width;
};

struct NewObjTable {
    /* 0x00 */ void *objs[20];
    /* 0x50 */ struct Sprite *sprites[20];
    /* 0xa0 */ unsigned int count;
    /* 0xa4 */ int current;
};

struct EditFootPrint {
    unsigned char data[16];
    void *next;
};

struct ProfileData {
    /* 0x00 */ char name[0x1e];
    /* 0x1e */ unsigned char name_len;
    /* 0x1f */ unsigned char pad_1f;
    /* 0x20 */ unsigned int field_20;
    /* 0x24 */ unsigned char field_24;
    /* 0x25 */ unsigned char gap_25[3];
    /* 0x28 */ unsigned int speech_volume;
    /* 0x2c */ unsigned int music_volume;
    /* 0x30 */ unsigned int fx_volume;
    /* 0x34 */ unsigned int field_34;
    /* 0x38 */ unsigned int field_38;
    /* 0x3c */ unsigned int field_3c;
    /* 0x40 */ unsigned short field_40;
    /* 0x42 */ unsigned char field_42;
    /* 0x43 */ unsigned char field_43[200];
    /* 0x10b */ unsigned char field_10b;
    /* 0x10c */ unsigned char gap_10c[4];
};

struct EditState {
    unsigned int unk0;
    unsigned int unk4;
    struct Ride *unk8; /* instance of the object being placed */
};

struct ScreenMode {
    unsigned int unk0;
    unsigned int unk4;
    unsigned int unk8;
};

struct ScreenState {
    /* 0x00 */ unsigned char gap_0[0x20];
    /* 0x20 */ unsigned int field_20;
    /* 0x24 */ int speech_volume;
    /* 0x28 */ int music_volume;
    /* 0x2c */ int fx_volume;
    /* 0x30 */ unsigned char flags[0x13];
    /* 0x43 */ unsigned char profile_slot;
    /* 0x44 */ unsigned char save_slot;
    /* 0x45 */ unsigned char field_45;
    /* 0x46 */ unsigned char field_46[0xca];
};

// ---------------------------------------------------------------------------
// All game globals, sorted by ascending address. Definitions (with any
// initializers) live in globals.c. This phase only collects the declarations;
// per-TU inline externs are migrated to use this header in a later phase.
// ---------------------------------------------------------------------------
/* Per object-type map statistics (0x2c bytes). The scan_* fields accumulate during DoMapAI's
   tile sweep and are latched into built/capacity/tiles/salvage when the sweep completes. */
struct MapAIClass {
    /* 0x00 */ int built;
    /* 0x04 */ int classes;
    /* 0x08 */ int capacity;
    /* 0x0c */ int tiles;
    /* 0x10 */ int salvage;
    /* 0x14 */ int percent;
    /* 0x18 */ int limit;
    /* 0x1c */ int scan_tiles;
    /* 0x20 */ int scan_built;
    /* 0x24 */ int scan_salvage;
    /* 0x28 */ int scan_capacity;
};

/* 0x832800..0x832bf0: map AI statistics (ResetMapAI clears all 0x3f0 bytes). */
struct MapStats {
    /* 0x000 */ unsigned int field_0;
    /* 0x004 */ int scan_x;
    /* 0x008 */ int scan_y;
    /* 0x00c */ int scan_stage;
    /* 0x010 */ struct MapAIClass classes[6];
    /* 0x118 */ int total_tiles;
    /* 0x11c */ unsigned int capacity;
    /* 0x120 */ unsigned int capacity_max;
    /* 0x124 */ unsigned int capacity_min;
    /* 0x128 */ int mood_threshold0;
    /* 0x12c */ int mood_threshold1;
    /* 0x130 */ int mood_threshold2;
    /* 0x134 */ int mood_threshold3;
    /* 0x138 */ int mood_threshold4;
    /* 0x13c */ int mood_delta[13];
    /* 0x170 */ int entrance_fee;
    /* 0x174 */ unsigned int brick_meter_max;
    /* 0x178 */ unsigned int timer_minutes;
    /* 0x17c */ int field_17c;
    /* 0x180 */ unsigned int field_180;
    /* 0x184 */ unsigned int field_184;
    /* 0x188 */ unsigned int field_188;
    /* 0x18c */ unsigned int field_18c;
    /* 0x190 */ unsigned int field_190;
    /* 0x194 */ unsigned int field_194;
    /* 0x198 */ char field_198[256];
    /* 0x298 */ char field_298[256];
    unsigned char pad_398[0x4];
    /* 0x39c */ int field_39c;
    /* 0x3a0 */ unsigned int field_3a0;
    /* 0x3a4 */ unsigned int field_3a4;
    /* 0x3a8 */ unsigned int field_3a8;
    /* 0x3ac */ unsigned int field_3ac;
    /* 0x3b0 */ char leave_ratings[0x19];
    /* 0x3c9 */ signed char leave_rating_index;
    unsigned char pad_3ca[0x2];
    /* 0x3cc */ int power_spare_percent;
    /* 0x3d0 */ int power_supply;
    /* 0x3d4 */ int power_demand;
    /* 0x3d8 */ unsigned int unpowered_demand;
    /* 0x3dc */ int unpowered_count;
    /* 0x3e0 */ int field_3e0[4];
};

struct FortRect {
    int left;
    int top;
    int right;
    int bottom;
};

// 0x004ab5f0
extern GUID IID_IDirectMusicComposer;
// 0x004ab640
extern GUID IID_IDirectMusicPerformance;
// 0x004ab6a0
extern GUID IID_IDirectMusicLoader;
// 0x004ab7b0
extern GUID GUID_Download;
// 0x004ab880
extern GUID GUID_NOTIFICATION_MEASUREANDBEAT;
// 0x004ab8a0
extern GUID GUID_NOTIFICATION_SEGMENT;
// 0x004ab900
extern GUID CLSID_DirectMusicLoader;
// 0x004ab920
extern GUID CLSID_DirectMusicComposer;
// 0x004aba00
extern GUID CLSID_DirectMusicPerformance;
// 0x004acfd0
extern GUID NullGuid;
// 0x004b4580
extern struct FortRect DAT_004b4580;

// 0x004ab390
extern float FLOAT_004ab390;
extern float FLOAT_004ab468;
// 0x004ab398
extern double DOUBLE_004ab398;
// 0x004ab3dc
extern float DAT_004ab3dc;
// 0x004ab3e0
extern float DAT_004ab3e0;
// 0x004ab3e4
extern float DAT_004ab3e4;
// 0x004ab3e8
extern float DAT_004ab3e8;
// 0x004ab3ec
extern float DAT_004ab3ec;
// 0x004ab3f0
extern float DAT_004ab3f0;
// 0x004ab3f4
extern float DAT_004ab3f4;
// 0x004ab3f8
extern float DAT_004ab3f8;
// 0x004ab3fc
extern float DAT_004ab3fc;
// 0x004ab478
extern double DOUBLE_004ab478;
// 0x004ab4a0
extern float DAT_004ab4a0;
// 0x004ab4a8
extern double DAT_004ab4a8;
// 0x004ab4b0
extern double DAT_004ab4b0;
// 0x004ab430
extern float DAT_004ab430;
// 0x004ab43c
extern float FLOAT_004ab43c;
// 0x004ab418
extern double DAT_004ab418;
// 0x004ab444
extern float FLOAT_004ab444;
// 0x004ab550
extern float FLOAT_004ab550;
// 0x004b4860
extern struct Point DAT_004b4860;

// 0x004c2b00
extern struct FlumeShape DAT_004c2b00;

// 0x004c2b30
extern struct FlumeDims DAT_004c2b30[4];

// 0x004c2b58
extern struct FlumeShape DAT_004c2b58;

// 0x004c2b78
extern struct FlumeDims DAT_004c2b78[4];

// 0x004c2bc0
extern struct FlumeShape DAT_004c2bc0;

// 0x004c2bc8
extern struct FlumeDims DAT_004c2bc8[4];

// 0x004c2be8
extern struct FlumeShape DAT_004c2be8;

// 0x004c2c08
extern struct FlumeShape DAT_004c2c08;

// 0x004c2c10
extern struct FlumeShape DAT_004c2c10;

// 0x004c8d58
extern struct FlumeDims DAT_004c8d58[2];

// 0x004cbde8
extern struct FlumeDims DAT_004cbde8[4];

// 0x004cbe38
extern struct FlumeDims DAT_004cbe38[2];

// 0x004d8bb8
extern int DAT_004d8bb8[1024][4];
// 0x004dcbb8
extern float DAT_004dcbb8[3];
// 0x0066b638
extern struct ColorLutEntry ColorLut[256];
// 0x004ab44c
extern float FLOAT_004ab44c;
// 0x004ab454
extern float FLOAT_004ab454;
// 0x004ab458
extern float FLOAT_004ab458;
// 0x004ab45c
extern float FLOAT_004ab45c;
// 0x004ab4bc
extern float DAT_004ab4bc;
// 0x004ab4c0
extern float DAT_004ab4c0;
// 0x004ab4c8
extern double DAT_004ab4c8;
// 0x004ab518
extern float DAT_004ab518;
// 0x004ab528
extern float DAT_004ab528;
// 0x004ab52c
extern float FLOAT_004ab52c;
// 0x004ab530
extern double DOUBLE_004ab530;
// 0x004ab538
extern double DOUBLE_004ab538;
// 0x004ab558
extern double DOUBLE_004ab558;
// 0x004ab560 c_dfDIKeyboard, 0x004ab578 c_dfDIMouse — declared in <dinput.h>, defined in globals.c
// 0x004ab5e0
extern GUID IID_IDirectMusicBand;
// 0x004ab600
extern GUID IID_IDirectMusicChordMap;
// 0x004ab610
extern GUID IID_IDirectMusicStyle;
// 0x004ab670
extern GUID IID_IDirectMusicSegment;
// 0x004ab6d0
extern GUID GUID_PerfMasterGrooveLevel;
// 0x004ab8b0
extern GUID GUID_DirectMusicAllTypes;
// 0x004ab8e0
extern GUID DirectMusicBandClassGuid;
// 0x004ab930
extern GUID DirectMusicChordMapClassGuid;
// 0x004ab980
extern GUID CLSID_DirectMusicStyle;
// 0x004ab9f0
extern GUID CLSID_DirectMusicSegment;
// 0x004ac090 GUID_SysKeyboard, 0x004ac0a0 GUID_SysMouse — declared in <dinput.h>, defined in globals.c
// 0x004acf80
extern GUID IID_IDirectDraw2_Guid;
// 0x004b4034
extern int DAT_004b4034[16];
// 0x004b40a4
extern unsigned int DAT_004b40a4[4];
extern int DAT_004b40b4[4];
// 0x004b40c8
extern unsigned char Catapult_SFX[0x70];
// 0x004b4140
extern unsigned char Helicopter_SFX[0x70];
// 0x004b43f8
extern unsigned char DRIVING_SCHOOL_SFX[0xa8];
// 0x004b4440
extern unsigned int DAT_004b4440[5];
// 0x004b4458
extern unsigned int DAT_004b4458[5];
// 0x004b4470
extern unsigned int DAT_004b4470[5];
// 0x004b4688
extern unsigned char JOUST_SFX[12];
// 0x004b4728
extern unsigned int LogFlumeFootprint;
// 0x004b472c
extern unsigned int DAT_004b472c;
// 0x004b4730
extern unsigned int DAT_004b4730;
// 0x004b4734
extern unsigned int DAT_004b4734;
// 0x004b473c
extern int DAT_004b473c[4];
// 0x004b474c
extern int DAT_004b474c[2];
// 0x004b4754
extern int DAT_004b4754[4];
// 0x004b4764
extern int DAT_004b4764;
// 0x004b47f8
extern struct Sprite *DAT_004b47f8;
// 0x004b4804
extern struct Sprite *DAT_004b4804;
// 0x004b4808
extern struct FlumeTemplate DAT_004b4808;
// 0x004b4818
extern struct Sprite *DAT_004b4818;
// 0x004b4824
extern struct Sprite *DAT_004b4824;
// 0x004b4828
extern struct FlumeTemplate DAT_004b4828;
// 0x004b4838
extern struct Sprite *DAT_004b4838;
// 0x004b4850
extern struct Sprite *DAT_004b4850;
// 0x004b4858
extern struct FlumeTemplate DAT_004b4858;
// 0x004b4bd0
extern unsigned char DAT_004b4bd0[0x14];
// 0x004b4bf0
extern unsigned int DAT_004b4bf0[5];
// 0x004b4c04
extern int DAT_004b4c04;
// 0x004b4cb8
extern struct FXItem SAFARI_SFX[1];
// 0x004b4cc4
extern int DAT_004b4cc4[8];
// 0x004b4ce4
extern int DAT_004b4ce4[8];
// 0x004b4d88
extern struct FXItem SpiderRide_SFX[1];
// 0x004b4d94
struct SpiderBnv {
    char name[8];
    int tab1[16];
    int tab2[17];
};
extern struct SpiderBnv SpiderBnvInfo;
// 0x004b4e20
extern Point DAT_004b4e20;
// 0x004b78b4
extern char BoxBlokeBnvName[];
// 0x004b4f08
extern char *DAT_004b4f08[4];
// 0x004b4f18
extern signed char DAT_004b4f18[4];
// 0x004b4f1c
extern signed char DAT_004b4f1c[4];
// 0x004b4fa8
extern unsigned char WATERWORKS_SFX[0x24];
// 0x004b5118
extern struct BoatDirStep BoatingSchoolDirSteps[4];
// 0x004b5158
extern struct BoatArc DAT_004b5158[4];
// 0x004b5198
extern struct BoatArc DAT_004b5198[4];
// 0x004b51d8
extern int DAT_004b51d8[0x80];
// 0x004b5260
extern struct Footprint DAT_004b5260;
// 0x004b5278
extern struct Footprint DAT_004b5278;
// 0x004b5290
extern struct Point DAT_004b5290[6];
// 0x004b52c0
extern unsigned char PTR_s_Boat_Noise_wav[0x18];
// 0x004b53c0
extern struct Footprint DAT_004b53c0;
// 0x004b53d4
extern unsigned char DAT_004b53d4[0x190];
// 0x004b5570
extern unsigned int DAT_004b5570[5];
// 0x004b5584
extern short DAT_004b5584[4][2];
// 0x004b55f4
extern unsigned int DAT_004b55f4;
// 0x004b5608
extern unsigned int DAT_004b5608;
// 0x004b560c
extern int DAT_004b560c;
// 0x004b5b20
extern void *DAT_004b5b20;
// 0x004b5b24
extern unsigned short *DAT_004b5b24;
// 0x004b5b28
extern int DAT_004b5b28;
// 0x004b5b4c
extern int DAT_004b5b4c;
// 0x004b5b50
extern int DAT_004b5b50;
// 0x004b59e8
extern char SitLoGirlSitName[];
// 0x004b59f8
extern char SitLoManSitName[];
// 0x004b5abc
extern float DAT_004b5abc[4][4];
// 0x004b5b58
extern unsigned short DAT_004b5b58;
// 0x004b5b5a
extern unsigned short DAT_004b5b5a;
// 0x004b5b60
extern unsigned short DAT_004b5b60;
// 0x004b5b62
extern unsigned short DAT_004b5b62;
// 0x004b5b3c
extern struct RecBuf *DAT_004b5b3c;
// 0x004b5988
extern struct BlokeInfo DAT_004b5988;
// 0x004b5c1c
extern struct FMat4 DAT_004b5c1c[2];
// 0x004b5c9c
extern unsigned int DAT_004b5c9c;
// 0x004b5ca0
extern unsigned int DAT_004b5ca0;
// 0x004b5ca4
extern unsigned int DAT_004b5ca4;
// 0x004b5ca8
extern unsigned int DAT_004b5ca8;
// 0x004b5cac
extern float DAT_004b5cac;
// 0x004b5cb0
extern unsigned int DAT_004b5cb0;
// 0x004b5cb4
extern unsigned int DAT_004b5cb4;
// 0x004b5cb8
extern unsigned int DAT_004b5cb8;
// 0x004b5cbc
extern float DAT_004b5cbc;
// 0x004b5cc0
extern unsigned int DAT_004b5cc0;
// 0x004b5cc4
extern unsigned int DAT_004b5cc4;
// 0x004b5cc8
extern unsigned int DAT_004b5cc8;
// 0x004b5d20
extern char RollerCoasterSavePath[32];
extern const unsigned char DAT_004b5d20[1];

// 0x004b5d58
extern const unsigned char DAT_004b5d58[1];

// 0x004b5d90
extern const unsigned char DAT_004b5d90[1];

// 0x004b5dc8
extern const unsigned char DAT_004b5dc8[1];
// 0x004b5df0
extern unsigned int DAT_004b5df0[4];
// 0x004b5e00
extern float DAT_004b5e00[4][3];
// 0x004b5e30
extern float DAT_004b5e30[4][3];
// 0x004b5e60
extern float DAT_004b5e60[4][3];
// 0x004b5e90
extern float DAT_004b5e90[4][3];
// 0x004b5ec0
extern int DAT_004b5ec0[4];
// 0x004b5ee0
extern int DAT_004b5ee0[5];
// 0x004b5ef4
extern unsigned int DAT_004b5ef4[4][2];
// 0x004b5f14
extern unsigned int DAT_004b5f14[4][2];
// 0x004b5f60
extern struct TexMesh DAT_004b5f60;
// 0x004b6150
extern unsigned int DAT_004b6150[12];
// 0x004b61e0
extern struct CastleFloatEnt DAT_004b61e0[8];

// 0x004b62f0
extern unsigned int DAT_004b62f0;
// 0x004b6300
extern unsigned int DAT_004b6300;
// 0x004b6398
extern float DAT_004b6398[4][3];
// 0x004b63c8
extern float DAT_004b63c8[4][3];
// 0x004b6408
extern int DAT_004b6408[3];
// 0x004ab490
extern float FLOAT_004ab490;
// 0x004ab494
extern float FLOAT_004ab494;
// 0x004b64d4
extern char DAT_004b64d4[4];
// 0x004b64d8
extern struct FXItem CAROUSSEL_SFX[2];
// 0x004b65c0
extern int DAT_004b65c0[8];
// 0x004b6638
extern char DAT_004b6638[4];
// 0x004b6654
extern int ENTRANCE_DEST_RIGHT[2];
// 0x004b665c
extern int ENTRANCE_DEST_LEFT[2];
// 0x004b6668
extern unsigned char ENTRANCE_SFX[0x1c];
// 0x004b66e8
extern unsigned char RESTAURANT_SFX[0xc];
// 0x004b66f4
extern int DAT_004b66f4[15 * 6];
// 0x004b685c
extern int DAT_004b685c;
// 0x004b6860
extern int DAT_004b6860[0x20];
// 0x004b68e0
extern int DAT_004b68e0[0x22];
// 0x004b6968
extern unsigned char OCTOPUS_SFX[0x24];
// 0x004b6990
extern int DAT_004b6990[0x29];
// 0x004b6a34
extern int DAT_004b6a34[4];
// 0x004b6a44
extern int DAT_004b6a44[0xd];
// 0x004b6a78
extern int DAT_004b6a78[0x30];
// 0x004b6b38
extern int DAT_004b6b38[0x2c];
// 0x004b6be8
extern int DAT_004b6be8[0x5c];
// 0x004b6d58
extern unsigned char DAT_004b6d58[0x120];
// 0x004b7260
extern struct Footprint DAT_004b7260;
// 0x004b7278
extern struct Footprint JungleCruiseStartFootprint;
// 0x004b7288
extern struct Footprint *DAT_004b7288;
// 0x004b7148
extern struct BoatDirStep DAT_004b7148[4];
// 0x004b7188
extern struct BoatArc DAT_004b7188[4];
// 0x004b71c8
extern struct BoatArc DAT_004b71c8[4];
// 0x004b7290
extern struct Point DAT_004b7290[5];
// 0x004b7230
extern struct Footprint DAT_004b7230;
// 0x004b7248
extern struct Footprint DAT_004b7248;
// 0x004b72e4
extern unsigned char DAT_004b72e4[0x190];
// 0x004b7478
extern struct Footprint DAT_004b7478;
// 0x004b7618
extern struct FXItem SPACE_TOWER_SFX[1];
// 0x004b76b8
extern unsigned char DAT_004b76b8[16];
// 0x004b7750
extern unsigned char DAT_004b7750[16];
// 0x004b7758
extern struct SpaceTowerLayout DAT_004b7758[8];
// 0x004b7798
extern struct SpaceTowerSeatData DAT_004b7798[4];
// 0x004b77e8
extern struct Point DAT_004b77e8[8];
// 0x004b79d0
extern char DAT_004b79bc[];
extern unsigned char DAT_004b79d0[0x18];
// 0x004b7abc
extern unsigned short DAT_004b7abc;
// 0x004b7ac0
extern unsigned char BlokeColours[0x18];
// 0x004b7d70
extern unsigned int DAT_004b7d70;
// 0x004b7d74
extern unsigned int DAT_004b7d74;
// 0x004b7d78
extern unsigned int DAT_004b7d78;
// 0x004b7d7e
extern unsigned short DAT_004b7d7e;
// 0x004b7d84
extern unsigned int DAT_004b7d84;
// 0x004b7e9c
extern char *DAT_004b7e9c[22];
// 0x004b81c0
extern char *GraphicsPath;
// 0x004b81c4
extern char *GraphicsPathPrefix;
// 0x004b81c8
extern char *GraphicsSmallPath;
// 0x004b81cc
extern char *MasksPath;
// 0x004b81d0
extern char *MasksSmallPath;
// 0x004b81d4
extern char *IconsPath;
// 0x004b81d8
extern char *ModelsPath;
// 0x004b8318
extern struct Point DAT_004b8318;
// 0x004b8320
extern struct Point DAT_004b8320;
// 0x004b8328
extern struct Point DAT_004b8328;
// 0x004b8334
extern int DAT_004b8334[4];
// 0x004b8344
extern char DAT_004b8344;
// 0x004b8348
extern char *PTR_DAT_004b8348[8];
// 0x004b8368
extern void (*PTR_Bloke_DoNothing_004b8368[16])(struct Bloke *);
// 0x004b85c4
extern HANDLE DAT_004b85c4;
// 0x004b8710
extern unsigned char FountainSFX[0x40];
// 0x004b8750
extern unsigned char PowerStationSFX[0x18];
// 0x004b8768
extern struct FXItem DINO_SFX[5];
// 0x004b87a8
extern struct FXItem MONEY_SFX[2];
// 0x004b8bbc
extern unsigned char PercentSFormat[1];
// 0x004b90f8
extern unsigned int BrickCount;
// 0x004b90fc
extern unsigned int UnlimitedBricks;
// 0x004b9210
extern int DAT_004b9210;
// 0x004b9214
extern int DAT_004b9214;
// 0x004b9218
extern int DAT_004b9218;
// 0x004b921c
extern int DAT_004b921c;
// 0x004b9220
extern LEGO_EXPORT unsigned int BGFullUpdate;
// 0x004b9228
extern struct FXItem GameFX[FX_COUNT];
// 0x004b95f0
extern struct DeferredSprite *PTR_DAT_004b95f0;
// 0x004b9550
extern unsigned char DAT_004b9550[8];
// 0x004b9558
extern unsigned int DAT_004b9558[9];
// 0x004b957c
extern int DAT_004b957c[9];
// 0x004b95a0
extern int DAT_004b95a0[9];
// 0x004b95c4
extern unsigned char DAT_004b95c4[8];
// 0x004b95cc
extern unsigned char DAT_004b95cc[8];
// 0x004b95d4
extern unsigned char DAT_004b95d4[8];
// 0x004b95dc
extern unsigned char DAT_004b95dc[8];
// 0x004b95e8
extern int DAT_004b95e8;
// 0x004b95ec
extern int DAT_004b95ec;
// 0x004b95f4
extern int DAT_004b95f4;
// 0x004b95f8
extern int DAT_004b95f8;
// 0x004b95fc
extern int DAT_004b95fc;
// 0x004b9600
extern int DAT_004b9600;
// 0x004b9604
extern int DAT_004b9604;
// 0x004b9608
extern int DAT_004b9608;
// 0x004b960c
extern int DAT_004b960c;
// 0x004b9610
extern int DAT_004b9610;
// 0x004b9ca4
extern int (*BlitFrameFunc)(void);
// 0x004b9ca8
extern unsigned int OverrideFrame;
// 0x004b9e5c
extern unsigned int DAT_004b9e5c[71];
// 0x004b9f78
extern int DAT_004b9f78[4];
// 0x004b9f88
extern unsigned int DAT_004b9f88;
// 0x004b9f8c
extern unsigned int DAT_004b9f8c;
// 0x004ba87c
extern void *PTR_some_list_head_004ba87c;
// 0x004ba880
extern void *PTR_DAT_004ba880;
// 0x004ba884
extern unsigned int DAT_004ba884;
// 0x004ba8e0
extern struct InfoTimedEntry DAT_004ba8e0[17];
// 0x004ba9ac
extern unsigned int DAT_004ba9ac[234];
// 0x004bad54
extern int mouse_granularity;
struct KeyMapping {
    /* 0x00 */ unsigned char flags;
    /* 0x01 */ signed char code;
};
// 0x004bad58
extern struct KeyMapping DAT_004bad58[0x3c];
// 0x004bafa8
extern unsigned int DAT_004bafa8[20];
// 0x004baff8
extern unsigned int DAT_004baff8;
// 0x004baffc
extern char DAT_004baffc[4][0x14];
// 0x004bb094
extern int DAT_004bb04c[18];
extern unsigned int DAT_004bb094;
// 0x004bb098
extern unsigned int DAT_004bb098;
// 0x004bb09c
extern unsigned int DAT_004bb09c;
// 0x004bb0a0
extern unsigned int DAT_004bb0a0;
// 0x004bb0a4
extern struct TrackElemPair DAT_004bb0a4[29];
// 0x004bb18c
extern unsigned int DAT_004bb18c[4];
// 0x004bb4dc
extern float DAT_004bb4dc;
// 0x004bb4e0
extern BITMAPINFOHEADER DAT_004bb4e0;
// 0x004bb588
// 0x004bb58c
// 0x004bb598
extern struct Point DAT_004bb598;
// 0x004bb5a0
extern int DAT_004bb5a0;
// 0x004bb5a4
extern int DAT_004bb5a4;
// 0x004bb5ac
extern unsigned int DAT_004bb5ac;
// 0x004bb5b0
extern unsigned int ScriptConditionActive;
// 0x004bb5b4
extern char *DAT_004bb5b4[4];
// 0x004bb5c4
extern char *DAT_004bb5c4[5];
// 0x004bb5d8
extern char *DAT_004bb5d8[2];
// 0x004bb5e0
extern char *DAT_004bb5e0[5];
// 0x004bb5f4
extern char *DAT_004bb5f4[12];
// 0x004bb624
extern char *DAT_004bb624[25];
// 0x004bb688
extern char *DAT_004bb688[13];
// 0x004bb6bc
extern char *DAT_004bb6bc[6];
// 0x004bb6d4
extern char *DAT_004bb6d4[9];
// 0x004bb6f8
extern struct ScriptCommand DAT_004bb6f8[0x5d];
// 0x004bcba4
extern const char *ResourceFileNames[3];
// 0x004bcbf4
extern LEGO_EXPORT struct LegoConfig *lpConfig;
// 0x004bcecc
extern char *PTR_s_Aaron_004bcecc[0x53];
// 0x004bd018
extern char *PTR_s_Abbie_004bd018[0x5a];
// 0x004bd180
extern char *PTR_s_Adams_004bd180[0x6b];
// 0x004bd32c
extern short DAT_004bd32c[8][2]; /* unit step (x, y) per direction, 8.8 fixed point */
// 0x004bd34c
extern void (*PTR_FUN_004bd34c[16])(struct Bloke *);
// 0x004bdd00
extern struct HoverInfo Hover;
// 0x004bdea0
extern LEGO_EXPORT RECT SPRITE_ClipRect;
// 0x004beb80
extern struct ProgressTables ProgressScreenTables;
// 0x004bed40
extern char DAT_004bed40[0x25c]; /* progress/tutorial sprite-name strings, see globals.c */
// 0x004bef9c
extern unsigned int DAT_004bef9c;
// 0x004bf670
extern unsigned int DAT_004bf670;
// 0x004bf774
extern unsigned int MusicEnabled;
// 0x004bf778
extern unsigned int MusicState;
// 0x004bff28
extern const int DAT_004bff28[12][2];
// 0x004c10d4
extern void *DAT_004c10d4;
// 0x004c10dc
extern struct Ride *CastleLevelRide;
// 0x004c10e4
extern struct Sprite *CastleLevelMatteSprite;
// 0x004c10e8
extern struct RenderItemNode *DAT_004c10e8;
// 0x004c10f0
extern void *DAT_004c10f0;
// 0x004c10f4
extern void *ActiveCatapultRide;
// 0x004c1100
extern struct RideSpriteInfo DAT_004c1100;
// 0x004c1118
extern struct CatapultNode *CatapultNodeList;
// 0x004c1120
extern struct Sprite *CopterBaseMatteSprite;
// 0x004c1124
extern int CopterQueueTables[5];
// 0x004c1138
extern void *CopterModelLayers;
// 0x004c113c
extern struct Sprite *CopterModelSprites[10];
// 0x004c1164
extern struct RenderItemNode **DAT_004c1164;
// 0x004c1168
extern struct RenderItemNode **DAT_004c1168;
// 0x004c1170
extern struct RideSpriteInfo DAT_004c1170;
// 0x004c1188
extern struct RenderItemNode **DAT_004c1188;
// 0x004c1190
extern struct RenderItemNode **DAT_004c1190;
// 0x004c1194
extern struct RenderItemNode **DAT_004c1194;
// 0x004c1198
extern void *ActiveCopterRide;
// 0x004c119c
extern struct RenderItemNode *DAT_004c119c;
// 0x004c11a0
extern struct RenderItemNode *DAT_004c11a0;
// 0x004c11a4
extern struct RenderItemNode *DAT_004c11a4;
// 0x004c11a8
extern struct RenderItemNode *DAT_004c11a8;
// 0x004c11ac
extern struct RenderItemNode *DAT_004c11ac;
// 0x004c11b0
extern struct RenderItemNode *DAT_004c11b0;
// 0x004c11b4
extern struct CopterNode *CopterNodeList;
// 0x004c11bc
extern void *DrivingSchoolCountList;
// 0x004c11c0
extern int DAT_004c11c0;
// 0x004c11c4
extern struct RideQueueEntry *DAT_004c11c4;
// 0x004c11c8
extern struct RideQueueEntry *DAT_004c11c8;
// 0x004c11cc
extern struct Sprite *FortMaskSprite;
// 0x004c11d8
extern struct Sprite *FortLayer;
// 0x004c11dc
extern struct Ride *FortRide;
// 0x004c11e0
extern struct RenderItemNode *DAT_004c11e0;
// 0x004c11e4
extern struct Sprite *DAT_004c11e4;
// 0x004c11e8
extern struct Sprite *GoldRushLayer;
// 0x004c11f0
extern void *GoldRushRide;
// 0x004c11f4
extern struct Sprite *GoldMaskSprite;
// 0x004c11f8
extern struct Sprite *GoldWashMatte1Sprite;
// 0x004c11fc
extern struct Sprite *GoldWashSprite;
// 0x004c1200
extern struct Sprite *GoldWashMatte2Sprite;
// 0x004c1204
extern struct GoldNode *GoldWashList;
// 0x004c1208
extern struct RenderItemNode *GoldRushBlokeRenderList;
// 0x004c1210
extern struct Sprite *ZJoustSprite;
// 0x004c1214
extern unsigned int JoustLayer;
// 0x004c1218
extern void *JoustRideBnv;
// 0x004c121c
extern unsigned int JoustRide;
// 0x004c1228
extern struct RideSpriteInfo DAT_004c1228;
// 0x004c123c
extern void *DAT_004c123c[1];
// 0x004c1240
extern struct Sprite *DAT_004c1240;
// 0x004c1244
extern struct Sprite *JoustFMaskSprite;
// 0x004c1248
extern struct Sprite *JoustSpecRMSprite;
// 0x004c124c
extern struct Sprite *JoustSpecLMSprite;
// 0x004c1250
extern struct JoustNode *JoustNodeList;
// 0x004c1258
extern struct Sprite *LogFlumeTunelMSprite;
// 0x004c1260
extern struct Cursor DAT_004c1260;
// 0x004c2a94
extern struct Sprite *LogFlumeHup1M1Sprite;
// 0x004c2a98
extern struct Sprite *LogFlumeHup1M2Sprite;
// 0x004c2aa8
extern struct Footprint DAT_004c2aa8;
// 0x004c2aa0
extern struct CursorSource *DAT_004c2aa0;
// 0x004c2abc
extern struct Sprite *LogFlumeTrackSprites[10];
// 0x004c2ae4
extern struct Sprite *DAT_004c2ae4;
// 0x004c2ae8
extern void *DAT_004c2ae8;
// 0x004c2aec
extern struct Sprite *LogFlumeDrop2MSprite;
// 0x004c2af0
extern struct Sprite *LogFlumeDrop1MSprite;
// 0x004c2af4
extern unsigned int DAT_004c2af4;
// 0x004c2af8
extern void *DAT_004c2af8;
// 0x004c2afc
extern unsigned int LogFlumeImageListId;
// 0x004c2b0c
extern struct CursorSource *DAT_004c2b0c;
// 0x004c2b10
extern unsigned int LogFlumeTrackEndyListId;
// 0x004c2b50
extern void *DAT_004c2b50;
// 0x004c2b60
extern struct CursorSource *DAT_004c2b60;
// 0x004c2b64
extern struct Sprite *LogFlumeSplashSprite;
// 0x004c2b68
extern struct SpriteSet *LogFlumeImageList;
// 0x004c2b6c
extern struct Sprite *LogFlumeFc1M1Sprite;
// 0x004c2b70
extern struct Sprite *LogFlumeFc1M2Sprite;
// 0x004c2b98
extern unsigned int DAT_004c2b98;
// 0x004c2b9c
extern struct Ride *LogFlumeEntranceRide;
// 0x004c2ba0
extern void *DAT_004c2ba0;
// 0x004c2c18
extern struct Cursor DAT_004c2c18;
// 0x004c2bf0
extern struct CursorSource *DAT_004c2bf0;
// 0x004c445c
extern struct CursorSource *DAT_004c445c;
// 0x004c4460
extern void *DAT_004c4460;
// 0x004c5c90
extern unsigned int DAT_004c5c90;
// 0x004c74c8
extern unsigned int DAT_004c74c8;
// 0x004c74f4
extern struct Element *DAT_004c74f4;
// 0x004c74f8
extern struct Cursor LogFlumeCursor;
// 0x004c4468
extern struct Cursor DAT_004c4468;
// 0x004c5ca0
extern struct Cursor DAT_004c5ca0;
// 0x004c74d4
extern struct CursorSource *DAT_004c74d4;
// 0x004c74d8
extern struct RideSpriteInfo DAT_004c74d8;
// 0x004c8d2c
extern struct Sprite *LogFlumeFc2M1Sprite;
// 0x004c8d38
extern struct Footprint DAT_004c8d38;
// 0x004c8d4c
extern void *DAT_004c8d4c;
// 0x004c8d50
extern unsigned int DAT_004c8d50;
// 0x004c8d54
extern struct Sprite *LogFlumeEntranceLayer;
// 0x004c8d68
extern struct Sprite *LogFlumeFc3M3Sprite;
// 0x004c8d78
extern struct Cursor DAT_004c8d78;
// 0x004c8d6c
extern struct CursorSource *DAT_004c8d6c;
// 0x004c8d70
extern struct Sprite *LogFlumeFc4MSprite;
extern struct RenderItemNode *DAT_004c8d74;
extern struct RenderItemNode *DAT_004ca5ac;
// 0x004ca5b0
extern struct Cursor DAT_004ca5b0;
// 0x004cbe08
extern struct Sprite *LogFlumeFc3M2Sprite;
// 0x004cbe0c
extern struct Sprite *LogFlumeFc3M1Sprite;
// 0x004cbe10
extern void *DAT_004cbe10;
// 0x004cbe14
extern struct Sprite *LogFlumeCsaw2MSprite;
// 0x004cbe18
extern struct CursorSource *DAT_004cbe18;
// 0x004cbe1c
extern struct Sprite *LogFlumeFc1M3Sprite;
// 0x004cbe20
extern unsigned int DAT_004cbe20;
// 0x004cbe24
extern unsigned int DAT_004cbe24;
// 0x004cbe28
extern unsigned int DAT_004cbe28;
// 0x004cbe2c
extern unsigned int DAT_004cbe2c;
// 0x004cbe30
extern struct Ride *LogFlumeTrackRide;
// 0x004cbe48
extern void *DAT_004cbe48;
// 0x004cbe4c
extern struct Sprite *LogFlumeEnta3MSprite;
// 0x004cbe70
extern struct RenderItemNode *DAT_004cbe70;
// 0x004cbe74
extern struct Sprite *LogFlumeBarrelSprite;
// 0x004cbe78
extern struct Sprite *LogFlumeBarrelMSprite;
// 0x004cbe7c
extern struct Sprite *LogFlumeBarrel1Sprite;
// 0x004cbe80
extern struct Sprite *LogFlumeBarrelMatteSprite;
// 0x004cbe50
extern struct SpriteSet *LogFlumeTrackEndyList;
// 0x004cbe58
extern struct RideSpriteInfo DAT_004cbe58;
// 0x004cbe84
extern struct FlumeEntry *FlumeEntryList;
// 0x004cbe88
extern struct Sprite *LogFlumeEnta1Matte2Sprite;
// 0x004cbe8c
extern struct Sprite *LogFlumeSignSprite;
// 0x004cbe90
extern struct Sprite *LogFlumeEntrance2Sprite;
// 0x004cbe94
extern struct Sprite *LogFlumeEntrance3Sprite;
// 0x004cbe98
extern struct Sprite *LogFlumeEntrance1Sprite;
// 0x004cbe9c
extern unsigned int DrivingSchoolPumpsRide;
// 0x004cbea4
extern void *PumpList;
// 0x004cbeac
extern struct RideQueueEntry *DAT_004cbeac;
// 0x004cbeb0
extern int DAT_004cbeb0;
// 0x004cbeb4
extern int DAT_004cbeb4;
// 0x004cbeb8
extern unsigned int DAT_004cbeb8;
// 0x004cbec4
extern struct SafariOwner *SafariRide;
// 0x004cbed0
extern struct RideSpriteInfo DAT_004cbed0;
// 0x004cbec0
extern void *SafariOffBNV;
// 0x004cbec8
extern struct Sprite *SafariLayer;
// 0x004cbecc
extern struct RenderItemNode *DAT_004cbecc;
// 0x004cbee8
extern int DAT_004cbee8;
// 0x004cbeec
extern int DAT_004cbeec;
// 0x004cbef4
extern void *SafariRunBNV;
// 0x007988d0
extern void *DAT_007988d0[600];
// 0x00799230
extern struct DirectMusicSegment *MusicSegments[35];
// 0x00799c1c
extern unsigned char *DAT_00799c1c[35];
// 0x0079a608
extern int DAT_0079a608[35];
// 0x0079a69c
extern void *DMusicNotificationEvent;
// 0x0079a6b0
extern int DAT_0079a6b0;
// 0x007fe9a4
extern int DAT_007fe9a4;
// 0x007fea14
extern int DAT_007fea14;
// 0x007fea1c
extern int DAT_007fea1c;
// 0x007fea4c
extern struct DrawClipOrigin SpriteClipOrigin;
// 0x007febac
extern struct DrawClipSize DrawClipExtent;
// 0x0082c66c
extern struct Sprite *ZSafariSprite;
// 0x0082c670
extern struct Point DAT_0082c670;
// 0x004cbef8
extern void *DAT_004cbef8;
// 0x004cbefc
extern void *DAT_004cbefc;
// 0x004cbf00
extern void *DAT_004cbf00;
// 0x004cbf04
extern void *DAT_004cbf04[1];
// 0x004cbf08
extern struct Sprite *DAT_004cbf08;
// 0x004cbf0c
extern struct SafariNode *SafariNodeList;
// 0x004cbf10
extern void *SpiderRunBinV;
// 0x004cbf14
extern struct Sprite *SpiderHutMask1Sprite;
// 0x004cbf18
extern void *SpiderOffBinV;
// 0x004cbf1c
extern struct Sprite *SpiderHutMask2Sprite;
// 0x004cbf20
extern unsigned int DAT_004cbf20;
// 0x004cbf24
extern void *SpiderOnBinV;
// 0x004cbf28
extern struct Sprite *SpiderRideLayer;
// 0x0082c660
extern Point DAT_0082c660;
// 0x004cbf30
extern void *DAT_004cbf30[2];
// 0x004cbf38
extern void *DAT_004cbf38[2];
// 0x004cbf40
extern struct RideSpriteInfo DAT_004cbf40;
// 0x004cbf58
extern struct SpiderNode *SpiderNodeList;
// 0x004cbf5c
extern struct Ride *TempleBuildingRide;
// 0x004cbf64
extern void *TempleLayer;
// 0x004cbf68
extern struct Sprite *TempleMatte1Sprite;
// 0x004cbf6c
extern struct Sprite *TempleMatte2Sprite;
// 0x004cbf70
extern struct RenderItemNode *DAT_004cbf70;
// 0x004cbf7c
extern struct SlideCar *DAT_004cbf7c;
// 0x004cbf78
extern struct Sprite *TempSlideMatteSprite;
// 0x004cbf80
extern struct SlideTrack *DAT_004cbf80;
// 0x004cbf84
extern struct RenderItemNode *DAT_004cbf84;
// 0x004cbf88
extern unsigned int DAT_004cbf88;
// 0x004cbf8c
extern unsigned int DAT_004cbf8c;
// 0x004cbf98
extern struct RideSpriteInfo DAT_004cbf98;
// 0x004cbfb8
extern unsigned int DAT_004cbfb8[3];
// 0x004cbfc4
extern void *TempSlideBinV;
// 0x004cbfc8
extern int DAT_004cbfc8;
// 0x004cbfcc
extern int DAT_004cbfcc[1];
// 0x004cbfd0
extern struct Sprite *ZTempSlideSprite;
// 0x004cbfd4
extern struct SlideNode *SlideNodeList;
// 0x004cbfd8
extern struct Sprite *TopwaterSprite;
// 0x004cbfdc
extern struct WaterContext *ElephantFountainRide;
// 0x004cbfe0
extern struct WaterSub *ShowerLayer;
// 0x004cbfe4
extern unsigned int DAT_004cbfe4;
// 0x004cbfe8
extern unsigned int WaterWorksImageListHandle;
// 0x004cbfec
extern unsigned int ElephantFountainLayer;
// 0x004cbff0
extern struct RideSpriteInfo DAT_004cbff0;
// 0x004cc008
extern struct WaterContext *WaterBlockRide;
// 0x004cc014
extern struct Sprite *ShowerSprite;
// 0x004cc018
extern void *WaterWorksImageListData;
// 0x004cc01c
extern unsigned int DAT_004cc01c;
// 0x004cc020
extern struct Sprite *WwElsquirtSprite;
// 0x004cc024
extern struct WaterContext *ShowerRide;
// 0x004cc028
extern unsigned int WaterWorksSfxRefCount;
// 0x004cc02c
extern struct WaterNode *WaterNodeList;
// 0x004cc030
extern struct WaterNode *DAT_004cc030;
// 0x004cc034
extern struct WaterNode *ElephantFountainList;
// 0x004cc03c
extern struct BoatRide *BoatRideList;
// 0x004cc048
extern struct Footprint DAT_004cc048;
// 0x004cc060
extern struct Footprint BoatingSchoolStartFootprint;
// 0x004cc070
extern int *DAT_004cc070;
// 0x004cc074
extern struct BoatRideNode *BoatRideNodeList;
// 0x004cc078
extern struct Footprint BoatingSchoolFootprint;
// 0x004cc088
extern int *DAT_004cc088;
// 0x004cc08c
extern unsigned int BoatingSchoolAnimTick;
// 0x004cc090
extern struct Cursor DAT_004cc090[4];
// 0x004d2164
extern struct MermaidNode *MermaidList;
// 0x004d2168
extern struct Cursor DAT_004d2168[4];
// 0x004d823c
extern struct PathNode *BoatPathList;
// 0x004d8240
extern struct PathNode *DAT_004d8240;
// 0x004d8244
extern struct PathNode *DAT_004d8244;
// 0x004d8250
struct SprOwner;
struct SprEnt {
    unsigned int field_0;
    struct SprOwner *owner;
    int id_a;
    unsigned int h_a;
    int id_b;
    unsigned int h_b;
};
extern struct SprEnt DAT_004d8250;
// 0x004d8268
extern unsigned int DAT_004d8268;
// 0x004d8270
extern unsigned int DAT_004d8270;
extern float DAT_004d829c[64];
extern unsigned int DAT_004d83c0;
// 0x004d88f4
extern unsigned char DAT_004d88f4[0x80];
// 0x004d8974
extern unsigned char DAT_004d8974[0x40];
// 0x004d89c4
extern void *DAT_004d89c4;
// 0x004d89c8
extern unsigned int LtxFileTable[30];
// 0x004d8a40
extern unsigned int LmsFileTable[31];
// 0x004d8abc
extern unsigned int LfmFileTable[30];
// 0x004d8b34
extern unsigned int LfmFileSizes[30];
// 0x004d8bac
extern unsigned int RollercoasterLpt;
// 0x004d8bb0
extern char DAT_004d8bb0[0x100];
// 0x004dcbd0
extern void *DAT_004dcbd0[10];
// 0x004dcbf8
extern unsigned int DAT_004dcbf8;
// 0x004dcc00
extern unsigned char DAT_004dcc00[2520];
// 0x004dd5d8
extern unsigned int DAT_004dd5d8;
// 0x004dd5e0
extern void *DAT_004dd5e0[24];
// 0x004dd758
extern unsigned int CoasterTxtFile;
// 0x004dd760
extern char DAT_004dd760[256];
// 0x004dd860
extern unsigned int CoasterObjFile;
// 0x004dd868
extern unsigned int DAT_004dd868;
// 0x004dd86c
extern unsigned int DAT_004dd86c;
// 0x004dd870
extern char DAT_004dd870[0x6000];
// 0x0060f908
extern int DAT_0060f908;
// 0x0060f90c
extern int DAT_0060f90c;
// 0x004e3870
extern char DAT_004e3870[];
// 0x0060f914
extern struct LSub DAT_0060f914[20];
// 0x006102f8
extern struct CastlePathObj DAT_006102f8[3];
// 0x00610a04
extern unsigned int DAT_00610a04;
// 0x00610a08
extern unsigned int DAT_00610a08;
// 0x00610a18
extern struct RenderItemNode *DAT_00610a18;
// 0x00610a20 — sqrtf lookup table (64 entries: {intercept, slope} pairs)
extern float sqrtf_table[128];
// 0x00610c40 — invsqrtf lookup table (64 entries: {intercept, slope} pairs)
extern float invsqrtf_table[128];
// 0x00610e44 — invsqrtf exponent scale table (256 entries)
extern float invsqrtf_exp_table[256];
// 0x00611244 — sqrtf exponent scale table (256 entries)
extern float sqrtf_exp_table[256];
// 0x00611648
extern unsigned int DAT_00611648;
// 0x0061164c
extern float DAT_0061164c;
// 0x00611650
extern float DAT_00611650;
// 0x00611654
extern float DAT_00611654;
// 0x00611658
extern Vector3 DAT_00611658[4];
// 0x00611688
/* castle.c lookup table: FUN_00427c00 finds value by key; DAT_00611958 counts the entries */
struct KeyValuePair {
    unsigned int key;
    unsigned int value;
};
extern struct KeyValuePair DAT_00611688[11];
// 0x006116e0
extern Vector3 DAT_006116e0[4];
// 0x00611710
extern unsigned int DAT_00611710[16];
// 0x00611750
extern Vector3 DAT_00611750[4];
// 0x00611780
extern unsigned char DAT_00611780[0x40];
// 0x006117c0
extern Vector3 DAT_006117c0[34];
// 0x00611958
extern unsigned int DAT_00611958;
// 0x00612178
extern int DAT_00612178[20];
// 0x006122a0
extern float DAT_006122a0[90][3];
// 0x00614858
extern struct CastleFloatEnt DAT_00614858[8];

// 0x00616180
extern struct Cursor DAT_00616180[8];
// 0x00622320
extern struct Cursor JungleCruiseCursors[4];
// 0x006159c8
extern int DAT_006159c8[90][4];
// 0x00615f6c
extern unsigned int DAT_00615f6c;
// 0x00615f70
extern unsigned int DAT_00615f70[5];
// 0x00615f84
extern void *DAT_00615f84;
extern int DAT_00615fc4;
extern int DAT_00615fc8;
extern int DAT_00615fcc;
extern float DAT_00615fd0;
extern float DAT_00615fd8;
extern float DAT_00615fdc;
extern float DAT_00615fe0;
extern int DAT_00615fe4;
extern int DAT_00615fe8;
extern int DAT_00615fec;
extern float *DAT_00615f8c;
extern int (*DAT_004b63fc)(float (*fn)(unsigned int), float lo, float hi, float *out);

extern void *DAT_00615f80;
extern int DAT_00615ff0;
extern float DAT_00615ff4;
extern struct CastleFloatEnt DAT_004b5f80[3][4];
extern struct CastleFloatEnt DAT_00612210[3][4];
// 0x00615f90
extern int DAT_00615f90;
// 0x00615fd4
extern float DAT_00615fd4;
// 0x00615f98
extern unsigned int DAT_00615f98;
// 0x00616000
extern unsigned int CoasterTrainWheelLms;
// 0x00616004
extern unsigned int CoasterTrainWheelLfm;
// 0x00616010
extern struct BinVFile *BalloonzBinV;
// 0x00616018
extern struct BinVFile *DAT_00616018[4];
// 0x0061603c
extern struct Sprite *DAT_0061603c[1];
// 0x00616040
extern struct Sprite *DAT_00616040;
// 0x00616044
extern struct Sprite *BalloonzLayer;
// 0x00616048
extern struct Sprite *BallBaseM1Sprite;
// 0x0061604c
extern struct Sprite *BallBaseM2Sprite;
// 0x00616050
extern struct Sprite *BallBaseM3Sprite;
// 0x00616054
extern struct Sprite *BZRedCarM1Sprite;
// 0x00616058
extern struct Sprite *BZGreenCarM1Sprite;
// 0x0061605c
extern struct Sprite *BZBlueCarM1Sprite;
// 0x00616060
extern struct BalloonNode *BalloonNodeList;
// 0x00616068
extern void *CarouselLayer;
// 0x0061606c
extern struct Sprite *CarouselEntranceMatteSprite;
// 0x00616070
extern struct Sprite *CarouselEntranceMatte2Sprite;
// 0x00616078
extern int DAT_00616078;
// 0x0061607c
extern int DAT_0061607c;
// 0x00616080
extern void *CarouselOnBinV;
// 0x00616084
extern void *CarouselOffBinV;
// 0x0061608c
extern void *CarouselBinV;
// 0x00616090
extern void *DAT_00616090;
// 0x00616094
extern void *DAT_00616094;
// 0x00616098
extern void *DAT_00616098;
// 0x006160b8
extern struct Sprite *ZCarouselSprite;
// 0x006160bc
extern struct CarouselRide *DAT_006160bc;
// 0x006160c0
extern struct Sprite *DAT_006160c0;
// 0x006160c4
extern struct CarouselNode *CarouselNodeList;
// 0x006160d0
extern unsigned int EarthSlideRide;
// 0x006160d4
extern struct RinData *EarthSlideRin;
// 0x006160d8
extern struct Sprite *EarthSlideEntranceMatteSprite;
// 0x006160e0
extern struct Sprite *EarthSlideEntranceMatte2Sprite;
// 0x006160e4
extern struct Position *EarthPos;
// 0x006160c8
extern unsigned int DAT_006160c8;
// 0x006160cc
extern unsigned int DAT_006160cc;
// 0x006160e8
extern struct EarthNode *EarthNodeHead;
// 0x006160ec
extern struct RenderItemNode *BlokeRenderList;
// 0x006160f4
extern struct Ride *EntranceRide;
// 0x006160f0
extern struct Sprite *EntranceLayer;
// 0x00616110
extern int DAT_00616110;
// 0x006160fc
extern struct Sprite *EntranceMatte1Sprite;
// 0x00616100
extern struct Sprite *EntranceMatte2Sprite;
// 0x00616104
extern struct Sprite *EntranceMatte3Sprite;
// 0x00616108
extern struct Sprite *EntranceMatte4Sprite;
// 0x0061610c
extern struct Sprite *Booth1Sprite;
// 0x00616118
extern unsigned int EateryLayerOwner;
// 0x00616120
extern struct RideSpriteInfo DAT_00616120;
// 0x0061613c
extern unsigned int BrollyImagesHandle;
// 0x00616140
extern struct BrollyData *BrollyImagesData;
// 0x00616144
extern struct BrollyNode *DAT_00616144;
// 0x00616148
extern struct SaveBlock *SaveBlockList;
// 0x0061614c
extern unsigned int HedgeImagesData;
// 0x00616150
extern unsigned int FlowerImagesHandle;
// 0x00616158
extern unsigned int FlowerImagesData;
// 0x0061615c
extern unsigned int HedgeImagesHandle;
// 0x00616164
extern struct JungleRide *JungleRideList;
// 0x00629c2c
extern struct JungleObj *DAT_00629c2c;
// 0x00629c30
extern struct JungleFish *JungleFishList;
// 0x00629c34
extern struct JungleObj *DAT_00629c34;
// 0x00629c3c
extern struct JungleScore *JungleScoreList;
// 0x00629c40
extern struct Footprint JungleCruiseFootprint;
// 0x00629c50
extern struct Footprint *DAT_00629c50;
// 0x00629c54
extern int JungleCruiseStep;
// 0x00629c58
extern struct Cursor DAT_00629c58[4];
// 0x0062fd2c
extern struct JunglePath *JunglePathList;
// 0x0062fd30
extern struct JunglePath *DAT_0062fd30;
// 0x0062fd34
extern struct JunglePath *JungleBfsNextFrontier;
// 0x0062fd3c
extern struct JailCell *JailCellList;
// 0x0062fd40
extern void *DAT_0062fd40;
// 0x0062fd48
extern struct RideSpriteInfo DAT_0062fd48;
// 0x0062fd60
extern void *SpaceTowerLayers;
// 0x0062fd64
extern struct Sprite *SpaceTowerSeatMatteSprites[4];
// 0x0062fd74
extern void *SpaceTowerRideObj;
// 0x0062fd7c
extern struct Sprite *SpaceTowerMatte2Sprite;
// 0x0062fd80
extern struct Sprite *SpaceTowerMatte1Sprite;
// 0x0062fd88
extern struct Point DAT_0062fd88[4];
// 0x0062fda8
extern struct SpaceTowerCar *SpaceTowerCarList;
// 0x0062fdb0
extern struct RideSpriteInfo DAT_0062fdb0;
// 0x0062fdc8
extern void *BoxBlokesOffBNV;
// 0x0062fdcc
extern struct Sprite *SpinningBarrelsEntranceMatte2Sprite;
// 0x0062fdd0
extern struct Sprite *SpinningBarrelsEntranceMatteSprite;
// 0x0062fde4
extern struct Ride *SpinningBarrelsRide;
// 0x0062fde0
extern struct Sprite *SpinningBarrelsLayer;
// 0x0062fdd8
extern struct Point DAT_0062fdd8;
// 0x0062fde8
extern void *SpinningBarrelsBNV;
// 0x0062fdf0
extern void *DAT_0062fdf0[3];
// 0x0062fdfc
extern void *BoxBlokesOn1BNV;
// 0x0062fe00
extern void *DAT_0062fe00[1];
// 0x0062fe04
extern struct Sprite *ZSpinningBarrelsSprite;
// 0x0062fe08
extern struct BarrelNode *SpinningBarrelList;
// 0x0062fe48
extern struct RideLayer *PottingShedLayer;
// 0x0062fe4c
extern struct Sprite *GShedMatteSprite;
// 0x0062fe50
extern struct RideLayer *MechanicsHutLayer;
// 0x0062fe54
extern struct Sprite *MechHutMaskSprite;
// 0x0062fe58
extern struct Ride *PlaneRide;
// 0x0062fe60
extern struct RideSpriteInfo DAT_0062fe60;
// 0x0062fe78
extern void *Zoomer0ffBinV;
// 0x0062fe7c
extern struct Sprite *PlaneRideLayer;
// 0x0062fe84
extern void *DAT_0062fe84[3];
// 0x0062fe90
extern void *ZoomerideBinV;
// 0x0062fe94
extern void *Zoomer0nBinV;
// 0x0062fe98
extern struct Sprite *DAT_0062fe98;
// 0x0062fe9c
extern struct PlaneRideNode *PlaneRideNodeList;
// 0x0062fea0
extern int DialogListScrollY;
// 0x0062fea4
extern int DAT_0062fea4;
// 0x0062fea8
extern int *DAT_0062fea8;
// 0x0062feac
extern void *AltWomanFileData;
// 0x0062feb0
extern void *GeoffMeshes[2];
// 0x0062feb8
extern int DAT_0062feb8[1];
// 0x0062febc
extern void *ManMeshes[6];
// 0x0062fed4
extern void *WomanMeshes[6];
// 0x0062feec
extern unsigned int DAT_0062feec[1];
// 0x0062fef0
extern unsigned int DAT_0062fef0;
// 0x0062fef4
extern void *TracyWalkMesh;
// 0x0062fef8
extern int *DAT_0062fef8;
// 0x00630100
extern void *AltManFileData;
// 0x00630108
extern unsigned int DAT_00630108[0x2000];
// 0x00638218
extern unsigned int DAT_00638218[80];
// 0x00638358
extern unsigned short DAT_00638358;
// 0x0064cd8c
extern int *DAT_0064cd8c;
// 0x00655a38
extern int *DAT_00655a38;
// 0x00655a3c
extern void *PersonListHead;
// 0x00655a4c
extern unsigned int RenderItemCount;
// 0x00655a50
extern unsigned int RenderItem2Count;
// 0x00665e8c
extern unsigned int DAT_00665e8c;
// 0x00665eec
extern int DAT_00665eec;
// 0x00665f48
extern unsigned int DAT_00665f48;
// 0x00665f5c
extern void *DAT_00665f5c;
// 0x00665f60
extern void *DAT_00665f60;
// 0x00665f64
extern unsigned int DAT_00665f64;
// 0x00665f68
extern unsigned int DAT_00665f68;
// 0x00665f6c
extern unsigned int DAT_00665f6c;
// 0x00665fe8
extern unsigned int DAT_00665fe8;
// 0x00665fec
extern unsigned int DAT_00665fec;
// 0x00665ff8
extern unsigned int ReportFlags;
// 0x00665ffc
extern unsigned int DAT_00665ffc;
// 0x00666000
extern unsigned int DAT_00666000;
// 0x00666004
extern unsigned int DAT_00666004;
// 0x00666008
extern unsigned int DAT_00666008;
// 0x0066600c
extern unsigned int DAT_0066600c;
// 0x00666010
extern unsigned int DAT_00666010;
// 0x00666014
extern unsigned int DAT_00666014;
// 0x00666018
extern unsigned int DAT_00666018;
// 0x0066601c
extern unsigned int DAT_0066601c;
// 0x00666020
extern unsigned int DAT_00666020;
// 0x00666024
extern unsigned int DAT_00666024;
// 0x00666028
extern unsigned int DAT_00666028;
// 0x0066602c
extern unsigned int DAT_0066602c;
// 0x00666030
extern unsigned int DAT_00666030;
// 0x00666034
extern unsigned int DAT_00666034;
// 0x00666038
extern unsigned int DAT_00666038;
// 0x0066603c
extern unsigned int DAT_0066603c;
// 0x00666040
extern unsigned int DAT_00666040;
// 0x00666044
extern unsigned int DAT_00666044;
// 0x00666048
extern unsigned int DAT_00666048;
// 0x0066604c
extern unsigned int DAT_0066604c;
// 0x00666050
extern unsigned int DAT_00666050;
// 0x00666054
extern unsigned int DAT_00666054;
// 0x00666058
extern unsigned int DAT_00666058;
// 0x0066605c
extern unsigned int DAT_0066605c;
// 0x00666060
extern unsigned int DAT_00666060;
// 0x00666064
extern unsigned int DAT_00666064;
// 0x00666068
extern unsigned int DAT_00666068;
// 0x0066606c
extern unsigned int DAT_0066606c;
// 0x00666070
extern unsigned int DAT_00666070;
// 0x00666074
extern unsigned int DAT_00666074;
// 0x00666078
extern unsigned int DAT_00666078;
// 0x0066607c
extern unsigned int DAT_0066607c;
// 0x00666080
extern unsigned int DAT_00666080;
// 0x00666084
extern unsigned int DAT_00666084;
// 0x00666088
extern unsigned int DAT_00666088;
// 0x0066608c
extern unsigned int DAT_0066608c;
// 0x00666090
extern unsigned int DAT_00666090;
// 0x00666094
extern unsigned int DAT_00666094;
// 0x00666098
extern unsigned int AppraisalDeadline;
// 0x0066609c
extern int DAT_0066609c;
// 0x006660a0
extern int DAT_006660a0;
// 0x006660a4
extern int DAT_006660a4;
// 0x006660a8
extern struct IconNode *DAT_006660a8;
// 0x006660ac
extern struct IconNode *DAT_006660ac;
// 0x006660b0
extern char DAT_006660b0[256];
// 0x006661bc
extern int DAT_006661bc;
// 0x006661c0
extern struct Element *SharkCafeBrollyElem;
// 0x006661c4
extern struct Element *Entrance1Elem;
// 0x006661c8
extern int DAT_006661c8;
// 0x006661cc
extern char DAT_006661cc[8][100];
// 0x006664ec
extern int DAT_006664ec;
// 0x006664f8
extern struct BuildObj BuildObjArray[256]; /* objects under construction */
// 0x006670f8
extern int BuildObjCount;
// 0x006670fc
extern unsigned int ButtonRepeatDelay;
// 0x00667104
extern unsigned int ControllersInitialized;
// 0x00667108
extern unsigned int DAT_00667108;
// 0x0066710c
extern unsigned int ButtonRepeatLastTick;
// 0x00667114
extern unsigned int FountainSFXRefCount;
// 0x00667118
extern unsigned int PowerStationSFXRefCount;
// 0x0066711c
extern unsigned int DinoSFXRefCount;
// 0x00667120
extern unsigned int MoneySFXLoadCount;
// 0x00667128
extern char LogBuffer[512];
// 0x0066752c
extern char ProductVersionString[0x88];
// 0x006675b4
extern unsigned int BubbleHelpGFXLoaded;
// 0x006675b8
extern volatile int TextCellCount;
// 0x006675c0
extern struct TextCell TextCells[50];
// 0x00667c00
extern int MapWorldMinX;
// 0x00667c04
extern int DAT_00667c04;
// 0x00667c08
extern int DAT_00667c08;
// 0x00667c0c
extern int DAT_00667c0c;
// 0x00667c14
extern short DAT_00667c14;
// 0x00667c16
extern short DAT_00667c16;
// 0x00667c18
extern int MapWorldHeight;
// 0x00667c1c
extern int MapWorldWidth;
// 0x00667c20
extern int MapCenterOffsetY;
// 0x00667c10
extern unsigned int DAT_00667c10;
// 0x00667c28
extern unsigned int DAT_00667c28;
// 0x00667c2c
extern struct Sprite *FullMapSprite;
// 0x00667c30
extern unsigned int DAT_00667c30;
// 0x00667c34
extern struct Sprite *MapSpannerSprite;
// 0x00667c3c
extern int DAT_00667c3c;
// 0x00667c54
extern LEGO_EXPORT TileId QueryObj;
// 0x00667c58
extern LEGO_EXPORT struct ObjClass *QueryClass;
// 0x00667c5c
extern unsigned int DAT_00667c5c;
// 0x00667c60
extern unsigned int DAT_00667c60;
// 0x00667c68
extern unsigned int DAT_00667c68;
// 0x00667c70
extern long DAT_00667c70;
// 0x00667c74
extern long DAT_00667c74;
// 0x00667c64
extern const char *DAT_00667c40;
// 0x00667c48
extern unsigned int DAT_00667c48;
// 0x00667c4c
extern int DAT_00667c4c;
extern unsigned int DAT_00667c64;
extern unsigned int SamplesPaused;
// 0x00667c7c
extern unsigned int MapLoaded;
// 0x00667c80
extern unsigned int DAT_00667c80;
// 0x00667c88
extern struct Sprite *Arrow02Sprite;
// 0x00667c8c
extern struct Sprite *Arrow01Sprite;
// 0x00667c90
extern struct Sprite *Arrow04Sprite;
// 0x00667c94
extern struct Sprite *Arrow03Sprite;
// 0x00667c9c
extern void *GameMapRawBlock;
// 0x00667ca0
extern unsigned int LoadInProgress;
// 0x00667ca4
extern unsigned int DAT_00667ca4;
// 0x00667ca8
extern LEGO_EXPORT void *OverlayList;
// 0x00667cac
extern LEGO_EXPORT unsigned int OverlayILF;
// 0x00667cb0
extern void *BridgesData;
// 0x00667cb4
extern LEGO_EXPORT int ScrollX;
// 0x00667cb8
extern LEGO_EXPORT int ScrollY;
// 0x00667cbc
extern LEGO_EXPORT int ScrollSpeedX;
// 0x00667cc0
extern LEGO_EXPORT int ScrollSpeedY;
// 0x00667cc4
extern int DAT_00667cc4;
// 0x00667cd0
extern int DAT_00667cd0;
// 0x00667cd4
extern int DAT_00667cd4;
// 0x00667cd8
extern unsigned int DAT_00667cd8;
// 0x00667cdc
extern unsigned int DAT_00667cdc;
// 0x00667ce0
extern int DAT_00667ce0;
// 0x00667ce4
extern int DAT_00667ce4;
// 0x00667ce8
extern int DAT_00667ce8;
// 0x00667cec
extern int DAT_00667cec;
// 0x00667cf0
extern int DAT_00667cf0;
// 0x00667cf4
extern int DAT_00667cf4;
// 0x00667cf8
extern int DAT_00667cf8;
// 0x00667cfc
extern int DAT_00667cfc;
// 0x00667d00
extern int DAT_00667d00;
// 0x00667d04
extern int DAT_00667d04;
// 0x00667d08
extern int DAT_00667d08;
// 0x00667d0c
extern int DAT_00667d0c;
// 0x00667d10
extern unsigned int DAT_00667d10;
// 0x00667d3c
extern int DAT_00667d3c;
// 0x00667d40
extern unsigned int DAT_00667d40;
// 0x00667d44
extern int DAT_00667d44;
// 0x00667d48
extern unsigned int DAT_00667d48;
// 0x00667d4c
extern unsigned int DAT_00667d4c;
// 0x00667d50
extern unsigned int MapDataLoaded;
// 0x00667d54
extern unsigned int DAT_00667d54;
// 0x00667d58
extern int DAT_00667d58;
// 0x00667d60
extern int LastWatchDrawTicks;
// 0x00667d68
extern unsigned int LastRenderingCompleteTick;
// 0x00667d6c
extern int WinDebugMode;
// 0x00667d70
/* DX6 DDCAPS is 0x17c; this SDK's DX5 DDCAPS is 0x16c, so each caps block has a four-DWORD DDSCAPS2 tail. */
struct DDRAWENV {
    /* 0x000 */ LPDIRECTDRAW ddraw;
    /* 0x004 */ LPDIRECTDRAW2 ddraw2;
    /* 0x008 */ DDCAPS caps;
    /* 0x174 */ unsigned int ddsCaps[4];
    /* 0x184 */ DDCAPS hel_caps;
    /* 0x2f0 */ unsigned int hel_ddsCaps[4];
    /* 0x300 */ unsigned int unk[54];
};
extern LEGO_EXPORT struct DDRAWENV DDRAWENV;
// 0x00668070
extern LPDIRECTDRAWSURFACE PrimarySurface;
// 0x00668074
extern LPDIRECTDRAWSURFACE DAT_00668074;
// 0x00668078
extern LPDIRECTDRAWSURFACE OffscreenSurface;
// 0x0066807c
extern LPDIRECTDRAWSURFACE renderEngine;
// 0x00668080
extern LPDIRECTDRAWCLIPPER DDrawClipper;
// 0x00668084
extern unsigned int DDPalette;
// 0x00668088
extern unsigned int DisplayPixelFormat;
// 0x0066808c
extern HFONT LegoFont20Bold;
// 0x00668090
extern HFONT LegoFont24Bold;
// 0x00668094
extern HFONT LegoFont18SemiBold;
// 0x00668098
extern HFONT LegoFont28Normal;
// 0x0066809c
extern DDSURFACEDESC CurrentSurfaceDesc;
// 0x00668108
extern RECT DAT_00668108;
// 0x00668118
extern int renderEngineTargetIdx;
// 0x0066811c
extern LPDIRECTDRAWSURFACE renderEngineTargets[10];
// 0x00668144
extern int VideoSurfaceLocked;
// 0x00668148
extern struct Sprite *DAT_00668148;
// 0x0066814c
extern unsigned int DAT_0066814c;
// 0x00668164
extern int DAT_00668164[32];
// 0x006681e4
extern int RenderingStatusStackDepth;
// 0x006681e8
extern unsigned int OverridePalette;
// 0x006681ec
extern void *DAT_006681ec;
// 0x006681f0
extern unsigned int FpsWindowStartTicks;
// 0x006681f4
extern unsigned int LastFrameTicks;
// 0x006681f8
extern int FramesThisSecond;
// 0x006681fc
extern LEGO_EXPORT unsigned int LastFrameMS;
// 0x00668200
extern unsigned int LastPresentTicks;
// 0x00668204
extern unsigned int WatchActive;
// 0x00668208
extern struct Sprite *WatchSprite;
// 0x0066820c
extern char DAT_0066820c[0x404];
// 0x00668610
extern unsigned int DAT_00668610;
// 0x00668614
extern unsigned int DAT_00668614;
// 0x00668618
extern unsigned int DAT_00668618;
// 0x0066861c
extern char DAT_0066861c[128];
// 0x0066869c
extern char DAT_0066869c[128];
// 0x0066871c
extern unsigned int DAT_0066871c;
// 0x00668720
extern unsigned int ScriptStringCount;
// 0x00668724
extern struct ObjectiveEvent *DAT_00668724;
// 0x00668728
extern struct ObjectiveEvent *ObjectiveEventList;
// 0x0066872c
extern unsigned int DAT_0066872c[0x15];
// 0x00668780
extern unsigned int DAT_00668780;
// 0x00668784
extern void *DAT_00668784;
// 0x00668788
extern unsigned int DAT_00668788;
// 0x0066878c
extern int DAT_0066878c;
// 0x00668790
extern unsigned int DAT_00668790;
// 0x00668794
extern unsigned int DAT_00668794;
// 0x00668798
extern void *DAT_00668798;
// 0x0066879c
extern unsigned int DAT_0066879c;
// 0x006687a0
extern unsigned int ScriptLoadErrorCount;
// 0x006687a4
extern unsigned int DAT_006687a4;
// 0x006687a8
extern unsigned int DAT_006687a8;
// 0x006687ac
extern unsigned int DAT_006687ac;
// 0x006687b0
extern unsigned int DAT_006687b0;
// 0x006687b4
extern unsigned int DAT_006687b4;
// 0x006687bc
extern unsigned int DAT_006687bc;
// 0x006687c0
extern unsigned int DAT_006687c0;
// 0x006687c8
extern struct IconNode *IconListHead;
// 0x006687cc
extern struct IconNode *DAT_006687cc;
// 0x006687d0
extern LEGO_EXPORT void *FocussedIconPtr;
// 0x00668828
extern struct Sprite *GBarFrameSprite;
// 0x0066882c
extern struct Sprite *IfSideBUpSprite;
// 0x00668830
extern struct Sprite *IfSideBDownSprite;
// 0x00668834
extern struct Sprite *IfSidebar1Sprite;
// 0x00668840
extern short DAT_00668840;
// 0x0066884c
extern short DAT_0066884c;
// 0x00668858
extern struct IconNode DAT_00668858;
// 0x006688a8
extern void *DAT_006688a8;
// 0x006688ac
extern void *DAT_006688ac;
// 0x006688b0
extern void *DAT_006688b0;
// 0x006688b4
extern unsigned int LastScrollIconTick;
// 0x006688b8
extern unsigned int DAT_006688b8;
// 0x006688bc
extern int PowerDemandBarPos;
// 0x006688c0
extern int EnergyBarSupplyWidth;
// 0x006688c4
extern int DAT_006688c4;
// 0x006688c8
extern unsigned int RenderIconsLastTicks;
// 0x006688cc
extern unsigned int DAT_006688cc;
// 0x006688d0
extern unsigned int GBarSpritesLoaded;
// 0x006688d4
extern void *DAT_006688d4;
// 0x006688d8
extern void *ActiveIndicators;
// 0x006688e0
extern struct Sprite *PuBgMainSprite;
// 0x006688e4
extern struct Sprite *PuBgCentreTopSprite;
// 0x006688e8
extern struct Sprite *PuBgRightTopSprite;
// 0x006688ec
extern struct Sprite *PuBgLeftMidSprite;
// 0x006688f0
extern struct Sprite *PuBgCentreMidSprite;
// 0x006688f4
extern struct Sprite *PuBgRightMidSprite;
// 0x006688f8
extern struct Sprite *PuBgLeftBtmSprite;
// 0x006688fc
extern struct Sprite *PuBgCentreBtmSprite;
// 0x00668900
extern struct Sprite *PuBgRightBtmSprite;
// 0x00668904
extern struct Sprite *CBBGLeftSprite;
// 0x00668908
extern struct Sprite *CBBGCentreSprite;
// 0x0066890c
extern struct Sprite *CBBGRightSprite;
// 0x00668910
extern struct Sprite *NewPopMockSprite;
// 0x00668914
extern struct Sprite *PuDeleteObjectOnSprite;
// 0x00668918
extern struct Sprite *PuDeleteObjectSprite;
// 0x0066891c
extern struct Sprite *PuClosePopUpOnSprite;
// 0x00668920
extern struct Sprite *PuClosePopUpSprite;
// 0x00668924
extern struct Sprite *NextIconOnSprite;
// 0x00668928
extern struct Sprite *NextIconSprite;
// 0x0066892c
extern struct Sprite *PrevIconOnSprite;
// 0x00668930
extern struct Sprite *PrevIconSprite;
// 0x00668934
extern struct Sprite *PUOKOnSprite;
// 0x00668938
extern struct Sprite *PUOKSprite;
// 0x0066893c
extern struct Sprite *CBCloseSprite;
// 0x00668940
extern struct Sprite *CBCloseOnSprite;
// 0x00668944
extern struct Sprite *PuAddGardenerOnSprite;
// 0x00668948
extern struct Sprite *PuAddGardenerSprite;
// 0x0066894c
extern struct Sprite *PuAddMechanicsOnSprite;
// 0x00668950
extern struct Sprite *PuAddMechanicsSprite;
// 0x00668954
extern unsigned int DAT_00668954;
// 0x00668958
extern struct Sprite *PopUpSpritesLoaded;
// 0x0066895c
extern int DAT_0066895c;
// 0x00668960
extern unsigned int DAT_00668960;
// 0x00668964
extern int DAT_00668964;
// 0x00668968
extern char DAT_00668968[0x400];
// 0x00668d68
extern unsigned int DAT_00668d68;
// 0x00668d78  (DIMOUSESTATE; full type in <dinput.h>, dereferenced only by input.c)
extern struct _DIMOUSESTATE MouseState;
// 0x00668d88
extern void *dinput;
// 0x00668d8c
extern void *dinput_keyboard;
// 0x00668d90
extern void *dintput_mouse;
// 0x00668d94
extern char CheatKeyBuffer[0x14];
// 0x00668da8
extern unsigned char DAT_00668da8[0x3b];
// 0x00668de4
extern unsigned char DAT_00668de4[0x3b];
// 0x00668e20
extern unsigned int DAT_00668e20[4];
// 0x00668e34
extern unsigned int DAT_00668e34;
// 0x00668e38
extern unsigned int DAT_00668e38;
// 0x00668e3c
extern void *DAT_00668e3c;
// 0x00668e40
extern struct InterfaceListNode *DAT_00668e40;
// 0x00668e44
extern int DAT_00668e44[8];
// 0x00668e64
extern unsigned char DAT_00668e64;
// 0x00668e68
extern struct Sprite *InterfaceBgSprite;
// 0x00668e6c
extern struct Sprite *NoEnergySprite;
// 0x00668e70
extern struct Sprite *BarPointerSprite;
// 0x00668e74
extern struct Sprite *AttractHighlightOnSprite;
// 0x00668e78
extern struct Sprite *AttractHighlightOffSprite;
// 0x00668e7c
extern struct Sprite *AttractNewOffSprite;
// 0x00668e80
extern struct Sprite *AttractNewOnSprite;
// 0x00668e84
extern struct Sprite *SideScrollDownLitSprite;
// 0x00668e88
extern struct Sprite *SideScrollUpLitSprite;
// 0x00668e8c
extern struct Sprite *LinkMiddleSprite;
// 0x00668e90
extern struct Sprite *LinkBottomSprite;
// 0x00668e94
extern struct Sprite *BriefIcon2Sprite;
// 0x00668e98
extern struct Sprite *BriefIconSprite;
// 0x00668e9c
extern void *DAT_00668e9c;
// 0x00668ea0
extern struct Sprite *ScriptEndSprite;
// 0x00668ea4
extern unsigned int DAT_00668ea4;
// 0x00668eb0
extern unsigned int SelectedThemeIcon;
// 0x00668eb4
extern unsigned int DAT_00668eb4;
// 0x00668eb8
extern unsigned int ScriptEndIcon;
// 0x00668ebc
extern unsigned int DAT_00668ebc;
extern unsigned int DAT_00668ec0;
// 0x00668ed8
extern struct InterfaceResearchNode *ResearchList;
// 0x00668ee0
extern int DAT_00668ee0;
// 0x00668ee8
extern unsigned int AviAcmStreamHeader[0x15];
// 0x00668f3c
extern unsigned int AviSoundBytesPerFrame;
// 0x00668f44
extern int AcmStreamOpenResult;
// 0x00668f48
extern void *AviSoundBuffer;
// 0x00668f4c
extern unsigned int DAT_00668f4c;
// 0x00668f50
extern unsigned int DAT_00668f50;
// 0x00668f54
extern int DAT_00668f54;
// 0x00668f58
extern unsigned int DAT_00668f58;
// 0x00668f5c
extern void *AviAcmStream;
// 0x00668f60
extern int AviAudioSamplePos;
// 0x00668f64
extern unsigned int AviAudioSampleSize;
// 0x00668f70
extern WAVEFORMATEX AviPcmFormat;
// 0x00668f84
extern void *AviAudioStream;
// 0x00668f88
extern int DAT_00668f88;
// 0x00668f8c
extern void *AviAcmDstBase;
// 0x00668f90
extern unsigned int DAT_00668f90;
// 0x00668f9c
extern int DAT_00668f9c;
// 0x00668fa0
extern unsigned int DAT_00668fa0;
extern unsigned int AviOpenCount;
// 0x00668fa4
extern unsigned int DAT_00668fa4;
// 0x00668fac
extern unsigned int PerfCounterState;
// 0x00668fb0
extern unsigned int DAT_00668fb0;
// 0x00668fb4
extern unsigned int DAT_00668fb4;
// 0x00668fb8
extern struct MoviePool *DAT_00668fb8;
// 0x00668fc0
extern void *DAT_00668fc0;
// 0x00668fc4
extern struct InterfaceQueryNode *DAT_00668fc4;
// 0x00668fcc
extern int DAT_00668fcc;
// 0x00668fd0
extern char DAT_00668fd0[128];
// 0x00669050
extern unsigned char CurrentObjectiveEventFlags;
// 0x00669054
extern unsigned int DAT_00669054;
// 0x00669058
extern char DAT_00669058[0x40];
// 0x00669098
extern unsigned int DAT_00669098;
// 0x004bc0ec
extern char LanguageName[36];
// 0x006691a0
extern unsigned int LLIDB_Capacity;
// 0x006691a4
extern unsigned int LLIDB_ElementCount;
// 0x006691a8
extern struct Element **LLIDB_Pages;
// 0x006691ac
extern struct LLSNode *LLSPlayList;
// 0x006691b0
extern int SaveFileHandle;
// 0x006691b4
extern int SavedElementCount;
// 0x006691bc
extern int MeasuredBlockStarts[16];
// 0x006691fc
extern int MeasuredBlockDepth;
// 0x00669200
extern struct Element **SavedElementTable;
// 0x00669204
extern unsigned int GameTimerMark;
// 0x00669208
extern void *g_hInstance;
// 0x0066920c
extern int g_nCmdShow;
// 0x00669210
extern HWND WndEnvHwnd;
// 0x00669238
extern unsigned int PauseGameTimerResult;
// 0x00669240
extern LEGO_EXPORT struct Ride *ObjectClassList;
// 0x00669244
struct LibraryNode;
extern struct LibraryNode *ObjectLibraryHead;
// 0x00669248
extern void *ClassRideList;
// 0x0066924c
extern unsigned int DAT_0066924c;
// 0x00669250
extern int DAT_00669250;
// 0x00669254
extern unsigned int DAT_00669254;
// 0x004bcec0
extern struct Point DAT_004bcec0;
// 0x00669258
extern unsigned int DAT_00669258[1152];
// 0x0066a45c
extern void *DAT_0066a45c[1020];
// 0x0066b44c
extern void *DAT_0066b44c;
// 0x0066b450
extern struct DirNode *DirSearchNodeList;
// 0x0066b454
extern struct DirNode *DAT_0066b454;
// 0x0066b458
extern struct DirNode *DAT_0066b458;
// 0x0066b460
extern struct Point Entrance1Point;
// 0x0066b468
extern int LastPathUpdateTime;
// 0x0066b46c
extern unsigned int PathUpdateNeeded;
// 0x0066b470
extern char VisitorNameBuffer[0x104];
// 0x0066b574
extern LEGO_EXPORT struct Bloke *FirstBloke;
// 0x0066b57c
extern struct Bloke *BlokePool; /* bloke pool */
// 0x0066b580
extern int DirPickCounts[9];
// 0x0066b5a4
extern struct SortNode *PrintListHead;
// 0x0066b5a8
extern unsigned int PrintListBytesUsed;
// 0x0066b5ac
extern unsigned int DAT_0066b5ac;
// 0x0066b5b0
extern DDSURFACEDESC DAT_0066b5b0;
// 0x0066b61c
extern unsigned int DAT_0066b61c;
// 0x0066b620
extern int DAT_0066b620;
// 0x0066b624
extern int DAT_0066b624;
// 0x0066b628
extern int DAT_0066b628;
// 0x0066b62c
extern int DAT_0066b62c;
// 0x0066b630
extern void *DAT_0066b630;
// 0x0066be40
extern unsigned int DAT_0066be40;
// 0x0066be44
extern unsigned int DAT_0066be44;
// 0x0066be48
extern unsigned int DAT_0066be48;
// 0x0066be4c
extern unsigned int DAT_0066be4c;
// 0x0066be50
extern struct Image *DAT_0066be50;
// 0x0066be54
extern unsigned short DAT_0066be54[0x4b002];
// 0x00701e58
extern unsigned int DAT_00701e58;
// 0x00701e5c
extern unsigned char *DAT_00701e5c;
// 0x00701e60
extern unsigned int DAT_00701e60;
// 0x00701e64
extern struct Sprite *DAT_00701e64;
// 0x00701e68
extern unsigned short DAT_00701e68[0x4b000];
// 0x00797e68
extern unsigned int DAT_00797e68;
// 0x00797e6c
extern void *DAT_00797e6c;
// 0x00797e70
extern float DAT_00797e70[100];
// 0x00798000
extern int DAT_00798000[100];
// 0x00798190
extern void *DAT_00798190[256];
// 0x00798590
extern void *DAT_00798590;
// 0x00798598
extern DDSURFACEDESC DAT_00798598;
// 0x00798608
extern RECT DAT_00798608;
// 0x0079861c
extern LPDIRECTDRAWSURFACE DAT_0079861c;
// 0x00798620
extern int DAT_00798620;
// 0x00798624
extern void *MasterDirList;
// 0x00798628
extern void *MasterVolList;
// 0x0079862c
extern void *DAT_0079862c;
// 0x00798630
extern unsigned int SavedClipLeft;
// 0x00798634
extern unsigned int SavedClipTop;
// 0x00798638
extern unsigned int SavedClipRight;
// 0x0079863c
extern unsigned int SavedClipBottom;
// 0x00798648
extern unsigned int DAT_00798648;
// 0x0079864c
extern struct IconNode *DAT_0079864c;
// 0x00798650
extern unsigned int DAT_00798650;
// 0x00798660
extern unsigned int DAT_00798660;
// 0x00798664
extern unsigned int DAT_00798664;
// 0x00798668
extern unsigned int DAT_00798668;
extern unsigned long DAT_0079866c;
// 0x00798674
extern struct Sprite *PuOkOnSprite;
// 0x00798678
extern struct Sprite *PuOkSprite;
// 0x0079867c
extern struct Sprite *PopUpCloseSprite;
// 0x00798680
extern struct Sprite *PuCloseOnSprite;
// 0x00798684
extern struct Sprite *ClosePopUpSprite;
// 0x00798688
extern struct Sprite *ClosePopUpOnSprite;
// 0x0079868c
extern struct Sprite *RegDeleteOnSprite;
// 0x00798690
extern struct Sprite *RegDeleteSprite;
// 0x00798694
extern struct Sprite *RegProfileOff1Sprite;
// 0x00798698
extern struct Sprite *RegProfileOff2Sprite;
// 0x0079869c
extern struct Sprite *RegProfileOff3Sprite;
// 0x007986a0
extern struct Sprite *RegProfileOff4Sprite;
// 0x007986a4
extern struct Sprite *RegProfileOff5Sprite;
// 0x007986a8
extern struct Sprite *RegProfileOff6Sprite;
// 0x007986ac
extern struct Sprite *RegProfileOff7Sprite;
// 0x007986b0
extern struct Sprite *RegProfileOff8Sprite;
// 0x007986b4
extern struct Sprite *RegProfileOnSprite;
// 0x007986b8
extern struct Sprite *DAT_007986b8;
// 0x007986bc
extern struct Sprite *RegDiffPopUpSprite;
// 0x007986c0
extern struct Sprite *RegEasyOnSprite;
// 0x007986c4
extern struct Sprite *RegEasyOffSprite;
// 0x007986c8
extern struct Sprite *RegMidOnSprite;
// 0x007986cc
extern struct Sprite *RegMidOffSprite;
// 0x007986d0
extern struct Sprite *RegHardOnSprite;
// 0x007986d4
extern struct Sprite *RegHardOffSprite;
// 0x007986d8
extern struct IconNode *PopUpOkIcon;
// 0x007986dc
extern struct IconNode *PopUpCloseIcon;
// 0x007986e0
extern unsigned int AcceptIcon;
// 0x007986e4
extern unsigned int DeletePopUpShown;
// 0x007986e8
extern unsigned int NewProfilePopUpShown;
// 0x007986f0
extern unsigned int DAT_007986f0;
// 0x007986f4
extern unsigned int DAT_007986f4;
// 0x007986f8
extern unsigned int DAT_007986f8;
// 0x00798700
extern unsigned int DAT_00798700;
// 0x00798704
extern struct Sprite *RegSaveSlotOnSprite;
// 0x00798708
extern struct Sprite *RegSaveOff1Sprite;
// 0x0079870c
extern struct Sprite *RegSaveOff2Sprite;
// 0x00798710
extern struct Sprite *RegSaveOff3Sprite;
// 0x00798714
extern struct Sprite *RegSaveOff4Sprite;
// 0x00798718
extern struct Sprite *RegSaveOff5Sprite;
// 0x0079871c
extern struct Sprite *RegSaveOff6Sprite;
// 0x00798720
extern struct Sprite *RegSaveOff7Sprite;
// 0x00798724
extern struct Sprite *RegSaveOff8Sprite;
// 0x00798728
extern struct Sprite *SaveTypeNormalSprite;
// 0x0079872c
extern struct Sprite *SaveTypeFreeSprite;
// 0x00798730
extern struct Sprite *RegCornerMaskSprite;
// 0x00798734
extern void *SavedGameList;
// 0x00798738
extern int DAT_00798738;
// 0x0079873c
extern unsigned int DAT_0079873c;
// 0x00798740
extern unsigned int DAT_00798740;
// 0x00798748
extern struct IconNode *SpeechVolumeMarkerIcon;
// 0x0079874c
extern struct IconNode *FxVolumeMarkerIcon;
// 0x00798750
extern struct IconNode *MusicVolumeMarkerIcon;
// 0x00798754
extern unsigned int DAT_00798754;
// 0x00798764
extern struct Sprite *PrintInfoSprite;
// 0x00798768
extern int CertificatePrintResultTimer;
// 0x0079876c
extern unsigned int DAT_0079876c;
// 0x00798770
extern unsigned int DAT_00798770;
// 0x00798777
extern char DAT_00798777;
// 0x00798778
extern char DAT_00798778[0x100];
// 0x00798878
extern unsigned int DAT_00798878;
// 0x0079887c
extern unsigned int DAT_0079887c;
// 0x00798880
extern unsigned int DAT_00798880;
// 0x00798884
extern unsigned int DAT_00798884;
// 0x00798888
extern unsigned int DAT_00798888;
// 0x00798890
extern void *ProfileListHead;
// 0x00798894
extern int DAT_00798894;
// 0x00798898
extern unsigned int AviLockBytes1;
// 0x0079889c
extern unsigned int AviLockBytes2;
// 0x007988a0
extern unsigned int DAT_007988a0;
// 0x007988a4
extern void *AviLockPtr1;
// 0x007988a8
extern void *AviLockPtr2;
// 0x007988b0
extern void *SoundHwnd;
// 0x007988bc
extern unsigned int DAT_007988bc;
// 0x007988c0
extern unsigned int SoundAvailable;
// 0x007988c4
extern unsigned int SfxMuted;
// 0x007988c8
extern unsigned int DAT_007988c8;
// 0x007988cc
extern void *SampleListHead;
// 0x0079a694
extern LEGO_EXPORT unsigned int DMusicInitialised;
// 0x0079a698
extern void *MusicThread;
// 0x0079a6a0
extern void *MusicCommandEvent;
// 0x0079a6a4
extern unsigned int MusicCommand;
// 0x0079a6a8
extern int NextMusicTheme;
// 0x0079a6ac
extern int CurrentMusicTheme;
// 0x0079a7d0
extern unsigned int SpeechVolume;
// 0x0079a7d8
extern int SpeechRawReadPos;
// 0x0079a7dc
extern int SpeechRawWritePos;
// 0x0079a7e0
extern unsigned int SpeechChunkIndex;
// 0x0079a7e4
extern unsigned int SpeechChunkByteCounts[20];
// 0x0079a834
extern int SpeechPcmReadPos;
// 0x0079a838
extern int SpeechPcmWritePos;
// 0x0079a83c
extern unsigned int SpeechFlags;
// 0x0079a840
extern int SpeechPageIndex;
// 0x0079a844
extern int DAT_0079a844;
// 0x0079a848
extern void *SpeechSoundBuffer;
// 0x0079a84c
extern unsigned int SpeechState;
// 0x0079a850
extern void *strings[10];
// 0x0079a878
extern unsigned int DAT_0079a878[6];
// 0x0079a890
extern unsigned int GameTimerPaused;
// 0x0079a894
extern unsigned int GameTimerPausedTicks;
// 0x0079a898
extern unsigned int DAT_0079a898;
// 0x0079a89c
extern unsigned int GameTimerPausedFrame;
// 0x0079a8a0
extern unsigned int DAT_0079a8a0;
// 0x0079a8a8
extern LEGO_EXPORT struct Bloke *GardenerList;
// 0x0079a8ac
extern LEGO_EXPORT struct Bloke *MechanicList;
// 0x0079a8b0
extern struct WorkOrder *GardenerOrderHead;
// 0x0079a8b4
extern struct WorkOrder *GardenerOrderTail;
// 0x0079a8b8
extern int DAT_0079a8b8;
// 0x0079a8bc
extern int GardenerCount;
// 0x0079a8c0
extern struct WorkOrder *MechanicOrderHead;
// 0x0079a8c4
extern struct WorkOrder *MechanicOrderTail;
// 0x0079a8c8
extern int DAT_0079a8c8;
// 0x0079a8cc
extern int MechanicCount;
// 0x0079a8d0
extern unsigned int CastlePlacedFlag;
// 0x0079a8d4
extern struct RepairOrder *RepairOrderList;
// 0x0079abfc
extern struct Element *NormalPathTilesElement;
// 0x0079ac04
extern unsigned int SpeechDataSize;
// 0x0079ac08
extern unsigned char *SpeechAcmDstBuffer;
// 0x0079ac0c
extern void *SpeechAcmSrcBuffer;

// 0x0079ac20
extern unsigned char SpeechRawBuffer[0x10000];
// 0x007aac24
extern int SpeechConvertedUsed;
// 0x007aac40
struct AcmHdr {
    unsigned int cbStruct;
    unsigned int fdwStatus;
    unsigned int dwUser;
    unsigned char *pbSrc;
    int cbSrcLength;
    int cbSrcLengthUsed;
    unsigned int dwSrcUser;
    unsigned char *pbDst;
    int cbDstLength;
    int cbDstLengthUsed;
    /* ACMSTREAMHEADER goes on to 0x54 bytes: the ACM driver keeps its state in these while the header is
     * prepared. The original reserves 0x60 bytes here (0x7aac40..0x7aaca0). */
    unsigned int dwDstUser;
    unsigned int dwReservedDriver[10];
    unsigned char pad_54[0x60 - 0x54];
};
extern struct AcmHdr SpeechAcmHeader;

// 0x007aaca0
extern unsigned char SpeechPcmBuffer[0x20000];
// 0x007caca0
extern int SpeechAcmSrcSize;
// 0x007caca4
extern int SpeechConvertedSize;
// 0x007caca8
extern unsigned int SpeechFileHandle;
// 0x007cacac
extern unsigned int SpeechBytesRemaining;
// 0x007cacb0
extern WAVEFORMATEX *SpeechSourceFormat;
// 0x007cacb8
extern void *SpeechAcmStream;
// 0x007cacc0
extern WAVEFORMATEX SpeechPcmFormat;
// 0x007cacb4
extern unsigned int SpeechDataOffset;
// 0x007cacd4
extern LEGO_EXPORT unsigned int FrameNumber;
// 0x007cacd8
extern struct DirectMusicLoader *DMusicLoader;
// 0x007cacdc
extern struct DirectMusicPerformance *DMusicPerformance;
// 0x007cace0
extern unsigned int DSoundCaps[0x18];
// 0x007cad40
extern void *DSound;
// 0x007cad44
extern struct DirectMusicComposer *DMusicComposer;
// 0x007cad48
extern unsigned int MusicThreadId;
// 0x007cad4c
extern void *DMusicSoundBuffer;
// 0x007cad60
extern struct ProfileData TempProfile;
// 0x007cae80
extern char DAT_007cae80[0x100];
// 0x007caf80
extern struct Sprite *RepHint1Sprite;
// 0x007cafa0
extern char *DAT_007cafa0[104];
// 0x007cb140
extern char *DAT_007cb140[32];
// 0x007cb1c0
extern struct IconNode *RepHint1Icon;
// 0x007cb1c4
extern struct Sprite *RepHint2Sprite;
// 0x007cb1e0
extern char DAT_007cb1e0[0x100];
// 0x007cb2e0
extern struct IconNode *DAT_007cb2e0;
// 0x007cb2e4
extern struct IconNode *DAT_007cb2e4;
// 0x007cb2f0
extern char DAT_007cb2f0[0x10];
// 0x007cb300
extern char DAT_007cb300[0xc];
// 0x007cb30c
extern char DAT_007cb30c[4];
// 0x007cb310
extern unsigned int DAT_007cb310;
// 0x007cb314
extern char DAT_007cb314;
// 0x007cb315
extern char DAT_007cb315;
// 0x007cb318
extern unsigned int DAT_007cb318;
// 0x007cb31c
extern char DAT_007cb31c;
// 0x007cb320
extern unsigned int DAT_007cb320;
// 0x007cb324
extern unsigned int DAT_007cb324;
// 0x007cb328
extern unsigned int LoadMode;
// 0x007cb340
extern char DAT_007cb340[0x20];
// 0x007cb360
extern struct IconNode *DeleteIcon;
// 0x007cb380
extern unsigned int DAT_007cb380[5];
// 0x007cb394
extern unsigned int DAT_007cb394;
// 0x007cb398
extern struct Sprite *FreePlayTickSprite;
// 0x007cb39c
extern struct PanelNode *DAT_007cb39c;
// 0x007cb3a0
extern unsigned int DAT_007cb3a0;
// 0x007cb3a4
extern struct PanelNode *DAT_007cb3a4;
// 0x007cb3a8
extern struct Sprite *FreePlayDown4Sprite;
// 0x007cb3ac
extern struct Sprite *FreePlayDown2Sprite;
// 0x007cb3b0
extern struct Sprite *FreePlayDown3Sprite;
// 0x007cb3b4
extern struct Sprite *FreePlayDown1Sprite;
// 0x007cb3b8
extern struct PanelNode *DAT_007cb3b8;
// 0x007cb3bc
extern struct Element *DAT_007cb3bc;
// 0x007cb3c0
extern struct Sprite *FreePlayUp2Sprite;
// 0x007cb3c4
extern struct Sprite *FreePlayUp1Sprite;
// 0x007cb3c8
extern struct Sprite *FreePlayUp4Sprite;
// 0x007cb3cc
extern struct Sprite *FreePlayUp3Sprite;
// 0x007cb3d0
extern struct PanelNode *DAT_007cb3d0;
// 0x007cb3d4
extern struct Sprite *FreePlayCoverSprite;
// 0x007cb3e0
extern struct ObjTableEntry ObjInstanceTable[128];
// 0x007cb5e0
extern struct ObjTableEntry DAT_007cb5e0;
// 0x007cb600
extern unsigned char PrintListPool[0x32000];
// 0x007fd600
extern struct SortNode *SortCursor;
// 0x007fd610
extern struct LibraryNode DAT_007fd610;
// 0x007fd620
extern LEGO_EXPORT void *NewObjectPtr;
// 0x007fd624
extern void *PathControlObject;
// 0x007fd630
extern unsigned int MidiTimerId;
// 0x007fd634
extern void *CurrentMidiFile;
// 0x007fd638
extern unsigned int MidiOutHandle;
// 0x007fd640
extern struct ResVolume *ResourceVolumes[4];
// 0x007fd660
extern unsigned int DAT_007fd660[256];
// 0x007fda60
extern struct BlokeSave BlokeSaveBuffer;
// 0x007fdb84
extern int DAT_007fdb84;
// 0x007fdb88
extern unsigned int SelectElementMask;
// 0x007fdba0
extern char DAT_007fdba0[0x100];

// 0x007fdca0
extern int SelectedElementIndex;
// 0x007fdca4
extern unsigned int DAT_007fdca4;
// 0x007fdca8
extern float PerfCounterScale;
// 0x007fdcc0
extern struct Sprite *LegolandThemeOnSprite;
// 0x007fdcc4
extern struct Sprite *WesternThemeOnSprite;
// 0x007fdcc8
extern struct Sprite *CastleThemeOnSprite;
// 0x007fdccc
extern struct Sprite *AdventurersThemeOnSprite;
// 0x007fdcd0
extern struct Sprite *IfPathIconPressedSprite;
// 0x007fdcd4
extern struct Sprite *IfQueryIconPressedSprite;
// 0x007fdcd8
extern struct Sprite *IfEraserIconPressedSprite;
// 0x007fdcdc
extern struct Sprite *IfMapIconPressedSprite;
// 0x007fdce0
extern struct Sprite *IfOptionsIconPressedSprite;
// 0x007fdd00
extern unsigned int ButtonFlashStates[9];
// 0x007fdd40
extern struct Sprite *LegolandThemeOffSprite;
// 0x007fdd44
extern struct Sprite *WesternThemeOffSprite;
// 0x007fdd48
extern struct Sprite *CastleThemeOffSprite;
// 0x007fdd4c
extern struct Sprite *AdventurersThemeOffSprite;
// 0x007fdd50
extern struct Sprite *IfPathIconSprite;
// 0x007fdd54
extern struct Sprite *IfQueryiconSprite;
// 0x007fdd58
extern struct Sprite *IfEraserIconSprite;
// 0x007fdd5c
extern struct Sprite *IfMapiconSprite;
// 0x007fdd60
extern struct Sprite *IfOptionsIconSprite;
// 0x007fdd70
extern struct InterfaceProfileObj *DAT_007fdd70[4];
// 0x007fdd80
extern unsigned char DAT_007fdd80;
// 0x007fdd84
extern unsigned int DAT_007fdd84;
// 0x007fdd88
extern unsigned int DAT_007fdd88;
// 0x007fdd8c
extern unsigned char DAT_007fdd8c;
// 0x007fdda0
extern unsigned char KeyboardState[256];
// 0x007fdea4
extern struct IconNode *AddMechanicsIcon;
// 0x007fdea8
extern struct IconNode *PopUpInfoOkIcon;
// 0x007fdeac
extern struct Sprite *IFullSprite;
// 0x007fdeb0
extern struct Sprite *ObjectNoRepair1Sprite;
// 0x007fdecc
extern int PopUpInfoX;
// 0x007fded0
extern int PopUpInfoY;
// 0x007fded4
extern struct NewObjTable NewObjects;
// 0x007fdf7c
extern unsigned int DAT_007fdf7c;
// 0x007fdf80
extern struct InfoObjData *DAT_007fdf80;
// 0x007fdf84
extern unsigned char *DAT_007fdf84;
// 0x007fdf88
extern unsigned short DAT_007fdf88;
// 0x007fdf8c
extern void *DAT_007fdf8c; /* object the info popup is showing */
// 0x007fdf90
extern unsigned int DAT_007fdf90;
// 0x007fdf94
extern unsigned int DAT_007fdf94;
// 0x007fdf98
extern unsigned int DAT_007fdf98;
// 0x007fdf9c
extern int DAT_007fdf9c;
// 0x007fdfa0
extern unsigned int DAT_007fdfa0;
// 0x007fdfa4
extern unsigned int DAT_007fdfa4;
// 0x007fdfa8
extern unsigned int DAT_007fdfa8;
// 0x007fdfac
extern unsigned char PopupInfoLineCount;
// 0x007fdfb0
extern unsigned int PottingShedHandle;
// 0x007fdfb4
extern unsigned int MechanicsHutHandle;
// 0x007fdfb8
extern unsigned int PathControlHandle;
// 0x007fdfbc
extern unsigned int Entrance1Handle;
// 0x007fdfc0
extern struct IconNode *ClosePopUpIcon;
// 0x007fdfc4
extern struct IconNode *NextPopUpIcon;
// 0x007fdfc8
extern struct Sprite *ISadSprite;
// 0x007fdfcc
extern struct IconNode *DAT_007fdfcc;
// 0x007fdfd0
extern struct Sprite *IHungrySprite;
// 0x007fdfd8
extern struct IconNode *PuCornerMaskIcon;
// 0x007fdfdc
extern struct IconNode *DeleteObjectIcon;
// 0x007fdfe0
extern struct IconNode *AddGardenerIcon;
// 0x007fdfe4
extern struct Sprite *INormSprite;
// 0x007fdfe8
extern struct IconNode *PrevPopUpIcon;
// 0x007fdff0
extern struct Bloke *WorkerOnMouse;
// 0x007fdff4
extern int WorkerOldX;
// 0x007fdff8
extern int WorkerOldY;
// 0x007fdffc
extern unsigned int WorkerOnMouseType;
// 0x007fe000
extern struct IconNode *CBCloseIcon;
// 0x007fe004
extern struct Sprite *ObjectRepairOkSprite;
// 0x007fe008
extern struct Sprite *IPeckishSprite;
// 0x007fe010
extern int DAT_007fe010;
// 0x007fe014
extern int DAT_007fe014;
// 0x007fe018
extern struct Sprite *IHappySprite;
// 0x007fe020
extern RECT ScreenRect;
// 0x007fe040
extern unsigned int DAT_007fe040;
// 0x007fe044
extern unsigned int DAT_007fe044;
// 0x007fe048
extern char *AdvisorHelpText;
// 0x007fe04c
extern unsigned int AdvisorHelpStartTime;
// 0x007fe050
extern unsigned int DAT_007fe050;
// 0x007fe054
extern unsigned int DAT_007fe054;
// 0x007fe114
extern unsigned char LegolandCommonThemeCount;
// 0x007fe115
extern unsigned char WesternThemeCount;
// 0x007fe116
extern unsigned char CastleThemeCount;
// 0x007fe117
extern unsigned char AdventurersThemeCount;
// 0x007fe120
extern unsigned int ScriptStringTable[256];
// 0x007fe920
extern unsigned int DAT_007fe920;
// 0x007fe930
extern unsigned char ObjectiveCounters[10];
// 0x007fe994
extern unsigned int DAT_007fe994;
// 0x007fe998
extern unsigned int DAT_007fe998;
// 0x007fe9a8
extern unsigned int DAT_007fe9a8;
// 0x007fe9c0
extern struct Sprite *PointerSprites[9];
// 0x007fea30
extern RECT WatchRect;
// 0x007fea44
extern unsigned int StoredTransparentColour;
// 0x007fea48
extern LEGO_EXPORT unsigned int FramesPerSecond;
// 0x007feb14
extern unsigned int DAT_007feb14;
// 0x007febb8
extern unsigned short DAT_007febb8;
// 0x007febc0
extern LEGO_EXPORT struct Cursor EditCursor;
// 0x008003f4
extern int LevelMapHandle;
// 0x008003f8
extern unsigned int DAT_008003f8;
// 0x00800400
extern LEGO_EXPORT RECT ObjectPartArray[256];
// 0x00801400
extern LEGO_EXPORT struct MapElement **GameMap;
// 0x00801404
extern void *BridgesHandle;
// 0x00801408
extern unsigned int DAT_00801408;
// 0x0080140c
extern void *LevelTileMapHandle;
// 0x00801410
extern void *OverlayILFHandle;
// 0x00801420
extern struct DeferredSprite DAT_00801420[100];
// 0x00801a60
extern int DAT_00801a60;
// 0x00801a64
extern int DAT_00801a64;
// 0x00801a68
extern void *DAT_00801a68;
// 0x00801a6c
extern int *DAT_00801a6c;
// 0x00801a70
extern void *DAT_00801a70;
// 0x00801a74
extern int DAT_00801a74;
// 0x00801a80
extern struct MapRect DAT_00801a80[10];
// 0x00801b20
extern unsigned int DAT_00801b20;
// 0x00801b24
extern LEGO_EXPORT unsigned int ObjectPartCount;
// 0x00801b40
extern int ObjectPartKey[256];
// 0x00801b28
extern int DAT_00801b28;
// 0x00801f40
extern LEGO_EXPORT struct TileSpriteEntry TileSpriteInfo[2048];
// 0x00805f40
extern int DAT_00805f40;
// 0x00805f44
extern int DAT_00805f44;
// 0x00805f48
extern unsigned int DAT_00805f48;
// 0x00805f60
extern LEGO_EXPORT struct Sprite *TileSpriteArray[2048];
// 0x00807f60
extern struct MapRenderOrderEntry MapRenderOrderList[4096];
// 0x0080ff60
extern unsigned int DAT_0080ff60;
// 0x0080ff64
extern struct Element *CastleObjElem;
// 0x0080ff68
extern unsigned int DAT_0080ff68;
// 0x0080ff6c
extern void *DAT_0080ff6c;
extern unsigned int DAT_0080ff70;
// 0x0080ff74
extern LEGO_EXPORT unsigned int NEWFLC_AutoPlay;
// 0x0080ff78
extern LEGO_EXPORT unsigned char NEWFLC_PauseType;
// 0x0080ff80
extern struct ScreenMode DAT_0080ff80;
// 0x0080ffa0
extern struct ScreenState CurrentProfile;
// 0x008100c0
extern char DAT_008100c0[0x80];
// 0x00810140
extern unsigned int DAT_00810140;
// 0x00810144
extern unsigned int DAT_00810144;
// 0x00810148
extern struct Sprite *SPRITE_TitleScreenBk;
// 0x0081014c
extern LEGO_EXPORT unsigned char NEWFLC_ID[20];
// 0x00810160
extern LEGO_EXPORT struct Cursor QueryCursor;
// 0x008119a0
extern LEGO_EXPORT unsigned int NEWFLC_CheckDuplicate;
// 0x008119a4
extern unsigned int FrameCounter;
// 0x008119a8
extern LEGO_EXPORT unsigned int NEWFLC_BuffSize;
// 0x008119ac
extern LEGO_EXPORT unsigned short NEWFLC_Repeat;
// 0x008119b0
extern LEGO_EXPORT struct EditState EditMode;
// 0x008119bc
extern unsigned int DAT_008119bc;
struct MapMarker {
    unsigned int x, y;
};
// 0x008119c0
extern struct MapMarker DAT_008119c0[31][32];
// 0x008138c0
extern struct MapMarker DAT_008138c0[32];
// 0x008139c0
extern int MapViewHeight;
// 0x008139c4
extern int MapViewWidth;
// 0x008139c8
extern int MapViewX;
// 0x008139cc
extern int MapViewY;
// 0x008139e0
extern struct Sprite *DAT_008139e0;
// 0x008139e4
extern struct Sprite *MiHungrySprite;
// 0x008139e8
extern struct Sprite *MiHappySprite;
// 0x008139ec
extern struct Sprite *MiSadSprite;
// 0x008139f0
extern struct Sprite *MiHomeSprite;
// 0x008139f4
extern struct Sprite *MiEatSprite;
// 0x008139f8
extern struct Sprite *GreatSprite;
// 0x008139fc
extern struct Sprite *PoorSprite;
// 0x00813a00
extern struct Sprite *FavouriteSprite;
// 0x00813a04
extern struct Sprite *OpinionSprite;
// 0x00813a08
extern struct Sprite *MiBoredSprite;
// 0x00813a0c
extern void *SpeechBubbleData;
// 0x00813a10
extern unsigned int DebugAllocatedBytes;
// 0x00813a18
extern unsigned long DAT_00813a18;
// 0x00813a2c
extern int DAT_00813a2c;
// 0x00813a34
extern unsigned short DAT_00813a34;
// 0x00813a38
extern unsigned int DAT_00813a38;
// 0x00813a3c
extern unsigned int DAT_00813a3c;
// 0x00813a40
extern LEGO_EXPORT unsigned int GamePad;
// 0x00813a44
extern struct Point MousePos;
// 0x00813a4c
extern unsigned int DAT_00813a4c;
// 0x00813a50
extern unsigned int DAT_00813a50;
// 0x00813a54
extern unsigned int DAT_00813a54;
// 0x00813a5c
extern unsigned int DAT_00813a5c;
// 0x00813a60
extern unsigned int DAT_00813a60;
// 0x00813a64
extern unsigned int MouseTileX;
// 0x00813a68
extern unsigned int MouseTileY;
// 0x00813a6c
extern unsigned int FootprintWidth;
// 0x00813a70
extern unsigned int FootprintHeight;
// 0x00813a74
extern unsigned int DAT_00813a74;
// 0x00813a78
extern unsigned int DAT_00813a78;
// 0x00813a7c
extern unsigned int DAT_00813a7c;
// 0x00813a80
extern unsigned int DAT_00813a80;
// 0x00813a84
extern unsigned int DAT_00813a84;
// 0x00813a88
extern unsigned int DAT_00813a88;
// 0x00813a8c
extern unsigned int DAT_00813a8c;
// 0x00813a90
extern unsigned int DAT_00813a90;
// 0x00813a94
extern unsigned int DAT_00813a94;
// 0x00813a98
extern unsigned int DAT_00813a98;
// 0x00813a9c
extern unsigned int DAT_00813a9c;
// 0x00813aa0
extern unsigned int DAT_00813aa0;
// 0x00813aa4
extern unsigned int DAT_00813aa4;
// 0x00813aa8
extern unsigned int DAT_00813aa8;
// 0x00813aac
extern unsigned int DAT_00813aac;
// 0x00813ab0
extern unsigned int DAT_00813ab0;
// 0x00813ab4
extern unsigned int DAT_00813ab4;
// 0x00813ab8
extern unsigned int DAT_00813ab8;
// 0x00813abc
extern unsigned int DAT_00813abc;
// 0x00813ac0
extern unsigned int DAT_00813ac0;
// 0x00813ac4
extern unsigned int DAT_00813ac4;
// 0x00813ac8
extern unsigned int DAT_00813ac8;
// 0x00813acc
extern unsigned int DAT_00813acc;
// 0x00813ad0
extern unsigned int DAT_00813ad0;
// 0x00813ad4
extern unsigned int DAT_00813ad4;
// 0x00813ad8
extern unsigned int DAT_00813ad8;
// 0x00813adc
extern unsigned int DAT_00813adc;
// 0x00813ae0
extern unsigned int DAT_00813ae0;
// 0x00813af0
extern int DAT_00813af0;
// 0x00813af4
extern int DAT_00813af4;
// 0x00813af8
extern int DAT_00813af8;
// 0x00813afc
extern int DAT_00813afc;
// 0x00813b00
extern LEGO_EXPORT struct CtrlBuffer *CONTROLLERBUFFER;
// 0x00813b04
extern char CdDrivePath[4];
// 0x00813b08
extern unsigned int DAT_00813b08; /* letter of the bloke being processed */
// 0x00813b20
extern unsigned char DAT_00813b20[0x300];
// 0x00813e20
extern unsigned short DAT_00813e20[256];
// 0x00814020
extern unsigned char ColourLookupTable[0x8000];
// 0x0081c028
extern struct Sprite *AppBarSprite;
// 0x0081c02c
extern struct Sprite *NextPageSprite;
// 0x0081c030
extern struct Sprite *AppBarMarkerSprite;
// 0x0081c034
extern struct Sprite *NextPageLitSprite;
// 0x0081c038
extern unsigned int DAT_0081c038;
// 0x0081c040
extern struct Sprite *DAT_0081c040[5];
// 0x0081c054
extern struct Sprite *DAT_0081c054[5];
// 0x0081c068
extern struct Sprite *DAT_0081c068[5];
// 0x0081c07c
extern unsigned int DAT_0081c07c;
// 0x0081c080
extern struct Sprite *PreviousPageSprite;
// 0x0081c084
extern struct Sprite *PreviousPageLitSprite;
// 0x0081c088
extern unsigned int DAT_0081c088;
// 0x0081c08c
extern void *AdBlinkAnim;
// 0x0081c090
extern void *AdWobbleAnim;
// 0x0081c094
extern void *AdLRAnim;
// 0x0081c098
extern void *AdPhoneDownAnim;
// 0x0081c09c
extern unsigned int DAT_0081c09c;
// 0x0081c0a0
extern void *AdPhoneAnim;
// 0x0081c0a4
extern void *AdPhoneGestureAnim;
// 0x0081c0c0
extern int DAT_0081c0c0[0x200];
// 0x0081c4c0
extern unsigned int DAT_0081c4c0[0x100];
// 0x0081c8c0
extern void *VisitorLocData;
// 0x0081c8c4
extern void *TracyLocData;
// 0x0081c8c8
extern void *GeoffLocData;
// 0x0081c8cc
extern void *DAT_0081c8cc;
// 0x0081c8d0
extern unsigned int ViewportLeft;
// 0x0081c8d4
extern unsigned int ViewportTop;
// 0x0081c8d8
extern unsigned int ViewportRight;
// 0x0081c8dc
extern unsigned int ViewportBottom;
// 0x0081c8e0
extern char DAT_0081c8e0[512];
// 0x0081cae0
extern struct Sprite *ZoomerSprite;
// 0x0081cae8
extern int DAT_0081cae8;
// 0x0081caec
extern int DAT_0081caec;
// 0x0081caf0
extern struct Ride *PottingShedRide;
// 0x0081caf4
extern struct Ride *MechanicsHutRide;
// 0x0081cb00
extern struct Sprite *SaloonMatte1Sprite;
// 0x0081cb04
extern struct Sprite *SaloonMatte2Sprite;
// 0x0081cb08
extern struct Sprite *GStoreMatteSprite;
// 0x0081cb0c
extern struct Sprite *JailCellMaskSprite;
// 0x0081cb10
extern struct Building *DAT_0081cb10;
// 0x0081cb14
extern struct Building *SheriffBuilding;
// 0x0081cb18
extern struct Sprite *LegoShop1MatteSprite;
// 0x0081cb1c
extern struct Building *SaloonBuilding;
// 0x0081cb20
extern struct Sprite *LegoShop2MatteSprite;
// 0x0081cb24
extern struct Sprite *GStoreMatte2Sprite;
// 0x0081cb28
extern struct Sprite *ExplorersInstituteMatteSprite;
// 0x0081cb2c
extern struct Building *BankBuilding;
// 0x0081cb30
extern struct Building *GeneralStoreBuilding;
// 0x0081cb34
extern struct Sprite *BankMatteSprite;
// 0x0081cb38
extern struct Sprite *SherifshutMatteSprite;
// 0x0081cb3c
extern struct Building *LegoShop1Building;
// 0x0081cb40
extern struct Building *LegoMediaShopBuilding;
// 0x0081cb44
extern struct Building *ExplorersInstituteBuilding;
// 0x0081cb48
extern struct Sprite *LegMediaShopMask1Sprite;
// 0x0081cb4c
extern struct Building *LegoShop2Building;
// 0x0081cb50
extern struct Sprite *LegMediaShopMask2Sprite;
// 0x0081cb54
extern struct Ride *DAT_0081cb54;
// 0x0081cb58
extern struct TileMap *BoatingSchoolTileMap;
// 0x0081cb5c
extern struct Sprite *JungMaskSprite;
// 0x0081cb60
extern struct Ride *JungleCruiseRide;
// 0x0081cb64
extern struct Ride *DAT_0081cb64;
// 0x0081cb68
extern struct Sprite *BrijMaskSprite;
// 0x0081cb6c
extern struct Sprite *MFish2Sprite;
// 0x0081cb70
extern struct Ride *MonkeyTreeRide;
// 0x0081cb74
extern struct Ride *MFish2Ride;
// 0x0081cb80
extern struct Point DAT_0081cb80[3][16];
// 0x0081cd00
extern struct SpriteSet *JungleCruiseBoats;
// 0x0081cd04
extern void *DAT_0081cd04;
// 0x0081cd08
extern void *HedgeObjectClass;
// 0x0081cd0c
extern unsigned int CastleBBQRide;
// 0x0081cd10
extern unsigned int CastleBBQLayer;
// 0x0081cd14
extern unsigned int FoodcartDrinkRide;
// 0x0081cd18
extern struct EateryFX *SharkCafeRide;
// 0x0081cd1c
extern struct EateryFX *OctopusCafeRide;
// 0x0081cd20
extern struct Sprite *R2TowermSprite;
// 0x0081cd28
extern struct Sprite *RestMaskMainSprite;
// 0x0081cd2c
extern unsigned int EateryFxInner;
// 0x0081cd30
extern unsigned int Restaurant2Ride;
// 0x0081cd34
extern struct Sprite *R2FdoormSprite;
// 0x0081cd38
extern struct EateryFX *DAT_0081cd38;
// 0x0081cd3c
extern unsigned int FoodcartFoodRide;
// 0x0081cd40
extern unsigned int Restaurant1Ride;
// 0x0081cd44
extern struct EateryFX *ChuckWagonRide;
// 0x0081cd48
extern struct Sprite *R2Fdoorm1Sprite;
// 0x0081cd60
extern struct Sprite *OctTentASprite;
// 0x0081cd64
extern struct Sprite *OctTentBSprite;
// 0x0081cd68
extern struct Sprite *OctTentCSprite;
// 0x0081cd6c
extern struct Sprite *OctTentDSprite;
// 0x0081cd70
extern struct Sprite *OctTentESprite;
// 0x0081cd74
extern struct Sprite *OctTentFSprite;
// 0x0081cd78
extern struct Sprite *OctTentGSprite;
// 0x0081cd7c
extern struct Sprite *OctTentHSprite;
// 0x0081cd80
extern struct Sprite *OctoKioskSprite;
// 0x0081cd84
extern struct Sprite *R2BdoormSprite;
// 0x0081cd88
extern struct Sprite *RestMaskLevel1Sprite;
// 0x0081cd8c
extern struct Sprite *RestMaskLevel1aaSprite;
// 0x0081cd90
extern struct Sprite *RestMaskLevel3Sprite;
// 0x0081cd94
extern struct Sprite *RestMaskLevel2Sprite;
// 0x0081cda0
extern struct Sprite *OctTabAASprite;
// 0x0081cda4
extern struct Sprite *OctTabABSprite;
// 0x0081cda8
extern struct Sprite *OctTabBASprite;
// 0x0081cdac
extern struct Sprite *OctTabBBSprite;
// 0x0081cdb0
extern struct Sprite *OctTabCASprite;
// 0x0081cdb4
extern struct Sprite *OctTabCBSprite;
// 0x0081cdb8
extern struct Sprite *OctTabDASprite;
// 0x0081cdbc
extern struct Sprite *OctTabDBSprite;
// 0x0081cdc0
extern struct Sprite *OctTabEASprite;
// 0x0081cdc4
extern struct Sprite *OctTabEBSprite;
// 0x0081cdc8
extern struct Sprite *OctTabFASprite;
// 0x0081cdcc
extern struct Sprite *OctTabFBSprite;
// 0x0081cdd0
extern struct Sprite *OctTabGASprite;
// 0x0081cdd4
extern struct Sprite *OctTabGBSprite;
// 0x0081cdd8
extern struct Sprite *OctTabHASprite;
// 0x0081cddc
extern struct Sprite *OctTabHBSprite;
// 0x0081cde0
extern struct EateryFX *FoodcartIcecreamRide;
// 0x0081cde4
extern struct Ride *BalloonzRide;
// 0x0081cde8
extern struct Sprite *ZBalloon2Sprite;
// 0x0081cdec
extern void *DAT_0081cdec;
// 0x0081ce00
extern struct Cursor DAT_0081ce00[8];
// 0x00828fe0
extern unsigned int DAT_00828fe0[2];
// 0x00829980
extern void *BasicTilesData;
// 0x00829990
extern float DAT_00829990[3];
// 0x0082999c
extern float DAT_0082999c;
// 0x008299a0
extern float DAT_008299a0[3];
// 0x008299bc
extern struct FMat4 DAT_008299bc;
// 0x008299fc
extern struct FMat4 DAT_008299fc;
// 0x008299ac
extern int DAT_008299ac;
// 0x008299b0
extern int DAT_008299b0;
// 0x008299b4
extern int DAT_008299b4;
// 0x008299b8
extern int DAT_008299b8;
// 0x00829a3c
extern struct ListLink DAT_00829a3c;
// 0x00829a58
extern void (*DAT_00829a58)(void);
// 0x00829a5c
extern void (*DAT_00829a5c)(void);
// 0x00829a60
extern float DAT_00829a60;
// 0x00829a64
extern unsigned int DAT_00829a64;

// 0x00829a80
extern struct EditFootPrint DAT_00829a80;
// 0x00829abc
extern struct Element *DAT_00829abc;
// 0x00829ae0
extern unsigned int DAT_00829ae0;
// 0x00829ae4
extern unsigned int DAT_00829ae4;
// 0x00829ae8
extern short DAT_00829ae8[2];
// 0x00829af8
extern unsigned int DAT_00829af8[3];
// 0x00829b04
extern unsigned int DAT_00829b04[3];
// 0x00829b0c
extern unsigned int DAT_00829b0c;
// 0x00829aec
extern unsigned int DAT_00829aec;
// 0x00829af0
extern unsigned int DAT_00829af0;
// 0x00829af4
extern unsigned int DAT_00829af4;
// 0x004b5b48
extern unsigned int DAT_004b5b48;
// 0x00829b88
extern unsigned int DAT_00829b88;
// 0x00829b8c
extern unsigned int DAT_00829b8c;
// 0x00829ba0
extern unsigned int DAT_00829ba0;
// 0x00829ba4
extern unsigned int DAT_00829ba4;
// 0x00829b90
extern short DAT_00829b90[4][2];
// 0x00829ba8
extern short DAT_00829ba8[4][2];
// 0x00829bec
extern void *DAT_00829bec;
// 0x00829bf0
extern void *DAT_00829bf0;
// 0x00829bf4
extern void *DAT_00829bf4;
// 0x00829bf8
extern unsigned int DAT_00829bf8;
// 0x00829bfc
extern unsigned int DAT_00829bfc;

// 0x00829c00
extern int DAT_00829c00;
// 0x00829c04
extern struct Sprite *CastleMatteSprite;
// 0x00829c08
extern unsigned int DAT_00829c08;

// 0x00829c34
extern int DAT_00829c34;
// 0x00829c54
extern char *DAT_00829c54;
// 0x00829c60
extern void *DAT_00829c60[1024];
// 0x0082ac60
extern void *DAT_0082ac60[48];
// 0x0082ad20
extern unsigned char CastleDispatchTable[0x90];
// 0x0082adb0
extern unsigned int DAT_0082adb0[6];
// 0x0082add0
extern unsigned int CoasterTrainHeadCarLms;
// 0x0082add4
extern unsigned int CoasterTrainMidCarLms;
// 0x0082add8
extern unsigned int CoasterTrainTailCarLms;
// 0x0082ade0
extern unsigned int CoasterTrainHeadCarLfm;
// 0x0082ade4
extern unsigned int CoasterTrainMidCarLfm;
// 0x0082ade8
extern unsigned int CoasterTrainTailCarLfm;
// 0x0082adec
extern unsigned int DAT_0082adec;
// 0x0082adf0
extern struct Ride *BoatingSchoolWaterRide;
// 0x0082adf4
extern struct TileMap *BoatingSchoolTileMapping;
// 0x0082adf8
extern struct Ride *BoatingSchoolMermaidRide;
// 0x0082adfc
extern struct Sprite *BoatingSchoolHullMaskSprite;
// 0x0082ae00
extern void *DAT_0082ae00;
// 0x0082ae20
extern struct Cursor DAT_0082ae20;
// 0x0082c654
extern struct Sprite *BoatingSchoolRailmSprite;
// 0x0082c658
extern struct Ride *BoatingSchoolRide;
// 0x0082c65c
extern struct SpriteSet *BoatingSchoolBoats;
// 0x0082c668
extern struct Sprite *ZSpiderSprite;
// 0x0082c678
extern void *ZebraCrossingRide;
// 0x0082c67c
struct DSMapEntry {
    int pad_0;
    unsigned short *base;
};
extern struct DSMapEntry *DSchoolMappingData;
// 0x0082c680
struct RoadLights;
extern struct RoadLights *DrivingSchoolLightsData;
// 0x0082c684
extern void *DAT_0082c684;
// 0x0082c688
extern void *DAT_0082c688;
// 0x0082c690
extern unsigned short *DSchoolRedPalette;
// 0x0082c694
extern struct DSCursorSource *DrivingSchoolRide;
// 0x0082c6b8
extern unsigned short *DSchoolYellowPalette;
// 0x0082c6bc
extern unsigned short *DSchoolBluePalette;
// 0x0082c6c0
extern struct Sprite *DSchoolMatteSprite;
// 0x0082c6e0
extern struct Cursor DAT_0082c6e0;
// 0x0082df20
extern struct Cursor DAT_0082df20;
// 0x0082f760
extern struct Cursor DAT_0082f760;
// 0x00830f94
extern struct Sprite *DSCarSprite;
// 0x00830f98
extern struct Position *CoptersPos;
// 0x00830f9c
struct DriveTable {
    unsigned char pad_0[0xc];
    int *x;
    int *y;
};
extern struct DriveTable *DSchoolBlueCarData;
// 0x00830fc0
extern LEGO_EXPORT struct Cursor PathCursor;
// 0x00832800
extern LEGO_EXPORT struct MapStats MapStats;
// 0x00832bf0
extern LEGO_EXPORT void *PathSprite;
// 0x0082c6a0
extern struct RideSpriteInfo RideSpriteInfoBuffer;
// 0x00616028
extern struct RideSpriteInfo DAT_00616028;
// 0x006160a0
extern struct RideSpriteInfo DAT_006160a0;
// 0x0062fe30
extern struct RideSpriteInfo DAT_0062fe30;
// 0x0062fe10
extern struct RideSpriteInfo DAT_0062fe10;

// 0x004ab404
extern float FLOAT_004ab404;

// 0x004b55fc
struct PlaneSet;
extern struct PlaneSet *DAT_004b55fc;

// 0x004d83b4
extern struct RingHost *DAT_004d83b4;

// 0x004d83a0
extern unsigned int DAT_004d83a0[5];

// 0x004b5634
extern float FLOAT_004b5634;

// 0x004b559c
extern float DAT_004b559c[3];

// 0x004b55a8
extern float FLOAT_004b55a8;

// 0x004b4cac
extern char DAT_004b4cac[];

// 0x00641000
extern int DAT_00641000;

// 0x00641004: per-vertex depth/shade values of the 3D person being drawn (FUN_00440a30)
extern int DAT_00641004[3001];

// 0x00643ee8: that person's vertices (x, y, z), rotated and scaled
extern int DAT_00643ee8[3001][3];

// 0x0063810c
extern int DAT_0063810c;

// 0x0064cd90
extern int DAT_0064cd90;

// 0x0064cd88
extern int DAT_0064cd88;

// 0x0063835c
extern int DAT_0063835c;

// 0x00638110
extern int DAT_00638110;

// 0x00638108
extern int DAT_00638108;

// 0x004b5600
extern int **DAT_004b5600[2];
// 0x004b5610
extern float DAT_004b5610;
// 0x004b5614
extern float DAT_004b5614;
// 0x004b5618
extern float DAT_004b5618[3];
// 0x004b5624
extern float DAT_004b5624;
// 0x004b5628
extern float DAT_004b5628[3];
// 0x004d83c4
extern int DAT_004d83c4[0x100];
// 0x004d87c4
extern int *DAT_004d87c4;
// 0x004d884c
extern int *DAT_004d884c[32];
// 0x004d88cc
extern unsigned int *DAT_004d88cc[5];

// 0x004b5660
extern float DAT_004b5660[4][2];

// 0x004dd75c
extern unsigned int DAT_004dd75c;
// 0x004dd864
extern unsigned int DAT_004dd864;

struct Struct1e40;
// 0x004dd644
extern struct Struct1e40 *DAT_004dd644;
// 0x004dd648
extern struct Struct1e40 *DAT_004dd648;
// 0x004dd64c
extern float *DAT_004dd64c;
// 0x004dd650
extern int DAT_004dd650;

// 0x004ab438
extern float FLOAT_004ab438;

// 0x00610804
extern int DAT_00610804[64];

// 0x00610500
extern int DAT_00610500[64];

// 0x00610400
extern int DAT_00610400[64];

// 0x00610704
extern int DAT_00610704[64];

// 0x00610604
extern int DAT_00610604[64];

// 0x006100f8
extern int DAT_006100f8[64];

// 0x0060fdf8
extern int DAT_0060fdf8[64];

// 0x0060fef8
extern int DAT_0060fef8[64];

// 0x0060fbf8
extern int DAT_0060fbf8[64];

// 0x006101f8
extern int DAT_006101f8[64];

// 0x0060fcf8
extern int DAT_0060fcf8[64];

// 0x00610904
extern int DAT_00610904[64];

// 0x00610a10
extern unsigned int DAT_00610a10;

// 0x0060f900
extern int DAT_0060f900;

// 0x0060f904
extern int DAT_0060f904;

// 0x00579878
extern char DAT_00579878[0x80];

// 0x0060f8fc
extern int DAT_0060f8fc;

// 0x00611644
extern int DAT_00611644;

// 0x00615f68
extern int DAT_00615f68;

// 0x004dcbc8
extern int DAT_004dcbc8;

// 0x004d83bc
extern int DAT_004d83bc;

// 0x0060f910
extern int DAT_0060f910;

// 0x004b5964
extern unsigned int DAT_004b5964[2];
// 0x004b596c
extern const char *DAT_004b596c[2];
// 0x004b5974
extern unsigned int DAT_004b5974[2];
// 0x004b597c
extern const char *DAT_004b597c[2];

// 0x006121c8
extern float DAT_006121c8[6][3];
// 0x006126d8
extern float DAT_006126d8[6][2];
// 0x00612708
extern unsigned int DAT_00612708[30][36];
// 0x006137e8
extern float DAT_006137e8[3][4][3];
// 0x00613878
extern unsigned int DAT_00613878[36];
// 0x00613908
extern int DAT_00613908[24][2];
// 0x006139c8
extern int DAT_006139c8[31][6][5];
// 0x006148b8
extern int DAT_006148b8[546][2];

// 0x00667528
extern int ExceptionReportStarted;
// 0x004b8a88
extern int DAT_004b8a88;
// 0x004b8a8c
extern int DAT_004b8a8c;
// 0x004b8a90
extern int DAT_004b8a90;
