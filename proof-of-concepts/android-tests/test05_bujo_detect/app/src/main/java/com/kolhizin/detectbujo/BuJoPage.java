package com.kolhizin.detectbujo;

import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.os.Parcel;
import android.os.Parcelable;

import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;

public class BuJoPage {
    public class BuJoStatus{
        public String statusMessage, errorMessage;
        public boolean fInProgress, fSuccess;
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
            status.fSuccess = false;
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

        float getDistance(float x, float y){
            //assumes that line is almost horizontal
            if(x < xCoords[0]){
                float dx = x - xCoords[0];
                float dy = y - yCoords[0];
                return (float)Math.sqrt(dx * dx + dy * dy);
            }else if(x > xCoords[xCoords.length - 1]){
                float dx = x - xCoords[xCoords.length - 1];
                float dy = y - yCoords[yCoords.length - 1];
                return (float)Math.sqrt(dx * dx + dy * dy);
            }
            int i0 = 0, i1 = 1;
            for(int i = 1; i < xCoords.length; i++){
                if((x >= xCoords[i-1])&&(x <= xCoords[i])){
                    i0 = i - 1;
                    i1 = i;
                    break;
                }
            }
            float a = (x - xCoords[i0]) / (xCoords[i1] - xCoords[i0]);
            float yx = yCoords[i0] * (1 - a) + yCoords[i1] * a;
            return Math.abs(yx - y);
        }
    }
    public class BuJoSplit{
        float angle = 0.0f, offset = 0.0f, margin = 0.0f;
        int direction = 0;
    }

    private BuJoStatus status;
    private Bitmap originalBitmap, alignedBitmap, detectorBitmap;
    private float angle;
    private List<BuJoSplit> splits;
    private List<BuJoLine> lines;
    private BuJoWord [][] words;

    public BuJoPage()
    {
        status = new BuJoStatus();
        splits = new LinkedList<BuJoSplit>();
        lines = new LinkedList<BuJoLine>();
    }

    public void setOriginal(Bitmap bmp, float detectorScale){
        originalBitmap = bmp;
        int dw = (int)(bmp.getWidth() * detectorScale);
        int dh = (int)(bmp.getHeight() * detectorScale);
        detectorBitmap = Bitmap.createScaledBitmap(bmp, dw, dh, true);

        status.resetStatus();
        splits.clear();
        lines.clear();
    }

    public void addSplit(float angle, float offset, float margin, int direction) {
        BuJoSplit split = new BuJoSplit();
        split.angle = angle;
        split.offset = offset;
        split.margin = margin;
        split.direction = direction;
        splits.add(split);
    }
    public List<BuJoSplit> getSplits(){return splits;}
    public BuJoSplit getSplit(int id){
        return splits.get(id);
    }
    public int numSplits(){
        return splits.size();
    }

    public void resetNumWordLines(int n){
        words = new BuJoWord[n][];
    }
    public void resetNumWords(int lineId, int n){
        words[lineId] = new BuJoWord[n];
    }
    public void setWord(int lineId, int wordId, float []xs, float []ys, float negOffset, float posOffset){
        BuJoWord w = new BuJoWord();
        w.xCoords = xs;
        w.yCoords = ys;
        w.negOffset = negOffset;
        w.posOffset = posOffset;
        words[lineId][wordId] = w;
    }
    public BuJoWord[][] getWords(){return words;}

    public void addLine(float []xs, float []ys){
        if(xs.length != ys.length)
            throw new IllegalArgumentException("BuJoPage.addLine: xs and ys should be same length!");
        BuJoLine line = new BuJoLine();
        line.xCoords = xs;
        line.yCoords = ys;
        lines.add(line);
    }
    public List<BuJoLine> getLines(){return lines;}
    public BuJoLine getLine(int id){
        return lines.get(id);
    }
    public int numLines(){
        return lines.size();
    }

    public int getClosestLine(float x, float y){
        if(lines.isEmpty())
            return -1;
        int id = 0;
        float dst = lines.get(0).getDistance(x, y);
        for(int i = 0; i < lines.size(); i++){
            BuJoLine line = lines.get(i);
            float cur_dst = line.getDistance(x, y);
            if(cur_dst < dst){
                dst = cur_dst;
                id = i;
            }
        }
        return id;
    }

    public BuJoStatus getStatus(){ return status; }

    public void setAngle(float a){
        angle = a;

        Matrix m = new Matrix();
        m.setRotate(-angle * 180.0f / (float)Math.PI);
        alignedBitmap = Bitmap.createBitmap(originalBitmap, 0, 0,
                originalBitmap.getWidth(), originalBitmap.getHeight(), m, true);
    }
    public float getAngle(){ return angle; }

    public Bitmap getOriginal(){
        return originalBitmap;
    }
    public Bitmap getSource(){
        return detectorBitmap;
    }
    public Bitmap getAligned() {return alignedBitmap; }



    public void setError(String msg){
        status.fErrors = true;
        status.fInProgress = false;
        status.fSuccess = false;
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
    public void setStatusFinish(boolean fSuccess){
        status.fInProgress = false;
        status.fSuccess = fSuccess;
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
    public void setStatusDetectedCurves(String msg){
        status.fDetectedCurves = true;
        status.statusMessage = msg;
    }
    public void setStatusDetectedLines(String msg){
        status.fDetectedLines = true;
        status.statusMessage = msg;
    }
    public void setStatusDetectedWords(String msg){
        status.fDetectedWords = true;
        status.statusMessage = msg;
    }
}
