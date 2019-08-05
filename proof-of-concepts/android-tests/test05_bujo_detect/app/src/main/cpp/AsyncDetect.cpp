//
// Created by KolhiziN on 02.08.2019.
//

#include "AsyncDetect.h"

#include <exception>
#include <android/bitmap.h>
#include <sstream>
#include <xtensor/xview.hpp>

BuJoSettings::BuJoSettings(JNIEnv *env, jobject settings) {
    env_ = env;
    object_ = settings;
    class_ = env_->FindClass("com/kolhizin/detectbujo/BuJoSettings");
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
    class_ = env_->FindClass("com/kolhizin/detectbujo/BuJoPage");
    if(!class_)
        throw std::runtime_error("Could not link BuJoPage-class in JNI!");

    loadMethod_(setError_, "setError", "(Ljava/lang/String;)V");
    loadMethod_(getSource_, "getSource", "()Landroid/graphics/Bitmap;");

    loadMethod_(setAngle_, "setAngle", "(F)V");
    loadMethod_(addSplit_, "addSplit", "(FFFI)V");
    loadMethod_(addLine_, "addLine", "([F[F)V");
    loadMethod_(resetNumWordLines_, "resetNumWordLines", "(I)V");
    loadMethod_(resetNumWords_, "resetNumWords", "(II)V");
    loadMethod_(setWord_, "setWord", "(II[F[FFF)V");

    loadMethod_(setStatusTransformedImage_, "setStatusTransformedImage", "(Ljava/lang/String;)V");
    loadMethod_(setStatusStartedDetector_, "setStatusStartedDetector", "(Ljava/lang/String;)V");
    loadMethod_(setStatusDetectedAngle_, "setStatusDetectedAngle", "(Ljava/lang/String;)V");
    loadMethod_(setStatusAlignedImages_, "setStatusAlignedImages", "(Ljava/lang/String;)V");
    loadMethod_(setStatusFilteredImages_, "setStatusFilteredImages", "(Ljava/lang/String;)V");
    loadMethod_(setStatusDetectedRegion_, "setStatusDetectedRegion", "(Ljava/lang/String;)V");
    loadMethod_(setStatusDetectedCurves_, "setStatusDetectedCurves", "(Ljava/lang/String;)V");
    loadMethod_(setStatusDetectedLines_, "setStatusDetectedLines", "(Ljava/lang/String;)V");
    loadMethod_(setStatusDetectedWords_, "setStatusDetectedWords", "(Ljava/lang/String;)V");
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
    else if(status == BuJoStatus::DETECTED_CURVES)
        env_->CallVoidMethod(object_, setStatusDetectedCurves_, msg);
    else if(status == BuJoStatus::DETECTED_LINES)
        env_->CallVoidMethod(object_, setStatusDetectedLines_, msg);
    else if(status == BuJoStatus::DETECTED_WORDS)
        env_->CallVoidMethod(object_, setStatusDetectedWords_, msg);
    else
        throw std::runtime_error("Unexpected status in BuJoPage::setStatus!");
}

void BuJoPage::setError(const std::string &str) {
    jobject msg = env_->NewStringUTF(str.c_str());
    env_->CallVoidMethod(object_, setError_, msg);
}

jobject BuJoPage::getSource() {
    return env_->CallObjectMethod(object_, getSource_);
}

void BuJoPage::addLine(const bujo::curves::Curve &curve) const {
    jfloatArray xArr = env_->NewFloatArray(curve.x_value.size());
    jfloatArray yArr = env_->NewFloatArray(curve.y_value.size());
    env_->SetFloatArrayRegion(xArr, 0, curve.x_value.size(), &curve.x_value[0]);
    env_->SetFloatArrayRegion(yArr, 0, curve.y_value.size(), &curve.y_value[0]);
    env_->CallVoidMethod(object_, addLine_, xArr, yArr);
}

void BuJoPage::setWord(int lid, int wid, const xt::xtensor<float, 1> &xCoord, const xt::xtensor<float, 1> &yCoord,
                       float negOffset, float posOffset) const {
    jfloatArray xArr = env_->NewFloatArray(xCoord.size());
    jfloatArray yArr = env_->NewFloatArray(yCoord.size());
    env_->SetFloatArrayRegion(xArr, 0, xCoord.size(), &xCoord[0]);
    env_->SetFloatArrayRegion(yArr, 0, yCoord.size(), &yCoord[0]);
    env_->CallVoidMethod(object_, setWord_, lid, wid, xArr, yArr, negOffset, posOffset);
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

void asyncDetection(BuJoPage &page, const BuJoSettings &settings, const TaskNotifier &notifier)
{
    //convert to tensor
    jobject bitmap = page.getSource();
    auto original = bitmap2tensor(page.getEnv(), bitmap);
    page.setStatus(BuJoStatus::CONVERTED_BITMAP, "Converted bitmap! Loading in detector...");
    notifier.notify();

    //load into detectos
    bujo::detector::Detector detector;
    float scaleFactor = settings.getFloatValue("detectorScaleFactor", 1.0f);
    detector.loadImage(original, scaleFactor);
    page.setStatus(BuJoStatus::LOADED_DETECTOR, "Loaded in detector. Detecting angle...");
    notifier.notify();

    //detect angle
    float angle = detector.detectAngle(settings.getFloatValue("detectorMaxAlignAngle", 1.57f));
    page.setAngle(angle);
    std::stringstream ss0;
    ss0 << "Detected angle (" << angle << "). Aligning images...";
    page.setStatus(BuJoStatus::DETECTED_ANGLE, ss0.str());
    notifier.notify();

    //use angle to align
    detector.alignImages();
    page.setStatus(BuJoStatus::ALIGNED_IMAGES, "Aligned images. Filtering images...");
    notifier.notify();

    //filter images
    bujo::detector::FilteringOptions filteringOptions;
    detector.filterImages(filteringOptions);
    page.setStatus(BuJoStatus::FILTERED_IMAGES, "Filtered images. Detecting regions...");
    notifier.notify();

    //select region
    float minSplitAngle = settings.getFloatValue("splitMinAngle", 0.5f);
    unsigned numSplitAngle = settings.getIntValue("splitNumAngle", 50);
    float minSplitIntensityAbs = settings.getFloatValue("splitMinIntensityAbs", 10.0f);
    float maxSplitIntersectionAbs = settings.getFloatValue("splitMaxIntersectionAbs", 2.0f);
    float minSplitRatio = settings.getFloatValue("splitMinRatio", 0.05f);
    detector.updateRegionAuto(minSplitAngle, numSplitAngle, minSplitIntensityAbs, maxSplitIntersectionAbs, minSplitRatio);
    auto mainShape = detector.mainImage().shape();
    float mainDiag = std::sqrtf(mainShape[0]*mainShape[0]+mainShape[1]*mainShape[1]);
    for(unsigned i = 0; i < detector.numSplits(); i++) {
        page.addSplit(bujo::splits::rescaleSplit(detector.getSplit(i).desc, 1.0f / mainDiag));
    }
    page.setStatus(BuJoStatus::DETECTED_REGION, "Detected regions. Detecting curves...");
    notifier.notify();

    //select support surves
    bujo::curves::CurveGenerationOptions curveGenerationOptions;
    int numSupportCurves = settings.getIntValue("curvesNum", 6);
    float curvesXWindow = settings.getFloatValue("curveXWindowRel", 0.08f);
    float curvesQuantileV = settings.getFloatValue("curveVQuantile", 0.5f);
    float curvesQuantileH = settings.getFloatValue("curveHQuantile", 0.5f);
    float curvesRegCoef = settings.getFloatValue("curveRegCoef", 0.5f);
    int curvesXWindowAbs = curvesXWindow * detector.mainImage().shape()[1];

    page.setStatus(BuJoStatus::DETECTED_REGION, "Detected region. Detecting curves...");
    notifier.notify();

    detector.selectSupportCurvesAuto(numSupportCurves, curvesXWindowAbs, curvesQuantileV, curvesQuantileH,
                                     curvesRegCoef, curveGenerationOptions);
    page.setStatus(BuJoStatus::DETECTED_CURVES, "Detected curves. Detecting lines...");
    notifier.notify();

    float linesXWindow = settings.getFloatValue("lineXWindowRel", 0.08f);
    int linesXWindowAbs = linesXWindow * detector.mainImage().shape()[1];
    detector.detectLines(linesXWindowAbs);
    float xDetSize = detector.mainImage().shape()[1];
    float yDetSize = detector.mainImage().shape()[0];
    for(int i = 0; i < detector.numLineCurves(); i++){
        page.addLine(bujo::curves::affineTransformCurve(detector.getLineCurve(i),
                                                        0.0f, 1.0f/xDetSize, 0.0f, 1.0f/yDetSize));
    }
    page.setStatus(BuJoStatus::DETECTED_LINES, "Detected lines. Detecting words...");
    notifier.notify();

    bujo::curves::WordDetectionOptions wordDetectionOptions;
    wordDetectionOptions.cutoff_word_std = settings.getFloatValue("wordStdDevCutoff", 0.02f);
    int wordFilterSize = settings.getIntValue("wordFilterSizeAbs", 4);
    float wordRegCoef = settings.getFloatValue("wordRegCoef", 0.1f);
    int wordXWindowAbs = settings.getIntValue("wordXWindowAbs", 5);
    detector.detectWords(wordXWindowAbs, wordFilterSize, wordRegCoef, wordDetectionOptions);

    page.resetNumWordLines(detector.numLines());
    int wordNumPoints = settings.getIntValue("wordNumPoints", 10);
    auto wordLocCoord = xt::linspace(0.0f, 1.0f, wordNumPoints);
    for(int i = 0; i < detector.numLines(); i++)
    {
        page.resetNumWords(i, detector.numWords(i));
        for(int j = 0; j < detector.numWords(i); j++)
        {
            auto wrd = detector.getWord(i, j, wordLocCoord);
            auto off = detector.getWordHeight(i, j);
            xt::xtensor<float, 1> xc = xt::view(wrd, xt::range(0, wrd.shape()[0]), 0);
            xt::xtensor<float, 1> yc = xt::view(wrd, xt::range(0, wrd.shape()[0]), 1);
            page.setWord(i, j, xc, yc, std::get<0>(off), std::get<1>(off));
        }
    }

    page.setStatus(BuJoStatus::DETECTED_WORDS, "Detected words.");
    notifier.notify();
}

void runLoad(bujo::detector::Detector &detector, BuJoPage &page, const BuJoSettings &settings) {
    jobject bitmap = page.getSource();
    auto original = bitmap2tensor(page.getEnv(), bitmap);
    page.setStatus(BuJoStatus::CONVERTED_BITMAP, "Converted bitmap! Loading in detector...");

    float scaleFactor = settings.getFloatValue("detectorScaleFactor", 1.0f);
    detector.loadImage(original, scaleFactor);
    page.setStatus(BuJoStatus::LOADED_DETECTOR, "Loaded in detector. Detecting angle...");
}

void runPreprocess(bujo::detector::Detector &detector, BuJoPage &page, const BuJoSettings &settings) {
    float angle = detector.detectAngle(settings.getFloatValue("detectorMaxAlignAngle", 1.57f));
    page.setAngle(angle);
    page.setStatus(BuJoStatus::DETECTED_ANGLE, "Detected angle. Aligning images...");

    detector.alignImages();
    page.setStatus(BuJoStatus::ALIGNED_IMAGES, "Aligned images. Filtering images...");

    bujo::detector::FilteringOptions filteringOptions;
    detector.filterImages(filteringOptions);
    page.setStatus(BuJoStatus::FILTERED_IMAGES, "Filtered images. Detecting regions...");
}

void runDetectRegions(bujo::detector::Detector &detector, BuJoPage &page, const BuJoSettings &settings) {
    float minSplitAngle = settings.getFloatValue("splitMinAngle", 0.5f);
    unsigned numSplitAngle = settings.getIntValue("splitNumAngle", 50);
    float minSplitIntensityAbs = settings.getFloatValue("splitMinIntensityAbs", 10.0f);
    float maxSplitIntersectionAbs = settings.getFloatValue("splitMaxIntersectionAbs", 2.0f);
    float minSplitRatio = settings.getFloatValue("splitMinRatio", 0.05f);
    detector.updateRegionAuto(minSplitAngle, numSplitAngle, minSplitIntensityAbs, maxSplitIntersectionAbs, minSplitRatio);
    auto mainShape = detector.mainImage().shape();
    float mainDiag = std::sqrtf(mainShape[0]*mainShape[0]+mainShape[1]*mainShape[1]);
    for(unsigned i = 0; i < detector.numSplits(); i++) {
        page.addSplit(bujo::splits::rescaleSplit(detector.getSplit(i).desc, 1.0f / mainDiag));
    }
    page.setStatus(BuJoStatus::DETECTED_REGION, "Detected regions. Detecting curves...");
}

void runDetectLines(bujo::detector::Detector &detector, BuJoPage &page, const BuJoSettings &settings) {
    bujo::curves::CurveGenerationOptions curveGenerationOptions;
    int numSupportCurves = settings.getIntValue("curvesNum", 6);
    float curvesXWindow = settings.getFloatValue("curveXWindowRel", 0.08f);
    float curvesQuantileV = settings.getFloatValue("curveVQuantile", 0.5f);
    float curvesQuantileH = settings.getFloatValue("curveHQuantile", 0.5f);
    float curvesRegCoef = settings.getFloatValue("curveRegCoef", 0.5f);
    int curvesXWindowAbs = curvesXWindow * detector.mainImage().shape()[1];

    detector.selectSupportCurvesAuto(numSupportCurves, curvesXWindowAbs, curvesQuantileV, curvesQuantileH,
                                     curvesRegCoef, curveGenerationOptions);
    page.setStatus(BuJoStatus::DETECTED_CURVES, "Detected curves. Detecting lines...");

    float linesXWindow = settings.getFloatValue("lineXWindowRel", 0.08f);
    int linesXWindowAbs = linesXWindow * detector.mainImage().shape()[1];
    detector.detectLines(linesXWindowAbs);
    float xDetSize = detector.mainImage().shape()[1];
    float yDetSize = detector.mainImage().shape()[0];
    for(int i = 0; i < detector.numLineCurves(); i++){
        page.addLine(bujo::curves::affineTransformCurve(detector.getLineCurve(i),
                                                        0.0f, 1.0f/xDetSize, 0.0f, 1.0f/yDetSize));
    }
    page.setStatus(BuJoStatus::DETECTED_LINES, "Detected lines. Detecting words...");
}

void runDetectWords(bujo::detector::Detector &detector, int lineId, BuJoPage &page,
                    const BuJoSettings &settings) {
    bujo::curves::WordDetectionOptions wordDetectionOptions;
    wordDetectionOptions.cutoff_word_std = settings.getFloatValue("wordStdDevCutoff", 0.02f);
    int wordFilterSize = settings.getIntValue("wordFilterSizeAbs", 4);
    float wordRegCoef = settings.getFloatValue("wordRegCoef", 0.1f);
    int wordXWindowAbs = settings.getIntValue("wordXWindowAbs", 5);
    detector.detectWords(static_cast<unsigned>(wordXWindowAbs), static_cast<unsigned>(wordFilterSize),
            wordRegCoef, wordDetectionOptions);

    page.resetNumWordLines(detector.numLines());
    int wordNumPoints = settings.getIntValue("wordNumPoints", 10);
    auto wordLocCoord = xt::linspace(0.0f, 1.0f, wordNumPoints);
    for(int i = 0; i < detector.numLines(); i++)
    {
        page.resetNumWords(i, detector.numWords(i));
        for(int j = 0; j < detector.numWords(i); j++)
        {
            auto wrd = detector.getWord(i, j, wordLocCoord);
            auto off = detector.getWordHeight(i, j);
            xt::xtensor<float, 1> xc = xt::view(wrd, xt::range(0, wrd.shape()[0]), 0);
            xt::xtensor<float, 1> yc = xt::view(wrd, xt::range(0, wrd.shape()[0]), 1);
            page.setWord(i, j, xc, yc, std::get<0>(off), std::get<1>(off));
        }
    }
}

