//
// Function prototypes
//
void FillInMediaType(JNIEnv *, AM_MEDIA_TYPE *, jclass, jobject);
void FillInBitMapInfo(JNIEnv *, AM_MEDIA_TYPE *, jclass, jobject);
HRESULT FindPin(JNIEnv *, IBaseFilter *, LPWSTR *, IPin **);
HRESULT FindCrossBar(IAMCrossbar **);
void GetPinForSourceStream(JNIEnv *, jobject, IPin *);
void FindCaptureDevice(JNIEnv *, jobject, IBaseFilter **);
void FindCaptureDevice(JNIEnv *, jobject, jclass, IBaseFilter **);
void FindCaptureDevice(JNIEnv *, LPWSTR *, IBaseFilter **);
HRESULT GetUnconnectedPin(IBaseFilter *, PIN_DIRECTION, IPin **);

void CheckForFailure(JNIEnv *, char *msg, HRESULT);
void FailWithException(JNIEnv *, char *msg, HRESULT);

void Msg(TCHAR *szFormat, ...);

enum PLAYSTATE { Stopped, Paused, Running, Init };

//
// Macros
//
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

// Application-defined message to notify app of filtergraph events
#define WM_GRAPHNOTIFY  WM_APP+1
