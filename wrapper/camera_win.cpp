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
	DShowCallbackHandler*	cb;
	AM_MEDIA_TYPE mt;
	////////
	int m_is_on,m_has_been_on;
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
	if(g_inited){ return g_inited; }
	g_inited=-1;
	//////////////
	// initialize COM
	HRESULT hr = CoInitializeEx(NULL,COINIT_MULTITHREADED);
	/*
	QM: we SHOULD NOT check COM failures - someone may have initialized it in a different mode before us.
	In that case, we should just continue silently.
	*/
	/*
	if (hr != S_OK) {
		if (hr == S_FALSE) { CoUninitialize(); }// balance the init 
		return -1;
	}
	*/
	hr = 0;
	ICreateDevEnum*		dev_enum=NULL;
	IEnumMoniker*		enum_moniker=NULL;
	IMoniker*			moniker[MAX_DEVICES] = {NULL};
	// to get a list
	hr = CoCreateInstance(
		CLSID_SystemDeviceEnum,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum,
		(void**) &dev_enum
	);
	if (hr < 0) { CoUninitialize(); return -1; }
	// to get a list of Video input device
	hr = dev_enum->CreateClassEnumerator(
		CLSID_VideoInputDeviceCategory,
		&enum_moniker,
		NULL
	);
	if (hr < 0) { CoUninitialize(); return -1; }
	if (hr == S_FALSE) { CoUninitialize();  return -1; }
	ULONG dev_count=0;
	enum_moniker->Next(MAX_DEVICES, moniker, &dev_count);
	memset(g_cameras,0,sizeof(g_cameras));
	g_n_cameras=(int)dev_count;
	// set the camera devices found.
	for (int i=0; i<(int)dev_count; i++){
		//we could afford leaks on failure - it's a constant amount anyway, and it's rather unlikely
		////////////////////////
		// Create the Capture Graph builder 
		hr = CoCreateInstance(
			CLSID_CaptureGraphBuilder2,NULL,
			CLSCTX_INPROC_SERVER,
			IID_ICaptureGraphBuilder2,
			(void**) &g_cameras[i].capture
		);
		if (hr < 0) {continue;}
		// Create source filter
		hr = CoCreateInstance(
			CLSID_FilterGraph, NULL,
			CLSCTX_INPROC_SERVER,
			IID_IFilterGraph2,
			(void**)&g_cameras[i].graph
		);
		if (hr < 0) { continue; }
		// media control
		hr = g_cameras[i].graph->QueryInterface(
			IID_IMediaControl, 
			(void**) &g_cameras[i].control
		);
		if (hr < 0) {continue;}
		hr = g_cameras[i].graph->AddSourceFilterForMoniker(
			moniker[i], 0, L"Source", 
			&g_cameras[i].source_filter
		);
		if (hr != S_OK){continue;}
		// pin the source filter to the capture 
		g_cameras[i].capture->SetFiltergraph(g_cameras[i].graph);
		//moniker[i].Release();

		///////////////////////
		//sample grabber
		hr = CoCreateInstance(
			CLSID_SampleGrabber, NULL, 
			CLSCTX_INPROC_SERVER,
			IID_IBaseFilter,
			(void**)&g_cameras[i].samplegrabberfilter
		);
		if (hr < 0) {continue;}
		hr = g_cameras[i].graph->AddFilter(
			g_cameras[i].samplegrabberfilter, 
			L"Sample Grabber"
		);
		if (hr < 0) {continue;}
		hr = g_cameras[i].samplegrabberfilter->QueryInterface(
			IID_ISampleGrabber,
			(void**)&g_cameras[i].samplegrabber
		);
		if (hr != S_OK) {continue;}
		//set the media type
		memset(&g_cameras[i].mt, 0, sizeof(AM_MEDIA_TYPE));
		g_cameras[i].mt.majortype	= MEDIATYPE_Video;
		g_cameras[i].mt.subtype		= MEDIASUBTYPE_RGB24;
		//latest comment:
		// set subtype as YUV I420, to support more devices.
		/*
		g_cameras[i].mt.subtype		= MEDIASUBTYPE_RGB24;
		//original comment:
		// setting the above to 32 bits fails consecutive Select for some reason
		// and only sends one single callback (flush from previous one ???)
		// must be deeper problem. 24 bpp seems to work fine for now.
		*/
		hr = g_cameras[i].samplegrabber->SetMediaType(&g_cameras[i].mt);
		if (hr != S_OK) { printf("fail to set Mediatype."); continue; }

		g_cameras[i].cb=new DShowCallbackHandler;
		g_cameras[i].cb->id=i;
		g_cameras[i].samplegrabber->SetCallback(g_cameras[i].cb,1);

		///////////////////////
		//set a null renderer, discards every sample it receives, 
		//without displaying or rendering the sample data.
		hr = CoCreateInstance(CLSID_NullRenderer,NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,(void**) &g_cameras[i].nullrenderer);
		if (hr < 0) {continue;}
		g_cameras[i].graph->AddFilter(g_cameras[i].nullrenderer, L"Null Renderer");
		
		///////////////////////
		//the camera seems valid
		g_cameras[i].m_cam_mutex=SDL_CreateMutex();
		g_cameras[i].m_is_valid=1;
	}
	g_inited=1;
	CoUninitialize();
	return 1;
}

static void _FreeMediaType(AM_MEDIA_TYPE& mt){
    if (mt.cbFormat != 0)
    {
        CoTaskMemFree((PVOID)mt.pbFormat);
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL)
    {
        // pUnk should not be used.
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}


static void _DeleteMediaType(AM_MEDIA_TYPE *pmt){
    if (pmt != NULL)
    {
        _FreeMediaType(*pmt); 
        CoTaskMemFree(pmt);
    }
}

static void SetResolution(int cam_id,int w,int h,int fps){
	IAMStreamConfig *pConfig = NULL;
	HRESULT hr=0;
	hr = g_cameras[cam_id].capture->FindInterface(
		&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Video,
		g_cameras[cam_id].source_filter,
		IID_IAMStreamConfig,
		(void**)&pConfig);
	if(FAILED(hr)||!pConfig){return;}

	int iBest=-1,iMax=-1;
	int h_best=0;
	int h_max=0;
	
	int iCount = 0, iSize = 0;
	hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if(FAILED(hr)){return;}
	

	AM_MEDIA_TYPE *pmt=NULL;
	VIDEO_STREAM_CONFIG_CAPS scc;
	memset(&scc,0,sizeof(scc));
	
	// Check the size to make sure we pass in the correct structure.
	if (iSize != sizeof(VIDEO_STREAM_CONFIG_CAPS)){return;}

	// Use the video capabilities structure.
	// Find the largest video that fits in the desired width
	for (int iFormat = 0; iFormat < iCount; iFormat++)
	{
		pmt=NULL;
		hr = pConfig->GetStreamCaps(iFormat, &pmt, (BYTE*)&scc);
		if (!SUCCEEDED(hr)){continue;}
		if (h_max < scc.MaxOutputSize.cy){
			h_max = scc.MaxOutputSize.cy;
			iMax = iFormat;
		}
		if(h<=scc.MaxOutputSize.cy&&(!h_best||h_best>scc.MaxOutputSize.cy)){
			h_best=scc.MaxOutputSize.cy;
			iBest = iFormat;
		}
		_DeleteMediaType(pmt);
		pmt=NULL;
	}
	if(iBest<0){
		iBest=iMax;
	}
	if(iBest<0){
		iBest=0;
	}

	// Get the resulting video capability
	pmt=NULL;
	hr = pConfig->GetStreamCaps(iBest, &pmt, (BYTE*)&scc);
	if (!SUCCEEDED(hr)){return;}
	// default capture format, reusing the space
	_DeleteMediaType(pmt);pmt=NULL;
	pConfig->GetFormat(&pmt);
	if (pmt&&pmt->formattype == FORMAT_VideoInfo) {
		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
		pvi->bmiHeader.biWidth = scc.MaxOutputSize.cx;
		pvi->bmiHeader.biHeight = scc.MaxOutputSize.cy;
		hr = pConfig->SetFormat(pmt);
		_DeleteMediaType(pmt);pmt=NULL;
	}
	pConfig->Release();
	pConfig=NULL;
}

EXPORT int osal_TurnOnCamera(int cam_id,int w,int h,int fps){
	HRESULT hr=0;
	if(initDShowStuff()<0){
		return 0;
	}
	if(!((unsigned int)cam_id<(unsigned int)g_n_cameras)|| // id goes out the device no
	   !g_cameras[cam_id].m_is_valid){ // not valid
		return 0;
	}

	//ignore w,h,fps
	if(g_cameras[cam_id].m_is_on){return 1;}
	SetResolution(cam_id,w,h,fps);
	if(!g_cameras[cam_id].m_has_been_on){
		g_cameras[cam_id].m_has_been_on=1;
		//////////////////
		//g_cameras[cam_id].graph->RemoveFilter(g_cameras[cam_id].source_filter);
		//g_cameras[cam_id].graph->RemoveFilter(g_cameras[cam_id].samplegrabberfilter);
		//g_cameras[cam_id].graph->AddFilter(g_cameras[cam_id].samplegrabberfilter,L"Sample Grabber");
		//g_cameras[cam_id].graph->AddFilter(g_cameras[cam_id].source_filter, L"Source");
		#ifdef DEBUG_SHOW_RENDERER
		hr = g_cameras[cam_id].capture->RenderStream(
			&PIN_CATEGORY_CAPTURE, 
			&MEDIATYPE_Video, 
			g_cameras[cam_id].source_filter, 
			g_cameras[cam_id].samplegrabberfilter, 
			NULL
		);
		#else
		hr = g_cameras[cam_id].capture->RenderStream(
			&PIN_CATEGORY_CAPTURE, 
			&MEDIATYPE_Video, 
			g_cameras[cam_id].source_filter, 
			g_cameras[cam_id].samplegrabberfilter, 
			g_cameras[cam_id].nullrenderer
		);
		#endif
		if (hr != S_OK) {return 0;}
	}
	// get the media type of input pin
	memset(&g_cameras[cam_id].mt, 0, sizeof(AM_MEDIA_TYPE));
	hr = g_cameras[cam_id].samplegrabber->GetConnectedMediaType(&g_cameras[cam_id].mt);
	if (hr != S_OK) {return 0;}
	if (!(g_cameras[cam_id].mt.formattype == FORMAT_VideoInfo&&g_cameras[cam_id].mt.cbFormat >= sizeof(VIDEOINFOHEADER))){
		return 0;
	}
	//////////////////
	LONGLONG start=0,stop=0x7FFFFFFFFFFFFFFFLL;
	hr = g_cameras[cam_id].capture->ControlStream(
		&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Video,
		g_cameras[cam_id].source_filter,
		NULL, &stop, 1, 2
	);
	if (hr < 0) {return 0;}
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
	//LONGLONG start=0,stop=0x7FFFFFFFFFFFFFFFLL;
	//g_cameras[cam_id].capture->ControlStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, g_cameras[cam_id].source_filter, NULL, &stop, 1,2);
	//g_cameras[cam_id].control->Run();
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
	return S_OK;
}

HRESULT DShowCallbackHandler::BufferCB(double time, BYTE *buffer, long len){
	HRESULT hr=0;
	//////////////
	AM_MEDIA_TYPE* mt=&g_cameras[id].mt;
	VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(mt->pbFormat);
	int w=pVih->bmiHeader.biWidth;
	int h=pVih->bmiHeader.biHeight;
	if(len!=w*h*3){return S_OK;}
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
	unsigned char *p = buffer + (h - 1) * w * 3;
	int delta_p = -w * 3 * 2;
	for(int i=0;i<h;i++){
		for(int j=0;j<w;j++){
			ret[j]=u32(p[2])+(u32(p[1])<<8)+(u32(p[0])<<16)+0xff000000u;
			p+=3;
		}
		ret+=w;
		p += delta_p;
	}
	cam->m_w=w;
	cam->m_h=h;
	cam->m_image_ready=1;
	SDL_UnlockMutex(cam->m_cam_mutex);
	{
		//ignore camera id, just send something
		SDL_Event a;
		memset(&a,0,sizeof(a));
		a.type=SDL_USEREVENT;
		a.user.code=3;
		SDL_PushEvent(&a);
	}
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
