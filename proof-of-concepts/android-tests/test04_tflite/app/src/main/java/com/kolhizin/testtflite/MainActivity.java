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

public class MainActivity extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        float [][] input = readTestCSV();
    }

    private float[][] readTestCSV() throws RuntimeException {
        String line = "";
        LinkedList<float[]> rawResult = new LinkedList<float[]>();
        int prevSize = -1;
        try {
            InputStreamReader is = new InputStreamReader(getAssets().open("htr_test.csv"));
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
