import glob
import json
import argparse

g_punctuaters = [
    "<<=", ">>=", "<<", ">>", "<=", ">=", "==", "!=", "..", "&&", "||", "->",
    "<",   ">",   "+",  "-",  "/",  "*",  "%",  "=",  ";",  ":",  ",",  ".",
    "[",   "]",   "(",  ")",  "{",  "}",  "!",  "?",  "&",  "^",  "|",
]

class FormatStyle:
    Indent_Size = 2
    Indent_UseSpace = True

    NewLineBeforeBracket_Base = "Attach"
    NewLineBeforeBracket_BeforeElse = True

class Formatter:
    def __init__(self, style: FormatStyle, source: str):
        self.style = style
        self.source = source
    
    def run(self):



        pass

parser = argparse.ArgumentParser()

parser.add_argument("--dir", default=["include", "src"], nargs="*", help="directory to format all sources")
parser.add_argument("--file", default="///", help="if you want format only one file")

args = parser.parse_args()

sources = [ ]

if args.file == "///":
    if "include" not in args.dir:
        args.dir.append("include")

    if "src" not in args.dir:
        args.dir.append("src")

    for _dir in args.dir:
        sources.extend(glob.glob(f"{_dir}/*.c"))
        sources.extend(glob.glob(f"{_dir}/*.h"))

        # feature
        # sources.extend(glob.glob(f"{_dir}/*.cpp"))
        # sources.extend(glob.glob(f"{_dir}/*.hpp"))
else:
    sources = [args.file]

print(sources)

fmtstyle = FormatStyle()

# todo: read style from format.json

for path in sources:
    fs = open(path, "r")

    source = fs.read()

    fmt = Formatter(fmtstyle, source)
    fmt.run()

    fs.close()
    fs = open(path, "w")

    fs.write(fmt.source)
    fs.close()

