# 编译webrtc.lib
首先从webrtc源码编译出webrtc.lib供我们上层开发使用，需要注意几个地方：
- 使用最新的release分支

    git checkout -b branch-heads/72 remotes/branch-heads/72

    切换完分支记得gclient sync，会同步不同分支下的编译工具，否则可能会编译不过

- 编译md格式

    webrtc在windows下默认构建为mt格式的静态库，但是现在主流是md，不得已要修改webrtc编译脚本

    修改build\config\win\BUILD.gn中默认的configs = [ ":static_crt" ]改为configs = [ ":dynamic_crt" ]

- 指定gn参数

    编译参数|说明
    -|-
    is_debug=true|debug还是release
    target_cpu="x86"|编译32位
    is_clang=false|不适用clang编译器
    rtc_build_examples=true|编译examples，指定了这个才会编译peerconnection相关库
    rtc_build_tools=false|不用编译tools
    rtc_include_tests=false|不用编译tests
    enable_iterator_debugging=true|debug模式要打开这个选项，否则[_ITERATOR_DEBUG_LEVEL](https://docs.microsoft.com/en-us/cpp/standard-library/debug-iterator-support?view=vs-2019)不匹配
         

# 使用webrtc.lib
按照以上步骤编译出的webrtc.lib我们可以集成到自己的项目中使用了，不过需要注意几个问题：

- windows下定义几个宏

    宏|说明
    -|-
    WEBRTC_WIN|告诉webrtc当前是windows平台
    WIN32_LEAN_AND_MEAN|待补充
    USE_BUILTIN_SW_CODECS|待补充
    HAVE_WEBRTC_VIDEO|待补充
    NOMINMAX|windows下max宏和std max冲突
    INCL_EXTRA_HTON_FUNCTIONS|待补充

- 找不到头文件
根据缺少的头文件来看，需要指定几个头文件包含路径

..\..\..\webrtc\src\third_party\libyuv\include;

..\..\..\webrtc\src\third_party\jsoncpp\source\include;

..\..\..\webrtc\src;..\..\..\webrtc\src\third_party\abseil-cpp

- 出现很多未定义的符号
未定义的符号只有一个原因，就是相应lib库没有引入，主要分三部分：

- webrtc在windows下用了很多特定库，例如directshow的库，要引入以下几个库：

    - wmcodecdspuuid.lib
    - dmoguids.lib
    - Msdmo.lib
    - Secur32.lib
    - Shell32.lib、
    - Gdi32.lib
    - Winmm.lib
    - Advapi32.lib
    - strmiids.lib

- webrtc.lib并没有包含所有的实现，如果用到的符号在webrtc.lib中没有，需要自己引入需要的lib，例如编译example/peerconnection_client的话，你还要引入其他几个webrtc相关库

    - webrtc.lib
    - create_peerconnection_factory.lib
    - rtc_base.lib 
    - json_vc71_libmdd.lib json实现没有暴露出来，需要自己编译一下third_party/jsconcpp

- 有的符号webrtc.lib没有包含，其他导出的lib库都没有包含，例如原gn项目依赖了test中的项目，test没有导出来
需要我们自己把相关的实现文件复制出来添加到现有工程中

例如webrtc\src\rtc_base\strings\json.cpp中GetStringFromJson相关实现