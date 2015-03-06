
die=function(){
	throw new Error(Array.prototype.slice.call(arguments,0).join(""))
};

UpdateTo=function(fn_old,fn_new){
	if(!FileExists(fn_new))return 0;
	if(FileExists(fn_old)&&!IsNewerThan(fn_new,fn_old))return 0;
	if(!!shell(["cp",fn_new,fn_old])){
		print("can't update '@1' to '@2'".replace("@1",fn_old).replace("@2",fn_new));
		return 0;
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
	shell(["rsync","-rtpzuv","--chmod=ug=rwX,o=rX",sopt0,'-e','ssh -p'+(port||22)+' ',sopt,src+'/',tar]);
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
g_regexp_chopext=new RegExp("(.*)\\.([^.]*)");
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
		var fnh=sprefix_real+RemovePath(fn);
		UpdateTo(fnh,fn);
	}
};

g_action_handlers.ssh=function(){
	envssh(g_cli_args[0])
};

g_action_handlers.rsync=function(){
	rsync(g_cli_args[0],g_cli_args[1],g_cli_args[2])
};

g_action_handlers.clean=function(){
	shell(["rm","-rf",g_arch+"/pm_tmp"])
	shell(["rm","-rf",g_arch+"/bin"])
};

(function(){
	var fn_config=g_root+"/js/config.json";
	g_config=eval("(function(){"+ReadFile(fn_config)+"})()");
	//handle utilities
	if(g_action_handlers[g_action]){
		//it's a utility, g_json_file is actually a hack arch
		(g_action_handlers[g_action])();
		return {};
	}
	//parse the json
	var s_json=ReadFile(g_json_file);
	g_json=JSON.parse(s_json);
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
		for(var i=0;i<arr.length;i++){
			arr[i]=arr[i].replace(re_envvar,ftranslate_envvar)
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
	var s_jssrc=ReadFile(g_root+"/js/build_"+g_arch+".js")
	if(!s_jssrc){die("I don't know how to build for platform '@1'".replace("@1",g_arch));}
	eval(s_jssrc);
	if(!g_action_handlers[g_action]){die("I don't know how to perform action '@1'".replace("@1",g_action));}
	//run the js files first
	var js_files=g_json.js_files;
	if(js_files){
		for(var i=0;i<js_files.length;i++){
			var fn=SearchForFile(js_files[i]);
			var scode=ReadFile(fn);
			if(!scode){die("unable to find @1".replace("@1",js_files[i]));continue;}
			eval(scode);
		}
	}
	(g_action_handlers[g_action])();
	return {};
})();
