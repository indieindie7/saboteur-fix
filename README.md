# The Saboteur (Steam) on modern PCs: 6+ core fix + borderless window

Two small tools that got the Steam version of The Saboteur (2009) running and borderless
on a 6-core CPU at 1920x1080 (tested 2026-10-18, Windows 11, i5-10600K, driver 617.42).

## The problems

1. **The game refuses to start on CPUs with more than 4 cores.** It reads the processor count
   at startup and bails out before writing any config. The known fix for the GOG exe is a
   byte patch, but the Steam exe is SecuROM + Steam-wrapped (`.secu`/`.bind` sections,
   encrypted on disk), so there is nothing to patch on disk.
2. **No windowed or borderless option.** The in-game options and `VideoSetup.exe` do not
   expose it. ThirteenAG's Fusion Fix can make the window borderless, but only when the
   game itself runs windowed, and the config key for that does not exist until you add it.

## The fix

### 1. Fusion Fix (ASI loader + borderless)
Download `TheSaboteur.FusionFix.zip` from
https://github.com/ThirteenAG/WidescreenFixesPack/releases/tag/thesaboteur and unpack it
into the game folder (`dinput8.dll` + `scripts\TheSaboteur.FusionFix.asi` + `.ini`).
In the ini keep `BorderlessWindowed = 1` (default).

### 2. `cores4.asi` (this repo): make the game see 4 cores
`cores4.c` is a tiny plugin loaded by the Fusion Fix ASI loader. At attach it

- pins the process to the first 4 logical CPUs (`SetProcessAffinityMask(0xF)`), and
- patches the main module's import table so `GetSystemInfo` / `GetNativeSystemInfo`
  report at most 4 processors and an active mask of `0xF`.

Nothing on disk is changed, so it works with the DRM-wrapped Steam exe.

Build (Visual Studio C++ build tools, 32-bit):

```
build.cmd "H:\SteamLibrary\steamapps\common\The Saboteur"
```

or just compile `cores4.c` as an x86 DLL named `cores4.asi` and drop it into `scripts\`.

### 3. `saboteur_windowed.py`: turn on windowed mode in the game config
Start the game once (it now reaches the menu), quit, then run

```
py -3 saboteur_windowed.py 1920 1080
```

It finds `Documents\My Games\The Saboteur™\SaveGames\config.ini` (written on the first
successful run; `key<TAB>value` lines, no `Windowed` key by default), adds `Windowed 1`
after the `VSync` line, optionally sets `ScreenWidth`/`ScreenHeight`, and keeps a `.bak`.
Fusion Fix then turns the windowed game into a borderless one on the next start.

## Notes
- Both `cores4.asi` and `TheSaboteur.FusionFix.asi` must stay in `scripts\`.
- If the game ever rewrites `config.ini` without the key, rerun the script.
- Core-count background: https://vttoth.com/CMS/technical-notes/327-fixing-the-saboteur

## Licence
MIT.
