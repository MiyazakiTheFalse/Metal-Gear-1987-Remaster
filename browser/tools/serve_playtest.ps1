# Windows PowerShell 5.1 fallback when Python is not installed.
# Serve the COMPLETE localhost browser tree, including dynamic atlas assets and
# binary GLBs. Normalize and containment-check every decoded request path.
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$rootWithSlash = $root.TrimEnd([IO.Path]::DirectorySeparatorChar) +
    [IO.Path]::DirectorySeparatorChar
$mime = @{
  '.html'='text/html';'.js'='text/javascript';'.css'='text/css'
  '.json'='application/json';'.glb'='model/gltf-binary'
  '.png'='image/png';'.jpg'='image/jpeg';'.jpeg'='image/jpeg'
  '.svg'='image/svg+xml';'.webp'='image/webp'
  '.wav'='audio/wav';'.ogg'='audio/ogg';'.mp3'='audio/mpeg'
  '.txt'='text/plain'
}
$listener = [Net.Sockets.TcpListener]::new([Net.IPAddress]::Loopback,19870)
try {
    $listener.Start()
    $url = 'http://127.0.0.1:19870/'
    Write-Host "Opening $url. Keep this window open while playing."
    Write-Host "Whole Outer Heaven atlas: $($url)atlas.html"
    Start-Process $url
    while ($true) {
        $client = $listener.AcceptTcpClient()
        try {
            $client.ReceiveTimeout = 2000
            $client.SendTimeout = 2000
            $stream = $client.GetStream()
            $reader = [IO.StreamReader]::new($stream)
            $request = $reader.ReadLine()
            $target = $null
            if ($request -match '^GET /([^ ?]*)(?:\?[^ ]*)? HTTP/1\.[01]$') {
                $name = [Uri]::UnescapeDataString($Matches[1]).Replace('/', '\')
                if ([string]::IsNullOrWhiteSpace($name)) { $name = 'index.html' }
                $candidate = [IO.Path]::GetFullPath((Join-Path $root $name))
                $extension = [IO.Path]::GetExtension($candidate).ToLowerInvariant()
                if ($candidate.StartsWith($rootWithSlash,
                      [StringComparison]::OrdinalIgnoreCase) -and
                    [IO.File]::Exists($candidate) -and $mime.ContainsKey($extension)) {
                    $target = $candidate
                }
            }
            if ($target) {
                $body = [IO.File]::ReadAllBytes($target)
                $contentType = $mime[[IO.Path]::GetExtension($target).ToLowerInvariant()]
                $status = '200 OK'
            } else {
                $body = [byte[]]@()
                $contentType = 'text/plain'
                $status = '404 Not Found'
            }
            $header = [Text.Encoding]::ASCII.GetBytes(
              "HTTP/1.1 $status`r`nContent-Type: $contentType`r`nContent-Length: $($body.Length)`r`nCache-Control: no-store`r`nConnection: close`r`n`r`n")
            $stream.Write($header,0,$header.Length)
            $stream.Write($body,0,$body.Length)
        } catch [IO.IOException] {
            # Browser may close idle connections; normal on loopback.
        } finally { $client.Dispose() }
    }
} finally { $listener.Stop() }
