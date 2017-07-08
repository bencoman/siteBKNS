package com.johnrriggsdev.canvasviewexample.customobjects;

import android.content.Context;
import android.graphics.Paint;

import com.johnrriggsdev.canvasviewexample.R;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/**
 * Created by johnriggs on 11/3/16.
 */

public class CustomLine {
    enum LINE_TYPE {THIN_LINE, THICK_LINE}
    public float xStartPixel;
    public float yStartPixel;
    public float xEndPixel;
    public float yEndPixel;
    public Paint linePaint;
    private float THIN_STROKE = 2f;
    private float THICK_STROKE = 15f;

    public CustomLine(LINE_TYPE lineType, float xStartPixel, float yStartPixel, float xEndPixel,
                      float yEndPixel, Context context) {
        this.xStartPixel = xStartPixel;
        this.yStartPixel = yStartPixel;
        this.xEndPixel = xEndPixel;
        this.yEndPixel = yEndPixel;

        linePaint = new Paint();

        switch (lineType) {
            case THIN_LINE:
                linePaint.setColor(context.getResources().getColor(R.color.black));
                linePaint.setStrokeWidth(THIN_STROKE);
                break;
            case THICK_LINE:
                linePaint.setColor(context.getResources().getColor(R.color.white));
                linePaint.setStrokeWidth(THICK_STROKE);
                break;
        }
    }

    public static List<CustomLine> initializeList(int screenWidth, int screenHeight, Context context){
        List<CustomLine> lineList = new ArrayList<>();
        float x1Pixel = screenWidth / 4;
        float y1Pixel = screenHeight / 4;
        float x2Pixel = screenWidth - (screenWidth / 4);
        float y2Pixel = screenHeight - (screenHeight / 4);

        lineList.add(new CustomLine(LINE_TYPE.THIN_LINE, x1Pixel, y1Pixel, x2Pixel,
                y2Pixel, context));

        lineList.add(new CustomLine(LINE_TYPE.THICK_LINE, x2Pixel, y1Pixel, x1Pixel,
                y2Pixel, context));

        return lineList;
    }

    public static List<CustomLine> randomizeList(List<CustomLine> lineList, int screenWidth,
                                                 int screenHeight){
        Random random = new Random();
        for (CustomLine customLine : lineList){
            customLine.xStartPixel = screenWidth * random.nextFloat();
            customLine.yStartPixel = screenHeight * random.nextFloat();
            customLine.xEndPixel = screenWidth * random.nextFloat();
            customLine.yEndPixel = screenHeight * random.nextFloat();
        }

        return lineList;
    }
}
