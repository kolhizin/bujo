#include <jni.h>
#include <string>
#include <sstream>

int getRandom();

extern "C" JNIEXPORT jstring JNICALL
Java_com_kolhizin_test01_1jni_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::stringstream ss;
    ss << getRandom();
    std::string hello = "Hello from C++ -> " +ss. str();
    return env->NewStringUTF(hello.c_str());
}
