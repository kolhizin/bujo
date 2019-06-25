package com.kolhizin.asyncbujo;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Path;
import android.net.Uri;
import android.os.Environment;
import android.provider.MediaStore;
import android.support.v4.content.FileProvider;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Calendar;


public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    TextView txtStatus = null;
    ImageView imgView = null;
    Button btnLoad = null;
    Button btnPhoto = null;
    Button btnInspect = null;
    Uri cameraImgUri = null;

    private static final int RC_SELECT_PICTURE = 1;
    private static final int RC_TAKE_PHOTO = 2;

    private static final int PERCODE_CAMERA = 1;
    private static final int PERCODE_WRITE_EXTSTORAGE = 2;

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
    protected void onSaveInstanceState(Bundle outState) {
        //if (cameraImgUri != null)
        //    outState.putString("camera_uri", cameraImgUri.toString());
        super.onSaveInstanceState(outState);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (savedInstanceState != null) {
            //if (cameraImgUri == null && savedInstanceState.getString("camera_uri") != null) {
            //    cameraImgUri = Uri.parse(savedInstanceState.getString("camera_uri"));
            //}
        }

        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        txtStatus = findViewById(R.id.txtStatus);
        btnLoad = findViewById(R.id.btnLoad);
        btnInspect = findViewById(R.id.btnInspect);
        btnPhoto = findViewById(R.id.btnTakePhoto);
        imgView = findViewById(R.id.imageView);

        btnLoad.setOnClickListener(this);
        btnInspect.setOnClickListener(this);
        btnPhoto.setOnClickListener(this);
    }

    @Override
    public void onClick(View v){
        switch(v.getId())
        {
            case R.id.btnLoad: loadPicture(); return;
            case R.id.btnInspect: inspectPage(); return;
            case R.id.btnTakePhoto: takePhoto(); return;
        }
    }

    protected void takePhoto(){
        if(!getPackageManager().hasSystemFeature(getPackageManager().FEATURE_CAMERA)){
            Toast.makeText(this, "you have no camera :(", Toast.LENGTH_SHORT);
            return;
        }
        if (checkSelfPermission(Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.CAMERA}, PERCODE_CAMERA);
        }else if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, PERCODE_WRITE_EXTSTORAGE);
        }else{
            File mainDirectory = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES), "BuJo/tmp");
            if (!mainDirectory.exists())
                mainDirectory.mkdirs();

            Calendar calendar = Calendar.getInstance();
            //File tmpFile = new File(mainDirectory, "IMG_" + calendar.getTimeInMillis() + ".jpeg");
            File tmpFile = null;
            try {
                tmpFile = File.createTempFile(
                        "IMG_"+calendar.getTimeInMillis(),
                        ".jpeg",
                        mainDirectory
                );
            }catch (IOException e){
                Toast.makeText(this, "Exception", Toast.LENGTH_LONG);
            }
            cameraImgUri = FileProvider.getUriForFile(this,
                    BuildConfig.APPLICATION_ID + ".provider", tmpFile);
            Intent cameraIntent = new Intent(android.provider.MediaStore.ACTION_IMAGE_CAPTURE);
            cameraIntent.putExtra(MediaStore.EXTRA_OUTPUT, cameraImgUri);
            cameraIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            try {
                startActivityForResult(cameraIntent, RC_TAKE_PHOTO);
            }catch (Exception e){
                Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
            }
        }
    }

    protected void loadPicture(){
        Intent intent = new Intent(Intent.ACTION_PICK,
                android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        startActivityForResult(intent, RC_SELECT_PICTURE);
        txtStatus.setText("Wait");
        btnInspect.setVisibility(View.INVISIBLE);
        btnInspect.setClickable(false);
    }
    protected void inspectPage(){
        Intent intent = new Intent(this, InspectActivity.class);
        startActivity(intent);
    }




    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // TODO Auto-generated method stub
        super.onActivityResult(requestCode, resultCode, data);

        Uri targetUri = null;
        if (requestCode == RC_SELECT_PICTURE){
            if (resultCode == RESULT_OK){
                targetUri = data.getData();
            }
        }
        if(requestCode == RC_TAKE_PHOTO){
            if(resultCode == RESULT_OK){
                targetUri = cameraImgUri;
            }
        }
        if(targetUri != null){
            BuJoSettings settings = new BuJoSettings();
            AsyncDetect task = new AsyncDetect(this, settings, imgView.getMaxHeight());
            task.execute(targetUri);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERCODE_CAMERA) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(this, "camera permission granted", Toast.LENGTH_LONG).show();
            }else{
                Toast.makeText(this, "camera permission denied", Toast.LENGTH_LONG).show();
            }
        }else if (requestCode == PERCODE_WRITE_EXTSTORAGE) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(this, "storage write permission granted", Toast.LENGTH_LONG).show();
            }else{
                Toast.makeText(this, "storage write permission denied", Toast.LENGTH_LONG).show();
            }
        }
    }

}
