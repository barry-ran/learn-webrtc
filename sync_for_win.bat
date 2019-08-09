@echo off

:: ��ȡ�ű�����·��
set script_path=%~dp0
:: ����ű�����Ŀ¼,��Ϊ���Ӱ��ű���ִ�еĳ���Ĺ���Ŀ¼
set old_cd=%cd%
cd /d %~dp0

echo=
echo ---------------------------------------------------------------
echo ���depot_tools
echo ---------------------------------------------------------------
set depot_tools_path=%script_path%depot_tools
if exist %depot_tools_path% (
    echo %depot_tools_path% �Ѵ���
) else (
    echo %depot_tools_path% �����ڣ�����depot_tools
    call git clone --depth=1 https://chromium.googlesource.com/chromium/tools/depot_tools.git
)
:: if�Ӿ��ERRORLEVEL�ò�����ȷ��ֵ������git clone����õ������ж�
if not %ERRORLEVEL% == 0 (
    echo depot_tools����ʧ��
    exit 1
)

:: ������������
set PATH=%depot_tools_path%;%PATH%

set GYP_GENERATORS=ninja
set GYP_MSVS_VERSION=2017
set DEPOT_TOOLS_WIN_TOOLCHAIN=0

:: ����gclient sync��Ҫ�⼸����������
:: ����vs·����ͨ��for��call�Ľ�����浽����GYP_MSVS_OVERRIDE_PATH
for /f "tokens=* delims=" %%o in ('call python script/find_vs_path.py') do (
    set GYP_MSVS_OVERRIDE_PATH=%%o
)
echo vs·��=%GYP_MSVS_OVERRIDE_PATH%
if "%GYP_MSVS_OVERRIDE_PATH%" == "" (
    echo δ�ҵ�vs·��    
    exit 1
)

echo=
echo ---------------------------------------------------------------
echo ��ʼ��depot_tools
echo ---------------------------------------------------------------
call gclient

if not %ERRORLEVEL% == 0 (
    echo ��ʼ��depot_toolsʧ��
    exit 1
)

echo=
echo ---------------------------------------------------------------
echo ͬ��webrtc����
echo ---------------------------------------------------------------
set webrtc_path=%script_path%webrtc
set webrtc_src_path=%webrtc_path%/src
if not exist %webrtc_path% (
    md %webrtc_path%
)
cd %webrtc_path%
if not exist %webrtc_src_path% (
    call fetch --nohooks webrtc
)
call gclient sync --force
cd %webrtc_src_path%
:: ���ڵ�ǰ����release��֧72������
call git checkout -b branch-heads/72 remotes/branch-heads/72
call git branch --set-upstream-to=remotes/branch-heads/72 branch-heads/72
call git pull
:: �л���֧�Ժ����sync����ͬ����ͬ��֧��build tools
:: �����ټ�--nohooks�����򲻻�����webrtc\src\buildtools\win\gn.exe�ȱ��빤��
call gclient sync

if not %ERRORLEVEL% == 0 (
    echo webrtcͬ��ʧ��
    exit 1
)

echo=
echo ---------------------------------------------------------------
echo ����webrtc
echo ---------------------------------------------------------------

call python %script_path%script/custom_webrtc.py
if not %ERRORLEVEL% == 0 (
    echo ����webrtcʧ��
    exit 1
)

:: �ָ�����Ŀ¼
cd %old_cd%

echo allͬ���ɹ�