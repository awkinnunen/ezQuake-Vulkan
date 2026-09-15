"""Compare rendered results, not implementation text. Codex, 2026-09-14."""
from pathlib import Path
import json,sys,hashlib,re
import numpy as np
from PIL import Image
root=Path(__file__).resolve().parent.parent;p=root/('cache/runtime-'+sys.argv[1])
spec=json.loads((p/'shadow2-spec.json').read_text(encoding='utf-8-sig'))
images={c['name']:np.array(Image.open(p/'qw'/c['shot']).convert('RGB')).astype(int) for c in spec}
results={}
for a,b in [('uncached','cached'),('move','move-uncached'),('actor','actor-uncached'),('eight','cached-eight'),('overflow','overflow-reference'),('baked','map-shadows'),('map-shadows','spot-realtime'),('spot-realtime','spot-soft'),('spot-soft','ambient'),('ambient','map-radius'),('quality512','quality128')]:
 d=np.abs(images[a]-images[b]);n=int(np.any(d,axis=2).sum());results[a+' / '+b]={'changedPixels':n,'meanAbsoluteDifference':float(d.mean())}
 if (a,b) in [('uncached','cached'),('move','move-uncached'),('actor','actor-uncached'),('eight','cached-eight'),('overflow','overflow-reference')]: assert n==0,(a,b,n)
 else:assert n>100,(a,b,n)
log=(p/'qw/qconsole.log').read_text(errors='replace')
assert not re.search('VUID-|Validation Error|VK_ERROR_DEVICE_LOST',log)
(p/'pixel-verification.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(results,indent=2))

