package com.johnrriggsdev.canvasviewexample;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class MainActivity extends AppCompatActivity {
    CustomCanvasView customCanvasView;
    Button randomizeButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        customCanvasView = (CustomCanvasView) findViewById(R.id.custom_canvas_view);
        randomizeButton = (Button) findViewById(R.id.randomize_button);

        randomizeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                customCanvasView.randomizeObjects();
            }
        });

        customCanvasView.initializeObjects(this);
    }
}
