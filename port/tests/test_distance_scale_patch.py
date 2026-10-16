#!/usr/bin/env python3
"""Run the real translator and compile a synthetic distance function; no game assets needed."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

TOOLS = Path(__file__).resolve().parents[1] / "tools" / "translate"
PATCH = TOOLS / "golf.distance.json"

def call(cmd):
    p = subprocess.run(cmd, capture_output=True, text=True)
    if p.returncode:
        raise AssertionError(f"{cmd}\n{p.stdout}\n{p.stderr}")
    return p.stdout

def main():
    d = json.loads(PATCH.read_text())
    old, new = d["patches"]["FUN_0040acd0"][0]
    assert d["required_patch_counts"]["FUN_0040acd0"] == [[old, 1]]
    assert old.count("local_4 = iVar2 * 8;") == 1
    assert "param_1 = (int)param_1 / 8;" in new
    assert "param_2 = (int)param_2 / 8;" in new
    original_c = ("int FUN_0040acd0(uint param_1,uint param_2) {\n"
        "uint uVar1; int iVar2; int local_4;\n" + old + "\n"
        "return __ftol(SQRT((float10)(int)(param_1*param_1+param_2*param_2))*(float10)local_4);\n}\n")
    with tempfile.TemporaryDirectory() as temp:
        root = Path(temp)
        exp = root / "export"
        exp.mkdir()
        (exp / "types.h").write_text("")
        (exp / "functions.jsonl").write_text(json.dumps({
            "addr":"0040acd0", "name":"FUN_0040acd0",
            "sig":"int FUN_0040acd0(uint param_1,uint param_2)",
            "c":original_c, "globals":[]
        }) + "\n")
        for variant in ("before", "after"):
            cfg = root / variant
            cfg.mkdir()
            (cfg / "golf.json").write_text("{}")
            if variant == "after":
                shutil.copy2(PATCH, cfg / "golf.distance.json")
            output = cfg / "out"
            call([sys.executable, str(TOOLS / "translate.py"), "--export",str(exp),
                "--module","golf", "--range","0x40acd0-0x40acd1",
                "--out",str(output),"--config",str(cfg / "golf.json")])
            source = (output / "golf_00.c").read_text()
            assert ("param_1 = (int)param_1 / 8;" in source) == (variant == "after")
            source = source.replace('#include "golf_protos.h"', '#include "fixture.h"')
            source = source.replace("FUN_0040acd0",
                                    "fixed_distance" if variant == "after" else "baseline_distance")
            (root / (variant + ".c")).write_text(source)
        (root / "fixture.h").write_text("""#include <stdint.h>
#include <math.h>
typedef uint32_t uint;
typedef uint32_t undefined4;
typedef long double float10;
#define SQRT sqrtl
#define PORT_TRACE(x) ((void)(x))
static inline int fixture_ftol(long double n) { return (int)n; }
#define __ftol fixture_ftol
""")
        (root / "test.c").write_text(r"""#include <math.h>
#include <stdint.h>
#include <stdio.h>
int baseline_distance(uint32_t,uint32_t),fixed_distance(uint32_t,uint32_t);
static uint32_t rng=0x31a730e7u;
static uint32_t next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static int small=0,large=0,changed=0;
static int check(int x,int y){
  int a=baseline_distance((uint32_t)x,(uint32_t)y);
  int b=fixed_distance((uint32_t)x,(uint32_t)y);
  if(fabsl((long double)b-hypotl((long double)x,(long double)y))>11)return 1;
  if(x>=-16384&&x<=16384&&y>=-16384&&y<=16384){small++;if(a!=b)return 2;}
  else{large++;if(a!=b)changed++;}
  return 0;
}
int main(void){
  for(int x=-16400;x<=16400;x+=347)for(int y=-16400;y<=16400;y+=431)
    if(check(x,y))return 3;
  for(int i=0;i<100000;i++){
    int x=(int)(next()%320001u)-160000,y=(int)(next()%320001u)-160000;
    if(check(x,y))return 4;
  }
  if(small<8000||large<90000||changed<80000)return 5;
  printf("short exact %d; long within 11 units %d; changed %d\n",small,large,changed);
  return 0;
}
""")
        cc = shutil.which(os.environ.get("CC","clang")) or shutil.which("gcc")
        assert cc, "C compiler required"
        for opt in (0, 2):
            exe = root / f"test_O{opt}"
            call([cc,"-std=c11",f"-O{opt}","-Wall","-Wextra","-Werror",
                  "-fsanitize=address,undefined","-I",str(root),
                  str(root/"before.c"),str(root/"after.c"),str(root/"test.c"),
                  "-lm","-o",str(exe)])
            print(f"O{opt}: {call([str(exe)]).strip()}")
    print("distance translator regression passed")

if __name__ == "__main__":
    main()
