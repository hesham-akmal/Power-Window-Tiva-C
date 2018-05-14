package com.supreme.abc.powerwindowservice;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Network;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.Editable;
import android.text.InputFilter;
import android.text.InputType;
import android.util.Log;
import android.view.ContextThemeWrapper;
import android.view.MotionEvent;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.lang.reflect.Type;
import java.net.Socket;

public class MainActivity extends AppCompatActivity {

    public static TextView statusText;

    public static Socket socket;
    public static ObjectOutputStream oos;
    public static ObjectInputStream ois;
    private boolean engineOn = false;

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        statusText =  findViewById(R.id.statusText);

        final EditText input = new EditText(this);
        input.setText("192.168.");
        setIPfilter(input);

        ///Power windows ctrl btns ///////////////////////////////////////////////////
        ImageButton upBtn =  findViewById(R.id.upBtn);
        ImageButton downBtn =  findViewById(R.id.downBtn);

        upBtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch(event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        // PRESSED
                        new SendChar('c').executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
                        return true;
                    case MotionEvent.ACTION_UP:
                        // RELEASED
                        new SendChar('c').executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
                        return true;
                }
                return false;
            }
        });

        downBtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch(event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        new SendChar('d').executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
                        return true;
                    case MotionEvent.ACTION_UP:
                        // RELEASED
                        new SendChar('d').executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
                        return true;
                }
                return false;
            }
        });

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        final Button button = findViewById(R.id.engineBtn);
        button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if(engineOn)
                {
                    new SendChar('b').executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
                    engineOn = false;
                }
                else
                {
                    new SendChar('b').executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
                    engineOn = true;
                }
            }
        });

        //////////////////////////////
        ///
        /// //Get IP and start Connection
        new AlertDialog.Builder(this)
                .setTitle("Update Status")
                .setMessage("Enter Server IP")
                .setView(input)
                .setPositiveButton("Connect", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        new Network(input.getText().toString());
                    }
                }).show();

        //new Network("192.168.1.8");

    }

    static class Network extends AsyncTask<String, String, Void> {

        private String MainServerIP;
        private int MainServerPORT = 10001;

        public Network(String MainServerIP) {
            this.MainServerIP = MainServerIP;
            Log.v("XXX",MainServerIP);
            executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        }

        @Override
        protected Void doInBackground(String... s) {
            try {
                //Connect to Server
                Log.v("XXX","CONNECTING");
                socket = new Socket(MainServerIP, MainServerPORT);
                oos = new ObjectOutputStream(socket.getOutputStream());
                ois = new ObjectInputStream(socket.getInputStream());
                Log.v("XXX","SUCCESS");
                ////

                ////Continously listen to messages being sent from server
                while (true) {
                    String c = (String) ois.readObject();
                    //publish on UI thread
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

    //Send a character to server
    static class SendChar extends AsyncTask<String, Void, Void> {
        char c;
        public SendChar(char c) {
            this.c = c;
        }
        @Override
        protected Void doInBackground(String... s) {
            try {
                oos.writeObject(c);
                oos.flush();
            } catch (Exception e) {
                e.printStackTrace();
            }
            return null;
        }
    }


    void setIPfilter(EditText et){
        InputFilter[] filters = new InputFilter[1];
        filters[0] = new InputFilter() {
            @Override
            public CharSequence filter(CharSequence source, int start, int end,
                                       android.text.Spanned dest, int dstart, int dend) {
                if (end > start) {
                    String destTxt = dest.toString();
                    String resultingTxt = destTxt.substring(0, dstart)
                            + source.subSequence(start, end)
                            + destTxt.substring(dend);
                    if (!resultingTxt
                            .matches("^\\d{1,3}(\\.(\\d{1,3}(\\.(\\d{1,3}(\\.(\\d{1,3})?)?)?)?)?)?")) {
                        return "";
                    } else {
                        String[] splits = resultingTxt.split("\\.");
                        for (int i = 0; i < splits.length; i++) {
                            if (Integer.valueOf(splits[i]) > 255) {
                                return "";
                            }
                        }
                    }
                }
                return null;
            }

        };
        et.setFilters(filters);
    }
}



