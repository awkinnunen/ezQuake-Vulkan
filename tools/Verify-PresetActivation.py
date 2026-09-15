"""CFG-PRESETS-002: saved runtime values after native keyboard/mouse activation."""
from pathlib import Path
import re, sys
root = Path(__file__).resolve().parent.parent
p = root / ('cache/runtime-' + sys.argv[1])
def values(path):
    return dict(re.findall(r'^\s*(\w+)\s+"([^"\r\n]*)"\s*$', path.read_bytes().decode('latin1'), re.M))
def check(name, preset, delta=None):
    expected = values(root / f'profiles/ezquake/presets/graphics/builtin/{preset}.cfg')
    assert len(expected) == 170
    expected.update(delta or {})
    got = values(p / f'ezquake/configs/activation-{name}.cfg')
    def equal(a, b):
        try: return float(a) == float(b)
        except ValueError: return a.lower() == b.lower()
    bad = {k: (got.get(k), v) for k, v in expected.items() if k not in got or not equal(got[k], v)}
    assert not bad, (name, bad)
    assert float(got['volume']) == .37
    raw = (p / f'ezquake/configs/activation-{name}.cfg').read_bytes().decode('latin1')
    assert re.search(r'^bind\s+w\s+"\+forward"', raw, re.M)
for name, preset in [('base','Balanced'),('enter','Ultra-competitive'),('mouse','Athmospheric'),
                     ('cancel','Athmospheric'),('invalid','Athmospheric'),('f3','Balanced')]:
    check(name, preset)
check('navigation', 'Balanced', {'r_cv_exposure': '1.7'})
log = (p / 'qw/qconsole.log').read_text(errors='replace')
for marker, loaded in [('ENTER','Ultra-competitive'),('MOUSE','Athmospheric'),('SHORTCUT','Balanced'),('NAVIGATION','last-activation')]:
    state = log.split('ACTIVATE_' + marker, 1)[1].split('GFX_STATE',1)[1].splitlines()[0]
    assert 'browsing=0 preview=0' in state and 'notice=Loaded '+loaded+'.' in state, (marker,state)
invalid = log.split('ACTIVATE_INVALID',1)[1].split('GFX_STATE',1)[1].splitlines()[0]
assert 'browsing=1 preview=0' in invalid and 'Invalid or unsupported preset' in invalid, invalid
assert 'No valid preview.' in log and 'PRESET_ACTIVATION_COMPLETE' in log
assert not re.search(r'VUID-|Validation Error|VK_ERROR_DEVICE_LOST', log)
print('PASS: native Enter, mouse click, F3 and arrow navigation commit correct presets; menu exit retains values; Escape/invalid activation roll back; all 170 values, audio and binding isolation verified.')
