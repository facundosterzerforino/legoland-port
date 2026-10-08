# Attempt log for unmatched functions

One file per function that is not yet at 100%, named `<address>-<name>.md`. Each says what still differs
from the original, what has already been tried (with the score it gave), and ideas nobody has tried yet.

**Before working on a function, read its file and don't repeat an attempt listed there** unless you are
changing something else at the same time. **After working on it, add your attempts** to the table, even the
ones that failed: date, who (person, or model for agents), the change, the score. Update "Best" and "What
still differs" when they change. When a function reaches 100%, delete its file.

## What applies to most of them

- **Declaration order almost never changes MSVC6's register allocation.** Nearly every agent tried
  reordering locals; it changed nothing in all but one case. Don't spend cycles on it.
- **A frame-size difference (`sub esp, N`) means a local is missing or extra.** MSVC drops locals that are
  never used, so a declared-but-unused variable doesn't count. Find the real variable behind the
  original's extra slot (look at which `[esp+...]` offsets it uses). Don't add fake padding locals.
- **The original has `push ebp; mov ebp, esp` and no asm-only instructions?** Wrap the function in
  `#pragma optimize("y", off)` / `#pragma optimize("y", on)` (`decomp-tips.md`). FUN_00485fe0 went from 33% to 77%
  with that alone.
- **Late pushes of callee-saved registers** (after an early check) usually mean the rest of the function is
  inside an `if (...) { }` block in the source, not after an early `return`.
- **`repe cmpsd` comes from `memcmp`**, `rep stosd` from `memset` with a constant size (FUN_00452030).
- **MSVC already turns a tail call to the same function into a loop**; writing the loop by hand made
  SearchJunglePathConnected worse.
- **Hand-written asm:** `tools/asm2naked.py` writes `lea reg, [ebp-x]` with a 1-byte displacement; if the
  original uses the 4-byte form, the function comes out shorter and every later jump shifts. Emit those
  instructions as raw bytes with `_emit` (RenderTransSprite).
- **reccmp false positives:** a constant that falls inside a data symbol's range in only one of the two
  images shows as a diff on a byte-identical line (`decomp-tips.md`).

## Index

| Address | Function | File | Best |
|---|---|---|---|
| 0x004019c0 | [FUN_004019c0](0x004019c0-FUN_004019c0.md) | ride_bloke.c | 92.08% |
| 0x00404630 | [CoptersPlaceRider](0x00404630-CoptersPlaceRider.md) | copters.c | 54.39% |
| 0x00407ad0 | [JoustRemoveObject](0x00407ad0-JoustRemoveObject.md) | joust.c | 95.00% |
| 0x00408f90 | [FUN_00408f90](0x00408f90-FUN_00408f90.md) | log_flume.c | 40.96% |
| 0x0040abf0 | [LogFlumeEntranceRemoveObject](0x0040abf0-LogFlumeEntranceRemoveObject.md) | log_flume.c | 95.10% |
| 0x0040ae90 | [FUN_0040ae90](0x0040ae90-FUN_0040ae90.md) | log_flume.c | 45.11% |
| 0x0040bab0 | [FUN_0040bab0](0x0040bab0-FUN_0040bab0.md) | log_flume.c | 44.09% |
| 0x0040be00 | [FUN_0040be00](0x0040be00-FUN_0040be00.md) | log_flume.c | 90.61% |
| 0x0040c250 | [FUN_0040c250](0x0040c250-FUN_0040c250.md) | log_flume.c | 42.86% |
| 0x0040c8d0 | [FUN_0040c8d0](0x0040c8d0-FUN_0040c8d0.md) | log_flume.c | 92.59% |
| 0x0040f5b0 | [FUN_0040f5b0](0x0040f5b0-FUN_0040f5b0.md) | log_flume.c | 91.50% |
| 0x00411680 | [AdvanceFlumeMover](0x00411680-AdvanceFlumeMover.md) | log_flume.c | 40.52% |
| 0x00413450 | [FUN_00413450](0x00413450-FUN_00413450.md) | roads.c | 90.00% |
| 0x00415ae0 | [RenderSpider](0x00415ae0-RenderSpider.md) | spider_ride.c | 98.62% |
| 0x00416fa0 | [RenderTempleSlide](0x00416fa0-RenderTempleSlide.md) | temple_slide.c | 95.00% |
| 0x0041a720 | [FUN_0041a720](0x0041a720-FUN_0041a720.md) | boating_school.c | 94.41% |
| 0x0041df00 | [FUN_0041df00](0x0041df00-FUN_0041df00.md) | castle.c | 44.94% |
| 0x0041ee40 | [FUN_0041ee40](0x0041ee40-FUN_0041ee40.md) | castle.c | 90.32% |
| 0x0041ef60 | [FUN_0041ef60](0x0041ef60-FUN_0041ef60.md) | castle.c | 43.10% |
| 0x00421e90 | [FUN_00421e90](0x00421e90-FUN_00421e90.md) | castle.c | 99.09% |
| 0x004227c0 | [EdgeMeshRemoveBackfaces](0x004227c0-EdgeMeshRemoveBackfaces.md) | castle.c | 95.06% |
| 0x00425e20 | [FUN_00425e20](0x00425e20-FUN_00425e20.md) | castle.c | 90.70% |
| 0x00429cf0 | [FUN_00429cf0](0x00429cf0-FUN_00429cf0.md) | castle.c | 97.27% |
| 0x0042aa90 | [FUN_0042aa90](0x0042aa90-FUN_0042aa90.md) | balloonz.c | 94.20% |
| 0x0042bcf0 | [RenderCarousel](0x0042bcf0-RenderCarousel.md) | carousel.c | 42.80% |
| 0x0042fbb0 | [Restaurant2Update](0x0042fbb0-Restaurant2Update.md) | eatery.c | 41.89% |
| 0x00430b10 | [RenderRestaurant2](0x00430b10-RenderRestaurant2.md) | eatery.c | 97.36% |
| 0x00437260 | [SearchJunglePathConnected](0x00437260-SearchJunglePathConnected.md) | jungle_cruise.c | 38.41% |
| 0x0043a7a0 | [FUN_0043a7a0](0x0043a7a0-FUN_0043a7a0.md) | space_tower.c | 37.65% |
| 0x0043be70 | [RenderSpinningBarrels](0x0043be70-RenderSpinningBarrels.md) | spinning_barrels.c | 99.20% |
| 0x0043da60 | [RenderPlaneRide](0x0043da60-RenderPlaneRide.md) | plane_ride.c | 95.45% |
| 0x0043e110 | [LoadZoomer](0x0043e110-LoadZoomer.md) | plane_ride.c | 97.89% |
| 0x00442040 | [RemapTexCoordsToCell](0x00442040-RemapTexCoordsToCell.md) | render3d.c | 41.17% |
| 0x004431f0 | [GetLegColourOfBloke](0x004431f0-GetLegColourOfBloke.md) | render3d.c | 25.00% |
| 0x00443220 | [GetArmColourOfBloke](0x00443220-GetArmColourOfBloke.md) | render3d.c | 25.00% |
| 0x00443bd0 | [OpenAviAnim](0x00443bd0-OpenAviAnim.md) | challenge.c | 91.98% |
| 0x004453a0 | [RunAppraisal](0x004453a0-RunAppraisal.md) | challenge.c | 15.39% |
| 0x0044e010 | [__BMPLoader](0x0044e010-__BMPLoader.md) | gfx.c | 39.57% |
| 0x0044fe80 | [FUN_0044fe80](0x0044fe80-FUN_0044fe80.md) | bloke_ai.c | 94.89% |
| 0x00451390 | [FUN_00451390](0x00451390-FUN_00451390.md) | cdcheck.c | 90.00% |
| 0x00452030 | [FUN_00452030](0x00452030-FUN_00452030.md) | controller.c | 33.15% |
| 0x00455370 | [BubbleHelp](0x00455370-BubbleHelp.md) | text.c | 24.23% |
| 0x00457a70 | [FUN_00457a70](0x00457a70-FUN_00457a70.md) | bricks.c | 100% effective match (reccmp), bytes still differ |
| 0x00459970 | [FUN_00459970](0x00459970-FUN_00459970.md) | gamemap.c | 94.83% |
| 0x0045acc0 | [GetTileBounds](0x0045acc0-GetTileBounds.md) | tilemap.c | 28.30% |
| 0x0045ad60 | [GetTileCentre](0x0045ad60-GetTileCentre.md) | tilemap.c | 93.02% |
| 0x0045ade0 | [FUN_0045ade0](0x0045ade0-FUN_0045ade0.md) | tilemap.c | 43.41% |
| 0x0045bcd0 | [PointToIsoPlane](0x0045bcd0-PointToIsoPlane.md) | tilemap.c | 25.70% |
| 0x0045c9c0 | [FUN_0045c9c0](0x0045c9c0-FUN_0045c9c0.md) | tilemap.c | 49.38% |
| 0x0045ca90 | [FUN_0045ca90](0x0045ca90-FUN_0045ca90.md) | tilemap.c | 42.86% |
| 0x0045d5d0 | [FUN_0045d5d0](0x0045d5d0-FUN_0045d5d0.md) | tilemap.c | 42.28% |
| 0x0045d770 | [FUN_0045d770](0x0045d770-FUN_0045d770.md) | tilemap.c | 67.69% |
| 0x0045da60 | [RestoreBaseMap](0x0045da60-RestoreBaseMap.md) | tilemap.c | 95.65% |
| 0x0045dd80 | [AddObjectToMap](0x0045dd80-AddObjectToMap.md) | map_object.c | 99.10% |
| 0x0045eb30 | [BuildObject](0x0045eb30-BuildObject.md) | map_object.c | 96.81% |
| 0x0045ed30 | [ObjectIsBuilt](0x0045ed30-ObjectIsBuilt.md) | map_object.c | 93.42% |
| 0x0045f810 | [ValidateCursor](0x0045f810-ValidateCursor.md) | map_object.c | 92.01% |
| 0x0045fad0 | [FUN_0045fad0](0x0045fad0-FUN_0045fad0.md) | map_object.c | 93.59% |
| 0x0045fca0 | [FUN_0045fca0](0x0045fca0-FUN_0045fca0.md) | map_object.c | 94.76% |
| 0x0045ff00 | [RenderCursor](0x0045ff00-RenderCursor.md) | map_object.c | 95.48% |
| 0x00461290 | [FUN_00461290](0x00461290-FUN_00461290.md) | map_object.c | 43.92% |
| 0x00462ef0 | [DoMapAI](0x00462ef0-DoMapAI.md) | map_object.c | 93.43% |
| 0x00465850 | [FUN_00465850](0x00465850-FUN_00465850.md) | draw.c | 34.23% |
| 0x00469c80 | [ScriptEventClear](0x00469c80-ScriptEventClear.md) | objectives.c | 93.18% |
| 0x0046c7e0 | [LoadObjectiveEventList](0x0046c7e0-LoadObjectiveEventList.md) | nerps.c | 98.29% |
| 0x0046f9a0 | [FUN_0046f9a0](0x0046f9a0-FUN_0046f9a0.md) | icon.c | 41.45% |
| 0x00471ca0 | [RemoveNewObject](0x00471ca0-RemoveNewObject.md) | popupinfo.c | 63.16% |
| 0x00471d90 | [FUN_00471d90](0x00471d90-FUN_00471d90.md) | popupinfo.c | 96.67% |
| 0x00476d20 | [FUN_00476d20](0x00476d20-FUN_00476d20.md) | interface.c | 92.62% |
| 0x0047cba0 | [LLIDB_LoadTSFData](0x0047cba0-LLIDB_LoadTSFData.md) | llidb.c | 90.58% |
| 0x0047f880 | [GameMain](0x0047f880-GameMain.md) | debug.c | 99.62% |
| 0x00484a70 | [SetBlokePositionFromBNV](0x00484a70-SetBlokePositionFromBNV.md) | bloke.c | 94.15% |
| 0x004856a0 | [PrintSpriteEx](0x004856a0-PrintSpriteEx.md) | print_sprite.c | 96.64% |
| 0x00489750 | [RES_OpenVolume](0x00489750-RES_OpenVolume.md) | resource.c | 92.37% |
| 0x0048dd00 | [PrintSavedGameDetails](0x0048dd00-PrintSavedGameDetails.md) | savegame_ui.c | 92.33% |
| 0x004921c0 | [ConvertWaveToPcm16](0x004921c0-ConvertWaveToPcm16.md) | sound_sfx.c | 92.26% |
| 0x00492380 | [CreateSampleFromWAV](0x00492380-CreateSampleFromWAV.md) | sound_sfx.c | 97.49% |
| 0x00498420 | [SpeechParseWavHeader](0x00498420-SpeechParseWavHeader.md) | stream.c | 93.19% |
| 0x0049a7f0 | [Mechanic_Build](0x0049a7f0-Mechanic_Build.md) | worker.c | 96.15% |
