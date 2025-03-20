package com.example.breakout;

import android.graphics.Canvas;
import android.view.SurfaceHolder;

class GameThread extends Thread
{
    private SurfaceHolder surfaceHolder;
    private Game gameView;
    private boolean running;
    private boolean paused = false;

    public GameThread(SurfaceHolder holder, Game view)
    {
        this.surfaceHolder = holder;
        this.gameView = view;
    }

    public void setRunning(boolean isRunning)
    {
        running = isRunning;
    }
    public void setPaused(boolean isPaused)
    {
        paused = isPaused;
    }

    @Override
    public void run()
    {
        while (running)
        {
            if(paused)
            {
                continue;
            }

            Canvas canvas = null;
            try
            {
                canvas = surfaceHolder.lockCanvas();
                synchronized (surfaceHolder)
                {
                    gameView.update();
                    gameView.render(canvas);
                }
            }
            finally
            {
                if (canvas != null)
                {
                    surfaceHolder.unlockCanvasAndPost(canvas);
                }
            }

            // Sleep for 16ms to maintain ~60 FPS
            try
            {
                sleep(16);
            }
            catch (InterruptedException e)
            {
                e.printStackTrace();
            }
        }
    }
}
