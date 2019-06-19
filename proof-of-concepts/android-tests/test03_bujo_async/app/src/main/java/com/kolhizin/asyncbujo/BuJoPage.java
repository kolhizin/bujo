package com.kolhizin.asyncbujo;

import android.graphics.Bitmap;

import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;

public class BuJoPage {
    public class BuJoStatus{
        public String statusMessage, errorMessage;
        public boolean fInProgress;
        public boolean fErrors;

        public boolean fLoadedBitmap;
        public boolean fTransformedImage;
        public boolean fStartedDetector;
        public boolean fDetectedAngle;
        public boolean fAlignedImages;
        public boolean fFilteredImages;
        public boolean fDetectedRegion;
        public boolean fDetectedCurves;
        public boolean fDetectedLines;
        public boolean fDetectedWords;

        public void resetStatus(){
            status.fInProgress = false;
            status.fErrors = false;

            status.fLoadedBitmap = false;
            status.fTransformedImage = false;
            status.fStartedDetector = false;
            status.fDetectedAngle = false;
            status.fAlignedImages = false;
            status.fFilteredImages = false;
            status.fDetectedRegion = false;
            status.fDetectedCurves = false;
            status.fDetectedLines = false;
            status.fDetectedWords = false;

            status.statusMessage = "";
            status.errorMessage = "";
        }
    }
    public class BuJoLine {
        float [] xCoords;
        float [] yCoords;
        float [] lenParam;
    }
    public class BuJoSplit{
        float angle = 0.0f, offset = 0.0f, margin = 0.0f;
        int direction = 0;
    }


    private BuJoStatus status;
    private Bitmap original;
    private float angle;
    private List<BuJoSplit> splits;


    public BuJoPage()
    {
        status = new BuJoStatus();
        splits = new LinkedList<BuJoSplit>();
    }

    public void setOriginal(Bitmap bmp){
        original = bmp;
        status.resetStatus();
        splits.clear();
    }

    public void addSplit(float angle, float offset, float margin, int direction) {
        BuJoSplit split = new BuJoSplit();
        split.angle = angle;
        split.offset = offset;
        split.margin = margin;
        split.direction = direction;
        splits.add(split);
    }

    public void setAngle(float a){ angle = a; }
    public float getAngle(){ return angle; }

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
    public void setStatusDetectedAngle(String msg){
        status.fDetectedAngle = true;
        status.statusMessage = msg;
    }
    public void setStatusAlignedImages(String msg){
        status.fAlignedImages = true;
        status.statusMessage = msg;
    }
    public void setStatusFilteredImages(String msg){
        status.fFilteredImages = true;
        status.statusMessage = msg;
    }
    public void setStatusDetectedRegion(String msg){
        status.fDetectedRegion = true;
        status.statusMessage = msg;
    }
}
