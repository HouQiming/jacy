# 和C/C++的接口

### 调用C函数

最简单的接口形式是用`System.Native.ImportCFunction`直接将C函数引入JC，比如这样：
```javascript
printf=System.Native.ImportCFunction(int,'printf','stdio.h')
exit=System.Native.ImportCFunction(int,'exit','stdlib.h')

(function(){
	printf('The magic number is %d\n', 42);
	exit(42);
})();
```

这里有几点需要注意：
- 在ImportCFunction中需要依次填写返回类型，C侧的函数名，以及C侧的头文件
- JC不关心C函数的参数数量/类型，如果写错的话，会由C编译器返回错误，错误信息可能比较难懂，但位置应该是正确的
- JC里没有void类型，如果函数没有返回值，请填写int，然后不要在JC中实际使用这个返回值（比如上面的exit）
- 如果在JC里没有实际调用这个函数，那么`ImportCFunction`里填写的头文件并不会被引用
- 老代码里会有类似`__c_function(int,'printf','stdio.h')`的老写法，和`System.Native.ImportCFunction`是等价的

### 为C++代码封装C接口

为了最大限度保证平台兼容性，JC编译之后会生成纯C代码，无法直接调用C++的类/函数。

解决方法是先把C++功能封装成C函数。例如，假如想在JC里调用这样一个C++类的话：
```C++
class CSomeObject{
public:
	int SomeFunction(std::string some_name, double some_coefficient);
};
```

可以进行这样的封装：
```C++
extern "C" void* CreateSomeObject(){
	return (void*)(new CSomeObject);
}

extern "C" void DestroySomeObject(void* handle){
	delete (CSomeObject*)handle;
}

extern "C" int CallSomeFunctionForSomeObject(void* handle, const char* some_name, double some_coefficient){
	std::string some_name_string(some_name);
	return ((CSomeObject*)handle)->SomeFunction(some_name_string, some_coefficient);
}
```

然后写一个兼容纯C的头文件：
```C++
#ifdef __cplusplus
extern "C"{
#endif
void* CreateSomeObject();
void DestroySomeObject(void* handle);
int CallSomeFunctionForSomeObject(void* handle, const char* some_name, double some_coefficient);
#ifdef __cplusplus
}
#endif
```

最后在JC里用`System.Native.ImportCFunction`引用这三个函数：
```javascript
CreateSomeObject=System.Native.ImportCFunction(__pointer,'CreateSomeObject','some_wrapper.h')
DestroySomeObject=System.Native.ImportCFunction(int,'DestroySomeObject','some_wrapper.h')
CallSomeFunctionForSomeObject=System.Native.ImportCFunction(int,'CallSomeFunctionForSomeObject','some_wrapper.h')
```

### 使用C指针

JC中只有一个指针类型`__pointer`，对应于C的`void*`，除了转换成指针大小整数类型`iptr`或者传给其它C函数外，无法直接使用。

从C接收过来的指针一般都是这个类型。如果需要在JC中访问指针指向的内存的话，可以用`iptr`手工实现指针运算，然后使用JC的`__memory`函数访存，例如这样：
```javascript
malloc=System.Native.ImportCFunction(__pointer,'malloc','stdlib.h')

(function(){
	p=malloc(sizeof(int)*4);
	for i=0:3
		__memory(int, iptr(p) + i*sizeof(int)) = i
})();
```

`__memory(int, p)`相当于C的`(*(int*)p)`。

### 使用C结构体

对于某些需要借助特定结构体传递参数的C函数，可以在JC中使用`System.Native.CreateLocalCStruct`来定义结构体类型的局部变量。比如对这样的头文件：
```C
#ifndef __NATIVE_H
#define __NATIVE_H
typedef struct{
	int ival;
	double dval;
	char sval[32];
	char sval2[32];
}TNativeStruct;

void FillStructMembers(TNativeStruct* pstruct);
void PrintStruct(TNativeStruct* pstruct);
#endif
```

可以在JC里面这么使用：
```javascript
import "system.jc"
import System.Console.*

(function(){
	System.Native.ProjectAddSource("native.c")
	FillStructMembers=System.Native.ImportCFunction(int,"FillStructMembers","native.h")
	PrintStruct=System.Native.ImportCFunction(int,"PrintStruct","native.h")
	s=System.Native.CreateLocalCStruct("TNativeStruct")
	s2=System.Native.CreateLocalCStruct("TNativeStruct")
	FillStructMembers(s)
	Writeln(s["ival"].as(int),' ',s["dval"].as(double),' ',s["sval"].as(string),' ',s["sval2"].as(string))
	PrintStruct(s)
	s["ival"]=100
	s["dval"]=9876.5
	s["sval"]="modified"
	s["sval2"]="modified2"
	PrintStruct(s)
	s2["ival"]=1000
	s2["dval"]=98765.4321
	s2["sval"]="s2_modified"
	s2["sval2"]="s2_modified2"
	ptr=System.Native.CreateCPointer(char,s2["sval2"].as(__pointer))
	ptr[0]='S'
	ptr[1]=ptr[0]
	PrintStruct(s2)
	Writeln(s2["ival"].as(int),' ',s2["dval"].as(double),' ',s2["sval"].as(string),' ',s2["sval2"].as(string))
})()
```

其中`s=System.Native.CreateLocalCStruct("TNativeStruct")`会创建一个类型为`TNativeStruct`的局部变量`s`。之后可以用`s["ival"].as(int)`这样的方法，手动填写成员名和类型来读取结构体的成员。需要写入结构体成员的时候，只需正常赋值`s2["ival"]=1000`，JC会自动检测类型。

### 直接插入C代码

对于C中`#define`的常量/宏，OpenMP之类的特殊语法，或者诸如此类的琐碎问题，可以通过在JC中直接插入C代码来解决。比如这样`__C(int,'MB_OK')`。其中第一个参数的`int`表示插入的C表达式的返回类型，之后的字符串是一个C表达式。

如果要在C代码中访问JC变量，可以这样：
```javascript
a=42
b=3.14
__C(int,'printf("%d %lf\n",@1,@2)',a,b)
```

其中的`@1`和`@2`会在最终的C代码里替换成后面的表达式（这里的`a`和`b`）。
