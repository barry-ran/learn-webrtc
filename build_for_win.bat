@echo off

:: ��ȡ�ű�����·��
set script_path=%~dp0
:: ����ű�����Ŀ¼,��Ϊ���Ӱ��ű���ִ�еĳ���Ĺ���Ŀ¼
set old_cd=%cd%
cd /d %~dp0

:: ������������
set debug_mode="false"

echo=
echo=
echo ---------------------------------------------------------------
echo ���������[debug/release]
echo ---------------------------------------------------------------

:: ���������� /i���Դ�Сд
if /i "%1"=="debug" (
    set debug_mode="true"
    goto param_ok
)
if /i "%1"=="release" (
    set debug_mode="false"
    goto param_ok
)

echo "waring: unkonow build mode -- %1, default debug"
set debug_mode="true"
goto param_ok

:param_ok

:: ��ʾ
if /i %debug_mode% == "true" (
    echo ��ǰ����汾Ϊdebug�汾
) else (
    echo ��ǰ����汾Ϊrelease�汾
)

:: ������������
set depot_tools_path=%script_path%depot_tools
set PATH=%depot_tools_path%;%PATH%

set GYP_MSVS_VERSION=2019
set DEPOT_TOOLS_WIN_TOOLCHAIN=0

:: �������·��
:: set gn=%script_path%buildtools\win\gn.exe
:: set ninja=%script_path%buildtools\win\ninja.exe
set gn=gn
set ninja=autoninja
set dispatch_path=%script_path%out

if /i %debug_mode% == "true" (
    set dispatch_path=%script_path%out\debug
) else (
    set dispatch_path=%script_path%out\release
)

:: ����webrtcĿ¼
cd webrtc\src

echo=
echo=
echo ---------------------------------------------------------------
echo gn����ninja�ű�
echo ---------------------------------------------------------------

:: ninja file
set args=is_debug=%debug_mode%
set args=%args% target_cpu=\"x86\"

:: ����H264����֧��
set args=%args% proprietary_codecs=true

:: m98 ������������
:: set args=%args% use_lld=false
:: set args=%args% is_clang=false
set args=%args% use_rtti=false
set args=%args% rtc_build_examples=true
set args=%args% rtc_build_tools=false
set args=%args% rtc_enable_protobuf=false
set args=%args% rtc_include_tests=false

if /i %debug_mode% == "true" (
    set args=%args% enable_iterator_debugging=true
)
:: �鿴֧�ֵı������
:: gn gen out/temp
:: gn args out/temp --list
call %gn% gen %dispatch_path% --ide=vs2019 --args="%args%"

if not %errorlevel%==0 (
    echo "generate ninja failed"
    goto return
)

echo=
echo=
echo ---------------------------------------------------------------
echo ��ʼninja����
echo ---------------------------------------------------------------

:: build
:: call %ninja% -C %dispatch_path% examples     ����ָ��target��examples
:: Ĭ�ϱ���target��default
set NINJA_SUMMARIZE_BUILD=1
call %ninja% -C %dispatch_path%
if not %errorlevel%==0 (
    echo "ninja build failed"  
    goto return
)

echo=
echo=
echo ---------------------------------------------------------------
echo ��ɣ�
echo ---------------------------------------------------------------

:return

:: �ָ�����Ŀ¼
cd %old_cd%