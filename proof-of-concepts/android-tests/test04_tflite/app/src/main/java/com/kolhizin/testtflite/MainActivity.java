package com.kolhizin.testtflite;

import android.content.res.AssetFileDescriptor;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.reflect.Array;
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


        try {
            AssetFileDescriptor fileDescriptor = this.getAssets().openFd("model.tflite");
            classifier_ = new Classifier(this, "model.tflite");
        }catch (IOException e){
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG);
        }

        float [][] input = readAssetCSV("htr_test.csv");
        float [][] output = null;
        try {
            output = classifier_.detect(input);
        }catch (Exception e){
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG);
        }
        if(output != null) {
            int a = output.length;
            int b = output[0].length;
            int c = output[output.length - 1].length;
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
