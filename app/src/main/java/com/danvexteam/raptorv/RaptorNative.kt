package com.danvexteam.raptorv

import android.content.res.AssetManager
import android.view.Surface

object RaptorNative {
    init { System.loadLibrary("raptorv") }

    external fun initEngine(assetManager: AssetManager)

    external fun updateEngine(deltaTime: Float)
    external fun onSurfaceCreated(surface: Surface)
    external fun onSurfaceDestroyed()
}