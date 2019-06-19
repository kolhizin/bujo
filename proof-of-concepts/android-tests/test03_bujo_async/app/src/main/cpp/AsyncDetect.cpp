//
// Created by KolhiziN on 18.06.2019.
//
#include "AsyncDetect.h"
#include <exception>
#include <android/bitmap.h>
#include <sstream>

BuJoSettings::BuJoSettings(JNIEnv *env, jobject settings) {
    env_ = env;
    object_ = settings;
    class_ = env_->FindClass("com/kolhizin/asyncbujo/BuJoSettings");
    if(!class_)
        throw std::runtime_error("Could not link BuJoSettings-class in JNI!");

    loadFloatField_("detectorScaleFactor");
    loadFloatField_("detectorMaxAlignAngle");

    loadFloatField_("splitMinAngle");
    loadIntField_("splitNumAngle");
    loadFloatField_("splitMinIntensityAbs");
    loadFloatField_("splitMaxIntersectionAbs");
    loadFloatField_("splitMinRatio");
}

float BuJoSettings::getFloatValue(const std::string &name, float defValue) const {
    auto it = fields_.find(name);
    if(it == fields_.end())
        return defValue;
    return env_->GetFloatField(object_, it->second);
}

int BuJoSettings::getIntValue(const std::string &name, int defValue) const {
    auto it = fields_.find(name);
    if(it == fields_.end())
        return defValue;
    return env_->GetIntField(object_, it->second);
}

BuJoPage::BuJoPage(JNIEnv *env, jobject page) {
    env_ = env;
    object_ = page;
    class_ = env_->FindClass("com/kolhizin/asyncbujo/BuJoPage");
    if(!class_)
        throw std::runtime_error("Could not link BuJoPage-class in JNI!");

    loadMethod_(setError_, "setError", "(Ljava/lang/String;)V");
    loadMethod_(getOriginal_, "getOriginal", "()Landroid/graphics/Bitmap;");

    loadMethod_(setAngle_, "setAngle", "(F)V");
    loadMethod_(addSplit_, "addSplit", "(F,F,F,I)V");

    loadMethod_(setStatusTransformedImage_, "setStatusTransformedImage", "(Ljava/lang/String;)V");
    loadMethod_(setStatusStartedDetector_, "setStatusStartedDetector", "(Ljava/lang/String;)V");
    loadMethod_(setStatusDetectedAngle_, "setStatusDetectedAngle", "(Ljava/lang/String;)V");
    loadMethod_(setStatusAlignedImages_, "setStatusAlignedImages", "(Ljava/lang/String;)V");
    loadMethod_(setStatusFilteredImages_, "setStatusFilteredImages", "(Ljava/lang/String;)V");
    loadMethod_(setStatusDetectedRegion_, "setStatusDetectedRegion", "(Ljava/lang/String;)V");
}

void BuJoPage::setStatus(BuJoStatus status, const std::string &message)
{
    jobject msg = env_->NewStringUTF(message.c_str());
    if(status == BuJoStatus::CONVERTED_BITMAP)
        env_->CallVoidMethod(object_, setStatusTransformedImage_, msg);
    else if(status == BuJoStatus::LOADED_DETECTOR)
        env_->CallVoidMethod(object_, setStatusStartedDetector_, msg);
    else if(status == BuJoStatus::DETECTED_ANGLE)
        env_->CallVoidMethod(object_, setStatusDetectedAngle_, msg);
    else if(status == BuJoStatus::ALIGNED_IMAGES)
        env_->CallVoidMethod(object_, setStatusAlignedImages_, msg);
    else if(status == BuJoStatus::FILTERED_IMAGES)
        env_->CallVoidMethod(object_, setStatusFilteredImages_, msg);
    else if(status == BuJoStatus::DETECTED_REGION)
        env_->CallVoidMethod(object_, setStatusDetectedRegion_, msg);
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
    //convert to tensor
    jobject bitmap = page.getOriginal();
    auto original = bitmap2tensor(page.getEnv(), bitmap);
    page.setStatus(BuJoStatus::CONVERTED_BITMAP, "Converted bitmap! Loading in detector...");
    notifier.notify();

    //load into detectos
    bujo::detector::Detector detector;
    float scaleFactor = settings.getFloatValue("detectorScaleFactor", 1.0f);
    detector.loadImage(original, scaleFactor);
    page.setStatus(BuJoStatus::LOADED_DETECTOR, "Loaded in detector.");
    notifier.notify();

    //detect angle
    float angle = detector.detectAngle(settings.getFloatValue("detectorMaxAlignAngle", 1.57f));
    page.setAngle(angle);
    std::stringstream ss0;
    ss0 << "Image " << original.shape()[0] << "x" << original.shape()[1] << "/" << angle;
    page.setStatus(BuJoStatus::DETECTED_ANGLE, ss0.str());
    notifier.notify();

    //use angle to align
    detector.alignImages();
    page.setStatus(BuJoStatus::ALIGNED_IMAGES, "Aligned images.");
    notifier.notify();

    //filter images
    bujo::detector::FilteringOptions filteringOptions;
    detector.filterImages(filteringOptions);
    page.setStatus(BuJoStatus::FILTERED_IMAGES, "Filtered images.");
    notifier.notify();

    //select region
    float minSplitAngle = settings.getFloatValue("splitMinAngle", 0.5f);
    unsigned numSplitAngle = settings.getIntValue("splitNumAngle", 50);
    float minSplitIntensityAbs = settings.getFloatValue("splitMinIntensityAbs", 10.0f);
    float maxSplitIntersectionAbs = settings.getFloatValue("splitMaxIntersectionAbs", 2.0f);
    float minSplitRatio = settings.getFloatValue("splitMinRatio", 0.05f);
    detector.updateRegionAuto(minSplitAngle, numSplitAngle, minSplitIntensityAbs, maxSplitIntersectionAbs, minSplitRatio);
    std::stringstream ss1;
    ss1 << "Image " << original.shape()[0] << "x" << original.shape()[1] << "/" << angle << "*" << detector.numSplits();
    for(unsigned i = 0; i < detector.numSplits(); i++)
        page.addSplit(detector.getSplit(i));
    page.setStatus(BuJoStatus::DETECTED_REGION, ss1.str());
    notifier.notify();
}
