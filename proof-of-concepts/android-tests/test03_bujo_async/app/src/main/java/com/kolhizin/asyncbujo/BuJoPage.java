package com.kolhizin.asyncbujo;

import android.graphics.Bitmap;

public class BuJoPage {
    public class BuJoStatus{
        public String statusMessage, errorMessage;
        public boolean fInProgress;
        public boolean fErrors;

        public boolean fLoadedBitmap;
        public boolean fTransformedImage;
        public boolean fStartedDetector;
        public boolean fDetectAngle;
        public boolean fDetectRegion;
        public boolean fDetectCurves;
        public boolean fDetectLines;
        public boolean fDetectWords;

        public void resetStatus(){
            status.fInProgress = false;
            status.fErrors = false;

            status.fLoadedBitmap = false;
            status.fTransformedImage = false;
            status.fStartedDetector = false;
            status.fDetectAngle = false;
            status.fDetectRegion = false;
            status.fDetectCurves = false;
            status.fDetectLines = false;
            status.fDetectWords = false;

            status.statusMessage = "";
            status.errorMessage = "";
        }
    }
    public class BuJoLine {
        float [] xCoords;
        float [] yCoords;
        float [] lenParam;
    }


    private BuJoStatus status;
    private Bitmap original;


    public BuJoPage()
    {
        status = new BuJoStatus();
    }

    public void setOriginal(Bitmap bmp){
        original = bmp;
        status.resetStatus();
    }

    public Bitmap getOriginal(){
        return original;
    }

    public void setError(String msg){
        status.fErrors = true;
        status.errorMessage = msg;
    }

    public String getStatusMessage(){
        return status.statusMessage;
    }
    public String getErrorMessage(){
        return status.errorMessage;
    }

    public void setStatusStart(String msg){
        status.resetStatus();
        status.fInProgress = true;
        status.statusMessage = msg;
        status.errorMessage = "";
    }


    public void setStatusLoadedBitmap(String msg){
        status.fLoadedBitmap = true;

        status.statusMessage = msg;
    }


    public void setStatusTransformedImage(String msg){
        status.fTransformedImage = true;

        status.statusMessage = msg;
    }


    public void setStatusStartedDetector(String msg){
        status.fStartedDetector = true;

        status.statusMessage = msg;
    }

}
