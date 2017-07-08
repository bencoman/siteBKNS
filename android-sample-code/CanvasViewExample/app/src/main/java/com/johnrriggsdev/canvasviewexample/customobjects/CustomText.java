package com.johnrriggsdev.canvasviewexample.customobjects;

import android.content.Context;
import android.graphics.Paint;
import android.graphics.Typeface;

import com.johnrriggsdev.canvasviewexample.R;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/**
 * Created by johnriggs on 11/3/16.
 */

public class CustomText {
    enum TEXT_TYPE {ORANGE_BOLD, WHITE_ITALICS}
    public float xPixel;
    public float yPixel;
    public float shadowXPixel;
    public float shadowYPixel;
    public String text;

    public Paint textPaint;
    public Paint shadowPaint;

    static final float TEXT_SIZE = 75f;
    static final float SHADOW_OFFSET = 10f;

    private CustomText(float xPixel, float yPixel, String text, TEXT_TYPE textType, Context context){
        this.xPixel = xPixel;
        this.yPixel = yPixel;
        shadowXPixel = xPixel + SHADOW_OFFSET;
        shadowYPixel = yPixel + SHADOW_OFFSET;
        this.text = text;

        textPaint = new Paint();
        textPaint.setFlags(Paint.ANTI_ALIAS_FLAG);
        textPaint.setTextSize(TEXT_SIZE);
        textPaint.setTextAlign(Paint.Align.CENTER);

        shadowPaint = new Paint();
        shadowPaint.setFlags(Paint.ANTI_ALIAS_FLAG);
        shadowPaint.setTextSize(TEXT_SIZE);
        shadowPaint.setTextAlign(Paint.Align.CENTER);
        shadowPaint.setColor(context.getResources().getColor(R.color.shadow));

        switch (textType){
            case ORANGE_BOLD:
                textPaint.setColor(context.getResources().getColor(R.color.orange));
                textPaint.setTypeface(Typeface.create(Typeface.DEFAULT, Typeface.BOLD));
                shadowPaint.setTypeface(Typeface.create(Typeface.DEFAULT, Typeface.BOLD));
                break;
            case WHITE_ITALICS:
                textPaint.setColor(context.getResources().getColor(R.color.white));
                textPaint.setTypeface(Typeface.create(Typeface.DEFAULT, Typeface.ITALIC));
                shadowPaint.setTypeface(Typeface.create(Typeface.DEFAULT, Typeface.ITALIC));
                break;
        }
    }

    public static List<CustomText> initializeList(int screenWidth, int screenHeight, Context context){
        List<CustomText> textList = new ArrayList<>();
        textList.add(new CustomText(
                screenWidth / 2,
                screenHeight / 4,
                context.getResources().getString(R.string.first_name),
                TEXT_TYPE.WHITE_ITALICS,
                context));
        textList.add(new CustomText(
                screenWidth / 2,
                screenHeight / 4 + TEXT_SIZE,
                context.getResources().getString(R.string.last_name),
                TEXT_TYPE.ORANGE_BOLD,
                context));

        return textList;
    }

    public static List<CustomText> randomizeList(List<CustomText> textList, int screenWidth,
                                                 int screenHeight){
        Random random = new Random();
        for (CustomText customText : textList){
            float textWidth = customText.textPaint.measureText(customText.text);
            customText.xPixel = (screenWidth - (textWidth / 2)) * random.nextFloat();
            if (customText.xPixel < (textWidth / 2)){
                customText.xPixel = textWidth / 2;
            }
            customText.yPixel = (screenHeight - TEXT_SIZE) * random.nextFloat();
            customText.shadowXPixel = customText.xPixel + SHADOW_OFFSET;
            customText.shadowYPixel = customText.yPixel + SHADOW_OFFSET;
        }

        return textList;
    }
}