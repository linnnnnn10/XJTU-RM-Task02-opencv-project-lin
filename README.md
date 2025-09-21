遇到的报错：

正确安装opencv且修改CMakeLists之后，头文件引用<opencv4/opencv2/core.hpp>报错，但似乎可以编译。

核心原因：

​编译环境（CMake）和代码智能感知环境（Clangd）的头文件搜索路径不一致​​

解决方案：

1.在CMakeLists内，project()行以下添加如下代码：

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

用来强制生成compile_commands.json文件。

2.在CMakeLists同级目录下创建.clangd文件，文件格式严格如下：

CompileFlags:
  CompilationDatabase: build

build处替换为compile_commands.json所处目录。

3.于终端运行如下代码：

ln -s /home/linnnnnn/opencv-project/build/compile_commands.json /home/linnnnnn/opencv-project/

路径根据实际修改。用来链接文件。

4.重启vscode。
