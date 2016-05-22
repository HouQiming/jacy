//todo: 64-bit c generation
/*
PROVISIONING_PROFILE = "487F3EAC-05FB-4A2A-9EA0-31F1F35760EB";
"PROVISIONING_PROFILE[sdk=iphoneos*]" = "487F3EAC-05FB-4A2A-9EA0-31F1F35760EB";
to build SDL:
	cd ~/pmenv/SDL2-2.0.3
	rm -rf build
	build-scripts/iosbuild.sh
	
old
	cd ~/pmenv/SDL2-2.0.3/Xcode-iOS/SDL
	rm -rf build
	xcodebuild -sdk iphoneos -configuration Release -scheme='libSDL' build
	xcodebuild -sdk iphonesimulator -configuration Release -scheme='libSDL' build
	lipo -create build/Release-iphoneos/libSDL2.a build/Release-iphonesimulator/libSDL2.a -output build/libSDL2.a
to build freetype... we no longer need it
security add-generic-password -s Xcode:itunesconnect.apple.com -a LOGIN -w PASSWORD -U
build: CODE_SIGN_IDENTITY="iPhone Distribution:"
----
/usr/bin/xcrun -sdk iphoneos PackageApplication -v $FULL_PATH_TO_APP -o the-ipa --sign "iPhone Distribution:" --embed *.mobileprovision
codesign -s "Distribution" the-ipa
xcrun -sdk iphoneos Validation -online -upload -verbose "path to ipa"
*/
var g_need_ssh_for_mac=(g_current_arch!="mac");
var IOS=[]
IOS.c_file_list=[]

IOS.CopyToUpload=function(fn0){
	var fn=fn0
	var spackage_dir=g_work_dir+"/upload"
	var star=spackage_dir+"/"+RemovePath(fn0)
	UpdateTo(star,fn)
	IOS.c_file_list.push(RemovePath(fn0))
}

g_action_handlers.make=function(){
	var ssh_addr
	if(g_need_ssh_for_mac){
		ssh_addr=GetServerSSH('mac');
	}
	var dir_pmenv=g_root+"/mac/pmenv"
	//build the project dir locally
	mkdir(g_work_dir+"/upload/")
	if(!FileExists(g_work_dir+"/SDL_setup")){
		//the SDL skeleton project
		shell(["cp","-r",g_config.IOS_SKELETON_PATH+"/*",g_work_dir+"/upload/"])
		var s_text=ReadFile(g_work_dir+"/upload/___PROJECTNAME___.xcodeproj/project.pbxproj")
		CreateFile(g_work_dir+"/upload/___PROJECTNAME___.xcodeproj/project.pbxproj",s_text.replace(new RegExp("___PROJECTNAME___","g"),g_main_name))
		s_text=ReadFile(g_work_dir+"/upload/Info.plist")
		var s_display_name=(g_json.app_display_name&&g_json.app_display_name[0]||g_main_name);
		s_text=s_text.replace(/__display_name__/g,s_display_name);
		CreateFile(g_work_dir+"/upload/Info.plist",s_text.replace(new RegExp("com.yourcompany.*\\}","g"),"com.spap."+g_main_name.replace(new RegExp("_","g"),"")))
		shell(["mv",g_work_dir+'/upload/___PROJECTNAME___.xcodeproj',g_work_dir+'/upload/'+g_main_name+'.xcodeproj'])
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
	var fn_c_32=g_work_dir+"/s7main.c";
	var fn_c_64=g_work_dir+"/s7main64.c";
	var fn_c_bi=g_work_dir+"/s7main_bi.c";
	if(IsNewerThan(fn_c_32,fn_c_64)){
		//64-bit build
		var jc_cmdline=[g_root+"/bin/win32_release/jc.exe","--64","-a"+g_arch,"-b"+g_build,"-c","--c="+fn_c_64];
		for(var i=0;i<g_json.input_files.length;i++){
			jc_cmdline.push(g_json.input_files);
		}
		shell(jc_cmdline);
	}
	var got_original_main_c=0;
	for(var i=0;i<g_json.c_files.length;i++){
		if(g_base_dir+"/"+g_json.c_files[i]==fn_c_32){
			got_original_main_c=1;
			g_json.c_files[i]=fn_c_bi;
			break;
		}
	}
	if(!got_original_main_c){
		print(JSON.stringify(g_json.c_files),fn_c_32)
		throw new Error("cannot find s7main.c in c_files")
	}
	g_json.h_files.push(fn_c_64)
	g_json.h_files.push(fn_c_32)
	CreateIfDifferent(fn_c_bi,'#if __LP64__\n#include "s7main64.c"\n#else\n#include "s7main.c"\n#endif\n')
	//var CopyFiles=function(file_set){
	//	if(!file_set){return;}
	//	for(var i=0;i<file_set.length;i++){
	//		var fn=SearchForFile(file_set[i])
	//		IOS.CopyToUpload(fn)
	//	}
	//}
	//CopyFiles(g_json.h_files)
	//CopyFiles(g_json.c_files)
	//CopyFiles(g_json.objc_files)
	//CopyFiles(g_json.lib_files)
	var c_files=CreateProjectForStandardFiles(g_work_dir+"/upload/")
	for(var i=0;i<c_files.length;i++){
		IOS.c_file_list.push(c_files[i])
	}
	if(g_lib_files){
		for(var i=0;i<g_lib_files.length;i++){
			IOS.c_file_list.push(g_lib_files[i])
		}
	}
	if(FileExists(g_work_dir+"/reszip.bundle")){
		IOS.CopyToUpload(g_work_dir+"/reszip.bundle")
	}
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
	if(FileExists(g_base_dir+"/dist.mobileprovision")){
		UpdateTo(g_work_dir+'/upload/dist.mobileprovision',g_base_dir+"/dist.mobileprovision")
	}
	var spython=[]
	spython.push('from modxproj import XcodeProject\n')
	spython.push('project = XcodeProject.Load("'+g_main_name+'.xcodeproj/project.pbxproj")\n')
	//we don't have to add main.c
	//print(JSON.stringify(IOS.c_file_list))
	for(var i=0;IOS.c_file_list[i];i++){
		var fn=IOS.c_file_list[i]
		spython.push('fn0="'+RemovePath(fn)+'"\n')
		spython.push('fn="'+fn+'"\n')
		spython.push('if len(project.get_files_by_name(fn0))<=0:\n')
		spython.push('	project.add_file(fn)\n')
	}
	if(g_json.ios_frameworks){
		for(var i=0;g_json.ios_frameworks[i];i++){
			spython.push('project.add_file("'+g_json.ios_frameworks[i]+'",tree="SDKROOT")\n')
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
	if(g_build!="debug"||g_config.IOS_USE_REAL_PHONE){
		//sshell.push('security list-keychains -s login.keychain;')
		//sshell.push('security default-keychain -s login.keychain;')
		//todo: prompt for pwd instead? could find a local-only solution
		sshell.push('security unlock-keychain -p \'\' login.keychain;')
	}
	var s_ld_flags=['-L.']
	if(g_json.ldflags){
		for(var i=0;i<g_json.ldflags.length;i++){
			s_ld_flags.push(g_json.ldflags[i]);
		}
	}
	var s_xcode_flags=['']
	if(g_json.xcode_flags){
		for(var i=0;i<g_json.xcode_flags.length;i++){
			s_xcode_flags.push(g_json.xcode_flags[i]);
		}
	}
	//todo: sudo case - add that for the first local run?
	if(g_build!="debug"){
		sshell.push('xcodebuild -sdk iphoneos -configuration Release build '+s_xcode_flags.join(' ')+' OTHER_CFLAGS=\'${inherited} -DNEED_MAIN_WRAPPING -w -Isdl/include -Isdl/src \' OTHER_LDFLAGS=\' '+s_ld_flags.join(' ')+' \' || exit;')
	}else{
		if(g_config.IOS_USE_REAL_PHONE){
			sshell.push('xcodebuild -sdk iphoneos -configuration Debug build '+s_xcode_flags.join(' ')+' OTHER_CFLAGS=\'${inherited} -O0 -DNEED_MAIN_WRAPPING -w -Isdl/include -Isdl/src -L./ \' OTHER_LDFLAGS=\' '+s_ld_flags.join(' ')+' \' || exit;')
		}else{
			sshell.push('xcodebuild -sdk iphonesimulator -configuration Debug build '+s_xcode_flags.join(' ')+' OTHER_CFLAGS=\'${inherited} -O0 -DNEED_MAIN_WRAPPING -w -Isdl/include -Isdl/src -L./ \' OTHER_LDFLAGS=\' '+s_ld_flags.join(' ')+' \' || exit;')
		}
	}
	if(g_build!="debug"){
		sshell.push('cp Icon.png build/Release-iphoneos/'+g_main_name+'.app/Icon.png;')
		sshell.push('cp Icon-72.png build/Release-iphoneos/'+g_main_name+'.app/Icon-72.png;')
		sshell.push('cp Icon-Small.png build/Release-iphoneos/'+g_main_name+'.app/Icon-Small.png;')
		sshell.push('cp Icon-Small-50.png build/Release-iphoneos/'+g_main_name+'.app/Icon-Small-50.png;')
		sshell.push('cp Icon@2x.png build/Release-iphoneos/'+g_main_name+'.app/Icon@2x.png;')
		sshell.push('cp Icon-72@2x.png build/Release-iphoneos/'+g_main_name+'.app/Icon-72@2x.png;')
		sshell.push('cp Icon-Small@2x.png build/Release-iphoneos/'+g_main_name+'.app/Icon-Small@2x.png;')
		sshell.push('cp Icon-Small-50@2x.png build/Release-iphoneos/'+g_main_name+'.app/Icon-Small-50@2x.png;')
		sshell.push('cp Icon-76.png build/Release-iphoneos/'+g_main_name+'.app/Icon-76.png;')
		sshell.push('cp Icon-60.png build/Release-iphoneos/'+g_main_name+'.app/Icon-60.png;')
		sshell.push('cp Icon-76@2x.png build/Release-iphoneos/'+g_main_name+'.app/Icon-76@2x.png;')
		sshell.push('cp Icon-60@2x.png build/Release-iphoneos/'+g_main_name+'.app/Icon-60@2x.png;')
		sshell.push('cp Default-568h.png build/Release-iphoneos/'+g_main_name+'.app/Default-568h.png;')
		sshell.push('cp Default.png build/Release-iphoneos/'+g_main_name+'.app/Default.png;')
		sshell.push('xcrun -sdk iphoneos PackageApplication -v `pwd`/build/Release-iphoneos/'+g_main_name+'.app --sign \'iPhone Distribution\' --embed dist.mobileprovision;')
		sshell.push('codesign -s \'iPhone Distribution\' \'build/Release-iphoneos/'+g_main_name+'.ipa\';')
	}
	//if g_build!="debug"||g_config.IOS_USE_REAL_PHONE:
	//	sshell.push('security list-keychains -s login.keychain;')
	//	sshell.push('security default-keychain -s login.keychain;')
	if(g_need_ssh_for_mac){
		sshell.push('exit')
		envssh('mac',sshell.join(""))
	}else{
		CreateFile(g_work_dir+"/build_local.sh",sshell.join(""))
		shell([g_work_dir+"/build_local.sh"])
	}
	//get back the build result? messy
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
		s_target_dir=g_work_dir+"/upload"
	}
	var sshell=[]
	if(g_build!="debug"){
		sshell.push('killall lldb; killall ios-deploy; '+s_target_dir+'/ios-deploy -d -b '+s_target_dir+'/build/Release-iphoneos/'+g_main_name+'.ipa; ')
	}else{
		if(g_config.IOS_USE_REAL_PHONE){
			sshell.push('killall lldb; killall ios-deploy; '+s_target_dir+'/ios-deploy -d -b '+s_target_dir+'/build/Debug-iphoneos/'+g_main_name+'.app;')
		}else{
			sshell.push('~/pmenv/ios-sim launch '+s_target_dir+'/build/Debug-iphonesimulator/'+g_main_name+'.app;')
		}
	}
	if(g_need_ssh_for_mac){
		sshell.push('exit')
		var ssh_addr=GetServerSSH('mac')
		envssh('mac',sshell.join(""))
	}else{
		CreateFile(g_work_dir+"/run_local.sh",sshell.join(""))
		shell([g_work_dir+"/run_local.sh"])
	}
}
