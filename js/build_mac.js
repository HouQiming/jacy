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
	//var CopyFiles=function(file_set){
	//	if(!file_set){return;}
	//	for(var i=0;i<file_set.length;i++){
	//		var fn=SearchForFile(file_set[i])
	//		MAC.CopyToUpload(fn)
	//	}
	//}
	//CopyFiles(g_json.h_files)
	//CopyFiles(g_json.c_files)
	//CopyFiles(g_json.objc_files)
	//CopyFiles(g_json.lib_files)
	var c_files=CreateProjectForStandardFiles(g_work_dir+"/upload/")
	for(var i=0;i<c_files.length;i++){
		MAC.c_file_list.push(c_files[i])
	}
	if(g_lib_files){
		for(var i=0;i<g_lib_files.length;i++){
			MAC.c_file_list.push(g_lib_files[i])
		}
	}
	if(FileExists(g_work_dir+"/reszip.bundle")){
		MAC.CopyToUpload(g_work_dir+"/reszip.bundle")
	}
	//icons
	var need_icon_conversion=0;
	if(g_json.icon_file){
		var fn_icon=SearchForFile(g_json.icon_file[0]);
		var fntouch=g_work_dir+"/ic_launcher.png._touch"
		if(IsNewerThan(fn_icon,fntouch)){
			mkdir(g_work_dir+"/upload/Icon.iconset")
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon.iconset/icon_16x16.png',16,16)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon.iconset/icon_16x16@2x.png',16*2,16*2)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon.iconset/icon_32x32.png',32,32)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon.iconset/icon_32x32@2x.png',32*2,32*2)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon.iconset/icon_128x128.png',128,128)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon.iconset/icon_128x128@2x.png',128*2,128*2)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon.iconset/icon_256x256.png',256,256)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon.iconset/icon_256x256@2x.png',256*2,256*2)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon.iconset/icon_512x512.png',512,512)
			ResampleImage(fn_icon,g_work_dir+'/upload/Icon.iconset/icon_512x512@2x.png',512*2,512*2)
			CreateFile(fntouch,fn_icon)
			need_icon_conversion=1;
		}
		//MAC.c_file_list.push('Icon.iconset');
	}
	//coulddo: http://stackoverflow.com/questions/96882/how-do-i-create-a-nice-looking-dmg-for-mac-os-x-using-command-line-tools
	//if(g_json.default_screen_file){
	//	var fn_icon=SearchForFile(g_json.default_screen_file[0])
	//	var fntouch=g_work_dir+"/Default.png._touch"
	//	if(IsNewerThan(fn_icon,fntouch)){
	//		ResampleImage(fn_icon,g_work_dir+'/upload/Default.png',320,480)
	//		ResampleImage(fn_icon,g_work_dir+'/upload/Default-568h.png',320,568)
	//		CreateFile(fntouch,fn_icon)
	//	}
	//}
	var spython=[]
	spython.push('from modxproj import XcodeProject\n')
	spython.push('project = XcodeProject.Load("'+g_main_name+'.xcodeproj/project.pbxproj")\n')
	//we don't have to add main.c
	for(var i=0;MAC.c_file_list[i];i++){
		var fn=MAC.c_file_list[i]
		spython.push('fn0="'+RemovePath(fn)+'"\n')
		spython.push('fn="'+fn+'"\n')
		spython.push('if len(project.get_files_by_name(fn0))<=0:\n')
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
	if(g_json.icon_file){
		sshell.push('iconutil --convert icns Icon.iconset --output Icon.icns;')
	}
	sshell.push('echo "----building----";')
	if(g_build!="debug"){
		//sshell.push('security list-keychains -s login.keychain;')
		//sshell.push('security default-keychain -s login.keychain;')
		//todo: prompt for pwd instead? could find a local-only solution
		sshell.push('echo "--- Setting and unlocking keychains ---";')
		sshell.push('security default-keychain -s ios.keychain;')
		sshell.push('security unlock-keychain -p \'\' ios.keychain;')
	}
	if(g_build!="debug"){
		sshell.push('xcodebuild -sdk macosx -configuration Release build OTHER_CFLAGS=\'${inherited} -w -Isdl/include -Isdl/src \';')
	}else{
		sshell.push('xcodebuild -sdk macosx -configuration Debug build OTHER_CFLAGS=\'${inherited} -O0 -w -Isdl/include -Isdl/src \';')
	}
	var sdirname
	if(g_build!="debug"){
		sdirname="Release"
	}else{
		sdirname="Debug"
	}
	if(g_json.icon_file){
		sshell.push('mkdir -p build/',sdirname,'/',g_main_name,'.app/Contents/Resources;')
		sshell.push('cp Icon.icns build/',sdirname,'/',g_main_name,'.app/Contents/Resources/;')
	}
	if(g_need_ssh_for_mac){
		//hdiutil create -volname WhatYouWantTheDiskToBeNamed -srcfolder /path/to/the/folder/you/want/to/create -ov -format UDZO name.dmg
		sshell.push('mkdir -p download; cd build/',sdirname,'/; strip '+g_main_name,'.app/Contents/MacOS/',g_main_name,';')
		sshell.push('rm -rf dmg; mkdir -p dmg; cp -r ',g_main_name,'.app dmg/; ln -s /Applications dmg/Applications;')
		//sshell.push('; tar -czf '+g_main_name+'.tar.gz '+g_main_name+'.app; mv '+g_main_name+'.tar.gz ../../download/;')
		sshell.push('hdiutil create -volname ',g_main_name,' -srcfolder dmg/ -ov -format UDZO ',g_main_name,'.dmg; mv '+g_main_name+'.dmg ../../download/;')
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
