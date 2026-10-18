#!/usr/bin/env python3
"""Serve ORBIT locally. No installation or external network connection is needed."""
from http.server import ThreadingHTTPServer, SimpleHTTPRequestHandler
from functools import partial
from pathlib import Path
import socket
root = Path(__file__).resolve().parent
handler = partial(SimpleHTTPRequestHandler, directory=str(root))
server = None
for port in range(8765, 8776):
    try:
        server = ThreadingHTTPServer(('127.0.0.1', port), handler)
        break
    except OSError:
        continue
if server is None:
    raise SystemExit('利用可能なポートがありません。8765〜8775番の使用状況を確認してください。')
print(f'\nORBIT Browser Lab\nブラウザーで開いてください: http://127.0.0.1:{port}/\n終了: Control+C\n', flush=True)
try:
    server.serve_forever()
except KeyboardInterrupt:
    pass
finally:
    server.server_close()
