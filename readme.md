# The JC language

Crunching numbers on the mobile? Tired of weird memory bugs in C? JC (read "Jacy") is developed just for things like this. Main features:

* Javascript-inspired language with memory management and functional programming
* Compiles to C and easily interops with C libraries
* Convenient Javascript embedding using [duktape](http://duktape.org/)
* [stb](https://github.com/nothings/stb)+SDL cross-platform GUI for fast prototyping
* Comes with a miniature build system supporting Windows, Linux, Android, Mac OS X (need some porting for now), and iOS

## To build

On Windows, run: 

	bootstrap.bat

With a Windows debug build in place, build the compiler for other platforms using:

	2 --arch=something --build=release main.jc

And build the build system using:

	2 --arch=something --build=release test/pmjs.jc

## To use

See [doc/language.md](doc/language.md)

## To do

Document the SDL stuff

Commit the SDL patch for Win32 IME handling somewhere

Provide a Unix-based bootstrap compiler
