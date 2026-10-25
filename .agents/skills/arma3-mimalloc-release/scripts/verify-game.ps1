param(
    [Parameter(Mandatory)]
    [ValidateScript({
        if ($_ -match '^[^\\/]+\.dll$') { return $true }
        throw 'Pass only the DLL filename (no path), including the .dll extension.'
    })]
    [string]$DllFileName
)

$ErrorActionPreference = 'Stop'
$allocatorName = [IO.Path]::GetFileNameWithoutExtension($DllFileName)
$rptDir = Join-Path $env:LOCALAPPDATA 'Arma 3'

if (-not (Get-Process -Name steam -ErrorAction SilentlyContinue)) {
    throw 'Start Steam before running this verification.'
}

$steamRoot = (Get-ItemProperty 'HKCU:\Software\Valve\Steam' `
    -ErrorAction SilentlyContinue).SteamPath

if (-not $steamRoot) {
    $steamRoot = (Get-ItemProperty `
        'HKLM:\SOFTWARE\WOW6432Node\Valve\Steam' `
        -ErrorAction SilentlyContinue).InstallPath
}

if (-not $steamRoot -or -not (Test-Path -LiteralPath $steamRoot)) {
    throw 'Could not detect the Steam installation directory.'
}

$steamRoot = $steamRoot -replace '/', '\'
$libraryPaths = @($steamRoot)
$vdfPath = Join-Path $steamRoot 'steamapps\libraryfolders.vdf'

if (Test-Path -LiteralPath $vdfPath) {
    $vdf = Get-Content -LiteralPath $vdfPath -Raw
    $libraryPaths += @(
        [regex]::Matches($vdf, '"path"\s*"([^"]+)"') |
        ForEach-Object { $_.Groups[1].Value -replace '\\\\', '\' }
    )
}

$gameDir = $null
foreach ($library in ($libraryPaths | Select-Object -Unique)) {
    $manifest = Join-Path $library 'steamapps\appmanifest_107410.acf'
    $candidate = Join-Path $library 'steamapps\common\Arma 3'

    if ((Test-Path -LiteralPath $manifest) -and
        (Test-Path -LiteralPath (Join-Path $candidate 'arma3_x64.exe'))) {
        $gameDir = $candidate
        break
    }
}

if (-not $gameDir) {
    throw 'Could not find Arma 3 in the detected Steam libraries.'
}

$dllPath = Join-Path (Join-Path $gameDir 'Dll') $DllFileName
if (-not (Test-Path -LiteralPath $dllPath)) {
    throw "DLL not found: $dllPath"
}

$startedAt = Get-Date
$gameProcess = $null
$foundRpt = $null
$allocatorLine = $null

try {
    $gameProcess = Start-Process `
        -FilePath (Join-Path $gameDir 'arma3_x64.exe') `
        -WorkingDirectory $gameDir `
        -ArgumentList @(
            '-noSplash',
            '-skipIntro',
            '-world=empty',
            '-hugePages',
            '-loadMissionToMemory',
            '-noPause',
            '-noSound',
            '-window',
            "-malloc=$allocatorName"
        ) `
        -PassThru

    for ($i = 0; $i -lt 60 -and -not $foundRpt; $i++) {
        Start-Sleep -Seconds 1

        $newRpts = Get-ChildItem -LiteralPath $rptDir `
            -Filter '*.rpt' -File -ErrorAction SilentlyContinue |
            Where-Object {
                $_.LastWriteTime -ge $startedAt -and
                $_.CreationTime -ge $startedAt
            } |
            Sort-Object LastWriteTime -Descending

        foreach ($file in $newRpts) {
            $allocatorMatch = Select-String -LiteralPath $file.FullName `
                -Pattern ('Allocator:.*' + [regex]::Escape($allocatorName)) `
                -ErrorAction SilentlyContinue |
                Select-Object -First 1

            if ($allocatorMatch) {
                $foundRpt = $file
                $allocatorLine = $allocatorMatch.Line
                break
            }
        }
    }
}
finally {
    if ($gameProcess -and -not $gameProcess.HasExited) {
        $null = $gameProcess.CloseMainWindow()

        if (-not $gameProcess.WaitForExit(5000)) {
            Stop-Process -Id $gameProcess.Id -Force `
                -ErrorAction SilentlyContinue
        }
    }
}

$crashFiles = @(
    Get-ChildItem -LiteralPath $rptDir -File -ErrorAction SilentlyContinue |
    Where-Object {
        $_.LastWriteTime -ge $startedAt -and
        $_.Extension -in @('.mdmp', '.bidmp')
    }
)

if (-not $foundRpt) {
    if ($crashFiles.Count -gt 0) {
        [Console]::Error.WriteLine(('FAIL: The game crashed before confirming the allocator. Crash artifacts: ' +
            (($crashFiles | ForEach-Object Name) -join ', ')))
    }
    else {
        [Console]::Error.WriteLine("FAIL: No new RPT confirmed allocator $DllFileName within 60 seconds.")
    }
    exit 1
}

Write-Output $allocatorLine

$failed = $false

if ($crashFiles.Count -gt 0) {
    [Console]::Error.WriteLine(('FAIL: New crash artifacts: ' +
        (($crashFiles | ForEach-Object Name) -join ', ')))
    $failed = $true
}

if ($failed) {
    exit 1
}

Write-Output 'PASS: Allocator confirmed and no new crash artifacts found.'
