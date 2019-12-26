package ru.gamsoft.dplus;

import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.OrientationEventListener;

public class SensorPortraitActivity extends AppCompatActivity {
    private static final int PORTRAIT = 0;
    private static final int REVERSE_PORTRAIT = 180;
    private static final int OFFSET = 45;
    private static final int UNKNOWN = -1;

    private static final int PORTRAIT_START = PORTRAIT - OFFSET + 360;
    private static final int PORTRAIT_END = PORTRAIT + OFFSET;
    private static final int REVERSE_PORTRAIT_START = REVERSE_PORTRAIT - OFFSET;
    private static final int REVERSE_PORTRAIT_END = REVERSE_PORTRAIT + OFFSET;

    private OrientationEventListener mOrientationListener;
    private CurrentOrientation mCurrentOrientation;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mOrientationListener = new OrientationEventListener(this) {
            @Override
            public void onOrientationChanged(int i) {
                orientationChanged(i);
            }
        };
    }

    @Override
    protected void onResume() {
        super.onResume();
        mOrientationListener.enable();
    }

    @Override
    protected void onPause() {
        super.onPause();
        mOrientationListener.disable();
    }

    private void orientationChanged(int degrees) {
        if (degrees != UNKNOWN){
            if (degrees >= PORTRAIT_START || degrees <= PORTRAIT_END){
                if (mCurrentOrientation != CurrentOrientation.PORTRAIT){
                    mCurrentOrientation = CurrentOrientation.PORTRAIT;
                    setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
                }
            } else if (degrees >= REVERSE_PORTRAIT_START && degrees <= REVERSE_PORTRAIT_END){
                if (mCurrentOrientation != CurrentOrientation.REVERSE_PORTRAIT){
                    mCurrentOrientation = CurrentOrientation.REVERSE_PORTRAIT;
                    setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT);
                }
            }
        }
    }

    enum CurrentOrientation {
        PORTRAIT, REVERSE_PORTRAIT
    }
}
