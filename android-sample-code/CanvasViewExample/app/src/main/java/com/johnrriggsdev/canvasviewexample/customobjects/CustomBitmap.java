package com.johnrriggsdev.canvasviewexample.customobjects;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Paint;

import com.johnrriggsdev.canvasviewexample.R;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/**
 * Created by johnriggs on 11/3/16.
 */

public class CustomBitmap {
    public float xStartPixel;
    public float yStartPixel;
    public float bitmapWidth;
    public float bitmapHeight;

    public Bitmap bitmap;
    public Paint bitmapPaint;

    public CustomBitmap(float screenWidth, float screenHeight, Context context) {
        bitmap = BitmapFactory.decodeResource(context.getResources(), R.drawable.image);
        bitmapWidth = bitmap.getWidth();
        bitmapHeight = bitmap.getHeight();
        this.xStartPixel = (screenWidth / 2) - (bitmapWidth / 2);
        this.yStartPixel = (screenHeight / 2) - (bitmapHeight / 2);

        bitmapPaint = new Paint();
    }

    public static List<CustomBitmap> initializeList(int screenWidth, int screenHeight, Context context){
        List<CustomBitmap> lineList = new ArrayList<>();

        lineList.add(new CustomBitmap(screenWidth, screenHeight, context));

        return lineList;
    }

    public static List<CustomBitmap> randomizeList(List<CustomBitmap> bitmapList, int screenWidth,
                                                 int screenHeight){
        Random random = new Random();
        for (CustomBitmap customBitmap : bitmapList){
            customBitmap.xStartPixel = (screenWidth - customBitmap.bitmapWidth) * random.nextFloat();
            customBitmap.yStartPixel = (screenHeight - customBitmap.bitmapHeight) * random.nextFloat();
        }

        return bitmapList;
    }
}
