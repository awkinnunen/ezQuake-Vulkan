"""DIST-001: verify startup precedence, installed config preservation and output values."""
from pathlib import Path
import re
root=Path(__file__).resolve().parent.parent
def read(path):return path.read_bytes().decode('latin1')
def values(path):return {m[1].lower():m[2] for m in re.finditer(r'^\s*(\w+)\s+"([^"\r\n]*)"\s*$',read(path),re.M)}
balanced=values(root/'profiles/ezquake/presets/graphics/builtin/Balanced.cfg')
assert len(balanced)==170
for label in ['distribution-fresh','distribution-existing','distribution-normal']:
 p=root/f'cache/runtime-{label}'
 result=p/'ezquake/configs/distribution-result.cfg';v=values(result);raw=read(result)
 if label!='distribution-normal':
  expected=dict(balanced)
  if label=='distribution-existing':expected.update(r_cv_edgewidth='2.6',r_cv_midtone='0.9')
  assert all(v[k]==value or float(v[k])==float(value) for k,value in expected.items()),label
 if label=='distribution-fresh':
  assert re.search(r'^bind\s+w\s+"\+legacy_fw"',raw,re.M)
 else:
  assert v['r_cv_edgewidth']=='2.6' and v['r_cv_midtone']=='0.9'
  assert re.search(r'^bind\s+w\s+"\+back"',raw,re.M)
  assert 'alias personal_test' in raw
  assert 'r_cv_edgewidth "1.1"' in read(p/'ezquake/configs/config.cfg')
  assert 'r_cv_edgewidth "2.6"' in read(p/'qw/autoexec.cfg')
 if label=='distribution-normal':assert v['r_cv_edgestrength']!='0.3','First-run defaults leaked into normal startup'
 log=read(p/'qw/qconsole.log')
 assert 'DISTRIBUTION_COMPLETE' in log and 'CODEX_SMOKE_COMPLETE' in log
 assert not re.search(r'VUID-|Validation Error|VK_ERROR_DEVICE_LOST',log)
print('PASS: 170 first-run graphics defaults; saved config overrides defaults; autoexec wins last; normal start skips package defaults; setup preserves user files and edited presets.')
