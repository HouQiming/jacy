(function(){
	var ssh_addr,ssh_port;
	shell(["jc","-alinux64","-brelease","main.jc"])
	shell(["jc","-alinux64","-brelease","test/pmjs.jc"])
	var ssh_addr=GetServerSSH('linux');
	var ssh_port=GetPortSSH('linux');
	shell(["rsync","-rtzuv","--chmod=ug=rwX,o=rX",'-q','-e','ssh -p'+(ssh_port||22)+' ',
		'./js',ssh_addr+':~/jc/']);
	shell(["rsync","-rtzuv","--chmod=ug=rwX,o=rX",'-q','-e','ssh -p'+(ssh_port||22)+' ',
		'./units',ssh_addr+':~/jc/']);
	shell(["ssh","-t",'-P'+(ssh_port||22),ssh_addr,"mkdir -p ~/jc/bin/linux64_release;exit"]);
	shell(["scp",'-P'+(ssh_port||22),
		'./bin/linux64_release/main',ssh_addr+':~/jc/bin/linux64_release/jc']);
	shell(["scp",'-P'+(ssh_port||22),
		'./test/bin/linux64_release/pmjs',ssh_addr+':~/jc/bin/linux64_release/pmjs']);
	shell(["ssh","-t",'-P'+(ssh_port||22),ssh_addr,"chmod +x ~/jc/bin/linux64_release/*;exit"]);
})()
