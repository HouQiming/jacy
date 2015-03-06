/*
PROVISIONING_PROFILE = "487F3EAC-05FB-4A2A-9EA0-31F1F35760EB";
"PROVISIONING_PROFILE[sdk=iphoneos*]" = "487F3EAC-05FB-4A2A-9EA0-31F1F35760EB";
to build SDL:
	cd ~/pmenv/SDL2-2.0.3
	build-scripts/iosbuild.sh configure-armv7
	cp ../build/armv7/include/SDL_config.h ../include/
	cd ~/pmenv/SDL2-2.0.3/Xcode-iOS/SDL
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
	//todo: local case
	var ssh_addr=GetServerSSH('mac')
	var dir_pmenv=g_root+"/mac/pmenv"
	//build the project dir locally
	!? //g_need_ssh_for_mac
	if !FileExists(g_work_dir+"/buildtmp_ready"):
		sbuildtmp=SHA1(NormalizeFileName(g_work_dir),8)
		CreateFile(g_work_dir+"/buildtmp_ready",sbuildtmp)
		mkdir(g_work_dir+"/upload/SDL/lib")
		if FileExists(PRJ.root+"/default_screen.png"):
			s_rm_default="rm Default.png;"
		else
			s_rm_default=""
		if FileExists(PRJ.root+"/ic_launcher.png"):
			s_rm_icon="rm Icon.png;"
		else
			s_rm_icon=""
		envssh('mac',
			'echo "----cleanup----";'+
			'chmod -R 777 ~/_buildtmp/'+sbuildtmp+';'+
			'rm -rf ~/_buildtmp/'+sbuildtmp+';'+
			'mkdir -p ~/_buildtmp/'+sbuildtmp+'/SDL/lib;'+
			'mkdir -p ~/_buildtmp/'+sbuildtmp+'/SDL/include;'+
			'echo "----template----";'+
			'cp -r ~/pmenv/SDL2-2.0.3/Xcode-iOS/Template/SDL\\ iOS\\ Application/* ~/_buildtmp/'+sbuildtmp+'/;'+
			'cp ~/pmenv/modxproj.py ~/_buildtmp/'+sbuildtmp+'/;'+
			'echo "----SDL----";'+
			'cp -r ~/pmenv/SDL2-2.0.3/include/* ~/_buildtmp/'+sbuildtmp+'/SDL/include/;'+
			'cp -r ~/pmenv/SDL2-2.0.3/Xcode-iOS/SDL/build/libSDL2.a ~/_buildtmp/'+sbuildtmp+'/SDL/lib/libSDL.a;'+
			'cd ~/_buildtmp/'+sbuildtmp+';'+
			'echo "----patch----";'+
			"sed -i '' 's/___PROJECTNAME___/"+PRJ.name+"/g' ___PROJECTNAME___.xcodeproj/project.pbxproj;"+
			"sed -i '' 's/com.yourcompany.*\\}/"+StringReplace("com.spap."+PRJ.name,["_",""])+"/g' Info.plist;"+
			"cp ___PROJECTNAME___.xcodeproj/project.pbxproj ./xproj.bak;"+
			'mv ___PROJECTNAME___.xcodeproj "'+PRJ.name+'.xcodeproj";'+
			s_rm_default+s_rm_icon+
			'rm main.c;'+
			'exit')
		cd(g_work_dir)
		//rsync(ssh_addr+':~/_buildtmp/'+sbuildtmp,'./upload')
		//sinfo=ReadFileNonLocked(g_work_dir+"/upload/Info.plist")
		//sinfo=StringReplace(sinfo,["com.yourcompany.${PRODUCT_NAME:identifier}",StringReplace("com.spap."+PRJ.name,["_",""])])
		//CreateIfDifferent(g_work_dir+"/upload/Info.plist",sinfo)
		//<string></string>
	sbuildtmp=ReadFileNonLocked(g_work_dir+"/buildtmp_ready")
	cd(g_work_dir)
	////////////////////////
	dir_prj_template=dir_pmenv+""
	//copy-in the relevant source files
	for(i=0;i<len(wrapper_files);i++)
		fn=wrapper_files[i]
		if MatchRegex(".*\\.(c|cpp|m)",StringToLower(fn)):
			IOS.CopyToUpload(fn)
	for(i=0;i<len(libs);i++)
		fn=libs[i]
		IOS.CopyToUpload(fn)
	if VAR.ios_use_reszip_png:
		IOS.CopyToUpload(g_work_dir+"\\reszip.bundle")
	//generate SPAP as main.c
	if PRJ.has_spap:
		SPAP.is_release=PRJ.is_release
		SPAP.is64=0
		SPAP.Detect()
		SPAP.options=" -g --C --outputC -Dnodllmess -Drebuild -Done_pass -Ddumb_temp_names -Dcpp.entrypoint=SDL_main -Denabled.platform.ios=1 -Denabled.platform.unix=1 "
		s_C_output=NormalizeFileName(g_work_dir+"/spap_main_c.txt")
		ret=SPAP.CompileMain(s_C_output)
		if !ret:
			Write('error> SPAP compiler failed\n')
			exit(1)
		CreateIfDifferent(g_work_dir+"/upload/main.c",ReadFile(s_C_output))
	//icons
	if FileExists(PRJ.root+"/ic_launcher.png"):
		fntouch=g_work_dir+"/ic_launcher.png._touch"
		if IsNewer(fntouch,PRJ.root+"/ic_launcher.png")
			sexe_resample_png=NormalizeFileName(root+"/osslib/android/misc_tools/resample_png.exe")
			RunProgram('"'+sexe_resample_png+'" "'+PRJ.root+'/ic_launcher.png" "'+g_work_dir+'/upload/Icon.png" 57 57')
			RunProgram('"'+sexe_resample_png+'" "'+PRJ.root+'/ic_launcher.png" "'+g_work_dir+'/upload/Icon-72.png" 72 72')
			RunProgram('"'+sexe_resample_png+'" "'+PRJ.root+'/ic_launcher.png" "'+g_work_dir+'/upload/Icon-Small.png" 29 29')
			RunProgram('"'+sexe_resample_png+'" "'+PRJ.root+'/ic_launcher.png" "'+g_work_dir+'/upload/Icon-Small-50.png" 50 50')
			RunProgram('"'+sexe_resample_png+'" "'+PRJ.root+'/ic_launcher.png" "'+g_work_dir+'/upload/Icon@2x.png" 114 114')
			RunProgram('"'+sexe_resample_png+'" "'+PRJ.root+'/ic_launcher.png" "'+g_work_dir+'/upload/Icon-72@2x.png" 144 144')
			RunProgram('"'+sexe_resample_png+'" "'+PRJ.root+'/ic_launcher.png" "'+g_work_dir+'/upload/Icon-Small@2x.png" 58 58')
			RunProgram('"'+sexe_resample_png+'" "'+PRJ.root+'/ic_launcher.png" "'+g_work_dir+'/upload/Icon-Small-50@2x.png" 100 100')
			RunProgram('"'+sexe_resample_png+'" "'+PRJ.root+'/ic_launcher.png" "'+g_work_dir+'/upload/Icon-76.png" 76 76')
			RunProgram('"'+sexe_resample_png+'" "'+PRJ.root+'/ic_launcher.png" "'+g_work_dir+'/upload/Icon-60.png" 60 60')
			RunProgram('"'+sexe_resample_png+'" "'+PRJ.root+'/ic_launcher.png" "'+g_work_dir+'/upload/Icon-76@2x.png" 152 152')
			RunProgram('"'+sexe_resample_png+'" "'+PRJ.root+'/ic_launcher.png" "'+g_work_dir+'/upload/Icon-60@2x.png" 120 120')
			CreateFile(fntouch,PRJ.root+"/ic_launcher.png")
	if FileExists(PRJ.root+"/default_screen.png"):
		RunProgram('"'+sexe_resample_png+'" "'+PRJ.root+'/default_screen.png" "'+g_work_dir+'/upload/Default-568h.png" 320 568')
		UpdateTo(g_work_dir+'/upload/Default.png',PRJ.root+"/default_screen.png")
	if FileExists(PRJ.root+"/dist.mobileprovision"):
		UpdateTo(g_work_dir+'/upload/dist.mobileprovision',PRJ.root+"/dist.mobileprovision")
	//the re-add approach has guid issues
	//if !PRJ.is_release:
	//	spaptemp=ls(g_work_dir+"/upload/__spaptemp__/*.c")
	//	for(i=0;i<len(spaptemp);i++)
	//		fn=spaptemp[i]
	//		CopyToUpload(fn)
	spython=""
	StringAppend(spython,'from modxproj import XcodeProject\n')
	StringAppend(spython,'project = XcodeProject.Load("'+PRJ.name+'.xcodeproj/project.pbxproj")\n')
	//we don't have to add main.c
	for(i=0;IOS.c_file_list[i];i++)
		fn=IOS.c_file_list[i]
		StringAppend(spython,'fn="'+fn+'"\n')
		StringAppend(spython,'if len(project.get_files_by_name(fn))<=0:\n')
		StringAppend(spython,'	project.add_file(fn)\n')
	for(i=0;ios_frameworks[i];i++)
		StringAppend(spython,'project.add_file("'+ios_frameworks[i]+'",tree="SDKROOT")\n')
	StringAppend(spython,'if project.modified:\n')
	StringAppend(spython,'	project.save()\n')
	CreateIfDifferent(g_work_dir+"/upload/tmp.py",spython)
	rsync('./upload',ssh_addr+':~/_buildtmp/'+sbuildtmp)
	sshell=""
	StringAppend(sshell,'echo "----updating project----";')
	StringAppend(sshell,'cd ~/_buildtmp/'+sbuildtmp+';')
	StringAppend(sshell,'chmod -R 700 ~/_buildtmp/'+sbuildtmp+'/*;')
	//the re-add approach has guid issues
	//StringAppend(sshell,'cp xproj.bak "'+PRJ.name+'.xcodeproj/project.pbxproj";')
	StringAppend(sshell,'python ./tmp.py;')
	StringAppend(sshell,'echo "----building----";')
	if PRJ.is_release||VAR.use_real_phone:
		//StringAppend(sshell,'security list-keychains -s login.keychain;')
		//StringAppend(sshell,'security default-keychain -s login.keychain;')
		StringAppend(sshell,'security unlock-keychain -p \'\' login.keychain;')
	if PRJ.is_release:
		StringAppend(sshell,'xcodebuild -sdk iphoneos -configuration Release build OTHER_CFLAGS=\'${inherited} -ffast-math -w -I${HOME}/pmenv/include\';')
	else
		if VAR.use_real_phone:
			StringAppend(sshell,'xcodebuild -sdk iphoneos -configuration Debug build OTHER_CFLAGS=\'${inherited} -O0 -ffast-math -w -I${HOME}/pmenv/include\';')
		else
			StringAppend(sshell,'xcodebuild -sdk iphonesimulator -configuration Debug build OTHER_CFLAGS=\'${inherited} -O0 -ffast-math -w -I${HOME}/pmenv/include\';')
	if PRJ.is_release:
		StringAppend(sshell,'cp Icon.png build/Release-iphoneos/'+PRJ.name+'.app/Icon.png;')
		StringAppend(sshell,'cp Icon-72.png build/Release-iphoneos/'+PRJ.name+'.app/Icon-72.png;')
		StringAppend(sshell,'cp Icon-Small.png build/Release-iphoneos/'+PRJ.name+'.app/Icon-Small.png;')
		StringAppend(sshell,'cp Icon-Small-50.png build/Release-iphoneos/'+PRJ.name+'.app/Icon-Small-50.png;')
		StringAppend(sshell,'cp Icon@2x.png build/Release-iphoneos/'+PRJ.name+'.app/Icon@2x.png;')
		StringAppend(sshell,'cp Icon-72@2x.png build/Release-iphoneos/'+PRJ.name+'.app/Icon-72@2x.png;')
		StringAppend(sshell,'cp Icon-Small@2x.png build/Release-iphoneos/'+PRJ.name+'.app/Icon-Small@2x.png;')
		StringAppend(sshell,'cp Icon-Small-50@2x.png build/Release-iphoneos/'+PRJ.name+'.app/Icon-Small-50@2x.png;')
		StringAppend(sshell,'cp Icon-76.png build/Release-iphoneos/'+PRJ.name+'.app/Icon-76.png;')
		StringAppend(sshell,'cp Icon-60.png build/Release-iphoneos/'+PRJ.name+'.app/Icon-60.png;')
		StringAppend(sshell,'cp Icon-76@2x.png build/Release-iphoneos/'+PRJ.name+'.app/Icon-76@2x.png;')
		StringAppend(sshell,'cp Icon-60@2x.png build/Release-iphoneos/'+PRJ.name+'.app/Icon-60@2x.png;')
		StringAppend(sshell,'cp Default-568h.png build/Release-iphoneos/'+PRJ.name+'.app/Default-568h.png;')
		StringAppend(sshell,'cp Default.png build/Release-iphoneos/'+PRJ.name+'.app/Default.png;')
		StringAppend(sshell,'xcrun -sdk iphoneos PackageApplication -v `pwd`/build/Release-iphoneos/'+PRJ.name+'.app --sign \'iPhone Distribution\' --embed dist.mobileprovision;')
		StringAppend(sshell,'codesign -s \'iPhone Distribution\' \'build/Release-iphoneos/'+PRJ.name+'.ipa\';')
		//StringAppend(sshell,'echo xcrun -sdk iphoneos Validation -online -upload -verbose build/Release-iphoneos/'+PRJ.name+'.ipa|grep -v should;')
	//if PRJ.is_release||VAR.use_real_phone:
	//	StringAppend(sshell,'security list-keychains -s login.keychain;')
	//	StringAppend(sshell,'security default-keychain -s login.keychain;')
	StringAppend(sshell,'exit')
	//Write(sshell)
	envssh('mac',sshell)
	return 1
}

PRJ.Run=function(sdir_target){
	if !(PRJ.target=="ios"||PRJ.target=="ios-release"):
		return 0
	sbuildtmp=ReadFileNonLocked(g_work_dir+"/buildtmp_ready")
	if !sbuildtmp:
		Write("error> the project hasn't been built yet")
		exit(1)
	ssh_addr=GetServerSSH('mac')
	sshell=""
	if PRJ.is_release:
		StringAppend(sshell,'killall lldb; killall ios-deploy; ~/pmenv/ios-deploy/ios-deploy -d -b ~/_buildtmp/'+sbuildtmp+'/build/Release-iphoneos/'+PRJ.name+'.app; ')
	else
		if VAR.use_real_phone:
			StringAppend(sshell,'killall lldb; killall ios-deploy; ~/pmenv/ios-deploy/ios-deploy -d -b ~/_buildtmp/'+sbuildtmp+'/build/Debug-iphoneos/'+PRJ.name+'.app;')
		else
			StringAppend(sshell,'~/pmenv/ios-sim launch ~/_buildtmp/'+sbuildtmp+'/build/Debug-iphonesimulator/'+PRJ.name+'.app; ')
	StringAppend(sshell,'exit')
	envssh('mac',sshell)
}
