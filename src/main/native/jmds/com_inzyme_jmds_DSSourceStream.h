/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_inzyme_jmds_DSSourceStream */

#ifndef _Included_com_inzyme_jmds_DSSourceStream
#define _Included_com_inzyme_jmds_DSSourceStream
#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jint JNICALL Java_com_inzyme_jmds_DSSourceStream_getBufferSize(JNIEnv *, jobject);

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSSourceStream_setFormat0(JNIEnv *, jobject, jint);

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSSourceStream_fillInBitMapInfo(JNIEnv *, jobject, jobject);

JNIEXPORT jfloat JNICALL Java_com_inzyme_jmds_DSSourceStream_getFrameRate(JNIEnv *, jobject);

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSSourceStream_connect0(JNIEnv *, jobject);

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSSourceStream_start0(JNIEnv *, jobject);

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSSourceStream_stop0(JNIEnv *, jobject);

JNIEXPORT void JNICALL Java_com_inzyme_jmds_DSSourceStream_disconnect0(JNIEnv *, jobject);

JNIEXPORT jint JNICALL Java_com_inzyme_jmds_DSSourceStream_fillBuffer(JNIEnv *, jobject, jbyteArray, jint, jint);

#ifdef __cplusplus
}
#endif
#endif
