# Windows Starter

DIST-002, 2026-09-16. Requested by the user; installer, packaging and validation
implemented by OpenAI Codex. This is an unofficial nQuake-based distribution.

Download the Starter ZIP, extract it and run `Install.cmd`. Choose a new directory;
setup downloads about 125 MB, verifies each archive and creates a complete
installation with the pinned Vulkan raster engine. Run the installed `Start.cmd`.
An existing nQuake installation, compiler, Python or Vulkan SDK is not required.
Windows x64, Windows PowerShell 5.1 and a Vulkan-capable graphics driver are required.

The four nQuake archives come directly from its official `distfiles` release.
The engine comes from the 0.2.0-beta.1 Friends test release; the exact commit is pinned in the lock.
`dist/windows/starter/downloads.lock.json` pins origins, sizes and SHA-256 values.
Mutable upstream snapshot URLs are never trusted without matching these hashes.
A replaced snapshot requires a reviewed lock update and new Starter version.
Starter 0.2.0 includes source scripts, documentation, Quick profiles and the five
user-authorized legacy crosshair PNGs. See `crosshairs.json` for attribution/hashes.
The engine includes reusable Friends invitations and native Windows link handling.
Setup offers per-user link registration after the final installation is in place;
non-interactive setup can request this with `-RegisterLinks`. Registration is also
available under Friends -> Windows links. Re-enable it after moving the folder.
`starter-install.json` records profile hashes and the pinned download versions.
Downloaded resource packs are not mirrored or relicensed by this project.

## Installation and configuration

- Existing destination directories are rejected, even if empty. Work occurs in
  a new sibling staging directory; the finished install is renamed into place.
- Verified downloads are cached in `%LOCALAPPDATA%/ezQuake-Vulkan/downloads`.
  A failed install retains its staging directory for diagnosis and does not publish
  a partially complete destination. There is no automatic removal of user files.
- nQuake's original `autoexec.cfg` and initial `config.cfg` are preserved. Its own
  first-run flag loads `nquake_default.cfg`, then our generated `preset.cfg` once.
  That preset contains Balanced's 170 fields and Quick WASD. Subsequent config
  saves and manual autoexec edits keep their ordinary precedence.
- The installed launchers use relative directories and `-nohome`. They wait for
  the actual game process before creating the first-run marker or collecting logs.
  This fixes the premature-exit behavior found in the beta.1 portable launcher
  during Windows PowerShell testing. The launcher uses the pinned Friends-enabled engine.
- `Start.cmd -Windowed` is supported. Advanced users can run a CFG from `qw`
  after initialization with `Start.cmd -StartupConfig my-settings.cfg`.
- `Diagnose.cmd` starts windowed, enables the console log and copies diagnostics
  into `engine/logs`. No game is launched automatically during installation.

## Optional registered-data import

Select the user's classic full Quake root or `id1` directory. The importer checks
the numbered `pak1.pak` and later PAKs for valid directories and collectively for
`gfx/pop.lmp` and `maps/e2m1.bsp`. This identifies suitable content, not ownership.
It supports both standard and numbered repacked installations. It copies those
PAKs under their original names, verifies hashes, and never copies the user's
configs, executables or unrelated loose addons. The downloaded shareware pak0
remains in place. Importing a source with modified PAKs also imports those changes.
The installed `gpl_maps.pk3` is retained as `.disabled` so owned maps take priority;
nQuake's other enhancements remain installed. Rerelease assets are not supported.

With no import, shareware Episode 1 and nQuake multiplayer content are available.
Local Arena uses nQuake's bundled KTX 1.46-dev QVM with bot navigation; it does not
depend on the development workspace's patched KTX DLL. Full-game PAKs in the
resulting installation must never be uploaded as part of our public package.

## Validation and reproduction

Windows PowerShell 5.1 was used, including paths containing spaces. Tests cover
fresh installation, all 170 Balanced fields, Quick WASD, first-run marking,
diagnostics, later config/autoexec precedence, start/E1M1 with monsters and saving,
and a DM6 Local Arena with a bot using the downloaded QVM. Integrity and archive
tests cover corrupt cache rejection, traversal and Windows path alias rejection,
preflight before extraction, existing-directory protection, full-data import and
repeat-import refusal. The pinned engine archive was also downloaded live over HTTPS.
See `provenance/starter-validation.json` for the final evidence summary.

The pinned nQuake default CFG refers to the unsupported `r_fx_geometry` cvar;
its startup warning is inherited and does not invalidate the verified defaults.
These checks are not an all-GPU qualification or a full campaign playthrough.

Build the Starter installer archive:

```text
python tools/Package-Starter.py --output output
```

Use `tools/Test-Starter.ps1` with fresh work/cache directories for the integrity
and import tests. `tools/Test-StarterRuntime.ps1` requires a disposable Starter
installation: it deliberately changes test configs and creates save files.
Never point runtime tests at a player's normal installation.

## GitHub distribution

GitHub Releases is intended for distributable software and binary attachments.
Each attachment must be below 2 GiB. GitHub's general acceptable-use rules still
apply, including intellectual property and excessive-bandwidth provisions.
Source: https://docs.github.com/en/repositories/releasing-projects-on-github/about-releases
and https://docs.github.com/en/site-policy/acceptable-use-policies/github-acceptable-use-policies
(checked 2026-09-16). Starter publishes our installer and the approved crosshair/profile payload; third-party downloads
retain their own licenses. This does not grant blanket redistribution rights to
the fully installed directory. Original upstream notices are preserved.
