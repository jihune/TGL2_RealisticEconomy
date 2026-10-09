# Puts what goes to GitHub into one folder. PowerShell 7. Opens no window. Uploads nothing.
#   pwsh -NoProfile -File mod_src/github.ps1 [-Control]
# Run mod_src/build.ps1 and mod_src/package.ps1 first. Output under mod_src/_build/github/:
#   repo/      the repository's content: README.md and one README a language, LICENSE, docs/ (the two full
#              descriptions), src/ (the plugin's source with its tests, the files of the package, the build scripts)
#   release/   the zip to attach to the release, and a text file with its SHA256
# No file of the game and no save goes in: only the sources named below are copied.
# `repo` may be the clone that is pushed: its .git stays as it is and everything else in it is made anew, so that
# `git -C mod_src/_build/github/repo status` shows what a new version changes.
# The last line is `github=<folder> readmes=<n> source_files=<n> zip=<name> leaks=<n> verdict=PASS|FAIL`.
# -Control puts a file with a home path into the copy, which must give FAIL; the folder it leaves is not for upload.
param([switch]$Control)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$build = Join-Path $PSScriptRoot '_build'
$out = Join-Path $build 'github'
$repo = Join-Path $out 'repo'
$release = Join-Path $out 'release'
$utf8 = [Text.UTF8Encoding]::new($false)
$languages = 'ko', 'de', 'es', 'fr', 'pt-BR', 'ja', 'zh-CN' # README.md is the English one

$main = [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'plugin\re_main.c'))
if ($main -notmatch '#define PLUGIN_VERSION "([^"]+)"') { 'no PLUGIN_VERSION in re_main.c'; 'github= readmes=0 source_files=0 zip= leaks=0 verdict=FAIL'; exit 1 }
$version = $Matches[1]
$zip = Join-Path $build "package\RealisticEconomy-$version.zip"
if (-not (Test-Path -LiteralPath $zip)) { "missing: $zip (run mod_src/package.ps1)"; 'github= readmes=0 source_files=0 zip= leaks=0 verdict=FAIL'; exit 1 }

if (Test-Path -LiteralPath $release) { Remove-Item -LiteralPath $release -Recurse -Force }
if (Test-Path -LiteralPath $repo) { Get-ChildItem -LiteralPath $repo -Force | Where-Object { $_.Name -ne '.git' } | Remove-Item -Recurse -Force }
New-Item -ItemType Directory -Force (Join-Path $repo 'docs'), (Join-Path $repo 'src\analysis\scripts'), $release | Out-Null
function Get-Copied([string]$folder) { Get-ChildItem -LiteralPath $folder -Recurse -File -Force | Where-Object { $_.FullName -notmatch '\\\.git\\' } }

# the READMEs: each names this version and links to every other one
$names = @('README.md') + ($languages | ForEach-Object { "README.$_.md" })
$readmesOk = $true
foreach ($name in $names) {
    $source = Join-Path $PSScriptRoot "github\$name"
    if (-not (Test-Path -LiteralPath $source)) { "missing: $source"; $readmesOk = $false; continue }
    $text = [IO.File]::ReadAllText($source)
    $others = @($names | Where-Object { $_ -ne $name -and -not $text.Contains("($_)") })
    if (-not $text.Contains($version)) { "$name does not name version $version"; $readmesOk = $false }
    if ($others.Count) { "$name has no link to: $($others -join ', ')"; $readmesOk = $false }
    Copy-Item -LiteralPath $source (Join-Path $repo $name)
}

Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'dist\licenses\RealisticEconomy-LICENSE.txt') (Join-Path $repo 'LICENSE')
[IO.File]::WriteAllText((Join-Path $repo '.gitattributes'), "# Files are stored byte for byte: no end-of-line conversion on any path.`n* -text`n", $utf8)
foreach ($language in 'ko', 'en') {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot "dist\README_$language.txt") (Join-Path $repo "docs\RealisticEconomy_README_$language.txt")
}

# the source keeps its place under the folder the scripts call the root, so that their paths hold
$src = Join-Path $repo 'src'
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'github\src_README.md') (Join-Path $src 'README.md')
foreach ($folder in 'plugin', 'tests', 'dist') {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot $folder) (Join-Path $src "mod_src\$folder") -Recurse
}
foreach ($script in 'build.ps1', 'package.ps1', 'github.ps1') {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot $script) (Join-Path $src "mod_src\$script")
}
New-Item -ItemType Directory -Force (Join-Path $src 'mod_src\github') | Out-Null
Copy-Item -Path (Join-Path $PSScriptRoot 'github\*') -Destination (Join-Path $src 'mod_src\github')
Copy-Item -LiteralPath (Join-Path $root 'analysis\scripts\lang_placeholders.py') (Join-Path $src 'analysis\scripts\lang_placeholders.py')

Copy-Item -LiteralPath $zip (Join-Path $release (Split-Path -Leaf $zip))
$hash = (Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash
[IO.File]::WriteAllText((Join-Path $release "RealisticEconomy-$version.zip.sha256.txt"), "$hash  RealisticEconomy-$version.zip`r`n", $utf8)

if ($Control) { [IO.File]::WriteAllText((Join-Path $src 'control.txt'), ('C:' + '\Users\somebody\Desktop'), $utf8) }

# nothing that names this machine or its user: a drive path of a home or work folder, an e-mail address, the name
# of the Windows account and of the git user (asked here, written nowhere)
$leaks = 0
$patterns = @('[A-Za-z]:\\Users\\', '[A-Za-z]:\\_dev', '[A-Za-z]:\\Games\\', ('Steam' + 'Library'), '[A-Za-z0-9._-]+@(naver|gmail|outlook|hotmail)\.')
$gitUser = (& git -C $root config user.name 2>$null)
foreach ($person in $env:USERNAME, $gitUser) {
    if ($person -and $person.Length -ge 4) { $patterns += '(?i)\b' + [regex]::Escape($person) }
}
foreach ($file in Get-Copied $repo) {
    $text = [IO.File]::ReadAllText($file.FullName)
    foreach ($pattern in $patterns) {
        if ($text -match $pattern) { $leaks++; "leak: $($file.FullName.Substring($repo.Length + 1)) has '$($Matches[0])'" }
    }
}
$gameFiles = @(Get-Copied $out | Where-Object { $_.Name -match '^(TGL2\.exe|.*\.sav|misc\.txt)$' -or $_.Extension -in '.png', '.dll', '.exe', '.asi' })

$sourceFiles = @(Get-Copied $src).Count
$readmes = @(Get-ChildItem -LiteralPath $repo -File -Filter 'README*.md').Count
$ok = $readmesOk -and $readmes -eq $names.Count -and $leaks -eq 0 -and $gameFiles.Count -eq 0 -and $sourceFiles -gt 0
if ($gameFiles.Count) { $gameFiles | ForEach-Object { "not for upload: $($_.FullName.Substring($out.Length + 1))" } }
"version=$version readmes_complete=$readmesOk binaries_or_game_files=$($gameFiles.Count)"
"github=$out readmes=$readmes source_files=$sourceFiles zip=$(Split-Path -Leaf $zip) leaks=$leaks verdict=$(if ($ok) { 'PASS' } else { 'FAIL' })"
exit ([int](-not $ok))
