package com.supreme.abc.powerwindowservice;

import android.net.Network;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;
import android.widget.Toast;

import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;

public class MainActivity extends AppCompatActivity {

    public static TextView statusText;

    public static Socket socket;
    public static ObjectOutputStream oos;
    public static ObjectInputStream ois;
    public static final Object NetLock = new Object();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        statusText =  findViewById(R.id.statusText);

        new Network();
    }

    public void StopBtnClick(View view) {
        new SendChar('z').executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    static class Network extends AsyncTask<String, String, Void> {

        private String MainServerIP = "192.168.1.8";
        private int MainServerPORT = 3000;

        public Network() {
            executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        }

        @Override
        protected Void doInBackground(String... s) {
            try {
                socket = new Socket(MainServerIP, MainServerPORT);
                oos = new ObjectOutputStream(socket.getOutputStream());
                ois = new ObjectInputStream(socket.getInputStream());

                while (true) {
                    String c = (String) ois.readObject();
                    publishProgress(c);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(String... msgs) {
            Log.v("AAA",msgs[0]);
            statusText.setText(msgs[0]);
        }
    }

    static class SendChar extends AsyncTask<String, Void, Void> {

        char c;

        public SendChar(char c) {
            this.c = c;
        }

        @Override
        protected Void doInBackground(String... s) {
            try {
                synchronized (NetLock) {
                    oos.writeObject(c);
                    oos.flush();
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            return null;
        }
    }
}
