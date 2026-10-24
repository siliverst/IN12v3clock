"""Command line build of IN12v3clock with Cosmic and/or SDCC, without STVD and Docker.

usage: python -I tools/build.py [cosmic] [sdcc] [-DNAME ...]      (both compilers by default)
  -DNAME  - define a macro for the sources, e.g. -DDIAG_RESET
Output goes to build/cosmic and build/sdcc. Exit code != 0 on compile error
or if the firmware does not fit into flash.
"""
import glob, os, re, subprocess, sys

sys.stdout.reconfigure(encoding="utf-8")

PRJ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(PRJ, "build")
INC = os.path.join(PRJ, "STM8_headers", "include")
COSMIC = os.environ.get("COSMIC_DIR", r"C:\Program Files (x86)\COSMIC\FSE_Compilers\CXSTM8")
SDCC = os.environ.get("SDCC_DIR", r"C:\Program Files\SDCC\bin")
ROM_SIZE, RAM_SIZE = 8192, 1024
SRCS = sorted(glob.glob(os.path.join(PRJ, "*.c")))
DEFS = [a[2:] for a in sys.argv[1:] if a.startswith("-D")]

# same options as the STVD Release configuration, but with the optimizer enabled (no "-no")
COSMIC_CFLAGS = ["+mods0", "+fast", "-pp", "-dSTM8S003"]
SDCC_CFLAGS = ["-mstm8", "--std-sdcc99", "--opt-code-size", "--max-allocs-per-node", "100000"]

LKF = """+seg .const -b 0x8080 -m 0x1f80 -n .const -it
+seg .text -a .const -n .text
+seg .eeprom -b 0x4000 -m 0x280 -n .eeprom
+seg .bsct -b 0x0 -m 0x100 -n .bsct
+seg .ubsct -a .bsct -n .ubsct
+seg .bit -a .ubsct -n .bit -id
+seg .share -a .bit -n .share -is
+seg .data -b 0x100 -m 0x100 -n .data
+seg .bss -a .data -n .bss
crtsi0.sm8
{objs}
libis0.sm8
libm0.sm8
+seg .const -b 0x8000 -k
stm8_interrupt_vector.o
+def __endzp=@.ubsct
+def __memory=@.bss
+def __startmem=@.bss
+def __endmem=0x1ff
+def __stack=0x3ff
"""


def run(cmd, cwd):
    r = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, errors="replace")
    out = "\n".join(l for l in (r.stdout + r.stderr).splitlines()
                    if l.strip() and not l.rstrip().endswith(".c:"))  # cxstm8 echoes file names
    if out:
        print(out)
    return r.returncode == 0


def report(name, rom, ram):
    print(f"{name}: ROM {rom} / {ROM_SIZE} ({ROM_SIZE - rom} free), static RAM {ram} / {RAM_SIZE}")
    return rom <= ROM_SIZE


def cosmic():
    d = os.path.join(OUT, "cosmic")
    os.makedirs(d, exist_ok=True)
    for s in SRCS:
        if not run([os.path.join(COSMIC, "cxstm8.exe")] + COSMIC_CFLAGS + ["-d" + d for d in DEFS] +
                   ["-i" + INC, "-i" + os.path.join(COSMIC, "Hstm8"), "-co" + d, s], PRJ):
            return False
    objs = [os.path.basename(s)[:-2] + ".o" for s in SRCS]
    objs.remove("stm8_interrupt_vector.o")
    with open(os.path.join(d, "in12v3clock.lkf"), "w") as f:
        f.write(LKF.format(objs="\n".join(objs)))
    if not (run([os.path.join(COSMIC, "clnk.exe"), "-l" + os.path.join(COSMIC, "Lib"), "-m", "in12v3clock.map",
                 "-o", "in12v3clock.sm8", "in12v3clock.lkf"], d) and
            run([os.path.join(COSMIC, "chex.exe"), "-fm", "-o", "in12v3clock.s19", "in12v3clock.sm8"], d)):
        return False
    rom = ram = 0
    with open(os.path.join(d, "in12v3clock.map")) as f:
        for n, seg, rest in re.findall(r"length\s+(\d+) segment (\S+)(.*)", f.read()):
            if seg in (".text", ".const", ".init") or (seg == ".bsct" and "from" in rest):
                rom += int(n)
            if seg in (".data", ".bss", ".ubsct") or (seg == ".bsct" and "initialized" in rest):
                ram += int(n)
    return report("COSMIC", rom, ram)


def sdcc():
    d = os.path.join(OUT, "sdcc")
    os.makedirs(d, exist_ok=True)
    rels = []
    for s in SRCS:
        rel = os.path.join(d, os.path.basename(s)[:-2] + ".rel")
        if not run([os.path.join(SDCC, "sdcc.exe")] + SDCC_CFLAGS + ["-D" + d for d in DEFS] + ["-I" + INC, "-c", s, "-o", rel], PRJ):
            return False
        rels.append(rel)
    ihx = os.path.join(d, "main.ihx")
    if not (run([os.path.join(SDCC, "sdcc.exe"), "-mstm8", "--out-fmt-ihx", "-o", ihx] + rels, d) and
            run([os.path.join(SDCC, "sdobjcopy.exe"), "-I", "ihex", "-O", "srec", ihx,
                 os.path.join(d, "main.s19")], d)):
        return False
    with open(os.path.join(d, "main.map")) as f:
        areas = {a: (int(s, 16), int(l, 16)) for a, s, l in
                 re.findall(r"^([A-Z_]+)\s+([0-9A-F]{8})\s+([0-9A-F]{8}) =", f.read(), re.M)}
    rom = max(s + l for s, l in areas.values() if s >= 0x8000 and l) - 0x8000
    ram = sum(areas.get(a, (0, 0))[1] for a in ("DATA", "INITIALIZED"))
    return report("SDCC  ", rom, ram)


ok = True
for target in [a for a in sys.argv[1:] if not a.startswith("-D")] or ["cosmic", "sdcc"]:
    ok &= {"cosmic": cosmic, "sdcc": sdcc}[target]()
sys.exit(0 if ok else 1)
