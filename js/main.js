
die=function(){
	throw new Error(Array.prototype.slice.call(arguments,0).join(""))
};

UpdateTo=function(fn_old,fn_new,use_symlink){
	if(!FileExists(fn_new))return 0;
	if(FileExists(fn_old)&&!IsNewerThan(fn_new,fn_old))return 0;
	if(use_symlink){
		shell(["rm","-f",fn_old])
		if(!!shell(["cp","-s","`realpath "+fn_new+"`",fn_old])){
			print("can't update '@1' to '@2'".replace("@1",fn_old).replace("@2",fn_new));
			return 0;
		}
	}else{
		if(!!shell(["cp",fn_new,fn_old])){
			print("can't update '@1' to '@2'".replace("@1",fn_old).replace("@2",fn_new));
			return 0;
		}
	}
	return 1;
};

UpdateToCLike=function(fn_old,fn_new,use_symlink){
	if(!FileExists(fn_new))return 0;
	if(FileExists(fn_old)&&!IsNewerThan(fn_new,fn_old))return 0;
	if(use_symlink){
		shell(["rm","-f",fn_old])
		if(!!shell(["cp","-s","`realpath "+fn_new+"`",fn_old])){
			print("can't update '@1' to '@2'".replace("@1",fn_old).replace("@2",fn_new));
			return 0;
		}
	}else{
		CreateFile(fn_old,['#line 1 "',fn_new,'"\n',ReadFile(fn_new)].join(""))
	}
	return 1;
};

CreateIfDifferent=function(fn,scontent){
	if(FileExists(fn)&&ReadFile(fn)==scontent){return;}
	CreateFile(fn,scontent);
};

mkdir=function(sdir){
	if(DirExists(sdir))return;
	if(g_current_arch=="win32"||g_current_arch=="win64"){
		shell(["mkdir",sdir.replace(new RegExp("\\/","g"),"\\")]);
	}else{
		shell(["mkdir","-p",sdir]);
	}
};

GetServerSSH=function(server){
	var ret=g_config["SSH_SERVER_"+server.toUpperCase()];
	var fn_config=g_root+"/js/config.json";
	if(!ret){
		print("can't find configuration entry @1, please fill out its corresponding entry in @2".replace("@1","SSH_SERVER_"+server).replace("@2",fn_config));
		die("can't find the SSH server");
	}
	return ret;
};

GetPortSSH=function(server){
	return (g_config["SSH_PORT_"+server.toUpperCase()]||22);
};

rsync=function(src,tar,port){
	var sopt,sopt0;
	if(g_json.verbose){
		sopt='--progress';
		sopt0='-v';
	}else{
		sopt='';
		sopt0='-q';
	}
	//shell(["rsync","-rtpzuv","--chmod=ug=rwX,o=rX",sopt0,'-e','ssh -p'+(port||22)+' ',sopt,src+'/',tar]);
	shell(["rsync","-rtzuv","--chmod=ug=rwX,o=rX",sopt0,'-e','ssh -p'+(port||22)+' ',sopt,src+'/',tar]);
};

envssh=function(server,cmd){
	var ssh_addr=GetServerSSH(server)
	var ssh_port=GetPortSSH(server)
	if(cmd){
		shell(["ssh","-p"+ssh_port,"-t",ssh_addr,cmd])
	}else{
		shell(["ansicon","ssh","-p"+ssh_port,ssh_addr])
	}
};

/////////////////////////////////////
//wait - we don't need a project configuration file!
//we only need the json! and the platform scripts
//there is no command - we build the json file
g_action_handlers={};
g_json={};
g_config={};
g_arch=g_current_arch;
g_build="debug";
g_regexp_chopdir=new RegExp("(.*)[/\\\\]([^/\\\\]*)");
g_regexp_chopext=new RegExp("(.*)\\.([^/\\\\.]*)");
g_is64=0;
g_search_paths=[g_root+"/js"];

GetMainFileName=function(fname){
	var ret=fname.match(g_regexp_chopdir);
	var main_name=null;
	if(!ret){
		main_name=fname;
	}else{
		main_name=ret[2];
	}
	ret=main_name.match(g_regexp_chopext);
	if(ret){
		main_name=ret[1];
	}
	return main_name;
};

RemoveExtension=function(fname){
	var ret=fname.match(g_regexp_chopext);
	if(ret){
		fname=ret[1];
	}
	return fname;
};

RemovePath=function(fname){
	var ret=fname.match(g_regexp_chopdir);
	var main_name=null;
	if(!ret){
		main_name=fname;
	}else{
		main_name=ret[2];
	}
	return main_name;
};

SearchForFile=function(fn){
	if(FileExists(fn)){return fn};
	if(FileExists(g_base_dir+"/"+fn)){return g_base_dir+"/"+fn};
	for(var i=0;i<g_search_paths.length;i++){
		var fnx=g_search_paths[i]+"/"+fn;
		if(FileExists(fnx)){return fnx};
	}
	return fn;
};

CopyToWorkDir=function(c_files,sprefix){
	if(!c_files){return;}
	var sprefix_real=g_work_dir+"/"+(sprefix||"");
	for(var i=0;i<c_files.length;i++){
		var fn=SearchForFile(c_files[i]);
		//var fnh=sprefix_real+RemovePath(fn);
		var fnh=sprefix_real+c_files[i];
		UpdateTo(fnh,fn);
	}
};

var g_regexp_is_absolute_path=new RegExp("^(([a-zA-Z]:[\\\\/])|[\\\\/]|(pm_tmp/)|(.*/pm_tmp/)).*$");
CreateProjectForFileSet=function(is_c_like,c_files,s_target_dir,use_symlink){
	if(!c_files){return undefined;}
	var ret=[];
	for(var i=0;i<c_files.length;i++){
		var fn_relative=c_files[i];
		var fn=SearchForFile(fn_relative);
		if(fn_relative.match(g_regexp_is_absolute_path)){
			fn_relative=RemovePath(fn_relative);
		}
		var subdirs=fn_relative.split("/")
		if(subdirs.length>1){
			mkdir(s_target_dir+subdirs.slice(0,subdirs.length-1).join("/"));
		}
		var fnh=s_target_dir+fn_relative;
		if(is_c_like){
			UpdateToCLike(fnh,fn,use_symlink);
		}else{
			UpdateTo(fnh,fn,use_symlink);
		}
		ret.push(fn_relative)
	}
	return ret;
};

CreateProjectForStandardFiles=function(s_target_dir,use_symlink){
	CreateProjectForFileSet(1,g_json.h_files,s_target_dir,use_symlink);
	CreateProjectForFileSet(0,g_json.lib_files,s_target_dir,use_symlink);
	return CreateProjectForFileSet(1,g_json.c_files,s_target_dir,use_symlink);
};

g_action_handlers.ssh=function(){
	envssh(g_cli_args[0])
};

g_action_handlers.rsync=function(){
	rsync(g_cli_args[0],g_cli_args[1],g_cli_args[2])
};

g_action_handlers.envpush=function(){
	var server=g_cli_args[0];
	var ssh_addr=GetServerSSH(server)
	var s_original_dir=pwd()
	cd(g_root+"/osslib/"+server)
	rsync('./pmenv',ssh_addr+':~/pmenv')
	cd(s_original_dir)
	UpdateTo(g_root+"/osslib/include/wrapper_defines.h",g_root+"/units/wrapper_defines.h")
	cd(g_root+"/osslib")
	rsync('./include',ssh_addr+':~/pmenv/include')
	cd(s_original_dir)
}

g_action_handlers.envpull=function(){
	var server=g_cli_args[0];
	var ssh_addr=GetServerSSH(server)
	var dir0=pwd()
	cd(g_root+"/osslib/"+server)
	rsync(ssh_addr+':~/pmenv','./pmenv')
	cd(dir0)
}

g_action_handlers.clean=function(){
	if(!g_cli_args[0]){
		die("please specify a project directory")
	}
	shell(["rm","-rf",g_cli_args[0]+"/pm_tmp"])
	shell(["rm","-rf",g_cli_args[0]+"/bin"])
};

g_action_handlers.runjs=function(){
	if(!g_cli_args[0]){
		die("please specify a js file")
	}
	var scode=ReadFile(g_cli_args[0]);
	if(!scode){
		die("unable to find @1".replace("@1",g_cli_args[0]));
	}
	debugEval(scode,g_cli_args[0]);
};

(function(){
	var fn_config=g_root+"/js/config.json";
	g_config=eval("(function(){"+ReadFile(fn_config)+"})()");
	g_config.ROOT=g_root
	//handle utilities
	if(g_action_handlers[g_action]){
		//it's a utility, g_json_file is actually a hack arch
		(g_action_handlers[g_action])();
		return {};
	}
	//parse the json
	var s_json=ReadFile(g_json_file);
	g_json=JSON.parse(s_json);
	if(g_json.delete_json_file){
		shell(["rm",g_json_file])
	}
	if(g_json.include_json){
		//the length may CHANGE every iteration
		for(var i=0;i<g_json.include_json.length;i++){
			var fn=SearchForFile(g_json.include_json[i]);
			var scode=ReadFile(fn);
			if(!scode){die("unable to find @1".replace("@1",g_json.include_json[i]));continue;}
			var obj=JSON.parse(scode)
			for(var j in obj){
				if(!Array.isArray(obj[j])){
					obj[j]=[obj[j]];
				}
				var tmp={}
				var a0=(g_json[j]||[]),a1=obj[j]
				for(var k=0;k<a0.length;k++){
					tmp[a0[k]]=1;
				}
				for(var k=0;k<a1.length;k++){
					tmp[a1[k]]=1;
				}
				var a2=[]
				for(var ret in tmp){
					a2.push(ret)
				}
				g_json[j]=a2;
			}
		}
	}
	//translate the configuration variables
	var re_envvar=new RegExp('\\$\\{([0-9a-zA-Z$_]+)\\}',"g")
	var ftranslate_envvar=function(smatch,svname){
		var value=g_config[svname];
		if(!value){
			print("unresolved configuration variable @1, please fill out its corresponding entry in @2".replace("@1",svname).replace("@2",fn_config));
			die("unresolved configuration variable");
		}
		return value;
	};
	for(var name in g_json){
		var arr=g_json[name];
		if(Array.isArray(arr)){
			for(var i=0;i<arr.length;i++){
				if(typeof arr[i]=='string'){
					arr[i]=arr[i].replace(re_envvar,ftranslate_envvar)
				}
			}
		}
	}
	//create the necessary globals
	g_build=g_json.Platform_BUILD[0];
	var ret=g_json.input_files[0].match(g_regexp_chopdir);
	var base_dir,main_name;
	if(!ret){
		base_dir=".";
		main_name=g_json.input_files[0];
	}else{
		base_dir=ret[1];
		main_name=ret[2];
	}
	ret=main_name.match(g_regexp_chopext);
	if(ret){
		main_name=ret[1];
	}
	g_main_name=main_name;
	g_arch=g_json.Platform_ARCH[0];
	var s_dirname=g_build=="debug"?g_arch:g_arch+"_"+g_build;
	g_work_dir=base_dir+"/pm_tmp/"+s_dirname+"/"+main_name;
	g_bin_dir=base_dir+"/bin/"+s_dirname;
	g_relative_dir_name=s_dirname
	g_base_dir=base_dir;
	mkdir(g_work_dir);
	mkdir(g_bin_dir);
	//translate the search paths
	if(g_json.search_paths){
		for(var i=0;i<g_json.search_paths.length;i++){
			var path=g_json.search_paths[i];
			if(!g_config[path]){
				print("unresolved search path @1, please fill out its corresponding entry in @2".replace("@1",path).replace("@2",fn_config));
				die("unresolved search path");
				return;
			}
			g_search_paths.push(g_config[path]);
		}
	}
	if(g_config.DEFAULT_SEARCH_PATHS){
		for(var i=0;i<g_config.DEFAULT_SEARCH_PATHS.length;i++){
			g_search_paths.push(g_config.DEFAULT_SEARCH_PATHS[i]);
		}
	}
	//locate the per-arch script and eval that
	if(g_json.c_files){
		var fn=g_root+"/js/build_"+g_arch+".js";
		var s_jssrc=ReadFile(fn)
		if(!s_jssrc){die("I don't know how to build for platform '@1'".replace("@1",g_arch));}
		debugEval(s_jssrc,fn);
	}
	//run the js files first
	var include_js=g_json.include_js;
	if(include_js){
		for(var i=0;i<include_js.length;i++){
			var fn=SearchForFile(include_js[i]);
			var scode=ReadFile(fn);
			if(!scode){die("unable to find @1".replace("@1",include_js[i]));continue;}
			debugEval(scode,fn);
		}
	}
	if(!g_action_handlers[g_action]){die("I don't know how to perform action '@1'".replace("@1",g_action));}
	(g_action_handlers[g_action])();
	if(g_json.pause_after_run){
		if(g_current_arch=="win32"||g_current_arch=="win64"){
			shell(["pause"])
		}else{
			shell(["read","-p","Press any key to continue..."])
		}
	}
	return {};
})();
