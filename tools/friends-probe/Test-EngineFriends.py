"""OpenAI Codex: isolated real-engine Friends gameplay integration (Windows)."""
import argparse, pathlib, subprocess, time, os, json, re, socket, ctypes, zipfile
p=argparse.ArgumentParser();p.add_argument('executable',type=pathlib.Path);p.add_argument('gamedata',type=pathlib.Path);p.add_argument('output',type=pathlib.Path);p.add_argument('--arena',action='store_true');p.add_argument('--links',action='store_true',help='Test Windows URI registration; leave the supplied engine/gamedata registered afterward');a=p.parse_args()
exe=a.executable.resolve();assets=a.gamedata.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
children=[];handles=[];serial=0
def text(path,s):path.write_text(s,encoding='ascii')
def profile(name,host=False,invite=None):
 d=out/name
 for folder in ['id1','qw','ezquake']:
  (d/folder).mkdir(parents=True)
  for f in (assets/folder).iterdir():
   if f.is_file() and f.suffix.lower() in ['.pak','.pk3']:os.link(f,d/folder/f.name)
 if host:
  # Exercise the actual package's server module, including nQuake's stock QVM.
  modules=[assets/'qw/qwprogs.dll',assets/'qw/qwprogs.qvm']
  module=next((f for f in modules if f.is_file()),None)
  if module:os.link(module,d/'qw'/module.name)
  else:assert any('qwprogs.qvm' in zipfile.ZipFile(f).namelist() for f in d.rglob('*.pk3')),'Missing packaged KTX QVM'
 text(d/'qw/control.cfg','')
 text(d/'qw/poll.cfg','alias f_spawn "exec poll.cfg"\ndev_friends poll\n')
 common='cfg_save_onquit 0\ncl_confirmquit 0\ncl_onload console\ncl_maxfps 60\ncl_physfps 60\nvid_vsync 0\ndeveloper 1\ntp_triggers 1\nset friends_test_serial 0\nname '+name+'\n'
 if host:
  text(d/'qw/online.cfg','alias f_spawn "exec poll.cfg"\nexec poll.cfg\n')
  run=common+'sv_progtype 1\nsv_progsname qwprogs\nmaxclients 16\ndeathmatch 3\ncoop 0\nalias f_spawn "exec online.cfg"\nmap dm6\n'
 else:
  run=common+'alias f_spawn "exec poll.cfg"\ndev_friends join "'+str(invite).replace('\\','/')+'"\n'
 if host and a.arena:
  run=common+'alias f_spawn "exec poll.cfg"\nmenu_local\ndev_local_menu map dm6\ndev_local_menu bots 1\ndev_local_menu friends 1\ndev_local_menu start\n'
 if not host and a.links:run=common+'alias f_spawn "exec poll.cfg"\nexec poll.cfg\n'
 text(d/'qw/run.cfg',run)
 err=(d/'stderr.log').open('w');handles.append(err)
 args=[str(exe),'-allowmultiple','-condebug','-nohome','-basedir',str(d),'-window','-width','640','-height','480','+set','vid_renderer','2','+exec','run.cfg']
 if not host and a.links and name=='guest1':args+=['-friends-invite',invite.read_text()]
 child=subprocess.Popen(args,cwd=d,stdout=err,stderr=err,creationflags=subprocess.CREATE_NO_WINDOW)
 children.append(child)
 if not host and a.links:
  if name=='guest2':
   wait(lambda:'ezQuake Initialized' in log(d))
   register=subprocess.run([str(exe),'-friends-register',str(d)],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=15,creationflags=subprocess.CREATE_NO_WINDOW);assert register.returncode==0
   shell=ctypes.windll.shell32.ShellExecuteW;shell.argtypes=[ctypes.c_void_p,ctypes.c_wchar_p,ctypes.c_wchar_p,ctypes.c_wchar_p,ctypes.c_wchar_p,ctypes.c_int];shell.restype=ctypes.c_void_p
   result=shell(None,'open',invite.read_text(),None,str(d),0);assert result and result>32,'Windows failed to open invitation'
  wait(lambda:'waiting for your confirmation' in log(d))
  command(d,'dev_friends status\ncl_cmdline\ncfg_save uri-test')
  assert 'FRIENDS_UI menu=1 confirm=2' in log(d),'Link did not open confirmation'
  assert 'Connected through' not in log(d),'Link auto-joined without confirmation'
  for f in [d/'qw/qconsole.log',*d.rglob('uri-test.cfg')]:assert invite.read_text().split('#key=')[1][:64].encode() not in f.read_bytes(),'Invitation secret leaked to engine report/config'
  if name=='guest2':
   assert 'registered=1' in log(d),'Registration points at the wrong installation'
   command(d,'dev_friends key escape\ndev_friends status');assert log(d).rfind('confirm=0')>log(d).rfind('confirm=2'),'Cancel did not preserve the game'
   forward=subprocess.run([str(exe),'-basedir',str(d),'-friends-invite',invite.read_text()],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=10,creationflags=subprocess.CREATE_NO_WINDOW);assert forward.returncode==0
   wait(lambda:log(d).count('waiting for your confirmation')>=2)
  command(d,'dev_friends key enter',ack=False)
 return d,child
def log(d):
 f=d/'qw/qconsole.log';return ''.join(chr(b & 127) for b in f.read_bytes()) if f.exists() else ''
def wait(fn,timeout=50):
 end=time.monotonic()+timeout
 while time.monotonic()<end:
  if fn():return
  time.sleep(.2)
 raise RuntimeError('Engine test timed out; inspect isolated console logs')
def command(d,commands,ack=True):
 global serial
 serial+=1;text(d/'qw'/('job%d.cfg'%serial),'set friends_test_serial %d\n'%serial+commands+'\necho FRIENDS_TEST_DONE_%d\n'%serial)
 tmp=d/'qw/control.tmp';text(tmp,'if $friends_test_serial != %d then exec job%d.cfg\n'%(serial,serial));os.replace(tmp,d/'qw/control.cfg')
 if ack:wait(lambda:'FRIENDS_TEST_DONE_%d'%serial in log(d),20)
try:
 host,hp=profile('host',True);wait(lambda:'host entered the game' in log(host))
 if not a.arena:
  with socket.socket(socket.AF_INET,socket.SOCK_DGRAM) as udp:
   udp.settimeout(2);udp.sendto(b'\xff\xff\xff\xffgetchallenge\n',('127.0.0.1',27500));response=udp.recv(2048);assert response.startswith(b'\xff\xff\xff\xff'),'Normal UDP unavailable'
  command(host,'friends_host')
 wait(lambda:'Ready. Copy invitation' in log(host));invite=out/'invitation.txt'
 if a.arena:
  command(host,'dev_local_menu inspect');assert re.search(r'LOCAL_MENU .*bots=1',log(host)), 'Arena bot did not start'
 with socket.socket(socket.AF_INET,socket.SOCK_DGRAM) as udp:
  udp.settimeout(1);udp.sendto(b'\xff\xff\xff\xffgetchallenge\n',('127.0.0.1',27500))
  try:udp.recv(2048);raise AssertionError('Plain UDP bypassed invitation gate')
  except socket.timeout:pass
 command(host,'dev_friends export "'+str(invite).replace('\\','/')+'"');wait(lambda:invite.exists())
 if a.links:
  for badargs in [[invite.read_text(),'+quit'],['not-an-invitation']]:
   rejected=subprocess.run([str(exe),'-friends-invite',*badargs],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=10,creationflags=subprocess.CREATE_NO_WINDOW);assert rejected.returncode==2,'Unsafe URI startup arguments were accepted'
 g1,p1=profile('guest1',invite=invite);wait(lambda:'Connected through an encrypted Friends link.' in log(g1))
 wait(lambda:'guest1 entered the game' in log(host))
 command(g1,'dev_friends status\nstatus')
 g2,p2=profile('guest2',invite=invite);wait(lambda:'Connected through an encrypted Friends link.' in log(g2))
 wait(lambda:'guest2 entered the game' in log(host))
 command(g2,'dev_friends status\nstatus')
 command(host,'dev_friends status\nstatus\nmenu_friends\nscreenshot')
 assert re.search(r'FRIENDS_STATE .*peers=2 .*players=3',log(host)),'Host did not spawn two remote players'
 assert re.search(r'FRIENDS_STATE .*client=4',log(g1)), 'Guest1 not active'
 assert re.search(r'FRIENDS_STATE .*client=4',log(g2)), 'Guest2 not active'
 command(g1,'dev_friends join "'+str(invite).replace('\\','/')+'"',ack=False)
 wait(lambda:log(host).count('guest1 entered the game')>=2,45)
 command(g1,'dev_friends status');command(host,'dev_friends status')
 assert re.search(r'FRIENDS_STATE .*peers=2 .*players=3',log(host).split('guest1 entered the game')[-1]), 'Rejoin left a stale player or lost the guest'
 command(host,'friends_close')
 command(g1,'say FRIENDS_GAME_CHAT_TEST\n+forward')
 time.sleep(1);command(g1,'-forward\ndev_friends status')
 command(host,'dev_friends status')
 wait(lambda:'FRIENDS_GAME_CHAT_TEST' in log(g2),10)
 command(host,'friends_host\nmap dm2')
 def active_on_new_map(d):
  command(d,'dev_friends status')
  return bool(re.search(r'FRIENDS_STATE .*client=4 .*map=dm2',log(d)))
 wait(lambda:active_on_new_map(g1),45);wait(lambda:active_on_new_map(g2),45);wait(lambda:active_on_new_map(host),45)
 command(host,'dev_friends status\nstatus')
 assert re.search(r'FRIENDS_STATE .*client=4 .*map=dm2',log(g1)) and re.search(r'FRIENDS_STATE .*client=4 .*map=dm2',log(g2)), 'Map change not replicated'
 assert re.search(r'FRIENDS_STATE .*peers=2 .*players=3 map=dm2',log(host)), 'Host lost guests on map change'
 for d in [host,g1,g2]:assert 'Unknown command "changelevel"' not in log(d)
 for d,c in [(g1,p1),(g2,p2),(host,hp)]:
  serial+=1;text(d/'qw'/('job%d.cfg'%serial),'set friends_test_serial %d\nquit\n'%serial)
  tmp=d/'qw/control.tmp';text(tmp,'if $friends_test_serial != %d then exec job%d.cfg\n'%(serial,serial));os.replace(tmp,d/'qw/control.cfg')
  c.wait(timeout=20);assert c.returncode==0,'Engine did not exit cleanly'
 result={'pass':True,'map':'dm6','mapTransition':'dm2','spawnedPlayers':3,'friendsGuests':2,'sameProcessRejoin':True,'chatReplicated':True,'closeInvitesKeepsGuests':True,'ordinaryUDPWorks':not a.arena,'arenaAutoInvitesAndBot':a.arena,'invitationGateBlocksPlainUDP':True,'cleanExit':True,'scope':'local processes through existing Frag-Net broker, native Vulkan engine'}
 result.update(uriColdStart=a.links,uriWindowsShellAndExistingProcess=a.links,uriConfirmationAndCancel=a.links,uriSecretRedacted=a.links,uriInjectionRejected=a.links)
 (out/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
finally:
 for c in children:
  if c.poll() is None:c.terminate()
 for c in children:c.wait(timeout=15)
 for h in handles:h.close()
 if a.links:
  register=subprocess.run([str(exe),'-friends-register',str(assets)],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=15,creationflags=subprocess.CREATE_NO_WINDOW)
  assert register.returncode==0,'Could not restore final game installation registration'
