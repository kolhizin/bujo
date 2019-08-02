//
// Created by KolhiziN on 18.06.2019.
//

#ifndef TEST03_BUJO_ASYNC_ASYNCDETECT_H
#define TEST03_BUJO_ASYNC_ASYNCDETECT_H

#include <jni.h>
#import "detector.h"
#include <xtensor/xtensor.hpp>
#include <string>
#include <map>

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

    std::map<std::string, jfieldID> fields_;
    inline void loadFloatField_(const std::string &name)
    {
        auto tmp = env_->GetFieldID(class_, name.c_str(), "F");
        if(!tmp)throw std::runtime_error("Failed to load field-location in BuJoSettings!");
        fields_[name] = tmp;
    }
    inline void loadIntField_(const std::string &name)
    {
        auto tmp = env_->GetFieldID(class_, name.c_str(), "I");
        if(!tmp)throw std::runtime_error("Failed to load field-location in BuJoSettings!");
        fields_[name] = tmp;
    }
public:
    BuJoSettings(JNIEnv *env, jobject settings);

    float getFloatValue(const std::string &name, float defValue) const;
    int getIntValue(const std::string &name, int defValue) const;
};

enum BuJoStatus{
    UNDEFINED,
    CONVERTED_BITMAP,
    LOADED_DETECTOR,
    DETECTED_ANGLE,
    ALIGNED_IMAGES,
    FILTERED_IMAGES,
    DETECTED_REGION,
    DETECTED_CURVES,
    DETECTED_LINES,
    DETECTED_WORDS
};

class BuJoPage
{
    JNIEnv *env_;
    jclass class_;
    jobject object_;

    jmethodID getSource_;
    jmethodID setError_, setAngle_, addSplit_, addLine_, resetNumWordLines_, resetNumWords_, setWord_;

    jmethodID setStatusTransformedImage_, setStatusStartedDetector_, setStatusDetectedAngle_,
            setStatusAlignedImages_, setStatusFilteredImages_, setStatusDetectedRegion_,
            setStatusDetectedCurves_, setStatusDetectedLines_, setStatusDetectedWords_;

    inline void loadMethod_(jmethodID &var, const char * name, const char * sig){
        var = env_->GetMethodID(class_, name, sig);
        if(!var){
            std::string err_msg = "Could not link BuJoPage::";
            throw std::runtime_error(err_msg + name + " method in JNI with signature: " + sig + "!");
        }
    }
public:
    BuJoPage(JNIEnv *env, jobject page);

    inline JNIEnv * getEnv() {return env_;}
    jobject getSource();
    inline void setAngle(float a) const { env_->CallVoidMethod(object_, setAngle_, a); }
    inline void addSplit(const bujo::splits::SplitDesc &splt) const{
        env_->CallVoidMethod(object_, addSplit_, splt.angle, splt.offset, splt.offset_margin, splt.direction);
    }
    inline void resetNumWordLines(int n) const {
        env_->CallVoidMethod(object_, resetNumWordLines_, n);
    }
    inline void resetNumWords(int id, int n) const {
        env_->CallVoidMethod(object_, resetNumWords_, id, n);
    }
    void setWord(int lid, int wid, const xt::xtensor<float, 1> &xCoord, const xt::xtensor<float, 1> &yCoord,
            float negOffset, float posOffset) const;
    void addLine(const bujo::curves::Curve &curve) const;
    void setStatus(BuJoStatus status, const std::string &message);
    void setError(const std::string &str);
};


void performDetection(BuJoPage &page, const BuJoSettings &settings, const TaskNotifier &notifier);

#endif //TEST03_BUJO_ASYNC_ASYNCDETECT_H
