/*
don't build it, use the framework
/Users/houqiming/pmenv/SDL2-2.0.3/build/.libs
remove -fmodules
*/
var g_need_ssh_for_mac=(g_current_arch!="mac");
var MAC={}
MAC.c_file_list=[]

MAC.CopyToUpload=function(fn0){
	var fn=fn0
	var spackage_dir=g_work_dir+"/upload"
	var star=spackage_dir+"/"+RemovePath(fn0)
	UpdateTo(star,fn)
	MAC.c_file_list.push(RemovePath(fn0))
}

g_action_handlers.make=function(){
	//todo: local case, xcode vs plain-old-make
	!?
	var ssh_addr=GetServerSSH('mac')
	var dir_pmenv=g_root+"/mac/pmenv"
	if !FileExists(g_work_dir+"/buildtmp_ready"):
		sbuildtmp=SHA1(NormalizeFileName(g_work_dir),8)
		CreateFile(g_work_dir+"/buildtmp_ready",sbuildtmp)
		mkdir(g_work_dir+"/upload/")
		envssh('mac',
			'echo "----cleanup----";'+
			'chmod -R 777 ~/_buildtmp/'+sbuildtmp+';'+
			'rm -rf ~/_buildtmp/'+sbuildtmp+';'+
			'mkdir -p ~/_buildtmp/'+sbuildtmp+'/SDL/include;'+
			'rm ~/pmenv/SDL2-2.0.3/build/.libs/libSDL2.la*;'+
			'rm ~/pmenv/SDL2-2.0.3/build/.libs/libSDL2*so*;'+
			'rm ~/pmenv/SDL2-2.0.3/build/libSDL2.la*;'+
			'echo "----template----";'+
			'cp -r ~/pmenv/__SDL_TEMPLATE_PROJECT__/* ~/_buildtmp/'+sbuildtmp+'/;'+
			'cp ~/pmenv/modxproj.py ~/_buildtmp/'+sbuildtmp+'/;'+
			'echo "----SDL----";'+
			'cp -r ~/pmenv/SDL2-2.0.3/include/* ~/_buildtmp/'+sbuildtmp+'/SDL/include/;'+
			'cp -r ~/pmenv/SDL2-2.0.3/build/.libs/libSDL2.a ~/_buildtmp/'+sbuildtmp+'/libSDL2.a;'+
			'cp -r ~/pmenv/SDL2-2.0.3/build/libSDL2main.a ~/_buildtmp/'+sbuildtmp+'/libSDL2main.a;'+
			//'cp -r ~/pmenv/SDL2-2.0.3/src/main/SDLMain.m ~/_buildtmp/'+sbuildtmp+'/SDLMain.m;'+
			'cd ~/_buildtmp/'+sbuildtmp+';'+
			'echo "----patch----";'+
			"sed -i '' 's/__SDL_TEMPLATE_PROJECT__/"+PRJ.name+"/g' __SDL_TEMPLATE_PROJECT__.xcodeproj/project.pbxproj;"+
			"cp __SDL_TEMPLATE_PROJECT__.xcodeproj/project.pbxproj ./xproj.bak;"+
			'mv __SDL_TEMPLATE_PROJECT__.xcodeproj "'+PRJ.name+'.xcodeproj";'+
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
			MAC.CopyToUpload(fn)
	for(i=0;i<len(libs);i++)
		fn=libs[i]
		MAC.CopyToUpload(fn)
	if VAR.mac_use_reszip_png:
		MAC.CopyToUpload(g_work_dir+"\\reszip.bundle")
	//generate SPAP as main.c
	if PRJ.has_spap:
		SPAP.is_release=PRJ.is_release
		SPAP.is64=1
		SPAP.Detect()
		SPAP.options=" -g --C --outputC -Dnodllmess -Drebuild -Done_pass -Ddumb_temp_names -Dcpp.entrypoint=real_main -Denabled.platform.mac=1 -Denabled.platform.unix=1 "
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
			CreateFile(fntouch,PRJ.root+"/ic_launcher.png")
	if FileExists(PRJ.root+"/default_screen.png"):
		UpdateTo(g_work_dir+'/upload/Default.png',PRJ.root+"/default_screen.png")
	//the re-add approach has guid issues
	//if !PRJ.is_release:
	//	spaptemp=ls(g_work_dir+"/upload/__spaptemp__/*.c")
	//	for(i=0;i<len(spaptemp);i++)
	//		fn=spaptemp[i]
	//		CopyToUpload(fn)
	spython=""
	StringAppend(spython,'from modxproj import XcodeProject\n')
	StringAppend(spython,'project = XcodeProject.Load("'+PRJ.name+'.xcodeproj/project.pbxproj")\n')
	StringAppend(spython,'fn="libSDL2.a"\n')
	StringAppend(spython,'if len(project.get_files_by_name(fn))<=0:\n')
	StringAppend(spython,'	project.add_file(fn)\n')
	StringAppend(spython,'fn="libSDL2main.a"\n')
	StringAppend(spython,'if len(project.get_files_by_name(fn))<=0:\n')
	StringAppend(spython,'	project.add_file(fn)\n')
	//StringAppend(spython,'project.add_file("main.c")\n')
	MAC.c_file_list.push("main.c")
	//MAC.c_file_list.push("SDLMain.m")
	for(i=0;MAC.c_file_list[i];i++)
		fn=MAC.c_file_list[i]
		StringAppend(spython,'fn="'+fn+'"\n')
		StringAppend(spython,'if len(project.get_files_by_name(fn))<=0:\n')
		StringAppend(spython,'	project.add_file(fn)\n')
	for(i=0;mac_frameworks[i];i++)
		StringAppend(spython,'project.add_file("'+mac_frameworks[i]+'",tree="SDKROOT")\n')
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
		StringAppend(sshell,'xcodebuild -sdk macosx -configuration Release build OTHER_CFLAGS=\'${inherited} -ffast-math -w -I${HOME}/pmenv/include\';')
	else
		StringAppend(sshell,'xcodebuild -sdk macosx -configuration Debug build OTHER_CFLAGS=\'${inherited} -O0 -ffast-math -w -I${HOME}/pmenv/include\';')
	if PRJ.is_release:
		sdirname="Release"
	else
		sdirname="Debug"
	StringAppend(sshell,'mkdir -p download; cd build/'+sdirname+'/; strip '+PRJ.name+'.app/Contents/MacOS/'+PRJ.name+'; tar -czf '+PRJ.name+'.tar.gz '+PRJ.name+'.app; mv '+PRJ.name+'.tar.gz ../../download/;')
	StringAppend(sshell,'exit')
	envssh('mac',sshell)
	cd(PRJ.bin)
	rsync(ssh_addr+':~/_buildtmp/'+sbuildtmp+'/download','.')
	return 1
}

g_action_handlers.run=function(sdir_target){
	if !(PRJ.target=="mac"||PRJ.target=="mac-release"):
		return 0
	sbuildtmp=ReadFileNonLocked(g_work_dir+"/buildtmp_ready")
	if !sbuildtmp:
		Write("error> the project hasn't been built yet")
		exit(1)
	ssh_addr=GetServerSSH('mac')
	sshell=""
	//todo
	if PRJ.is_release:
		//StringAppend(sshell,'open ~/_buildtmp/'+sbuildtmp+'/build/Release/'+PRJ.name+'.app; ')
		StringAppend(sshell,'~/_buildtmp/'+sbuildtmp+'/build/Release/'+PRJ.name+'.app/Contents/MacOS/'+PRJ.name+'; ')
	else
		StringAppend(sshell,'~/_buildtmp/'+sbuildtmp+'/build/Debug/'+PRJ.name+'.app/Contents/MacOS/'+PRJ.name+'; ')
	StringAppend(sshell,'exit')
	envssh('mac',sshell)
}
