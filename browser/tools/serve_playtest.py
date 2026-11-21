"""Serve the complete source browser build on a loopback-only origin.

The former hardcoded eight-file allowlist silently blocked the cinematic,
story, MGS equipment script, interactive atlas and EVERY native .glb. That
made a downloaded ZIP appear to run while important assets 404ed. Python's
SimpleHTTPRequestHandler supports correct MIME types and normalized paths,
and is restricted to the browser/ root and 127.0.0.1.
"""
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import webbrowser

ROOT = Path(__file__).resolve().parents[1]


def serve(host="127.0.0.1", port=19870, launch=True):
    handler = partial(SimpleHTTPRequestHandler, directory=str(ROOT))
    with ThreadingHTTPServer((host, port), handler) as server:
        url = f"http://{host}:{port}/"
        print("Keep this window open while playing:", url, flush=True)
        print("Architectural atlas:", url + "atlas.html", flush=True)
        if launch:
            webbrowser.open(url)
        server.serve_forever()


if __name__ == "__main__":
    serve()
