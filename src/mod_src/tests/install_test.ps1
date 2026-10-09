# Runs the package's two batch files in a made-up game folder and checks what they leave behind. PowerShell 7.
#   pwsh -NoProfile -File mod_src/tests/install_test.ps1 [-Control]
# Run mod_src/build.ps1 and mod_src/package.ps1 first: the files under test are the packaged ones (Windows line ends).
# The game folder is made under mod_src/_build/install_test/ with spaces, brackets, "&", "%", "!" and Korean in its
# name, and the batch files are started the way a double click starts them, from another folder, with no window.
# The last line is `install_test cases=<n> failed=<n> control=<0|1> verdict=PASS|FAIL`.
# -Control runs the same cases with an install file that has lost its test for another mod's version.dll: FAIL.
param([switch]$Control)
$ErrorActionPreference = 'Stop'
$build = Join-Path (Split-Path -Parent $PSScriptRoot) '_build'
$packages = @(Get-ChildItem -LiteralPath (Join-Path $build 'package') -Directory -Filter 'RealisticEconomy-*' -ErrorAction SilentlyContinue)
if ($packages.Count -ne 1) { "expected one package folder under $build\package, found $($packages.Count)"; "install_test cases=0 failed=1 control=$([int][bool]$Control) verdict=FAIL"; exit 1 }
$work = Join-Path $build 'install_test'
if (Test-Path -LiteralPath $work) { Remove-Item -LiteralPath $work -Recurse -Force }
$game = Join-Path $work '게임 (x86) & 100% 폴더!'
$inner = Join-Path $game 'RealisticEconomy'
$saves = Join-Path $game 'saves'
New-Item -ItemType Directory -Force (Join-Path $saves '세이브 하나') | Out-Null
[IO.File]::WriteAllText((Join-Path $saves '세이브 하나\misc.txt'), 'the first save')
[IO.File]::WriteAllText((Join-Path $saves 'Autosave0.txt'), 'another file')
Copy-Item -Path (Join-Path $packages[0].FullName '*') -Destination $game -Recurse

$install = 'RealisticEconomy_install.bat'
$uninstall = 'RealisticEconomy_uninstall.bat'
if ($Control) {
    $path = Join-Path $game $install
    $text = [IO.File]::ReadAllText($path)
    $guard = "fc /b `"version.dll`" `"RealisticEconomy\version.dll`" >nul 2>&1`r`nif not errorlevel 1 goto loader_ok`r`ncall :say"
    $at = $text.IndexOf($guard, [StringComparison]::Ordinal)
    "control: the test for another version.dll found at $at and taken out"
    if ($at -ge 0) { [IO.File]::WriteAllText($path, $text.Remove($at, $guard.Length).Insert($at, "goto loader_ok`r`ncall :say"), [Text.UTF8Encoding]::new($false)) }
}

$script:cases = 0
$script:failed = 0
function Check([string]$what, [bool]$ok) {
    $script:cases++
    if (-not $ok) { $script:failed++ }
    "  $(if ($ok) { 'ok  ' } else { 'FAIL' }) $what"
}
function Invoke-Batch([string]$name, [string]$locale = 'ko-KR') {
    $psi = [Diagnostics.ProcessStartInfo]::new('cmd.exe', "/d /c `"`"$(Join-Path $game $name)`" <nul`"")
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.StandardOutputEncoding = [Text.Encoding]::UTF8
    $psi.WorkingDirectory = $build
    $psi.Environment['RE_LOCALE'] = $locale
    $process = [Diagnostics.Process]::Start($psi)
    $out = $process.StandardOutput.ReadToEnd() + $process.StandardError.ReadToEnd()
    $process.WaitForExit()
    [pscustomobject]@{ Exit = $process.ExitCode; Out = $out }
}
function Get-Tree([string]$folder) {
    if (-not (Test-Path -LiteralPath $folder)) { return 'no such folder' }
    (Get-ChildItem -LiteralPath $folder -Recurse -File | Sort-Object FullName | ForEach-Object { $_.FullName.Substring($folder.Length) + ' ' + (Get-FileHash -LiteralPath $_.FullName).Hash }) -join '|'
}
function Get-Hash([string]$path) { if (Test-Path -LiteralPath $path) { (Get-FileHash -LiteralPath $path).Hash } else { 'no such file' } }
function Test-Same([string]$name) { (Test-Path -LiteralPath (Join-Path $game $name)) -and (Get-Hash (Join-Path $game $name)) -eq (Get-Hash (Join-Path $inner $name)) }
function Test-There([string]$name) { Test-Path -LiteralPath (Join-Path $game $name) }
$savesBefore = Get-Tree $saves

'1. the folder has no TGL2.exe'
$r = Invoke-Batch $install
Check "install refuses (exit $($r.Exit))" ($r.Exit -eq 1 -and $r.Out.Contains('이 폴더에 TGL2.exe 가 없습니다'))
Check 'nothing was put there' (-not (Test-There 'version.dll') -and -not (Test-There 'RealisticEconomy.asi') -and -not (Test-There 'saves_before_RealisticEconomy_1'))
$r = Invoke-Batch $uninstall
Check "uninstall refuses (exit $($r.Exit))" ($r.Exit -eq 1 -and $r.Out.Contains('이 폴더에 TGL2.exe 가 없습니다'))
[IO.File]::WriteAllText((Join-Path $game 'TGL2.exe'), 'not the game')

'2. first install'
$r = Invoke-Batch $install
Check "install succeeds (exit $($r.Exit))" ($r.Exit -eq 0 -and $r.Out.Contains('지금 상태: 모드가 설치돼 있지 않습니다') -and $r.Out.Contains('설치했습니다. 게임을 켜면'))
Check 'the three files are the package''s' ((Test-Same 'version.dll') -and (Test-Same 'RealisticEconomy.asi') -and (Test-Same 'RealisticEconomy.ini'))
Check 'the saves were copied to saves_before_RealisticEconomy_1' ((Get-Tree (Join-Path $game 'saves_before_RealisticEconomy_1')) -eq $savesBefore -and $r.Out.Contains('세이브를 saves_before_RealisticEconomy_1 폴더에 복사해 두었습니다'))
Check 'the saves are as they were' ((Get-Tree $saves) -eq $savesBefore)

'3. install again over changed settings, English text'
[IO.File]::AppendAllText((Join-Path $game 'RealisticEconomy.ini'), "`r`n; the player's own line`r`n")
$r = Invoke-Batch $install 'en-US'
Check "install succeeds (exit $($r.Exit))" ($r.Exit -eq 0 -and $r.Out.Contains('Now: the mod is already installed.') -and $r.Out.Contains('Installed. In the game the version'))
Check 'the settings file is the player''s' ((Test-There 'RealisticEconomy.ini') -and -not (Test-Same 'RealisticEconomy.ini') -and $r.Out.Contains('was left as it was'))
Check 'no second copy of the saves' (-not (Test-There 'saves_before_RealisticEconomy_2'))

'3b. the text follows the language Windows is set to: the game''s six other languages, English for any other'
$installed = [ordered]@{
    'de-DE' = 'Stand: der Mod ist schon installiert.'
    'es-MX' = 'Estado: el mod ya está instalado.'
    'fr-FR' = 'État : le mod est déjà installé.'
    'pt-BR' = 'Estado: o mod já está instalado.'
    'ja-JP' = '現在の状態: MOD はすでにインストールされています。'
    'zh-CN' = '当前状态：模组已经安装。'
    'it-IT' = 'Now: the mod is already installed.'
}
foreach ($locale in $installed.Keys) {
    $r = Invoke-Batch $install $locale
    Check "install with $locale says so in its language (exit $($r.Exit))" ($r.Exit -eq 0 -and $r.Out.Contains($installed[$locale]) -and ($locale -eq 'it-IT' -or -not $r.Out.Contains('Now: the mod')))
}

'4. a file is in use (the game is running)'
[IO.File]::WriteAllText((Join-Path $game 'RealisticEconomy.state'), 'records')
[IO.File]::WriteAllText((Join-Path $game 'RealisticEconomy.log'), 'log')
$held = [IO.File]::Open((Join-Path $game 'RealisticEconomy.asi'), 'Open', 'Read', 'Read')
$r = Invoke-Batch $uninstall
$held.Dispose()
Check "uninstall fails (exit $($r.Exit))" ($r.Exit -eq 1 -and $r.Out.Contains('모드 파일을 지우지 못했습니다'))
Check 'both files are still there' ((Test-Same 'version.dll') -and (Test-Same 'RealisticEconomy.asi'))
$held = [IO.File]::Open((Join-Path $game 'version.dll'), 'Open', 'Read', 'Read')
$r = Invoke-Batch $install
$held.Dispose()
Check "install fails (exit $($r.Exit))" ($r.Exit -eq 1 -and $r.Out.Contains('파일을 쓰지 못했습니다'))

'5. uninstall'
$r = Invoke-Batch $uninstall
Check "uninstall succeeds (exit $($r.Exit))" ($r.Exit -eq 0 -and $r.Out.Contains('삭제했습니다. 게임은 모드를 넣기 전처럼 동작합니다') -and $r.Out.Contains('saves_before_RealisticEconomy_ 로 시작하는 폴더'))
Check 'the loader and the plugin are gone' (-not (Test-There 'version.dll') -and -not (Test-There 'RealisticEconomy.asi'))
Check 'settings and records stay' ((Test-There 'RealisticEconomy.ini') -and (Test-There 'RealisticEconomy.state') -and (Test-There 'RealisticEconomy.log'))
Check 'the saves and their copy are as they were' ((Get-Tree $saves) -eq $savesBefore -and (Get-Tree (Join-Path $game 'saves_before_RealisticEconomy_1')) -eq $savesBefore)
$r = Invoke-Batch $uninstall 'en-US'
Check "uninstall a second time changes nothing (exit $($r.Exit))" ($r.Exit -eq 0 -and $r.Out.Contains('Now: the mod is not installed. Nothing was changed.'))
$notInstalled = [ordered]@{
    'de-AT' = 'Stand: der Mod ist nicht installiert. Nichts wurde geändert.'
    'es-ES' = 'Estado: el mod no está instalado. No se cambió nada.'
    'fr-CA' = 'État : le mod n''est pas installé. Rien n''a été modifié.'
    'pt-PT' = 'Estado: o mod não está instalado. Nada foi alterado.'
    'ja-JP' = '現在の状態: MOD はインストールされていません。何も変更していません。'
    'zh-TW' = '当前状态：模组未安装。没有做任何更改。'
}
foreach ($locale in $notInstalled.Keys) {
    $r = Invoke-Batch $uninstall $locale
    Check "uninstall with $locale says so in its language (exit $($r.Exit))" ($r.Exit -eq 0 -and $r.Out.Contains($notInstalled[$locale]))
}

'6. install after an uninstall'
[IO.File]::WriteAllText((Join-Path $saves 'Autosave1.txt'), 'saved without the mod')
$savesLater = Get-Tree $saves
$r = Invoke-Batch $install
Check "install succeeds (exit $($r.Exit))" ($r.Exit -eq 0 -and (Test-Same 'version.dll') -and (Test-Same 'RealisticEconomy.asi'))
Check 'a new copy of the saves, the first one untouched' ((Get-Tree (Join-Path $game 'saves_before_RealisticEconomy_2')) -eq $savesLater -and (Get-Tree (Join-Path $game 'saves_before_RealisticEconomy_1')) -eq $savesBefore)
$r = Invoke-Batch $uninstall

'7. another mod''s version.dll'
$other = Join-Path $game 'version.dll'
[IO.File]::WriteAllText($other, 'the loader of another mod')
$otherHash = Get-Hash $other
$r = Invoke-Batch $install
Check "install refuses (exit $($r.Exit))" ($r.Exit -eq 1 -and $r.Out.Contains('게임 폴더에 다른 version.dll 이 이미 있습니다'))
Check 'that file is as it was' ((Get-Hash $other) -eq $otherHash)
Check 'no plugin and no third copy of the saves' (-not (Test-There 'RealisticEconomy.asi') -and -not (Test-There 'saves_before_RealisticEconomy_3'))
Copy-Item -LiteralPath (Join-Path $inner 'RealisticEconomy.asi') (Join-Path $game 'RealisticEconomy.asi')
$r = Invoke-Batch $uninstall
Check "uninstall takes the plugin out only (exit $($r.Exit))" ($r.Exit -eq 0 -and -not (Test-There 'RealisticEconomy.asi') -and (Get-Hash $other) -eq $otherHash -and $r.Out.Contains('확인되지 않아 그대로 두었습니다'))
if (Test-Path -LiteralPath $other) { Remove-Item -LiteralPath $other }

'8. the package was not extracted whole'
Remove-Item -LiteralPath (Join-Path $inner 'RealisticEconomy.asi')
$r = Invoke-Batch $install
Check "install refuses (exit $($r.Exit))" ($r.Exit -eq 1 -and $r.Out.Contains('RealisticEconomy 폴더에 모드 파일이 없습니다') -and -not (Test-There 'version.dll'))

$ok = $script:failed -eq 0
"install_test cases=$($script:cases) failed=$($script:failed) control=$([int][bool]$Control) verdict=$(if ($ok) { 'PASS' } else { 'FAIL' })"
exit ([int](-not $ok))
