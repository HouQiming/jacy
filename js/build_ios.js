/*
todo: update archiving commandline
	http://bitbar.com/tipstricks-how-to-archive-and-export-ipa-from-script/
	xcodebuild archive -project BitbarIOSSample.xcodeproj -scheme BitbarIOSSample-cal -archivePath BitbarIOSSample.xcarchive
	xcodebuild -exportArchive -archivePath <PROJECT_NAME>.xcarchive -exportPath <PROJECT_NAME> -exportFormat ipa -exportProvisioningProfile "Name of Provisioning Profile"
----
security add-generic-password -s Xcode:itunesconnect.apple.com -a LOGIN -w PASSWORD -U
build: CODE_SIGN_IDENTITY="iPhone Distribution:"
----
/usr/bin/xcrun -sdk iphoneos PackageApplication -v $FULL_PATH_TO_APP -o the-ipa --sign "iPhone Distribution:" --embed *.mobileprovision
codesign -s "Distribution" the-ipa
/Applications/Xcode.app/Contents/Applications/Application\ Loader.app/Contents/Frameworks/ITunesSoftwareService.framework/Support/altool -v -f ./build/Release-iphoneos/fudemo.ipa -u itunesconnect@user.com
/Applications/Xcode.app/Contents/Applications/Application\ Loader.app/Contents/Frameworks/ITunesSoftwareService.framework/Support/altool --upload-app -f ./build/Release-iphoneos/fudemo.ipa -u itunesconnect@user.com
----
#setting up a new system
security create-keychain -p '' ios.keychain
security import ./ios_distribution.cer -k ios.keychain -T /usr/bin/codesign
security import ./ios_development.cer -k ios.keychain -T /usr/bin/codesign
#then manually import apple wwdr ca cert to root
#https://developer.apple.com/library/ios/documentation/IDEs/Conceptual/AppStoreDistributionTutorial/AddingYourAccounttoXcode/AddingYourAccounttoXcode.html

nano /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/PackageApplication
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

var pushMakeItem=function(smakefile,fn_c,arch,CC,CFLAGS){
	smakefile.push(RemoveExtension(fn_c),'-',arch,'.o: ',fn_c,'\n')
	smakefile.push('\t',CC,' ')
	var s_ext=GetExtension(fn_c).toLowerCase();
	if(s_ext=='m'||s_ext=='c'){
		smakefile.push(' -std=c99 ');
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
	if(g_build!="debug"){
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
	var ssh_addr;
	if(g_need_ssh_for_mac){
		ssh_addr=GetServerSSH('mac');
	}
	var dir_pmenv=g_root+"/mac/pmenv"
	//build the project dir locally
	mkdir(g_work_dir+"/upload/")
	var smakefile=undefined;
	if(g_json.is_library){
		//skeleton makefile
		smakefile=[];
		smakefile.push('IOSSDK=$(shell ls /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/ | grep [0-9]\\.)\n')
		smakefile.push('EMUSDK=$(shell ls /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/ | grep [0-9]\\.)\n')
	}else{
		if(!FileExists(g_work_dir+"/SDL_setup")){
			//the SDL skeleton project
			shell(["cp","-r",g_config.IOS_SKELETON_PATH+"/*",g_work_dir+"/upload/"])
			var s_text=ReadFile(g_work_dir+"/upload/___PROJECTNAME___.xcodeproj/project.pbxproj")
			CreateFile(g_work_dir+"/upload/___PROJECTNAME___.xcodeproj/project.pbxproj",s_text.replace(new RegExp("___PROJECTNAME___","g"),g_main_name))
			shell(["mv",g_work_dir+'/upload/___PROJECTNAME___.xcodeproj',g_work_dir+'/upload/'+g_main_name+'.xcodeproj'])
			CreateFile(g_work_dir+"/SDL_setup","1")
		}
	}
	var sbuildtmp;
	if(g_need_ssh_for_mac){
		if(!FileExists(g_work_dir+"/buildtmp_ready")){
			var s_machine_tag="";
			if(g_current_arch=="win32"||g_current_arch=="win64"){
				s_machine_tag=ExpandEnvironmentStrings("%APPDATA%%PATH%")
			}else{
				s_machine_tag=ExpandEnvironmentStrings("${APPDATA}${PATH}")
			}
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
	//////////////////////
	var s_build_number=(ReadFile(g_work_dir+"/build_number")||'0');
	s_build_number=((parseInt(s_build_number)||0)+1).toString();
	var s_text=ReadFile(g_config.IOS_SKELETON_PATH+"/Info.plist")
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
	//////////////////////
	var fn_c_32=g_work_dir+"/s7main.c";
	var fn_c_64=g_work_dir+"/s7main64.c";
	var fn_c_bi=g_work_dir+"/s7main_bi.c";
	if(IsNewerThan(fn_c_32,fn_c_64)){
		//64-bit build
		var jc_cmdline=[g_root+"/bin/win32_release/jc.exe","--64","-a"+g_arch,"-b"+g_build,"-c","--c="+fn_c_64];
		if(g_json.is_library){
			jc_cmdline.push('--shared');
		}
		for(var i=0;i<g_json.input_files.length;i++){
			jc_cmdline.push(g_json.input_files);
		}
		shell(jc_cmdline);
	}
	var got_original_main_c=0;
	for(var i=0;i<g_json.c_files.length;i++){
		if(g_base_dir+"/"+g_json.c_files[i]==fn_c_32||g_json.c_files[i]==fn_c_32){
			got_original_main_c=1;
			g_json.c_files[i]=fn_c_bi;
			break;
		}
	}
	if(!got_original_main_c){
		print(JSON.stringify(g_json.c_files),fn_c_32)
		throw new Error("cannot find s7main.c in c_files")
	}
	if(g_json.is_library){
		var s_c_64=ReadFile(fn_c_64)
		var s_c_32=ReadFile(fn_c_32)
		CreateIfDifferent(fn_c_bi,
			['#if __LP64__\n',
				s_c_64,
			'\n#else\n',
				s_c_32,
			'\n#endif\n'].join(''))
	}else{
		g_json.h_files.push(fn_c_64)
		g_json.h_files.push(fn_c_32)
		CreateIfDifferent(fn_c_bi,'#if __LP64__\n#include "s7main64.c"\n#else\n#include "s7main.c"\n#endif\n')
	}
	//////////////////////
	var c_files=CreateProjectForStandardFiles(g_work_dir+"/upload/")
	if(g_json.is_library){
		//library makefile
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
			smakefile.push('libtmp.a: libtmp-armv7.a libtmp-armv7s.a libtmp-arm64.a libtmp-emu.a\n')
			smakefile.push('	lipo -create -output libtmp.a libtmp-armv7.a libtmp-armv7s.a libtmp-arm64.a libtmp-emu.a\n\n')
		}else{
			smakefile.push('lib'+g_main_name+'.a: libtmp-armv7.a libtmp-armv7s.a libtmp-arm64.a libtmp-emu.a\n')
			smakefile.push('	lipo -create -output lib'+g_main_name+'.a libtmp-armv7.a libtmp-armv7s.a libtmp-arm64.a libtmp-emu.a\n\n')
		}
		////////////////////////////////
		var s_extra_cflags=[];
		if(g_json.ios_frameworks){
			for(var i=0;g_json.ios_frameworks[i];i++){
				s_extra_cflags.push(' -framework ',g_json.ios_frameworks[i]);
			}
		}
		if(g_json.cflags){
			for(var i=0;g_json.cflags[i];i++){
				s_extra_cflags.push(' ',g_json.cflags[i]);
			}
		}
		pushMakeItemArch(smakefile,c_files,'armv7',
			'/Applications/Xcode.app/Contents/Developer/usr/bin/gcc',
			s_extra_cflags.join('')+" -fembed-bitcode -DNDEBUG -DHAS_NEON -arch armv7 -pipe -mdynamic-no-pic -Wno-trigraphs -fpascal-strings -O2 -Wreturn-type -Wunused-variable -fmessage-length=0 -fvisibility=hidden -miphoneos-version-min=3.2 -I/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/${IOSSDK}/usr/include/libxml2 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/${IOSSDK}/",
			"/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar",
			"/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip");
		pushMakeItemArch(smakefile,c_files,'armv7s',
			'/Applications/Xcode.app/Contents/Developer/usr/bin/gcc',
			s_extra_cflags.join('')+" -fembed-bitcode -DNDEBUG -DHAS_NEON -arch armv7s -pipe -mdynamic-no-pic -Wno-trigraphs -fpascal-strings -O2 -Wreturn-type -Wunused-variable -fmessage-length=0 -fvisibility=hidden -miphoneos-version-min=3.2 -I/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/${IOSSDK}/usr/include/libxml2 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/${IOSSDK}/",
			"/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar",
			"/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip");
		pushMakeItemArch(smakefile,c_files,'arm64',
			'/Applications/Xcode.app/Contents/Developer/usr/bin/gcc',
			s_extra_cflags.join('')+" -fembed-bitcode -DNDEBUG -DHAS_NEON -arch arm64 -pipe -mdynamic-no-pic -Wno-trigraphs -fpascal-strings -O2 -Wreturn-type -Wunused-variable -fmessage-length=0 -fvisibility=hidden -miphoneos-version-min=3.2 -I/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/${IOSSDK}/usr/include/libxml2 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/${IOSSDK}/",
			"/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar",
			"/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip");
		pushMakeItemArch(smakefile,c_files,'emu',
			'/Applications/Xcode.app/Contents/Developer/usr/bin/gcc',
			s_extra_cflags.join('')+" -arch x86_64 -pipe -mdynamic-no-pic -Wno-trigraphs -fpascal-strings -DUSE_SSE -O2 -Wreturn-type -Wunused-variable -fmessage-length=0 -fvisibility=hidden -miphoneos-version-min=3.2 -I/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/${EMUSDK}/usr/include/libxml2 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/${EMUSDK}/",
			"/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar",
			"/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip");
		CreateIfDifferent(g_work_dir+"/upload/Makefile",smakefile.join(""))
	}else{
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
			CreateIfDifferent(g_work_dir+'/upload/dist.mobileprovision',ReadFile(g_base_dir+"/dist.mobileprovision"))
		}
		if(FileExists(g_base_dir+"/dev.mobileprovision")){
			CreateIfDifferent(g_work_dir+'/upload/dev.mobileprovision',ReadFile(g_base_dir+"/dev.mobileprovision"))
		}
		var spython=[];
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
	}
	var sshell=[]
	if(g_need_ssh_for_mac){
		sshell.push('echo "----updating project----";')
		sshell.push('cd ~/_buildtmp/'+sbuildtmp+';')
		sshell.push('chmod -R 700 ~/_buildtmp/'+sbuildtmp+'/*;')
	}else{
		sshell.push('#!/bin/sh\n')
		sshell.push('cd '+g_work_dir+'/upload;')
	}
	if(g_build!="debug"||g_config.IOS_USE_REAL_PHONE){
		//sshell.push('security list-keychains -s login.keychain;')
		//sshell.push('security default-keychain -s login.keychain;')
		//sshell.push('security unlock-keychain -p \'\' login.keychain;')
		sshell.push('echo "--- Setting and unlocking keychains ---";')
		sshell.push('security default-keychain -s ios.keychain;')
		sshell.push('security unlock-keychain -p \'\' ios.keychain;')
		//sshell.push('security set-keychain-settings -t 3600 -u ios.keychain;')
	}
	if(g_json.is_library){
		//sshell.push('export IOSSDK=`ls /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/ | grep [0-9]\\.`;')
		//sshell.push('export EMUSDK=`ls /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/ | grep [0-9]\\.`;')
		sshell.push('echo "----building----";')
		sshell.push('make lib'+g_main_name+'.a;')
	}else{
		//the re-add approach has guid issues
		sshell.push('python ./tmp.py;')
		sshell.push('echo "----building----";')
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
		sshell.push('rm ~/Library/MobileDevice/Provisioning\\ Profiles/*;')
		var s_provision="";
		if(g_build!="debug"){
			if(FileExists(g_base_dir+"/dist.mobileprovision")){
				sshell.push('cp dist.mobileprovision ~/Library/MobileDevice/Provisioning\\ Profiles/;')
				if(!s_provision&&g_build!="debug"){s_provision=ReadFile(g_base_dir+"/dist.mobileprovision");}
			}
		}else{
			if(FileExists(g_base_dir+"/dev.mobileprovision")){
				sshell.push('cp dev.mobileprovision ~/Library/MobileDevice/Provisioning\\ Profiles/;')
				if(!s_provision&&g_build=="debug"){s_provision=ReadFile(g_base_dir+"/dev.mobileprovision");}
			}
		}
		var team_id="no_team";
		var team_match=s_provision.match(/<key>TeamIdentifier<\/key>[ \t\r\n]*<array>[ \t\r\n]*<string>(.*)<\/string>/);
		if(team_match){
			team_id=team_match[1];
			//print('team_id=',team_id);
		}
		var uuid="no_uuid";
		var uuid_match=s_provision.match(/<key>UUID<\/key>[ \t\r\n]*<string>(.*)<\/string>/);
		if(uuid_match){
			uuid=uuid_match[1];
			//print('team_id=',team_id);
		}
		if(g_build!="debug"){
			sshell.push('xcodebuild -sdk iphoneos -configuration Release build ',s_xcode_flags.join(' '),' CODE_SIGN_IDENTITY="iPhone Distribution" PROVISIONING_PROFILE="'+uuid+'" OTHER_CFLAGS=\'${inherited} -DNEED_MAIN_WRAPPING -w -Isdl/include -Isdl/src \' DEVELOPMENT_TEAM=\'',team_id,'\' OTHER_LDFLAGS=\' '+s_ld_flags.join(' ')+' \' || exit;')
		}else{
			if(g_config.IOS_USE_REAL_PHONE){
				sshell.push('xcodebuild -sdk iphoneos -configuration Debug build '+s_xcode_flags.join(' ')+' CODE_SIGN_IDENTITY="iPhone Developer" PROVISIONING_PROFILE="'+uuid+'" OTHER_CFLAGS=\'${inherited} -O0 -DNEED_MAIN_WRAPPING -w -Isdl/include -Isdl/src -L./ \' DEVELOPMENT_TEAM=\'',team_id,'\' OTHER_LDFLAGS=\' '+s_ld_flags.join(' ')+' \' || exit;')
			}else{
				sshell.push('xcodebuild -sdk iphonesimulator -configuration Debug build '+s_xcode_flags.join(' ')+' CODE_SIGN_IDENTITY="iPhone Developer" OTHER_CFLAGS=\'${inherited} -O0 -DNEED_MAIN_WRAPPING -w -Isdl/include -Isdl/src -L./ \' OTHER_LDFLAGS=\' '+s_ld_flags.join(' ')+' \' || exit;')
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
	}
	//if g_build!="debug"||g_config.IOS_USE_REAL_PHONE:
	//	sshell.push('security list-keychains -s login.keychain;')
	//	sshell.push('security default-keychain -s login.keychain;')
	if(g_need_ssh_for_mac){
		sshell.push('exit')
		rsync(g_work_dir+'/upload',ssh_addr+':~/_buildtmp/'+sbuildtmp)
		envssh('mac',sshell.join(""))
		if(g_json.is_library){
			_rsync(ssh_addr+':~/_buildtmp/'+sbuildtmp+'/lib'+g_main_name+'.a',g_bin_dir)
		}else if(g_build!="debug"){
			_rsync(ssh_addr+':~/_buildtmp/'+sbuildtmp+'/build/Release-iphoneos/'+g_main_name+'.ipa',g_bin_dir)
		}
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
