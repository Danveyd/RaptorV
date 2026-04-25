package com.danvexteam.raptorv

object RaptorNative {
    init {
        System.loadLibrary("raptorv")
    }

    external fun initEngine()
    external fun updateEngine(deltaTime: Float)
}