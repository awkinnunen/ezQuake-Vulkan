"""OpenAI Codex: adverse admission tests against an owned real-broker room."""
import argparse,pathlib,subprocess,time,json,re
p=argparse.ArgumentParser();p.add_argument('executable',type=pathlib.Path);p.add_argument('output',type=pathlib.Path);a=p.parse_args()
exe=str(a.executable.resolve());out=a.output.resolve();out.mkdir(parents=True,exist_ok=False);children=[];files=[]
def launch(name,*args):
 f=(out/(name+'.log')).open('w');files.append(f);c=subprocess.Popen([exe,*map(str,args)],stdout=f,stderr=f,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0));children.append(c);return c
def log(name):return (out/(name+'.log')).read_text(errors='replace')
def wait(fn,seconds=30):
 end=time.monotonic()+seconds
 while time.monotonic()<end:
  if fn():return
  time.sleep(.1)
 raise RuntimeError('Admission state timed out')
try:
 host=launch('host','--host',out/'host');invite=out/'host/invite.txt';wait(lambda:invite.exists() and invite.stat().st_size>100)
 original=invite.read_text();good=launch('good','--join',invite);wait(lambda:'Connected through' in log('good'))
 (out/'host/close').touch();wait(lambda:'Invitations closed' in log('host'))
 closed=launch('closed','--join',invite);time.sleep(3);assert 'Connected through' not in log('closed');closed.terminate();closed.wait(timeout=5)
 wait(lambda:good.poll() is not None);assert good.returncode==0,'Closing interrupted an admitted guest'
 (out/'host/open').touch();wait(lambda:'Invitations open' in log('host'))
 wrongkey=out/'wrong-key.txt';wrongkey.write_text(re.sub(r'key=[0-9A-Fa-f]+','key='+'0'*64,original));assert wrongkey.read_text()!=original
 bad=launch('bad-key','--join',wrongkey);wait(lambda:'Invitation invalid or closed' in log('host'));bad.terminate();bad.wait(timeout=5)
 wrongfp=out/'wrong-fingerprint.txt';wrongfp.write_text(re.sub(r'fp=[0-9A-Fa-f]+','fp='+'0'*64,original));assert wrongfp.read_text()!=original
 bad=launch('bad-pin','--join',wrongfp);wait(lambda:'Host identity does not match invitation' in log('bad-pin'));bad.terminate();bad.wait(timeout=5)
 survivor=launch('survivor','--join',invite);wait(lambda:'Connected through' in log('survivor'))
 (out/'host/rotate').touch();wait(lambda:'Invitation replaced' in log('host'))
 wait(lambda:survivor.poll() is not None);assert survivor.returncode==0,'Rotation interrupted admitted guest'
 old=out/'old-link.txt';old.write_text(original)
 bad=launch('old-link','--join',old);time.sleep(3);assert 'Connected through' not in log('old-link');bad.terminate();bad.wait(timeout=5)
 (out/'host/stop').touch();wait(lambda:host.poll() is not None);assert host.returncode==0
 time.sleep(1);invite.unlink();host=launch('restart','--host',out/'host');wait(lambda:invite.exists() and invite.stat().st_size>100)
 assert invite.read_text()!=original
 good=launch('new-link','--join',invite);wait(lambda:good.poll() is not None);assert good.returncode==0
 (out/'host/stop').touch();wait(lambda:host.poll() is not None)
 result=dict(pass_=True,closedBlocksNewGuests=True,closePreservesExisting=True,wrongKeyRejected=True,wrongPinRejected=True,rotationPreservesExisting=True,oldInvitationRejected=True,newInvitationWorksAfterRestart=True)
 result['pass']=result.pop('pass_');(out/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
finally:
 for c in children:
  if c.poll() is None:c.terminate()
 for c in children:c.wait(timeout=15)
 for f in files:f.close()
