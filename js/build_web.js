var onlydir=function(sdir,serr){
	var files=ls(sdir)
	if(!files.length){
		die("can't find "+serr+'\n')
	}else{
		return files[0].replace(/[\\]/g,"/")
	}
};

var BLOCK_SIZE=1048576;
var SplitAndUpdate=function(fntar,fnsrc){
	var data=new Buffer(ReadFileBuffer(fnsrc));
	var sz=data.length;
	var n=0;
	for(var i=0;i<sz;i+=BLOCK_SIZE){
		var szi=Math.min(sz-i,BLOCK_SIZE);
		CreateFile(fntar+(i/BLOCK_SIZE|0).toString()+'.bin',data.slice(i,i+szi))
		n++;
	}
	return n;
};

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
	var s_emcc=(g_current_arch=='win32'||g_current_arch=='win64'?"emcc.bat":"emcc");
	var smakefile_array=["CC = ",s_emcc,"\nLD = ",s_emcc,"\nCFLAGS0 = -DWEB\n"];
	if(g_build!="debug"){
		smakefile_array.push('LDFLAGS = -O2 -Os --memory-init-file 1 \n','CFLAGS1= --memory-init-file 1 -O2 -Os -fno-exceptions -fno-rtti -fno-unwind-tables -fno-strict-aliasing -w -DPM_RELEASE -s WASM=1 ');
	}else{
		smakefile_array.push('LDFLAGS = --memory-init-file 1 \n','CFLAGS1= --memory-init-file 1 -g2 -fno-exceptions -fno-rtti -fno-unwind-tables -fno-strict-aliasing -w -s WASM=1 ');
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
	smakefile_array.push('\nCXXFLAGS = ');
	if(g_json.cxxflags){
		for(var i=0;i<g_json.cxxflags.length;i++){
			var smain=g_json.cxxflags[i]
			smakefile_array.push(" "+smain);
		}
	}
	var s_emcc_output,is_default_build;
	if(!g_json.output_file){
		if(g_json.is_library){
			s_emcc_output=g_main_name+".bc";
		}else if(g_json.enable_web_assembly){
			s_emcc_output=g_main_name+".wasm";
		}else{
			s_emcc_output=g_main_name+".js";
		}
		is_default_build=1;
	}else{
		s_emcc_output=RemovePath(g_json.output_file[0]);
	}
	smakefile_array.push("\n"+s_emcc_output+":");
	for(var i=0;i<c_files.length;i++){
		var smain=RemoveExtension(c_files[i])
		smakefile_array.push(" "+smain+".o");
	}
	smakefile_array.push("\n\t$(LD) $(LDFLAGS) -o $@")
	if(g_json.exported_functions){
		smakefile_array.push(' -s EXPORTED_FUNCTIONS="[\'_main\',\'_');
		for(var i=0;i<g_json.exported_functions.length;i++){
			smakefile_array.push(g_json.exported_functions[i])
			smakefile_array.push("','_")
		}
		smakefile_array.pop()
		smakefile_array.push('\']"');
	}
	for(var i=0;i<c_files.length;i++){
		var smain=RemoveExtension(c_files[i])
		smakefile_array.push(" "+smain+".o");
	}
	if(g_lib_files){
		for(var i=0;i<g_lib_files.length;i++){
			smakefile_array.push(" ",g_lib_files[i]);
		}
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
		if(GetExtension(scfile)!=='c'){
			smakefile_array.push("\n"+smain+".o: "+scfile+"\n\t$(CC) $(CFLAGS0) $(CFLAGS1) $(CXXFLAGS) -c $< -o $@\n")
		}else{
			smakefile_array.push("\n"+smain+".o: "+scfile+"\n\t$(CC) $(CFLAGS0) $(CFLAGS1) -c $< -o $@\n")
		}
	}
	CreateIfDifferent(g_work_dir+"/upload/Makefile",smakefile_array.join(""))
	//////////////////////////
	var s_qualified_emcc_output;
	if(!g_json.output_file){
		s_qualified_emcc_output=g_bin_dir+"/"+s_emcc_output;
	}else{
		s_qualified_emcc_output=g_json.output_file
	}
	//var s_bat=g_config.EMSCRIPTEN_PATH+(g_current_arch=='win32'||g_current_arch=='win64'?"/emsdk.bat":"/emsdk");
	//if(g_current_arch=='win32'||g_current_arch=='win64'){
	//	s_bat=s_bat.replace(/[/]/g,'\\');
	//}
	//their bat is broken, force re-activate
	var s_path_clang=onlydir(g_config.EMSCRIPTEN_PATH+"/clang/*","clang")
	var s_path_nodejs=onlydir(g_config.EMSCRIPTEN_PATH+"/node/*","nodejs")
	var s_path_python=onlydir(g_config.EMSCRIPTEN_PATH+"/python/*","python")
	var s_path_emcc=onlydir(g_config.EMSCRIPTEN_PATH+"/emscripten/*","emcc")
	var cmd_make=[];
	if(g_current_arch=='win32'||g_current_arch=='win64'){
		cmd_make.push("set")
		cmd_make.push(["PATH=%PATH%",g_config.EMSCRIPTEN_PATH,s_path_clang,s_path_nodejs+"/bin",s_path_python,s_path_emcc].join(';').replace(/[/]/g,'\\'))
		cmd_make.push("&&")
		var s_bat=g_config.EMSCRIPTEN_PATH+"/emsdk.bat";
		cmd_make.push(s_bat,'construct_env','&&')
	}else{
		cmd_make.push(["PATH=${PATH}",g_config.EMSCRIPTEN_PATH,s_path_clang,s_path_nodejs+"/bin",s_path_python,s_path_emcc].join(';'))
		cmd_make.push(["EM_CONFIG=~/.emscripten"].join(''))
		cmd_make.push(["EMSCRIPTEN=",g_config.EMSCRIPTEN_PATH])
	}
	cmd_make.push("make","-C",g_work_dir+"/upload");
	//var ret=shell([s_bat,'activate','latest','&&',"make","-C",g_work_dir+"/upload"]);
	var ret=shell(cmd_make);
	if(ret!=0){
		throw new Error("make returned an error code of @1".replace("@1",ret.toString()));
	}
	if(g_json.is_library){
		UpdateTo(s_qualified_emcc_output,g_work_dir+"/upload/"+s_emcc_output);
	}else{
		var n_js_blocks=SplitAndUpdate(s_qualified_emcc_output,g_work_dir+"/upload/"+s_emcc_output);
		//UpdateTo(s_qualified_emcc_output,g_work_dir+"/upload/"+s_emcc_output)
		//if(FileExists(g_work_dir+"/upload/"+s_emcc_output+".mem")){
		var n_mem_blocks=SplitAndUpdate(s_qualified_emcc_output+".mem",g_work_dir+"/upload/"+s_emcc_output+".mem")
		//}
		if(is_default_build){
			var fn_shim=SearchForFile("shim.html");
			var s_prolog=(g_json.shim_prolog||[]);
			var s_shim=ReadFile(fn_shim).
				replace(/[$]PROJECT_NAME/g,g_main_name).
				replace(/[$]PROLOG/,s_prolog.join('')).
				replace(/[$]BLOCK_COUNTS/g,['{js:',n_js_blocks.toString(),',mem:',n_mem_blocks.toString(),'}'].join('')).
				replace(/[$]EXTRA_JS/g,(g_json.shim_extra_js||[]).join(''));
			CreateIfDifferent(RemoveExtension(s_qualified_emcc_output)+".html",s_shim);
		}
	}
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
