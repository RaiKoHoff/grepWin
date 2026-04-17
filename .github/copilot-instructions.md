# grepWin – Copilot Instructions

## What this project is

grepWin is a **Win32 native C++ application** (no MFC/Qt) for regex search and replace on Windows. The UI is dialog-based, built on a thin layer of custom Win32 wrappers. The repo contains two Visual Studio projects:

- `src/grepWin.vcxproj` – the main executable
- `src/ExplorerCommandVerb/ExplorerCommandVerb.vcxproj` – a shell-extension DLL for the Windows context menu (Win11 style)

`sktoolslib/` is a **git submodule** containing the shared Win32 utility and UI library used by this and other projects by the same author. Always clone with `--recursive`.

---

## Build

### Quick build (exe only, version 1.0.0.0 in resources)
Open `grepWin.sln` in **VS2022** and build. vcpkg handles the Boost dependencies automatically.

Dependencies via vcpkg: `boost-regex`, `boost-iostreams`, `boost-filesystem`  
Dependencies via NuGet: see `src/packages.config`

### Full build with installer
Requires: VS2022, NAnt 0.92, WiX 3.10, TortoiseGit (all on PATH).

```bat
# Must run from a VS developer command prompt first
nant          # build exe only (Win32 + x64)
nant setup    # build exe + MSI installers + portable ZIPs + checksums
nant msi      # build MSI from already-built binaries
nant pot      # regenerate translation template (.pot) from resources
nant setup -l:buildlog.txt   # with log file on failure
```

Copy `default.build.user.tmpl` → `default.build.user` and adjust paths before running nant.

Output packages land in `.\bin\` (`bin\Release\grepWin.exe` for Win32, `bin\Release64\grepWin.exe` for x64).

---

## Architecture

### Startup (`src/grepWin.cpp`)
Entry point creates a `CSearchDlg` and runs a standard Win32 message loop. Supports a "portable mode" where settings are stored in an INI file beside the exe instead of the Windows Registry.

### Dialog layer
All dialogs inherit from `CDialog` (`sktoolslib/BaseDialog.h`). The pattern is:
- Override `DlgFunc(HWND, UINT, WPARAM, LPARAM)` for message handling
- Override `DoCommand(int id, int msg)` for WM_COMMAND dispatch
- Override `PreTranslateMessage(MSG*)` for accelerator/keyboard handling

### Main dialog: `CSearchDlg` (`src/SearchDlg.cpp`)
This is the heart of the app. Key responsibilities:
- Collects search parameters from the UI
- Spawns `SearchThread()` on a `std::thread`
- Communicates results back to the UI thread via custom `WM_APP` messages:
  - `SEARCH_FOUND` – a matching file was found
  - `SEARCH_PROGRESS` – progress update
  - `SEARCH_END` – thread finished
- Result list items are `CSearchInfo` objects (`src/SearchInfo.h`)

### Search pipeline (`SearchDlg.cpp`)
1. `DirFileEnum` (sktoolslib) walks the directory tree
2. `MatchPath()` filters by file-name pattern and exclude-dirs regex  
3. `SearchFile()` dispatches per-file  
4. `SearchOnTextFile()` / `SearchByFilePath<CharT>()` perform the actual Boost.Regex match/replace  
5. `SendResult()` posts `SEARCH_FOUND` back to the UI thread

### Settings persistence
- **Registry** (`CRegStdDWORD`, `CRegStdString` from `sktoolslib/Registry.h`) for all UI state
- **INI file** (`CSimpleIni` / `sktoolslib/SimpleIni.h`) in portable mode and for bookmarks
- `CBookmarks` inherits from `CSimpleIni` and stores named search presets

### Theme / Dark mode
`src/Theme.cpp` + `sktoolslib/DarkModeHelper.cpp` provide dark mode support. `CTheme` is a singleton — dialogs call `CTheme::Instance().RegisterThemeChangeCallback(...)`, store the returned callback ID in `m_themeCallbackId`, and deregister on destruction.

### Translations
`.lang` files in `translations/` use PO format. `sktoolslib/Language.h` loads them at runtime based on a user preference. The English fallback is built into the binary resources (`src/Resources/grepWin.rc`).

---

## Key conventions

### Naming
- Classes: `C` prefix (`CSearchDlg`, `CBookmarks`, `CSearchInfo`)
- Plain data structs: no prefix (`Bookmark`)
- Member variables: `m_` prefix (`m_searchString`, `m_bUseRegex`)
- Boolean members: `m_b` prefix (`m_bCaseSensitive`)
- Registry-backed members: `m_reg` prefix (`m_regUseRegex`)
- Wide strings (`std::wstring`) everywhere; `LPCTSTR`/`LPCWSTR` at Win32 API boundaries

### Code style
Formatted with **clang-format** (config in `sktoolslib/.clang-format`):
- 4-space indentation, no tabs
- Allman/custom brace style (brace on new line after control statements, functions, classes)
- No column limit (`ColumnLimit: 0`)
- Pointer alignment left: `int* foo`
- `#pragma once` for include guards (no `#ifndef` guards)
- Precompiled header: `stdafx.h` / `stdafx.cpp`

### Adding a new dialog
1. Add `CFooDlg` inheriting `CDialog` from `sktoolslib/BaseDialog.h`
2. Add dialog resource to `src/Resources/grepWin.rc`
3. Add string IDs to `src/resource.h`
4. Call `CLanguage::Instance().TranslateWindow(*this)` in the dialog's `WM_INITDIALOG` handler to apply translations
5. Add translatable strings to `translations/English.lang` and regenerate with `nant pot`

### sktoolslib usage
Include headers directly from the submodule path, e.g.:
```cpp
#include "../sktoolslib/Registry.h"
#include "../sktoolslib/DirFileEnum.h"
```
Do not copy files out of `sktoolslib/`; it is shared across projects.
