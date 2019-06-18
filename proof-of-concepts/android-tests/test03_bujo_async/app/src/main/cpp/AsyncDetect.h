//
// Created by KolhiziN on 18.06.2019.
//

#ifndef TEST03_BUJO_ASYNC_ASYNCDETECT_H
#define TEST03_BUJO_ASYNC_ASYNCDETECT_H

#include <jni.h>
#import "detector.h"
#include <xtensor/xtensor.hpp>
#include <string>

class TaskNotifier
{
    JNIEnv *env_;
    jclass class_;
    jobject object_;
    jmethodID notify_;
public:
    TaskNotifier(JNIEnv * env, jobject task, const char * className);
    void notify() const;
};

class BuJoSettings
{
    JNIEnv *env_;
    jclass class_;
    jobject object_;
public:
    BuJoSettings(JNIEnv *env, jobject settings);
};

enum BuJoStatus{
    UNDEFINED,
    CONVERTED_BITMAP,
    LOADED_DETECTOR,
    DETECTED_ANGLE
};

class BuJoPage
{
    JNIEnv *env_;
    jclass class_;
    jobject object_;

    jmethodID getOriginal_;
    jmethodID setError_;

    jmethodID setStatusTransformedImage_;
public:
    BuJoPage(JNIEnv *env, jobject page);

    inline JNIEnv * getEnv() {return env_;}
    jobject getOriginal();
    void setStatus(BuJoStatus status, const std::string &message);
    void setError(const std::string &str);
};


void performDetection(BuJoPage &page, const BuJoSettings &settings, const TaskNotifier &notifier);

#endif //TEST03_BUJO_ASYNC_ASYNCDETECT_H
