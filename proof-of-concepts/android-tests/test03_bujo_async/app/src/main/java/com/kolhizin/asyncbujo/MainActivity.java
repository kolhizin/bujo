package com.kolhizin.asyncbujo;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
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


public class MainActivity extends AppCompatActivity {

    class AsyncDetect extends AsyncDetectionTask{
        AsyncDetect(Context context, int maxDim){
            super(context, maxDim);
        }
        @Override
        protected void onProgressUpdate(BuJoPage ...values){
            super.onProgressUpdate(values);
            txtStatus.setText(values[0].status);
            if(values[0].bitmap != null)
                imgView.setImageBitmap(values[0].bitmap);
        }
    }

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    TextView txtStatus = null;
    ImageView imgView = null;
    Button btnLoad = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        txtStatus = findViewById(R.id.txtStatus);
        btnLoad = findViewById(R.id.btnLoad);
        imgView = findViewById(R.id.imageView);

        btnLoad.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(Intent.ACTION_PICK,
                        android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
                startActivityForResult(intent, 0);
                txtStatus.setText("Wait");
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // TODO Auto-generated method stub
        super.onActivityResult(requestCode, resultCode, data);

        if (resultCode == RESULT_OK){
            Uri targetUri = data.getData();
            txtStatus.setText("Test");
            AsyncDetect task = new AsyncDetect(this, imgView.getMaxHeight());
            task.execute(targetUri);
        }
    }


/*
    private Bitmap loadImage(Uri imageUri)
    {
        Bitmap bitmap;
        try {
            // bitmap = BitmapFactory.decodeStream(getContentResolver().openInputStream(targetUri));
            Bitmap bitmap0 = scaleImage(this, imageUri, imgView.getMaxHeight());
            bitmap = Bitmap.createScaledBitmap(bitmap0, bitmap0.getWidth()/2, bitmap0.getHeight()/2, true);
            imgView.setImageBitmap(bitmap);

            int width = bitmap.getWidth();
            int height = bitmap.getHeight();

            int []pixels = new int[width * height]; // assume RGBA, do once only
            bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
            long start = System.nanoTime();
            int res = loadImageIntoDecoder(pixels, width, height);
            long end = System.nanoTime();
            double diff = (end - start) / 1000000000.0;
            String szString = String.valueOf(width) + "x" + String.valueOf(height);
            textView.setText(String.valueOf(res) + " in " + String.valueOf(diff) + " for " + szString);
        } catch (FileNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
*/
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}
