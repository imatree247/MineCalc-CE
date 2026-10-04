import re, sys

perm = [3, 2, 1, 0, 7, 6, 5, 4]
pat = re.compile(r'(uint8_t\s+\w+\[64\]\s*=?\s*\{)(.*?)(\};?)', re.S)

def fix(m):
    nums = re.findall(r'\d+', m.group(2))
    assert len(nums) == 64, m.group(1)
    new = [nums[perm[r]*8 + perm[c]] for r in range(8) for c in range(8)]
    rows = ",\n".join("\t" + ",".join(new[r*8:r*8+8]) for r in range(8))
    return m.group(1) + "\n" + rows + "\n" + m.group(3)

src = open(sys.argv[1]).read()
open(sys.argv[1] + ".fixed", "w").write(pat.sub(fix, src))
