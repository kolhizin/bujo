#include <jni.h>
#include <string>
#include "AsyncDetect.h"

extern "C"
JNIEXPORT jint JNICALL
Java_com_kolhizin_detectbujo_AsyncDetectionTask_detect(JNIEnv *env, jobject instance,
        jobject page, jobject settings) {

    try {
        TaskNotifier notifier(env, instance, "com/kolhizin/detectbujo/AsyncDetectionTask");
        BuJoPage page_(env, page);
        BuJoSettings settings_(env, settings);

        try {
            performDetection(page_, settings_, notifier);
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
JNIEXPORT jint JNICALL
Java_com_kolhizin_detectbujo_AsyncDetectionTask_detectLines(JNIEnv *env, jobject instance,
                                                            jobject page, jobject settings) {

    try {
        TaskNotifier notifier(env, instance, "com/kolhizin/detectbujo/AsyncDetectionTask");
        BuJoPage page_(env, page);
        BuJoSettings settings_(env, settings);

        try {
            performLineDetection(page_, settings_, notifier);
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
JNIEXPORT jint JNICALL
Java_com_kolhizin_detectbujo_AsyncDetectionTask_detectWords(JNIEnv *env, jobject instance,
                                                            jobject page, jobject settings) {

    try {
        TaskNotifier notifier(env, instance, "com/kolhizin/detectbujo/AsyncDetectionTask");
        BuJoPage page_(env, page);
        BuJoSettings settings_(env, settings);

        try {
            performWordDetection(page_, settings_, notifier);
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