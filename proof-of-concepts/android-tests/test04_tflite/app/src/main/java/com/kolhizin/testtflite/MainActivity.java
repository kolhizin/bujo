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

        Bitmap input_ = null;
        float [][] output = null;
        try {
            chars_ = readAssetChars("model.chars");
            input_ = BitmapFactory.decodeStream(getAssets().open("htr_test.jpg"));
            classifier_ = new Classifier(this, "model.tflite");
        }catch (IOException e){
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG);
        }

        long tStart = System.currentTimeMillis();
        long tPrep = tStart;
        long tGray = tStart;
        long tCutoff = tStart;
        long tTrim = tStart;
        long tFit = tStart;
        long tNorm = tStart;
        try {

            float [][] in_tmp1 = transformGrayscale(input_);
            tGray = System.currentTimeMillis();
            transformCutoff(in_tmp1, 0.7f, 1.0f, 0.0f);
            tCutoff = System.currentTimeMillis();
            float [][] in_tmp2 = transformTrim(in_tmp1, 1e-3f);
            tTrim = System.currentTimeMillis();
            float [][] in_tmp3 = transformFitSize(in_tmp2, 32, 128, 0.0f);
            tFit = System.currentTimeMillis();
            transformMeanStd(in_tmp3, -1.0f);
            tNorm = System.currentTimeMillis();
            float [][] in_tmp4 = transformTranspose(in_tmp3);

            tPrep = System.currentTimeMillis();
            output = classifier_.detect(in_tmp4);
        }catch (Exception e){
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
        }

        String res = decodeOutput(output, chars_);
        long tEnd = System.currentTimeMillis();
        double tArgPrep = (tPrep-tStart)/1000.0;
        double tInfer = (tEnd-tPrep)/1000.0;
        String strTime = "";
        strTime += "Arg-prep: " + Double.toString(tArgPrep) + "s;\n";
        strTime += "Inference: " + Double.toString(tInfer) + "s;\n";
        strTime += "AP\tGray: " + Double.toString((tGray - tStart)/1000.0) + "s;\n";
        strTime += "AP\tCutoff: " + Double.toString((tCutoff - tGray)/1000.0) + "s;\n";
        strTime += "AP\tTrim: " + Double.toString((tTrim - tCutoff)/1000.0) + "s;\n";
        strTime += "AP\tFit: " + Double.toString((tFit - tTrim)/1000.0) + "s;\n";
        strTime += "AP\tNorm: " + Double.toString((tNorm - tFit)/1000.0) + "s;\n";
        strTime += "AP\tTranspose: " + Double.toString((tArgPrep - tNorm)/1000.0) + "s;\n";
        textView.setText("Result: " + res + "\n" + strTime);
    }

    private void setBitmap(Bitmap bmp){
        imgView.setImageBitmap(bmp);
        float [][] input = prepareInput(bmp, 0.7f, 32, 128);
        float [][] output = classifier_.detect(input);
        String res = decodeOutput(output, chars_);
        textView.setText("Decoded: " + res);
    }

    private float [][] prepareInput(Bitmap src, float cutoff, int rows, int cols) {
        float [][] res = null;
        try {
            float[][] in_tmp1 = transformGrayscale(src);
            transformCutoff(in_tmp1, cutoff, 1.0f, 0.0f);
            float[][] in_tmp2 = transformTrim(in_tmp1, 1e-3f);
            float[][] in_tmp3 = transformFitSize(in_tmp2, rows, cols, 0.0f);
            transformMeanStd(in_tmp3, -1.0f);
            res = transformTranspose(in_tmp3);
        }catch (Exception e){
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
        }
        return res;
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

        // Here we need to check if the activity that was triggers was the Image Gallery.
        // If it is the requestCode will match the LOAD_IMAGE_RESULTS value.
        // If the resultCode is RESULT_OK and there is some data we know that an image was picked.
        if (requestCode == LOAD_IMAGE_RESULTS && resultCode == RESULT_OK && data != null) {
            // Let's read picked image data - its URI
            Uri pickedImage = data.getData();
            // Let's read picked image path using content resolver

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

    private String decodeOutput(float [][] encoded, String chars){
        int terminal = encoded[0].length - 1;
        LinkedList<Integer> res0 = new LinkedList<Integer>();
        int prev = -1;
        for(int i = 0; i < encoded.length; i++){
            int best = 0;
            for(int j = 1; j < encoded[i].length; j++){
                if(encoded[i][j] > encoded[i][best])
                    best = j;
            }
            if(best == prev)
                continue;
            prev = best;
            if(best != terminal)
                res0.add(best);
        }
        StringBuffer res = new StringBuffer();
        for(int i = 0; i < res0.size(); i++)
            res.append(chars.charAt(res0.get(i)));
        return res.toString();
    }

    private Bitmap array2Bitmap(float [][] src){
        int rows = src.length;
        int cols = src[0].length;

        byte [] bitmapdata = new byte[rows * cols];
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < cols; j++){
                bitmapdata[i*cols + j] = (byte)(255.0f * src[i][j]);
            }
        }
        Bitmap bmp = Bitmap.createBitmap(cols, rows, Bitmap.Config.ALPHA_8);
        ByteBuffer buffer = ByteBuffer.wrap(bitmapdata);
        bmp.copyPixelsFromBuffer(buffer);
        return bmp;
    }
    private float [][] bitmap2Array(Bitmap src){
        int rows = src.getHeight();
        int cols = src.getWidth();
        int nbytes = src.getByteCount();
        byte [] buff = new byte[nbytes];
        src.copyPixelsToBuffer(ByteBuffer.wrap(buff));
        float [][] res = new float[rows][cols];
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < cols; j++){
                int tmp = ((int)buff[i*cols+j])&(0xFF);
                res[i][j] = ((float)tmp) / 255.0f;
            }
        }
        return res;
    }
    private float[][] transformFitSize(float [][]src, int rows, int cols, float fillValue){
        Bitmap bmp = array2Bitmap(src);
        Bitmap rsz = fitSizeBitmap(bmp, rows, cols);
        float [][] tmp = bitmap2Array(rsz);
        if((tmp.length == rows) && (tmp[0].length == cols))
            return tmp;
        float [][] res = new float[rows][cols];
        for(int i = 0; i < rows; i++){
            if(i >= tmp.length){
                for(int j = 0; j < cols; j++){
                    res[i][j] = fillValue;
                }
                continue;
            }
            for(int j = 0; j < cols; j++){
                if(j >= tmp[i].length){
                    res[i][j] = fillValue;
                }else{
                    res[i][j] = tmp[i][j];
                }
            }
        }
        return res;
    }

    private Bitmap fitSizeBitmap(Bitmap src, int rows, int cols){
        float fY = ((float)rows) / src.getHeight();
        float fX = ((float)cols) / src.getWidth();
        float f = Math.min(fX, fY);
        return Bitmap.createScaledBitmap(src, (int)(src.getWidth()*f), (int)(src.getHeight()*f), true);
    }
    private float [][] transformGrayscale(Bitmap src){
        int rows = src.getHeight();
        int cols = src.getWidth();
        int []tmp = new int[rows * cols];
        src.getPixels(tmp, 0, cols, 0, 0, cols, rows);
        float [][] res = new float[rows][cols];
        for(int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                //int t = src.getPixel(j, i);
                int t = tmp[i * cols + j];
                float f = ((float) (t & 0xFF)) + ((float) ((t >> 8) & 0xFF)) + ((float) ((t >> 16) & 0xFF));
                res[i][j] = f / 255.0f / 3.0f;
            }
        }
        return res;
    }

    private void transformCutoff(float [][]arr, float cutoff, float valueBelow, float valueAbove){
        //1.0*(img < transform.get('cutoff', 0.5)*np.mean(img))
        float sum_x = 0.0f;
        for(int i = 0; i < arr.length; i++) {
            for (int j = 0; j < arr[i].length; j++) {
                sum_x += arr[i][j];
            }
        }
        float mean = sum_x / (arr.length * arr[0].length);
        float val = cutoff * mean;
        for(int i = 0; i < arr.length; i++){
            for(int j = 0; j < arr[i].length; j++){
                if(arr[i][j] < val)
                    arr[i][j] = valueBelow;
                else
                    arr[i][j] = valueAbove;
            }
        }
    }
    private int findFirstNonzero(boolean []arr){
        for(int i = 0; i < arr.length; i++){
            if(arr[i])
                return i;
        }
        return arr.length;
    }
    private int findLastNonzero(boolean []arr){
        for(int i = arr.length-1; i >= 0; i--){
            if(arr[i])
                return i+1;
        }
        return 0;
    }
    private float [][] transformTrim(float [][]arr, float cutoff) throws Exception{
        int rows = arr.length;
        int cols = arr[0].length;
        boolean [] col_any = new boolean[cols];
        boolean [] row_any = new boolean[rows];
        for(int i = 0; i < cols; i++){
            col_any[i] = false;
        }
        for(int i = 0; i < rows; i++){
            row_any[i] = false;
        }
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < cols; j++){
                if(arr[i][j] > cutoff){
                    col_any[j] = true;
                    row_any[i] = true;
                }
            }
        }

        int col_id1 = findFirstNonzero(col_any);
        int col_id2 = findLastNonzero(col_any);
        int row_id1 = findFirstNonzero(row_any);
        int row_id2 = findLastNonzero(row_any);

        if((col_id1 >= col_id2)||(row_id1 >= row_id2)){
            throw new Exception("trimArray: image is empty!");
        }
        float [][] res = new float[row_id2-row_id1][col_id2-col_id1];
        for(int i = 0; i < row_id2-row_id1; i++) {
            for (int j = 0; j < col_id2-col_id1; j++) {
                res[i][j] = arr[row_id1+i][col_id1+j];
            }
        }
        return res;
    }
    private void transformMeanStd(float [][]arr, float coef){
        int rows = arr.length;
        int cols = arr[0].length;

        float sum_x = 0.0f, sum_x2 = 0.0f;
        for(int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                sum_x += arr[i][j];
                sum_x2 += arr[i][j] * arr[i][j];
            }
        }
        int num = (rows * cols);
        float mean = sum_x / num;
        float std = (float)Math.sqrt(sum_x2 / num - mean*mean);
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < cols; j++){
                arr[i][j] = coef * (arr[i][j]-mean) / std;
            }
        }
    }

    private float [][] transformTranspose(float [][]arr){
        int rows = arr.length;
        int cols = arr[0].length;
        float [][] res = new float[cols][rows];
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < cols; j++){
                res[j][i] = arr[i][j];
            }
        }
        return res;
    }
    private String readAssetChars(String fname) throws RuntimeException {
        String res = null;
        try {
            InputStreamReader is = new InputStreamReader(getAssets().open(fname), StandardCharsets.UTF_16);
            BufferedReader reader = new BufferedReader(is);
            res = reader.readLine();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return res;
    }

}
