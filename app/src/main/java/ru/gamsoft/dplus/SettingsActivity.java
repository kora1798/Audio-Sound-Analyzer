package ru.gamsoft.dplus;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;


public class SettingsActivity extends AppCompatActivity {
    final String TAG = "myLogs";
    Button btnSave;
    int[] parValues;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        parValues = getIntent().getIntArrayExtra("parameters");
        btnSave = findViewById(R.id.btn);
        btnSave.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent();
                intent.putExtra("parameters", parValues);
                setResult(0, intent);
                finish();
            }
        });
        RecyclerView recyclerView = findViewById(R.id.recyclerView);
        MyAdapter adapter = new MyAdapter();
        recyclerView.setLayoutManager(new LinearLayoutManager(this));
        recyclerView.setAdapter(adapter);
        recyclerView.setItemViewCacheSize(parValues.length);
    }
    private class MyAdapter extends RecyclerView.Adapter<MyAdapter.MyViewHolder> {
        @NonNull
        @Override
        public MyViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
            View view = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.settings_item, viewGroup, false);
            return new MyViewHolder(view);
        }

        @Override
        public void onBindViewHolder(@NonNull MyViewHolder myViewHolder, int i) {
            myViewHolder.bind(i);
        }

        @Override
        public int getItemCount() {
            return parValues.length;
        }

        class MyViewHolder extends RecyclerView.ViewHolder {
            ImageButton btnUp, btnDown;
            EditText valueText;

            public MyViewHolder(View view) {
                super(view);
                btnUp = view.findViewById(R.id.btnUp);
                btnDown = view.findViewById(R.id.btnDown);
                valueText = view.findViewById(R.id.valueText);
            }

            public void bind(final int position) {
                valueText.setText(String.valueOf(parValues[position]));
                valueText.setOnFocusChangeListener(new View.OnFocusChangeListener() {
                    @Override
                    public void onFocusChange(View v, boolean hasFocus) {
                        if(!hasFocus){
                            InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                            imm.hideSoftInputFromWindow(valueText.getWindowToken(), 0);
                            if(valueText.getText().toString().isEmpty()) {
                                valueText.setText("0");
                                parValues[position] = 0;
                            }
                        }
                    }
                });
                valueText.addTextChangedListener(new TextWatcher() {
                    @Override
                    public void beforeTextChanged(CharSequence s, int start, int count, int after) {

                    }

                    @Override
                    public void onTextChanged(CharSequence s, int start, int before, int count) {

                    }

                    @Override
                    public void afterTextChanged(Editable s) {
                        if (!s.toString().isEmpty())
                            parValues[position] = Integer.valueOf(s.toString());
                    }
                });
                btnUp.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        int value = parValues[position];
                        if (value < 1000)
                            parValues[position] += 5;
                        else if (value < 2000)
                            parValues[position] += 10;
                        else if (value < 5000)
                            parValues[position] += 50;
                        else if (value < 10000)
                            parValues[position] += 100;
                        else
                            parValues[position] += 500;
                    valueText.setText(String.valueOf(parValues[position]));
                    }
                });
                btnDown.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        int value = parValues[position];
                        if (value <= 1000)
                            parValues[position] -= 5;
                        else if (value <= 2000)
                            parValues[position] -= 10;
                        else if (value <= 5000)
                            parValues[position] -= 50;
                        else if (value <= 10000)
                            parValues[position] -= 100;
                        else
                            parValues[position] -= 500;
                        valueText.setText(String.valueOf(parValues[position]));
                    }
                });
            }
        }
    }
    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
}
