"""SHADOW-003 regression evidence, OpenAI Codex, 2026-09-15."""
from pathlib import Path
import hashlib,json,re,sys
import numpy as np
from PIL import Image
r=Path(__file__).resolve().parent.parent
report={'author':'OpenAI Codex','date':'2026-09-15','runs':{}}
for label in sys.argv[1:]:
 p=r/('cache/runtime-'+label)
 spec=json.loads((p/'maplight-spec.json').read_text(encoding='utf-8-sig'))
 log=(p/'qw/qconsole.log').read_text(errors='replace')
 assert 'CODEX_SMOKE_COMPLETE' in log and not re.search(r'VUID-|Validation Error|VK_ERROR_DEVICE_LOST',log)
 cases={};maximumErrors={}
 for a,b in zip(spec[::2],spec[1::2]):
  assert a['mode']==0 and b['mode']==1 and a['camera']==b['camera'] and a['radius']==b['radius']
  x,y=[np.asarray(Image.open(p/'qw'/v['shot']).convert('RGB')) for v in (a,b)]
  delta=int(np.any(x!=y,axis=2).sum())
  cases[str(a['camera'])+'/'+a['radius']]=delta
  maximumErrors[str(a['camera'])+'/'+a['radius']]=int(np.abs(x.astype(int)-y.astype(int)).max())
 report['runs'][label]={'changedPixelsVsBaked':cases,'maximumChannelError':maximumErrors,'logSHA256':hashlib.sha256((p/'qw/qconsole.log').read_bytes()).hexdigest()}
 print(label,cases)
 # The two shader paths can round a final 8-bit channel differently. Allow at
 # most four isolated pixels by one level, never broad or visible darkening.
 if 'fixed' in label:assert all(n<=4 for n in cases.values()) and max(maximumErrors.values())<=1,'Baked world changed'
 if 'before' in label:assert any(n>1000 for n in cases.values()),'Regression was not reproduced'
if len(sys.argv)>2:
 report['finalExecutables']={c:hashlib.sha256((r/f'build-msvc-x64/{c}/ezquake.exe').read_bytes()).hexdigest() for c in ['Debug','Release']}
 report['scope']='E1M1 and DM6, three camera positions each, map radius 0.25/1/4, eight lights/updates, HDR/MSAA. No entity rendering: static world matches baked reference within isolated one-level 8-bit rounding (maximum four pixels allowed). Dynamic caster and realtime-lighting coverage uses shadow3-regression separately.'
 report['casterAndRealtimeRegression']=json.loads((r/'cache/runtime-shadow3-regression/pixel-verification.json').read_text())
 (r/'provenance/shadow3-validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
