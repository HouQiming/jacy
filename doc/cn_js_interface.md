# 和JS的接口

JC里面的JS支持是基于[duktape](http://duktape.org)实现的，但是为了方便，做了一些封装。以下介绍JC封装的接口：

### 搭建JS解释器环境

一个最简单的例子：
```javascript
import 'system.jc'
import 'javascript.jc'
import System.Console.*
import Javascript.*

(function(){
	JS=new JSContext
	///////////
	Writeln('----------------------------------------------------')
	JS.eval(int,"print('Hello world!')")
	///////////
	Writeln('----------------------------------------------------')
	ret=JS.eval(double,"1+0.23")
	Writeln('JS returned: ',ret)
})();
```

其中`new JSContext`会创建一个新的JS解释器环境（包含所有duktape自带的全局对象）。`JS.eval`会运行一段JS，然后返回一个指定类型的值。

### 在JC中使用JS对象

在JC中对应JS对象的类型是`JSObject`。可以用`对象[成员名].as(类型)`的方式来访问JS对象的成员：
```javascript
import "system.jc"
import "javascript.jc"
import System.Console.*
import Javascript.*

(function(){
	JS=new JSContext
	///////////
	Writeln('----------------------------------------------------')
	obj0=JS.eval(JSObject,"(function(){return {dbl_value:4.56,str_value:'Hello world!',array_value:[100,200,300]}})()")
	Writeln('obj0.dbl_value=',obj0["dbl_value"].as(double))
	Writeln('obj0.str_value=',obj0["str_value"].as(string))
	Writeln('obj0.str_value is not a double: ',obj0["str_value"].as(double))
	Writeln('obj0.array_value[1]=',obj0["array_value"][1].as(int))
	Writeln('obj0.array_value[100] is undefined: ',obj0["array_value"][100].as(double))
})();
```

也可以通过`对象[成员名] = 值`的方式来修改JS对象的成员，以及用`对象.CallMethod(返回类型, 方法名, 参数……);`的方式来调用JS对象的方法：
```javascript
import "system.jc"
import "javascript.jc"
import System.Console.*
import Javascript.*

(function(){
	JS=new JSContext
	Javascript.duktape.duk_console_init(JS._ctx(),0) // initialize 'console.log' for debugging
	///////////
	Writeln('----------------------------------------------------')
	obj0=JS.eval(JSObject,"(function(){
		return {
			m_name:'initial_name',
			PrintName:function(){
				console.log('My name is',this.m_name);
				return 0;
			}
		};
	})()")
	obj0.CallMethod(int,'PrintName')
	obj0['m_name']='new_name'
	obj0.CallMethod(int,'PrintName')
})();
```

最后，在JC中可以用`JS.New()`和`JS.NewArray()`来创建空JS对象和空JS数组（相当于JS中的`{}`和`[]`）。

由于JS本身的设计特点，大多数的语言功能都可以借助对象成员进行访问。例如想利用JS的标准库解析JSON的话，可以这样：
```javascript
JS['JSON'].CallMethod(JSObject, 'parse', '{"value":42}')
```
这里相当于直接把`JS`当作全局对象进行访问，调用了JS标准库的`JSON.parse`函数。

### 给JS导出JC函数

这应该是接口中最重要的一项功能，其作用是将应用的底层接口暴露给JS，以便在JS中实现上层逻辑。基本做法是将JS对象的成员赋值成特定形式的JC函数：
```javascript
obj_gl=JS.New()
obj_gl["clearColor"]=function(JSContext JS){
	red=JS.Param(0).as(GLclampf);
	green=JS.Param(1).as(GLclampf);
	blue=JS.Param(2).as(GLclampf);
	alpha=JS.Param(3).as(GLclampf);
	glClearColor(red,green,blue,alpha);
	return 0;
}
```



### 在JS中使用JC对象

!? todo: typed array, ...
