VC={};
VC.Detect=function(){
	if(VC.compiler_path)return;
	var testbat=function(spath0){
		var spath=ExpandEnvironmentStrings(spath0);
		if(FileExists(spath+"/vsvars32.bat"))return spath;
		return 0;
	}
	VC.compiler_path=(testbat("%VS110COMNTOOLS%")||testbat("%VS100COMNTOOLS%")||testbat("%VS90COMNTOOLS%")||testbat("%VS80COMNTOOLS%"));
	if(!VC.compiler_path){return 0;}
	var compiler_path=VC.compiler_path;
	var sbatname=compiler_path+'vsvars32.bat';
	if(g_is64){
		sbatname=compiler_path+'\\..\\..\\vc\\bin\\x86_amd64\\vcvarsx86_amd64.bat';
		if(!FileExists(sbatname)){
			sbatname=compiler_path+'\\..\\..\\vc\\bin\\amd64\\vcvars64.bat';
		}
	}
	VC.sbatname=sbatname
	return VC
};
VC.Compile=function(fnsrc,soutput){
	if(IsNewerThan(soutput,fnsrc)){return -1;}
	var compiler_path=VC.compiler_path;
	var sbatname=VC.sbatname;
	var sopt0=" "
	var sopt0=sopt0+" /c /Zi"
	if(g_build!="debug"){
		sopt0=sopt0+" /DNDEBUG /Gm- /GS- /Gd /Ot /Ob1 /Oy /Gy /GF /Ox /O2";
	}
	sopt0=sopt0+" /D_HAS_ITERATOR_DEBUGGING=0 /D_SECURE_SCL=0 /D_SCL_SECURE_NO_WARNINGS /MT /DPM_C_MODE /DNEED_MAIN_WRAPPING"
	if(g_build!="debug"){
		sopt0=sopt0+" /DPM_RELEASE";
	}
	if(!g_is64){
		sopt0=sopt0+" /arch:SSE2";
	}
	for(var i=0;i<g_search_paths.length;i++){
		var s_include_path=g_search_paths[i]+"/include";
		if(DirExists(s_include_path)){
			sopt0=sopt0+' /I "'+s_include_path+'"';
		}
	}
	if(g_json.c_include_paths){
		for(var i=0;i<g_json.c_include_paths.length;i++){
			var s_include_path=g_json.c_include_paths[i];
			if(DirExists(s_include_path)){
				sopt0=sopt0+' /I "'+s_include_path+'"';
			}
		}
	}
	if(g_json.cflags){
		for(var i=0;i<g_json.cflags.length;i++){
			var smain=g_json.cflags[i]
			sopt0=sopt0+(" "+smain);
		}
	}
	/////////////////
	sopt0=sopt0+' /Fo"'+soutput+'"';
	var scmd='@echo off\ncall "'+sbatname+'"\ncl /nologo '+sopt0+' "'+fnsrc+'"\n';
	var scallcl=g_work_dir+"/callcl.bat";
	if(!CreateFile(scallcl,scmd)){
		throw new Error("can't create callcl.bat");
	}
	var ret=shell([scallcl]);
	if(!!ret){
		throw new Error("cl returned an error code '@1'".replace("@1",ret.toString()));
	}
	return 1
}
VC.Link=function(fnlist,soutput){
	var compiler_path=VC.compiler_path
	var sbatname=VC.sbatname
	var sopt1=" ";
	if(g_is64){
		sopt1=sopt1+" /MACHINE:x64";
	}
	var subsystem=(g_json.subsystem||["console"])[0].toLowerCase();
	if(subsystem=="windows"&&g_build!="debug"){
		sopt1=sopt1+" /SUBSYSTEM:WINDOWS";
	}
	if(subsystem=="dll"){
		sopt1=sopt1+" /DLL";
	}
	if(g_json.ldflags){
		for(var i=0;i<g_json.ldflags.length;i++){
			var smain=g_json.ldflags[i]
			sopt1=sopt1+(" "+smain);
		}
	}
	var scmd='@echo off\ncall "'+sbatname+'"\nlink /DEBUG /OUT:"'+soutput+'" /NOLOGO /OPT:REF /OPT:ICF /DYNAMICBASE /NXCOMPAT /ERRORREPORT:PROMPT /LARGEADDRESSAWARE '+sopt1+' '+fnlist;
	var scallcl=g_work_dir+"/calllink.bat";
	if(!CreateFile(scallcl,scmd)){
		throw new Error("can't create calllink.bat");
	}
	var ret=shell([scallcl]);
	if(!!ret){
		throw new Error("link returned an error code '@1'".replace("@1",ret.toString()));
	}
}

//could have multiple targets here
g_action_handlers.make=function(){
	var s_final_output;
	if(!g_json.output_file){
		s_final_output=g_bin_dir+"/"+g_main_name+".exe";
	}else{
		s_final_output=g_json.output_file[0];
	}
	VC.Detect();
	//sync the header files
	var c_files=g_json.c_files;
	for(var i=0;i<g_json.h_files.length;i++){
		var fn=SearchForFile(g_json.h_files[i]);
		var fnh=g_work_dir+"/"+RemovePath(fn)
		UpdateTo(fnh,fn);
	}
	//sync and compile the C/C++ files
	var sopt1='';
	var need_link=0;
	for(var i=0;i<c_files.length;i++){
		//VC.Compile checks date
		var fn=SearchForFile(c_files[i]);
		var fnc=g_work_dir+"/"+RemovePath(fn)
		UpdateTo(fnc,fn);
		var fnobj=g_work_dir+"/"+GetMainFileName(fn)+".obj"
		var ret=VC.Compile(fnc,fnobj)
		if(ret>0||IsNewerThan(fnobj,s_final_output)){
			need_link=1;
		}
		sopt1=sopt1+' "'+fnobj+'"';
	}
	//icon and application name
	if(g_json.icon_file){
		var fn_icon=g_json.icon_file[0];
		var fn_res=g_work_dir+"/a.res";
		if(!IsNewerThan(fn_res,fn_icon)){
			CreateFile(g_work_dir+"/a.rc",'1 ICON "a.ico"\n')
			ResampleImage(fn_icon,g_work_dir+'/a.ico','ico');
			var scmd='@echo off\ncall "'+VC.sbatname+'"\nrc /fo "'+fn_res+'" "'+g_work_dir+"/a.rc"+'"'
			var scallrc=g_work_dir+"/callrc.bat";
			if(!CreateFile(scallrc,scmd)){
				throw new Error("can't create callrc.bat")
			}
			var ret=shell([scallrc]);
			if(!!ret){
				throw new Error("rc returned an error code '@1'".replace("@1",ret.toString()));
			}
		}
		sopt1=sopt1+' "'+fn_res+'"'
	}
	if(g_json.lib_files){
		for(var i=0;i<g_json.lib_files.length;i++){
			sopt1=sopt1+' "'+SearchForFile(g_json.lib_files[i])+'"';
		}
	}
	if(need_link||!FileExists(s_final_output)){
		VC.Link(sopt1,s_final_output);
	}
	if(g_json.dll_files){
		//copy the dll dependency
		for(var i=0;i<g_json.dll_files.length;i++){
			var fn=SearchForFile(g_json.dll_files[i]);
			var star=g_bin_dir+"/"+RemovePath(fn)
			UpdateTo(star,fn);
		}
	}
	return 1;
}

g_action_handlers.run=function(){
	var s_final_output;
	if(!g_json.output_file){
		s_final_output=g_bin_dir+"/"+g_main_name+".exe";
	}else{
		s_final_output=g_json.output_file[0];
	}
	shell([s_final_output].concat(g_json.run_args||[]));
}
