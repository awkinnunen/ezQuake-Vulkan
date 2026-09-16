"""OpenAI Codex: bounded real-broker multi-guest integration test. Private output directory."""
import argparse, pathlib, subprocess, time, json
p=argparse.ArgumentParser();p.add_argument('executable',type=pathlib.Path);p.add_argument('output',type=pathlib.Path);a=p.parse_args()
exe=str(a.executable.resolve());out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
processes=[];logs=[]
def launch(name,*args):
 f=(out/(name+'.log')).open('w');logs.append(f)
 child=subprocess.Popen([exe,*map(str,args)],stdout=f,stderr=subprocess.STDOUT,creationflags=subprocess.CREATE_NO_WINDOW)
 processes.append(child);return child
def wait_for(fn,seconds=30):
 end=time.monotonic()+seconds
 while time.monotonic()<end:
  if fn():return
  time.sleep(.1)
 raise RuntimeError('Timed out waiting for test state')
try:
 host=launch('host','--host',out/'host');invite=out/'host/invite.txt'
 wait_for(lambda:invite.exists() and invite.stat().st_size>100)
 first=invite.read_text();guests=[launch('guest'+str(i),'--join',invite) for i in range(2)]
 wait_for(lambda:all(g.poll() is not None for g in guests),45)
 assert all(g.returncode==0 for g in guests),'Multi-guest echo failed'
 # Keep the same room alive after peers leave; old dead peers must not block new joins.
 again=launch('again','--join',invite);wait_for(lambda:again.poll() is not None,45);assert again.returncode==0
 (out/'host/stop').touch();wait_for(lambda:host.poll() is not None)
 assert host.returncode==0
 time.sleep(1)
 invite.unlink();host=launch('restarted','--host',out/'host');wait_for(lambda:invite.exists() and invite.stat().st_size>100)
 assert invite.read_text()==first,'Invitation changed after process restart'
 again=launch('after-restart','--join',invite);wait_for(lambda:again.poll() is not None,45);assert again.returncode==0
 (out/'host/stop').touch();wait_for(lambda:host.poll() is not None)
 result={'pass':True,'simultaneousGuests':2,'packetsPerGuest':100,'bytesPerPacket':1450,'sequentialRejoin':True,'processRestartSameInvitation':True,'broker':'existing Frag-Net','networkScope':'one computer, multiple native processes'}
 (out/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
finally:
 for child in processes:
  if child.poll() is None:child.terminate()
 for child in processes:child.wait(timeout=15)
 for f in logs:f.close()
