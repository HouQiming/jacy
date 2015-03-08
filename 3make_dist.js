
var copydir=function(dir){
	var files=ls(dir+"/*");
	mkdir("../jc/"+dir)
	for(var i=0;i<files.length;i++){
		UpdateTo("../jc/"+files[i],files[i])
	}
};

(function(){
	//the binaries
	mkdir("../jc/bin/win32_release")
	UpdateTo("../jc/bin/win32_release/jc.exe","bin/win32_release/main.exe")
	UpdateTo("../jc/bin/win32_release/pmjs.exe","test/bin/win32_release/pmjs.exe")
	//the units
	copydir("units")
	copydir("units/gui2d")
	copydir("js")
	copydir("wrapper")
	copydir("doc")
	//test programs
	shell(["rm","-rf","../jc/test"])
	mkdir("../jc/test")
	shell(["cp","test/*.jc","../jc/test/"])
	shell(["cp","-r","test/app_example","../jc/test/"])
	shell(["cp","-r","test/app_example1","../jc/test/"])
})()
