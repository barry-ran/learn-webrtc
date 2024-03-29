@echo off

:: 获取脚本绝对路径
set script_path=%~dp0
:: 进入脚本所在目录,因为这会影响脚本中执行的程序的工作目录
set old_cd=%cd%
cd /d %~dp0

:: 启动参数声明
set debug_mode="false"

echo=
echo=
echo ---------------------------------------------------------------
echo 检查编译参数[debug/release]
echo ---------------------------------------------------------------

:: 编译参数检查 /i忽略大小写
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

:: 提示
if /i %debug_mode% == "true" (
    echo 当前编译版本为debug版本
) else (
    echo 当前编译版本为release版本
)

:: 环境变量设置
set depot_tools_path=%script_path%depot_tools
set PATH=%depot_tools_path%;%PATH%

set GYP_MSVS_VERSION=2019
set DEPOT_TOOLS_WIN_TOOLCHAIN=0

:: 设置相关路径
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

:: 进入webrtc目录
cd webrtc\src

echo=
echo=
echo ---------------------------------------------------------------
echo gn生成ninja脚本
echo ---------------------------------------------------------------

:: ninja file
set args=is_debug=%debug_mode%
set args=%args% target_cpu=\"x86\"

:: 开启H264编码支持
set args=%args% proprietary_codecs=true
set args=%args% ffmpeg_branding=\"Chrome\"

set args=%args% use_rtti=true
set args=%args% treat_warnings_as_errors=true
set args=%args% rtc_include_pulse_audio=false
set args=%args% rtc_build_examples=true
set args=%args% rtc_build_tools=false
set args=%args% rtc_enable_protobuf=false
set args=%args% rtc_include_tests=false
set args=%args% enable_libaom=false
set args=%args% enable_google_benchmarks=false
set args=%args% libyuv_include_tests=false
set args=%args% symbol_level=0
set args=%args% strip_debug_info=true

set args=%args% use_lld=false
set args=%args% is_clang=false
:: 分析包体积时使用
::set args=%args% generate_linker_map=true

if /i %debug_mode% == "true" (
    set args=%args% enable_iterator_debugging=true
)
:: 查看支持的编译参数
:: gn args ./args
:: gn args ./args --list
call %gn% gen %dispatch_path% --ide=vs2019 --args="%args%"

if not %errorlevel%==0 (
    echo "generate ninja failed"
    goto return
)

echo=
echo=
echo ---------------------------------------------------------------
echo 开始ninja编译
echo ---------------------------------------------------------------

:: build
:: call %ninja% -C %dispatch_path% examples     编译指定target：examples
:: 默认编译target：default
set NINJA_SUMMARIZE_BUILD=1
call %ninja% -C %dispatch_path%
if not %errorlevel%==0 (
    echo "ninja build failed"  
    goto return
)

echo=
echo=
echo ---------------------------------------------------------------
echo 完成！
echo ---------------------------------------------------------------

:return

:: 恢复工作目录
cd %old_cd%