# Builds the plugin and its tests, then runs each test and its control. PowerShell 7. Opens no window.
#   pwsh -NoProfile -File mod_src/build.ps1
# Output: mod_src/_build/RealisticEconomy.asi, mod_src/_build/engine_test.exe, mod_src/_build/credit_test.exe,
#         mod_src/_build/TestWindow.asi, mod_src/_build/dump_texts.exe
# The third check, analysis/scripts/ticker_fit.py, needs Python with Pillow and the game's fonts/ folder; the fourth,
# analysis/scripts/tip_fit.py, Python alone.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$gccDir = Join-Path $root 'analysis\_tools\w64devkit\bin'
$out = Join-Path $PSScriptRoot '_build'
New-Item -ItemType Directory -Force $out | Out-Null
$env:PATH = $gccDir + ';' + $env:PATH
$flags = @('-O2', '-Wall', '-Wextra', '-Werror', '-static-libgcc')
function Src([string[]]$names) { $names | ForEach-Object { Join-Path $PSScriptRoot "plugin\$_" } }
$plugin = Src 're_main.c', 're_game.c', 're_credit.c', 're_text.c', 're_trace.c', 're_loan.c', 're_wording.c', 're_guard.c', 're_ipo.c', 're_board.c', 're_stock.c', 're_casino.c', 're_business.c', 're_futures.c', 're_memory.c', 're_lang.c', 're_xp.c', 're_font.c', 're_data.c', 're_util.c', 're_hook.c', 're_sites.c', 're_getter.c'
$engine = Src 're_hook.c', 're_sites.c', 're_getter.c', 're_trace.c', 're_text.c', 're_loan.c', 're_wording.c', 're_guard.c', 're_ipo.c', 're_board.c', 're_stock.c', 're_casino.c', 're_business.c', 're_futures.c', 're_memory.c', 're_lang.c', 're_xp.c', 're_font.c', 're_data.c'

& gcc @flags -shared -o (Join-Path $out 'RealisticEconomy.asi') @plugin
if ($LASTEXITCODE -ne 0) { 'build plugin: FAIL'; exit 2 }
& gcc @flags -o (Join-Path $out 'engine_test.exe') (Join-Path $PSScriptRoot 'tests\engine_test.c') @engine
if ($LASTEXITCODE -ne 0) { 'build engine_test: FAIL'; exit 2 }
& gcc @flags -o (Join-Path $out 'credit_test.exe') (Join-Path $PSScriptRoot 'tests\credit_test.c') (Src 're_credit.c')
if ($LASTEXITCODE -ne 0) { 'build credit_test: FAIL'; exit 2 }
# the fixture that places the game's window in a test run (mod_src/tools/game_run.ps1); it is in no package
& gcc @flags -shared -o (Join-Path $out 'TestWindow.asi') (Join-Path $PSScriptRoot 'tests\test_window.c')
if ($LASTEXITCODE -ne 0) { 'build TestWindow.asi: FAIL'; exit 2 }
# the plugin's texts written out, for the check of the ticker lines below
& gcc @flags -o (Join-Path $out 'dump_texts.exe') (Join-Path $PSScriptRoot 'tests\dump_texts.c') (Src 're_lang.c')
if ($LASTEXITCODE -ne 0) { 'build dump_texts: FAIL'; exit 2 }

$exe = Join-Path $root 'TGL2.exe'
$test = Join-Path $out 'engine_test.exe'
$run = & $test $exe 2>&1
$runCode = $LASTEXITCODE
$run | ForEach-Object { "  $_" }
$control = & $test $exe --control 2>&1
$controlCode = $LASTEXITCODE
"  control: $($control | Select-Object -Last 1)"

$credit = Join-Path $out 'credit_test.exe'
$creditRun = & $credit 2>&1
$creditCode = $LASTEXITCODE
$creditRun | ForEach-Object { "  $_" }
$creditControl = & $credit --control 2>&1
$creditControlCode = $LASTEXITCODE
"  control: $($creditControl | Select-Object -Last 1)"

# the ticker lines fit the ticker's band, in the eight languages (REQUEST.md [60])
$fit = Join-Path $root 'analysis\scripts\ticker_fit.py'
$fitRun = & python $fit 2>&1
$fitCode = $LASTEXITCODE
$fitRun | ForEach-Object { "  $_" }
$fitControl = & python $fit --control 2>&1
$fitControlCode = $LASTEXITCODE
"  control: $($fitControl | Select-Object -Last 1)"

# the mod's part of a hover text keeps to its lines, in the eight languages (DECISIONS.md D-120)
$tip = Join-Path $root 'analysis\scripts\tip_fit.py'
$tipRun = & python $tip 2>&1
$tipCode = $LASTEXITCODE
$tipRun | ForEach-Object { "  $_" }
$tipControl = & python $tip --control 2>&1
$tipControlCode = $LASTEXITCODE
"  control: $($tipControl | Select-Object -Last 1)"

$pass = $runCode -eq 0 -and $controlCode -ne 0 -and $creditCode -eq 0 -and $creditControlCode -ne 0 -and $fitCode -eq 0 -and $fitControlCode -ne 0 -and $tipCode -eq 0 -and $tipControlCode -ne 0
$verdict = if ($pass) { 'PASS' } else { 'FAIL' }
if ($pass) {
    # the working copy keeps the mod in (REQUEST.md [36]): the loader, this build, and the settings when none are there
    $root = Split-Path -Parent $PSScriptRoot
    try {
        Copy-Item -LiteralPath (Join-Path $root 'analysis\_tools\asi_loader_v9.7.4\dinput8.dll') (Join-Path $root 'version.dll') -Force
        Copy-Item -LiteralPath (Join-Path $out 'RealisticEconomy.asi') (Join-Path $root 'RealisticEconomy.asi') -Force
        if (-not (Test-Path -LiteralPath (Join-Path $root 'RealisticEconomy.ini'))) { Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'dist\RealisticEconomy.ini') (Join-Path $root 'RealisticEconomy.ini') }
        '  the working copy has this build: version.dll, RealisticEconomy.asi, RealisticEconomy.ini'
    } catch { "  the working copy was NOT given this build (is the game running?): $($_.Exception.Message)" }
}
"engine_test_exit=$runCode control_exit=$controlCode credit_test_exit=$creditCode credit_control_exit=$creditControlCode ticker_fit_exit=$fitCode ticker_control_exit=$fitControlCode tip_fit_exit=$tipCode tip_control_exit=$tipControlCode (controls must be non-zero) verdict=$verdict"
exit ([int]($verdict -ne 'PASS'))
