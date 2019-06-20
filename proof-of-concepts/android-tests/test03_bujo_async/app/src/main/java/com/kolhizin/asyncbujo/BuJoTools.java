package com.kolhizin.asyncbujo;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;

import java.util.List;

public class BuJoTools {
    public static Path makeLinePath(int w, int h, float []xs, float []ys){
        Path p = new Path();
        p.moveTo(w * xs[0], h* ys[0]);
        for(int i = 0; i < xs.length; i++)
            p.lineTo(w * xs[i], h*ys[i]);
        return p;
    }
    public static Path makeLinePath(int w, int h, BuJoPage.BuJoLine line){
        return makeLinePath(w, h, line.xCoords, line.yCoords);
    }

    public static Bitmap drawLines(Bitmap src, List<BuJoPage.BuJoLine> lines) {
        int w = src.getWidth();
        int h = src.getHeight();

        Bitmap res = src.copy(src.getConfig(), true);
        Canvas canvas = new Canvas(res);
        Paint paint = new Paint();
        paint.setColor(Color.rgb(255,0, 0));
        paint.setAlpha(110);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(10);

        for(int i = 0; i < lines.size(); i++){
            canvas.drawPath(makeLinePath(w, h, lines.get(i)), paint);
        }

        return res;
    }

    public static Bitmap drawWords(Bitmap src, BuJoPage.BuJoWord [][]words) {
        int w = src.getWidth();
        int h = src.getHeight();

        Bitmap res = src.copy(src.getConfig(), true);
        Canvas canvas = new Canvas(res);
        Paint []paints = new Paint[2];
        paints[0] = new Paint();
        paints[1] = new Paint();
        paints[0].setColor(Color.rgb(0,255, 0));
        paints[0].setStrokeWidth(10);
        paints[0].setStyle(Paint.Style.STROKE);
        paints[1].setColor(Color.rgb(0,0, 255));
        paints[1].setStrokeWidth(10);
        paints[1].setStyle(Paint.Style.STROKE);

        int cntr = 0;
        for(int i = 0; i < words.length; i++)
            for(int j = 0; j < words[i].length; j++) {
                canvas.drawPath(makeLinePath(w, h, words[i][j].xCoords, words[i][j].yCoords), paints[cntr%2]);
                cntr++;
            }

        return res;
    }

    public static Path makeShadeRegion(int w, int h, float angle, float offset, int dir){
        if(dir == 0)
            return new Path();

        float x_min = -(float)(w) * 0.5f;
        float y_min = -(float)(h) * 0.5f;
        float a_x = (float)Math.sin(angle), a_y = -(float)Math.cos(angle);
        float diag = (float)Math.sqrt(w*w + h*h);
        float r_offset = x_min * a_x + y_min * a_y - offset * diag;

        if(Math.abs(a_x) < 1e-3f)
            throw new RuntimeException("BuJoTools.shadeBitmap() expects vertical splits, i.e. sin(angle) should not be zero!");

        //float val = (i * a_y + j * a_x + r_offset);
        //(x1, 0), (x2, h)
        //x1 * a_x + r_offset = 0 -> x1 = -r_offset / a_x
        //h * a_y + x2 * a_x + r_offset = 0 -> x2 = (-r_offset - h*a_y) / a_x

        float x1 = -r_offset / a_x;
        float x2 = -(r_offset + h * a_y) / a_x;
        float x3 = 0.0f, x4 = 0.0f;
        if(dir * a_x < 0.0f){
            x3 = Math.min(x1, 0.0f);
            x4 = Math.min(x2, 0.0f);
        }else{
            x3 = Math.max(x1, w);
            x4 = Math.max(x2, w);
        }
        Path p = new Path();
        p.moveTo(x1, 0.0f);
        p.lineTo(x2, h);
        p.lineTo(x4, h);
        p.lineTo(x3, 0.0f);
        p.close();
        return p;
    }

    public static Bitmap shadeRegions(Bitmap src, List<BuJoPage.BuJoSplit> splits){
        Path p = null;
        int w = src.getWidth();
        int h = src.getHeight();
        for(int i = 0; i < splits.size(); i++){
            BuJoPage.BuJoSplit split = splits.get(i);
            Path tmp = makeShadeRegion(w, h, split.angle, split.offset, split.direction);
            if(p == null)
                p = tmp;
            else
                p.op(tmp, Path.Op.UNION);
        }

        Bitmap res = src.copy(src.getConfig(), true);
        Canvas canvas = new Canvas(res);
        Paint paint = new Paint();
        paint.setColor(Color.rgb(0,0, 0));
        paint.setAlpha(110);
        paint.setStyle(Paint.Style.FILL_AND_STROKE);
        paint.setStrokeWidth(10);
        canvas.drawPath(p, paint);
        return res;
    }

}
