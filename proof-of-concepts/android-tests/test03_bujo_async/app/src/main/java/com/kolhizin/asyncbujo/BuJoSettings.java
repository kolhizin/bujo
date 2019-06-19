package com.kolhizin.asyncbujo;

public class BuJoSettings {
    public float detectorScaleFactor = 0.2f; //scale-factor of image that is used for image filtering and line-detection
    public float detectorMaxAlignAngle = 1.57f; //bounds on search range for best align angle

    public float splitMinAngle = 0.5f; //lower bounds on angle in region-detection by vertical splits
    public int splitNumAngle = 50; //number of test-angles during region-detection
    public float splitMinIntensityAbs = 10.0f; //intensity of "text" in each of regions identified by split
    public float splitMaxIntersectionAbs = 2.0f; //intensity of intersection of split-line
    public float splitMinRatio = 0.05f; //minimal ratio of erased region volume to whole image
}
