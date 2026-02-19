$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path

function Start-Component {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][string]$RelativeExePath,
        [int]$DelayMsAfterStart = 0
    )

    $exePath = Join-Path $repoRoot $RelativeExePath
    if (-not (Test-Path $exePath)) {
        Write-Warning "[$Name] not found: $exePath"
        return $null
    }

    $workDir = Split-Path -Parent $exePath
    $proc = Start-Process -FilePath $exePath -WorkingDirectory $workDir -PassThru
    Write-Host ("[{0}] started. PID={1}  EXE={2}" -f $Name, $proc.Id, $exePath)

    if ($DelayMsAfterStart -gt 0) {
        Start-Sleep -Milliseconds $DelayMsAfterStart
    }

    return $proc
}

$components = @(
    @{ Name = "GateServer";  Exe = "GateServer\x64\Debug\GateServer.exe";      Delay = 200 },
    @{ Name = "VerifyServer"; Exe = "VerifyServer\x64\Debug\VerifyServer.exe"; Delay = 200 },
    @{ Name = "StatusServer"; Exe = "StatusServer\x64\Debug\StatusServer.exe"; Delay = 200 },
    @{ Name = "ResourceServer"; Exe = "ResourceServer\x64\Debug\ResourceServer.exe"; Delay = 300 },
    @{ Name = "ChatServer1"; Exe = "ChatServer\x64\Debug\ChatServer.exe";      Delay = 300 },
    @{ Name = "ChatServer2"; Exe = "ChatServer2\x64\Debug\ChatServer.exe";     Delay = 500 },
    @{ Name = "Client1"; Exe = "TinyChat\x64\Debug\TinyChat.exe";              Delay = 300 },
    @{ Name = "Client2"; Exe = "TinyChat\x64\Debug\TinyChat.exe";              Delay = 0 }
)

$started = @()
foreach ($c in $components) {
    $p = Start-Component -Name $c.Name -RelativeExePath $c.Exe -DelayMsAfterStart $c.Delay
    if ($null -ne $p) {
        $started += $p
    }
}

Write-Host ""
Write-Host ("Started {0} process(es)." -f $started.Count)
if ($started.Count -gt 0) {
    $started | ForEach-Object {
        Write-Host ("  PID={0}  Name={1}" -f $_.Id, $_.ProcessName)
    }
}
