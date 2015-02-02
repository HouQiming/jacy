var g_need_ssh_for_linux=(g_current_arch!="linux32"&&g_current_arch!="linux64");

g_action_handlers.make=function(){
	var ssh_addr,ssh_port;
	if(g_need_ssh_for_linux){
		if(g_json.verbose){print("building for Linux on a @1 machine, doing ssh".replace("@1",g_current_arch));}
		ssh_addr=GetServerSSH('linux');
		ssh_port=GetPortSSH('linux');
	}
	mkdir(g_work_dir+"/upload/");
	if(g_need_ssh_for_linux&&!FileExists(g_work_dir+"/buildtmp_ready")){
		var sbuildtmp=SHA1(g_work_dir,8);
		CreateFile(g_work_dir+"/buildtmp_ready",sbuildtmp);
		envssh('linux',
			'echo "----cleanup----";'+
			'chmod -R 777 ~/_buildtmp/'+sbuildtmp+';'+
			'rm -rf ~/_buildtmp/'+sbuildtmp+';'+
			'mkdir -p ~/_buildtmp/'+sbuildtmp+';'+
			'exit');
	}
	var sbuildtmp=ReadFile(g_work_dir+"/buildtmp_ready");
	CopyToWorkDir(g_json.c_files,"upload/")
	CopyToWorkDir(g_json.h_files,"upload/")
	CopyToWorkDir(g_json.lib_files,"upload/")
	if(FileExists(g_work_dir+"/res.zip")){
		UpdateTo(g_work_dir+"/upload/res.zip",g_work_dir+"/res.zip")
	}
	if(g_json.icon_file){
		var fn_icon=g_json.icon_file[0];
		UpdateTo(g_work_dir+"/upload/ic_launcher.png",fn_icon)
	}
	////////////////////////
	//create a makefile
	//-ffast-math
	var smakefile_array=["CC = gcc\nLD = gcc\nCFLAGS0 = -I$(HOME)/pmenv/SDL2-2.0.3/include -I$(HOME)/pmenv/include -DLINUX\nLDFLAGS = -s -L$(HOME)/pmenv/SDL2-2.0.3/build/.libs -L$(HOME)/pmenv/SDL2-2.0.3/build"];
	if(g_build!="debug"){
		smakefile_array.push('\nCFLAGS1= -O3 -fno-var-tracking-assignments -fno-exceptions -fno-rtti -fno-unwind-tables -fno-strict-aliasing -w -static-libgcc -DPM_RELEASE ');
	}else{
		smakefile_array.push('\nCFLAGS1= -fno-var-tracking-assignments -fno-exceptions -fno-rtti -fno-unwind-tables -fno-strict-aliasing -w -static-libgcc ');
	}
	if(g_json.c_include_paths){
		for(var i=0;i<g_json.c_include_paths.length;i++){
			var s_include_path=g_json.c_include_paths[i];
			if(DirExists(s_include_path)){
				smakefile_array.push(' "-I'+s_include_path+'"');
			}
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
		s_linux_output=g_main_name;
	}else{
		s_linux_output=RemovePath(g_json.output_file[0]);
	}
	smakefile_array.push("\n"+s_linux_output+":");
	for(var i=0;i<g_json.c_files.length;i++){
		var scfile=RemovePath(g_json.c_files[i])
		var smain=GetMainFileName(scfile)
		smakefile_array.push(" "+smain+".o");
	}
	smakefile_array.push("\n\t$(LD) $(LDFLAGS) -o $@")
	for(var i=0;i<g_json.c_files.length;i++){
		var scfile=RemovePath(g_json.c_files[i])
		var smain=GetMainFileName(scfile)
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
	for(var i=0;i<g_json.c_files.length;i++){
		var scfile=RemovePath(g_json.c_files[i]);
		var smain=GetMainFileName(scfile)
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
	if(g_need_ssh_for_linux){
		if(g_json.verbose){print("=== rsyncing the project directory")}
		rsync(g_work_dir+'/upload',ssh_addr+':_buildtmp/'+sbuildtmp,ssh_port)
		var sshell_array=[];
		sshell_array.push('cd ~/_buildtmp/'+sbuildtmp+';')
		sshell_array.push('make;')
		if(FileExists(g_work_dir+"/upload/ic_launcher.png")){
			sshell_array.push('mkdir -p ~/.icons/hicolor/48x48/apps;cp ic_launcher.png ~/.icons/hicolor/48x48/apps/'+s_linux_output+".png;")
		}
		sshell_array.push("exit")
		if(g_json.verbose){print("=== making on remote machine")}
		envssh('linux',sshell_array.join(""))
		shell(["rm",s_qualified_linux_output]);
		shell(["scp","-P"+ssh_port,ssh_addr+':_buildtmp/'+sbuildtmp+'/'+s_linux_output,s_qualified_linux_output])
		if(!FileExists(s_qualified_linux_output)){
			throw new Error("make failed to produce an output on the remote side");
		}
	}else{
		var ret=shell(["make","-C",g_work_dir+"/upload"]);
		if(ret!=0){
			throw new Error("make returned an error code of @1".replace("@1",ret.toString()));
		}
		UpdateTo(s_qualified_linux_output,g_work_dir+"/upload/"+s_linux_output)
	}
	return 1;
};

g_action_handlers.run=function(sdir_target){
	if(g_need_ssh_for_linux){
		print("running Linux program on a @1 machine, doing ssh".replace("@1",g_current_arch));
		var sbuildtmp=ReadFile(g_work_dir+"/buildtmp_ready")
		if(!sbuildtmp){
			throw new Error("error> the project hasn't been built yet")
		}
		var s_linux_output;
		if(!g_json.output_file){
			s_linux_output=g_main_name;
		}else{
			s_linux_output=RemovePath(g_json.output_file[0]);
		}
		var sshell_array=[];
		sshell_array.push('~/_buildtmp/'+sbuildtmp+'/'+s_linux_output+' ')
		sshell_array.push((g_json.run_args||[]).join(" "))
		sshell_array.push(';exit')
		envssh('linux',sshell_array.join(""))
	}else{
		var s_final_output;
		if(!g_json.output_file){
			s_final_output=g_bin_dir+"/"+g_main_name;
		}else{
			s_final_output=g_json.output_file[0];
		}
		shell([s_final_output].concat(g_json.run_args||[]));
	}
};
