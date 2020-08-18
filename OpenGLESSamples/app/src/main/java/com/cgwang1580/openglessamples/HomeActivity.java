package com.cgwang1580.openglessamples;

import android.Manifest;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Spinner;
import android.widget.Toast;
import com.cgwang1580.permission.PermissionHelper;
import com.cgwang1580.permission.PermissionInterface;
import com.cgwang1580.utils.CommonDefine;
import com.cgwang1580.utils.MyLog;

import androidx.appcompat.app.AppCompatActivity;

public class HomeActivity extends AppCompatActivity {

    private final String TAG  = this.getClass().getName();
    private int mEffectType = 0;

    private final static String[]PermissionList = new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE};

    //todo: 从shared_preferenced里获取权限情况
    private boolean bPermissionOK = false;

    @Override
    protected void onCreate (Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);
        init ();
        requestPermission ();
    }

    public void requestPermission () {
        PermissionHelper.MyRequestPermission(this, PermissionList, new PermissionInterface() {
            @Override
            public int doPermissionSucceed() {
                bPermissionOK = true;
                Toast.makeText(HomeActivity.this, "onCreate doPermissionSucceed", Toast.LENGTH_SHORT).show();
                return 0;
            }

            @Override
            public int doPermissionFailed() {
                Toast.makeText(HomeActivity.this, "onCreate doPermissionFailed", Toast.LENGTH_SHORT).show();
                return 0;
            }
        });
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        PermissionHelper.onMyRequestPermissionsResult(requestCode, permissions, grantResults);
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }

    @Override
    protected void onResume () {
        super.onResume();
    }

    @Override
    protected void onDestroy () {
        super.onDestroy();
    }

    private void init () {

        findViewById(R.id.home_text_view).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startGLActivity ();
            }
        });

        ((Spinner)findViewById(R.id.home_spinner_effect)).setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                mEffectType = position;
                MyLog.d(TAG, "onItemSelected mEffectType = " + mEffectType);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
    }

    private void startGLActivity () {
//        if (bPermissionOK) {
            Intent intent = new Intent(this, GLViewActivity.class);
            intent.putExtra(CommonDefine.MESSAGE_EFFECT_TYPE, mEffectType);
            startActivity(intent);
//        }
        /*else {
            Toast.makeText(HomeActivity.this, "Please ammit permission", Toast.LENGTH_SHORT).show();
        }*/
    }

}