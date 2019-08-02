package com.kolhizin.detectbujo;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
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

import java.io.File;
import java.io.IOException;
import java.util.Calendar;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private static final int RC_SELECT_PICTURE = 1;
    private static final int RC_TAKE_PHOTO = 2;

    private static final int PERCODE_CAMERA = 1;
    private static final int PERCODE_WRITE_EXTSTORAGE = 2;

    Button btnLoad = null, btnPhoto = null;
    ImageView imgMain = null;
    Uri cameraImgUri = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        imgMain = (ImageView)findViewById(R.id.imageView);
        btnLoad = (Button)findViewById(R.id.btnLoad);
        btnPhoto = (Button)findViewById(R.id.btnPhoto);

        btnLoad.setOnClickListener(this);
        btnPhoto.setOnClickListener(this);
    }

    @Override
    public void onClick(View v){
        switch(v.getId())
        {
            case R.id.btnLoad: loadPicture(); return;
            case R.id.btnPhoto: takePhoto(); return;
        }
    }

    protected void takePhoto(){
        if(!getPackageManager().hasSystemFeature(getPackageManager().FEATURE_CAMERA)){
            Toast.makeText(this, "you have no camera :(", Toast.LENGTH_SHORT).show();
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
            //BuJoSettings settings = new BuJoSettings();
            //AsyncDetect task = new AsyncDetect(this, settings, imgView.getMaxHeight());
            //task.execute(targetUri);

            Bitmap bmp = null;
            try{
                ImageUtils.loadImage(this, targetUri, 4096);
            }catch (Exception e){
                Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
            }
            if(bmp != null)
                imgMain.setImageBitmap(bmp);
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
