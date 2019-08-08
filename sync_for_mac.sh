# 获取绝对路径，保证其他目录执行此脚本依然正确
{
cd $(dirname "$0")
script_path=$(pwd)
cd -
} &> /dev/null # disable output

# 设置当前目录，cd的目录影响接下来执行程序的工作目录
old_cd=$(pwd)
cd $(dirname "$0")

echo
echo ---------------------------------------------------------------
echo 检查depot_tools
echo ---------------------------------------------------------------

depot_tools_path=$script_path/depot_tools
if [ -d $depot_tools_path ];then
    echo $depot_tools_path 已存在
else
    echo $depot_tools_path 不存在，下载depot_tools
    git clone --depth=1 https://chromium.googlesource.com/chromium/tools/depot_tools.git
fi

if [ $? -ne 0 ]; then
    echo depot_tools下载失败
    exit 1
fi

# 环境变量设置
export PATH=$depot_tools_path:$PATH

echo
echo ---------------------------------------------------------------
echo 初始化depot_tools
echo ---------------------------------------------------------------

gclient

if [ $? -ne 0 ]; then
    echo 初始化depot_tools失败
    exit 1
fi

echo
echo ---------------------------------------------------------------
echo 同步webrtc代码
echo ---------------------------------------------------------------

webrtc_path=$script_path/webrtc
webrtc_src_path=$webrtc_path/src
if [ ! -d $webrtc_path ];then
    mkdir $webrtc_path
fi

cd $webrtc_path
if [ ! -d $webrtc_src_path ];then
    fetch --nohooks webrtc
fi

gclient sync --force
cd $webrtc_src_path

# 基于当前最新release分支72来开发
git checkout -b branch-heads/72 remotes/branch-heads/72
git pull
# 切换分支以后必须sync，来同步不同分支的build tools
# 不能再加--nohooks，否则不会下载webrtc\src\buildtools\win\gn.exe等编译工具
gclient sync

if [ $? -ne 0 ]; then
    echo webrtc同步失败
    exit 1
fi

# 恢复工作目录
cd $old_cd

echo all同步成功