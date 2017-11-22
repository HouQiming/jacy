# 内置功能

内置功能就是什么都不`import`也能用的功能。

### __basic_api

这些基本的C函数都可以用`__basic_api.memxxx`的形式调用：`memset`，`memcpy`，`memmove`，`memcmp`。例如这样：
```javascript
(function(){
	a=new int[1]
	b=new float[1]
	a[0]=0x3f800000
	__basic_api.memcpy(b,a,sizeof(int));
})();
```

### 向量类型

JC自带2～4维，成员是基本数值类型的向量类型，例如`float3`，`int4`。这些向量可以加减乘除，可以用`.x`、`.y`、`.z`、`.w`访问向量各分量。

目前暂不支持用`[变量下标]`访问向量各分量。

### 数组

JC中的数组可以用`[元素, 元素, ……]`或者`new 类型[大小]`的方式构造出来：

```javascript
import "system.jc"
import System.Console.*

(function(){
	Writeln([1, 2, 3, 4, 5]);
	Writeln(new int[5]);
})();
```

用`new`创建的数组元素会强制初始化成全0。

注意JC的数组是有类型的，用`[]`创建数组时必须确保所有元素全是同一类型：
- 正 `[1.0, 1.5, 2.0]`
- 误 `[1, 1.5, 2]` （1和2是int，1.5是double，JC不知道该用`new int[3]`还是`new double[3]`）

同样出于类型问题，JC无法像JS那样直接用`[]`创建空数组，而要用`new int[]`或者`new double[]`之类。

for in, for in (update)
new
push / resize
自动清0
传给C函数
slice
sort
ConvertToAsBinary
	over-optimization bug

### string

### Map

### crange

!?
