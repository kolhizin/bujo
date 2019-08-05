package com.kolhizin.detectbujo;

import android.graphics.Bitmap;

public class Detector {
    private BuJoPage page_ = null;
    private BuJoSettings settings_ = null;
    private long detectorHandle_ = 0;

    public Detector(){
        settings_ = new BuJoSettings();
        page_ = new BuJoPage();
        detectorHandle_ = createDetector();
    }
    public void reset(){
        dispose();
        page_ = new BuJoPage();
        settings_ = new BuJoSettings();
        detectorHandle_ = createDetector();
    }
    public void dispose(){
        deleteDetector(detectorHandle_);
        detectorHandle_ = 0;
    }
    public BuJoSettings getSettings(){
        return settings_;
    }
    public void setSettings(BuJoSettings settings){
        settings_ = settings;
    }
    public void load(Bitmap src) throws Exception{
        load(src, 0.5f);
    }

    public void load(Bitmap src, float scale) throws Exception{
        assertHandles_();
        page_.setOriginal(src, scale);
        String result = load(detectorHandle_, page_, settings_);
        if(!result.equals(""))
            throw new Exception(result);
    }

    public void preprocess() throws Exception{
        assertHandles_();
        String result = preprocess(detectorHandle_, page_, settings_);
        if(!result.equals(""))
            throw new Exception(result);
    }

    public void detectRegions() throws Exception{
        assertHandles_();
        String result = detectRegions(detectorHandle_, page_, settings_);
        if(!result.equals(""))
            throw new Exception(result);
    }

    public void detectLines() throws Exception{
        assertHandles_();
        String result = detectLines(detectorHandle_, page_, settings_);
        if(!result.equals(""))
            throw new Exception(result);
        long numLines = getNumDetectedLines(detectorHandle_);
        page_.resetNumLines((int)numLines);
    }

    public void detectWords(int id) throws Exception{
        assertHandles_();
        String result = detectWords(detectorHandle_, id, page_, settings_);
        if(!result.equals(""))
            throw new Exception(result);
    }

    public BuJoPage getPage() {
        return page_;
    }

    private void assertHandles_() throws Exception{
        if(detectorHandle_ == 0)
            throw new Exception("Detector: detector handle is uninitialized!");
    }

    private native long createDetector();
    private native void deleteDetector(long handle);
    private native long getNumDetectedLines(long handle);

    private native String load(long hDetector, BuJoPage page, BuJoSettings settings);
    private native String preprocess(long hDetector, BuJoPage page, BuJoSettings settings);
    private native String detectRegions(long hDetector, BuJoPage page, BuJoSettings settings);
    private native String detectLines(long hDetector, BuJoPage page, BuJoSettings settings);
    private native String detectWords(long hDetector, int lineId, BuJoPage page, BuJoSettings settings);
}
