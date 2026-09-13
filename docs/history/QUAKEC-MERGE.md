# Quake rerelease gameplay fixes

LOCAL-008. Review and C adaptation: OpenAI Codex, 2026-09-13, at the user's
request. Scope: useful GPL QuakeC fixes without dependencies on closed engine
implementations or rerelease assets. This is a selective port, not wholesale
replacement with the rerelease's game logic.

## Integration target and pinned sources

The existing local nQuake map tests load `qw/ktx.pk3:qwprogs.qvm`, not the original
Quake `progs.dat`. ezQuake provides the game VM/server but has no `.qc` source
package. KTX already translates the shared Quake gameplay to C. Consequently the
applicable QuakeC fixes are adapted into `ktx/src/`, built as an API-16 native
`qwprogs.dll`, and loaded by ezQuake's existing native game-module support. No
ezQuake engine source change is needed for this integration.

| Repository | Pinned revision | Role |
|---|---|---|
| [id-Software/quake-rerelease-qc](https://github.com/id-Software/quake-rerelease-qc) | `634eefab09a77eb7b5f5ca7078ba3d8784a91142` | Unmodified GPL reference with its history and COPYING.txt |
| [QW-Group/ktx](https://github.com/QW-Group/ktx) | `631584f7fac3c3891826224d36d873c99880d353` | Latest fetched master, local `dev/rerelease-fixes` branch, KTX 1.48-dev |

The user explicitly requested upgrading KTX to its newest source branch.
The final module therefore updates the bundled **1.46-dev** QVM (May 22, 2025)
to **1.48-dev** from master, plus the three local corrections. It includes the
intervening upstream KTX changes; it is not merely a rebuild of the old QVM.
Native DLL and QVM are different compilation targets. The old QVM remains in
the untouched archive for rollback. Exact upstream authorship/history and both
source pins are recorded separately from the local adaptations.

## Imported behavior

Apply `patches/ktx-quakec-fixes.patch` to the pinned KTX base. It changes two files.

| ID | QuakeC reference | KTX adaptation and effect |
|---|---|---|
| CODE-011a | `quakec/doors.qc:door_fire` | Use CHAN_ITEM for the key-unlock sound. The immediately following CHAN_VOICE movement sound no longer replaces it. The sound resource is unchanged. |
| CODE-011b | `quakec/doors.qc:func_door_secret`, `fd_secret_done` | Install the secret-door death callback both at spawn and after closing. A lethal first hit now activates the door. KTX already guarded null death callbacks, so its symptom was a stuck door, not the original QuakeC crash. |
| CODE-012 | `quakec/subs.qc:SUB_UseTargets` | Finish the killtarget loop, then dispatch normal targets. Previously any non-null killtarget returned early, even if no matching entity existed. Delayed events and activator restoration are preserved. |

The C adaptation of CODE-011b adds `fd_secret_activate(void)`, shared by the
use/death slots, calling the existing pain-compatible function with explicit
unused arguments. QuakeC's flexible callback convention must not become a C call
through an incompatible function type. The existing pain slot keeps its typed
`(gedict_t *, float)` callback. Targeted doors remain non-shootable unless their
existing `always_shoot` flag requests it. No movement constants, damage balance,
map/model names, asset contents, protocol opcodes or engine built-ins are added.

## Reviewed exclusions

| Candidate | Decision |
|---|---|
| Respawn velocity reset | Already present in KTX's player initialization. No duplicate patch. |
| Fish double-count prevention | KTX uses `common_monster_start` for the count and `swimmonster_start_go` does not count again. No duplicate patch. |
| Update 4 SetMovedir/trigger angle correction | KTX already uses ordinary `trap_makevectors` in these paths. Do not import the earlier rerelease pitch inversion. |
| Shub normal-damage guard | KTX already uses `nopain` to restore the boss's health; rerelease uses SUB_Null plus a different guard. Keep the existing KTX solution. End-map validation would also require registered assets not installed here. |
| Nightmare health/attack timing, weapon selection and cheats | Gameplay policy changes, not needed for the Vulkan/client project. Preserve KTX's behavior. |
| Localization (`ex_bprint`, `ex_sprint`, `ex_centerprint`) | Requires new engine handling/localization data. Not imported. |
| Achievements, finale handling, bot navigation, debug drawing, player EX flags | Depend on rerelease-specific services, messages or named engine built-ins. Not imported. KTX's own bot system is unchanged. |
| Extended gib movement, extended effects and rerelease model behavior | Requires additional engine features and/or resource assumptions. Not imported. |
| Mission-pack weapons/monsters, Horde, CTF, MachineGames map-specific entities | No wholesale import. These have different gameplay, map/resource and engine dependencies; existing KTX modes remain the target. |

This review identifies the directly applicable shared fixes above. It is not a
claim that every line of every expansion has been exhaustively compared, or that
this project now supports the Quake rerelease's content or networking.

## Build, activation and rollback

From the workspace root:

```powershell
.\scripts\Build-KTX.ps1 -Configuration Debug
.\scripts\Test-KTX.ps1
.\scripts\Build-KTX.ps1 -Configuration Release
.\scripts\Build-KTX.ps1 -Configuration Release -Install
```

`-Install` directs the linker output to `gamedata/nquake/qw/qwprogs.dll`. There is
no binary copy to keep in sync; use the same install command after changing KTX.
Debug stays under `build/ktx-msvc/Debug/`. Existing nQuake PAK/PK3 files and user
configs remain untouched. The existing launchers then use the patched module
for local KTX games (the default native-first `sv_progtype 1` path). Remote
servers still run their own game logic; this does not change their rules.

This also affects other ezQuake builds launched against this same nQuake folder.
The old Single Player menu's separate `spprogs.dat` mode is not replaced. The
ordinary local `map`/KTX path is the tested integration point.

To use the bundled QVM for a comparison, set `sv_progtype 2` before starting a
local map. To roll back permanently, close the game and move only the newly
built `qwprogs.dll` out of the `qw` search directory; the untouched `ktx.pk3`
still contains the original module. Debug/Release module updates require closing
any game process using that DLL first.

## Verification and authorship

The behavior suite compiles eight complete functions, including the local typed
wrapper, using the real KTX headers and recording engine-boundary stubs.
It exercises 19 scenarios covering target deletion/dispatch order, missing and
multiple killtargets, a null use callback, delayed dispatch, restored activator
and other globals, key/movement sound coexistence, ordinary/targeted/always-shoot
secret doors, first lethal activation, rearming and normal pain activation.
The same fixture reproduces all three regression groups in pristine Git HEAD.
It is not a replacement for a full engine physics/network test.

Build/runtime results and remaining limits are recorded in VALIDATION.md.
Source copyrights remain with id Software and the KTX contributors. The rerelease
Git history identifies the source publisher/committer; it does not establish
individual authorship of each fix. OpenAI Codex performed the C adaptation,
callback wrapper, tests and local documentation. No human review, commit identity,
upstream acceptance or publication is implied. See ATTRIBUTION.md and the source
manifest for CODE-011/012; all source histories and notices are retained.
