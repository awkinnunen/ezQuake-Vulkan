"""OpenAI Codex: publish audited Unix beta assets using GH_TOKEN; preserve other releases."""
from pathlib import Path
import os,sys,subprocess,json,urllib.request,urllib.parse,hashlib
root=Path(__file__).resolve().parent.parent
folder,action=sys.argv[1:3];assert action in ['draft','publish','status']
audit=json.loads((Path(folder)/'verified-assets.json').read_text());assert audit['pass']
secret=os.environ['GH_TOKEN']
def api(path,method='GET',data=None,raw=False):
 url=path if path.startswith('https://') else 'https://api.github.com/repos/awkinnunen/ezQuake-Vulkan'+path
 headers={'Authorization':'Bearer '+secret,'User-Agent':'ezv-unix-release','Accept':'application/vnd.github+json'}
 if data is not None:
  headers['Content-Type']='application/octet-stream' if raw else 'application/json'
  if not raw:data=json.dumps(data).encode()
 with urllib.request.urlopen(urllib.request.Request(url,data=data,headers=headers,method=method),timeout=900) as r:return json.load(r)
release=next((r for r in api('/releases') if r['tag_name']=='v0.3.0-beta.1'),None)
if action=='draft':
 if not release:release=api('/releases','POST',{'tag_name':'v0.3.0-beta.1','target_commitish':audit['sourceCommit'],'name':'ezQuake Vulkan 0.3.0-beta.1 — Linux and macOS testing','body':(root/'dist/unix/RELEASE-NOTES.md').read_text(),'draft':True,'prerelease':True})
 assert release['draft'] and release['target_commitish']==audit['sourceCommit']
 for item in audit['assets']:
  f=Path(item['path']);old=next((x for x in release['assets'] if x['name']==f.name),None)
  data=f.read_bytes();digest='sha256:'+hashlib.sha256(data).hexdigest();assert digest=='sha256:'+item['sha256']
  if old:assert old.get('digest')==digest;continue
  uploaded=api(release['upload_url'].split('{')[0]+'?name='+urllib.parse.quote(f.name),'POST',data,True)
  assert uploaded['size']==len(data) and uploaded.get('digest')==digest
  print('Uploaded and verified:',f.name,flush=True)
elif action=='publish':
 assert release and len(release['assets'])==len(audit['assets'])==16
 for item in audit['assets']:
  asset=next(x for x in release['assets'] if x['name']==item['name']);assert asset['digest']=='sha256:'+item['sha256']
 release=api('/releases/'+str(release['id']),'PATCH',{'draft':False,'prerelease':True,'make_latest':'false'})
 print(release['html_url'])
else:print(json.dumps({'url':release['html_url'] if release else None,'draft':release['draft'] if release else None,'assets':len(release['assets']) if release else 0}))
