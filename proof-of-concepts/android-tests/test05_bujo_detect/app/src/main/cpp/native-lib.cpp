#include <jni.h>
#include <string>
#include "AsyncDetect.h"

extern "C"
JNIEXPORT jint JNICALL
Java_com_kolhizin_detectbujo_AsyncDetectionTask_detect(JNIEnv *env, jobject instance,
        jobject page, jobject settings) {

    // TODO
/*
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
*/
    return 0;
}