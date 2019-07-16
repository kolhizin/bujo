package com.kolhizin.bujotflite;

import android.content.Intent;
import android.net.Uri;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    Button btnLoad = null;
    ImageView imgView = null;
    TextView txtStatus = null;

    private static final int RC_SELECT_PICTURE = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        btnLoad = findViewById(R.id.btnLoad);
        imgView = findViewById(R.id.imageView);
        txtStatus = findViewById(R.id.txtStatus);

        btnLoad.setOnClickListener(this);
    }

    @Override
    public void onClick(View v){
        switch(v.getId())
        {
            case R.id.btnLoad: loadPicture(); return;
        }
    }

    protected void loadPicture(){
        Intent intent = new Intent(Intent.ACTION_PICK,
                android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        startActivityForResult(intent, RC_SELECT_PICTURE);
        txtStatus.setText("Wait");
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
        if(targetUri != null){
            txtStatus.setText(targetUri.toString());
        }
    }
}
