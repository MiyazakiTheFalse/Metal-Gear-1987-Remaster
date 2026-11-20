$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$files = @('index.html','game.js','dialogue.js','tutorial.js','assets/freight_plates.js','assets/field_rigs.js','field_renderer.js','rigged_characters.js')
$listener = [Net.Sockets.TcpListener]::new([Net.IPAddress]::Loopback,19870)
try {
    $listener.Start()
    $url = 'http://127.0.0.1:19870/'
    Write-Host "Opening $url. Keep this window open while playing."
    Start-Process $url
    while ($true) {
        $client = $listener.AcceptTcpClient()
        try {
            $client.ReceiveTimeout = 2000
            $client.SendTimeout = 2000
            $stream = $client.GetStream()
            $reader = [IO.StreamReader]::new($stream)
            $request = $reader.ReadLine()
            $path = $null
            if ($request -match '^GET /([^ ?]*)(?:\?[^ ]*)? HTTP/1\.[01]$') {
                $path = $Matches[1]
                if (!$path) { $path = 'index.html' }
            }
            if ($files -contains $path) {
                $body = [IO.File]::ReadAllBytes((Join-Path $root $path))
                $mime = if ($path.EndsWith('.js')) { 'text/javascript' } else { 'text/html' }
                $status = '200 OK'
            } else { $body = [byte[]]@(); $mime = 'text/plain'; $status = '404 Not Found' }
            $header = [Text.Encoding]::ASCII.GetBytes("HTTP/1.1 $status`r`nContent-Type: $mime; charset=utf-8`r`nContent-Length: $($body.Length)`r`nConnection: close`r`n`r`n")
            $stream.Write($header,0,$header.Length)
            $stream.Write($body,0,$body.Length)
        } catch [IO.IOException] {
            # The browser may close an idle connection without making a request.
        } finally { $client.Dispose() }
    }
} finally { $listener.Stop() }
