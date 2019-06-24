package com.kolhizin.asyncbujo;

import android.content.Intent;
import android.graphics.Bitmap;
import android.media.Image;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

public class InspectActivity extends AppCompatActivity implements View.OnClickListener {
    private TextView newTextView(String text, int topPadding){
        TextView res = new TextView(this);
        res.setText(text);
        res.setTextSize(16);
        res.setPaddingRelative(0, topPadding,0,0);
        res.setLayoutParams(new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,LinearLayout.LayoutParams.WRAP_CONTENT));
        return res;
    }
    private ImageView newImageView(Bitmap bitmap){
        ImageView res = new ImageView(this);
        res.setImageBitmap(bitmap);
        res.setAdjustViewBounds(true);
        res.setLayoutParams(new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,LinearLayout.LayoutParams.WRAP_CONTENT));
        return res;
    }

    private BuJoPage page = null;
    private ImageView inspectImgView = null;
    private TextView inspectTxtView = null;
    private LinearLayout mainLayout = null;
    private EditText lineText = null;
    private Button btnInspect = null;
    private int fixedSize = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_inspect);

        Intent intent = getIntent();
        BuJoApp app = (BuJoApp)getApplication();
        page = app.getPage();

        mainLayout = findViewById(R.id.inspectViewLayout);
        inspectImgView = findViewById(R.id.inspectMainView);
        inspectImgView.setImageBitmap(page.getAligned());

        lineText = findViewById(R.id.lineText);
        btnInspect = findViewById(R.id.btnInspectLine);
        inspectTxtView = findViewById(R.id.inspectMainText);
        inspectTxtView.setText("Number of lines: "+String.valueOf(page.numLines()));

        fixedSize = mainLayout.getChildCount();
        btnInspect.setOnClickListener(this);
    }

    private void loadLine(int i){
        float yScale = 1.4f;
        BuJoPage.BuJoLine line = page.getLine(i);
        BuJoWord []words = page.getWords()[i];
        float negOffset = 0.02f, posOffset = 0.02f;
        if(words.length > 0){
            negOffset = -words[0].negOffset * yScale;
            posOffset = words[0].posOffset * yScale;
        }
        Bitmap tmpLine = BuJoTools.extractCurve(page.getAligned(), line.xCoords, line.yCoords, negOffset, posOffset);

        String lineText = "Line #" + String.valueOf(i);
        lineText += ", " + String.valueOf(page.getWords()[i].length) + " words:";
        mainLayout.addView(newTextView(lineText, 20));
        mainLayout.addView(newImageView(tmpLine));
        for(int j = 0; j < words.length; j++){
            String wordText = "Word #" + String.valueOf(j);
            float dx = words[j].xCoords[words[j].xCoords.length-1] - words[j].xCoords[0];
            if(dx * page.getAligned().getWidth() < 5.0f){
                mainLayout.addView(newTextView(wordText + ", not-a-word", 20));
                continue;
            }
            Bitmap tmpWord = BuJoTools.extractCurve(page.getAligned(),
                    words[j].xCoords, words[j].yCoords,
                    -words[j].negOffset*yScale, words[j].posOffset*yScale);

            float stddev = BuJoTools.calcImageStdDev(tmpWord);
            mainLayout.addView(newTextView(wordText + ", stddev=" + String.valueOf(stddev), 20));
            mainLayout.addView(newImageView(tmpWord));
        }
    }

    private void inspectLine()
    {
        String text = lineText.getText().toString();
        int id = Integer.valueOf(text);
        if(id >= page.numLines())
            return;
        mainLayout.removeViews(fixedSize, mainLayout.getChildCount() - fixedSize);
        loadLine(id);
    }
    @Override
    public void onClick(View v){
        switch(v.getId())
        {
            case R.id.btnInspectLine: inspectLine(); return;
        }
    }
}
