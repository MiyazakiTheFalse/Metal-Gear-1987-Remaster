"""Open the extracted browser playtest on a stable, loopback-only address."""
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlsplit
import webbrowser

ROOT = Path(__file__).resolve().parents[1]
FILES = {"index.html", "game.js", "dialogue.js", "tutorial.js",
         "assets/freight_plates.js", "assets/field_rigs.js", "field_renderer.js", "rigged_characters.js"}

class GameFiles(BaseHTTPRequestHandler):
    def do_GET(self):
        name = urlsplit(self.path).path[1:] or "index.html"
        if name not in FILES:
            self.send_error(404)
            return
        content = (ROOT / name).read_bytes()
        self.send_response(200)
        self.send_header("Content-Type", ("text/html" if name.endswith(".html") else "text/javascript") + "; charset=utf-8")
        self.send_header("Content-Length", str(len(content)))
        self.end_headers()
        self.wfile.write(content)

if __name__ == "__main__":
    with ThreadingHTTPServer(("127.0.0.1", 19870), GameFiles) as server:
        url = "http://127.0.0.1:19870/"
        print("Keep this window open while playing.", url, flush=True)
        webbrowser.open(url)
        server.serve_forever()
