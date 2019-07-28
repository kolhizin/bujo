package com.kolhizin.testtflite;

import android.content.res.AssetFileDescriptor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.reflect.Array;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.util.LinkedList;
import java.util.List;
import com.kolhizin.testtflite.Classifier;

public class MainActivity extends AppCompatActivity {
    private Classifier classifier_ = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Bitmap input_ = null;
        try {
            input_ = BitmapFactory.decodeStream(getAssets().open("htr_test.jpg"));
            classifier_ = new Classifier(this, "model.tflite");
        }catch (IOException e){
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG);
        }

        try {
            //Bitmap bmp = fitSizeBitmap(input_, 32, 128);
            float [][] in_tmp1 = transformBitmap(input_);
            transformCutoff(in_tmp1, 0.7f);
            float [][] in_tmp2 = transformTrimY(transformTrimX(in_tmp1));
            transformMeanStd(in_tmp2);

            float [][] output = null;

            //output = classifier_.detect(input);
        }catch (Exception e){
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
        }
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
        float [][] res = new float[rows][cols];
        for(int i = 0; i < rows; i++){
            for(int j = 0; j < cols; j++){
                res[i][j] = ((float)src.getPixel(j, i)) / 255.0f;
            }
        }
        return res;
    }
    private float[][] fitSizeArray(float [][]src, int rows, int cols, float fillValue){
        
    }

    private Bitmap fitSizeBitmap(Bitmap src, int rows, int cols){
        float fY = ((float)rows) / src.getHeight();
        float fX = ((float)cols) / src.getWidth();
        float f = Math.min(fX, fY);
        return Bitmap.createScaledBitmap(src, (int)(src.getWidth()*f), (int)(src.getHeight()*f), true);
    }
    private float [][] transformBitmap(Bitmap src){
        int rows = src.getHeight();
        int cols = src.getWidth();
        float [][] res = new float[rows][cols];
        for(int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int t = src.getPixel(j, i);
                float f = ((float) (t & 0xFF)) + ((float) ((t >> 8) & 0xFF)) + ((float) ((t >> 16) & 0xFF));
                res[i][j] = f / 255.0f / 3.0f;
            }
        }
        return res;
    }
    private void transformCutoff(float [][]arr, float cutoff){
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
                    arr[i][j] = 1.0f;
                else
                    arr[i][j] = 0.0f;
            }
        }
    }
    private float [][] transformTrimX(float [][] arr){
        int rows = arr.length;
        int cols = arr[0].length;
        boolean [] tmax = new boolean[cols];
        for(int j = 0; j < cols; j++){
            tmax[j] = false;
            for(int i = 0; i < rows; i++){
                if(arr[i][j] > 1e-5f) {
                    tmax[j] = true;
                    break;
                }
            }
        }
        int id1 = 0, id2 = cols-1;
        while(id1 < cols){
            if(tmax[id1])
                break;
            id1++;
        }
        while(id2 > 0){
            if(tmax[id2])
                break;
            id2--;
        }
        id2++;
        float [][] res = new float[rows][id2-id1];
        for(int i = 0; i < rows; i++) {
            for (int j = 0; j < id2 - id1; j++) {
                res[i][j] = arr[i][id1+j];
            }
        }
        return res;
    }
    private float [][] transformTrimY(float [][] arr){
        int rows = arr.length;
        int cols = arr[0].length;
        boolean [] tmax = new boolean[rows];
        for(int i = 0; i < rows; i++){
            tmax[i] = false;
            for(int j = 0; j < cols; j++){
                if(arr[i][j] > 1e-5f) {
                    tmax[i] = true;
                    break;
                }
            }
        }
        int id1 = 0, id2 = rows-1;
        while(id1 < rows){
            if(tmax[id1])
                break;
            id1++;
        }
        while(id2 > 0){
            if(tmax[id2])
                break;
            id2--;
        }
        id2++;
        float [][] res = new float[id2-id1][cols];
        for(int i = 0; i < id2-id1; i++) {
            for (int j = 0; j < cols; j++) {
                res[i][j] = arr[i+id1][j];
            }
        }
        return res;
    }

    private void transformMeanStd(float [][]arr){
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
                arr[i][j] = (arr[i][j]-mean) / std;
            }
        }
    }

    private float[][] readAssetCSV(String fname) throws RuntimeException {
        String line = "";
        LinkedList<float[]> rawResult = new LinkedList<float[]>();
        int prevSize = -1;
        try {
            InputStreamReader is = new InputStreamReader(getAssets().open(fname));
            BufferedReader reader = new BufferedReader(is);

            while ((line = reader.readLine()) != null) {
                // Split the line into different tokens (using the comma as a separator).
                String[] tokens = line.split(",");
                if (prevSize >= 0 && (tokens.length != prevSize))
                    throw new RuntimeException("Different number of numbers in lines!");
                prevSize = tokens.length;
                float [] res = new float[tokens.length];
                for(int i = 0; i < tokens.length; i++)
                    res[i] = Float.parseFloat(tokens[i]);
                rawResult.add(res);
            }
        } catch (IOException e) {
            Log.e("MainActivity", "Error" + line, e);
            e.printStackTrace();
        }
        float [][] res = new float[rawResult.size()][];
        int i = 0;
        for (float [] v : rawResult)
            res[i++] = v;
        return res;
    }

}
