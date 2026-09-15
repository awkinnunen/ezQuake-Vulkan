"""MENU-UNIFY-001 / CFG-PRESETS-001: compare actual runtime exports, not menu labels."""
from pathlib import Path
import re,sys,json
root=Path(__file__).resolve().parent.parent
p=root/('cache/runtime-'+sys.argv[1])
def values(path):
 out={}
 for line in path.read_bytes().decode('latin1').splitlines():
  m=re.fullmatch(r'\s*(\w+)\s+"([^"\r\n]*)"\s*',line)
  if m:out[m[1].lower()]=m[2]
 return out
def equivalent(a,b):
 try:return float(a)==float(b)
 except ValueError:return a.lower()==b.lower()
def check(got,want,where):
 bad={k:(got.get(k),v) for k,v in want.items() if k not in got or not equivalent(got[k],v)}
 assert not bad,(where,bad)
def cfg(name):return values(p/f'ezquake/configs/{name}.cfg')
fields=values(p/'ezquake/presets/graphics/test-roundtrip.cfg')
base={k:v for k,v in cfg('gfx-base').items() if k in fields}
assert len(base)==170
for name,delta in [('gfx-a',dict(r_cv_exposure='.8',gl_outline='1',r_cv_shadowstrength='.3',r_cv_hdr='1')),
 ('gfx-b',dict(r_cv_exposure='1.8')),('gfx-cancel',{}),('gfx-invalid',{}),('gfx-restart-cancel',{}),
 ('gfx-applied',dict(r_cv_exposure='1.8')),('gfx-roundtrip',dict(r_cv_exposure='1.8')),
 ('gfx-overwrite',dict(r_cv_exposure='1.6')),
 ('gfx-reset-cancel',dict(r_cv_exposure='1.8')),
 ('gfx-mouse',dict(r_cv_exposure='.5')),
 ('gfx-scope',dict(r_cv_exposure='1.8',r_cv_shadowstrength='.3')),
 ('gfx-disabled',dict(r_cv_exposure='1.8',r_cv_shadows='0')),('gfx-closed',dict(r_cv_exposure='1.8',r_cv_shadows='0'))]:
 check(cfg(name),base|delta,name)
for preset,save in [('Balanced','gfx-balanced'),('Ultra-competitive','gfx-ultra'),('Athmospheric','gfx-atmospheric'),('Balanced','gfx-final')]:
 expected=values(root/f'profiles/ezquake/presets/graphics/builtin/{preset}.cfg')
 assert len(expected)==170
 check(cfg(save),expected,save)
 check(cfg(save),{'volume':'.37'},save+' audio')
sdfe=cfg('keys-sdfe');wasd=cfg('keys-wasd')
def bindings(name):
 return {m[1].lower():m[2] for m in re.finditer(r'^bind\s+(\S+)\s+"([^"\r\n]*)"', (p/f'ezquake/configs/{name}.cfg').read_bytes().decode('latin1'),re.M)}
keys=bindings('keys-sdfe');other=bindings('keys-wasd')
assert all(keys.get(k)==v for k,v in dict(e='+legacy_fw',d='+legacy_bw',s='+moveleft',f='+moveright',a='legacy_sj',g='tp_msgsafe',h='tp_msghelp',q='weapon 6').items()),keys
assert all(other.get(k)==v for k,v in dict(w='+legacy_fw',s='+legacy_bw',a='+moveleft',d='+moveright',capslock='legacy_sj',f='tp_msgsafe',g='tp_msghelp',z='weapon 6').items()),other
check(wasd,{k:sdfe[k] for k in fields},'layout isolates graphics')
assert bindings('gfx-final')==other,'graphics preset changed bindings'
log=(p/'qw/qconsole.log').read_text(errors='replace')
backup=values(p/'ezquake/presets/graphics/test-roundtrip.cfg.bak-1')
check(backup,base|dict(r_cv_exposure='1.8'),'overwrite backup')
assert re.findall(r'GFX_INVALID name=(\S+)',log)==['map']
assert log.count('GFX_LOAD Balanced OK')==2 and 'GFX_LOAD Athmospheric OK' in log and 'GFX_LOAD Ultra-competitive OK' in log
assert 'RGBA16F linear' in log and 'UNIFIED_GRAPHICS_COMPLETE' in log
check(cfg('gfx-reset'),{'r_cv_exposure':'1','r_cv_bloom':'0','vid_framebuffer_multisample':'0'},'factory reset preview')
assert 'available=0' in log and 'Connect to a live KTX game' in log
assert 'Video restart required' in log or 'video restart required' in log
print('PASS: 170-value preview/partial/reset isolation, invalid-file atomicity, cancel/close/restart, save/reload, 3 built-ins, SDFE/WASD and audio/binding isolation.')
