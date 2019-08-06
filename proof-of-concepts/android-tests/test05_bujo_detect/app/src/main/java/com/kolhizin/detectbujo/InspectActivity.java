package com.kolhizin.detectbujo;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.os.AsyncTask;
import android.support.v4.app.FragmentActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

public class InspectActivity extends FragmentActivity {
    public class AsyncDetectWordsTask extends AsyncTask<Integer, BuJoPage, BuJoPage> {
        private Detector detector_;
        private Context context_;

        public AsyncDetectWordsTask(Context context, Detector detector) {
            super();
            context_ = context;
            detector_ = detector;
        }

        @Override
        protected BuJoPage doInBackground(Integer ... params) {
            try {
                detector_.detectWords(params[0]);
                publishProgress(detector_.getPage());
            } catch (Exception e) {
                e.printStackTrace();
                detector_.getPage().setError("Exception: " + e.getMessage());
            }
            return detector_.getPage();
        }

        @Override
        protected void onProgressUpdate(BuJoPage... values) {
            super.onProgressUpdate(values);
            Toast.makeText(context_, values[0].getStatusMessage(), Toast.LENGTH_SHORT).show();
            if(values[0].getStatus().fErrors)
                Toast.makeText(context_, values[0].getErrorMessage(), Toast.LENGTH_LONG).show();
        }

        @Override
        protected void onPostExecute(BuJoPage page) {
            super.onPostExecute(page);
            Toast.makeText(context_, "Done!", Toast.LENGTH_LONG).show();
        }
    }

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
    private ImageView lineImageView = null;
    private TextView lineTextView = null;
    private LinearLayout wordsLayout = null;
    private int lineId = -1;

    private void loadLine(float negOffset, float posOffset){
        BuJoPage.BuJoLine line = page.getLine(lineId);
        Bitmap bmpLine = BuJoTools.extractCurve(page.getAligned(), line.xCoords, line.yCoords, negOffset, posOffset);

        String text = "Width=" + Integer.toString(bmpLine.getWidth()) + ", Height=" + Integer.toString(bmpLine.getHeight());

        lineImageView.setImageBitmap(bmpLine);
        lineTextView.setText(text);
        /*
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
        */
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_inspect);

        Intent intent = getIntent();
        BuJoApp app = (BuJoApp)getApplication();
        page = app.getPage();
        lineId = page.getActiveLineId();

        wordsLayout = findViewById(R.id.inspectWordsLayout);
        lineTextView = findViewById(R.id.inspectLineTextView);
        lineImageView = findViewById(R.id.inspectLineImageView);

        loadLine(0.03f, 0.03f);
        /*
        BuJoSettings settings = new BuJoSettings();
        try {
            AsyncDetectWordsTask task = new AsyncDetectWordsTask(this, app.getDetector());
            task.execute(page.getActiveLineId());
            Toast.makeText(this, "Started!", Toast.LENGTH_SHORT).show();
        }catch (Exception e){
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
        }
        */
    }

}
