package com.example.breakout;

import android.app.AlertDialog;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.MotionEvent;
import androidx.annotation.NonNull;

public class Game extends SurfaceView implements SurfaceHolder.Callback
{
    private GameThread gameThread;
    private Paint paint;
    private Context myContext;

    static
    {
        System.loadLibrary("breakout"); // Ensure this matches your C++ library name
    }

    //C++ methods that handle logic
    public native void nativeInit();
    public native void nativeUpdate();
    public native void nativeMovePaddle(float pos);
    public native float nativeGetPaddleHeight();
    public native float nativeGetPaddleWidth();
    public native float nativeGetPaddleX();
    public native float nativeGetPaddleY();
    public native float nativeGetBallX();
    public native float nativeGetBallY();
    public native float nativeGetBallRadius();
    public native float nativeGetBrickX(int index);
    public native float nativeGetBrickY(int index);
    public native int nativeGetBrickType(int index);
    public native int nativeGetBricksCount();
    public native boolean nativeIsBrickDestroyed(int index);
    public native int nativeGetScore();
    public native int nativeGetLives();


    public Game(Context context)
    {
        super(context);
        getHolder().addCallback(this);

        paint = new Paint();
        myContext = context;

        nativeInit();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder)
    {
        gameThread = new GameThread(holder, this);
        gameThread.setRunning(true);
        gameThread.start();
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height)
    {
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder)
    {
        gameThread.setRunning(false);
        try
        {
            gameThread.join();
        } catch (InterruptedException e)
        {
            e.printStackTrace();
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        if (event.getAction() == MotionEvent.ACTION_MOVE)
        {
            float pos = event.getX() / getWidth();
            nativeMovePaddle(pos);
        }
        return true;
    }

    public void render(Canvas canvas)
    {
        //Set background color
        canvas.drawColor(Color.GREEN);

        // Draw Paddle
        paint.setColor(Color.RED);
        float paddleLeft = nativeGetPaddleX() * getWidth();
        float paddleTop = nativeGetPaddleY() * getHeight();
        float paddleRight = paddleLeft + (nativeGetPaddleWidth() * getWidth());
        float paddleBottom = paddleTop + (nativeGetPaddleHeight() * getHeight());
        canvas.drawRect(paddleLeft, paddleTop, paddleRight, paddleBottom, paint);

        // Draw Ball
        paint.setColor(Color.BLUE);
        float ballCenterX = nativeGetBallX() * getWidth();
        float ballCenterY = nativeGetBallY() * getHeight();
        float ballRadiusPx = nativeGetBallRadius() * getHeight(); // Convert radius to pixels
        canvas.drawCircle(ballCenterX, ballCenterY, ballRadiusPx, paint);

        // Draw Bricks
        paint.setColor(Color.BLUE);
        int brickCount = nativeGetBricksCount();
        for (int i = 0; i < brickCount; i++)
        {
            if (!nativeIsBrickDestroyed(i))
            {
                float brickX = nativeGetBrickX(i) * getWidth();
                float brickY = nativeGetBrickY(i) * getHeight();
                paint.setColor(nativeGetBrickType(i) == 0 ? Color.BLUE: Color.MAGENTA);
                canvas.drawRect(brickX, brickY, brickX + 130, brickY + 70, paint);
            }
        }

        //Draw HUD (score and lives)
        paint.setColor(Color.BLACK);
        paint.setTextSize(70);
        canvas.drawText("Score: " + nativeGetScore() + "  Lives: " + nativeGetLives(), 0.25f * getWidth(), 0.12f * getHeight(), paint);

    }

    public void update()
    {
        nativeUpdate();
    }

    public void showGameOverScreen()
    {
        gameThread.setPaused(true);

        ((MainActivity) myContext).runOnUiThread(() ->
        {
            new AlertDialog.Builder(myContext)
                    .setTitle("Game Over")
                    .setMessage("You lost all lives!")
                    .setCancelable(false)
                    .setPositiveButton("Retry", (dialog, which) ->
                    {
                        nativeInit();
                        gameThread.setPaused(false);

                    })
                    .setNegativeButton("Exit", (dialog, which) ->
                    {
                        ((MainActivity) myContext).finish();
                    })
                    .show();
        });
    }
}
