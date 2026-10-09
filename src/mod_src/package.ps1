# Puts the files a player needs into one folder and one zip. PowerShell 7. Opens no window.
#   pwsh -NoProfile -File mod_src/package.ps1 [-Control]
# Run mod_src/build.ps1 first. Output: mod_src/_build/package/RealisticEconomy-<version>/ and the zip next to it.
# The package is laid out to be extracted into the game folder: the two batch files and the two descriptions on top,
# everything else in the folder RealisticEconomy, from which the install file copies three files next to TGL2.exe.
# The last line is `package=<zip> files=<n> verdict=PASS|FAIL`. -Control checks the settings as if a test value had
# been left in them (trace=1), which must give FAIL; the package it writes is the normal one.
param([switch]$Control)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$build = Join-Path $PSScriptRoot '_build'
$dist = Join-Path $PSScriptRoot 'dist'
$loader = Join-Path $root 'analysis\_tools\asi_loader_v9.7.4\dinput8.dll' # Ultimate ASI Loader v9.7.4, see TOOLS_USED.md
$loaderHash = 'D5A059AA467A7A7127C8F6169F79FA63FF0F55986EE9EB2FD9A281BEBF2AA2E6'
$asi = Join-Path $build 'RealisticEconomy.asi'
$utf8Bom = [Text.UTF8Encoding]::new($true)
$utf8 = [Text.UTF8Encoding]::new($false)
$batches = 'RealisticEconomy_install.bat', 'RealisticEconomy_uninstall.bat'

$sources = $asi, $loader, (Join-Path $dist 'RealisticEconomy.ini'), (Join-Path $dist 'README_ko.txt'), (Join-Path $dist 'README_en.txt'), (Join-Path $dist 'licenses\Ultimate-ASI-Loader-LICENSE.txt'), (Join-Path $dist 'licenses\RealisticEconomy-LICENSE.txt')
$sources += $batches | ForEach-Object { Join-Path $dist $_ }
$missing = @($sources | Where-Object { -not (Test-Path -LiteralPath $_) })
if ($missing.Count) { $missing | ForEach-Object { "missing: $_" }; "package= files=0 verdict=FAIL"; exit 1 }
if ((Get-FileHash -LiteralPath $loader -Algorithm SHA256).Hash -ne $loaderHash) { 'the loader is not the file this script was written for'; "package= files=0 verdict=FAIL"; exit 1 }

$main = [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'plugin\re_main.c'))
if ($main -notmatch '#define PLUGIN_VERSION "([^"]+)"') { 'no PLUGIN_VERSION in re_main.c'; "package= files=0 verdict=FAIL"; exit 1 }
$version = $Matches[1]
# the built plugin must be the one this source describes
$binary = [Text.Encoding]::ASCII.GetString([IO.File]::ReadAllBytes($asi))
$inBinary = $binary.Contains('Realistic Economy %s attached') -and $binary.Contains($version)
$newest = Get-ChildItem (Join-Path $PSScriptRoot 'plugin') -File | Sort-Object LastWriteTime | Select-Object -Last 1
$stale = $newest.LastWriteTime -gt (Get-Item -LiteralPath $asi).LastWriteTime

$name = "RealisticEconomy-$version"
$out = Join-Path $build "package\$name"
$zip = Join-Path $build "package\$name.zip"
if (Test-Path -LiteralPath $out) { Remove-Item -LiteralPath $out -Recurse -Force }
if (Test-Path -LiteralPath $zip) { Remove-Item -LiteralPath $zip -Force }
$inner = Join-Path $out 'RealisticEconomy'
New-Item -ItemType Directory -Force (Join-Path $inner 'RealisticEconomy_licenses') | Out-Null

Copy-Item -LiteralPath $loader (Join-Path $inner 'version.dll')
Copy-Item -LiteralPath $asi (Join-Path $inner 'RealisticEconomy.asi')
Copy-Item -LiteralPath (Join-Path $dist 'RealisticEconomy.ini') (Join-Path $inner 'RealisticEconomy.ini')
Copy-Item -LiteralPath (Join-Path $dist 'licenses\Ultimate-ASI-Loader-LICENSE.txt') (Join-Path $inner 'RealisticEconomy_licenses\Ultimate-ASI-Loader-LICENSE.txt')
Copy-Item -LiteralPath (Join-Path $dist 'licenses\RealisticEconomy-LICENSE.txt') (Join-Path $inner 'RealisticEconomy_licenses\RealisticEconomy-LICENSE.txt')
# the two descriptions get a byte-order mark and Windows line ends, so that any Notepad shows them right
foreach ($language in 'ko', 'en') {
    $text = [IO.File]::ReadAllText((Join-Path $dist "README_$language.txt")) -replace "`r?`n", "`r`n"
    [IO.File]::WriteAllText((Join-Path $out "RealisticEconomy_README_$language.txt"), $text, $utf8Bom)
}
# the batch files get Windows line ends and no byte-order mark: cmd takes a mark for a part of the first command
foreach ($batch in $batches) {
    $text = [IO.File]::ReadAllText((Join-Path $dist $batch)) -replace "`r?`n", "`r`n"
    [IO.File]::WriteAllText((Join-Path $out $batch), $text, $utf8)
}

$ini = [IO.File]::ReadAllText((Join-Path $inner 'RealisticEconomy.ini'))
if ($Control) { $ini = $ini -replace '(?m)^trace=0', 'trace=1' }
$iniOk = $ini -match '(?m)^trace=0\s*$' -and $ini -match '(?m)^release=0\s*$' -and $ini -match '(?m)^marketShift=0\s*$' -and $ini -notmatch '(?m)^board(Total|AskAll)='

$files = Get-ChildItem -LiteralPath $out -Recurse -File | Sort-Object FullName
$lines = @("Realistic Economy $version for This Grand Life 2 v1.03.22, packaged $(Get-Date -Format 'yyyy-MM-dd HH:mm')", '', 'SHA256 of each file:')
foreach ($file in $files) {
    $lines += "$((Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash)  $($file.FullName.Substring($out.Length + 1).Replace('\', '/'))"
}
[IO.File]::WriteAllText((Join-Path $inner 'RealisticEconomy_SHA256.txt'), ($lines -join "`r`n") + "`r`n", $utf8Bom)

Compress-Archive -Path (Join-Path $out '*') -DestinationPath $zip
$count = @(Get-ChildItem -LiteralPath $out -Recurse -File).Count
$ok = $inBinary -and -not $stale -and $iniOk -and (Test-Path -LiteralPath $zip)
"version=$version version_in_plugin=$inBinary plugin_older_than_source=$stale settings_as_shipped=$iniOk"
Get-ChildItem -LiteralPath $out -Recurse -File | ForEach-Object { "  $($_.FullName.Substring($out.Length + 1))  $($_.Length) bytes" }
"package=$zip files=$count verdict=$(if ($ok) { 'PASS' } else { 'FAIL' })"
exit ([int](-not $ok))
