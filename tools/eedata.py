"""Generate the default EEPROM image (in12v3clock_Programmer/in12v3clock_STVP_data.s19/.hex)
from eeDef[] and EE_MAGIC of the sources, for: python -I tools/flash.py data

usage: python -I tools/eedata.py
"""
import os, re

PRJ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(PRJ, "in12v3clock_Programmer", "in12v3clock_STVP_data")
EE_START, EE_SIZE, REC = 0x4000, 128, 32

src = open(os.path.join(PRJ, "eeprom.c"), encoding="utf-8").read()
hdr = open(os.path.join(PRJ, "eeprom.h"), encoding="utf-8").read()
defines = dict(re.findall(r"#define\s+(\w+)\s+(\w+)", hdr))
items = re.search(r"eeDef\[EE_COUNT\]\s*=\s*\{([^}]*)\}", src).group(1).split(",")
values = [int(defines.get(v.strip(), v.strip()), 0) for v in items]
data = bytes(values + [int(defines["EE_MAGIC"], 0)]).ljust(EE_SIZE, b"\0")

s19, ihex = [], []
for off in range(0, EE_SIZE, REC):
    chunk, a = data[off:off + REC], EE_START + off
    rec = bytes([len(chunk) + 3, a >> 8, a & 255]) + chunk
    s19.append("S1%s%02X" % (rec.hex().upper(), ~sum(rec) & 255))
    rec = bytes([len(chunk), a >> 8, a & 255, 0]) + chunk
    ihex.append(":%s%02X" % (rec.hex().upper(), -sum(rec) & 255))
for ext, lines, end in ((".s19", s19, "S9030000FC"), (".hex", ihex, ":00000001FF")):
    with open(OUT + ext, "w", newline="\r\n") as f:
        f.write("\n".join(lines + [end]) + "\n")
print("EEPROM image:", " ".join(str(v) for v in data[:len(values) + 1]))
