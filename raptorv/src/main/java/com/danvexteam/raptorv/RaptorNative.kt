package com.danvexteam.raptorv

import android.content.res.AssetManager
import android.view.Surface

object RaptorNative {
    init { System.loadLibrary("raptorv") }

    external fun initEngine(assetManager: AssetManager)
    external fun updateEngine(deltaTime: Float)
    external fun onSurfaceCreated(surface: Surface)
    external fun onSurfaceChanged(width: Int, height: Int)
    external fun onSurfaceDestroyed()

    external fun createEntity(name: String): Int
    external fun destroyEntity(entityId: Int)

    external fun loadGlTF(entityId: Int, path: String)

    external fun setPosition(entityId: Int, x: Float, y: Float, z: Float)
    external fun setRotation(entityId: Int, x: Float, y: Float, z: Float)
    external fun setScale(entityId: Int, x: Float, y: Float, z: Float)

    external fun getPosition(entityId: Int): FloatArray
    external fun getRotation(entityId: Int): FloatArray

    external fun addBoxCollider(entityId: Int, halfExtX: Float, halfExtY: Float, halfExtZ: Float, mass: Float)
}
