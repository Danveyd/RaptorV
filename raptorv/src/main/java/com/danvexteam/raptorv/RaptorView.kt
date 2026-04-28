package com.danvexteam.raptorv

import android.content.Context
import android.util.AttributeSet
import android.view.Choreographer
import android.view.SurfaceHolder
import android.view.SurfaceView

class RaptorView @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null
) : SurfaceView(context, attrs), SurfaceHolder.Callback, Choreographer.FrameCallback {

    private var lastFrameTimeNanos: Long = 0
    private var isRunning = false

    init {
        holder.addCallback(this)
        RaptorNative.initEngine(context.assets)
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        RaptorNative.onSurfaceCreated(holder.surface)
        isRunning = true
        lastFrameTimeNanos = System.nanoTime()
        Choreographer.getInstance().postFrameCallback(this)
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        RaptorNative.onSurfaceChanged(width, height)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        isRunning = false
        Choreographer.getInstance().removeFrameCallback(this)
        RaptorNative.onSurfaceDestroyed()
    }

    override fun doFrame(frameTimeNanos: Long) {
        if (!isRunning) return
        val deltaTime = (frameTimeNanos - lastFrameTimeNanos) / 1_000_000_000.0f
        lastFrameTimeNanos = frameTimeNanos

        RaptorNative.updateEngine(deltaTime)
        Choreographer.getInstance().postFrameCallback(this)
    }
}
