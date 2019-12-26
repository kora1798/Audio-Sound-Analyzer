package ru.gamsoft.dplus;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Binder;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.Process;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;

public class AudioRecordService extends Service {
    final static String DIR_SD = "D+";
    final static String TAG = "myLogs";
    AudioRecord audioRecord;
    boolean isReading = false;
    boolean isWavFileCreated;
    int totalSizeInBytes; //total size of data in bytes was gotten from record (needed for writing wav file header)
    DataOutputStream dos;
    File sdFile;
    RecordThread recordThread;
    Handler mainActivityHandler;

    @Override
    public void onCreate() {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            NotificationManager notificationManager = (NotificationManager)getSystemService(Context.NOTIFICATION_SERVICE);
            NotificationChannel channel = new NotificationChannel("default", "mychannel", NotificationManager.IMPORTANCE_NONE);
            channel.setDescription("My channel description");
            channel.enableVibration(false);
            notificationManager.createNotificationChannel(channel);
        }
        Notification notification = new NotificationCompat.Builder(this, "default")
                .setSmallIcon(R.mipmap.ic_launcher)
                .setContentTitle("D+")
                .setContentText("Recording sound")
                .build();
        startForeground(1, notification);
        super.onCreate();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return new LocalBinder();
    }

    @Override
    public boolean onUnbind(Intent intent) {
        return super.onUnbind(intent);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    public void setHandler(Handler handler){
        mainActivityHandler = handler;
    }

    public void writeWavFileHeader(){
        Log.d(TAG, "writeWavFileHeader()");
        int totalDataLen = totalSizeInBytes + 36, longSampleRate = 16000, byteRate = 32000;
        byte[] header = new byte[44];
        header[0] = 'R';
        header[1] = 'I';
        header[2] = 'F';
        header[3] = 'F';
        header[4] = (byte) (totalDataLen & 0xff);
        header[5] = (byte) ((totalDataLen >> 8) & 0xff);
        header[6] = (byte) ((totalDataLen >> 16) & 0xff);
        header[7] = (byte) ((totalDataLen >> 24) & 0xff);
        header[8] = 'W';
        header[9] = 'A';
        header[10] = 'V';
        header[11] = 'E';
        header[12] = 'f';  // 'fmt ' chunk
        header[13] = 'm';
        header[14] = 't';
        header[15] = ' ';
        header[16] = 16;  // 4 bytes: size of 'fmt ' chunk
        header[17] = 0;
        header[18] = 0;
        header[19] = 0;
        header[20] = 1;  // format = 1
        header[21] = 0;
        header[22] = (byte)1;
        header[23] = 0;
        header[24] = (byte) (longSampleRate & 0xff);
        header[25] = (byte) ((longSampleRate >> 8) & 0xff);
        header[26] = (byte) ((longSampleRate >> 16) & 0xff);
        header[27] = (byte) ((longSampleRate >> 24) & 0xff);
        header[28] = (byte) (byteRate & 0xff);
        header[29] = (byte) ((byteRate >> 8) & 0xff);
        header[30] = (byte) ((byteRate >> 16) & 0xff);
        header[31] = (byte) ((byteRate >> 24) & 0xff);
        header[32] = (byte) (2);  // block align
        header[33] = 0;
        header[34] = 16;  // bits per sample
        header[35] = 0;
        header[36] = 'd';
        header[37] = 'a';
        header[38] = 't';
        header[39] = 'a';
        header[40] = (byte) (totalSizeInBytes & 0xff);
        header[41] = (byte) ((totalSizeInBytes >> 8) & 0xff);
        header[42] = (byte) ((totalSizeInBytes >> 16) & 0xff);
        header[43] = (byte) ((totalSizeInBytes >> 24) & 0xff);
        try {
            RandomAccessFile raf = new RandomAccessFile(sdFile, "rw");
            raf.write(header);
        }catch (IOException e){
            e.printStackTrace();
        }
    }
    public void writeWavFile(short[] sh){
        byte b1, b2;
        byte[] bytes = new byte[2 * sh.length];
        int counter = 0;
        for (short p : sh){
            b1 = (byte)(p & 0xff);
            b2 = (byte)((p >> 8) & 0xff);
            bytes[counter] = b1;
            bytes[counter+1] = b2;
            counter += 2;
        }
        try {
            dos.write(bytes);
        }catch (Exception e){
            e.printStackTrace();
        }
    }
    public boolean createWavFile(){
        DateFormat df = new SimpleDateFormat("yyyyMMdd-HHmmss");
        Calendar cal = Calendar.getInstance();
        String FILENAME = df.format(cal.getTime()) + ".wav";
        byte[] temp = new byte[44];
        if (!Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)){
            Log.d(TAG, "Cant access the SD card");
            return false;
        }
        File sdPath = Environment.getExternalStorageDirectory();
        sdPath = new File(sdPath.getAbsolutePath() + "/" + DIR_SD);
        if (!sdPath.exists())
            sdPath.mkdirs();
        sdFile = new File(sdPath, FILENAME);
        try{
            dos = new DataOutputStream(new FileOutputStream(sdFile));
            dos.write(temp);
            return true;
        }catch (IOException e){
            Log.d(TAG, "Didn't write file");
            e.printStackTrace();
            return false;
        }
    }
    protected AudioRecord CreateAudioRecorder() {
        int sampleRate = 16000, //size of sample
                channelConfig = AudioFormat.CHANNEL_IN_MONO,
                audioFormat = AudioFormat.ENCODING_PCM_16BIT,
                minInternalBufferSize = AudioRecord.getMinBufferSize(sampleRate, channelConfig, audioFormat),
                internalBufferSize = minInternalBufferSize * 10; //standart of internalBuffer is minInternalBufferSize * 10
        return new AudioRecord(MediaRecorder.AudioSource.MIC,
                sampleRate, channelConfig, audioFormat, internalBufferSize);
    }
    public void recordStart(){
        isWavFileCreated = createWavFile();
        int ret = a2start(); //beginning of work on c++
        Log.d(TAG, "recordStart: value of a2start() = " + ret);
        isReading = true;
        totalSizeInBytes = 0;
        recordThread = new RecordThread();
        recordThread.start();
    }
    public void recordStop(){
        isReading = false;
        int ret = a2stop();//end of work on c++
        Log.d(TAG, "recordStop: value of a2stop() = "+ ret);
        try {
            recordThread.join();
        }catch (InterruptedException e){
            e.printStackTrace();
        }
    }
    public class RecordThread extends Thread {
        @Override
        public void run() {
            final int bufferSize = 16000, //
                    offset = 11; //for getparameter()
            short[] buffer = new short[bufferSize], //buffer of shorts, getting from audioRecord
                    temporarySpecialBuffer = new short[bufferSize * 2], //special buffer that keep last 2 sec of record (for Chart) size: 16000 * 2 = 32000
                    realSpecialBuffer; //real special buffer if parameter = 2 or 3 for Chart
            int readCount, state, i, ms51, ms52; //ms51 and ms52 for Chart
            int[] a2getParameterArray = new int[9];
            mainActivityHandler.sendMessage(mainActivityHandler.obtainMessage(0, a2getParameterArray));
            android.os.Process.setThreadPriority(Process.THREAD_PRIORITY_AUDIO);
            audioRecord = CreateAudioRecorder();
            audioRecord.startRecording();
            while (isReading) {
                readCount = audioRecord.read(buffer, 0, bufferSize);
                a2fragment(buffer, readCount);
                for (i = 0; i < a2getParameterArray.length; i++) {
                    a2getParameterArray[i] = a2getparameter(i + offset);
                }
                mainActivityHandler.sendEmptyMessage(1);
                //fill the temporarySpecialBuffer with last 2 seconds: it refreshes every 1 sec
                if (readCount == bufferSize) {
                    System.arraycopy(temporarySpecialBuffer, 16000, temporarySpecialBuffer, 0, 16000);
                    System.arraycopy(buffer, 0, temporarySpecialBuffer, 16000, 16000);
                }
                //
                state = a2getparameter(11);
                //save last 2 sec of record for Chart to realSpecialBuffer because of the event: state == 2 or ==3
                if (state == 2 || state == 3) {
                    realSpecialBuffer = new short[32000];
                    System.arraycopy(temporarySpecialBuffer, 0, realSpecialBuffer, 0, 32000);
                    ms51 = a2getparameter(19);
                    ms52 = a2getparameter(20);
                    mainActivityHandler.sendMessage(Message.obtain(mainActivityHandler, 2, ms51, ms52));
                    mainActivityHandler.sendMessage(Message.obtain(mainActivityHandler, 3, realSpecialBuffer));
                }
                totalSizeInBytes += 2 * readCount;
                Log.d(TAG, "ReadCountOfShorts = " + readCount + " TotalCountInShort = " + totalSizeInBytes / 2);
                writeWavFile(buffer);
            }
            audioRecord.stop();
            audioRecord.release();
            writeWavFileHeader();
            try {
                dos.close();
            } catch (IOException e) {
                Log.d(TAG, "Exception in closing wavfile");
                e.printStackTrace();
            }
        }
    }
    public class LocalBinder extends Binder {
        AudioRecordService getService(){
            return AudioRecordService.this;
        }
    }
    public native int a2start();
    public native int a2stop();
    public native int a2getparameter(int par);
    public native int a2fragment(short[] sample, int fragment);
}
