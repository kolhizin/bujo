#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_kolhizin_asyncbujo_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_kolhizin_asyncbujo_AsyncDetectionTask_detect(JNIEnv *env, jobject instance, jobject page,
                                                      jobject detectionTask) {

    // TODO
    jclass classBuJoPage = env->FindClass("com/kolhizin/asyncbujo/BuJoPage");
    jclass classTask = env->FindClass("com/kolhizin/asyncbujo/AsyncDetectionTask");
    if(!classBuJoPage)
        return -1;
    if(!classTask)
        return -1;

    jmethodID methodTask_notify = env->GetMethodID(classTask, "notifyUpdate", "()V");
    if(!methodTask_notify)
        return -2;

    jfieldID fieldBuJoPage_status = env->GetFieldID(classBuJoPage, "status", "Ljava/lang/String;");
    if(!fieldBuJoPage_status)
        return -2;


    env->SetObjectField(page, fieldBuJoPage_status, env->NewStringUTF("Success!"));
    env->CallVoidMethod(detectionTask, methodTask_notify);

    return 0;

}