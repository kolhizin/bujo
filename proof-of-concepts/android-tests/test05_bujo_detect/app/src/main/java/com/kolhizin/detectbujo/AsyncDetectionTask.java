package com.kolhizin.detectbujo;

import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.media.ExifInterface;
import android.net.Uri;
import android.os.AsyncTask;
import android.provider.MediaStore;
import android.widget.TextView;
import android.widget.Toast;

import org.w3c.dom.Text;

import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;


public class AsyncDetectionTask extends AsyncTask<Uri, BuJoPage, BuJoPage> {
    private Context context_;
    private int maxImageDim_;
    private BuJoPage res_;
    private BuJoSettings settings_;

    public AsyncDetectionTask(Context context, BuJoSettings settings, int maxImageDim){
        super();
        settings_ = settings;
        context_ = context;
        maxImageDim_ = maxImageDim;
    }

    @Override
    protected BuJoPage doInBackground(Uri ...params){
        res_ = new BuJoPage();
        res_.setStatusStart("Loading image!");
        notifyUpdate();

        try {
            Bitmap img0 = ImageUtils.loadImage(context_, params[0], maxImageDim_);
            res_.setOriginal(img0, 0.5f);
        }catch (FileNotFoundException e){
            e.printStackTrace();
            res_.setError("Image not found!");
            Toast.makeText(context_, e.getMessage(), Toast.LENGTH_LONG).show();
            return res_;
        }catch (IOException e){
            e.printStackTrace();
            res_.setError("IO Exception: " + e.getMessage());
            Toast.makeText(context_, e.getMessage(), Toast.LENGTH_LONG).show();
            return res_;
        }catch (Exception e){
            e.printStackTrace();
            res_.setError("Exception: " + e.getMessage());
            Toast.makeText(context_, e.getMessage(), Toast.LENGTH_LONG).show();
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
