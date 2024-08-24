import os
import glob
import subprocess
import json
import argparse

COL_DEFAULT     = "\033[0m"
COL_BOLD        = "\033[1m"

COL_BLACK       = "\033[30m"
COL_RED         = "\033[31m"
COL_GREEN       = "\033[32m"
COL_YELLOW      = "\033[33m"
COL_BLUE        = "\033[34m"
COL_MAGENTA     = "\033[35m"
COL_CYAN        = "\033[36;5m"
COL_WHITE       = "\033[37m"

COL_BK_BLACK    = "\033[40m"
COL_BK_RED      = "\033[41m"
COL_BK_GREEN    = "\033[42m"
COL_BK_YELLOW   = "\033[43m"
COL_BK_BLUE     = "\033[44m"
COL_BK_MAGENTA  = "\033[45m"
COL_BK_CYAN     = "\033[46;5m"
COL_BK_WHITE    = "\033[47m"


# -------- flags ----------

config = json.load(open("build.json", mode="r"))

CC          = config["cc"]
LD          = "gcc"

TARGET      = config["target"]

INCLUDE     = config["include"]
SRCDIR      = config["src"]
BUILD       = config["build"]

CFLAGS      = " ".join(config["cflags"])
LDFLAGS     = "-Wl," + ",".join(config["ldflags"])

# -------------------------

# TODO:
#   argparse
#   debug-build

def xprint(*args):
    for v in args:
        print(v, end="")

    print()

class Source:
    def __init__(self, path: str):
        self.path = path
        self.pathnotdir = path[path.rfind("/") + 1:]

        self.dpath = f"{BUILD}/{self.pathnotdir[:-2]}.d"
        self.objout = f"{BUILD}/{self.pathnotdir[:-2]}.o"

        self.mtime = 0 if not os.path.exists(self.objout) \
            else os.path.getmtime(self.objout)

        self.depends = self.get_depends()

        # for result
        self.is_succeed = False
        self.result = None

        self.is_compile_needed = \
            not os.path.exists(self.objout) or self.depends == [ ]

    def get_depends(self) -> list[str]:
        global CFLAGS

        if not os.path.exists(self.dpath):
            return [ ]

        # Get all dependencies
        with open(self.dpath, mode="r") as fs:
            depends = fs.read().replace("\\\n", "")
            depends = depends[depends.find(":") + 2: depends.find("\n")]

            while depends.find("  ") != -1:
                depends = depends.replace("  ", " ")

            return depends.split(" ")

    #
    # result = if compiled return True
    #
    def compile(self) -> tuple:
        if not self.is_compile_needed:
            for d in self.depends:
                if os.path.getmtime(d) > self.mtime:
                    self.is_compile_needed = True
                    break

        if not self.is_compile_needed:
            return False

        print(f"{COL_BOLD}{COL_WHITE}CC  {self.path}{COL_DEFAULT}", end="")
        
        self.result = subprocess.run(
            f"gcc -MP -MMD -MF {self.dpath} {CFLAGS} -c -o {self.objout} {self.path}",
            shell=True,
            capture_output=True,
            text=True
        )

        self.is_succeed = (self.result.returncode == 0)

        if not self.is_succeed:
            print(COL_BOLD + COL_RED, "  => failed", COL_DEFAULT)
        else:
            print()

        return True

def is_completed(target: str, incldir: str, sources: list[Source]):
    if not os.path.exists(target):
        return False

    mtime = os.path.getmtime(target)
    headers = glob.glob(f"{incldir}/**/*.h", recursive=True)

    for s in sources:
        if mtime < s.mtime:
            return False

    for h in headers:
        if mtime < os.path.getmtime(h):
            return False

    return True

CFLAGS  += f" -I {INCLUDE}"

sources = glob.glob(f"{SRCDIR}/**/*.c", recursive=True)
sources = [Source(s) for s in sources]

if is_completed(TARGET, INCLUDE, sources):
    print(COL_BOLD + COL_YELLOW + f"'{TARGET}' is up to date." + COL_DEFAULT)
    exit(0)

err_sources = [ ]

if not os.path.isdir(BUILD):
    os.system(f"mkdir {BUILD}")

# Try compile all sources
for src in sources:
    if not src.compile():
        continue

    if src.result.returncode != 0:
        err_sources.append(src)

# Emit error if failed
if err_sources != [ ]:
    xprint(
        COL_BOLD,
        COL_RED,
        f"=== detected {COL_YELLOW}{len(err_sources)}{COL_RED} errors ===\n",
        COL_DEFAULT
    )

    for src in err_sources:
        border = "=" * 15
        longborder = "=" * (32 + len(src.path))

        xprint(COL_BOLD, COL_WHITE, border, src.path, border, COL_DEFAULT)
        xprint(src.result.stderr)
        xprint(COL_BOLD, COL_WHITE, longborder, COL_DEFAULT)

    exit(1)

#
# Linking
#
xprint(COL_BOLD, COL_WHITE, "\nCREATE ",
    COL_GREEN, TARGET, COL_WHITE, " ...", COL_DEFAULT)

ld_result = \
    subprocess.run(
        [
            LD,
            LDFLAGS,
            "-pthread",
            "-o",
            TARGET
        ] + [s.objout for s in sources],
        capture_output=True,
        text=True
    )

if ld_result.returncode != 0:
    xprint(COL_BOLD, COL_RED, "\nlink failed:\n", COL_WHITE, "=" * 40, COL_DEFAULT)
    print(ld_result.stderr)
    xprint(COL_BOLD, COL_WHITE, "=" * 40, COL_DEFAULT)

    exit(1)
else:
    print(COL_BOLD + COL_WHITE + "\nDone." + COL_DEFAULT)