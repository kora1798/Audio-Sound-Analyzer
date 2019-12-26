#include <jni.h>
#include <string>
#include "a2.h"

extern "C" JNIEXPORT jint JNICALL
Java_ru_gamsoft_dplus_MainActivity_a2init(
        JNIEnv *env,
        jobject){
    int ret = a2_init();
    return ret;
};

extern "C" JNIEXPORT jint JNICALL
Java_ru_gamsoft_dplus_MainActivity_a2dispose(
        JNIEnv *env,
        jobject){
    int ret = a2_dispose();
    return ret;
};

extern "C" JNIEXPORT jint JNICALL
Java_ru_gamsoft_dplus_AudioRecordService_a2start(
        JNIEnv *env,
        jobject){
    int ret = a2_start();
    return ret;
};

extern "C" JNIEXPORT jint JNICALL
Java_ru_gamsoft_dplus_AudioRecordService_a2stop(
        JNIEnv *env,
        jobject){
    int ret = a2_stop();
    return ret;
};

extern "C" JNIEXPORT jint JNICALL
Java_ru_gamsoft_dplus_AudioRecordService_a2fragment(
        JNIEnv *env,
        jobject, jshortArray samples, jint n_sample){
    jshort *samplesptr = env->GetShortArrayElements(samples,0);
    int ret = a2_fragment(samplesptr, n_sample);
    env->ReleaseShortArrayElements(samples, samplesptr, 0);
    return ret;
};

extern "C" JNIEXPORT jint JNICALL
Java_ru_gamsoft_dplus_AudioRecordService_a2getparameter(
        JNIEnv *env,
        jobject, jint param){
    int ret = a2_get_parameter(param);
    return ret;
};

extern "C" JNIEXPORT jint JNICALL
Java_ru_gamsoft_dplus_MainActivity_a2getparameter(
        JNIEnv *env,
        jobject, jint param){
    int ret = a2_get_parameter(param);
    return ret;
};

extern "C" JNIEXPORT jint JNICALL
Java_ru_gamsoft_dplus_MainActivity_a2setparameter(
        JNIEnv *env,
        jobject, jint param, jint value){
    int ret = a2_set_parameter(param, value);
    return ret;
};

extern "C" JNIEXPORT jint JNICALL
Java_ru_gamsoft_dplus_MainActivity_a2learn(
        JNIEnv *env,
        jobject, jint fragment_type){
    int ret = a2_learn(fragment_type);
    return ret;
};

extern "C" JNIEXPORT jint JNICALL
Java_ru_gamsoft_dplus_MainActivity_a2comment(
        JNIEnv *env,
        jobject, jstring jstr) {
    const char *comment;
    comment = env->GetStringUTFChars(jstr, NULL);
    int ret = a2_comment(comment);
    env->ReleaseStringUTFChars(jstr, comment);
    return ret;
};
