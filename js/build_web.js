
g_action_handlers.make=function(){
	mkdir(g_work_dir+"/upload/");
	var c_files=CreateProjectForStandardFiles(g_work_dir+"/upload/")
	if(FileExists(g_bin_dir+"/res.zip")){
		UpdateTo(g_work_dir+"/upload/res.zip",g_bin_dir+"/res.zip")
	}
	if(g_json.icon_file){
		var fn_icon=g_json.icon_file[0];
		UpdateTo(g_work_dir+"/upload/ic_launcher.png",fn_icon)
	}
	////////////////////////
	//create a makefile
	//-ffast-math
	//var smakefile_array=["CC = gcc\nLD = gcc\nCFLAGS0 = -I$(HOME)/pmenv/SDL2-2.0.3/include -I$(HOME)/pmenv/include -DLINUX\nLDFLAGS = -s -L$(HOME)/pmenv/SDL2-2.0.3/build/.libs -L$(HOME)/pmenv/SDL2-2.0.3/build"];
	var smakefile_array=["CC = emcc","\nLD = emcc\nCFLAGS0 = -DWEB\nLDFLAGS = "];
	if(g_build!="debug"){
		smakefile_array.push('\nCFLAGS1= -O2 -fno-exceptions -fno-rtti -fno-unwind-tables -fno-strict-aliasing -w -DPM_RELEASE ');
	}else{
		smakefile_array.push('\nCFLAGS1= -fno-exceptions -fno-rtti -fno-unwind-tables -fno-strict-aliasing -w ');
	}
	if(g_json.c_include_paths){
		for(var i=0;i<g_json.c_include_paths.length;i++){
			var s_include_path=g_json.c_include_paths[i];
			//if(DirExists(s_include_path)){
			smakefile_array.push(' "-I'+s_include_path+'"');
			//}
		}
	}
	if(g_json.cflags){
		for(var i=0;i<g_json.cflags.length;i++){
			var smain=g_json.cflags[i]
			smakefile_array.push(" "+smain);
		}
	}
	var s_linux_output;
	if(!g_json.output_file){
		s_linux_output=g_main_name+".html";
	}else{
		s_linux_output=RemovePath(g_json.output_file[0]);
	}
	smakefile_array.push("\n"+s_linux_output+":");
	for(var i=0;i<c_files.length;i++){
		var smain=RemoveExtension(c_files[i])
		smakefile_array.push(" "+smain+".o");
	}
	smakefile_array.push("\n\t$(LD) $(LDFLAGS) -o $@")
	for(var i=0;i<c_files.length;i++){
		var smain=RemoveExtension(c_files[i])
		smakefile_array.push(" "+smain+".o");
	}
	if(g_json.ldflags){
		for(var i=0;i<g_json.ldflags.length;i++){
			var smain=g_json.ldflags[i]
			smakefile_array.push(" "+smain);
		}
	}
	smakefile_array.push("\n")
	//////////////////////////
	for(var i=0;i<c_files.length;i++){
		var scfile=c_files[i];
		var smain=RemoveExtension(scfile)
		smakefile_array.push("\n"+smain+".o: "+scfile+"\n\t$(CC) $(CFLAGS0) $(CFLAGS1) -c $< -o $@\n")
	}
	CreateIfDifferent(g_work_dir+"/upload/Makefile",smakefile_array.join(""))
	//////////////////////////
	var s_qualified_linux_output;
	if(!g_json.output_file){
		s_qualified_linux_output=g_bin_dir+"/"+s_linux_output;
	}else{
		s_qualified_linux_output=g_json.output_file
	}
	var s_bat=(g_current_arch=='win32'||g_current_arch=='win64'?"emsdk.bat":"./emsdk");
	if(g_current_arch=='win32'||g_current_arch=='win64'){
		s_bat=s_bat.replace(/[/]/g,'\\');
	}
	//their bat is broken, force re-activate
	var ret=shell(['cd',g_config.EMSCRIPTEN_PATH,'&&',s_bat,'activate','latest','&&','cd',pwd(),'&&',"make","-C",g_work_dir+"/upload"]);
	if(ret!=0){
		throw new Error("make returned an error code of @1".replace("@1",ret.toString()));
	}
	UpdateTo(s_qualified_linux_output,g_work_dir+"/upload/"+s_linux_output)
	return 1;
};

g_action_handlers.run=function(sdir_target){
	//if(g_json.html_shim){
	//	return;
	//}
	var s_final_output;
	if(!g_json.output_file){
		s_final_output=g_bin_dir+"/"+g_main_name+".html";
	}else{
		s_final_output=g_json.output_file[0];
	}
	//shell([s_final_output].concat(g_json.run_args||[]));
	shell([g_current_arch=='win32'||g_current_arch=='win64'?'start':(g_current_arch=='linux32'||g_current_arch=='linux64'?'xdg-open':'open'),
		s_final_output]);
};
