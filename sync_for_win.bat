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

:: https://chromium.googlesource.com/chromium/src/+/master/docs/windows_build_instructions.md
set GYP_MSVS_VERSION=2019
:: 使用本地vs
set DEPOT_TOOLS_WIN_TOOLCHAIN=0

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
call gclient sync --force --with_branch_heads --with_tags -D

if not %ERRORLEVEL% == 0 (
    echo webrtc同步失败
    goto return
)

cd %webrtc_src_path%
:: webrtc最新release https://chromiumdash.appspot.com/branches
call git checkout refs/remotes/branch-heads/4472

:: 切换分支以后必须sync，来同步不同分支的build tools
:: 不能再加--nohooks，否则不会下载webrtc\src\buildtools\win\gn.exe等编译工具
call gclient sync --with_branch_heads --with_tags -D

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