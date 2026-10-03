#include "setcustomcallbacks.h"
#include <string.h>
#include "balloonz.h"
#include "boating_school.h"
#include "carousel.h"
#include "castle.h"
#include "castle_level.h"
#include "catapult.h"
#include "copters.h"
#include "driving_school.h"
#include "earth_slide.h"
#include "eatery.h"
#include "entrance.h"
#include "fort.h"
#include "garden.h"
#include "gold_rush.h"
#include "joust.h"
#include "jungle_cruise.h"
#include "legoland.h"
#include "log_flume.h"
#include "mechanics_hut.h"
#include "objclass.h"
#include "path_control.h"
#include "plane_ride.h"
#include "potting_shed.h"
#include "power.h"
#include "pumps.h"
#include "roads.h"
#include "safari_ride.h"
#include "shops.h"
#include "space_tower.h"
#include "spider_ride.h"
#include "spinning_barrels.h"
#include "temple.h"
#include "temple_slide.h"
#include "water_works.h"

// FUNCTION: LEGOLAND 0x00452c20
LEGO_EXPORT void SetCustomCallbacks(struct ClassNode *head) {
    struct CallbackTable *iface = head->iface;

    if (_stricmp(head->name, "PATH CONTROL") == 0) {
        iface->cb_98 = AddBasicPath;
        iface->cb_9c = RemoveBasicPath;
        iface->cb_a0 = DrawBasicPath;
    }
    // STRING: LEGOLAND 0x004b8a64
    else if (_stricmp(head->name, "FOUNTAIN 1") == 0 ||
        // STRING: LEGOLAND 0x004b8a58
        _stricmp(head->name, "FOUNTAIN 2") == 0 ||
        // STRING: LEGOLAND 0x004b8a4c
        _stricmp(head->name, "FOUNTAIN 3") == 0) {
        iface->cb_98 = FountainAddObject;
        iface->cb_9c = RemoveSoundObject;
        iface->cb_ac = KillFountainSFX;
        LoadFountainSFX(head);
    }
    // STRING: LEGOLAND 0x004b8a34
    else if (_stricmp(head->name, "crystal power station") == 0) {
        iface->cb_98 = CrystalPowerStationAddObject;
        iface->cb_9c = RemoveSoundObject;
        iface->cb_ac = KillPowerStationSFX;
        LoadPowerStationSFX(head);
    }
    // STRING: LEGOLAND 0x004b8a20
    else if (_stricmp(head->name, "small power station") == 0) {
        iface->cb_98 = SmallPowerStationAddObject;
        iface->cb_9c = RemoveSoundObject;
        iface->cb_ac = KillPowerStationSFX;
        LoadPowerStationSFX(head);
    }
    // STRING: LEGOLAND 0x004b8a14
    else if (_stricmp(head->name, "Dino Big") == 0 ||
        // STRING: LEGOLAND 0x004b8a08
        _stricmp(head->name, "Dino Small") == 0 ||
        // STRING: LEGOLAND 0x004b89fc
        _stricmp(head->name, "Dino Mini") == 0) {
        iface->cb_98 = DinoAddObject;
        iface->cb_9c = RemoveSoundObject;
        iface->cb_ac = KillDinoSFX;
        LoadDinoSFX(head);
    }
    // STRING: LEGOLAND 0x004b89e4
    else if (_stricmp("DRIVING SCHOOL PUMPS", head->name) == 0) {
        iface->cb_a4 = InitDrivingSchoolPumps;
        iface->cb_8c = DrivingSchoolPumpsSetEditMode;
        iface->cb_90 = FUN_00411cd0;
        iface->cb_98 = DrivingSchoolPumpsAddObject;
        iface->cb_9c = DrivingSchoolPumpsRemoveObject;
    } else if (_stricmp("DRIVING SCHOOL", head->name) == 0) {
        iface->cb_a4 = LoadDrivingSchoolResources;
        iface->cb_8c = DrivingSchoolSetEditMode;
        iface->cb_90 = FUN_00405740;
        iface->cb_94 = FUN_004058a0;
        iface->cb_98 = DrivingSchoolAddObject;
        iface->cb_9c = DrivingSchoolRemoveObject;
        iface->cb_a8 = FUN_00405bd0;
        iface->cb_a0 = GetDrivingSchoolSpriteInfo;
        iface->cb_b0 = RenderDrivingSchool;
        iface->cb_bc = DrivingSchool_Save;
        iface->cb_b8 = DrivingSchool_Load;
        iface->cb_ac = UnloadDrivingSchoolResources;
        iface->cb_c0 = FUN_00406050;
    }
    // STRING: LEGOLAND 0x004b89cc
    else if (_stricmp("DRIVING SCHOOL ROADS", head->name) == 0) {
        iface->cb_a4 = LoadDrivingSchoolRoadsResources;
        iface->cb_ac = UnloadDrivingSchoolRoadsResources;
        iface->cb_8c = DrivingSchoolRoadsSetEditMode;
        iface->cb_90 = FUN_00413b50;
        iface->cb_94 = FUN_00413fa0;
        iface->cb_98 = FUN_00414020;
        iface->cb_9c = FUN_00414220;
    }
    // STRING: LEGOLAND 0x004b89bc
    else if (_stricmp("ZEBRA CROSSING", head->name) == 0) {
        iface->cb_a4 = InitZebraCrossing;
        iface->cb_8c = ZebraCrossingSetEditMode;
        iface->cb_90 = FUN_00414880;
        iface->cb_94 = FUN_00413fa0;
        iface->cb_98 = FUN_00414950;
        iface->cb_9c = FUN_00414220;
    } else if (_stricmp("ENTRANCE 1", head->name) == 0) {
        iface->cb_a4 = LoadEntranceResources;
        iface->cb_ac = UnloadEntranceResources;
        iface->cb_a8 = FUN_0042dfa0;
        iface->cb_b0 = RenderEntrance;
        iface->cb_9c = EntranceRemoveObject;
    } else if (_stricmp("POTTING SHED", head->name) == 0) {
        iface->cb_a4 = LoadGShedMatteSprite;
        iface->cb_8c = PottingShedSetEditMode;
        iface->cb_98 = PottingShedAddObject;
        iface->cb_9c = PottingShedRemoveObject;
        iface->cb_a8 = FUN_0043cf00;
        iface->cb_b0 = RenderPottingShed;
        iface->cb_ac = KillGShedMatteSprite;
        iface->cb_a0 = FUN_0043d210;
    } else if (_stricmp("MECHANICS HUT", head->name) == 0) {
        iface->cb_a4 = LoadMechHutMaskSprite;
        iface->cb_8c = MechanicsHutSetEditMode;
        iface->cb_98 = MechanicsHutAddObject;
        iface->cb_9c = MechanicsHutRemoveObject;
        iface->cb_a8 = FUN_0043d2f0;
        iface->cb_b0 = RenderMechanicsHut;
        iface->cb_ac = KillMechHutMaskSprite;
        iface->cb_a0 = GetMechanicsHutSpriteInfo;
    }
    // STRING: LEGOLAND 0x004b8990
    else if (_stricmp("CAROUSEL", head->name) == 0) {
        iface->cb_a4 = LoadCarouselResources;
        iface->cb_ac = UnloadCarouselResources;
        iface->cb_8c = CarouselSetEditMode;
        iface->cb_a8 = FUN_0042c820;
        iface->cb_b0 = RenderCarousel;
        iface->cb_9c = CarouselRemoveObject;
        iface->cb_98 = CarouselAddObject;
        iface->cb_a0 = GetCarouselSpriteInfo;
        iface->cb_b8 = Carousel_Load;
        iface->cb_bc = Carousel_Save;
    }
    // STRING: LEGOLAND 0x004b8984
    else if (_stricmp("BALLOONZ", head->name) == 0) {
        iface->cb_a4 = LoadBalloonzResources;
        iface->cb_8c = BalloonzSetEditMode;
        iface->cb_98 = BalloonzAddObject;
        iface->cb_9c = BalloonzRemoveObject;
        iface->cb_a8 = FUN_0042aa90;
        iface->cb_b0 = RenderBalloonz;
        iface->cb_ac = UnloadBalloonzResources;
        iface->cb_a0 = GetBalloonzSpriteInfo;
        iface->cb_bc = SaveBalloonNodes;
        iface->cb_b8 = Balloonz_Load;
    }
    // STRING: LEGOLAND 0x004b8970
    else if (_stricmp("EARTH SLIDE RIDE", head->name) == 0) {
        iface->cb_a4 = LoadEarthSlideResources;
        iface->cb_ac = UnloadEarthSlideResources;
        iface->cb_8c = EarthSlideSetEditMode;
        iface->cb_a8 = FUN_0042d610;
        iface->cb_b0 = RenderEarthSlide;
        iface->cb_9c = EarthSlideRemoveObject;
        iface->cb_98 = EarthSlideAddObject;
        iface->cb_bc = EarthSlideRide_Save;
        iface->cb_b8 = EarthSlideRide_Load;
    }
    // STRING: LEGOLAND 0x004b8964
    else if (_stricmp("CASTLE BBQ", head->name) == 0) {
        iface->cb_a4 = LoadCastleBBQResources;
        iface->cb_ac = UnloadCastleBBQResources;
        iface->cb_8c = EaterySetEditMode;
        iface->cb_a8 = FUN_0042ea60;
        iface->cb_b0 = RenderCastleBBQ;
        iface->cb_98 = CastleBBQAddObject;
        iface->cb_9c = CastleBBQRemoveObject;
    }
    // STRING: LEGOLAND 0x004b8954
    else if (_stricmp("FOODCART DRINK", head->name) == 0) {
        iface->cb_a4 = LoadFoodcartDrinkResources;
        iface->cb_ac = UnloadFoodcartDrinkResources;
        iface->cb_8c = EaterySetEditMode;
        iface->cb_a8 = FUN_0042ec10;
        iface->cb_9c = EateryRemoveObject;
        iface->cb_b0 = FUN_0042e830;
    }
    // STRING: LEGOLAND 0x004b8944
    else if (_stricmp("FOODCART FOOD", head->name) == 0) {
        iface->cb_a4 = LoadFoodcartFoodResources;
        iface->cb_ac = UnloadFoodcartFoodResources;
        iface->cb_8c = EaterySetEditMode;
        iface->cb_a8 = FUN_0042ed70;
        iface->cb_9c = EateryRemoveObject;
        iface->cb_b0 = FUN_0042e830;
    }
    // STRING: LEGOLAND 0x004b8930
    else if (_stricmp("FOODCART ICECREAM", head->name) == 0) {
        iface->cb_a4 = LoadFoodcartIcecreamResources;
        iface->cb_ac = UnloadFoodcartIcecreamResources;
        iface->cb_8c = EaterySetEditMode;
        iface->cb_a8 = FUN_00431170;
        iface->cb_9c = EateryRemoveObject;
        iface->cb_b0 = FUN_0042e830;
    }
    // STRING: LEGOLAND 0x004b8920
    else if (_stricmp("OCTOPUS CAFE", head->name) == 0) {
        iface->cb_a4 = LoadOctopusCafeResources;
        iface->cb_98 = OctopusCafeAddObject;
        iface->cb_8c = EaterySetEditMode;
        iface->cb_a8 = FUN_004316f0;
        iface->cb_9c = EateryRemoveObject;
        iface->cb_b0 = RenderOctopusCafe;
        iface->cb_ac = UnloadOctopusCafeResources;
    }
    // STRING: LEGOLAND 0x004b8910
    else if (_stricmp("RESTAURANT 1", head->name) == 0) {
        iface->cb_a4 = LoadRestaurant1Resources;
        iface->cb_8c = EaterySetEditMode;
        iface->cb_a8 = FUN_0042f1a0;
        iface->cb_98 = Restaurant1AddObject;
        iface->cb_9c = Restaurant1RemoveObject;
        iface->cb_ac = KillRestMaskSpritesAndMoneySFX;
        iface->cb_b0 = RenderRestaurant1;
        iface->cb_bc = Restaurant1_Save;
        iface->cb_b8 = Restaurant1_Load;
    }
    // STRING: LEGOLAND 0x004b8900
    else if (_stricmp("RESTAURANT 2", head->name) == 0) {
        iface->cb_a4 = LoadRestaurant2Resources;
        iface->cb_8c = EaterySetEditMode;
        iface->cb_a8 = FUN_0042fbb0;
        iface->cb_98 = Restaurant2AddObject;
        iface->cb_9c = Restaurant2RemoveObject;
        iface->cb_a0 = FUN_004304a0;
        iface->cb_ac = UnloadRestaurant2Resources;
        iface->cb_bc = Restaurant2_Save;
        iface->cb_b8 = Restaurant2_Load;
        iface->cb_b0 = RenderRestaurant2;
    }
    // STRING: LEGOLAND 0x004b88f4
    else if (_stricmp("CHUCK WAGON", head->name) == 0) {
        iface->cb_a4 = LoadChuckWagonResources;
        iface->cb_8c = EaterySetEditMode;
        iface->cb_9c = EateryRemoveObject;
        iface->cb_a8 = FUN_0042e2a0;
        iface->cb_b0 = RenderChuckWagon;
        iface->cb_ac = UnloadChuckWagonResources;
    }
    // STRING: LEGOLAND 0x004b88e8
    else if (_stricmp("SHARK CAFE", head->name) == 0) {
        iface->cb_a4 = LoadSharkCafeResources;
        iface->cb_ac = UnloadSharkCafeResources;
        iface->cb_8c = EaterySetEditMode;
        iface->cb_9c = EateryRemoveObject;
        iface->cb_a8 = FUN_0042e610;
        iface->cb_b0 = FUN_0042e830;
    }
    // STRING: LEGOLAND 0x004b83dc
    else if (_stricmp("SHARK CAFE BROLLY", head->name) == 0) {
        iface->cb_a4 = LoadBrollyImages;
        iface->cb_8c = BrollySetEditMode;
        iface->cb_98 = BrollyAddObject;
        iface->cb_a0 = GetBrollySpriteInfo;
        iface->cb_ac = UnloadBrollyImages;
    } else if (_stricmp("BOATING SCHOOL WATER", head->name) == 0) {
        iface->cb_a4 = FUN_0041b830;
        iface->cb_8c = BoatingSchoolSetEditMode;
        iface->cb_90 = FUN_0041bd40;
        iface->cb_94 = FUN_0041bfb0;
        iface->cb_98 = FUN_0041b8e0;
        iface->cb_9c = FUN_0041c130;
    } else if (_stricmp("BOATING SCHOOL", head->name) == 0) {
        iface->cb_a4 = LoadBoatingSchoolResources;
        iface->cb_ac = UnloadBoatingSchoolResources;
        iface->cb_8c = FUN_0041a000;
        iface->cb_90 = FUN_0041a2f0;
        iface->cb_94 = FUN_0041a3d0;
        iface->cb_98 = BoatingSchoolAddObject;
        iface->cb_9c = BoatingSchoolRemoveObject;
        iface->cb_a8 = FUN_0041a720;
        iface->cb_b0 = RenderBoatingSchool;
        iface->cb_bc = BoatingSchool_Save;
        iface->cb_b8 = BoatingSchool_Load;
        iface->cb_c0 = FUN_0041b100;
    }
    // STRING: LEGOLAND 0x004b5354
    else if (_stricmp("BOATING SCHOOL MERMAID", head->name) == 0) {
        iface->cb_a4 = InitBoatingSchoolMermaid;
        iface->cb_8c = BoatingSchoolMermaidSetEditMode;
        iface->cb_90 = FUN_0041b4c0;
        iface->cb_94 = FUN_0041b6d0;
        iface->cb_98 = BoatingSchoolMermaidAddObject;
        iface->cb_9c = BoatingSchoolMermaidRemoveObject;
    }
    // STRING: LEGOLAND 0x004b88d4
    else if (_stricmp("JUNGLE CRUISE WATER", head->name) == 0) {
        iface->cb_a4 = InitJungleCruiseWater;
        iface->cb_8c = JungleCruiseWaterSetEditMode;
        iface->cb_90 = FUN_00436200;
        iface->cb_94 = FUN_00436470;
        iface->cb_98 = FUN_004365f0;
        iface->cb_9c = FUN_00436a40;
    } else if (_stricmp("JUNGLE CRUISE", head->name) == 0) {
        iface->cb_a4 = LoadJungleCruiseResources;
        iface->cb_ac = UnloadJungleCruiseResources;
        iface->cb_8c = JungleCruiseSetEditMode;
        iface->cb_90 = FUN_00435150;
        iface->cb_94 = FUN_00435230;
        iface->cb_98 = JungleCruiseAddObject;
        iface->cb_9c = JungleCruiseRemoveObject;
        iface->cb_a8 = FUN_00435750;
        iface->cb_b0 = RenderJungleCruise;
        iface->cb_bc = JungleCruise_Save;
        iface->cb_b8 = JungleCruise_Load;
        iface->cb_c0 = FUN_00436160;
    }
    // STRING: LEGOLAND 0x004b88b8
    else if (_stricmp("JUNGLE CRUISE MONKEY TREE", head->name) == 0) {
        iface->cb_a4 = LoadBrijMaskSprite;
        iface->cb_ac = KillBrijMaskSprite;
        iface->cb_8c = MonkeyTreeSetEditMode;
        iface->cb_90 = FUN_00433d90;
        iface->cb_94 = FUN_00433fa0;
        iface->cb_98 = MonkeyTreeAddObject;
        iface->cb_9c = MonkeyTreeRemoveObject;
        iface->cb_a0 = GetMonkeyTreeSpriteInfo;
    }
    // STRING: LEGOLAND 0x004b889c
    else if (_stricmp("JUNGLE CRUISE MONKEY FISH", head->name) == 0) {
        iface->cb_a4 = LoadMFish2Sprite;
        iface->cb_ac = KillMFish2Sprite;
        iface->cb_8c = MonkeyFishSetEditMode;
        iface->cb_90 = FUN_00434330;
        iface->cb_94 = FUN_00434650;
        iface->cb_98 = MonkeyFishAddObject;
        iface->cb_9c = MonkeyFishRemoveObject;
        iface->cb_a0 = GetMonkeyFishSpriteInfo;
    }

    CastleLevel1_GetInterfaces(head, iface);
    LogFlume_GetInterfaces(head, iface);
    CoptersRide(head, iface);
    FortGetInterfaces(head, iface);
    GoldRush_GetInterfaces(head, iface);
    Temple_GetInterfaces(head, iface);
    Catapult_GetInterfaces(head, iface);
    Joust_GetInterfaces(head, iface);
    TempleSlide_GetInterfaces(head, iface);
    SpiderRide(head, iface);
    SafariRideGetInterfaces(head, iface);
    WaterWorksGetInterfaces(head, iface);
    GardenGetInterfaces(head, iface);
    ShopsGetInterfaces(head, iface);
    SpaceTowerRide(head, iface);
    SpinningBarrelsGetInterfaces(head, iface);
    PlaneRide_GetInterfaces(head, iface);
    CastleGetInterfaces(head, iface);
}
