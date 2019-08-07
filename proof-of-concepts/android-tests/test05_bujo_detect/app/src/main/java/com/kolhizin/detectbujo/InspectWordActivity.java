package com.kolhizin.detectbujo;

import android.graphics.Bitmap;
import android.support.v4.app.FragmentActivity;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

public class InspectWordActivity extends FragmentActivity {
    private ImageView imgSource = null, imgDetect = null;
    private TextView txtDescription = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_inspect_word);

        imgSource = findViewById(R.id.imgWordSource);
        imgDetect = findViewById(R.id.imgWordDetect);
        txtDescription = findViewById(R.id.textWordDetection);

        BuJoApp app = (BuJoApp)getApplication();
        BuJoWord word = app.getWord();
        Classifier classifier = app.getClassifier();
        float yScale = 1.4f;
        float cutoff = 0.7f;
        int topK = 3;

        Bitmap wordImage = BuJoTools.extractCurve(app.getPage().getAligned(),
                word.xCoords, word.yCoords, -word.negOffset*yScale, word.posOffset*yScale);

        imgSource.setImageBitmap(wordImage);
        try {
            imgDetect.setImageBitmap(classifier.preprocess(wordImage, cutoff));
            word.text = classifier.detect(wordImage, cutoff);
            Classifier.Result [] tmp = classifier.detect(wordImage, cutoff, topK);
            word.probs = new float[tmp.length][];
            word.chars = new char[tmp.length][];
            for(int j = 0; j < tmp.length; j++){
                word.probs[j] = tmp[j].probs;
                word.chars[j] = tmp[j].chars;
            }
        }catch (Exception e){
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
        }

        String txtResult = "Results:\n";
        if(word.text != null){
            txtResult += word.text + "\n";
        }
        if(word.chars != null && word.probs != null){
            for(int i = 0; i < word.chars.length; i++){
                for(int j = 0; j < word.chars[i].length; j++){
                    if(j > 0)
                        txtResult += "\t";
                    if(word.chars[i][j] == 0)
                        txtResult += "END:" + String.format("%.02f", word.probs[i][j]);
                    else
                        txtResult += "'" + word.chars[i][j] + "':" + String.format("%.02f", word.probs[i][j]);
                }
                txtResult += "\n\n";
            }
        }
        txtDescription.setText(txtResult);
    }
}
