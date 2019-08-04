package com.kolhizin.detectbujo;

public class Detector {
    private long handle_ = 0;
    public Detector(){
        handle_ = createDetector();
    }
    public void reset(){
        dispose();
        handle_ = createDetector();
    }
    public void dispose(){
        deleteDetector(handle_);
        handle_ = 0;
    }


    private native long createDetector();
    private native void deleteDetector(long handle);
}
