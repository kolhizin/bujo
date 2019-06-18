//
// Created by KolhiziN on 18.06.2019.
//
#include "AsyncDetect.h"
#include <exception>
#include <android/bitmap.h>


BuJoSettings::BuJoSettings(JNIEnv *env, jobject settings) {
    env_ = env;
    object_ = settings;
    class_ = env_->FindClass("com/kolhizin/asyncbujo/BuJoSettings");
    if(!class_)
        throw std::runtime_error("Could not link BuJoSettings-class in JNI!");
}

BuJoPage::BuJoPage(JNIEnv *env, jobject page) {
    env_ = env;
    object_ = page;
    class_ = env_->FindClass("com/kolhizin/asyncbujo/BuJoPage");
    if(!class_)
        throw std::runtime_error("Could not link BuJoPage-class in JNI!");

    getOriginal_ = env_->GetMethodID(class_, "getOriginal", "()Landroid/graphics/Bitmap;");
    if(!getOriginal_) throw std::runtime_error("Could not link BuJoPage::getOriginal method in JNI!");

    setStatusTransformedImage_ = env_->GetMethodID(class_, "setStatusTransformedImage", "(Ljava/lang/String;)V");
    if(!setStatusTransformedImage_) throw std::runtime_error("Could not link BuJoPage::setStatusTransformedImage method in JNI!");

    setError_ = env_->GetMethodID(class_, "setError", "(Ljava/lang/String;)V");
    if(!setError_) throw std::runtime_error("Could not link BuJoPage::setError method in JNI!");

}

void BuJoPage::setStatus(BuJoStatus status, const std::string &message)
{
    jobject msg = env_->NewStringUTF(message.c_str());
    if(status == BuJoStatus::CONVERTED_BITMAP)
        env_->CallVoidMethod(object_, setStatusTransformedImage_, msg);
    else
        throw std::runtime_error("Unexpected status in BuJoPage::setStatus!");
}

void BuJoPage::setError(const std::string &str) {
    jobject msg = env_->NewStringUTF(str.c_str());
    env_->CallVoidMethod(object_, setError_, msg);
}

jobject BuJoPage::getOriginal() {
    return env_->CallObjectMethod(object_, getOriginal_);
}

TaskNotifier::TaskNotifier(JNIEnv *env, jobject task, const char * className) {
    env_ = env;
    object_ = task;
    class_ = env_->FindClass(className);
    if(!class_) throw std::runtime_error("Failed to locate AsyncTask class!");

    notify_ = env_->GetMethodID(class_, "notifyUpdate", "()V");
    if(!notify_) throw std::runtime_error("Failed to locate notify-method in AsyncTask class!");
}

void TaskNotifier::notify() const
{
    env_->CallVoidMethod(object_, notify_);
}

xt::xtensor<float, 2> bitmap2tensor(JNIEnv * env, jobject bitmap)
{
    AndroidBitmapInfo bitmapInfo;
    AndroidBitmap_getInfo(env, bitmap, &bitmapInfo);
    if(bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
        throw std::runtime_error("Unexpected format of original image, expected RGBA8!");

    unsigned char * data;
    AndroidBitmap_lockPixels(env, bitmap, reinterpret_cast<void**>(&data));
    xt::xtensor<float, 2> res({bitmapInfo.height, bitmapInfo.width});
    for(unsigned i = 0; i < bitmapInfo.height; i++)
    {
        unsigned char * row = data + i * bitmapInfo.stride;
        for(unsigned j = 0; j < bitmapInfo.width; j++)
        {
            unsigned char * elem = row + j * 4;
            res.at(i, j) = static_cast<float>(elem[0] + elem[1] + elem[2]) / (255.0f * 3.0f);
        }
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    return res;
}

void performDetection(BuJoPage &page, const BuJoSettings &settings, const TaskNotifier &notifier)
{
    jobject bitmap = page.getOriginal();
    auto original = bitmap2tensor(page.getEnv(), bitmap);
    page.setStatus(BuJoStatus::CONVERTED_BITMAP, "Converted bitmap!");
    notifier.notify();
}
