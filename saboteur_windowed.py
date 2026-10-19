"""The Saboteur (Steam, H:\\SteamLibrary\\steamapps\\common\\The Saboteur): set Windowed = 1 in the game's config so
ThirteenAG's Fusion Fix (dinput8.dll + scripts\\TheSaboteur.FusionFix.asi/.ini, BorderlessWindowed = 1) turns the
window into a borderless one. The game writes Documents\\My Games\\The Saboteur(tm)\\SaveGames\\config.ini on its first
run (key<TAB>value lines, CRLF, then a // comment block of resolutions). It has no Windowed line until one is added.
Usage: py -3.13 -I Documents/Tools/saboteur_windowed.py [width height]   (run after the game has been started once)
"""
import glob, os, re, sys

root = os.path.join(os.path.expanduser("~"), "Documents", "My Games")
files = [f for d in glob.glob(os.path.join(root, "The Saboteur*")) for f in glob.glob(os.path.join(d, "SaveGames", "config.ini"))]
if not files:
    print("no config.ini under", root, "- start the game once (any mode), quit, run this again")
    sys.exit(1)
for f in files:
    old = open(f, encoding="utf-8", errors="ignore", newline="").read()
    new = old
    if not re.search(r"(?m)^Windowed\t", new):
        # insert after the VSync line (same key<TAB>value style), else at the top
        if re.search(r"(?m)^VSync\t\d+\r?\n", new):
            new = re.sub(r"(?m)^(VSync\t\d+\r?\n)", lambda m: m.group(1) + "Windowed\t0" + ("\r\n" if m.group(1).endswith("\r\n") else "\n"), new, count=1)
        else:
            new = "Windowed\t0\r\n" + new
    new = re.sub(r"(?m)^(Windowed\t)\d+", r"\g<1>1", new)
    if len(sys.argv) == 3:
        new = re.sub(r"(?m)^(ScreenWidth\t)\d+", r"\g<1>" + sys.argv[1], new)
        new = re.sub(r"(?m)^(ScreenHeight\t)\d+", r"\g<1>" + sys.argv[2], new)
    if new != old:
        open(f + ".bak", "w", encoding="utf-8", newline="").write(old)
        open(f, "w", encoding="utf-8", newline="").write(new)
        print("patched", f, "(backup .bak)")
    else:
        print("already windowed:", f)
    for line in new.splitlines():
        if re.match(r"(ScreenWidth|ScreenHeight|Windowed|RefreshRate|VSync)\t", line):
            print("  ", line.replace("\t", " = "))
