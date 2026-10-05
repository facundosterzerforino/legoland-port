# Remaining work (as of 2026-10-05, after round 6)

`./tools/verify` reported `Progress: 94.72%` for this build. That figure is the sum of every function's match score divided
by all 3634 functions reccmp counts, which include CRT and import entries. Counted per game function (`// FUNCTION:`):

| State | Functions |
|---|---|
| 100% (exact or "effective") | 2958 |
| Pure-C partials (1-99%) | 248 |
| Inline asm, partial | 5 + 3 (see below) |
| Inline asm, still `STUB()` (0%) | 44 |
| Total | 3255 |

Recount with `uv run tools/agent/status.py` (per function) and `uv run tools/agent/partials.py` (pure-C partials).

## Inline-asm functions: match with `__asm`

**All 44 functions still at 0% are hand-written assembly in the original.** MSVC6 never emits these instructions, so pure C
cannot match them (see "Functions With an ebp Frame" in `decomp-tips.md`); they need inline `__asm`, allowed since
2026-10-05. Each still at `STUB()` has a comment that names the fingerprint. The port replaces them with plain-C equivalents tagged `// [library:asm]`.
`find_inline_asm` in `tools/progress.py` detects them, and `partials.py` leaves them out.

| TU | Count | Functions (fingerprint) |
|---|---|---|
| castle | 21 | FUN_0041e130, FUN_00420e90, FUN_00423140, FUN_004234e0, FUN_00428cb0, FUN_004292f0 (rdtsc); FUN_0041f8d0, FUN_0041fa10, FUN_0041fba0, FUN_0041fd80, FUN_0041ff80, FUN_00423350, FUN_00428860 (xchg); FUN_00420810, FUN_00420a20, FUN_00420c40 (fistp + rdtsc); FUN_004236f0, FUN_00423730 (fldcw/fstcw); FUN_00426250, FUN_004263a0, FUN_0042a2f0 (fistp) |
| draw | 13 | FUN_00464480 (xchg, rep movsw/stosw); ZBufferHelper, FUN_00465240 (pusha, shrd, xchg); FUN_00464ee0, SoftPrint_XBltFast (pusha); FUN_00466d80, FUN_00467180, FUN_004673f0, FUN_004677b0, FUN_00467b00, FUN_00467d10, FUN_00468040, FUN_00468410 (rol, rep movsw/stosw) |
| render | 4 | FUN_00486590, FUN_004877b0 (shrd, xchg); FUN_00486c70, FUN_00487d40 (fistp, shrd, xchg) |
| man3d | 3 | FUN_0043fa80, SetPersonRotation (fistp); FUN_00440a30 (fistp, shrd) |
| render3d | 2 | FUN_00441980 (fistp); TransformVectorsL (shrd) |
| copters | 1 | FUN_00404630 (fistp) |

Inline asm, partly written in C (finish with `__asm`): Render3DPerson 51.6%, RenderTransSprite 18.8%, SoftPrint_Clear 18.9%,
FUN_00488730 17.5%, ApplyObjectOrientationToPerson 7.0%; HASM_lego_sqrtf and HASM_lego_invsqrtf 95.2% (`__declspec(naked)`),
LLSPlay 16.2% (`int 3`). Inline asm already at 100%: _ftol, RenderingComplete, ReadBigEndianU32, ReadBigEndianU16.

## The untried partials: round 6 (2026-10-05)

All 47 functions on the round-5 "not attempted yet" list (plus FUN_0042a020 and FUN_00485fe0) got one pass of up to three
variants each. `./tools/verify` went from 94.72% to 94.76%; 2961 game functions now match.

Matched (100%): CoptersUpdate (dropped the two `volatile` locals, own local for the queue index), RES_OpenFile and
RES_OpenFileFromVolume (pointers set before the first call, `while` loop for leading `.\\`, refcount through
`file->volume`).

Improved: LegoShop2Update 46.9 -> 71.6 (y computed before x), RenderBuildObjectIcon 81.7 -> 83.7, PrintCertificate
79.4 -> 81.2, DrawNewObjectPopup 62.2 -> 63.7 (inlined icon-placement helper), FindPathPosAtRangeAhead 73.0 -> 74.5,
PlaneRideUpdate 78.9 -> 79.1, LogFlumeEntranceAddObject 61.7 -> 62.1.

FUN_00485fe0 is inline asm (ebp frame + `rep stosd` clear block) that `find_inline_asm` misses: match it with `__asm`.

Named (28, all scores unchanged by the rename): the per-frame ride callbacks (`cb_a8`) PlaneRideUpdate, LogFlumeEntranceUpdate,
DrivingSchoolUpdate, TempleSlideUpdate, SafariRideUpdate, ExplorersInstituteUpdate, LegoShop2Update, SpaceTowerUpdate,
SpinningBarrelsUpdate, OctopusCafeUpdate, Restaurant2Update, JoustUpdate, FortUpdate; FortWanderUpdate, AdvanceFlumeMover,
EdgeMeshRemoveBackfaces, JungleCruiseBuildStepPath, BoatingSchoolBuildStepPath, GetQueueTurn, PickQueueTurn,
FindPathPosAtRangeAhead/Behind, FindMapPathAStar, FileSelectDialog, ListBoxDialog, SetPersonYawFromDir16,
SearchBoatPathConnected, RemapTexCoordsToCell.

Still partial, with what blocks them (no further attempts unless there is a new idea):

| % | Function | What differs |
|---|---|---|
| 95.48 | RenderCursor | two values in different registers only |
| 95.06 | EdgeMeshRemoveBackfaces | x87 operand order (orig keeps dx1/dy1 on the FPU stack) |
| 83.61 | JungleCruiseBuildStepPath | straight case placed first via a ternary; rewrites scored lower |
| 81.26 | BoatingSchoolBuildStepPath | same as the jungle version |
| 79.72 | GetQueueTurn | orig keeps three separate fallback calls; merging the tests makes ours tail-merge |
| 78.63 | LogFlumeEntranceUpdate | ride in ebx/stack instead of esi |
| 77.47 | FortWanderUpdate | node kept in edi for the whole function, param slots reused |
| 76.96 | DrivingSchoolUpdate | constant 7 kept in ebx |
| 76.47 | FortUpdate | tile x kept as an int in ebp |
| 75.35 | TempleSlideUpdate | node kept on the stack |
| 75.09 | SafariRideUpdate | register allocation |
| 74.30 | SpinningBarrelsUpdate | frame 12 bytes larger, ebx/ebp swapped |
| 72.33 | ExplorersInstituteUpdate | ours merges case 0 into the identical case 3 |
| 70.04 | FindMapPathAStar | point/result kept in dead parameter slots (would need `(&a)[1]`, UB in the port) |
| 69.93 | ParseScriptResFile | `cmd` in ebp, reloaded from the parameter at the loop end |
| 69.44 | ListBoxDialog | ebx/edi swapped, -1 kept in esi |
| 69.03 | FUN_00402780 | scheduling around the inlined MapToPlayfield |
| 67.67 | FUN_00423a10 | two zero registers, different local slots |
| 67.51 | SpaceTowerUpdate | ride re-read from the stack |
| 63.32 | SetPersonYawFromDir16 | `&field_40` in esi and 0 in edx across the switch |
| 61.81 | PickQueueTurn | `rand() & 3` in ebx, t1..t3 on the stack |
| 61.48 | LoadPos | frame pointer strength-reduced to `&f->mat` |
| 60.26 | PrintProfileDetails | 0x400 kept in ebp |
| 56.52 | SearchBoatPathConnected | tx/ty in registers, `found` reloaded |
| 54.66 | FileSelectDialog | dir/fname/ext buffers in another stack order (declaration order and names have no effect) |
| 41.17 | RemapTexCoordsToCell | FPU-heavy, not attempted |
| 40.52 | AdvanceFlumeMover | our build turns the `if (sub == N)` chain into jumps to the end |
| 38.41 | SearchJunglePathConnected | same as SearchBoatPathConnected |

Not attempted this round (large): LoadBaseMap 82.0, RenderFullMap 81.6, JoustUpdate 69.0, FUN_00466770 61.5,
OctopusCafeUpdate 60.9, FUN_004608c0 56.5, FUN_0040ae90 45.1, Restaurant2Update 41.9.

Patterns that worked this round: statement order matters around calls (initialise pointers where the original does, before
the first call); remove `volatile` workarounds left from earlier rounds and re-score; when the original's registers for two
values are swapped, try computing them in the other order.

Also not attempted, but triaged as unlikely to match from C (`agent-workflow.md`, round 5): 37 layout-dependent and
8 register-only partials.
