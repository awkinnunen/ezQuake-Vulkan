"""Two owned probe processes through the existing broker. NOT a two-network test.

Creates one short-lived public test room per case; no external relay is deployed.
Never prints invitation keys, peer addresses or SDP. Python is a developer test
dependency only; the player test package contains the native executable.
"""
from pathlib import Path
import argparse, json, subprocess, tempfile, time

p = argparse.ArgumentParser()
p.add_argument('--exe', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
p.add_argument('--case', choices=['success', 'wrong-key', 'wrong-fingerprint'], default='success')
p.add_argument('--join-delay', type=int, default=0, help='Idle host seconds before joining (0..3600)')
a = p.parse_args()
if not 0 <= a.join_delay <= 3600: p.error('join-delay must be 0..3600')
a.output.mkdir(parents=True, exist_ok=True)
work = Path(tempfile.mkdtemp(prefix=a.case+'-', dir=a.output.resolve()))
exe = a.exe.resolve()
processes = []
try:
    with (work/'host.log').open('w', encoding='utf-8') as hostlog, (work/'join.log').open('w', encoding='utf-8') as joinlog:
        host = subprocess.Popen([str(exe), '--host', '--report', 'host-result.json'], cwd=work, stdout=hostlog, stderr=subprocess.STDOUT)
        processes.append(host)
        deadline = time.monotonic()+25
        invitation = work/'invitation.txt'
        while not invitation.exists():
            if host.poll() is not None or time.monotonic()>deadline:
                raise RuntimeError('Host did not publish an invitation: '+(work/'host.log').read_text(encoding='utf-8'))
            time.sleep(.1)
        text = invitation.read_text(encoding='utf-8').strip()
        delay_end = time.monotonic()+a.join_delay
        while time.monotonic()<delay_end:
            if host.poll() is not None: raise RuntimeError('Host exited during idle wait')
            time.sleep(.1)
        assert invitation.read_text(encoding='utf-8').strip()==text, 'Invitation changed while idle'
        if a.case == 'wrong-key':
            prefix, rest = text.split('#key='); _, fp = rest.split('&fp=')
            text = prefix+'#key='+'0'*64+'&fp='+fp
        elif a.case == 'wrong-fingerprint':
            text = text.split('&fp=')[0]+'&fp='+'0'*64
        (work/'join-invitation.txt').write_text(text, encoding='utf-8')
        join = subprocess.Popen([str(exe), '--join-file', 'join-invitation.txt', '--seconds', '30', '--report', 'join-result.json'], cwd=work, stdout=joinlog, stderr=subprocess.STDOUT)
        processes.append(join)
        jr = join.wait(timeout=40)
        hr = host.wait(timeout=50)
    reports = {role: json.loads((work/(role+'-result.json')).read_text(encoding='utf-8')) for role in ('host','join')}
    if a.case == 'success':
        assert hr == jr == 0, 'Probe process failed'
        assert all(r['pass'] and r['admitted'] and r['received']==100 for r in reports.values())
    else:
        assert hr != 0 and jr != 0, 'Negative case incorrectly succeeded'
        assert not reports['host']['admitted'] and not any(r['pass'] for r in reports.values())
        if a.case == 'wrong-fingerprint':
            assert not reports['join']['dtlsPinned'], 'Wrong host certificate accepted'
    evidence = {'case': a.case, 'idleSeconds':a.join_delay, 'pass': True, 'scope':'Two processes on one Windows machine; real Frag-Net broker. Not cross-network evidence.', 'results': reports}
    (work/'validation.json').write_text(json.dumps(evidence, indent=2)+'\n', encoding='utf-8')
    print(json.dumps(evidence, indent=2))
except Exception:
    for role in ('host','join'):
        log = work/(role+'.log')
        if log.exists(): print(role+':\n'+log.read_text(encoding='utf-8'))
    raise
finally:
    for proc in processes:
        if proc.poll() is None:
            proc.terminate()
            try: proc.wait(timeout=5)
            except subprocess.TimeoutExpired: proc.kill(); proc.wait()
    for name in ('invitation.txt','join-invitation.txt'):
        (work/name).unlink(missing_ok=True)
