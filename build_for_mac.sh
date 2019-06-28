# 获取绝对路径，保证其他目录执行此脚本依然正确
{
cd $(dirname "$0")
script_path=$(pwd)
cd -
} &> /dev/null # disable output

# 设置当前目录，cd的目录影响接下来执行程序的工作目录
old_cd=$(pwd)
cd $(dirname "$0")

# 启动参数声明
debug_mode="false"

echo
echo
echo ---------------------------------------------------------------
echo 检查编译参数[debug/release]
echo ---------------------------------------------------------------

# 编译模式
mode=$(echo $1 | tr '[:upper:]' '[:lower:]')
if [[ $mode != "release" && $mode != "debug" ]]; then
    echo "waring: unkonow build mode -- $1, default debug"
    debug_mode="true"
else
    # 编译模式赋值
    if [ $1 == "release" ]; then
        debug_mode="false"
    else
        debug_mode="true"
    fi
fi

# 提示
if [ $debug_mode == "true" ]; then
    echo 当前编译版本为debug版本
else
    echo 当前编译版本为release版本
fi

# 环境变量设置
depot_tools_path=$script_path/depot_tools
export PATH=$depot_tools_path:$PATH

# 设置相关路径
gn=gn
ninja=ninja
dispatch_path=$script_path/out
if [ $debug_mode == "true" ]; then
    dispatch_path=$script_path/out/debug
else
    dispatch_path=$script_path/out/release
fi

# 进入webrtc目录
cd webrtc/src

echo
echo
echo ---------------------------------------------------------------
echo gn生成ninja脚本
echo ---------------------------------------------------------------

# ninja file
$gn gen $dispatch_path --args="is_debug=$debug_mode rtc_include_tests=false rtc_build_examples=false rtc_enable_protobuf=false rtc_build_tools=false"
if [ $? != 0 ]; then
    echo "generate ninja failed"
    exit 1
fi

echo
echo
echo ---------------------------------------------------------------
echo 开始ninja编译
echo ---------------------------------------------------------------

# build
$ninja -C $dispatch_path
if [ $? != 0 ]; then
    echo "ninja build failed"
    exit 1
fi

# 恢复当前目录
$old_cd

echo
echo
echo ---------------------------------------------------------------
echo 完成！
echo ---------------------------------------------------------------