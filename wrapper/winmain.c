/*
<package name="SDL Image">
	<target n="win32">
	</target>
	<target n="win64">
	</target>
	<var n="subsystem" v="windows"/>
</package>
*/

#if defined(PM_RELEASE)
extern int main(void);
int __stdcall WinMain(void* a,void* b,void* c,int d){
	return main();
}
#endif
