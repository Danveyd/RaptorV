package com.danvexteam.raptorv

import com.danvexteam.raptorv.internal.RaptorNative
import com.danvexteam.raptorv.types.CameraDesc

class Camera internal constructor(private val engine: Engine, val id: Long) {

    fun makeMain() {
        RaptorNative.setMainCamera(engine.handle, id)
    }

    fun update(desc: CameraDesc) {
        RaptorNative.updateCamera(
            engine.handle, id,
            desc.fovDegrees, desc.nearPlane, desc.farPlane,
            desc.position.x, desc.position.y, desc.position.z,
            desc.target.x, desc.target.y, desc.target.z,
            desc.up.x, desc.up.y, desc.up.z
        )
    }

    fun updateRaw(fov: Float, near: Float, far: Float,
                  px: Float, py: Float, pz: Float,
                  tx: Float, ty: Float, tz: Float,
                  ux: Float, uy: Float, uz: Float) {
        RaptorNative.updateCamera(engine.handle, id, fov, near, far, px, py, pz, tx, ty, tz, ux, uy, uz)
    }
}
