package com.kolhizin.detectbujo;

import android.app.Application;

public class BuJoApp extends Application {
    private Detector detector_ = null;
    private Classifier classifier_ = null;

    public BuJoPage getPage(){return detector_.getPage();}

    public Detector getDetector(){return detector_;}
    public void setDetector(Detector detector){detector_ = detector;}

    public Classifier getClassifier(){return classifier_;}
    public void setClassifier(Classifier classifier){classifier_ = classifier;}
}
