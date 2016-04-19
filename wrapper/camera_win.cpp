#pragma comment(lib, "strmiids")
#pragma comment(lib, "ole32")
#pragma comment(lib, "oleaut32")

#include <windows.h>
#include <dshow.h>
#include <stdlib.h>
#include "wrapper_defines.h"
#include "SDL.h"

//#define DEBUG_SHOW_RENDERER

interface ISampleGrabberCB : public IUnknown
{
	virtual STDMETHODIMP SampleCB( double SampleTime, IMediaSample *pSample ) = 0;
	virtual STDMETHODIMP BufferCB( double SampleTime, BYTE *pBuffer, long BufferLen ) = 0;
};



interface ISampleGrabber : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE SetOneShot( BOOL OneShot ) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetMediaType( const AM_MEDIA_TYPE *pType ) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType( AM_MEDIA_TYPE *pType ) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetBufferSamples( BOOL BufferThem ) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer( long *pBufferSize, long *pBuffer ) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentSample( IMediaSample **ppSample ) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetCallback( ISampleGrabberCB *pCallback, long WhichMethodToCallback ) = 0;
};


static const IID IID_ISampleGrabber = { 0x6B652FFF, 0x11FE, 0x4fce, { 0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F } };
static const IID IID_ISampleGrabberCB = { 0x0579154A, 0x2B53, 0x4994, { 0xB0, 0xD0, 0xE7, 0x73, 0x14, 0x8E, 0xFF, 0x85 } };
static const CLSID CLSID_SampleGrabber = { 0xC1F400A0, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
static const GUID CLSID_NullRenderer = { 0xC1F400A4, 0x3F08, 0x11D3,{0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37}};

//refactored version of www.rohitab.com/discuss/topic/34389-directshow-webcam-capture-class-c/
#define MAX_DEVICES 8
class DShowCallbackHandler : public ISampleGrabberCB{
public:
	virtual HRESULT __stdcall SampleCB(double time, IMediaSample* sample);
	virtual HRESULT __stdcall BufferCB(double time, BYTE* buffer, long len);
	virtual HRESULT __stdcall QueryInterface( REFIID iid, LPVOID *ppv );
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();
	int id;
};
typedef unsigned int u32;
struct TCamera{
	//we ignore the user-specified size
	int m_is_valid;
	IFilterGraph2*			graph;
	ICaptureGraphBuilder2*	capture;
	IMediaControl*			control;
	IBaseFilter*	source_filter;
	IBaseFilter*			nullrenderer;
	IBaseFilter*			samplegrabberfilter;
	ISampleGrabber*			samplegrabber;
	DShowCallbackHandler* cb;
	AM_MEDIA_TYPE mt;
	////////
	int m_is_on;
	SDL_mutex* m_cam_mutex;
	////////
	u32* m_image_front;
	u32* m_image_back;
	int m_w,m_h,m_image_ready;
};
static int g_inited=0;
static int g_n_cameras=0;
static TCamera g_cameras[MAX_DEVICES];

static int initDShowStuff(){
	if(g_inited){return g_inited;}
	g_inited=-1;
	//////////////
	CoInitialize(NULL);
	HRESULT hr=0;
	//
	ICreateDevEnum*		dev_enum=NULL;
	IEnumMoniker*		enum_moniker=NULL;
	IMoniker*			moniker=NULL;
	IPropertyBag*		pbag=NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,CLSCTX_INPROC_SERVER,IID_ICreateDevEnum,(void**) &dev_enum);
	if (hr < 0) {return -1;}
	hr = dev_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&enum_moniker,NULL);
	if (hr < 0) {return -1;}
	if (hr == S_FALSE) {return -1;}
	unsigned long dev_count=0;
	enum_moniker->Next(MAX_DEVICES, &moniker, &dev_count);
	memset(g_cameras,0,sizeof(g_cameras));
	g_n_cameras=(int)dev_count;
	for (int i=0; i<(int)dev_count; i++){
		//we could afford leaks on failure - it's a constant amount anyway, and it's rather unlikely
		////////////////////////
		//source filter
		hr = CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC_SERVER,IID_IFilterGraph2,(void**) &g_cameras[i].graph);
		if (hr < 0) {continue;}
		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2,NULL,CLSCTX_INPROC_SERVER,IID_ICaptureGraphBuilder2,(void**) &g_cameras[i].capture);
		if (hr < 0) {continue;}
		hr = g_cameras[i].graph->QueryInterface(IID_IMediaControl, (void**) &g_cameras[i].control);
		if (hr < 0) {continue;}
		g_cameras[i].capture->SetFiltergraph(g_cameras[i].graph);
		hr = g_cameras[i].graph->AddSourceFilterForMoniker(moniker+i, 0, L"Source", &g_cameras[i].source_filter);
		if (hr != S_OK){continue;}
		moniker[i].Release();
		///////////////////////
		//sample grabber
		hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,IID_IBaseFilter,(void**)&g_cameras[i].samplegrabberfilter);
		if (hr < 0) {continue;}
		hr = g_cameras[i].graph->AddFilter(g_cameras[i].samplegrabberfilter, L"Sample Grabber");
		if (hr < 0) {continue;}
		hr = g_cameras[i].samplegrabberfilter->QueryInterface(IID_ISampleGrabber, (void**)&g_cameras[i].samplegrabber);
		if (hr != S_OK) {continue;}
		//set the media type
		memset(&g_cameras[i].mt, 0, sizeof(AM_MEDIA_TYPE));
		g_cameras[i].mt.majortype	= MEDIATYPE_Video;
		g_cameras[i].mt.subtype		= MEDIASUBTYPE_RGB24;
		//original comment:
		// setting the above to 32 bits fails consecutive Select for some reason
		// and only sends one single callback (flush from previous one ???)
		// must be deeper problem. 24 bpp seems to work fine for now.
		hr = g_cameras[i].samplegrabber->SetMediaType(&g_cameras[i].mt);
		if (hr != S_OK) {continue;}
		g_cameras[i].cb=new DShowCallbackHandler;
		g_cameras[i].cb->id=i;
		g_cameras[i].samplegrabber->SetCallback(g_cameras[i].cb,0);
		///////////////////////
		//set a null renderer
		hr = CoCreateInstance(CLSID_NullRenderer,NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,(void**) &g_cameras[i].nullrenderer);
		if (hr < 0) {continue;}
		g_cameras[i].graph->AddFilter(g_cameras[i].nullrenderer, L"Null Renderer");
		#ifdef DEBUG_SHOW_RENDERER
		hr = g_cameras[i].capture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, g_cameras[i].source_filter, g_cameras[i].samplegrabberfilter, NULL);
		#else
		hr = g_cameras[i].capture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, g_cameras[i].source_filter, g_cameras[i].samplegrabberfilter, g_cameras[i].nullrenderer);
		#endif
		if (hr != S_OK) {continue;}
		hr = g_cameras[i].samplegrabber->GetConnectedMediaType(&g_cameras[i].mt);
		if (hr != S_OK) {continue;}
		if (!(g_cameras[i].mt.formattype == FORMAT_VideoInfo&&g_cameras[i].mt.cbFormat >= sizeof(VIDEOINFOHEADER))){
			continue;
		}
		///////////////////////
		//start streaming
		LONGLONG stop=0x7FFFFFFFFFFFFFFFLL;
		hr = g_cameras[i].capture->ControlStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, g_cameras[i].source_filter, NULL, &stop, 1,2);
		if (hr < 0) {continue;}
		///////////////////////
		//the camera seems valid
		g_cameras[i].m_cam_mutex=SDL_CreateMutex();
		g_cameras[i].m_is_valid=1;
	}
	return 1;
}

EXPORT int osal_TurnOnCamera(int cam_id,int w,int h,int fps){
	if(initDShowStuff()<0){
		return 0;
	}
	if(!((unsigned int)cam_id<(unsigned int)g_n_cameras)||!g_cameras[cam_id].m_is_valid){
		return 0;
	}
	//ignore w,h,fps
	if(g_cameras[cam_id].m_is_on){return 1;}
	HRESULT hr=0;
	hr = g_cameras[cam_id].control->Run();
	if (hr < 0) {return 0;}
	g_cameras[cam_id].m_is_on=1;
	return 1;
}

EXPORT int osal_TurnOffCamera(int cam_id){
	if(!((unsigned int)cam_id<(unsigned int)g_n_cameras)||!g_cameras[cam_id].m_is_valid){
		return 0;
	}
	//ignore w,h,fps
	if(!g_cameras[cam_id].m_is_on){return 1;}
	HRESULT hr=0;
	hr = g_cameras[cam_id].control->StopWhenReady();
	if (hr < 0) {return 0;}
	g_cameras[cam_id].m_is_on=0;
	return 1;
}

EXPORT u32* osal_GetCameraImage(int cam_id, int* pw,int* ph){
	if((unsigned int)cam_id>=(unsigned int)g_n_cameras||!g_cameras[cam_id].m_is_valid||!g_cameras[cam_id].m_is_on)return NULL;
	TCamera* cam=&g_cameras[cam_id];
	SDL_LockMutex(cam->m_cam_mutex);
	if(cam->m_image_ready){
		u32* ret=cam->m_image_back;
		cam->m_image_back=cam->m_image_front;
		cam->m_image_front=ret;
		cam->m_image_ready=0;
		SDL_UnlockMutex(cam->m_cam_mutex);
		*pw=cam->m_w;
		*ph=cam->m_h;
		return ret;
	}else{
		SDL_UnlockMutex(cam->m_cam_mutex);
		return NULL;
	}
}

EXPORT int osal_GetFrontCameraId(){
	return 0;
}

EXPORT int osal_GetBackCameraId(){
	return 0;
}

//#include <stdio.h>
HRESULT DShowCallbackHandler::SampleCB(double time, IMediaSample *sample){
	HRESULT hr=0;
	unsigned char* buffer=NULL;
	hr = sample->GetPointer((BYTE**)&buffer);
	if (hr != S_OK){return S_OK;}
	//////////////
	AM_MEDIA_TYPE* mt=&g_cameras[id].mt;
	VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(mt->pbFormat);
	int w=pVih->bmiHeader.biWidth;
	int h=pVih->bmiHeader.biHeight;
	int n=sample->GetActualDataLength();
	if(n!=w*h*3){return S_OK;}
	//real callback content
	TCamera* cam=g_cameras+id;
	SDL_LockMutex(cam->m_cam_mutex);
	if(!cam->m_is_on){
		SDL_UnlockMutex(cam->m_cam_mutex);
		return S_OK;
	}
	if(cam->m_w!=w||cam->m_h!=h){
		if(cam->m_image_back){
			free(cam->m_image_back);
			cam->m_image_back=NULL;
		}
		if(cam->m_image_front){
			free(cam->m_image_front);
			cam->m_image_front=NULL;
		}
		cam->m_w=w;
		cam->m_h=h;
	}
	if(!cam->m_image_back){
		cam->m_image_back=(u32*)malloc(4*w*h);
	}
	u32* ret=cam->m_image_back;
	for(int i=0;i<h;i++){
		unsigned char* p=buffer+(h-1-i)*(w*3);
		for(int j=0;j<w;j++){
			ret[j]=u32(p[2])+(u32(p[1])<<8)+(u32(p[0])<<16)+0xff000000u;
			p+=3;
		}
		ret+=w;
	}
	cam->m_w=w;
	cam->m_h=h;
	cam->m_image_ready=1;
	SDL_UnlockMutex(cam->m_cam_mutex);
	return S_OK;
}

HRESULT DShowCallbackHandler::BufferCB(double time, BYTE *buffer, long len){
	return S_OK;
}

HRESULT DShowCallbackHandler::QueryInterface(const IID &iid, LPVOID *ppv){
	if( iid == IID_ISampleGrabberCB || iid == IID_IUnknown ){
		*ppv = (void *) static_cast<ISampleGrabberCB*>( this );
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG DShowCallbackHandler::AddRef(){
	return 1;
}

ULONG DShowCallbackHandler::Release(){
	return 2;
}
