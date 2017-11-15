#include "stdafx.h"
#include <atlbase.h>
#include <windows.h>
#include <dshow.h>
#include <stdio.h>
#include <MType.h>
#include <Streams.h>
#include <jni.h>
#include <Dvdmedia.h>
#include <Qedit.h>
#include <Aviriff.h>

#include "jmds.h"
#include "com_inzyme_jmds_DSCaptureDeviceManager.h"
#include "com_inzyme_jmds_DSCaptureDeviceInfo.h"
#include "com_inzyme_jmds_DSCapturePin.h"
#include "com_inzyme_jmds_DSCrossBar.h"
#include "com_inzyme_jmds_DSDataSource.h"
#include "com_inzyme_jmds_DSSourceStream.h"

IMediaControl * g_pMediaControl = NULL;
IMediaEventEx * g_pMediaEvent = NULL;
IBaseFilter * g_pSrcFilter = NULL;
IGraphBuilder * g_pGraph = NULL;
ICaptureGraphBuilder2 * g_pCapture = NULL;
ISampleGrabber * g_pGrabber = NULL;
AM_MEDIA_TYPE * g_pMediaType = NULL;
PLAYSTATE g_psCurrent = Stopped;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
  return TRUE;
}

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSCaptureDeviceManager_fillInDevices(JNIEnv *pEnv, jclass jCaptureDeviceManagerClass, jobject jDevicesList) {
	//Msg(TEXT("Java_com_inzyme_jmds_DSCaptureDeviceManager_fillInDevices"));
	HRESULT hrInitialize = CoInitialize(NULL);
	CheckForFailure(pEnv, "CoInitialize Failed.", hrInitialize);

	try {
		HRESULT hr;
	
		// Create the system device enumerator
		CComPtr<ICreateDevEnum> pDevEnum = NULL;
		hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,	IID_ICreateDevEnum, (void **) &pDevEnum);
		CheckForFailure(pEnv, "Couldn't create system enumerator.", hr);

		// Create an enumerator for the video capture devices
		CComPtr<IEnumMoniker> pClassEnum = NULL;
		hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
		CheckForFailure(pEnv, "Couldn't create class enumerator!", hr);

		// If there are no enumerators for the requested type, then 
		// CreateClassEnumerator will succeed, but pClassEnum will be NULL.
		if (pClassEnum == NULL) {
			FailWithException(pEnv, "No video capture device was detected.", E_FAIL);
		}
		
		jclass jListClass = pEnv->GetObjectClass(jDevicesList);
		jmethodID jListAddMethodID = pEnv->GetMethodID(jListClass, "add", "(Ljava/lang/Object;)Z");

		jclass jCaptureDeviceInfoClass = pEnv->FindClass("com/inzyme/jmds/DSCaptureDeviceInfo");
		jmethodID jCaptureDeviceInfoConstructor = pEnv->GetMethodID(jCaptureDeviceInfoClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");

		CComPtr<IMoniker> pMoniker = NULL;
		while (pClassEnum->Next(1, &pMoniker, 0) == S_OK) {
			CComPtr<IPropertyBag> pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
			CheckForFailure(pEnv, "Failed to retrieve device properties.", hr);

			// Find the description or friendly name.
			CComVariant friendlyName;
			hr = pPropBag->Read(L"FriendlyName", &friendlyName, 0);
			CheckForFailure(pEnv, "Failed to retrieve friendly name for device.", hr);
			jstring jFriendlyNameString = pEnv->NewString((jchar *)friendlyName.bstrVal, wcslen(friendlyName.bstrVal));

			CComVariant devicePath;
			hr = pPropBag->Read(L"DevicePath", &devicePath, 0);
			CheckForFailure(pEnv, "Failed to retrieve device path for device.", hr);
			jstring jDeviceNameString = pEnv->NewString((jchar *)devicePath.bstrVal, wcslen(devicePath.bstrVal));

			jobject jCaptureDeviceInfo = pEnv->NewObject(jCaptureDeviceInfoClass, jCaptureDeviceInfoConstructor, jDeviceNameString, jFriendlyNameString);
			pEnv->CallBooleanMethod(jDevicesList, jListAddMethodID, jCaptureDeviceInfo);

			pMoniker = NULL;
		}
	}
	catch (HRESULT) {
		// do nothing
	}

	if (hrInitialize == S_OK) {
		CoUninitialize();
	}
}

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSCaptureDeviceInfo_fillInPins(JNIEnv *pEnv, jobject jCaptureDeviceInfo, jobject jPinsList) {
	//Msg(TEXT("Java_com_inzyme_jmds_DSCaptureDeviceInfo_fillInPins"));

	HRESULT hrInitialize = CoInitialize(NULL);
	CheckForFailure(pEnv, "CoInitialize Failed.", hrInitialize);

	try {
		CComPtr<IBaseFilter> pSrcFilter = NULL;
		FindCaptureDevice(pEnv, jCaptureDeviceInfo, &pSrcFilter);

		HRESULT hr;
		CComPtr<IEnumPins> pPinsEnum = NULL;
		hr = pSrcFilter->EnumPins(&pPinsEnum);
		CheckForFailure(pEnv, "Failed to enumerate pins.", hr);

		jclass jListClass = pEnv->GetObjectClass(jPinsList);
		jmethodID jListAddMethodID = pEnv->GetMethodID(jListClass, "add", "(Ljava/lang/Object;)Z");

		jclass jCapturePinClass = pEnv->FindClass("com/inzyme/jmds/DSCapturePin");
		jmethodID jCapturePinConstructor = pEnv->GetMethodID(jCapturePinClass, "<init>", "(Lcom/inzyme/jmds/DSCaptureDeviceInfo;Ljava/lang/String;Ljava/lang/String;)V");

		CComPtr<IPin> pPin = NULL;
		while (pPinsEnum->Next(1, &pPin, 0) == S_OK) {
			PIN_INFO PinInfo;
			hr = pPin->QueryPinInfo(&PinInfo);
			CheckForFailure(pEnv, "Failed to get pin info.", hr);
			
			if (PinInfo.pFilter) {
				PinInfo.pFilter->Release();
			}

			LPWSTR pinId;
			hr = pPin->QueryId(&pinId);
			CheckForFailure(pEnv, "Failed to get pin ID.", hr);

			if (PinInfo.dir == PINDIR_OUTPUT) {
				jstring jPinIdString = pEnv->NewString((jchar *)pinId, wcslen(pinId));
				jstring jPinNameString = pEnv->NewString((jchar *)PinInfo.achName, wcslen(PinInfo.achName));
				jobject jCapturePin = pEnv->NewObject(jCapturePinClass, jCapturePinConstructor, jCaptureDeviceInfo, jPinIdString, jPinNameString);
				pEnv->CallBooleanMethod(jPinsList, jListAddMethodID, jCapturePin);
			}

			pPin = NULL;
		}
	}
	catch (HRESULT) {
		// do nothing
	}

	if (hrInitialize == S_OK) {
		CoUninitialize();
	}
}

JNIEXPORT jobject JNICALL Java_com_inzyme_jmds_DSDataSource_getCrossBar0(JNIEnv *pEnv, jobject jDataSource) {
	//Msg(TEXT("Java_com_inzyme_jmds_DSDataSource_getCrossBar0"));

	if (!g_pGraph) {
		FailWithException(pEnv, "You must connect to the data source before you can retrieve its crossbar.", E_FAIL);
	}

	HRESULT hrInitialize = CoInitialize(NULL);
	CheckForFailure(pEnv, "CoInitialize Failed.", hrInitialize);

	jobject jCrossBar = NULL;
	try {
		CComPtr<IAMCrossbar> pCrossBar = NULL;
		if (FindCrossBar(&pCrossBar) == S_OK) {
			jclass jCrossBarClass = pEnv->FindClass("com/inzyme/jmds/DSCrossBar");
			jmethodID jCrossBarConstructor = pEnv->GetMethodID(jCrossBarClass, "<init>", "(Lcom/inzyme/jmds/DSDataSource;)V");
			jCrossBar = pEnv->NewObject(jCrossBarClass, jCrossBarConstructor, jDataSource);
		}
	}
	catch (HRESULT) {
		// do nothing
	}

	if (hrInitialize == S_OK) {
		CoUninitialize();
	}
	return jCrossBar;
}

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSCrossBar_fillInCrossBarPins(JNIEnv *pEnv, jobject jCrossBar) {
	//Msg(TEXT("Java_com_inzyme_jmds_DSCrossBar_fillInCrossBarPins"));

	if (!g_pGraph) {
		FailWithException(pEnv, "You must connect to the data source before you can retrieve its crossbar.", E_FAIL);
	}

	HRESULT hrInitialize = CoInitialize(NULL);
	CheckForFailure(pEnv, "CoInitialize Failed.", hrInitialize);

	try {
		HRESULT hr;

		jclass jCrossBarClass = pEnv->GetObjectClass(jCrossBar);
		jclass jInputCrossBarPinClass = pEnv->FindClass("com/inzyme/jmds/DSInputCrossBarPin");
		jmethodID jInputCrossBarPinConstructor = pEnv->GetMethodID(jInputCrossBarPinClass, "<init>", "(Lcom/inzyme/jmds/DSCrossBar;III)V");
		jfieldID jInputPinsField = pEnv->GetFieldID(jCrossBarClass, "myInputPins", "[Lcom/inzyme/jmds/DSInputCrossBarPin;");

		jclass jOutputCrossBarPinClass = pEnv->FindClass("com/inzyme/jmds/DSOutputCrossBarPin");
		jmethodID jOutputCrossBarPinConstructor = pEnv->GetMethodID(jOutputCrossBarPinClass, "<init>", "(Lcom/inzyme/jmds/DSCrossBar;IIII)V");
		jfieldID jOutputPinsField = pEnv->GetFieldID(jCrossBarClass, "myOutputPins", "[Lcom/inzyme/jmds/DSOutputCrossBarPin;");

		CComPtr<IAMCrossbar> pCrossBar = NULL;
		if (FindCrossBar(&pCrossBar) == S_OK) {
			long inputPinCount = 0;
			long outputPinCount = 0;
			pCrossBar->get_PinCounts(&outputPinCount, &inputPinCount);

			jobjectArray jInputPins = pEnv->NewObjectArray(inputPinCount, jInputCrossBarPinClass, NULL);
			pEnv->SetObjectField(jCrossBar, jInputPinsField, jInputPins);
			for (int inputNum = 0; inputNum < inputPinCount; inputNum ++) {
				long pinRelatedIndex = 0;
				long physicalType = 0;
				hr = pCrossBar->get_CrossbarPinInfo(TRUE, inputNum, &pinRelatedIndex, &physicalType);

				CheckForFailure(pEnv, "Failed to retrieve crossbar pin info.", hr);

				jobject jCrossBarPin = pEnv->NewObject(jInputCrossBarPinClass, jInputCrossBarPinConstructor, jCrossBar, inputNum, pinRelatedIndex, physicalType);
				pEnv->SetObjectArrayElement(jInputPins, inputNum, jCrossBarPin);
			}

			jobjectArray jOutputPins = pEnv->NewObjectArray(outputPinCount, jOutputCrossBarPinClass, NULL);
			pEnv->SetObjectField(jCrossBar, jOutputPinsField, jOutputPins);
			for (int outputNum = 0; outputNum < outputPinCount; outputNum ++) {
				long pinRelatedIndex = 0;
				long physicalType = 0;
				long routedToIndex = 0;
				hr = pCrossBar->get_CrossbarPinInfo(FALSE, outputNum, &pinRelatedIndex, &physicalType);
				CheckForFailure(pEnv, "Failed to retrieve crossbar pin info.", hr);

				hr = pCrossBar->get_IsRoutedTo(outputNum, &routedToIndex);
				CheckForFailure(pEnv, "Failed to retrieve routed pin index.", hr);

				jobject jCrossBarPin = pEnv->NewObject(jOutputCrossBarPinClass, jOutputCrossBarPinConstructor, jCrossBar, outputNum, pinRelatedIndex, physicalType, routedToIndex);
				pEnv->SetObjectArrayElement(jOutputPins, outputNum, jCrossBarPin);
			}
		}
		else {
			jobjectArray jInputPins = pEnv->NewObjectArray(0, jInputCrossBarPinClass, NULL);
			pEnv->SetObjectField(jCrossBar, jInputPinsField, jInputPins);

			jobjectArray jOutputPins = pEnv->NewObjectArray(0, jOutputCrossBarPinClass, NULL);
			pEnv->SetObjectField(jCrossBar, jOutputPinsField, jOutputPins);
		}
	}
	catch (HRESULT) {
		// do nothing
	}

	if (hrInitialize == S_OK) {
		CoUninitialize();
	}
}

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSCrossBar_route0(JNIEnv *pEnv, jobject jCrossBar, jint jOutputPinIndex, jint jInputPinIndex) {
	//Msg(TEXT("Java_com_inzyme_jmds_DSCrossBar_route0"));

	if (!g_pGraph) {
		FailWithException(pEnv, "You must connect to the data source before you can retrieve its crossbar.", E_FAIL);
	}

	HRESULT hrInitialize = CoInitialize(NULL);
	CheckForFailure(pEnv, "CoInitialize Failed.", hrInitialize);

	try {
		HRESULT hr;

		CComPtr<IAMCrossbar> pCrossBar = NULL;
		if (FindCrossBar(&pCrossBar) == S_OK) {
			hr = pCrossBar->CanRoute(jOutputPinIndex, jInputPinIndex);
			CheckForFailure(pEnv, "Unable to route pins together.", hr);
				if (hr == S_FALSE) {
					FailWithException(pEnv, "You attempted to create an invalid route.", hr);
				}

			hr = pCrossBar->Route(jOutputPinIndex, jInputPinIndex);
			CheckForFailure(pEnv, "Failed to route crossbar.", hr);

		}
	}
	catch (HRESULT) {
		// do nothing
	}

	if (hrInitialize == S_OK) {
		CoUninitialize();
	}
}

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSCapturePin_fillInFormats(JNIEnv *pEnv, jobject jCapturePin) {
	//Msg(TEXT("Java_com_inzyme_jmds_DSCapturePin_fillInFormats"));

	HRESULT hrInitialize = CoInitialize(NULL);
	CheckForFailure(pEnv, "CoInitialize Failed.", hrInitialize);

	try {
		HRESULT hr;

		jclass jCapturePinClass = pEnv->GetObjectClass(jCapturePin);
		jfieldID jCaptureDeviceFieldID = pEnv->GetFieldID(jCapturePinClass, "myCaptureDevice", "Lcom/inzyme/jmds/DSCaptureDeviceInfo;");
		jobject jCaptureDeviceInfo = pEnv->GetObjectField(jCapturePin, jCaptureDeviceFieldID);

		CComPtr<IBaseFilter> pSrcFilter = NULL;
		FindCaptureDevice(pEnv, jCaptureDeviceInfo, &pSrcFilter);

		jfieldID jPinIDFieldID = pEnv->GetFieldID(jCapturePinClass, "myPinID", "Ljava/lang/String;");
		jstring jPinIDString = (jstring)pEnv->GetObjectField(jCapturePin, jPinIDFieldID);
		LPWSTR pinID = (LPWSTR)pEnv->GetStringChars(jPinIDString, 0);
		CComPtr<IPin> pPin;
		FindPin(pEnv, pSrcFilter, &pinID, &pPin);
		pEnv->ReleaseStringChars(jPinIDString, pinID);

		CComPtr<IAMStreamConfig> pStreamConfig;
		hr = pPin->QueryInterface(IID_IAMStreamConfig, (void **)&pStreamConfig);
		CheckForFailure(pEnv, "Failed to get a stream config for the requested pin.", hr);

		int iCount = 0;
		int iSize = 0;
		hr = pStreamConfig->GetNumberOfCapabilities(&iCount, &iSize);
		CheckForFailure(pEnv, "Failed to get pin capabilities.", hr);

		if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS)) {
			jclass jBitMapInfoClass = pEnv->FindClass("com/sun/media/vfw/BitMapInfo");
			jmethodID jBitMapInfoConstructorMethodID = pEnv->GetMethodID(jBitMapInfoClass, "<init>", "()V");
			jmethodID jFormatFoundMethodID = pEnv->GetMethodID(jCapturePinClass, "formatFound", "(Lcom/sun/media/vfw/BitMapInfo;F)V");

			for (int iFormatNum = 0; iFormatNum < iCount; iFormatNum ++) {
				VIDEO_STREAM_CONFIG_CAPS ConfigCap;
				AM_MEDIA_TYPE *pMediaType;
				hr = pStreamConfig->GetStreamCaps(iFormatNum, &pMediaType, (BYTE *)&ConfigCap);
				if (SUCCEEDED(hr)) {
					jobject jBitmapInfo = pEnv->NewObject(jBitMapInfoClass, jBitMapInfoConstructorMethodID);
					float fFrameRate = 10000000.0f / (float)ConfigCap.MaxFrameInterval;
					FillInBitMapInfo(pEnv, pMediaType, jBitMapInfoClass, jBitmapInfo);
					pEnv->CallVoidMethod(jCapturePin, jFormatFoundMethodID, jBitmapInfo, fFrameRate);
					DeleteMediaType(pMediaType);
				}
			}
		}
	}
	catch (HRESULT) {
		// do nothing
	}

	if (hrInitialize == S_OK) {
		CoUninitialize();
	}
}

JNIEXPORT jint JNICALL Java_com_inzyme_jmds_DSSourceStream_getBufferSize(JNIEnv *pEnv, jobject jSourceStream) {
	//Msg(TEXT("Java_com_inzyme_jmds_DSSourceStream_getBufferSize"));

	HRESULT hrInitialize = CoInitialize(NULL);
	CheckForFailure(pEnv, "CoInitialize Failed.", hrInitialize);

	// Find the required buffer size.
	long cbBufferSize = -1;

	try {
		HRESULT hr;
		hr = g_pGrabber->GetCurrentBuffer(&cbBufferSize, NULL);
		if (hr == VFW_E_WRONG_STATE) {
			cbBufferSize = 0;
		}
		else {
			CheckForFailure(pEnv, "Failed to get current buffer size.", hr);
		}
	}
	catch (HRESULT) {
		// do nothing
	}

	if (hrInitialize == S_OK) {
		CoUninitialize();
	}

	return cbBufferSize;
}

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSSourceStream_setFormat0(JNIEnv *pEnv, jobject jSourceStream, jint jFormatIndex) {
	//Msg(TEXT("Java_com_inzyme_jmds_DSSourceStream_setBitMapInfo"));

	HRESULT hrInitialize = CoInitialize(NULL);
	CheckForFailure(pEnv, "CoInitialize Failed.", hrInitialize);

	try {
		jclass jSourceStreamClass = pEnv->GetObjectClass(jSourceStream);
		jfieldID jCapturePinFieldID = pEnv->GetFieldID(jSourceStreamClass, "myCapturePin", "Lcom/inzyme/jmds/DSCapturePin;");
		jobject jCapturePin = pEnv->GetObjectField(jSourceStream, jCapturePinFieldID);

		jclass jCapturePinClass = pEnv->GetObjectClass(jCapturePin);
		jfieldID jCaptureDeviceFieldID = pEnv->GetFieldID(jCapturePinClass, "myCaptureDevice", "Lcom/inzyme/jmds/DSCaptureDeviceInfo;");
		jobject jCaptureDeviceInfo = pEnv->GetObjectField(jCapturePin, jCaptureDeviceFieldID);

		CComPtr<IBaseFilter> pSrcFilterNew = NULL;
		IBaseFilter *pSrcFilter;
		if (g_pGraph) {
			pSrcFilter = g_pSrcFilter;
		}
		else {
			FindCaptureDevice(pEnv, jCaptureDeviceInfo, &pSrcFilterNew);
			pSrcFilter = pSrcFilterNew;
		}

		jfieldID jPinIDFieldID = pEnv->GetFieldID(jCapturePinClass, "myPinID", "Ljava/lang/String;");
		jstring jPinIDString = (jstring)pEnv->GetObjectField(jCapturePin, jPinIDFieldID);
		LPWSTR pinID = (LPWSTR)pEnv->GetStringChars(jPinIDString, 0);
		CComPtr<IPin> pPin;
		FindPin(pEnv, pSrcFilter, &pinID, &pPin);
		pEnv->ReleaseStringChars(jPinIDString, pinID);

		HRESULT hr;
		CComPtr<IAMStreamConfig> pStreamConfig;
		hr = pPin->QueryInterface(IID_IAMStreamConfig, (void **)&pStreamConfig);
		CheckForFailure(pEnv, "Failed to get a stream config for the requested pin.", hr);

		VIDEO_STREAM_CONFIG_CAPS ConfigCap;
		AM_MEDIA_TYPE *pMediaType;
		hr = pStreamConfig->GetStreamCaps(jFormatIndex, &pMediaType, (BYTE *)&ConfigCap);
		CheckForFailure(pEnv, "Failed to get ConfigCap.", hr);

//		BITMAPINFOHEADER *bmi = HEADER(pMediaType->pbFormat);
//		bmi->biWidth = jWidth;
//		bmi->biHeight = jHeight;

		hr = pStreamConfig->SetFormat(pMediaType);
		DeleteMediaType(pMediaType);
		CheckForFailure(pEnv, "Failed to change the stream format.", hr);

		if (g_pGraph) {
			hr = g_pGraph->Reconnect(pPin);
			CheckForFailure(pEnv, "Failed to reconnect pin.", hr);
		}

		hr = pStreamConfig->GetFormat(&pMediaType);
		if (FAILED(hr)) {
			DeleteMediaType(pMediaType);
			FailWithException(pEnv, "Failed to get the new format.", hr);
		}

		if (g_pMediaType) {
			DeleteMediaType(g_pMediaType);
		}
		g_pMediaType = pMediaType;
	}
	catch (HRESULT) {
		// do nothing
	}

	if (hrInitialize == S_OK) {
		CoUninitialize();
	}
}

	
JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSSourceStream_fillInBitMapInfo(JNIEnv *pEnv, jobject jSourceStream, jobject jBitMapInfo) {
	//Msg(TEXT("Java_com_inzyme_jmds_DSSourceStream_fillInBitMapInfo"));

	HRESULT hrInitialize = CoInitialize(NULL);
	CheckForFailure(pEnv, "CoInitialize Failed.", hrInitialize);

	try {
		jclass jBitMapInfoClass = pEnv->FindClass("com/sun/media/vfw/BitMapInfo");
		FillInBitMapInfo(pEnv, g_pMediaType, jBitMapInfoClass, jBitMapInfo);
	}
	catch (HRESULT) {
		// do nothing
	}

	if (hrInitialize == S_OK) {
		CoUninitialize();
	}
}

JNIEXPORT jfloat JNICALL Java_com_inzyme_jmds_DSSourceStream_getFrameRate(JNIEnv *pEnv, jobject jSourceStream) {
	//Msg(TEXT("Java_com_inzyme_jmds_DSSourceStream_getFrameRate"));

	REFERENCE_TIME avgTimePerFrame = ((VIDEOINFOHEADER *)g_pMediaType->pbFormat)->AvgTimePerFrame;
	float frameRate = (float)(10000000L) / avgTimePerFrame;
	return frameRate;
}

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSSourceStream_connect0(JNIEnv *pEnv, jobject jSourceStream) {
	//Msg(TEXT("Java_com_inzyme_jmds_DSSourceStream_connect0"));

	HRESULT hrInitialize = CoInitialize(NULL);
	CheckForFailure(pEnv, "CoInitialize Failed.", hrInitialize);

	try {
		HRESULT hr;

		if (g_pGraph) {
			FailWithException(pEnv, "A capture has already been connected.", E_FAIL);
		}

		CComPtr<IGraphBuilder> pGraph = NULL;
		// Create the filter graph
		hr = CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **) &pGraph);
		CheckForFailure(pEnv, "Failed to create filter graph.", hr);

		// Create the capture graph builder
		CComPtr<ICaptureGraphBuilder2> pCapture = NULL;
		hr = CoCreateInstance (CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **) &pCapture);
		CheckForFailure(pEnv, "Failed to create capture graph.", hr);
  
		// Obtain interfaces for media control and Video Window
		CComPtr<IMediaControl> pMediaControl = NULL;
		hr = pGraph->QueryInterface(IID_IMediaControl,(LPVOID *) &pMediaControl);
		CheckForFailure(pEnv, "Failed to create media control.", hr);

		CComPtr<IMediaEventEx> pMediaEvent = NULL;
		hr = pGraph->QueryInterface(IID_IMediaEvent, (LPVOID *) &pMediaEvent);
		CheckForFailure(pEnv, "Failed to create media event.", hr);

		hr = pCapture->SetFiltergraph(pGraph);
		CheckForFailure(pEnv, "Failed to set capture filter graph!", hr);

		// Get capture device
		jclass jSourceStreamClass = pEnv->GetObjectClass(jSourceStream);
		jfieldID jCapturePinFieldID = pEnv->GetFieldID(jSourceStreamClass, "myCapturePin", "Lcom/inzyme/jmds/DSCapturePin;");
		jobject jCapturePin = pEnv->GetObjectField(jSourceStream, jCapturePinFieldID);

		jclass jCapturePinClass = pEnv->GetObjectClass(jCapturePin);
		jfieldID jCaptureDeviceFieldID = pEnv->GetFieldID(jCapturePinClass, "myCaptureDevice", "Lcom/inzyme/jmds/DSCaptureDeviceInfo;");
		jobject jCaptureDevice = pEnv->GetObjectField(jCapturePin, jCaptureDeviceFieldID);

		CComPtr<IBaseFilter> pSrcFilter = NULL;
		FindCaptureDevice(pEnv, jCaptureDevice, &pSrcFilter);
 
		// Add Capture filter to our graph.
		hr = pGraph->AddFilter(pSrcFilter, L"Video Capture");
		CheckForFailure(pEnv, "Couldn't add capture filter to the graph.", hr);

		// Create sample grabber
		CComPtr<IBaseFilter> pGrabber = NULL;
		hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pGrabber);
		CheckForFailure(pEnv, "Couldn't create sample grabber.", hr);

		hr = pGraph->AddFilter(pGrabber, L"Sample Grabber");
		CheckForFailure(pEnv, "Couldn't add sample grabber to graph.", hr);

    CComPtr<IPin> pGrabberInputPin = NULL;
		hr = GetUnconnectedPin(pGrabber, PINDIR_INPUT, &pGrabberInputPin);
		CheckForFailure(pEnv, "Couldn't find grabber input pin.", hr);

		jfieldID jPinIDFieldID = pEnv->GetFieldID(jCapturePinClass, "myPinID", "Ljava/lang/String;");
		jstring jPinIDString = (jstring)pEnv->GetObjectField(jCapturePin, jPinIDFieldID);
		LPWSTR pinID = (LPWSTR)pEnv->GetStringChars(jPinIDString, 0);
		CComPtr<IPin> pSourceOutputPin;
		FindPin(pEnv, pSrcFilter, &pinID, &pSourceOutputPin);
		pEnv->ReleaseStringChars(jPinIDString, pinID);

		// Connect capture and grabber
		hr = pGraph->Connect(pSourceOutputPin, pGrabberInputPin);
		CheckForFailure(pEnv, "Couldn't connect capture source to grabber.", hr);

		// Create null renderer
		CComPtr<IBaseFilter> pNullRenderer = NULL;
		hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pNullRenderer);
		CheckForFailure(pEnv, "Couldn't create null renderer.", hr);

		hr = pGraph->AddFilter(pNullRenderer, L"Null Renderer");
		CheckForFailure(pEnv, "Couldn't add null renderer to graph.", hr);

    CComPtr<IPin> pGrabberOutputPin = NULL;
		hr = GetUnconnectedPin(pGrabber, PINDIR_OUTPUT, &pGrabberOutputPin);
		CheckForFailure(pEnv, "Couldn't find grabber output pin.", hr);

    CComPtr<IPin> pNullRendererInputPin = NULL;
		hr = GetUnconnectedPin(pNullRenderer, PINDIR_INPUT, &pNullRendererInputPin);
		CheckForFailure(pEnv, "Couldn't find null renderer input pin.", hr);

		// Connect grabber and null renderer
		hr = pGraph->Connect(pGrabberOutputPin, pNullRendererInputPin);
		CheckForFailure(pEnv, "Couldn't connect grabber to renderer.", hr);

		CComPtr<ISampleGrabber> pSampleGrabber = NULL;
		hr = pGrabber->QueryInterface(IID_ISampleGrabber, (void**)&pSampleGrabber);
		CheckForFailure(pEnv, "Failed to retrieve sample grabber interface.", hr);

		// Get stream config
		CComPtr<IAMStreamConfig> pStreamConfig = NULL;
		pSourceOutputPin->QueryInterface(IID_IAMStreamConfig, (void**)&pStreamConfig);
		AM_MEDIA_TYPE *pMediaType = NULL;
		pStreamConfig->GetFormat(&pMediaType);
		CheckForFailure(pEnv, "Failed to get media type from output pin.", hr);

		hr = pSampleGrabber->SetBufferSamples(TRUE);
		CheckForFailure(pEnv, "Failed to turn on buffering on sample grabber.", hr);

		g_pMediaControl = pMediaControl.Detach();
		g_pMediaEvent = pMediaEvent.Detach();
		g_pSrcFilter = pSrcFilter.Detach();
		g_pGraph = pGraph.Detach();
		g_pCapture = pCapture.Detach();
		g_pGrabber = pSampleGrabber.Detach();
		g_pMediaType = pMediaType;
	}
	catch (HRESULT) {
		// do nothing
	}

	if (hrInitialize == S_OK) {
		CoUninitialize();
	}
}

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSSourceStream_start0(JNIEnv *pEnv, jobject jSourceStream) {
	// Msg(TEXT("Java_com_inzyme_jmds_DSSourceStream_start0"));
	HRESULT hrInitialize = CoInitialize(NULL);
	CheckForFailure(pEnv, "CoInitialize Failed.", hrInitialize);

	try {
		HRESULT hr;

		//g_SampleGrabberCB.Initialize(pEnv, jSourceStream);
		//g_pGrabber->SetCallback(&g_SampleGrabberCB, 1);

		if (g_psCurrent == Running) {
			FailWithException(pEnv, "A capture has already been started.", E_FAIL);
		}

		// Start previewing video data
		hr = g_pMediaControl->Run();
		CheckForFailure(pEnv, "Couldn't run the graph.", hr);

		// Remember current state
		g_psCurrent = Running;
	}
	catch (HRESULT) {
		// do nothing
	}

	if (hrInitialize == S_OK) {
		CoUninitialize();
	}
}

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSSourceStream_stop0(JNIEnv *pEnv, jobject jSourceStream) {
	// Msg(TEXT("Java_com_inzyme_jmds_DSSourceStream_stop0"));

	HRESULT hrInitialize = CoInitialize(NULL);
	CheckForFailure(pEnv, "CoInitialize Failed.", hrInitialize);

	try {
		// Stop previewing data
		if (g_pMediaControl) {
			g_pMediaControl->StopWhenReady();
		}

		g_psCurrent = Stopped;

		// Stop receiving events
		if (g_pMediaEvent) {
			g_pMediaEvent->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);
		}
	}
	catch (HRESULT) {
		// do nothing
	}

	if (hrInitialize == S_OK) {
		CoUninitialize();
	}
}

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSSourceStream_disconnect0(JNIEnv *pEnv, jobject jSourceStream) {
	//Msg(TEXT("Java_com_inzyme_jmds_DSSourceStream_disconnect0"));

	HRESULT hrInitialize = CoInitialize(NULL);
	CheckForFailure(pEnv, "CoInitialize Failed.", hrInitialize);

	try {
		// Release DirectShow interfaces
		SAFE_RELEASE(g_pMediaControl);
		SAFE_RELEASE(g_pMediaEvent);
		SAFE_RELEASE(g_pGraph);
		SAFE_RELEASE(g_pCapture);
		SAFE_RELEASE(g_pGrabber);
		SAFE_RELEASE(g_pSrcFilter);
		FreeMediaType(*g_pMediaType);
	}
	catch (HRESULT) {
		// do nothing
	}

	if (hrInitialize == S_OK) {
		CoUninitialize();
	}
}

JNIEXPORT jint JNICALL Java_com_inzyme_jmds_DSSourceStream_fillBuffer(JNIEnv *pEnv, jobject jSourceStream, jbyteArray jBuffer, jint jOffset, jint jLength) {
	//Msg(TEXT("Java_com_inzyme_jmds_DSSourceStream_fillBuffer %d, %d"), jOffset, jLength);

	HRESULT hrInitialize = CoInitialize(NULL);
	CheckForFailure(pEnv, "CoInitialize Failed.", hrInitialize);

	long cbBytesRead = 0;

	try {
		HRESULT hr;

		long cbBufferLength = -1;
		hr = g_pGrabber->GetCurrentBuffer(&cbBufferLength, NULL);
		if (hr != VFW_E_WRONG_STATE) {
			CheckForFailure(pEnv, "Failed to get buffer size.", hr);

			if (jLength < cbBufferLength) {
				cbBufferLength = jLength;
			}

			jbyte *buffer = pEnv->GetByteArrayElements(jBuffer, NULL);
			g_pGrabber->GetCurrentBuffer(&cbBufferLength, (long *)(buffer + jOffset));
			CheckForFailure(pEnv, "Failed to fill buffer.", hr);

			pEnv->ReleaseByteArrayElements(jBuffer, buffer, 0);

			cbBytesRead = cbBufferLength;
		}
	}
	catch (HRESULT) {
		// do nothing
	}

	if (hrInitialize == S_OK) {
		CoUninitialize();
	}

	return cbBytesRead;
}

// OK .. This was "borrowed" from JMF.  Seemed silly to rewrite it when you can't really write this
// in a different way.
void FillInBitMapInfo(JNIEnv *pEnv, AM_MEDIA_TYPE *pMediaType, jclass jBitMapInfoClass, jobject jbmi) {
	BITMAPINFOHEADER *bmi = HEADER(pMediaType->pbFormat);
	int size = pMediaType->cbFormat;
	char fourcc[5];

	jfieldID bmi_biWidth = pEnv->GetFieldID(jBitMapInfoClass, "biWidth", "I");
	jfieldID bmi_biHeight = pEnv->GetFieldID(jBitMapInfoClass, "biHeight", "I");
	jfieldID bmi_biPlanes = pEnv->GetFieldID(jBitMapInfoClass, "biPlanes", "I");
	jfieldID bmi_biBitCount = pEnv->GetFieldID(jBitMapInfoClass, "biBitCount", "I");
	jfieldID bmi_biSizeImage = pEnv->GetFieldID(jBitMapInfoClass, "biSizeImage", "I");
	jfieldID bmi_biXPelsPerMeter = pEnv->GetFieldID(jBitMapInfoClass, "biXPelsPerMeter", "I");
	jfieldID bmi_biYPelsPerMeter = pEnv->GetFieldID(jBitMapInfoClass, "biYPelsPerMeter", "I");
	jfieldID bmi_biClrUsed = pEnv->GetFieldID(jBitMapInfoClass, "biClrUsed", "I");
	jfieldID bmi_biClrImportant = pEnv->GetFieldID(jBitMapInfoClass, "biClrImportant", "I");
	jfieldID bmi_fourcc = pEnv->GetFieldID(jBitMapInfoClass, "fourcc", "Ljava/lang/String;");

	pEnv->SetIntField(jbmi, bmi_biWidth, bmi->biWidth);
	pEnv->SetIntField(jbmi, bmi_biHeight, bmi->biHeight);
	pEnv->SetIntField(jbmi, bmi_biPlanes, bmi->biPlanes);
	pEnv->SetIntField(jbmi, bmi_biBitCount, bmi->biBitCount);
	pEnv->SetIntField(jbmi, bmi_biSizeImage, bmi->biSizeImage);
	pEnv->SetIntField(jbmi, bmi_biXPelsPerMeter, bmi->biXPelsPerMeter);
	pEnv->SetIntField(jbmi, bmi_biYPelsPerMeter, bmi->biYPelsPerMeter);
	pEnv->SetIntField(jbmi, bmi_biClrUsed, bmi->biClrUsed);
	pEnv->SetIntField(jbmi, bmi_biClrImportant, bmi->biClrImportant);

	if (bmi->biCompression == BI_RGB) {
		strcpy(fourcc, "RGB");
	}
	else if (bmi->biCompression == BI_RLE8) {
		strcpy(fourcc, "RLE8");
	}
	else if (bmi->biCompression == BI_RLE4) {
		strcpy(fourcc, "RLE4");
	}
	else if (bmi->biCompression == BI_BITFIELDS) {
		strcpy(fourcc, "RGB");
	}
	else {
		*(int*)fourcc = bmi->biCompression;
	}
	fourcc[4] = 0;

	/*printf("size = %d, biSizeImage = %d, fourcc = %s\n", size, bmi->biSizeImage, fourcc);*/

	if (size > sizeof(BITMAPINFOHEADER)) {
		int diff = size - sizeof(BITMAPINFOHEADER);
		signed char* extraData;
		jbyteArray jExtraBytes = pEnv->NewByteArray(diff);
		jclass jbmiClass = pEnv->GetObjectClass(jbmi); 
		jfieldID extraBytesField = pEnv->GetFieldID(jbmiClass, "extraBytes", "[B");
		pEnv->SetObjectField(jbmi, extraBytesField, jExtraBytes);
		jfieldID extraSizeField = pEnv->GetFieldID(jbmiClass, "extraSize", "I");
		pEnv->SetIntField(jbmi, extraSizeField, diff);
		extraData = (signed char*)pEnv->GetByteArrayElements(jExtraBytes, 0);
		for (int i = 0; i < diff; i++) {
			extraData[i] = ((signed char*)bmi)[i + sizeof(BITMAPINFOHEADER)];
		}
		pEnv->ReleaseByteArrayElements(jExtraBytes, extraData, 0);
	}

	/*
	for (i = 0; i < size; i++) {
	printf("%d = %d\n", i, ((unsigned char *)bmi)[i]);
	}
	*/
	jstring js = pEnv->NewStringUTF(fourcc);
	pEnv->SetObjectField(jbmi, bmi_fourcc, (jobject) js);
}

HRESULT FindPin(JNIEnv *pEnv, IBaseFilter *pSrcFilter, LPWSTR *pRequestedPinID, IPin **ppPin) {
	HRESULT hr;
	CComPtr<IEnumPins> pPinsEnum = NULL;
	hr = pSrcFilter->EnumPins(&pPinsEnum);
	CheckForFailure(pEnv, "Failed to enumerate pins.", hr);

	HRESULT hrFoundPin = S_FALSE;
	CComPtr<IPin> pPin = NULL;
	while (hrFoundPin == S_FALSE && pPinsEnum->Next(1, &pPin, 0) == S_OK) {
		LPWSTR pinId;
		hr = pPin->QueryId(&pinId);
		CheckForFailure(pEnv, "Failed to get pin ID.", hr);

		if (wcscmp(pinId, *pRequestedPinID) == 0) {
			*ppPin = pPin.Detach();
			hrFoundPin = S_OK;
		}

		pPin = NULL;
	}

	return hrFoundPin;
}

HRESULT FindCrossBar(IAMCrossbar **ppCrossBar) {
  HRESULT hr;
		
	CComPtr<IAMCrossbar> pCrossBar = NULL;
	hr = g_pCapture->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, g_pSrcFilter, IID_IAMCrossbar, (void**)&pCrossBar);
	if (SUCCEEDED(hr)) {
		*ppCrossBar = pCrossBar.Detach();
		hr = S_OK;
	}
	else {
		hr = S_FALSE;
	}
	return hr;
}

void FindCaptureDevice(JNIEnv *pEnv, jobject jCaptureDeviceInfo, IBaseFilter **ppSrcFilter) {
	jclass jCaptureDeviceInfoClass = pEnv->GetObjectClass(jCaptureDeviceInfo);
	FindCaptureDevice(pEnv, jCaptureDeviceInfo, jCaptureDeviceInfoClass, ppSrcFilter);
}

void FindCaptureDevice(JNIEnv *pEnv, jobject jCaptureDeviceInfo, jclass jCaptureDeviceInfoClass, IBaseFilter **ppSrcFilter) {
	jmethodID jDevicePathMethodID = pEnv->GetMethodID(jCaptureDeviceInfoClass, "getDevicePath", "()Ljava/lang/String;");
	jstring jDevicePathString = (jstring)pEnv->CallObjectMethod(jCaptureDeviceInfo, jDevicePathMethodID);
	LPWSTR devicePath = (LPWSTR)pEnv->GetStringChars(jDevicePathString, 0);
	FindCaptureDevice(pEnv, &devicePath, ppSrcFilter);
	pEnv->ReleaseStringChars(jDevicePathString, devicePath);
}

void FindCaptureDevice(JNIEnv *pEnv, LPWSTR *requestedDevicePath, IBaseFilter **ppSrcFilter) {
	HRESULT hr;

	CComPtr<ICreateDevEnum> pDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,	IID_ICreateDevEnum, (void **) &pDevEnum);
	CheckForFailure(pEnv, "Couldn't create system enumerator.", hr);

	CComPtr<IEnumMoniker> pClassEnum = NULL;
	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
	CheckForFailure(pEnv, "Couldn't create class enumerator.", hr);

	if (pClassEnum == NULL) {
		FailWithException(pEnv, "No video capture device was detected.", E_FAIL);
	}
	
	CComPtr<IBaseFilter> pSrc = NULL;
	CComPtr<IMoniker> pMoniker = NULL;
	while (pSrc == NULL && pClassEnum->Next(1, &pMoniker, 0) == S_OK) {
    IPropertyBag *pPropBag;
    hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
		CheckForFailure(pEnv, "Failed to retrieve device properties.", hr);

		CComVariant devicePath;
		hr = pPropBag->Read(L"DevicePath", &devicePath, 0);
		CheckForFailure(pEnv, "Failed to retrieve device path for device.", hr);

		if (wcscmp(devicePath.bstrVal, *requestedDevicePath) == 0) {
			hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void **)&pSrc);
			CheckForFailure(pEnv, "Failed to bind to device.", hr);
		}

		pMoniker = NULL;
	}

	*ppSrcFilter = pSrc.Detach();
}

void CheckForFailure(JNIEnv *pEnv, char *msg, HRESULT result) {
	if (FAILED(result)) {
		FailWithException(pEnv, msg, result);
	}
}

void FailWithException(JNIEnv *pEnv, char *msg, HRESULT result) {
	jclass jExceptionCls = pEnv->FindClass("java/lang/IllegalArgumentException");
	char msgFull[128];
	sprintf(msgFull, "%s hr = 0x%x", msg, result);
  pEnv->ThrowNew(jExceptionCls, msgFull);
	pEnv->DeleteLocalRef(jExceptionCls);
	throw result;
}

HRESULT GetUnconnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin) {
	CComPtr<IEnumPins> pEnum = NULL;
	CComPtr<IPin> pPin = NULL;
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr)) {
		return hr;
	}
	while (pEnum->Next(1, &pPin, NULL) == S_OK)	{
		PIN_DIRECTION ThisPinDir;
		pPin->QueryDirection(&ThisPinDir);
		if (ThisPinDir == PinDir) {
			CComPtr<IPin> pTmp = NULL;
			hr = pPin->ConnectedTo(&pTmp);
			if (FAILED(hr)) {
				*ppPin = pPin.Detach();
				return S_OK;
			}
		}
		pPin = NULL;
	}
	return E_FAIL;
}

void Msg(TCHAR *szFormat, ...) {
  TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
  const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
  const int LASTCHAR = NUMCHARS - 1;

  // Format the input string
  va_list pArgs;
  va_start(pArgs, szFormat);

  // Use a bounded buffer size to prevent buffer overruns.  Limit count to
  // character size minus one to allow for a NULL terminating character.
  _vsntprintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
  va_end(pArgs);

  // Ensure that the formatted string is NULL-terminated
  szBuffer[LASTCHAR] = TEXT('\0');

  MessageBox(NULL, szBuffer, TEXT("PlayCap Message"), MB_OK | MB_ICONERROR);
}

/*
HRESULT HandleGraphEvent(void) {
  LONG evCode, evParam1, evParam2;
  HRESULT hr = S_OK;

  if (!g_pME) {
    return E_POINTER;
	}

  while (SUCCEEDED(g_pME->GetEvent(&evCode, (LONG_PTR *) &evParam1, (LONG_PTR *) &evParam2, 0))) {
    //
    // Free event parameters to prevent memory leaks associated with
    // event parameter data.  While this application is not interested
    // in the received events, applications should always process them.
    //
    hr = g_pME->FreeEventParams(evCode, evParam1, evParam2);
    
    // Insert event processing code here, if desired
  }

  return hr;
}
*/