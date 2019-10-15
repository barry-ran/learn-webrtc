@echo off

:: 获取脚本绝对路径
set script_path=%~dp0
:: 进入脚本所在目录,因为这会影响脚本中执行的程序的工作目录
set old_cd=%cd%
cd /d %~dp0

echo=
echo ---------------------------------------------------------------
echo 检查depot_tools
echo ---------------------------------------------------------------
set depot_tools_path=%script_path%depot_tools
if exist %depot_tools_path% (
    echo %depot_tools_path% 已存在
) else (
    echo %depot_tools_path% 不存在，下载depot_tools
    call git clone --depth=1 https://chromium.googlesource.com/chromium/tools/depot_tools.git
)
:: if子句的ERRORLEVEL拿不到正确的值，所以git clone结果拿到这里判断
if not %ERRORLEVEL% == 0 (
    echo depot_tools下载失败
    goto return
)

:: 环境变量设置
set PATH=%depot_tools_path%;%PATH%

set GYP_GENERATORS=ninja
set GYP_MSVS_VERSION=2017
set DEPOT_TOOLS_WIN_TOOLCHAIN=0

:: 查找vcvarsall.bat路径
for /f "tokens=* delims=" %%o in ('call python script/find_vcvarsall_path.py') do (
    set vcvarsall=%%o
)
echo vcvarsall路径=%vcvarsall%
if "%vcvarsall%" == "" (
    echo 未找到vcvarsall路径    
    goto return
)

:: 注册vc环境
set cpu_mode=x86
if /i %cpu_mode% == x86 (
    call "%vcvarsall%" %cpu_mode%
) else (
    call "%vcvarsall%" %cpu_mode%
)

if not %errorlevel%==0 (
    echo "vcvarsall 注册失败"
    goto return
)

:: 后面gclient sync需要这几个环境变量
:: 查找vs路径，通过for将call的结果保存到变量GYP_MSVS_OVERRIDE_PATH
for /f "tokens=* delims=" %%o in ('call python script/find_vs_path.py') do (
    set GYP_MSVS_OVERRIDE_PATH=%%o
)
echo vs路径=%GYP_MSVS_OVERRIDE_PATH%
if "%GYP_MSVS_OVERRIDE_PATH%" == "" (
    echo 未找到vs路径    
    goto return
)

:: fix: No supported Visual Studio can be found
set vs2017_install=%GYP_MSVS_OVERRIDE_PATH%

echo=
echo ---------------------------------------------------------------
echo 初始化depot_tools
echo ---------------------------------------------------------------
call gclient

if not %ERRORLEVEL% == 0 (
    echo 初始化depot_tools失败
    goto return
)

echo=
echo ---------------------------------------------------------------
echo 同步webrtc代码
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

if not %ERRORLEVEL% == 0 (
    echo webrtc同步失败
    goto return
)

cd %webrtc_src_path%
:: webrtc最新release https://webrtc.org/release-notes/
:: 基于当前最新release分支m76来开发
:: m76 === depot_tools 61d3d4b0bd55ee9027a831d27210ddfcbb9531a7
call git checkout -b branch-heads/m76 refs/remotes/branch-heads/m76
:: call git pull
:: 切换分支以后必须sync，来同步不同分支的build tools
:: 不能再加--nohooks，否则不会下载webrtc\src\buildtools\win\gn.exe等编译工具
call gclient sync

if not %ERRORLEVEL% == 0 (
    echo webrtc同步失败
    goto return
)

echo=
echo ---------------------------------------------------------------
echo 定制webrtc
echo ---------------------------------------------------------------

call python %script_path%script/custom_webrtc.py
if not %ERRORLEVEL% == 0 (
    echo 定制webrtc失败
    goto return
)

echo all同步成功

:return
:: 恢复工作目录
cd %old_cd%