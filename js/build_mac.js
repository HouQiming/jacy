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

var pushMakeItem=function(smakefile,fn_c,arch,CC,CFLAGS){
	smakefile.push(RemoveExtension(fn_c),'-',arch,'.o: ',fn_c,'\n')
	smakefile.push('\t',CC,' ')
	var s_ext=GetExtension(fn_c).toLowerCase();
	if(s_ext=='m'||s_ext=='c'){
		smakefile.push(' -std=c99 ');
	}
	if(s_ext=='mm'||s_ext=='cc'||s_ext=='cpp'||s_ext=='cxx'){
		if(g_json.cxxflags){
			for(var i=0;g_json.cxxflags[i];i++){
				smakefile.push(' ',g_json.cxxflags[i]);
			}
		}
	}
	smakefile.push(' -DPM_IS_LIBRARY ');
	smakefile.push(CFLAGS,' -w -o $@ -c $<\n\n');
	return RemoveExtension(fn_c)+'-'+arch+'.o';
};

var pushMakeItemArch=function(smakefile,c_files,arch,CC,CFLAGS,AR,STRIP){
	var o_files=[];
	for(var i=0;i<c_files.length;i++){
		o_files.push(pushMakeItem(smakefile,c_files[i],arch,CC,CFLAGS));
	}
	if(g_json.mac_build_dynamic_library||g_json.is_library&&parseInt(g_json.is_library[0])===1){
		//do nothing
		smakefile.push(g_main_name+'.dylib: ')
		for(var i=0;i<o_files.length;i++){
			smakefile.push(' ');
			smakefile.push(o_files[i]);
		}
		if(g_lib_files){
			for(var i=0;i<g_lib_files.length;i++){
				smakefile.push(' ',g_lib_files[i]);
			}
		}
		smakefile.push('\n\tgcc -dynamiclib -o '+g_main_name+'.dylib')
		if(g_json.ldflags){
			for(var i=0;i<g_json.ldflags.length;i++){
				smakefile.push(' ',g_json.ldflags[i]);
			}
		}
		for(var i=0;i<o_files.length;i++){
			smakefile.push(' ');
			smakefile.push(o_files[i]);
		}
		if(g_lib_files){
			for(var i=0;i<g_lib_files.length;i++){
				smakefile.push(' ',g_lib_files[i]);
			}
		}
		if(g_json.mac_frameworks){
			smakefile.push(' -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/${MACSDK}')
			for(var i=0;g_json.mac_frameworks[i];i++){
				var match=g_json.mac_frameworks[i].match(/([^/]+)\.framework/);
				if(match){
					smakefile.push(' -framework ',match[1]);
				}
			}
		}
		smakefile.push(' -framework Foundation -lobjc\n\n')
	}else if(g_build!="debug"){
		smakefile.push('libtmp-',arch,'-unstripped.a:')
		for(var i=0;i<o_files.length;i++){
			smakefile.push(' ');
			smakefile.push(o_files[i]);
		}
		smakefile.push('\n\t',AR,' -rvs $@');
		for(var i=0;i<o_files.length;i++){
			smakefile.push(' ');
			smakefile.push(o_files[i]);
		}
		smakefile.push('\n\n');
		smakefile.push('libtmp-',arch,'.a: libtmp-',arch,'-unstripped.a\n')
		smakefile.push('\t',STRIP,' -S -x -o libtmp-',arch,'.a -r libtmp-',arch,'-unstripped.a\n\n')
	}else{
		smakefile.push('libtmp-',arch,'.a:')
		for(var i=0;i<o_files.length;i++){
			smakefile.push(' ');
			smakefile.push(o_files[i]);
		}
		smakefile.push('\n\t',AR,' -rvs $@');
		for(var i=0;i<o_files.length;i++){
			smakefile.push(' ');
			smakefile.push(o_files[i]);
		}
		smakefile.push('\n\n');
	}
};

g_action_handlers.make=function(){
	var ssh_addr,ssh_port;
	if(g_need_ssh_for_mac){
		ssh_addr=GetServerSSH('mac');
		ssh_port=GetPortSSH('mac');
	}
	var dir_pmenv=g_root+"/mac/pmenv"
	mkdir(g_work_dir+"/upload/")
	var smakefile=undefined;
	if(g_json.is_library){
		//skeleton makefile
		smakefile=[];
		smakefile.push('MACSDK=$(shell ls /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/ | grep [0-9]\\.)\n')
	}else{
		if(!FileExists(g_work_dir+"/SDL_setup")){
			//the SDL skeleton project
			shell(["cp","-r",g_config.MAC_SKELETON_PATH+"/*",g_work_dir+"/upload/"])
			var s_text=ReadFile(g_work_dir+"/upload/__SDL_TEMPLATE_PROJECT__.xcodeproj/project.pbxproj")
			CreateFile(g_work_dir+"/upload/__SDL_TEMPLATE_PROJECT__.xcodeproj/project.pbxproj",s_text.replace(new RegExp("__SDL_TEMPLATE_PROJECT__","g"),g_main_name))
			shell(["mv",g_work_dir+'/upload/__SDL_TEMPLATE_PROJECT__.xcodeproj',g_work_dir+'/upload/'+g_main_name+'.xcodeproj'])
			CreateFile(g_work_dir+"/SDL_setup","1")
		}
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
	if(g_json.is_library){
		//library makefile
		if(g_json.mac_build_dynamic_library||g_json.is_library&&parseInt(g_json.is_library[0])===1){
			//do nothing
		}else{
			if(g_lib_files){
				smakefile.push('lib'+g_main_name+'.a: libtmp.a')
				for(var i=0;i<g_lib_files.length;i++){
					smakefile.push(' ',g_lib_files[i]);
				}
				smakefile.push('\n\tlibtool -static -o lib'+g_main_name+'.a libtmp.a')
				for(var i=0;i<g_lib_files.length;i++){
					smakefile.push(' ',g_lib_files[i]);
				}
				smakefile.push('\n\n')
				smakefile.push('libtmp.a: libtmp-mac.a\n')
				smakefile.push('	lipo -create -output libtmp.a libtmp-mac.a\n\n')
			}else{
				smakefile.push('lib'+g_main_name+'.a: libtmp-mac.a\n')
				smakefile.push('	lipo -create -output lib'+g_main_name+'.a libtmp-mac.a\n\n')
			}
		}
		////////////////////////////////
		var s_extra_cflags=[];
		if(g_json.mac_frameworks){
			for(var i=0;g_json.mac_frameworks[i];i++){
				s_extra_cflags.push(' -framework ',g_json.mac_frameworks[i]);
			}
		}
		if(g_json.cflags){
			for(var i=0;g_json.cflags[i];i++){
				s_extra_cflags.push(' ',g_json.cflags[i]);
			}
		}
		pushMakeItemArch(smakefile,c_files,'mac',
			'/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang',
			s_extra_cflags.join('')+" -pipe -arch x86_64 -fmessage-length=0 -fdiagnostics-show-note-include-stack -fmacro-backtrace-limit=0 -fcolor-diagnostics -Wno-trigraphs -fpascal-strings -O2 -msse2 -DNDEBUG -DUSE_SSE -Wno-missing-field-initializers -Wno-missing-prototypes -Werror=return-type -Wno-implicit-atomic-properties -Werror=deprecated-objc-isa-usage -Werror=objc-root-class -Wno-receiver-is-weak -Wno-arc-repeated-use-of-weak -Wduplicate-method-match -Wno-missing-braces -Wparentheses -Wswitch -Wunused-function -Wno-unused-label -Wno-unused-parameter -Wunused-variable -Wunused-value -Wempty-body -Wconditional-uninitialized -Wno-unknown-pragmas -Wno-shadow -Wno-four-char-constants -Wno-conversion -Wconstant-conversion -Wint-conversion -Wbool-conversion -Wenum-conversion -Wshorten-64-to-32 -Wpointer-sign -Wno-newline-eof -Wno-selector -Wno-strict-selector-match -Wundeclared-selector -Wno-deprecated-implementations -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/${MACSDK} -fasm-blocks -fstrict-aliasing -Wprotocol -Wdeprecated-declarations -mmacosx-version-min=10.8 -g -Wno-sign-conversion -I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include ",
			"/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar",
			"/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip");
		CreateIfDifferent(g_work_dir+"/upload/Makefile",smakefile.join(""))
	}else{
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
		//////////////////////
		//info.plist
		var s_build_number=(ReadFile(g_work_dir+"/build_number")||'0');
		s_build_number=((parseInt(s_build_number)||0)+1).toString();
		var s_text=ReadFile(g_config.MAC_SKELETON_PATH+"/Info.plist")
		var s_display_name=(g_json.app_display_name&&g_json.app_display_name[0]||g_main_name);
		s_text=s_text.replace(/__display_name__/g,s_display_name);
		var s_version_string=(g_json.app_version&&g_json.app_version[0]||"1.0");
		s_text=s_text.replace(/__build_version__/g,s_version_string+"."+s_build_number);
		s_text=s_text.replace(/__version__/g,s_version_string);
		var a_extra_plist_keys=[];
		if(g_json.extra_plist_keys){
			for(var i=0;i<g_json.extra_plist_keys.length;i++){
				a_extra_plist_keys.push(g_json.extra_plist_keys[i])
			}
		}
		s_text=s_text.replace("__extra_keys__",a_extra_plist_keys.join(''))
		CreateIfDifferent(g_work_dir+"/build_number",s_build_number)
		CreateIfDifferent(g_work_dir+"/upload/Info.plist",s_text.replace(new RegExp("com.yourcompany.*\\}","g"),"com.spap."+g_main_name.replace(new RegExp("_","g"),"")))
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
	}
	var sshell=[]
	if(g_need_ssh_for_mac){
		rsync(g_work_dir+'/upload',ssh_addr+':~/_buildtmp/'+sbuildtmp,ssh_port)
		sshell.push('echo "----updating project----";')
		sshell.push('cd ~/_buildtmp/'+sbuildtmp+';')
		sshell.push('chmod -R 700 ~/_buildtmp/'+sbuildtmp+'/*;')
	}else{
		sshell.push('#!/bin/sh\n')
		sshell.push('cd '+g_work_dir+'/upload;')
	}
	//the re-add approach has guid issues
	if(g_json.is_library){
		//sshell.push('export IOSSDK=`ls /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/ | grep [0-9]\\.`;')
		//sshell.push('export EMUSDK=`ls /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/ | grep [0-9]\\.`;')
		sshell.push('echo "----building----";')
		if(g_json.mac_build_dynamic_library||g_json.is_library&&parseInt(g_json.is_library[0])===1){
			sshell.push('make '+g_main_name+'.dylib;')
		}else{
			sshell.push('make lib'+g_main_name+'.a;')
		}
	}else{
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
	}
	if(g_need_ssh_for_mac){
		//hdiutil create -volname WhatYouWantTheDiskToBeNamed -srcfolder /path/to/the/folder/you/want/to/create -ov -format UDZO name.dmg
		if(g_json.is_library){
			//do nothing
		}else if(g_build!="debug"){
			sshell.push('mkdir -p download; cd build/',sdirname,'/; strip '+g_main_name,'.app/Contents/MacOS/',g_main_name,';')
			sshell.push('rm -rf dmg; mkdir -p dmg; cp -r ',g_main_name,'.app dmg/; ln -s /Applications dmg/Applications;')
			//sshell.push('; tar -czf '+g_main_name+'.tar.gz '+g_main_name+'.app; mv '+g_main_name+'.tar.gz ../../download/;')
			sshell.push('hdiutil create -volname ',g_main_name,' -srcfolder dmg/ -ov -format UDZO ',g_main_name,'.dmg; mv '+g_main_name+'.dmg ../../download/;')
		}
		sshell.push('exit')
		envssh('mac',sshell.join(""))
		if(g_json.is_library){
			if(g_json.mac_build_dynamic_library||g_json.is_library&&parseInt(g_json.is_library[0])===1){
				_rsync(ssh_addr+':~/_buildtmp/'+sbuildtmp+'/'+g_main_name+'.dylib',g_bin_dir,ssh_port)
			}else{
				_rsync(ssh_addr+':~/_buildtmp/'+sbuildtmp+'/lib'+g_main_name+'.a',g_bin_dir,ssh_port)
			}
		}else if(g_build!="debug"){
			rsync(ssh_addr+':~/_buildtmp/'+sbuildtmp+'/download',g_bin_dir,ssh_port)
		}
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
