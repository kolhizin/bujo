package com.kolhizin.detectbujo;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.os.AsyncTask;
import android.support.v4.app.FragmentActivity;
import android.os.Bundle;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

public class InspectLineActivity extends FragmentActivity {
    public class AsyncDetectWordsTask extends AsyncTask<Integer, BuJoPage, BuJoPage> {
        private Detector detector_;
        private Classifier classifier_;
        private Context context_;

        public AsyncDetectWordsTask(Context context, Detector detector, Classifier classifier) {
            super();
            context_ = context;
            detector_ = detector;
            classifier_ = classifier;
        }

        @Override
        protected BuJoPage doInBackground(Integer ... params) {
            try {
                detector_.detectWords(lineId);
                wordImages = new Bitmap[detector_.getPage().getWords()[lineId].length];
                BuJoWord [] words = detector_.getPage().getWords()[lineId];
                for(int i = 0; i < words.length; i++) {
                    wordImages[i] = extractWord(i, 1.4f);
                }
                publishProgress(detector_.getPage());
                int topK = 2;
                for(int i = 0; i < words.length; i++){
                    if(wordImages[i] != null) {
                        words[i].text = classifier_.detect(wordImages[i], 0.7f);

                        Classifier.Result [] tmp = classifier_.detect(wordImages[i], 0.7f, topK);
                        words[i].probs = new float[tmp.length][];
                        words[i].chars = new char[tmp.length][];
                        for(int j = 0; j < tmp.length; j++){
                            words[i].probs[j] = tmp[j].probs;
                            words[i].chars[j] = tmp[j].chars;
                        }
                    }
                    publishProgress(detector_.getPage());
                }
            } catch (Exception e) {
                e.printStackTrace();
                detector_.getPage().setError("Exception: " + e.getMessage());
            }
            return detector_.getPage();
        }

        @Override
        protected void onProgressUpdate(BuJoPage... values) {
            super.onProgressUpdate(values);
            if(page.getWords()[lineId] == null)
                return;
            if(wordTextViews == null || wordTextViews.length != page.getWords()[lineId].length){
                wordTextViews = new TextView[page.getWords()[lineId].length];
                for(int i = 0; i < page.getWords()[lineId].length; i++){
                    loadWord(i);
                }
            }
            BuJoWord []words = page.getWords()[lineId];
            for(int i = 0; i < wordTextViews.length; i++){
                wordTextViews[i].setText(makeText(i, words[i], wordImages[i]));
            }
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
    private TextView [] wordTextViews = null;
    private Bitmap [] wordImages = null;
    private int lineId = -1;

    private String makeText(int id, BuJoWord word, Bitmap bmp){
        String wordText = "Word #" + String.valueOf(id);
        if(bmp != null) {
            float stddev = BuJoTools.calcImageStdDev(bmp);
            wordText += ", stddev=" + String.valueOf(stddev);
        }
        if(word.text != null){
            wordText += "\nResult=" + word.text;
        }
        if(word.chars != null && word.probs != null){
            wordText += "\n";
            for(int i = 0; i < word.chars.length; i++){
                wordText += "[";
                for(int j = 0; j < word.chars[i].length; j++){
                    if(j > 0)
                        wordText += "|";
                    if(word.chars[i]==0)
                        wordText += "END:" + Float.toString(word.probs[i][j]);
                    else
                        wordText += "'" word.chars[i][j] + "':" + Float.toString(word.probs[i][j]);
                }
                wordText += "]";
            }
        }
        return wordText;
    }

    private void loadLine(float negOffset, float posOffset){
        BuJoPage.BuJoLine line = page.getLine(lineId);
        Bitmap bmpLine = BuJoTools.extractCurve(page.getAligned(), line.xCoords, line.yCoords, negOffset, posOffset);

        String text = "Width=" + Integer.toString(bmpLine.getWidth()) + ", Height=" + Integer.toString(bmpLine.getHeight());

        lineImageView.setImageBitmap(bmpLine);
        lineTextView.setText(text);
    }
    private Bitmap extractWord(int j, float yScale){
        BuJoWord word = page.getWords()[lineId][j];
        float dx = Math.abs(word.xCoords[word.xCoords.length-1] - word.xCoords[0]);
        if(dx * page.getAligned().getWidth() < 4.0f)
            return null;
        return BuJoTools.extractCurve(page.getAligned(), word.xCoords, word.yCoords,
                -word.negOffset*yScale, word.posOffset*yScale);
    }
    private void loadWord(int j){
        BuJoPage.BuJoLine line = page.getLine(lineId);
        BuJoWord []words = page.getWords()[lineId];

        wordTextViews[j] = newTextView(makeText(j, words[j], wordImages[j]), 20);
        wordsLayout.addView(wordTextViews[j]);
        wordsLayout.addView(newImageView(wordImages[j]));
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_inspect_line);

        Intent intent = getIntent();
        BuJoApp app = (BuJoApp)getApplication();
        page = app.getPage();
        lineId = page.getActiveLineId();

        wordsLayout = findViewById(R.id.inspectWordsLayout);
        lineTextView = findViewById(R.id.inspectLineTextView);
        lineImageView = findViewById(R.id.inspectLineImageView);

        loadLine(0.03f, 0.03f);

        BuJoSettings settings = new BuJoSettings();
        try {
            AsyncDetectWordsTask task = new AsyncDetectWordsTask(this, app.getDetector(), app.getClassifier());
            task.execute(page.getActiveLineId());
            Toast.makeText(this, "Started!", Toast.LENGTH_SHORT).show();
        }catch (Exception e){
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
        }

    }

}
