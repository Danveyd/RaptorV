package com.danvexteam.raptorv

import android.content.Context
import android.util.AttributeSet
import android.view.Choreographer
import android.view.SurfaceHolder
import android.view.SurfaceView
import com.danvexteam.raptorv.internal.RaptorNative

class RaptorView @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null
) : SurfaceView(context, attrs), SurfaceHolder.Callback, Choreographer.FrameCallback {

    private var engine: Engine? = null
    private var lastFrameTimeNanos: Long = 0
    private var isRunning = false


    var onTick: ((deltaTime: Float) -> Unit)? = null

    init { holder.addCallback(this) }

    fun attachEngine(e: Engine) {
        this.engine = e
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        engine?.let {
            RaptorNative.onSurfaceCreated(it.handle, holder.surface)
            isRunning = true
            lastFrameTimeNanos = System.nanoTime()
            Choreographer.getInstance().postFrameCallback(this)
        }
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        engine?.let { RaptorNative.onSurfaceChanged(it.handle, width, height) }
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        isRunning = false
        Choreographer.getInstance().removeFrameCallback(this)
        engine?.let { RaptorNative.onSurfaceDestroyed(it.handle) }
    }

    override fun doFrame(frameTimeNanos: Long) {
        if (!isRunning || engine == null) return

        val deltaTime = (frameTimeNanos - lastFrameTimeNanos) / 1_000_000_000.0f
        lastFrameTimeNanos = frameTimeNanos

        onTick?.invoke(deltaTime)
        engine?.tick(deltaTime)

        Choreographer.getInstance().postFrameCallback(this)
    }
}
