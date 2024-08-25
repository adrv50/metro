import os
import sys
import glob
import shutil

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

def printer(indent, name, color):
    print(f"{COL_BOLD}{indent}{color}{name if name.rfind('/') == -1 else name[name.rfind('/') + 1:]}{COL_DEFAULT}")

def view(names: list, indentsize: int):
    folders = []
    files = []

    indent = indentsize * 2 * '  '

    for name in names:
        if os.path.isdir(name):
            folders.append(name)
        elif os.path.isfile(name):
            files.append(name)
        else:
            print(f"{COL_BOLD}{COL_RED}'{name}' is not exists {COL_DEFAULT}")
            exit(1)

    folders.sort()
    files.sort()

    for fol in folders:
        printer(indent, fol, COL_CYAN)

        view(sorted(glob.glob(f"{fol}/*")), indentsize + 1)

    for file in files:
        printer(indent, file, COL_GREEN if shutil.which(file) is not None else COL_WHITE)

if len(sys.argv) >= 2:
    view([f"{os.getcwd()}/{s}" for s in sys.argv[1:]], 0)
else:
    view(sorted(glob.glob(f"{os.getcwd()}/*")), 0)

