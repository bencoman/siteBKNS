package com.sitebkns.display;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.util.AttributeSet;
import android.view.View;

import java.util.List;

/**
 * Created by Ben on 2017-07-08.
 */

public class CustomCanvasView extends View {
    Bitmap bitmap;
    Canvas canvas;

    int screenWidth;
    int screenHeight;

    List<CustomText> textList;
    List<CustomRectF> rectList;
    List<CustomLine> lineList;
    List<CustomBitmap> bitmapList;

    public CustomCanvasView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        //Draws the canvas transparently
        canvas.drawColor(Color.argb(0, 255, 255, 255));

        //The order items are drawn indicates if they will be behind other items. Items drawn
        //first are in the background
        drawRects(canvas);
        drawLines(canvas);
        drawText(canvas);
        drawBitmaps(canvas);
    }

    public void initializeObjects(Context context){
        textList = CustomText.initializeList(screenWidth, screenHeight, context);
        rectList = CustomRectF.initializeList(screenWidth, screenHeight, context);
        lineList = CustomLine.initializeList(screenWidth, screenHeight, context);
        bitmapList = CustomBitmap.initializeList(screenWidth, screenHeight, context);
    }

    private void drawRects(Canvas canvas) {
        for (CustomRectF rect : rectList){
            canvas.drawRect(rect.xStartPixel, rect.yStartPixel, rect.xEndPixel, rect.yEndPixel, rect.rectPaint);
        }
    }

    private void drawLines(Canvas canvas) {
        for (CustomLine line : lineList){
            canvas.drawLine(line.xStartPixel, line.yStartPixel, line.xEndPixel, line.yEndPixel, line.linePaint);
        }
    }

    private void drawText(Canvas canvas) {
        for (CustomText text : textList){
            canvas.drawText(text.text, text.shadowXPixel, text.shadowYPixel, text.shadowPaint);
            canvas.drawText(text.text, text.xPixel, text.yPixel, text.textPaint);
        }
    }

    private void drawBitmaps(Canvas canvas) {
        for (CustomBitmap bitmap : bitmapList){
            canvas.drawBitmap(bitmap.bitmap, bitmap.xStartPixel, bitmap.yStartPixel, bitmap.bitmapPaint);
        }
    }

}
