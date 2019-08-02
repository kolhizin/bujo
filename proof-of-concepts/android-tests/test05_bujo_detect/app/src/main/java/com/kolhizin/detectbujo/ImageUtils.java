package com.kolhizin.detectbujo;

import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.media.ExifInterface;
import android.net.Uri;
import android.provider.MediaStore;
import android.widget.Toast;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class ImageUtils {
    public static float [] toLocalCoord(float x, float y, int w, int h, int w0, int h0){
        float [] res = new float[2];
        float f = ((float)w)/h;
        float f0 = ((float)w0)/h0;
        if(f == f0){
            res[0] = x / w;
            res[1] = y / h;
        }else if(f > f0){
            float diff = (w - h*f0) * 0.5f;
            res[0] = (x - diff) / (h * f0);
            res[1] = y / h;
        }else{
            float diff = (h - w/f0) * 0.5f;
            res[0] = x / w;
            res[1] = (y - diff) / (w / f0);
        }
        return res;
    }
    public static Bitmap loadImage(Context context, Uri photoUri, int maxImageDim) throws IOException {
        InputStream is = context.getContentResolver().openInputStream(photoUri);
        BitmapFactory.Options dbo = new BitmapFactory.Options();
        dbo.inJustDecodeBounds = true;
        BitmapFactory.decodeStream(is, null, dbo);
        is.close();

        int rotatedWidth = dbo.outWidth, rotatedHeight = dbo.outHeight;
        //int orientation = getOrientation(context, photoUri);
        int orientation = getOrientationExif(context, photoUri);

        if(orientation == -1){
            if(dbo.outWidth > dbo.outHeight)
            {
                orientation = 90;
                rotatedWidth = dbo.outHeight;
                rotatedHeight = dbo.outWidth;
            }
        }else if (orientation == 90 || orientation == 270) {
            rotatedWidth = dbo.outHeight;
            rotatedHeight = dbo.outWidth;
        }else{
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
/*
    public static int getOrientation(Context context, Uri photoUri) {
        Cursor cursor = context.getContentResolver().query(photoUri,
                new String[] { MediaStore.Images.ImageColumns.ORIENTATION }, null, null, null);

        int totCount = cursor.getCount();
        int colCount = cursor.getColumnCount();
        if (colCount != 1) {
            return -1;
        }

        cursor.moveToFirst();
        return cursor.getInt(0);
    }
*/
    public static int getOrientationExif(Context context, Uri photoUri){
        ExifInterface ei = null;
        try {
            InputStream inputStream = context.getContentResolver().openInputStream(photoUri);
            ei = new ExifInterface(inputStream);
        }catch(IOException e){
            Toast.makeText(context, "Failed to load Exif!", Toast.LENGTH_LONG);
            return 0;
        }
        int orientation = ei.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_UNDEFINED);
        switch (orientation){
            case ExifInterface.ORIENTATION_NORMAL: return 0;
            case ExifInterface.ORIENTATION_ROTATE_90: return 90;
            case ExifInterface.ORIENTATION_ROTATE_180: return 180;
            case ExifInterface.ORIENTATION_ROTATE_270: return 270;
        }
        return -1;
    }
}
