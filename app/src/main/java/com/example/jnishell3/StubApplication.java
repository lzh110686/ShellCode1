package com.example.jnishell3;

import androidx.appcompat.app.AppCompatActivity;

import android.app.Application;
import android.content.Context;
import android.os.Bundle;
import android.widget.TextView;

public class StubApplication extends Application {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary ("native-lib");
    }

    public native void onCreate();
    public native void  attachBaseContext(Context base);
}
