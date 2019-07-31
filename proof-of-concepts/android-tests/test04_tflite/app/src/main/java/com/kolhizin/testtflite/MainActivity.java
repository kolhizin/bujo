package com.kolhizin.testtflite;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.res.AssetFileDescriptor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
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

public class MainActivity extends AppCompatActivity {
    private Classifier classifier_ = null;
    private TextView textView = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        textView = (TextView)findViewById(R.id.mainText);

        Bitmap input_ = null;
        String chars_ = null;
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