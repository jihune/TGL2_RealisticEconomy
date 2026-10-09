@echo off
rem Realistic Economy for This Grand Life 2: takes the mod out of the game folder this file is in.
rem Saves, the settings file and the mod's records stay. The text below is UTF-8.
rem Ends with 0 when the mod is out (or was not in) and with 1 when a file could not be removed.
rem The text is in the game's eight languages, picked by the language Windows is set to: Korean, German, Spanish,
rem French, Portuguese, Japanese or Chinese, and English for any other (a test sets RE_LOCALE itself).
rem Every message is one line: "Korean" "English" "German" "Spanish" "French" "Portuguese" "Japanese" "Chinese".
setlocal
if not defined RE_LOCALE for /f "tokens=3" %%a in ('reg query "HKCU\Control Panel\International" /v LocaleName 2^>nul ^| find "LocaleName"') do set "RE_LOCALE=%%a"
set "RE_L=2"
if /i "%RE_LOCALE:~0,2%"=="ko" set "RE_L=1"
if /i "%RE_LOCALE:~0,2%"=="de" set "RE_L=3"
if /i "%RE_LOCALE:~0,2%"=="es" set "RE_L=4"
if /i "%RE_LOCALE:~0,2%"=="fr" set "RE_L=5"
if /i "%RE_LOCALE:~0,2%"=="pt" set "RE_L=6"
if /i "%RE_LOCALE:~0,2%"=="ja" set "RE_L=7"
if /i "%RE_LOCALE:~0,2%"=="zh" set "RE_L=8"
chcp 65001 >nul
cd /d "%~dp0"

call :say "Realistic Economy 모드 삭제" "Realistic Economy mod: uninstall" "Realistic Economy Mod: Deinstallation" "Mod Realistic Economy: desinstalación" "Mod Realistic Economy : désinstallation" "Mod Realistic Economy: desinstalação" "Realistic Economy MOD の削除" "Realistic Economy 模组：卸载"
echo(

if exist "TGL2.exe" goto in_game_folder
call :say "이 폴더에 TGL2.exe 가 없습니다. 게임 폴더에 있는 이 파일을 실행하십시오." "TGL2.exe is not in this folder. Run this file in the game folder." "TGL2.exe ist nicht in diesem Ordner. Starte diese Datei im Spielordner." "TGL2.exe no está en esta carpeta. Ejecuta este archivo en la carpeta del juego." "TGL2.exe n'est pas dans ce dossier. Lancez ce fichier dans le dossier du jeu." "TGL2.exe não está nesta pasta. Execute este arquivo na pasta do jogo." "このフォルダに TGL2.exe がありません。ゲームのフォルダにあるこのファイルを実行してください。" "此文件夹中没有 TGL2.exe。请在游戏文件夹中运行此文件。"
goto failed

:in_game_folder
rem only a version.dll that is this mod's loader is removed
set "RE_OURS="
if not exist "version.dll" goto loader_checked
if not exist "RealisticEconomy\version.dll" goto loader_checked
fc /b "version.dll" "RealisticEconomy\version.dll" >nul 2>&1
if not errorlevel 1 set "RE_OURS=1"

:loader_checked
if exist "RealisticEconomy.asi" goto remove
if defined RE_OURS goto remove
call :say "지금 상태: 모드가 설치돼 있지 않습니다. 바꾼 것이 없습니다." "Now: the mod is not installed. Nothing was changed." "Stand: der Mod ist nicht installiert. Nichts wurde geändert." "Estado: el mod no está instalado. No se cambió nada." "État : le mod n'est pas installé. Rien n'a été modifié." "Estado: o mod não está instalado. Nada foi alterado." "現在の状態: MOD はインストールされていません。何も変更していません。" "当前状态：模组未安装。没有做任何更改。"
goto done

:remove
if exist "RealisticEconomy.asi" del /f /q "RealisticEconomy.asi" >nul 2>&1
if exist "RealisticEconomy.asi" goto delete_failed
if defined RE_OURS del /f /q "version.dll" >nul 2>&1
if defined RE_OURS if exist "version.dll" goto delete_failed
if not defined RE_OURS if exist "version.dll" call :say "version.dll 은 이 모드가 넣은 파일로 확인되지 않아 그대로 두었습니다." "version.dll could not be confirmed as this mod's file and was left in place." "version.dll ließ sich nicht als Datei dieses Mods bestätigen und wurde nicht angerührt." "No se pudo confirmar que version.dll sea de este mod, así que se dejó en su sitio." "version.dll n'a pas pu être reconnu comme un fichier de ce mod et a été laissé en place." "Não foi possível confirmar que version.dll é deste mod, então ele foi deixado no lugar." "version.dll はこの MOD のファイルと確認できなかったため、そのまま残しました。" "无法确认 version.dll 是本模组的文件，所以保留不动。"
call :say "삭제했습니다. 게임은 모드를 넣기 전처럼 동작합니다." "Removed. The game runs as it did before the mod." "Entfernt. Das Spiel läuft wie vor dem Mod." "Eliminado. El juego funciona como antes del mod." "Supprimé. Le jeu fonctionne comme avant le mod." "Removido. O jogo funciona como antes do mod." "削除しました。ゲームは MOD を入れる前と同じように動きます。" "已卸载。游戏会像安装模组之前一样运行。"
call :say "설정과 기록(RealisticEconomy.ini, RealisticEconomy.state, RealisticEconomy.log)은 남겨 두었습니다. 다시 설치하면 이어서 씁니다. 필요 없으면 지워도 됩니다." "The settings and records (RealisticEconomy.ini, RealisticEconomy.state, RealisticEconomy.log) were kept: a later install goes on with them. Delete them if you do not need them." "Einstellungen und Aufzeichnungen (RealisticEconomy.ini, RealisticEconomy.state, RealisticEconomy.log) bleiben erhalten: eine spätere Installation arbeitet damit weiter. Lösche sie, wenn du sie nicht brauchst." "Los ajustes y registros (RealisticEconomy.ini, RealisticEconomy.state, RealisticEconomy.log) se han conservado: una instalación posterior sigue con ellos. Bórralos si no los necesitas." "Les réglages et les enregistrements (RealisticEconomy.ini, RealisticEconomy.state, RealisticEconomy.log) ont été conservés : une installation ultérieure les reprendra. Supprimez-les si vous n'en avez pas besoin." "As configurações e os registros (RealisticEconomy.ini, RealisticEconomy.state, RealisticEconomy.log) foram mantidos: uma instalação futura continua com eles. Apague-os se não precisar." "設定と記録(RealisticEconomy.ini、RealisticEconomy.state、RealisticEconomy.log)は残しました。もう一度インストールすると続きから使います。不要なら削除してかまいません。" "设置和记录（RealisticEconomy.ini、RealisticEconomy.state、RealisticEconomy.log）已保留：以后重新安装时会继续使用。不需要的话可以删除。"
call :say "세이브는 건드리지 않았습니다. 모드를 쓰는 동안 저장한 세이브도 모드 없이 열립니다. 그런 세이브에 남는 것은 설명서의 제거 항목에 있습니다." "The saves were not touched. Saves made with the mod load without it; what stays in them is listed in the description under Uninstall." "Die Spielstände wurden nicht angerührt. Mit dem Mod gespeicherte Spielstände lassen sich ohne ihn laden; was darin bleibt, steht in der englischen Beschreibung unter Uninstall." "Las partidas no se han tocado. Las guardadas con el mod se abren sin él; lo que queda en ellas está en la descripción en inglés, apartado Uninstall." "Les sauvegardes n'ont pas été touchées. Celles faites avec le mod s'ouvrent sans lui ; ce qui y reste est indiqué dans la description en anglais, rubrique Uninstall." "Os jogos salvos não foram tocados. Os salvos com o mod abrem sem ele; o que fica neles está na descrição em inglês, seção Uninstall." "セーブには触れていません。MOD を入れて保存したセーブは MOD なしでも開けます。セーブに残るものは英語の説明書の Uninstall の項にあります。" "存档没有被改动。使用模组时保存的存档在没有模组时也能打开；存档中保留的内容见英文说明的 Uninstall 部分。"
if exist "saves_before_RealisticEconomy_1" call :say "모드를 설치하기 전의 세이브는 saves_before_RealisticEconomy_ 로 시작하는 폴더에 있습니다." "The saves from before the install are in the folders whose names start with saves_before_RealisticEconomy_." "Die Spielstände von vor der Installation liegen in den Ordnern, deren Namen mit saves_before_RealisticEconomy_ beginnen." "Las partidas de antes de la instalación están en las carpetas cuyo nombre empieza por saves_before_RealisticEconomy_." "Les sauvegardes d'avant l'installation sont dans les dossiers dont le nom commence par saves_before_RealisticEconomy_." "Os jogos salvos de antes da instalação estão nas pastas cujo nome começa com saves_before_RealisticEconomy_." "インストール前のセーブは saves_before_RealisticEconomy_ で始まる名前のフォルダにあります。" "安装前的存档在名称以 saves_before_RealisticEconomy_ 开头的文件夹中。"

:done
echo(
pause
exit /b 0

:delete_failed
call :say "모드 파일을 지우지 못했습니다. 게임이 켜져 있으면 끄고 다시 실행하십시오." "The mod's files could not be removed. Close the game if it is running and run this again." "Die Dateien des Mods konnten nicht entfernt werden. Schließe das Spiel, falls es läuft, und starte dies noch einmal." "No se pudieron eliminar los archivos del mod. Cierra el juego si está abierto y ejecuta esto otra vez." "Les fichiers du mod n'ont pas pu être supprimés. Fermez le jeu s'il est lancé et relancez ce fichier." "Não foi possível remover os arquivos do mod. Feche o jogo se ele estiver aberto e execute isto de novo." "MOD のファイルを削除できませんでした。ゲームが起動していたら終了して、もう一度実行してください。" "无法删除模组文件。如果游戏正在运行，请关闭后再运行一次。"
call :say "그래도 안 되면 이 파일을 오른쪽 클릭해 [관리자 권한으로 실행]을 고르십시오." "If that does not help, right-click this file and choose [Run as administrator]." "Hilft das nicht, klicke diese Datei mit der rechten Maustaste an und wähle [Als Administrator ausführen]." "Si eso no ayuda, haz clic derecho en este archivo y elige [Ejecutar como administrador]." "Si cela ne suffit pas, faites un clic droit sur ce fichier et choisissez [Exécuter en tant qu'administrateur]." "Se isso não resolver, clique com o botão direito neste arquivo e escolha [Executar como administrador]." "それでもだめなら、このファイルを右クリックして [管理者として実行] を選んでください。" "如果仍然不行，请右键单击此文件并选择 [以管理员身份运行]。"

:failed
echo(
pause
exit /b 1

:say
if "%RE_L%"=="1" echo(%~1
if "%RE_L%"=="2" echo(%~2
if "%RE_L%"=="3" echo(%~3
if "%RE_L%"=="4" echo(%~4
if "%RE_L%"=="5" echo(%~5
if "%RE_L%"=="6" echo(%~6
if "%RE_L%"=="7" echo(%~7
if "%RE_L%"=="8" echo(%~8
goto :eof
