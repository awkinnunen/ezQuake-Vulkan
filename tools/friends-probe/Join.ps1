# FRIENDS-002, OpenAI Codex. The invitation is data; never evaluate it as a command.
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    $invite = Read-Host 'Paste the complete invitation, then press Enter'
    if (!$invite -or $invite.Length -gt 512) { throw 'Missing or oversized invitation.' }
    $file = Join-Path $PSScriptRoot 'received-invitation.txt'
    try {
        [IO.File]::WriteAllText($file, $invite, (New-Object Text.UTF8Encoding($false)))
        & (Join-Path $PSScriptRoot 'FriendsProbe.exe') --join-file $file --report join-result.json --seconds 90
        $probeExit = $LASTEXITCODE
    } finally {
        if (Test-Path -LiteralPath $file) { Remove-Item -LiteralPath $file }
    }
    Write-Host 'Test finished. Share join-result.json for diagnosis.'
    exit $probeExit
} finally { Pop-Location }
