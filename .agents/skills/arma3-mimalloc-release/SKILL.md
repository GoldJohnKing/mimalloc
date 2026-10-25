---
name: arma3-mimalloc-release
description: Use when updating the Arma 3 CMA fork of mimalloc: merging the latest upstream v3 stable release, reviewing runtime and build changes, checking the VS/Intel toolchain, building and verifying the DLL in Arma 3, or publishing a GitHub release.
---

# Arma 3 mimalloc: update, build, verify, release

Repository: `GoldJohnKing/mimalloc`; upstream remote: `microsoft`; target branch: `Arma-3-v3`.

Preserve all `Arma 3 CMA` patches, especially those in `src/cma/*`, `src/static.c`, `src/heap/init.c`, `src/util/options.c`, `CMakeLists.txt`, and `ide/Arma-3/*`. Adapt them to upstream changes rather than blindly choosing one side of a conflict.

`{VERSION}` is the upstream version without its leading `v` (e.g. `3.6.0`); `{MMP}` is the same version with dots removed (e.g. `3.6.0` → `360`), used only in the DLL filename. `{YYYYMMDD}` is the build date embedded in the produced DLL filename. Reuse that exact string in the tag, even if publishing happens on a later date:

- DLL: `mimalloc_v{MMP}_{YYYYMMDD}.dll`
- Tag: `Arma-3-v{VERSION}-{YYYYMMDD}`

Resolve machine-specific paths and installed versions from the host; never hardcode them.

## 1. Merge upstream

Verify that the working tree is clean before switching branches; if it is not clean, stop and ask the user how to proceed. Switch to `Arma-3-v3`, then fetch tags from `microsoft` and select the latest stable `v3.*` release by semantic version.

Merge that tag into `Arma-3-v3`. Resolve conflicts while preserving and adapting the CMA patches.

**Accept:** The merge is committed and the working tree is clean.

## 2. Review the merged changes

Review the upstream release notes at <https://github.com/microsoft/mimalloc#releases>, the commit log of the merged range, and the complete merge diff, covering both runtime behavior and build configuration.

Since `ide/Arma-3/mimalloc.sln` is the build entry point, upstream changes to `CMakeLists.txt` and other build files do not automatically apply to the Visual Studio project. Diff them against the `.vcxproj`/`.sln` (sources, includes, defines, compiler/linker options) and update it where needed.

Present the main upstream changes, any required Visual Studio project adaptations, and your assessment of the three categories below (including "none" verdicts), then **stop and wait for the user's approval** before building anything:

- **Build-method change.** Upstream changed how it builds the binary (for example, single-source vs multi-file compilation, or enabling link-time optimization). Present the difference and ask whether to follow upstream in the Visual Studio project.
- **Build-time switch.** Upstream added or changed a build-time switch (for example, a macro-defined feature). Explain its purpose, upstream default, and the difference between enabled and disabled, and ask how to configure it.
- **Runtime option.** A new or changed runtime option or default (`mi_option_t` in `include/mimalloc.h`). Summarize its upstream default and likely impact, and ask whether to tune it in `src/cma/cma_utils.c` or keep the default.

**Accept:** The merge is reviewed and the Visual Studio project is adapted; the user has approved the three-category assessment before building.

## 3. Check the toolchain

Detect the installed Visual Studio version using `vswhere.exe` under `%ProgramFiles(x86)%\Microsoft Visual Studio\Installer`.

Detect installed Intel C++ Compiler versions under `%ProgramFiles(x86)%\Intel\oneAPI\compiler\`, and identify the version actually used by the build. Compare the installed versions with the latest stable official releases.

- Intel C++ Compiler: <https://www.intel.com/content/www/us/en/developer/tools/oneapi/dpc-compiler-download.html>
- Visual Studio: <https://learn.microsoft.com/en-us/visualstudio/releases/2026/release-notes>

If either tool is outdated, report the difference and ask the user to update it manually or explicitly choose to continue. Never install or update tools yourself. If a tool is updated, recheck its installed version.

Verify that the project's `PlatformToolset` matches the installed Intel compiler version used by the build.

**Accept:** Both tools are current, or the user explicitly chose to continue; the project toolset matches the installed Intel compiler.

## 4. Build

Build `ide/Arma-3/mimalloc.sln` using Release x64 on the Windows host.

Verify that the build exits with code 0 and produces a fresh DLL in `ide/Arma-3\Release\`. Use `dumpbin /exports` or an equivalent PE export inspection tool to verify all 11 CMA APIs:

`MemAlloc`, `MemAllocA`, `MemFree`, `MemFreeA`, `MemSize`, `MemSizeA`, `MemTotalCommitted`, `MemTotalReserved`, `MemFlushCache`, `MemFlushCacheAll`, `EnableHugePages`.

**Accept:** Build succeeds, the DLL is freshly produced, and all 11 exports are present.

## 5. Verify in the real game

Start Steam before testing. A launch without Steam may fail to produce an RPT; this alone does not indicate a broken DLL.

Copy the freshly built DLL into the game's `Dll\` directory (locate the install via the Steam registry key and `steamapps\libraryfolders.vdf`, AppID 107410), then run the bundled verification script, passing the DLL's full filename (including `.dll`):

```
powershell.exe -NoProfile -ExecutionPolicy Bypass -File <skill-dir>\scripts\verify-game.ps1 -DllFileName mimalloc_v{MMP}_{YYYYMMDD}.dll
```

The script auto-detects the Steam and Arma 3 installations, launches the game with `-malloc=<dll name>`, watches for the `Allocator:` line in a new RPT (polling up to 60 s), and closes the game the moment it is found.

**Accept:** The script exits successfully, the `Allocator:` line identifies the intended DLL, and no new `.mdmp` or `.bidmp` files appear in `%LOCALAPPDATA%\Arma 3`.

On FAIL, inspect the newest RPT and any crash dump reported by the script, diagnose and remediate, then rebuild and re-verify. Never proceed to publishing with a failed verification.

## 6. Publish

Retrieve the fork's latest GitHub Release and reuse its body as the template. Preserve `# Note`, `# Changelog`, `# Description`, and `# Donation`; update only `# Changelog`.

Add entries in this order:

1. `Merge upstream changes v{VERSION}, see https://github.com/microsoft/mimalloc#releases for details.`
2. If the Intel compiler version differs from the one that built the previous release: `Update Intel® oneAPI DPC++/C++ Compiler to <version>.`
3. Summarize user- or performance-relevant CMA changes. Exclude developer-only changes such as build-method, project-file, and CMake housekeeping.

Publish only after the build and in-game verification pass.

Create the tag `Arma-3-v{VERSION}-{YYYYMMDD}`, push the branch and tag, attach the verified DLL, and publish the release as a pre-release.

**Accept:** The GitHub Release is visible as a pre-release and contains the verified DLL artifact.

## 7. User review

Present the final report and the pre-release URL to the user and **wait for their review**. If the user approves, mark the release as the latest release; otherwise keep it as a pre-release until the user decides.

**Accept:** The user has reviewed the report and artifact and decided the release's final state.
