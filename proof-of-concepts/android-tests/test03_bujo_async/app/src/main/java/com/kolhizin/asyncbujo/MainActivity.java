package com.kolhizin.asyncbujo;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Path;
import android.net.Uri;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;


public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    TextView txtStatus = null;
    ImageView imgView = null;
    Button btnLoad = null;
    Button btnInspect = null;

    class AsyncDetect extends AsyncDetectionTask{
        AsyncDetect(Context context, BuJoSettings settings, int maxDim){
            super(context, settings, maxDim);
        }
        @Override
        protected void onProgressUpdate(BuJoPage ...values){
            super.onProgressUpdate(values);
            txtStatus.setText(values[0].getStatusMessage());
            if(values[0].getStatus().fErrors)
                txtStatus.setText(values[0].getErrorMessage());
            if(values[0].getStatus().fDetectedWords){
                Bitmap img0 = BuJoTools.shadeRegions(values[0].getAligned(), values[0].getSplits());
                Bitmap img = BuJoTools.drawWords(img0, values[0].getWords());
                imgView.setImageBitmap(img);
            }else if(values[0].getStatus().fDetectedLines){
                Bitmap img0 = BuJoTools.shadeRegions(values[0].getAligned(), values[0].getSplits());
                Bitmap img = BuJoTools.drawLines(img0, values[0].getLines());
                imgView.setImageBitmap(img);
            }else if(values[0].getStatus().fDetectedRegion){
                Bitmap img = BuJoTools.shadeRegions(values[0].getAligned(), values[0].getSplits());
                imgView.setImageBitmap(img);
            }else if(values[0].getAligned() != null) {
                imgView.setImageBitmap(values[0].getAligned());
            }else if(values[0].getOriginal() != null) {
                imgView.setImageBitmap(values[0].getOriginal());
            }
        }

        @Override
        protected void onPostExecute(BuJoPage page) {
            super.onPostExecute(page);
            BuJoApp app = (BuJoApp)getApplication();
            app.setPage(page);
            if(page.getStatus().fSuccess){
                btnInspect.setVisibility(View.VISIBLE);
                btnInspect.setClickable(true);
            }
        }
    }

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        txtStatus = findViewById(R.id.txtStatus);
        btnLoad = findViewById(R.id.btnLoad);
        btnInspect = findViewById(R.id.btnInspect);
        imgView = findViewById(R.id.imageView);

        btnLoad.setOnClickListener(this);
        btnInspect.setOnClickListener(this);
    }

    @Override
    public void onClick(View v){
        switch(v.getId())
        {
            case R.id.btnLoad: loadPicture(); return;
            case R.id.btnInspect: inspectPage(); return;
        }
    }

    protected void loadPicture()
    {
        Intent intent = new Intent(Intent.ACTION_PICK,
                android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        startActivityForResult(intent, 0);
        txtStatus.setText("Wait");
        btnInspect.setVisibility(View.INVISIBLE);
        btnInspect.setClickable(false);
    }
    protected void inspectPage()
    {
        Intent intent = new Intent(this, InspectActivity.class);
        startActivity(intent);
    }




    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // TODO Auto-generated method stub
        super.onActivityResult(requestCode, resultCode, data);

        if (resultCode == RESULT_OK){
            Uri targetUri = data.getData();
            BuJoSettings settings = new BuJoSettings();
            AsyncDetect task = new AsyncDetect(this, settings, imgView.getMaxHeight());
            task.execute(targetUri);
        }
    }

}
