package com.kolhizin.detectbujo;

import android.graphics.Bitmap;
import android.support.v4.app.FragmentActivity;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.ImageView;
import android.widget.TextView;

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

        Bitmap wordImage = BuJoTools.extractCurve(app.getPage().getAligned(),
                word.xCoords, word.yCoords, -word.negOffset*yScale, word.posOffset*yScale)

        imgSource.setImageBitmap(wordImage);
        imgDetect.setImageBitmap(wordImage);

        String detectResult = classifier.detect(wordImage, cutoff);
        txtDescription.setText(detectResult);
    }
}
