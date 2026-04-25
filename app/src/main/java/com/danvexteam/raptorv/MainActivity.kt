package com.danvexteam.raptorv

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.Choreographer
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.widget.TextView
import com.danvexteam.raptorv.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity(), Choreographer.FrameCallback, SurfaceHolder.Callback {

    private var lastFrameTimeNanos: Long = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val surfaceView = SurfaceView(this)
        surfaceView.holder.addCallback(this)
        setContentView(surfaceView)

        RaptorNative.initEngine(assets)

        lastFrameTimeNanos = System.nanoTime()
        Choreographer.getInstance().postFrameCallback(this)
    }

    override fun doFrame(frameTimeNanos: Long) {
        val deltaTime = (frameTimeNanos - lastFrameTimeNanos) / 1_000_000_000.0f
        lastFrameTimeNanos = frameTimeNanos

        RaptorNative.updateEngine(deltaTime)
        Choreographer.getInstance().postFrameCallback(this)
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        RaptorNative.onSurfaceCreated(holder.surface)
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        RaptorNative.onSurfaceDestroyed()
    }
}