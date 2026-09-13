"""Procedural test fixtures only; OpenAI Codex, 2026-09-13. No game assets edited."""
from pathlib import Path
import struct
import sys
import zlib
target = Path(sys.argv[1]) / 'qw/env'
target.mkdir(parents=True, exist_ok=True)
def chunk(kind, content):
    return struct.pack('>I', len(content)) + kind + content + struct.pack('>I', zlib.crc32(kind+content))
for i, (side, w, h) in enumerate([('rt',256,256),('bk',512,512),('lf',1024,1024),
                                 ('ft',300,173),('up',1,513),('dn',513,1)]):
    # Distinct constant colours make any accidental face substitution visible.
    pixel = bytes([32+i*30, 180-i*20, 50+i*25])
    raw = (b'\0' + pixel*w)*h
    data = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR',struct.pack('>IIBBBBB',w,h,8,2,0,0,0))
    data += chunk(b'IDAT',zlib.compress(raw)) + chunk(b'IEND',b'')
    (target/f'codex{side}.png').write_bytes(data)
print('Created isolated test sky: 256, 512, 1024, 300x173, 1x513, 513x1.')
