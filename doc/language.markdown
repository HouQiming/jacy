# The Jacy Language Reference Manual

## Quick start

### Key properties

* Cross-platform development on Windows, Mac, Linux, Android, and iOS
* Fast enough for implementing numerical algorithms
* Native language with automatic memory management
* Bridges Javascript and C

### Using the compiler

!?

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

A slightly more complex example is available at *app_example/app.jc*.

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

Provide a `forEach` method, and `for in` is ready to use. You can alternatively write it as `foreach in`.

Jacy also provide C++-like maps in an intuitive syntax:

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

This section deals with features bending on the “application” side.

### Imports and modules

### Javascript-JC-C interface

__pointer
polymorphism

## Writing fast programs

### Templates and inlines

### Tuples and inline-structs

crange

### Compiler algorithm explained
SCCP
