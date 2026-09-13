"""MAINT-003/004, OpenAI Codex. Packet-aligned private MVD fixture with a real end marker."""
from pathlib import Path
import argparse, hashlib, json, struct

p = argparse.ArgumentParser()
p.add_argument('source', type=Path)
p.add_argument('output', type=Path)
p.add_argument('--seconds', type=int, default=20)
a = p.parse_args()
data = a.source.read_bytes()
offset = elapsed = packets = 0
while offset < len(data):
    start = offset
    dt, typ = data[offset], data[offset + 1] & 7
    if elapsed + dt > a.seconds * 1000:
        break
    offset += 2
    if typ == 2:
        offset += 8
    elif typ in (1, 3, 4, 5, 6):
        if typ == 3:
            offset += 4
        length = struct.unpack_from('<I', data, offset)[0]
        offset += 4 + length
    else:
        raise ValueError(f'Unsupported MVD record {typ} at {start}')
    if offset > len(data):
        raise ValueError('Truncated source packet')
    elapsed += dt
    packets += 1
# An arbitrary EOF leaves streaming demo playback waiting for more data.
# Match CL_Stop_f: svc_disconnect + EndOfDemo, addressed to all clients.
payload = b'\x02EndOfDemo\0'
output = data[:offset] + bytes((0, 6)) + struct.pack('<I', len(payload)) + payload
a.output.parent.mkdir(parents=True, exist_ok=True)
a.output.write_bytes(output)
a.output.with_suffix('.json').write_text(json.dumps(dict(
    sourceSHA256=hashlib.sha256(data).hexdigest(), fixtureSHA256=hashlib.sha256(output).hexdigest(),
    milliseconds=elapsed, packets=packets, bytes=len(output), endMarker=True, private=True), indent=2)+'\n', encoding='utf-8')
print(f'Prepared {elapsed} ms, {packets} packets and EndOfDemo; source unchanged.')
