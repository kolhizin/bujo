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

    public AsyncDetectionTask(BuJoPage page, BuJoSettings settings){
        super();
        res_ = page;
        settings_ = settings;
    }


    @Override
    protected BuJoPage doInBackground(Bitmap ...params){
        res_.setStatusStart("Loading image!");
        notifyUpdate();
        try {
            res_.setOriginal(params[0], 0.5f);
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

    private void notifyUpdate(){
        publishProgress(res_);
    }

    @Override
    protected void onProgressUpdate(BuJoPage ...values){
        super.onProgressUpdate(values);
    }

    public native int detect(BuJoPage page, BuJoSettings settings);
}
