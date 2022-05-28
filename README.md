[DesktopCapturer-image]: https://raw.githubusercontent.com/barry-ran/learn_webrtc/master/examples/DesktopCapturer/screenshot/main.png

[pc1-image]: https://raw.githubusercontent.com/barry-ran/learn_webrtc/master/examples/peerconnection/pc1/screenshot/main.jpg

# learn_webrtc
学习webrtc，基于webrtc native开发音视频工具

# example展示
## 桌面捕获
[![界面][DesktopCapturer-image]](https://github.com/barry-ran/learn_webrtc/tree/master/examples/DesktopCapturer)

## pc1
[![界面][pc1-image]](https://github.com/barry-ran/learn_webrtc/tree/master/examples/peerconnection/pc1)

# 环境要求
- 可以科学上网（首要前提）
- git

## mac
xcode

## win
- vs2019 C++开发环境（Windows 10 SDK(10.0.20348.0)）

    也可以给安装包指定命令行参数来安装(vs.exe为vs安装包，在这里[下载](https://visualstudio.microsoft.com/zh-hans/downloads/))
    ```
    vs.exe --add Microsoft.VisualStudio.Workload.NativeDesktop --add Microsoft.VisualStudio.Component.VC.ATLMFC --add Microsoft.VisualStudio.Component.Windows10SDK.20348 --includeRecommended

    ```

    注意：
    - Windows 10 SDK建议最新版本
    - win10 sdk需要安装了Debugging Tools For Windows(必须的，编译要求)，没有安装的话

        进入 控制面板\程序\程序和功能，选择Windows Software Development Kit，右键更改，选择Change，勾选Debugging Tools For Windows，点击Change    

# 编译步骤
## win
- 设置vs2019_install环境变量
    ```
    set vs2019_install=D:\Program Files (x86)\Microsoft Visual Studio\2019\Community
    ```
- 执行syc_for_win.bat同步webrtc代码(开发过程只需同步一次即可)
- 执行build_for_win.bat release编译

# 参考文档
- [webrtc官方编译说明](https://webrtc.github.io/webrtc-org/native-code/development/)
- [chromium编译官方说明](https://chromium.googlesource.com/chromium/src/+/master/docs/windows_build_instructions.md)

# F&Q
- ~~MAC下编译出的AppRTCMobile运行失败~~ 用的72分支，改为m76就没问题了
- peerconnection client为什么没有mac的？[PeerConnectionFactory::CreateVideoSource 不支持mac？](https://groups.google.com/forum/#!searchin/discuss-webrtc/mac$20peerconnection%7Csort:date/discuss-webrtc/ebLVdsXdU-g/Ot-80bZQAgAJ)

# 遇到过的编译问题
即使现在同步编译脚本没有问题了，也有可能过段时间编译不过了，因为depot_tools、src/build等都有可能更新导致编译不过

（官方说可以用老版本的depot_tools不知道是否可行，待验证）

遇到问题不要怕，根据报错问题耐心分析解决即可。
## 最近换了一台电脑编译源码，执行sync_for_win.bat报编码错误了，

错误信息如下（当然还有其它报错，这里就不贴出来了，因为这是第一个错误，下面的错误都是由它引起的）：

备注：解决问题要从第一个错误开始排查，因为下面的所有错误可能都是由它引起的，像这个问题我从最后一个错误排查，
是git_cache报错，查了半天是因为找不到git.bat脚本，原来是第一个错误导致git.bat没有复制过来，所以解决问题要从第一个错误开始排查。

    ```
    ---------------------------------------------------------------
    初始化depot_tools
    ---------------------------------------------------------------
    Traceback (most recent call last):
    File "D:\mygitcode\learn_webrtc\depot_tools\bootstrap\bootstrap.py", line 365, in <module>
        sys.exit(main(sys.argv[1:]))
    File "D:\mygitcode\learn_webrtc\depot_tools\bootstrap\bootstrap.py", line 324, in main
        git_postprocess(template, os.path.join(bootstrap_dir, 'git'))
    File "D:\mygitcode\learn_webrtc\depot_tools\bootstrap\bootstrap.py", line 264, in git_postprocess
        maybe_copy(
    File "D:\mygitcode\learn_webrtc\depot_tools\bootstrap\bootstrap.py", line 108, in maybe_copy
        content = fd.read()
    UnicodeDecodeError: 'gbk' codec can't decode byte 0x99 in position 18745: illegal multibyte sequence
    Usage: gclient.py <command> [options]

    Meta checkout dependency manager for Git.
    ```

难道是因为depot_tools更新了py脚本，导致了中文系统上的编码问题？

找到相关代码（D:\mygitcode\learn_webrtc\depot_tools\bootstrap\bootstrap.py", line 108），修复之：
    ```
    def maybe_copy(src_path, dst_path):
    """Writes the content of |src_path| to |dst_path| if needed.

    See `maybe_update` for more information.

    Args:
        src_path (str): The content source filesystem path.
        dst_path (str): The destination filesystem path.

    Returns (bool): True if |dst_path| was updated, False otherwise.
    """
    with open(src_path, 'r', encoding = 'UTF-8') as fd:
        content = fd.read()
    return maybe_update(content, dst_path)
    ```

主要是在open src_path的时候指定utf-8编码

## 报错找不到WINDOWSSDKDIR定义
WINDOWSSDKDIR是vs编译工具链中的环境变量，指示windows sdk的路径，运行一下vs自带的vcvarsall.bat即可注册

例如我的vs安装在d盘，那么vcvarsall.bat在这个路径：

```
D:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat
```

## gclient sync时vs_toolchain.py报错
报错信息:
```
if not available_versions:
    raise Exception('No supported Visual Studio can be found.'
                    ' Supported versions are: %s.' % supported_versions_str)
```

明明已经安装了 vs2017却提示找不到？查看vs_toolchain.py的提交记录发现这块代码最近有改动，查找vs默认只在c盘，
我vs2017安装在d盘它找不到，分析代码发现可以通过增加环境变量来解决：
```
set vs2017_install=D:\Program Files (x86)\Microsoft Visual Studio\2017
```



