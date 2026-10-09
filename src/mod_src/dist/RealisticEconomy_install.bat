@echo off
rem Realistic Economy for This Grand Life 2: puts the mod into the game folder this file is in.
rem The text below is UTF-8. Paths are relative and "!" is not expanded, so any folder name works.
rem Ends with 0 when the mod is installed and with 1 when nothing was installed.
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

call :say "Realistic Economy 모드 설치" "Realistic Economy mod: install" "Realistic Economy Mod: Installation" "Mod Realistic Economy: instalación" "Mod Realistic Economy : installation" "Mod Realistic Economy: instalação" "Realistic Economy MOD のインストール" "Realistic Economy 模组：安装"
echo(

if exist "TGL2.exe" goto in_game_folder
call :say "이 폴더에 TGL2.exe 가 없습니다. 압축을 게임 폴더(TGL2.exe 가 있는 폴더)에 푼 뒤 거기서 이 파일을 실행하십시오." "TGL2.exe is not in this folder. Extract the zip into the game folder (the one with TGL2.exe) and run this file there." "TGL2.exe ist nicht in diesem Ordner. Entpacke die Zip-Datei in den Spielordner (dort liegt TGL2.exe) und starte diese Datei dort." "TGL2.exe no está en esta carpeta. Extrae el zip en la carpeta del juego (la que contiene TGL2.exe) y ejecuta este archivo allí." "TGL2.exe n'est pas dans ce dossier. Extrayez le zip dans le dossier du jeu (celui qui contient TGL2.exe) et lancez ce fichier là-bas." "TGL2.exe não está nesta pasta. Extraia o zip na pasta do jogo (a que contém TGL2.exe) e execute este arquivo lá." "このフォルダに TGL2.exe がありません。zip をゲームのフォルダ(TGL2.exe があるフォルダ)に展開し、そこでこのファイルを実行してください。" "此文件夹中没有 TGL2.exe。请把 zip 解压到游戏文件夹（TGL2.exe 所在的文件夹），然后在那里运行此文件。"
goto failed

:in_game_folder
if not exist "RealisticEconomy\version.dll" goto no_files
if not exist "RealisticEconomy\RealisticEconomy.asi" goto no_files
if not exist "RealisticEconomy\RealisticEconomy.ini" goto no_files

rem a version.dll that is not this mod's loader belongs to something else
if not exist "version.dll" goto loader_ok
fc /b "version.dll" "RealisticEconomy\version.dll" >nul 2>&1
if not errorlevel 1 goto loader_ok
call :say "게임 폴더에 다른 version.dll 이 이미 있습니다. 다른 모드의 파일일 수 있어 덮어쓰지 않았습니다. 바꾼 것이 없습니다." "The game folder already has a different version.dll. It may belong to another mod, so it was not overwritten. Nothing was changed." "Im Spielordner liegt schon eine andere version.dll. Sie gehört vielleicht zu einem anderen Mod und wurde deshalb nicht überschrieben. Nichts wurde geändert." "La carpeta del juego ya tiene otro version.dll. Puede ser de otro mod, así que no se sobrescribió. No se cambió nada." "Le dossier du jeu contient déjà un autre version.dll. Il appartient peut-être à un autre mod et n'a donc pas été écrasé. Rien n'a été modifié." "A pasta do jogo já tem outro version.dll. Ele pode ser de outro mod, por isso não foi substituído. Nada foi alterado." "ゲームのフォルダに別の version.dll がすでにあります。ほかの MOD のファイルかもしれないので上書きしませんでした。何も変更していません。" "游戏文件夹中已经有另一个 version.dll。它可能属于其他模组，所以没有覆盖。没有做任何更改。"
goto failed

:loader_ok
if exist "RealisticEconomy.asi" goto was_installed
call :say "지금 상태: 모드가 설치돼 있지 않습니다." "Now: the mod is not installed." "Stand: der Mod ist nicht installiert." "Estado: el mod no está instalado." "État : le mod n'est pas installé." "Estado: o mod não está instalado." "現在の状態: MOD はインストールされていません。" "当前状态：模组未安装。"
if not exist "saves" goto copy_files
set "RE_N=0"
:next_backup
set /a RE_N+=1
if exist "saves_before_RealisticEconomy_%RE_N%" goto next_backup
robocopy "saves" "saves_before_RealisticEconomy_%RE_N%" /e /r:1 /w:1 /njh /njs /ndl /nfl /np >nul 2>&1
if errorlevel 8 goto write_failed
call :say "세이브를 saves_before_RealisticEconomy_%RE_N% 폴더에 복사해 두었습니다." "The saves were copied to the folder saves_before_RealisticEconomy_%RE_N%." "Die Spielstände wurden in den Ordner saves_before_RealisticEconomy_%RE_N% kopiert." "Las partidas se copiaron a la carpeta saves_before_RealisticEconomy_%RE_N%." "Les sauvegardes ont été copiées dans le dossier saves_before_RealisticEconomy_%RE_N%." "Os jogos salvos foram copiados para a pasta saves_before_RealisticEconomy_%RE_N%." "セーブを saves_before_RealisticEconomy_%RE_N% フォルダにコピーしました。" "存档已复制到 saves_before_RealisticEconomy_%RE_N% 文件夹。"
goto copy_files

:was_installed
call :say "지금 상태: 모드가 이미 설치돼 있습니다. 모드 파일을 이 묶음의 것으로 바꿉니다." "Now: the mod is already installed. Its files are replaced with the ones of this package." "Stand: der Mod ist schon installiert. Seine Dateien werden durch die dieses Pakets ersetzt." "Estado: el mod ya está instalado. Sus archivos se sustituyen por los de este paquete." "État : le mod est déjà installé. Ses fichiers sont remplacés par ceux de ce paquet." "Estado: o mod já está instalado. Os arquivos dele são trocados pelos deste pacote." "現在の状態: MOD はすでにインストールされています。MOD のファイルをこのパッケージのものに置き換えます。" "当前状态：模组已经安装。它的文件将替换为此压缩包中的文件。"

:copy_files
copy /y /b "RealisticEconomy\version.dll" "version.dll" >nul 2>&1
if errorlevel 1 goto write_failed
copy /y /b "RealisticEconomy\RealisticEconomy.asi" "RealisticEconomy.asi" >nul 2>&1
if errorlevel 1 goto write_failed
if exist "RealisticEconomy.ini" goto keep_settings
copy /y /b "RealisticEconomy\RealisticEconomy.ini" "RealisticEconomy.ini" >nul 2>&1
if errorlevel 1 goto write_failed
goto verify

:keep_settings
call :say "설정 파일 RealisticEconomy.ini 는 쓰던 것을 그대로 두었습니다." "The settings file RealisticEconomy.ini was left as it was." "Die Einstellungsdatei RealisticEconomy.ini wurde gelassen, wie sie war." "El archivo de ajustes RealisticEconomy.ini se dejó como estaba." "Le fichier de réglages RealisticEconomy.ini a été laissé tel quel." "O arquivo de configurações RealisticEconomy.ini foi deixado como estava." "設定ファイル RealisticEconomy.ini はそのまま残しました。" "设置文件 RealisticEconomy.ini 保持原样。"

:verify
fc /b "version.dll" "RealisticEconomy\version.dll" >nul 2>&1
if errorlevel 1 goto write_failed
fc /b "RealisticEconomy.asi" "RealisticEconomy\RealisticEconomy.asi" >nul 2>&1
if errorlevel 1 goto write_failed
echo(
call :say "설치했습니다. 게임을 켜면 메인 메뉴 오른쪽 아래의 버전 글자 뒤에 + Realistic Economy 가 보입니다." "Installed. In the game the version in the lower right corner of the main menu is followed by + Realistic Economy." "Installiert. Im Spiel steht hinter der Version unten rechts im Hauptmenü + Realistic Economy." "Instalado. En el juego, la versión de la esquina inferior derecha del menú principal va seguida de + Realistic Economy." "Installé. Dans le jeu, la version en bas à droite du menu principal est suivie de + Realistic Economy." "Instalado. No jogo, a versão no canto inferior direito do menu principal é seguida de + Realistic Economy." "インストールしました。ゲームのメインメニュー右下のバージョンの後ろに + Realistic Economy と表示されます。" "已安装。在游戏中，主菜单右下角的版本号后面会显示 + Realistic Economy。"
call :say "설치 전으로 되돌리려면 RealisticEconomy_uninstall.bat 을 실행하십시오." "To go back, run RealisticEconomy_uninstall.bat." "Zum Rückgängigmachen starte RealisticEconomy_uninstall.bat." "Para volver atrás, ejecuta RealisticEconomy_uninstall.bat." "Pour revenir en arrière, lancez RealisticEconomy_uninstall.bat." "Para voltar atrás, execute RealisticEconomy_uninstall.bat." "元に戻すには RealisticEconomy_uninstall.bat を実行してください。" "要恢复原状，请运行 RealisticEconomy_uninstall.bat。"
echo(
pause
exit /b 0

:no_files
call :say "RealisticEconomy 폴더에 모드 파일이 없습니다. 압축을 통째로 다시 푸십시오." "The mod's files are not in the RealisticEconomy folder. Extract the whole zip again." "Die Dateien des Mods fehlen im Ordner RealisticEconomy. Entpacke die ganze Zip-Datei noch einmal." "Los archivos del mod no están en la carpeta RealisticEconomy. Extrae de nuevo el zip entero." "Les fichiers du mod ne sont pas dans le dossier RealisticEconomy. Extrayez de nouveau le zip en entier." "Os arquivos do mod não estão na pasta RealisticEconomy. Extraia o zip inteiro de novo." "RealisticEconomy フォルダに MOD のファイルがありません。zip をもう一度まるごと展開してください。" "RealisticEconomy 文件夹中没有模组文件。请重新完整解压 zip。"
goto failed

:write_failed
call :say "파일을 쓰지 못했습니다. 게임이 켜져 있으면 끄고 다시 실행하십시오." "The files could not be written. Close the game if it is running and run this again." "Die Dateien konnten nicht geschrieben werden. Schließe das Spiel, falls es läuft, und starte dies noch einmal." "No se pudieron escribir los archivos. Cierra el juego si está abierto y ejecuta esto otra vez." "Les fichiers n'ont pas pu être écrits. Fermez le jeu s'il est lancé et relancez ce fichier." "Não foi possível gravar os arquivos. Feche o jogo se ele estiver aberto e execute isto de novo." "ファイルを書き込めませんでした。ゲームが起動していたら終了して、もう一度実行してください。" "无法写入文件。如果游戏正在运行，请关闭后再运行一次。"
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
