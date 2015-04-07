/*
don't build it, use the framework
/Users/houqiming/pmenv/SDL2-2.0.3/build/.libs
remove -fmodules
*/
var g_need_ssh_for_mac=(g_current_arch!="mac");
var MAC={}
MAC.c_file_list=[]
g_is64=1;

MAC.CopyToUpload=function(fn0){
	var fn=fn0
	var spackage_dir=g_work_dir+"/upload"
	var star=spackage_dir+"/"+RemovePath(fn0)
	UpdateTo(star,fn)
	MAC.c_file_list.push(RemovePath(fn0))
}

g_action_handlers.make=function(){
	var ssh_addr
	if(g_need_ssh_for_mac){
		ssh_addr=GetServerSSH('mac');
	}
	var dir_pmenv=g_root+"/mac/pmenv"
	mkdir(g_work_dir+"/upload/")
	if(!FileExists(g_work_dir+"/SDL_setup")){
		//the SDL skeleton project
		shell(["cp","-r",g_config.MAC_SKELETON_PATH+"/*",g_work_dir+"/upload/"])
		var s_text=ReadFile(g_work_dir+"/upload/__SDL_TEMPLATE_PROJECT__.xcodeproj/project.pbxproj")
		CreateFile(g_work_dir+"/upload/__SDL_TEMPLATE_PROJECT__.xcodeproj/project.pbxproj",s_text.replace(new RegExp("__SDL_TEMPLATE_PROJECT__","g"),g_main_name))
		//s_text=ReadFile(g_work_dir+"/upload/Info.plist")
		//CreateFile(g_work_dir+"/upload/Info.plist",s_text.replace(new RegExp("com.yourcompany.*\\}","g"),"com.spap."+g_main_name.replace(new RegExp("_","g"),"")))
		shell(["mv",g_work_dir+'/upload/__SDL_TEMPLATE_PROJECT__.xcodeproj',g_work_dir+'/upload/'+g_main_name+'.xcodeproj'])
		CreateFile(g_work_dir+"/SDL_setup","1")
	}
	var sbuildtmp;
	if(g_need_ssh_for_mac){
		if(!FileExists(g_work_dir+"/buildtmp_ready")){
			sbuildtmp=SHA1(g_work_dir,8)
			CreateFile(g_work_dir+"/buildtmp_ready",sbuildtmp)
			mkdir(g_work_dir+"/upload/")
			envssh('mac',
				'echo "----cleanup----";'+
				'chmod -R 777 ~/_buildtmp/'+sbuildtmp+';'+
				'rm -rf ~/_buildtmp/'+sbuildtmp+';'+
				'mkdir -p ~/_buildtmp/'+sbuildtmp+';'+
				'exit')
		}
		sbuildtmp=ReadFile(g_work_dir+"/buildtmp_ready")
	}
	var CopyFiles=function(file_set){
		if(!file_set){return;}
		for(var i=0;i<file_set.length;i++){
			var fn=SearchForFile(file_set[i])
			MAC.CopyToUpload(fn)
		}
	}
	CopyFiles(g_json.h_files)
	CopyFiles(g_json.c_files)
	CopyFiles(g_json.objc_files)
	CopyFiles(g_json.lib_files)
	MAC.CopyToUpload(g_work_dir+"\\reszip.bundle")
	//icons
	if(g_json.icon_file){
		var fn_icon=SearchForFile(g_json.icon_file[0]);
		var fntouch=g_work_dir+"/ic_launcher.png._touch"
		if(IsNewerThan(fn_icon,fntouch)){
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon.png',57,57)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon-72.png',72,72)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon-Small.png',29,29)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon-Small-50.png',50,50)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon@2x.png',114,114)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon-72@2x.png',144,144)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon-Small@2x.png',58,58)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon-Small-50@2x.png',100,100)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon-76.png',76,76)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon-60.png',60,60)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon-76@2x.png',152,152)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon-60@2x.png',120,120)
			CreateFile(fntouch,fn_icon)
		}
	}
	if(g_json.default_screen_file){
		var fn_icon=SearchForFile(g_json.default_screen_file[0])
		var fntouch=g_work_dir+"/Default.png._touch"
		if(IsNewerThan(fn_icon,fntouch)){
			ResampleImage(fn_icon,g_work_dir+'/upload/Default.png',320,480)
			ResampleImage(fn_icon,g_work_dir+'/upload/Default-568h.png',320,568)
			CreateFile(fntouch,fn_icon)
		}
	}
	var spython=[]
	spython.push('from modxproj import XcodeProject\n')
	spython.push('project = XcodeProject.Load("'+g_main_name+'.xcodeproj/project.pbxproj")\n')
	spython.push('fn="libSDL2.a"\n')
	spython.push('if len(project.get_files_by_name(fn))<=0:\n')
	spython.push('	project.add_file(fn)\n')
	spython.push('fn="libSDL2main.a"\n')
	spython.push('if len(project.get_files_by_name(fn))<=0:\n')
	spython.push('	project.add_file(fn)\n')
	//we don't have to add main.c
	for(var i=0;MAC.c_file_list[i];i++){
		var fn=MAC.c_file_list[i]
		spython.push('fn="'+fn+'"\n')
		spython.push('if len(project.get_files_by_name(fn))<=0:\n')
		spython.push('	project.add_file(fn)\n')
	}
	if(g_json.mac_frameworks){
		for(var i=0;g_json.mac_frameworks[i];i++){
			spython.push('project.add_file("'+g_json.mac_frameworks[i]+'",tree="SDKROOT")\n')
		}
	}
	spython.push('if project.modified:\n')
	spython.push('	project.save()\n')
	CreateIfDifferent(g_work_dir+"/upload/tmp.py",spython.join(""))
	var sshell=[]
	if(g_need_ssh_for_mac){
		rsync(g_work_dir+'/upload',ssh_addr+':~/_buildtmp/'+sbuildtmp)
		sshell.push('echo "----updating project----";')
		sshell.push('cd ~/_buildtmp/'+sbuildtmp+';')
		sshell.push('chmod -R 700 ~/_buildtmp/'+sbuildtmp+'/*;')
	}else{
		sshell.push('#!/bin/sh\n')
		sshell.push('cd '+g_work_dir+'/upload;')
	}
	//the re-add approach has guid issues
	sshell.push('python ./tmp.py;')
	sshell.push('echo "----building----";')
	if(g_build!="debug"){
		//sshell.push('security list-keychains -s login.keychain;')
		//sshell.push('security default-keychain -s login.keychain;')
		//todo: prompt for pwd instead? could find a local-only solution
		sshell.push('security unlock-keychain -p \'\' login.keychain;')
	}
	if(g_build!="debug"){
		sshell.push('xcodebuild -sdk macosx -configuration Release build OTHER_CFLAGS=\'${inherited} -std=c99 -w -I${HOME}/pmenv/include\';')
	}else{
		sshell.push('xcodebuild -sdk macosx -configuration Debug build OTHER_CFLAGS=\'${inherited} -O0 -std=c99 -w -I${HOME}/pmenv/include\';')
	}
	var sdirname
	if(g_build!="debug"){
		sdirname="Release"
	}else{
		sdirname="Debug"
	}
	if(g_need_ssh_for_mac){
		sshell.push('mkdir -p download; cd build/'+sdirname+'/; strip '+g_main_name+'.app/Contents/MacOS/'+g_main_name+'; tar -czf '+g_main_name+'.tar.gz '+g_main_name+'.app; mv '+g_main_name+'.tar.gz ../../download/;')
		sshell.push('exit')
		envssh('mac',sshell.join(""))
		rsync(ssh_addr+':~/_buildtmp/'+sbuildtmp+'/download',g_bin_dir)
	}else{
		sshell.push('cd build/'+sdirname+'/; strip '+g_main_name+'.app/Contents/MacOS/'+g_main_name+'; cp -r '+g_main_name+'.app ../../../../../../bin/'+g_relative_dir_name+'/;')
		CreateFile(g_work_dir+"/build_local.sh",sshell.join(""))
		shell([g_work_dir+"/build_local.sh"])
	}
	return 1
}

g_action_handlers.run=function(sdir_target){
	var s_target_dir;
	if(g_need_ssh_for_mac){
		var sbuildtmp=ReadFile(g_work_dir+"/buildtmp_ready")
		if(!sbuildtmp){
			die("the project hasn't been built yet")
		}
		s_target_dir='~/_buildtmp/'+sbuildtmp
	}else{
		s_target_dir=g_bin_dir
	}
	var sdirname
	if(g_build!="debug"){
		sdirname="Release"
	}else{
		sdirname="Debug"
	}
	var sshell=s_target_dir+'/build/'+sdirname+'/'+g_main_name+'.app/Contents/MacOS/'+g_main_name
	if(g_need_ssh_for_mac){
		envssh('mac',sshell+';exit')
	}else{
		shell([sshell])
	}
}
