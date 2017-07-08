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

public class CustomRectF {
    enum RECT_MODE {FILL, STROKE, FILL_N_STROKE}
    public float xStartPixel;
    public float yStartPixel;
    public float xEndPixel;
    public float yEndPixel;

    public Paint rectPaint;

    private static float RECT_WIDTH = 200f;
    private static float RECT_HEIGHT = 150f;
    private float THIN_STROKE = 2f;
    private float THICK_STROKE = 50f;

    private CustomRectF(RECT_MODE rectMode, float xStartPixel, float yStartPixel, float xEndPixel,
                        float yEndPixel, Context context){
        this.xStartPixel = xStartPixel;
        this.yStartPixel = yStartPixel;
        this.xEndPixel = xEndPixel;
        this.yEndPixel = yEndPixel;

        rectPaint = new Paint();

        switch (rectMode){
            case FILL:
                rectPaint.setStyle(Paint.Style.FILL);
                rectPaint.setColor(context.getResources().getColor(R.color.orange));
                break;
            case STROKE:
                rectPaint.setStyle(Paint.Style.STROKE);
                rectPaint.setColor(context.getResources().getColor(R.color.white));
                rectPaint.setStrokeWidth(THIN_STROKE);
                break;
            case FILL_N_STROKE:
                rectPaint.setStyle(Paint.Style.FILL_AND_STROKE);
                rectPaint.setColor(context.getResources().getColor(R.color.black));
                rectPaint.setStrokeWidth(THICK_STROKE);
                break;
        }
    }

    public static List<CustomRectF> initializeList(int screenWidth, int screenHeight, Context context){
        List<CustomRectF> rectList = new ArrayList<>();
        float startX = screenWidth / 4;
        float startY = screenHeight / 2;

        rectList.add(new CustomRectF(RECT_MODE.FILL, startX, startY, startX + RECT_WIDTH,
                startY + RECT_HEIGHT, context));
        startX += (RECT_WIDTH / 2);
        startY += (RECT_HEIGHT / 2);

        rectList.add(new CustomRectF(RECT_MODE.STROKE, startX, startY, startX + RECT_WIDTH,
                startY + RECT_HEIGHT, context));
        startX += (RECT_WIDTH / 2);
        startY += (RECT_HEIGHT / 2);

        rectList.add(new CustomRectF(RECT_MODE.FILL_N_STROKE, startX , startY, startX + RECT_WIDTH,
                startY + RECT_HEIGHT, context));

        return rectList;
    }

    public static List<CustomRectF> randomizeList(List<CustomRectF> rectList, int screenWidth,
                                                  int screenHeight){
        Random random = new Random();
        for (CustomRectF customRectF : rectList){
            customRectF.xStartPixel = (screenWidth - RECT_WIDTH) * random.nextFloat();
            customRectF.yStartPixel = (screenHeight - RECT_HEIGHT) * random.nextFloat();
            customRectF.xEndPixel = customRectF.xStartPixel + RECT_WIDTH;
            customRectF.yEndPixel = customRectF.yStartPixel + RECT_HEIGHT;
        }

        return rectList;
    }
}
