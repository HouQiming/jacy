
var copydir=function(dir){
	shell(["rsync","-qrtpzu",dir,"../jc/"]);
};

var copyosslib=function(file){
	UpdateTo("../jc/osslib/"+file,"osslib/"+file)
};

var clean=function(dir){
	shell(["rm","-rf",dir+"/pm_tmp"])
	shell(["rm","-rf",dir+"/bin"])
};

(function(){
	//build the stuff
	print("*** Release-building the compiler ***")
	shell(["bin/win32/main","--build=release","main.jc"])
	print("*** Release-building the UI editor ***")
	shell(["bin/win32/main","--build=release","mo/mo.jc"])
	//the binaries
	mkdir("../jc/bin/win32_release")
	UpdateTo("../jc/bin/win32_release/jc.exe","bin/win32_release/main.exe")
	UpdateTo("../jc/bin/win32_release/pmjs.exe","test/bin/win32_release/pmjs.exe")
	UpdateTo("../jc/bin/win32_release/mo.exe","mo/bin/win32_release/mo.exe")
	UpdateTo("../jc/bin/win32_release/res.zip","mo/bin/win32_release/res.zip")
	UpdateTo("../jc/bin/win32_release/sdl2.dll","mo/bin/win32_release/sdl2.dll")
	//the units
	copydir("units")
	copydir("js")
	copydir("wrapper")
	copydir("doc")
	//test programs
	shell(["rm","-rf","../jc/test"])
	mkdir("../jc/test")
	clean("test/app_example")
	clean("test/app_example1")
	shell(["rsync","-qrtpzu","test/*.jc","../jc/test/"]);
	shell(["rsync","-qrtpzu","test/app_example","../jc/test/"]);
	shell(["rsync","-qrtpzu","test/app_example1","../jc/test/"]);
	//development environments
	mkdir("../jc/osslib/lib/x86")
	mkdir("../jc/osslib/lib/x64")
	copyosslib("lib/x86/sdl2.lib")
	copyosslib("lib/x86/sdl2.dll")
	copyosslib("lib/x64/sdl2.lib")
	copyosslib("lib/x64/sdl2.dll")
	shell(["rsync","-rtpzuv",'osslib/ios',"../jc/osslib/"]);
	shell(["rsync","-rtpzuv",'osslib/include',"../jc/osslib/"]);
})()
