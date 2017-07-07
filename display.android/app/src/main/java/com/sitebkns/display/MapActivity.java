package com.sitebkns.display;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.Button;

import com.sitebkns.display.R;

public class MapActivity extends AppCompatActivity {
    CustomCanvasView customCanvasView;
    Button randomizeButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_map);

        customCanvasView = (CustomCanvasView) findViewById(R.id.custom_canvas_view);
        customCanvasView = (CustomCanvasView) findViewById(R.id.custom_canvas_view);
        customCanvasView.initializeObjects(this);
    }
}
