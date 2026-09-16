"""INPUT-004: compare runtime binding exports and graphics/input isolation."""
from pathlib import Path
import re,sys
root=Path(__file__).resolve().parent.parent
p=root/('cache/runtime-'+sys.argv[1])
def text(path):return path.read_bytes().decode('latin1')
def fields(raw):return dict(re.findall(r'^\s*(\w+)\s+"([^"\r\n]*)"\s*$',raw,re.M))
def binds(raw):return {m[1].lower():m[2] for m in re.finditer(r'^bind\s+(\S+)\s+"([^"\r\n]*)"',raw,re.M) if m[2]}
def cfg(name):return text(p/f'ezquake/configs/keyboard-{name}.cfg')
nquake=binds(text(root/'profiles/qw/ezv-nquake.cfg'))
# ezQuake's later generic SHIFT binding overrides both left/right declarations;
# config saving collapses identical sides back to SHIFT. Preserve source order.
nquake.pop('rshift',None)
balanced=fields(text(root/'profiles/ezquake/presets/graphics/builtin/Balanced.cfg'))
assert len(balanced)==170 and len(nquake)>50
for name in ['nquake-first','nquake-after','nquake-final']:
    assert binds(cfg(name))==nquake,(name,binds(cfg(name)).items()^nquake.items())
for name,expected in [('quick-wasd',dict(w='+legacy_fw',s='+legacy_bw',a='+moveleft',d='+moveright',capslock='legacy_sj')),
                      ('quick-esdf',dict(e='+legacy_fw',d='+legacy_bw',s='+moveleft',f='+moveright',a='legacy_sj'))]:
    values=fields(cfg(name))
    assert values['crosshairimage'] in {'legacy_sg','legacy_ng','legacy_gl','legacy_rl','legacy_lg'}, (name,values['crosshairimage'])
    for key,value in {'crosshair':0,'crosshairsize':2.5,'crosshairalpha':.6,'crosshairscale':0,'crosshairscalemethod':0,'r_smoothcrosshair':1}.items():
        assert abs(float(values[key])-value)<1e-6,(name,key,values[key])
    got=binds(cfg(name))
    expected.update(mouse1='+legacy_rl',mouse2='+legacy_shaft',mouse3='+legacy_gl')
    assert all(got.get(k)==v for k,v in expected.items()),(name,got)
assert binds(cfg('quick-wasd'))==binds(cfg('wasd-compat'))
assert binds(cfg('quick-esdf'))==binds(cfg('sdfe-compat'))
for name in ['nquake-first','quick-wasd','quick-esdf','nquake-after','nquake-final','wasd-compat','sdfe-compat']:
    raw=cfg(name);got=fields(raw)
    assert all(got.get(k)==v or float(got[k])==float(v) for k,v in balanced.items()),name
    assert float(got['sensitivity'])==3.17 and abs(float(got['volume'])-.37)<1e-6,name
    assert re.search(r'^alias\s+user_sentinel\s+"echo personal-alias"',raw,re.M),name
assert re.search(r'^alias\s+f_weaponchange\s+"echo personal-hook"',cfg('nquake-first'),re.M)
assert not re.search(r'^alias\s+f_weaponchange\s',cfg('nquake-after'),re.M),'Quick crosshair hook remains active'
assert re.search(r'^set\s+timer\s+"1"',cfg('timer'),re.M)
log=text(p/'qw/qconsole.log')
assert 'KEYBOARD_PRESETS_COMPLETE' in log and 'CONTROLS_FOCUS nQuake' in log and 'CONTROLS_FOCUS Quick ESDF' in log
assert not re.search(r'Unknown command|VUID-|Validation Error|VK_ERROR_DEVICE_LOST',log.split('CODEX_MAP_SPAWNED',1)[1])
print('PASS: native nQuake/Quick ESDF menu actions, Quick WASD, legacy command aliases, exact nQuake bindings after repeated switches, timer helpers, 170 graphics values, sensitivity/audio and unrelated aliases preserved; Quick hook detached.')
