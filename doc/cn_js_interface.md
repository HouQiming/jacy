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

这应该是接口中最重要的一项功能，其作用是将应用的底层接口暴露给JS，以便在JS中实现上层逻辑。基本做法是将JS对象的成员赋值成特定形式的JC函数，例如：
```javascript
obj_gl=JS.New()
obj_gl["getFramebufferAttachmentParameter"]=function(JSContext JS){
	target=JS.Param(0).as(GLenum);
	attachment=JS.Param(1).as(GLenum);
	pname=JS.Param(2).as(GLenum);
	ret=0
	glGetFramebufferAttachmentParameteriv(target,attachment,pname,&ret)
	return JS.Return(ret)
}
```

这里JC函数的参数必须只有一个`JSContext JS`，表示调用函数的JS解释器环境。由JS传过来的参数需要用`JS.Param(参数编号).as(类型)`的方法获得。如果需要返回什么值，必须用`return JS.Return(返回值)`。如果需要返回错误，必须用`return JS.ReturnError('错误信息')`。如果要返回`undefined`，请用`return 0`。

### 在JS中使用JC对象

普通JC对象在传递给JS之后会变成没有任何有意义成员/方法的黑盒子，只能经由JS重新传给JC处理。如果需要在JS中访问JC对象的方法/成员的话，需要在JC对象中暴露一个`__JS_prototype`方法：
```javascript
class CNativeClass
	int n
	js_hello=function(JSContext JS){
		Writeln('n=',n,' param=',JS.Param(0).as(string))
		return 0
	}
	auto __JS_prototype(JSObject proto)
		proto.ExportProperty(this,"n")
		proto.ExportMethod(this,"hello",js_hello)
		return proto
```

如上所示，`__JS_prototype`需要接收一个`proto`对象（其实是JS中对应于这个JC类的`__prototype__`对象），然后在里面填进去需要暴露的成员和方法。`proto.ExportProperty(this,"成员名")`用来暴露成员名，`proto.ExportMethod(this,"JS里见到的方法名"，JC里的方法名)`用来暴露方法。

### 功能限制

任何传给JS的数据都会受到JS语言层面和duktape实现层面的限制：
- 如无特殊说明，所有传给JS的数值类型都会被强制转换成`double`。如果原本是`i64`或者`u64`的话，可能会有精度损失。
- duktape会默认所有传给JS的字符串都经过了合法的UTF8编码，但并不会真的检查这一点。如果传给JS一个非UTF8字符串之后又在上面调用了JS自带的字符串处理函数的话（比如`.match(……)`），就可能会出现各种奇怪错误。
- 出于性能的考虑传给JS的数值数组会变成JS中的typed array而非普通数组。里面的数值不会强转成double，但要注意typed array并不支持array的很多方法，比如`.push()`，比如越界写入，比如修改数组元素类型。
- 传给JS的JC对象会由JS的垃圾回收机制接管，如果JS一侧没有运行gc，就可能得不到释放。这种情况下可以用`JS.ResetHeap()`强制重置JS解释器环境，释放所有的接管对象。

出于性能的考虑，JC中在访问JS对象成员时直接使用了duktape的全局栈，所以**同一条JC语句中只能出现至多一次JS对象的成员访问**，`JS.eval`、`JS.Return`和`JS.Param`之类也包括在内。对这一点并没有进行检查，需要自行注意。比如这么写就是错的：
```javascript
return JS.Return(JS.Param(0).as(int) + obj["m_constant"].as(int))
```
必须改成这样：
```javascript
param0=JS.Param(0).as(int)
param1=obj["m_constant"].as(int)
return JS.Return(param0 + param1)
```
