package com.kolhizin.asyncbujo;

import android.graphics.Bitmap;

public class BuJoPage {
    public class BuJoStatus{
        public String statusMessage;
        public boolean fInProgress;
        public boolean fErrors;

        public boolean fLoaded;
        public boolean fDetectAngle;
        public boolean fDetectRegion;
        public boolean fDetectCurves;
        public boolean fDetectLines;
        public boolean fDetectWords;
    }
    public class BuJoLine {
        float [] xCoords;
        float [] yCoords;
        float [] lenParam;
    }

    public String status;
    public Bitmap original;
}
