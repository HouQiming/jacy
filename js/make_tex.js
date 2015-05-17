g_action_handlers.make=function(){
	var fname=g_json.input_files[0]
	var ret=fname.match(g_regexp_chopext)
	var fname_pdf=(ret?ret[1]:fname)+".pdf"
	if(IsNewerThan(fname,fname_pdf)){
		var cmdline=["texify","--tex-option=--max-print-line=9999","--pdf"]
		if(g_json.m_latex_sync){
			cmdline.push("--tex-option=--synctex=1")
		}
		cmdline.push(fname)
		shell(cmdline)
	}
};

g_action_handlers.run=function(){
	var fname=g_json.input_files[0]
	var ret=fname.match(g_regexp_chopext)
	var fname_pdf=(ret?ret[1]:fname)+".pdf"
	if(FileExists(fname_pdf)){
		shell(["SumatraPDF","-reuse-instance",fname_pdf,"-inverse-search",'"'+g_json.m_editor_exe+'" "%f" --seek %l',"-forward-search",fname,g_json.m_line])
	}
};
