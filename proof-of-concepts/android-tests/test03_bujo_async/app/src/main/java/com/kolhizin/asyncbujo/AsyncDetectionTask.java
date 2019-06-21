package com.kolhizin.asyncbujo;

import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.net.Uri;
import android.os.AsyncTask;
import android.provider.MediaStore;
import android.widget.TextView;

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
            Bitmap img0 = loadImage(context_, params[0], maxImageDim_);
            res_.setOriginal(img0, 0.5f);
        }catch (FileNotFoundException e)
        {
            e.printStackTrace();
            res_.setError("Image not found!");
            return res_;
        }catch (IOException e)
        {
            e.printStackTrace();
            res_.setError("IO Exception: " + e.getMessage());
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

    public static Bitmap loadImage(Context context, Uri photoUri, int maxImageDim) throws IOException {
        InputStream is = context.getContentResolver().openInputStream(photoUri);
        BitmapFactory.Options dbo = new BitmapFactory.Options();
        dbo.inJustDecodeBounds = true;
        BitmapFactory.decodeStream(is, null, dbo);
        is.close();

        int rotatedWidth, rotatedHeight;
        int orientation = getOrientation(context, photoUri);

        if (orientation == 90 || orientation == 270) {
            rotatedWidth = dbo.outHeight;
            rotatedHeight = dbo.outWidth;
        } else {
            rotatedWidth = dbo.outWidth;
            rotatedHeight = dbo.outHeight;
        }

        Bitmap srcBitmap;
        is = context.getContentResolver().openInputStream(photoUri);
        if (rotatedWidth > maxImageDim || rotatedHeight > maxImageDim) {
            float widthRatio = ((float) rotatedWidth) / ((float) maxImageDim);
            float heightRatio = ((float) rotatedHeight) / ((float) maxImageDim);
            float maxRatio = Math.max(widthRatio, heightRatio);

            // Create the bitmap from file
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inSampleSize = (int) maxRatio;
            srcBitmap = BitmapFactory.decodeStream(is, null, options);
        } else {
            srcBitmap = BitmapFactory.decodeStream(is);
        }
        is.close();

        /*
         * if the orientation is not 0 (or -1, which means we don't know), we
         * have to do a rotation.
         */
        if (orientation > 0) {
            Matrix matrix = new Matrix();
            matrix.postRotate(orientation);

            srcBitmap = Bitmap.createBitmap(srcBitmap, 0, 0, srcBitmap.getWidth(),
                    srcBitmap.getHeight(), matrix, true);
        }

        String type = context.getContentResolver().getType(photoUri);
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        if (type.equals("image/png")) {
            srcBitmap.compress(Bitmap.CompressFormat.PNG, 100, baos);
        } else if (type.equals("image/jpg") || type.equals("image/jpeg")) {
            srcBitmap.compress(Bitmap.CompressFormat.JPEG, 100, baos);
        }
        byte[] bMapArray = baos.toByteArray();
        baos.close();
        return BitmapFactory.decodeByteArray(bMapArray, 0, bMapArray.length);
    }

    public static int getOrientation(Context context, Uri photoUri) {
        /* it's on the external media. */
        Cursor cursor = context.getContentResolver().query(photoUri,
                new String[] { MediaStore.Images.ImageColumns.ORIENTATION }, null, null, null);

        if (cursor.getCount() != 1) {
            return -1;
        }

        cursor.moveToFirst();
        return cursor.getInt(0);
    }

    public native int detect(BuJoPage page, BuJoSettings settings);
}
