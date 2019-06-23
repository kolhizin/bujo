package com.kolhizin.asyncbujo;

import android.content.Intent;
import android.graphics.Bitmap;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

public class InspectActivity extends AppCompatActivity {
    private TextView newTextView(String text, int topPadding){
        TextView res = new TextView(this);
        res.setText(text);
        res.setTextSize(16);
        res.setPaddingRelative(0, topPadding,0,0);
        res.setLayoutParams(new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,LinearLayout.LayoutParams.WRAP_CONTENT));
        return res;
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_inspect);

        Intent intent = getIntent();
        BuJoApp app = (BuJoApp)getApplication();
        BuJoPage page = app.getPage();

        LinearLayout mainLayout = findViewById(R.id.inspectViewLayout);


        mainLayout.addView(newTextView("Number of lines: "+String.valueOf(page.numLines()), 200));

        ImageView imgView = new ImageView(this);
        imgView.setImageBitmap(page.getAligned());
        imgView.setLayoutParams(new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,LinearLayout.LayoutParams.MATCH_PARENT));
        mainLayout.addView(imgView);

        for(int i = 0; i < Math.min(5, page.numLines()); i++)
        {
            String text = "Line #" + String.valueOf(i) + ", " + String.valueOf(page.getWords()[i].length) + " words:";
            mainLayout.addView(newTextView(text, 20));

            BuJoPage.BuJoLine line = page.getLine(i);
            Bitmap tmp = BuJoTools.extractCurve(page.getAligned(), line.xCoords, line.yCoords, 0.02f, 0.02f);
            ImageView lineImgView = new ImageView(this);
            lineImgView.setImageBitmap(tmp);
            lineImgView.setLayoutParams(new LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.MATCH_PARENT,LinearLayout.LayoutParams.MATCH_PARENT));
            mainLayout.addView(lineImgView);
        }
    }
}
