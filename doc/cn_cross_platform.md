# 跨平台问题

这里的“跨平台问题”指的是在跨平台开发的过程中遇到的平台相关问题，例如：
- 平台自带库的利用，比如Accelerate.framework
- 跨平台API的不兼容性，比如OpenGL
- 针对平台偏好语言的接口封装，比如ObjC, Java, ...

### 在JC或JS中判断平台

JC里提供一个表示平台的字符串常量`Platform.ARCH`。其值与编译时提供的`-a什么`或者`--arch=什么`相同。常见的取值有以下几种：
- `win32`，表示32-bit的Windows
- `win64`，表示64-bit的Windows
- `ios`，表示iOS
- `mac`，表示MacOS X
- `android`，表示Android
- `linux32`，表示32-bit的Linux
- `linux64`，表示64-bit的Linux

对于32位还是64位的判断，可以用`sizeof(iptr)==4`或者`sizeof(iptr)==8`来判断。`sizeof(iptr)==4`表示32位，`sizeof(iptr)==8`表示64位。

对于ARM还是x86的判断目前还没有支持。这种事情作者一般会选择在C里面做。

### 如何进行平台相关编译配置

JC支持在代码中加入“受静态if影响的编译选项”。可以利用这一点进行平台相关的编译配置，例如这样：
```javascript
if Platform.ARCH=="linux64":
	__generate_json("ldflags","-lOpenBLAS")
else if Platform.ARCH=="mac":
	__generate_json("mac_frameworks","System/Library/Frameworks/Accelerate.framework")
```

这段代码的功能是在Linux和Mac上分别链接平台提供的线性代数加速库，Linux上是OpenBLAS，Mac上是Accelerate.framework。

这里`__generate_json`是JC特有的编译选项设置函数，`__generate_json("ldflags","-lOpenBLAS")`表示在JC生成的json里的`ldflags`列表里加入`"-lOpenBLAS"`一项，以供平台相关的编译脚本使用。

注意到这里`Platform.ARCH=="linux64"`和`Platform.ARCH=="mac"`什么的都是常量判断。所以经过编译器剔除无用代码之后，里面的`__generate_json`只有在对应的平台上才会起作用。

基于同样道理，如果`__generate_json`写在了某个函数里，那么只有在这个函数会被实际调用的时候才会起作用。

### 平台相关的具体可配置内容

`__generate_json`主要有以下功能：

- 加入C源文件（全平台通用）：比如`__generate_json("c_files", "foo.c");`，`__generate_json("h_files", "foo.h");`。这样会把对应的文件加入到JC生成的工程里一起编译。对于`.cpp`或者`.m`之类的需要编译的类C语言代码文件也用`__generate_json("c_files", );`处理。对于本身不需要编译但会被其他文件用到的东西用`__generate_json("h_files", );`处理。
- 添加库文件（Windows）：Windows库一般是lib + dll的形式，添加到JC中时，需要用：
```javascript
__generate_json("lib_files", "foo.lib");
__generate_json("dll_files", "foo.dll");
```
把lib和dll分别加进来。
- 添加库文件（Mac和iOS）：在Mac和iOS上有两类库文件：普通的.a和ObjC的framework。对于普通的.a，用`__generate_json("lib_files", "foo.a");`加进去就好。但是对于framework，需要用特有选项的`mac_frameworks`和`ios_frameworks`，例如这样：
```javascript
if Platform.ARCH=="mac":
	__generate_json("mac_frameworks","System/Library/Frameworks/Accelerate.framework")
else if Platform.ARCH=="ios":
	__generate_json("ios_frameworks","System/Library/Frameworks/Accelerate.framework")
```
- 添加库文件（Android）：Android会把每个架构的库文件分散放在各自的目录里，所以Android链接库的时候需要先指定库目录，再指定库名。例如，如果要link `libsgemm/bin/android/libs/*/sgemm.a`的话，可以这么写：
```javascript
if Platform.ARCH=="android":
	__generate_json("lib_dirs","libsgemm/bin/android/libs")
	__generate_json("android_static_libnames","sgemm")
```
如果用扩展名是.so的动态库，要这么写：
```javascript
if Platform.ARCH=="android":
	__generate_json("lib_dirs","libsgemm/bin/android/libs")
	__generate_json("android_libnames","sgemm")
```
对于系统库的话，需要写入`android_system_libnames`：
```javascript
if Platform.ARCH=="android":
	__generate_json("android_system_libnames","android")
```
- 添加库文件（Linux）：对于Linux系统库，可以直接在`ldflags`里加个`-l`：
```javascript
if Platform.ARCH=="linux":
	__generate_json("ldflags","-lOpenBLAS")
```
对于普通的.a或.so，用`__generate_json("lib_files", "foo.a");`就好。
- 设置C编译选项（全平台通用）：使用`__generate_json("cflags","选项")`
- 设置C++编译选项（全平台通用）：使用`__generate_json("cxxflags","选项")`
- 设置链接选项（全平台通用）：使用`__generate_json("ldflags","选项")`
- 添加图标：使用`__generate_json("icon_file","文件")`。注意对于Android平台，目前就算是编译库文件也需要提供一个图标，以便通过ant的编译流程（图标最后不会真正用到，但必须是合法的图片文件）

Windows的特有选项：
可以通过`__generate_json("vc_versions", "内部版本")`来选择Visual Studio的版本。例如`__generate_json("vc_versions", "14")`会选择Visual Studio 2015。

Android的特有选项：
可以通过`__generate_json("java_files", "foo.java")`来添加用于接口封装的Java文件。这些文件在编译出库之后会统一封装到jar里。
