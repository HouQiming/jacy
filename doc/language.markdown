# The Jacy Language Reference Manual

## Quick start

### Key properties

* Cross-platform development on Windows, Mac, Linux, Android, and iOS
* Fast enough for implementing numerical algorithms
* Native language with automatic memory management
* Bridges Javascript and C

### Using the compiler

The command-line:

	bin\win32_release\jc [--build=BUILD] [--arch=ARCH] xxx.jc [--run arguments]

`BUILD` can be `debug` or `release`. `ARCH` can be `win32`, `win64`, `mac`, `linux32`, `linux64`, `android`, or `ios`. By default, we have `--build=debug --arch=win32`.

Examples:

* Build and run the app example 0 on win32

	`bin\win32_release\jc test\app_example\app.jc --run`

* Build and run the app example 1 on Android, using the release version

	`bin\win32_release\jc --build=release --arch=android test\app_example1\app1.jc --run`

The UI editor is *bin\win32_release\mo.exe*. Run it, hit `Ctrl+O`, and open *res/ui_main.js* in the examples. Move the cursor in the edit box to choose the current “window”.

### Example code

Let's start with a hello world program:

	import System.Console.*
	
	(function(){
		Writeln('Hello world')
	})()

Basically:
* Import libraries like Java: `import System.Console.*`
* Define functions like Javascript:	`(function(){})()`
* Library function names look like C#: `Writeln('Hello world')`
* Strings go either way like Javascript: `'Hello world'`

## Language

### Variables and expressions

The basic syntax closely follows C. The expression syntax is exactly the same, down to operator precedence. Please consult a C textbook for details.

Variable definitions are implicit: undeclared variables automatically goes to the lowest-appeared scope, and untyped variables have their types automatically deduced. For example:

	import System.Console.*
	
	(function(){
		a=100
		Writeln('a=',a)
	})()
	
	//you can not access 'a' out of the previous function
	//Writeln(a) <-- Error

You can also declare something the C-way if you really want to:

	import System.Console.*
	
	(function(){
		int a
		Writeln('a=',a) //prints 0
	})()

For the record, the compiler will just read `int a` as `a=int(0)`.

To define a constant, just define a variable and *don't change it*:

	import System.Console.*
	
	ONE=1
	TWO=2
	THREE=3
	(function(){
		Writeln(ONE+TWO+THREE)
		if ONE==TWO:
			Writeln('huh?')
	})()

In the generated C code or executable, you won't find 'huh?', which means they are indeed treated as constants and optimized out.

### Functions

As implied by the previous examples, Jacy functions are more like Javascript functions than C functions. You can use them as expressions:

	import System.Console.*
	import System.Algorithm.*
	
	(function(){
		dat=[1,4,2,8,5,7,3,6,9]
		//sort by negated values, i.e., sort in descending order
		dat.Sortby(function(int a){return -a})
		Writeln(dat)
	})()

or nest them:

	import System.Console.*
	
	(function(){
		a=0
		inc=function(){a++}
		Writeln('a=',a)
		inc()
		Writeln('a=',a)
	})()

or if you really want to, do it the C-way:

	import System.Console.*

	//the return type must be 'auto'
	auto square(int a){
		return a*a;
	}
	
	auto main(){
		Writeln(square(99))
	}
	
	main()
	
The compiler would just read it as:

	auto square=function(int a){
		return a*a;
	}
	auto main=function(){
		Writeln(square(99))
	}
	main()

Functions cannot be overloaded -- each function “definition” is simply an assignment and they simply overwrite each other (generating an error in case the prototypes are different). However, if you really want, you can use the operator `|` to combine functions for the same effect. That's not recommended though, since Jacy inline functions are more general. Please refer to `units/__builtin.jc` for examples.

### if, for, switch

The `if` statement is exactly the equivalent to the C counterpart.

C-like `for` statements are also supported:

	import System.Console.*
	
	(function(){
		for(i=0;i<10;i++)
			Writeln('i=',i)
	})

And there is a Matlab-like variant:

	import System.Console.*
	
	(function(){
		for i=0:9
			Writeln('i=',i)
	})

The `switch` statement is also C-like, but with two twists. First, each default / case clause is automatically terminated with a `break;`. Second, each `case` can have multiple labels. For example:

	import System.Console.*
	
	(function(){
		for(n=0;n<8;n++)
			Write(n,' ')
			switch(n){
			default:
				Writeln('default')
			case 5:
				Writeln('case 5')
			case 1,2:
				Writeln('case 1,2')
			case 3:
			case 4:
				Writeln('case 3,4')
			}
	})()

prints:

	0 default
	1 case 1,2
	2 case 1,2
	3 case 3,4
	4 case 3,4
	5 case 5
	6 default
	7 default

### Classes and objects

Jacy's class handling is the biggest difference from a lower level language like C++. Here is a moderately complicated example for illustration (also available at *test/test_rc0.jc*):

	import System.Console.*
	
	g_id=0
	class CTest{
		m_id=g_id++
		m_name=""
		ref0=CTest.NULL
		ref1=CTest.NULL
		auto __init__(string name){
			m_name=name
			Writeln(m_name,' ',m_id,' is created')
		}
		auto __done__(){
			Writeln(m_name,' ',m_id,' is destroyed')
		}
		auto print(){
			Writeln(m_name,' ',m_id,' prints')
		}
	}
	
	auto creator(){
		return CTest("creator")
	}
	
	auto f(){
		local=CTest("f")
		nested=creator()
		local.ref0=nested
		local.ref0.print()
		/////
		creator()
	}
	
	global=CTest("global")
	global=CTest("global")
	f()

There are several things to consider:
* Class members can be initialized in definition 'm_id=g_id++'
* `NULL` must be typed: `CTest.NULL`
* Constructors and finalizers are written as `__init__` and `__done__` respectively
* Classes can have methods like `print()`. They are, in fact, just function-valued variables that never get changed

There are also notable points not illustrated in the example:
* **There is no `delete`**, objects are destructed automatically when they are no longer needed (running the example shows that)
* Objects must be created using `new`. Declarations like `CTest ref0` are equivalent to `ref0=CTest.NULL` (try it yourself)
* There is no inheritance at all. The same functionality is more easily achieved using Javascript or creating an interface class (later in this section).

The automatic object destruction is implemented using reference counting. As a consequence, if you create a reference loop, it will leak. So don't do that, or do that in Javascript instead.

It is possible to create the illusion of “writable” function return values by implementing a separate setter function:
	
	import System.Console.*
	
	class CClass1{
		m_value=0
		auto print(){
			Writeln('calling CClass1.print, interal m_value=',m_value,', property value()=',value())
		}
		auto value(){
			return double(m_value)/100.0
		}
		auto set_value(double a){
			m_value=int(a*100.0)
		}
	}
	
	(function(){
		a=new CClass1()
		a.value()=1.5 //equivalent to a.set_value(1.5)
		a.print()
		a.value()=12.345
		a.print()
	})()

As shown above, the setter function is defined by prepending `set_` to the original method name and called using the operator “=”.

If you really want, you can also define `operator[]` and `set_operator[]` as methods. Refer to `units/__builtin.jc` for an example.

General operator overloading is provided in an intuitive syntax:

	(double3x3*double3)=inline(A,B){
		return double3(
			A.a00*B.x+A.a01*B.y+A.a02*B.z,
			A.a10*B.x+A.a11*B.y+A.a12*B.z,
			A.a20*B.x+A.a21*B.y+A.a22*B.z)
	}

If you look more closely, though, it's actually equivalent to:

	__set_operator*(double3x3,double3,inline(A,B){
		return double3(
			A.a00*B.x+A.a01*B.y+A.a02*B.z,
			A.a10*B.x+A.a11*B.y+A.a12*B.z,
			A.a20*B.x+A.a21*B.y+A.a22*B.z)
	})

which is just an syntactically-ordinary call of an intrinsic function `__set_operator*`.

Polymorphism in Jacy is either left to Javascript, or implemented using the interface paradigm. Here is an example (`test/test_get_interface.jc`):
	
	import System.Console.*
	
	class CClass0{
		auto print(){
			Writeln('calling CClass0.print')
		}
		auto set_value(double a){
			Writeln('CClass0.set_value is ignored')
		}
	}
	
	class CClass1{
		m_value=0.0
		auto print(){
			Writeln('calling CClass1.print, value=',m_value)
		}
		auto set_value(double a){
			m_value=a
		}
	}
	
	class IInterface{
		function(double a) set_value
		function() print
	}
	
	(function(){
		c0=new CClass0
		c1=new CClass1
		c2=new CClass1
		arr=[getInterface(c0,IInterface),getInterface(c1,IInterface),getInterface(c2,IInterface)]
		for(i=0;i<3;i++){
			a=arr[i]
			a.value()=123.45+double(I)*10000.0
			a.print()
		}
	})()

Here `getInterface` is a built-in function to fetch method pointers into members of an “interface object”. That allows different classes implementing similar methods to be converted into the same type, creating an illusion of overloaded methods.

### Structs

Structs are an *entirely different* matter in Jacy. It's more like C structs than Java/Javascript classes:
* While classes are always new-ed into references, Jacy structs reference *can never be referenced*. Struct assignments like `a=b` are value copies, not reference copies.
* By extension, declaring a struct variable like `int2 a` is equivalent to `a=int2()`, not `a=int2.NULL`
* By more extension, structs can be cheaper than classes as their creation do not involve an underlying `malloc`.
* By even more extension, structs are better suited than classes for implementing pure data like complex numbers or vectors. References to such types do not make any sense mathematically.

Here is an example of defining and using structs (*test/test_struct0.jc*):

	import System.Console.*
	
	struct int2x{
		x=0
		y=1
		auto __init__(v_x,v_y){
			x=v_x
			y=v_y
		}
	}
	
	(function(){
		a=int2x(3,4)
		b=int2x(5,6)
		c=a
		d=int2x()
		a.x+=b.x
		a.y+=b.y
		Writeln("a=",a)
		Writeln("b=",b)
		Writeln("c=",c)
		Writeln("d=",d)
	})()


### Arrays and maps

Arrays have been used several times before. The easiest way to create one is operator `[]` as in the previous function example:

	import System.Console.*
	import System.Algorithm.*
	
	(function(){
		dat=[1,4,2,8,5,7,3,6,9]
		dat.Sortby(function(int a){return -a})
		Writeln(dat)
	})()

While that may look like Javascript or Python, there is a key difference: Jacy arrays are typed, meaning that all elements must be of the same type. Something like [123,"string"] is a sure way to get an error. That makes Jacy arrays much, much faster than scripting-language arrays, though. They are more like the C++ `vector`. Furthermore, when applicable, Jacy arrays can be optimized into a simple C pointer for most accesses.

The more C++-like way of creating an array is to use `new` like:

	a=new int[100]

Arrays can be enumerated using Python-like `for in`:

	import System.Console.*
	
	(function(){
		dat=[1,4,2,8,5,7,3,6,9]
		for a in dat
			Writeln(a)
	})()

If you want to update the elements, simply put up an additional index variable:

	import System.Console.*
	
	(function(){
		dat=[1,4,2,8,5,7,3,6,9]
		for a,I in dat
			dat[I]=a*10
		Writeln(dat)
	})()

Normal classes can implement the same behavior, in fact, `for in` is just a syntax sugar for something like this:

	dat.forEach(inline(a,I){
		dat[I]=a*10
	})

Provide a `forEach` method, and `for in` is ready to use. You can alternatively write it as `foreach in`. Please refer to *units/__builtin.jc* for examples.

Jacy also provides C++-like maps in an intuitive syntax:

	a=new int[string]
	
For a comprehensive example, refer to *test/test_map.jc*

### Type manipulation

Like C++, Jacy has types. In addition to the C-like builtin types (char,short,int,float,double), Jacy provides bit-sized numerical types. You can write `i32` for 32-bit ints, `f64` for 64-bit floating point numbers, or `u8` for 16-bit unsigned integers, i.e., bytes. You can also use `iptr` for pointer-sized signed integers. Of course, classes and structs are also types.

Sometimes it's desirable to assign aliases to existing types. In that case, just use simple assignments and *don't change it*.

	GLintptr=iptr
	GLint64=i64
	GLuint64=u64

There are built-in 2D-4D vectors in D3D/CUDA names like `float3` or `uint4`.

## Writing apps

This section deals with features bending on the “application” side, i.e., developing phone apps with GUI.

### Getting started

An example app is available at *app_example/app.jc*, with explanation in comments.

### Imports and modules

Larger projects may have multiple files. In Jacy, all other files are imported into a main files not unlike the C `#include`. The syntax is

	import "some-file.jc"

For Windows compatibility, Jacy requires source files to use all-lowercase names. Each imported file will be in its own namespace. The convention is:

	"javascript.jc" -> Javascript
	"sdl.jc" -> Sdl
	"s-d-l.jc" -> SDL

The import command can also simulate C++ `using` like this:

	import Javascript.*
	import System.Console.*

You can also create sub-namespaces in a file using the `module` command:

	module Console{
		Writeln=inline(){
			//blah, blah
		}
	}
	
	Console.Writeln(/*blah, blah*/)

### C interface

The basic C interface is simple, try this:

	printf=__c_function(int,"printf","stdio.h")
	(function(){
		print("Hello world!\n");
	})()

Basically, a C-function is declared like this:
	
	__c_function(return-type,"name","something.h")

If it returns `void`, put up `int` in the return type place and do not use the “return value”. If you want to pull in a C file into the build process, add a line like:

	__generate_json("c_files","something.c")

If you have to dealing with pointer types, use the `__pointer` type, which translates to `void*` in C. To access memory directly in Jacy, use

	__memory(type,iptr(pointer))

as `(*(type*)pointer)`. There are a lot of examples in *units/system.jc*.

### Javascript embedding

A Javascript embedding is provided in the `javascript.jc` builtin-unit. It's based on the duktape interpreter. A simple example looks like this:

	import 'javascript.jc'
	import Javascript.*
	
	(function(){
		JS=new JSContext
		JS.eval(int,"print('Hello world!')")
	})()

More detailed examples are provided in *test/test_js0.jc* and *test/test_js1.jc*. They cover the following features:
* Loading and running JS code
* Accessing JS objects in native code
* Exposing native object to JS code

### Declarative UI using JS

The GUI part is written in a declarative manner. Each widget is a written as a function call like (in *app_example1*):

	/*widget*/(W.EditBox('editbox_629',{'x':91.86343383789063,'y':22,'w':186.38531251363247,'h':24.471364974975586,
		property_name:'a'}));

Here `W.EditBox` is the widget type (where `W`, by convention, is the namespace for holding widget types). 'editbox_629' is a unique name that can be used to access the widget in other places. The object `{'x':...}` defines widget attributes, and `property_name` associates the widget with a property sheet entry (something document-view-ish, explained in the comments of *test/app_example/res/ui_main.js*). The comment `/*widget*/` and the redundant parenthesis are generated by and provided for the UI editor.

Basically, you control the look, feel, and positioning of a widget using attributes. A more complicated example is like this (again in *app_example1*):

	/*widget*/(W.Label('text_2435',{
		anchor:obj,anchor_placement:'left',anchor_align:'right',anchor_valign:'center',
		'x':66.32084545389353,'y':0,
		font:UI.Font(UI.font_name,72,-50),
		text:'Pull here',color:UI.current_theme_color}));

Here, we have a customized label that is:
* anchored to the left of another widget (`anchor:obj,anchor_placement:'left'`)
* horizontally aligned to the right and vertically centered (`anchor_align:'right',anchor_valign:'center'`)
* rendered using a custom font `UI.Font(UI.font_name,72,-50)` that is large `72` and light `-50`
* colored using the UI theme color `color:UI.current_theme_color`
* and shows the text `'Pull here'` (which is obvious)

To find a list of available attributes and how to add custom widgets, read the widget implementation code in *units/gui2d/widgets.js*

Custom panels like in *app_example1* are written like this:

	var SomePanel=function(id,attrs){
		/*editor: UI.BeginVirtualWindow(id,{w:720,h:480,bgcolor:0xffffffff})//*/
		....
		/*editor: UI.EndVirtualWindow()//*/
	}

where the `/*editor: //*/` comments are again for the UI editor. The `function(id,attrs)` part is a convention even if you don't use the parameters. It also provides a way for the UI editor to recognize that, in fact, it is a panel.

## Writing fast programs

Comparing to apps, writing fast code is an entirely different matter. Here, you can't use JS (because it's deadly slow), and any abstraction mechanism you use must face a question: does it introduce run-time overhead? Below briefly explains the zero-overhead programming tricks in Jacy, i.e., the program structures that can be completely eliminated by the compiler.

### Do not use objects

This is a common misconception of C++ programmers. It's indeed true that carefully-written object-oriented code *can* have zero-overhead. However, this kind of things is very fragile and most C++ programmers fail miserably at the “carefully-written” part. The most notorious example is the STL. For instance, everyone uses `std::vector`, which can be more or less understood as a struct holding a pointer. One may imagine that, since `std::vector` is so commonly used, the compiler would have optimized that. But no, if you look at the disassembly, most often you will find at least two memory loads for each `vector[int]`. The reason is simple: vectors are resize-able and more often than not, the compiler would fail to find out that, in fact, you would never resize it. Making sure a vector is optimized requires an understanding of how your compiler works, knowing what program structures must be avoided, *and* not slipping even a single time in the entire program. The easier way is to just use a pointer for performance-critical code.

The situation is the same in Jacy. The compiler does try to optimize your arrays, but unless you're really careful, it will fail more than it succeeds. When performance is critical, take the `.d` member of the array (which is the pointer, in `iptr`) and use `__memory`.

Of course, more important is that you don't create classes for little things. You're better off, in terms of both programming effort and performance, using structs instead. Below are two common not-classes:
* mathematical values like a complex number, or a 3x3 matrix. 
* transient meta-data associated with other things, like the 2D coordinates of a positioned bitmap, or the image position corresponding to a mouse click. Such values are structs since references to them rarely makes sense

### Templates and inlines

Templates and inlines are an optimizer's best friend in C++. So Jacy provides them too. They have guaranteed zero-overhead: calling an inline function has absolutely no cost (beyond executing the function body), and a template function isn't any more costly than a normal function (in contrast to function pointers or C++ virtual functions, which involves an expensive indirect call and maybe some member fetches). 

Jacy template functions are defined by simply omitting the parameter type:

	//todo: example

using `const` as the type

!? Jacy inline power

*Do not bother defining a general template class for vectors or matrices.* Most likely, you'll end up using only one instance of it (or two), and you'll have to specialize a good half of methods for optimal performance (e.g. the SVD code is completely different for 2x2 and 3x3).

### Tuples and inline-structs

crange

### Compiler optimization

SCCP
strong / weak (screw-upable things like vector to pointer)
