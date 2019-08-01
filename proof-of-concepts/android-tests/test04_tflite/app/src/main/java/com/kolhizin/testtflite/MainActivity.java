package com.kolhizin.testtflite;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.res.AssetFileDescriptor;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.icu.text.UnicodeSetSpanner;
import android.media.ExifInterface;
import android.net.Uri;
import android.os.Environment;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.lang.reflect.Array;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.LinkedList;
import java.util.List;
import com.kolhizin.testtflite.Classifier;

import org.tensorflow.lite.Tensor;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    private Classifier classifier_ = null;
    private String chars_ = null;

    private TextView textView = null;
    private ImageView imgView = null;
    private Button btnLoad = null;
    private final int LOAD_IMAGE_RESULTS = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        textView = (TextView)findViewById(R.id.mainText);
        imgView = (ImageView)findViewById(R.id.imgView);
        btnLoad = (Button)findViewById(R.id.btnLoad);

        btnLoad.setOnClickListener(this);

        Bitmap testBmp = null;
        try {
            testBmp = BitmapFactory.decodeStream(getAssets().open("htr_test.jpg"));
            classifier_ = new Classifier(this, "model.tflite", "model.chars");
        }catch (IOException e){
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
        }catch (Exception e){
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
        }

        long tStart = System.currentTimeMillis();
        String res = "";
        try{
            res = classifier_.detect(testBmp, 0.7f);
        }catch (Exception e){
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
        }
        long tEnd = System.currentTimeMillis();

        double tInfer = (tEnd-tStart)/1000.0;
        String strTime = "";
        strTime += "Inference: " + Double.toString(tInfer) + "s;\n";
        textView.setText("Result: " + res + "\n" + strTime);
    }

    private void setBitmap(Bitmap bmp){
        imgView.setImageBitmap(bmp);
        String res = "";
        boolean ok = false;
        long tStart = System.currentTimeMillis();
        try{
            res = classifier_.detect(bmp, 0.7f);
            ok = true;
        }catch (Exception e){
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
        }
        if(ok){
            double tElapsed = (System.currentTimeMillis() - tStart)/1000.0;
            textView.setText("Decoded: `" + res + "` in " + Double.toString(tElapsed) + "s.");
        }else{
            textView.setText("Detection failed!");
        }

    }

    private void loadPicture(){
        Intent intent = new Intent(Intent.ACTION_PICK,
                android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        startActivityForResult(intent, LOAD_IMAGE_RESULTS);
        textView.setText("Wait");
    }

    @Override
    public void onClick(View v){
        switch(v.getId())
        {
            case R.id.btnLoad: loadPicture(); return;
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == LOAD_IMAGE_RESULTS && resultCode == RESULT_OK && data != null) {
            Uri pickedImage = data.getData();

            Bitmap bmp = null;
            try{
                bmp = loadImage(this, pickedImage, 4096);
            }catch(Exception e){
                Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG);
            }
            setBitmap(bmp);
        }
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
            //    orientation = 90;
            //    rotatedWidth = dbo.outHeight;
            //    rotatedHeight = dbo.outWidth;
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

    public static int getOrientationExif(Context context, Uri photoUri){
        ExifInterface ei = null;
        try {
            InputStream inputStream = context.getContentResolver().openInputStream(photoUri);
            ei = new ExifInterface(inputStream);
        }catch(IOException e){
            Toast.makeText(context, "Failed to load Exif!", Toast.LENGTH_LONG).show();
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
