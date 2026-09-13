"""PUBLIC-001, OpenAI Codex: compare runtime-saved cvars, not just source text."""
from pathlib import Path
import re, sys
root = Path(__file__).resolve().parent.parent
profile = root / ('cache/runtime-' + sys.argv[1])
def values(path):
    result = {}
    for line in path.read_bytes().decode('latin1').splitlines():
        m = re.fullmatch(r'\s*(\w+)\s+"?([^"\r\n]*?)"?\s*', line)
        if m:
            result[m[1]] = m[2]
    return result
expected = values(root / 'profiles/ezquake/competitive/project-default.cfg')
actual = values(profile / 'ezquake/competitive/public-verified.cfg')
assert len(expected) == 53
assert {k: actual.get(k) for k in expected} == expected, 'Graphics defaults changed'
saved = values(profile / 'ezquake/configs/public-result.cfg')
assert saved['w_switch'] == saved['b_switch'] == '8'
assert saved['crosshairsize'] == '2.5' and saved['crosshairimage'] == ''
particles = values(root / 'profiles/qw/ezv-particles.cfg')
assert len(particles) == 55
assert {k:saved.get(k) for k in particles} == particles, 'Conditional effects not preserved'
print('PASS: all 53 approved graphics values survive the public WASD overlay; pickup selection 8/8, built-in crosshairs size 2.5.')
print('PASS: all 55 supported conditional effects saved, including lightning 1 and sparks 0.4.')
