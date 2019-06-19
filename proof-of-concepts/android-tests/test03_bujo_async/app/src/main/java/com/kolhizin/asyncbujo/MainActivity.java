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


public class MainActivity extends AppCompatActivity {

    public static Bitmap shadeBitmap(Bitmap src, float angle, float offset){
        Bitmap res = src.copy(src.getConfig(), true);
        Canvas canvas = new Canvas(res);
        Paint paint = new Paint();
        paint.setColor(Color.rgb(255,0, 0));
        paint.setAlpha(110);
        paint.setStyle(Paint.Style.FILL);
        paint.setStrokeWidth(10);

        // draw text to the Canvas center
        int w = src.getWidth();
        int h = src.getHeight();
        int x = (src.getWidth())/2;
        int y = (src.getHeight())/2;

        Path p1 = new Path();
        p1.moveTo(0, 0);
        p1.lineTo(w, 0);
        p1.lineTo(w, h);
        p1.lineTo(0, 0);
        p1.close();

        Path p2 = new Path();
        p2.moveTo(0, 0);
        p2.lineTo(w/2, 0);
        p2.lineTo(w/2, h);
        p2.lineTo(0, 0);
        p2.close();

        p1.op(p2, Path.Op.UNION);
        canvas.drawPath(p1, paint);
        //canvas.drawPath(p2, paint);

        //canvas.drawCircle(x, y, 100, paint);
        //canvas.drawLine(-100, 0, w, h, paint);
        //float [] verts = {0.0f, 0.0f, w, h, w, 0.0f};
        //canvas.drawVertices(Canvas.VertexMode.TRIANGLES, 6, verts,0,
        //        null, 0, null, 0, null, 0,0,
        //        paint);
        return res;
    }

    class AsyncDetect extends AsyncDetectionTask{
        AsyncDetect(Context context, BuJoSettings settings, int maxDim){
            super(context, settings, maxDim);
        }
        @Override
        protected void onProgressUpdate(BuJoPage ...values){
            super.onProgressUpdate(values);
            txtStatus.setText(values[0].getStatusMessage());
            if(values[0].getOriginal() != null) {
                //raw: +1.4851326, +56.977455, -0.25100136, +1
                //raw: -0.6284956, -79.567635, -0.25100327, -1

                //scl: +1.4851326, +0.05704584, -2.513e-4, +1
                //scl: -0.6284956, -0.07966313, -2.513e-4, -1
                int w = values[0].getOriginal().getWidth();
                int h = values[0].getOriginal().getHeight();
                float diagsize = (float)Math.sqrt(w*w+h*h);
                float angle = 1.4851326f;
                float offset = 0.05704584f * diagsize;

                Bitmap src = values[0].getOriginal();
                txtStatus.setText("Test draw");
                imgView.setLayerType(View.LAYER_TYPE_SOFTWARE, null);
                imgView.setImageBitmap(shadeBitmap(src, angle, offset));
            }
            if(values[0].getStatus().fDetectedRegion){
                String res = "";
                for(int i = 0; i < values[0].numSplits(); i++){
                    BuJoPage.BuJoSplit split = values[0].getSplit(i);
                    res += "(ang=" + String.valueOf(split.angle);
                    res += ", off=" + String.valueOf(split.offset);
                    res += ", mrg=" + String.valueOf(split.margin);
                    res += ", dir=" + String.valueOf(split.direction) + ")";
                }
                txtStatus.setText(res);
            }
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
            BuJoSettings settings = new BuJoSettings();
            AsyncDetect task = new AsyncDetect(this, settings, imgView.getMaxHeight());
            task.execute(targetUri);
        }
    }

}
