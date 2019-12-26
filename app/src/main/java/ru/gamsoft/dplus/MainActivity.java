package ru.gamsoft.dplus;

import android.Manifest;
import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Paint;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.media.SoundPool;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.PowerManager;
import android.os.Process;
import android.provider.MediaStore;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.github.mikephil.charting.charts.CandleStickChart;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.CandleData;
import com.github.mikephil.charting.data.CandleDataSet;
import com.github.mikephil.charting.data.CandleEntry;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.lang.ref.WeakReference;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;


public class MainActivity extends SensorPortraitActivity {
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    final static String TAG = "myLogs";
    //Views
    Handler handler;
    Context context;
    TextView tv0, tv1, tv2, tv3, tv4, tv5, tv6, tv7;
    Button start, stop, F1, F2, F3, settings, quit, switchIm;
    ImageButton send;
    EditText comment;
    ViewPager viewPager;
    //
    int[] parameters;
    final int bufferSize = 16000; //
    short[] realSpecialBuffer; //real special buffer if parameter = 2 or 3
    boolean isReading = false, isPicDirCreated;
    public File[] pics;
    static SoundPool soundPool;
    static int soundOne, soundTwo;
    CandleStickChart candle;
    boolean chartVisibility;
    int ms51, ms52;//for Chart
    AudioRecordService audioRecordService;
    ServiceConnection serviceConnection;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        context = this;
        int ret = a2init(); //turning on analyzer on c++
        Log.d(TAG, "onCreate: value of a2init() = " + ret);
        //chart initialization
        candle = findViewById(R.id.candle);
        candle.setVisibility(View.INVISIBLE);
        chartVisibility = false;
        //view in main_activity initialization
        viewPager = findViewById(R.id.vp);
        comment = findViewById(R.id.comment);
        send = findViewById(R.id.send);
        tv0 = findViewById(R.id.tv0);
        tv1 = findViewById(R.id.tv1);
        tv2 = findViewById(R.id.tv2);
        tv3 = findViewById(R.id.tv3);
        tv4 = findViewById(R.id.tv4);
        tv5 = findViewById(R.id.tv5);
        tv6 = findViewById(R.id.tv6);
        tv7 = findViewById(R.id.tv7);
        switchIm = findViewById(R.id.switchIm);
        settings = findViewById(R.id.settings);
        start = findViewById(R.id.start);
        stop = findViewById(R.id.stop);
        stop.setEnabled(false);
        quit = findViewById(R.id.quit);
        F1 = findViewById(R.id.F1);
        F2 = findViewById(R.id.F2);
        F3 = findViewById(R.id.F3);
        //sound initialization
        soundPool = new SoundPool.Builder().build();
        soundOne = soundPool.load(this, R.raw.soundone, 1);
        soundTwo = soundPool.load(this, R.raw.soundtwo, 1);
        handler = new MainHandler(this);
        requestPermissions();
        //events with view in main_activity
        start.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                recordStart();
            }
        });
        stop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                recordStop();
            }
        });
        switchIm.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (realSpecialBuffer != null) {
                    if (!chartVisibility) {
                        createChart();
                        viewPager.setVisibility(View.INVISIBLE);
                        candle.setVisibility(View.VISIBLE);
                        chartVisibility = true;
                    } else {
                        candle.setVisibility(View.INVISIBLE);
                        viewPager.setVisibility(View.VISIBLE);
                        chartVisibility = false;
                    }
                }
            }
        });
        settings.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(context, SettingsActivity.class);
                for (int i = 0; i < 10; i++)
                    parameters[i] = a2getparameter(i + 1);
                intent.putExtra("parameters", parameters);
                startActivityForResult(intent, 0);
            }
        });
        quit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (isReading)
                    recordStop();
                finish();
            }
        });
        send.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String str = comment.getText().toString();
                if (!str.isEmpty()) {
                    a2comment(str);
                    comment.getText().clear();
                    Toast.makeText(context, "Comment was sent", Toast.LENGTH_SHORT).show();
                }
            }
        });
        F1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                a2learn(1);
            }
        });
        F2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                a2learn(2);
            }
        });
        F3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                a2learn(3);
            }
        });
        //parameters (for c++ program) initialization
        parameters = new int[10];
        for (int i = 0; i < 10; i++)
            parameters[i] = a2getparameter(i + 1);
        serviceConnection = new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder service) {
                audioRecordService = ((AudioRecordService.LocalBinder)service).getService();
                audioRecordService.setHandler(handler);
                Log.d(TAG, "onServiceConnected");
                audioRecordService.recordStart();
            }
            @Override
            public void onServiceDisconnected(ComponentName name) {

            }
        };
    }

    public void requestPermissions() {
        String[] perms = {Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.RECORD_AUDIO};
        if (!hasPermissions(perms)) {
            ActivityCompat.requestPermissions(this, perms, 1);
        } else {
            isPicDirCreated = createPicDir();
            if (isPicDirCreated) {
                ImagePageAdapter adapter = new ImagePageAdapter();
                viewPager.setAdapter(adapter);
            }
        }
    }
    public boolean hasPermissions(String... perms) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            for (String p : perms) {
                if (checkSelfPermission(p) != PackageManager.PERMISSION_GRANTED)
                    return false;
            }
        }
        return true;
    }
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (grantResults[0] == PackageManager.PERMISSION_GRANTED && grantResults[1] == PackageManager.PERMISSION_GRANTED) {
            Log.d(TAG, "GRANTED!");
            isPicDirCreated = createPicDir();
            if (isPicDirCreated) {
                ImagePageAdapter adapter = new ImagePageAdapter();
                viewPager.setAdapter(adapter);
            }
        } else {
            Log.d(TAG, "DENIED!");
            finish();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (data != null) {
            parameters = data.getIntArrayExtra("parameters");
            for (int i = 0; i < parameters.length; i++)
                a2setparameter(i + 1, parameters[i]);
        }
    }

    public void createChart() {
        final int frameSize = 80;
        int maximum = -1, average, sum, prevAverage = 0, frame1, frame2, x;
        int[] temp = new int[frameSize];
        int[] specialBufferInt = new int[realSpecialBuffer.length];
        frame1 = (ms51) / 5; ///-3000
        frame2 = (ms52) / 5; ///-3000
        if (frame1 < 0) {
            frame1 = 0;
            if (frame2 < 0)
                frame2 = 0;
        }
        for (int i = 0; i < realSpecialBuffer.length; i++) { //Convert to int
            specialBufferInt[i] = (int) (realSpecialBuffer[i]);
        }
        candle.setDrawBorders(true);
        candle.setBorderColor(Color.rgb(235, 0, 60));
        XAxis xAxis = candle.getXAxis(); // absciss
        xAxis.setAxisMinimum(0);
        xAxis.setAxisMaximum(400);
        xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
        YAxis yAxis = candle.getAxisLeft(); //ordinata
        yAxis.setAxisMinimum(0);
        YAxis yaxis2 = candle.getAxisRight(); //ordinata #2
        yaxis2.setEnabled(false);
        candle.getLegend().setEnabled(false); //disable legend
        candle.getDescription().setEnabled(false); //disable description
        List<CandleEntry> entries = new ArrayList<>();
        for (int i = 0; i < bufferSize * 2; i += frameSize) { //set points for chart
            System.arraycopy(specialBufferInt, i, temp, 0, frameSize);
            x = i / frameSize;
            sum = 0;
            for (int j : temp)
                sum += j;
            average = Math.abs(sum / frameSize);
            maximum = Math.max(maximum, average);
            if (x > frame1 && x < frame2) {
                entries.add(new CandleEntry(x, Math.min(average, prevAverage), Math.max(average, prevAverage), Math.min(average, prevAverage), Math.max(average, prevAverage)));
            } else {
                entries.add(new CandleEntry(x, Math.max(average, prevAverage), Math.min(average, prevAverage), Math.max(average, prevAverage), Math.min(average, prevAverage)));
            }
            prevAverage = average;
        }
        yAxis.setAxisMaximum(maximum + maximum / 10);
        CandleDataSet candleDataSet = new CandleDataSet(entries, "candleDataSet");
        candleDataSet.setDecreasingColor(Color.rgb(0, 0, 125));
        candleDataSet.setDecreasingPaintStyle(Paint.Style.FILL);
        candleDataSet.setIncreasingColor(Color.rgb(200, 0, 30));
        candleDataSet.setIncreasingPaintStyle(Paint.Style.FILL);
        candleDataSet.setNeutralColor(Color.rgb(0, 0, 125));
        candleDataSet.setDrawValues(false);
        candleDataSet.setBarSpace(0.05f);
        CandleData candleData = new CandleData(candleDataSet);
        candle.setData(candleData);
        candle.invalidate();
    }

    public boolean createPicDir() {
        File sdPath = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM);
        sdPath = new File(sdPath.getAbsolutePath() + "/" + "Camera");
        pics = sdPath.listFiles();
        if (pics != null)
            return true;
        return false;
    }

    public void recordStart() {
        start.setEnabled(false);
        stop.setEnabled(true);
        Intent intent = new Intent(context, AudioRecordService.class);
        bindService(intent, serviceConnection, BIND_AUTO_CREATE);
    }

    public void recordStop() {
        start.setEnabled(true);
        stop.setEnabled(false);
        audioRecordService.recordStop();
        unbindService(serviceConnection);
    }

    static class MainHandler extends Handler {
        WeakReference<MainActivity> wrActivity;
        int[] tvValues;
        public MainHandler(MainActivity activity) {
            wrActivity = new WeakReference<MainActivity>(activity);
        }
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            MainActivity activity = wrActivity.get();
            if (activity != null) {
                switch (msg.what) {
                    case 0:
                        tvValues = (int[])(msg.obj);
                        break;
                    case 1:
                        if(tvValues[7] == 4)
                            soundPool.play(soundTwo, 1, 1, 1, 0, 1);
                        if (tvValues[7] == 6)
                            soundPool.play(soundOne, 1, 1, 1, 0, 1);
                        activity.tv0.setText(String.valueOf(tvValues[0]));
                        activity.tv1.setText(String.valueOf(tvValues[1]));
                        activity.tv2.setText(String.valueOf(tvValues[2]));
                        activity.tv3.setText(String.valueOf(tvValues[3]));
                        activity.tv4.setText(String.valueOf(tvValues[4]));
                        activity.tv5.setText(String.valueOf(tvValues[5]));
                        activity.tv6.setText(String.valueOf(tvValues[6]));
                        activity.tv7.setText(String.valueOf(tvValues[7]));
                        break;
                    case 2:
                        activity.ms51 = msg.arg1;
                        activity.ms52 = msg.arg2;
                        break;
                    case 3:
                        activity.realSpecialBuffer = ((short[]) msg.obj);
                        break;
                }
            }
        }
    }

    private class ImagePageAdapter extends PagerAdapter {
        @Override
        public boolean isViewFromObject(@NonNull View view, @NonNull Object o) {
            return view.equals(o);
        }

        @Override
        public int getCount() {
            return pics.length;
        }

        @NonNull
        @Override
        public Object instantiateItem(@NonNull ViewGroup container, int position) {
            Context context = MainActivity.this;
            ImageView imageView = new ImageView(context);
            imageView.setImageBitmap(BitmapFactory.decodeFile(pics[position].getAbsolutePath()));
            imageView.setScaleType(ImageView.ScaleType.CENTER_CROP);
            container.addView(imageView, 0);
            return imageView;
        }

        @Override
        public void destroyItem(@NonNull ViewGroup container, int position, @NonNull Object object) {
            container.removeView((ImageView) object);
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    protected void onDestroy() {
        int ret = a2dispose(); //turning off analyzer on c++
        Log.d(TAG, "onDestroy: value of a2dispose: " + String.valueOf(ret));
        isReading = false;
        if (handler != null)
            handler.removeCallbacksAndMessages(null);
        super.onDestroy();
    }

    public native int a2init();
    public native int a2dispose();
    public native int a2setparameter(int param_num, int value);
    public native int a2getparameter(int param_num);
    public native int a2learn(int fragment_type);
    public native int a2comment(String str);
}
