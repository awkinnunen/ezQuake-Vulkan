# DIST-002. OpenAI Codex, 2026-09-16. Shared installer functions (Windows PowerShell 5.1+).
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
Add-Type -AssemblyName System.IO.Compression.FileSystem
Add-Type -AssemblyName System.IO.Compression

function Get-VerifiedDownload($Package, [string]$CacheDirectory) {
    if ($Package.url -notmatch '^https://github\.com/(nQuake/distfiles|awkinnunen/ezQuake-Vulkan)/releases/download/') { throw 'Unexpected download origin.' }
    if ($Package.name -notmatch '^[A-Za-z0-9_.-]+\.zip$' -or $Package.sha256 -notmatch '^[a-f0-9]{64}$') { throw 'Invalid download manifest.' }
    New-Item -ItemType Directory -Force -Path $CacheDirectory | Out-Null
    $path=Join-Path $CacheDirectory ($Package.sha256.Substring(0,12)+'-'+$Package.name)
    if (!(Test-Path -LiteralPath $path)) {
        $partial=$path+'.'+[guid]::NewGuid().ToString('N')+'.part'
        Write-Host "Downloading $($Package.name)..."
        [Net.ServicePointManager]::SecurityProtocol=[Net.SecurityProtocolType]::Tls12
        $client=New-Object Net.WebClient
        $client.Headers['User-Agent']='ezQuake-Vulkan-Starter/0.1.0'
        try { $client.DownloadFile($Package.url,$partial) } finally { $client.Dispose() }
        if ((Get-Item -LiteralPath $partial).Length -ne $Package.bytes -or
            (Get-FileHash -LiteralPath $partial -Algorithm SHA256).Hash.ToLowerInvariant() -ne $Package.sha256) {
            throw "Download verification failed: $($Package.name). The upstream snapshot may have changed; obtain an updated Starter. File retained: $partial"
        }
        [IO.File]::Move($partial,$path)
    }
    if ((Get-Item -LiteralPath $path).Length -ne $Package.bytes -or
        (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $Package.sha256) {
        throw "Cached download failed verification: $path. Move it aside and try again."
    }
    return $path
}

function Get-SafePath([string]$Root,[string]$Relative) {
    # Reject Windows aliases, streams, absolute paths, traversal and ambiguous names.
    $parts=$Relative.Replace('\','/').Split('/')
    if ([IO.Path]::IsPathRooted($Relative) -or $Relative.Contains(':') -or
        @($parts | Where-Object { $_ -eq '..' -or $_ -eq '.' -or $_ -match '[. ]$|[<>"|?*\x00-\x1f]' -or $_ -match '^(?i:CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(?:\.|$)' }).Count) {
        throw "Unsafe archive path: $Relative"
    }
    $prefix=[IO.Path]::GetFullPath($Root).TrimEnd('\')+'\'
    $path=[IO.Path]::GetFullPath((Join-Path $Root $Relative))
    if (!$path.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase)) { throw "Path escaped destination: $Relative" }
    return $path
}

function Expand-CheckedZip([string]$Archive,[string]$Destination,[string]$Kind='data') {
    $zip=[IO.Compression.ZipFile]::OpenRead($Archive)
    try {
        # Preflight the entire archive before writing anything.
        $seen=@{}; $total=0L
        foreach ($entry in $zip.Entries) {
            if (!$entry.Name) { continue }
            $path=Get-SafePath $Destination $entry.FullName
            if ($seen.ContainsKey($path)) { throw "Duplicate archive entry: $($entry.FullName)" }
            $seen[$path]=$true; $total+=$entry.Length
            if ($total -gt 4GB) { throw 'Archive exceeds the installation size limit.' }
        }
        foreach ($entry in $zip.Entries) {
            if (!$entry.Name) { continue }
            $relative=$entry.FullName.Replace('\','/')
            if ($Kind -eq 'shareware') {
                if ($relative -ieq 'id1/pak0.pak') { $relative='id1/pak0.pak' }
                elseif ($entry.Name -match '(?i)\.txt$') { $relative='licenses/quake-shareware/'+$entry.Name }
                else { continue }
            } elseif ($Kind -eq 'gpl' -and $relative -ieq 'ezquake.exe') { continue }
            $path=Get-SafePath $Destination $relative
            New-Item -ItemType Directory -Force -Path (Split-Path $path -Parent) | Out-Null
            # The pinned upstream packages overlap on ten player skins; nQuake's
            # non-GPL layer follows GPL (orange differs, the other nine are identical).
            $overlay=$Kind -eq 'nongpl' -and $relative -match '^qw/skins/player_(yellow|orange|purple|green|pink|blue|white|cyan|base|red)\.png$'
            if ((Test-Path -LiteralPath $path) -and !$overlay) { throw "Package file collision: $relative" }
            [IO.Compression.ZipFileExtensions]::ExtractToFile($entry,$path,$overlay)
        }
    } finally { $zip.Dispose() }
}

function Get-PakEntries([string]$Path) {
    $stream=[IO.File]::OpenRead($Path); $reader=New-Object IO.BinaryReader($stream)
    try {
        if ([Text.Encoding]::ASCII.GetString($reader.ReadBytes(4)) -ne 'PACK') { throw "Not a Quake PAK: $Path" }
        $offset=$reader.ReadInt32(); $length=$reader.ReadInt32()
        if ($offset -lt 12 -or $length -lt 0 -or $length % 64 -ne 0 -or $length -gt 4MB -or [long]$offset+$length -gt $stream.Length) { throw "Invalid PAK directory: $Path" }
        $null=$stream.Seek($offset,[IO.SeekOrigin]::Begin)
        for ($i=0;$i -lt $length/64;$i++) {
            $name=[Text.Encoding]::ASCII.GetString($reader.ReadBytes(56)).Split([char]0)[0].ToLowerInvariant()
            $start=$reader.ReadInt32(); $size=$reader.ReadInt32()
            if ($start -lt 0 -or $size -lt 0 -or [long]$start+$size -gt $stream.Length) { throw "Invalid PAK entry: $Path" }
            $name
        }
    } finally { $reader.Dispose(); $stream.Dispose() }
}

function Import-RegisteredData([string]$Source,[string]$GameDirectory) {
    $sourcePath=(Resolve-Path -LiteralPath $Source).Path
    if (Test-Path -LiteralPath (Join-Path $sourcePath 'id1')) { $sourcePath=Join-Path $sourcePath 'id1' }
    $paks=@(Get-ChildItem -LiteralPath $sourcePath -File | Where-Object { $_.Name -match '(?i)^pak[1-9][0-9]*\.pak$' } | Sort-Object Name)
    $entries=@($paks | ForEach-Object { Get-PakEntries $_.FullName })
    if ($entries -notcontains 'gfx/pop.lmp' -or $entries -notcontains 'maps/e2m1.bsp') {
        throw 'No complete classic registered Quake PAK set found. Select the original game folder (or id1), not the rerelease folder. No files were imported.'
    }
    foreach ($pak in $paks) {
        if (Test-Path -LiteralPath (Join-Path $GameDirectory ('id1/'+$pak.Name))) { throw "Import would replace an existing file: $($pak.Name)" }
    }
    $records=@()
    foreach ($pak in $paks) {
        $target=Join-Path $GameDirectory ('id1/'+$pak.Name.ToLowerInvariant())
        Copy-Item -LiteralPath $pak.FullName -Destination $target
        $hash=(Get-FileHash -LiteralPath $pak.FullName -Algorithm SHA256).Hash
        if ((Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash -ne $hash) { throw 'Imported PAK verification failed.' }
        $records+=@{file=$pak.Name;sha256=$hash;bytes=$pak.Length}
    }
    # Use the owned maps; keep nQuake's substitute maps for easy restoration.
    $substitute=Join-Path $GameDirectory 'id1/gpl_maps.pk3'
    if (Test-Path -LiteralPath $substitute) { Move-Item -LiteralPath $substitute -Destination ($substitute+'.disabled') }
    return $records
}
