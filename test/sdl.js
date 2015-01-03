(function(){with(SDL){
	var hwnd=SDL_CreateWindow("Hello world");
	for(;;){
		var event=SDL_WaitEvent();
		if(event.type==SDL_QUIT){break;}
		switch(event.type){
		case SDL_WINDOWEVENT:
			//todo: generic event callbacks - user-provided testing function on the event
			switch(event.event){
			case SDL_WINDOWEVENT_SHOWN:
			case SDL_WINDOWEVENT_EXPOSED:
			case SDL_WINDOWEVENT_RESIZED:
				GL_Begin(event.windowID)
				render()
				GL_End(event.windowID)
				break;
			}
			break
		}
		//print(JSON.stringify(event))
	}
}})();
