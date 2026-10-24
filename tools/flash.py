"""Flash IN12v3clock through ST-LINK (SWIM) with STVP_CmdLine. Option bytes are not touched.

usage: python -I tools/flash.py [sdcc] [data]
  sdcc  - flash build/sdcc/main.s19 instead of build/cosmic/in12v3clock.s19
  data  - also write the default EEPROM (in12v3clock_Programmer/in12v3clock_STVP_data.s19),
          without it the firmware keeps the settings or resets them itself if the layout changed
"""
import os, subprocess, sys

PRJ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
STVP = os.environ.get("STVP_CMDLINE",
                      r"C:\Program Files (x86)\STMicroelectronics\st_toolset\stvp\STVP_CmdLine.exe")

prog = os.path.join(PRJ, "build", "sdcc", "main.s19") if "sdcc" in sys.argv else \
       os.path.join(PRJ, "build", "cosmic", "in12v3clock.s19")
cmd = [STVP, "-BoardName=ST-LINK", "-Port=USB", "-ProgMode=SWIM", "-Device=STM8S003F3",
       "-no_loop", "-no_log", "-no_progress", "-verbose", "-no_progOption", "-verif", "-FileProg=" + prog]
if "data" in sys.argv:
    cmd.append("-FileData=" + os.path.join(PRJ, "in12v3clock_Programmer", "in12v3clock_STVP_data.s19"))
else:
    cmd.append("-no_progData")

out = subprocess.run(cmd, capture_output=True, text=True, errors="replace").stdout
areas = ["PROGRAM"] + (["DATA"] if "data" in sys.argv else [])
# not loaded areas are verified against a blank image and always fail: ignore them
lines = [l for l in out.splitlines()
         if any(l.startswith(("<<< Programming %s" % a, "<<< Verifying %s" % a)) for a in areas)]
print(out if not lines else "\n".join(lines))
ok = len(lines) == 2 * len(areas) and all(l.endswith("succeeds") for l in lines)
sys.exit(0 if ok else 1)
