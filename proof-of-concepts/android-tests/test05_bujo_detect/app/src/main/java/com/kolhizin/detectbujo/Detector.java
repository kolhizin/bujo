package com.kolhizin.detectbujo;

import android.graphics.Bitmap;

public class Detector {
    private BuJoPage page_ = null;
    private BuJoSettings settings_ = null;
    private long detectorHandle_ = 0, pageHandle_ = 0, settingsHandle_ = 0;

    public Detector(){
        settings_ = new BuJoSettings();
        page_ = new BuJoPage();
        detectorHandle_ = createDetector();
        pageHandle_ = createPage(page_);
        settingsHandle_ = createSettings(settings_);
    }
    public void reset(){
        dispose();
        page_ = new BuJoPage();
        settings_ = new BuJoSettings();
        detectorHandle_ = createDetector();
        pageHandle_ = createPage(page_);
        settingsHandle_ = createSettings(settings_);
    }
    public void dispose(){
        deleteDetector(detectorHandle_);
        detectorHandle_ = 0;
        deletePage(pageHandle_);
        pageHandle_ = 0;
        deleteSettings(settingsHandle_);
        settingsHandle_ = 0;
    }
    public BuJoSettings getSettings(){
        return settings_;
    }
    public void setSettings(BuJoSettings settings){
        deleteSettings(settingsHandle_);
        settings_ = settings;
        settingsHandle_ = createSettings(settings_);
    }
    public void load(Bitmap src) throws Exception{
        load(src, 1.0f);
    }

    public void load(Bitmap src, float scale) throws Exception{
        assertHandles_();
        page_.setOriginal(src, scale);
        String result = load(detectorHandle_, pageHandle_, settingsHandle_);
        if(!result.equals(""))
            throw new Exception(result);
    }

    public void preprocess() throws Exception{
        assertHandles_();
        String result = preprocess(detectorHandle_, pageHandle_, settingsHandle_);
        if(!result.equals(""))
            throw new Exception(result);
    }

    public void detectRegions() throws Exception{
        assertHandles_();
        String result = detectRegions(detectorHandle_, pageHandle_, settingsHandle_);
        if(!result.equals(""))
            throw new Exception(result);
    }

    public void detectLines() throws Exception{
        assertHandles_();
        String result = detectLines(detectorHandle_, pageHandle_, settingsHandle_);
        if(!result.equals(""))
            throw new Exception(result);
    }

    public void detectWords(int line) throws Exception{
        throw new Exception("Not implemented!");
    }

    public BuJoPage getPage() {
        return page_;
    }

    private void assertHandles_() throws Exception{
        if(detectorHandle_ == 0)
            throw new Exception("Detector: detector handle is uninitialized!");
        if(pageHandle_ == 0)
            throw new Exception("Detector: page handle is uninitialized!");
        if(settingsHandle_ == 0)
            throw new Exception("Detector: settings handle is uninitialized!");
    }

    private native long createDetector();
    private native void deleteDetector(long handle);

    private native long createPage(BuJoPage page);
    private native void deletePage(long handle);

    private native long createSettings(BuJoSettings settings);
    private native void deleteSettings(long handle);

    private native String load(long hDetector, long hPage, long hSettings);
    private native String preprocess(long hDetector, long hPage, long hSettings);
    private native String detectRegions(long hDetector, long hPage, long hSettings);
    private native String detectLines(long hDetector, long hPage, long hSettings);
}
