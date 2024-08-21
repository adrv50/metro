import glob
import os

def cwd(s):
    print(s)
    os.system(s)

ofiles = glob.glob("build/*.o")

ofiles.remove("build/main.o")

cwd(f"gcc -std=c17 -O3 -Wall -Wextra ./tester.c {' '.join(ofiles)} -o tester")
