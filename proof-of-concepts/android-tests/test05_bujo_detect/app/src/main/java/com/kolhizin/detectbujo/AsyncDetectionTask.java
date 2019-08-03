package com.kolhizin.detectbujo;

import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.media.ExifInterface;
import android.os.AsyncTask;
import android.provider.MediaStore;
import android.widget.TextView;
import android.widget.Toast;

import org.w3c.dom.Text;

import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;


public class AsyncDetectionTask extends AsyncTask<Bitmap, BuJoPage, BuJoPage> {
    private BuJoPage res_;
    private BuJoSettings settings_;
    private String task_;

    public AsyncDetectionTask(BuJoPage page, String task, BuJoSettings settings) throws Exception{
        super();
        res_ = page;
        settings_ = settings;
        if((task.equals("detect"))
                ||(task.equals("detectWords"))
                ||(task.equals("detectLines"))){
            task_ = task;
        }else throw new Exception("Unexpected task in AsyncDetectionTask!");
    }

    private BuJoPage detectTask(Bitmap img0){
        res_.setStatusStart("Loading image!");
        notifyUpdate();
        try {
            res_.setOriginal(img0, 0.5f);
        }catch (Exception e){
            e.printStackTrace();
            res_.setError("Exception: " + e.getMessage());
            return res_;
        }

        res_.setStatusLoadedBitmap("Finished reading image! Initiating full detection!");
        notifyUpdate();

        int res = detect(res_, settings_);
        res_.setStatusFinish(res == 0);
        return res_;
    }

    private BuJoPage detectLinesTask(Bitmap img0){
        res_.setStatusStart("Loading image!");
        notifyUpdate();
        try {
            res_.setOriginal(img0, 0.5f);
        }catch (Exception e){
            e.printStackTrace();
            res_.setError("Exception: " + e.getMessage());
            return res_;
        }

        res_.setStatusLoadedBitmap("Finished reading image! Initiating full detection!");
        notifyUpdate();

        int res = detectLines(res_, settings_);
        res_.setStatusFinish(res == 0);
        return res_;
    }

    @Override
    protected BuJoPage doInBackground(Bitmap ...params){
        if(task_.equals("detect")){
            return detectTask(params[0]);
        }else if(task_.equals("detectLines")){
            return detectLinesTask(params[0]);
        }else if(task_.equals("detectWords")) {
            return detectLinesTask(params[0]);
        }
        return null;
    }

    private void notifyUpdate(){
        publishProgress(res_);
    }

    @Override
    protected void onProgressUpdate(BuJoPage ...values){
        super.onProgressUpdate(values);
    }

    public native int detect(BuJoPage page, BuJoSettings settings);
    public native int detectLines(BuJoPage page, BuJoSettings settings);
    public native int detectWords(BuJoPage page, BuJoSettings settings);
}
