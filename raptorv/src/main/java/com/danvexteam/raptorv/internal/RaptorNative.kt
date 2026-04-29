package com.danvexteam.raptorv.internal

import android.content.res.AssetManager
import android.view.Surface

internal object RaptorNative {
    init { System.loadLibrary("raptorv") }

    @JvmStatic external fun createEngine(assetManager: AssetManager): Long
    @JvmStatic external fun destroyEngine(engineHandle: Long)
    @JvmStatic external fun onSurfaceCreated(engineHandle: Long, surface: Surface)
    @JvmStatic external fun onSurfaceChanged(engineHandle: Long, width: Int, height: Int)
    @JvmStatic external fun onSurfaceDestroyed(engineHandle: Long)
    @JvmStatic external fun tick(engineHandle: Long, dt: Float)

    @JvmStatic external fun createScene(engineHandle: Long): Long
    @JvmStatic external fun setActiveScene(engineHandle: Long, sceneHandle: Long)
    @JvmStatic external fun destroyScene(sceneHandle: Long)

    @JvmStatic external fun createEntity(sceneHandle: Long, name: String): Long
    @JvmStatic external fun setTransform(sceneHandle: Long, entityId: Long, px: Float, py: Float, pz: Float, rx: Float, ry: Float, rz: Float, sx: Float, sy: Float, sz: Float)
    @JvmStatic external fun getTransformRotation(sceneHandle: Long, entityId: Long): FloatArray

    @JvmStatic external fun loadMesh(engineHandle: Long, path: String): Long
    @JvmStatic external fun attachMesh(sceneHandle: Long, entityId: Long, meshHandle: Long)
    @JvmStatic external fun attachRigidBody(sceneHandle: Long, entityId: Long, hx: Float, hy: Float, hz: Float, mass: Float)

    @JvmStatic external fun createCamera(engineHandle: Long, fov: Float, near: Float, far: Float,
                                         px: Float, py: Float, pz: Float, tx: Float, ty: Float, tz: Float,
                                         ux: Float, uy: Float, uz: Float): Long
    @JvmStatic external fun setMainCamera(engineHandle: Long, cameraId: Long)
    @JvmStatic external fun updateCamera(engineHandle: Long, cameraId: Long, fov: Float, near: Float, far: Float,
                                         px: Float, py: Float, pz: Float, tx: Float, ty: Float, tz: Float,
                                         ux: Float, uy: Float, uz: Float)

    @JvmStatic external fun setRenderSettings(engineHandle: Long, bloom: Boolean, ssao: Boolean, fxaa: Boolean, taa: Boolean, cr: Float, cg: Float, cb: Float, ca: Float)

    @JvmStatic external fun createLight(engineHandle: Long, type: Int, cx: Float, cy: Float, cz: Float, intensity: Float, dx: Float, dy: Float, dz: Float, px: Float, py: Float, pz: Float, falloff: Float, shadows: Boolean): Long
    @JvmStatic external fun updateLight(engineHandle: Long, lightId: Long, type: Int, cx: Float, cy: Float, cz: Float, intensity: Float, dx: Float, dy: Float, dz: Float, px: Float, py: Float, pz: Float, falloff: Float, shadows: Boolean)
}
