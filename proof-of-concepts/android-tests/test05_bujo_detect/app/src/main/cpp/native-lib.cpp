#include <jni.h>
#include <string>
#include "AsyncDetect.h"
#include "detector.h"

extern "C"
JNIEXPORT jint JNICALL
Java_com_kolhizin_detectbujo_AsyncDetectionTask_detect(JNIEnv *env, jobject instance,
        jobject page, jobject settings) {

    try {
        TaskNotifier notifier(env, instance, "com/kolhizin/detectbujo/AsyncDetectionTask");
        BuJoPage page_(env, page);
        BuJoSettings settings_(env, settings);

        try {
            asyncDetection(page_, settings_, notifier);
        }catch (const std::exception &e)
        {
            page_.setError(e.what());
            notifier.notify();
            return 1;
        }catch (...)
        {
            page_.setError("Unknown error!");
            notifier.notify();
            return 1;
        }

    }catch (...)
    {
        return -1;
    }
    return 0;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_kolhizin_detectbujo_Detector_createDetector(JNIEnv *env, jobject instance) {
    return reinterpret_cast<long>(new bujo::detector::Detector());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_kolhizin_detectbujo_Detector_deleteDetector(JNIEnv *env, jobject instance, jlong handle) {
    delete reinterpret_cast<bujo::detector::Detector *>(handle);
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_kolhizin_detectbujo_Detector_load__JLcom_kolhizin_detectbujo_BuJoPage_2Lcom_kolhizin_detectbujo_BuJoSettings_2(
        JNIEnv *env, jobject instance, jlong hDetector, jobject page, jobject settings) {
    auto pDetector = reinterpret_cast<bujo::detector::Detector *>(hDetector);
    try {
        BuJoPage page_(env, page);
        BuJoSettings settings_(env, settings);
        runLoad(*pDetector, page_, settings_);
    }catch (const std::exception &e)
    {
        return env->NewStringUTF(e.what());
    }catch (...)
    {
        return env->NewStringUTF("Unknwon error");
    }
    return env->NewStringUTF(""); //empty means ok
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_kolhizin_detectbujo_Detector_preprocess__JLcom_kolhizin_detectbujo_BuJoPage_2Lcom_kolhizin_detectbujo_BuJoSettings_2(JNIEnv *env, jobject instance,
                                                      jlong hDetector, jobject page, jobject settings) {
    auto pDetector = reinterpret_cast<bujo::detector::Detector *>(hDetector);
    try {
        BuJoPage page_(env, page);
        BuJoSettings settings_(env, settings);
        runPreprocess(*pDetector, page_, settings_);
    }catch (const std::exception &e)
    {
        return env->NewStringUTF(e.what());
    }catch (...)
    {
        return env->NewStringUTF("Unknwon error");
    }
    return env->NewStringUTF(""); //empty means ok
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_kolhizin_detectbujo_Detector_detectRegions__JLcom_kolhizin_detectbujo_BuJoPage_2Lcom_kolhizin_detectbujo_BuJoSettings_2(JNIEnv *env, jobject instance,
                                                         jlong hDetector, jobject page, jobject settings) {
    auto pDetector = reinterpret_cast<bujo::detector::Detector *>(hDetector);
    try {
        BuJoPage page_(env, page);
        BuJoSettings settings_(env, settings);
        runDetectRegions(*pDetector, page_, settings_);
    }catch (const std::exception &e)
    {
        return env->NewStringUTF(e.what());
    }catch (...)
    {
        return env->NewStringUTF("Unknwon error");
    }
    return env->NewStringUTF(""); //empty means ok
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_kolhizin_detectbujo_Detector_detectLines__JLcom_kolhizin_detectbujo_BuJoPage_2Lcom_kolhizin_detectbujo_BuJoSettings_2(JNIEnv *env, jobject instance,
                                                       jlong hDetector, jobject page, jobject settings) {
    auto pDetector = reinterpret_cast<bujo::detector::Detector *>(hDetector);
    try {
        BuJoPage page_(env, page);
        BuJoSettings settings_(env, settings);
        runDetectLines(*pDetector, page_, settings_);
    }catch (const std::exception &e)
    {
        return env->NewStringUTF(e.what());
    }catch (...)
    {
        return env->NewStringUTF("Unknwon error");
    }
    return env->NewStringUTF(""); //empty means ok
}