package com.danvexteam.raptorv

import com.danvexteam.raptorv.internal.RaptorNative
import com.danvexteam.raptorv.types.CameraDesc
import com.danvexteam.raptorv.types.ProjectionType
import com.danvexteam.raptorv.types.Vec3

class Camera internal constructor(private val engine: Engine, val id: Long) {

    val desc = CameraDesc()

    var position: Vec3
        get() = desc.position
        set(value) { desc.position = value; apply() }

    var target: Vec3
        get() = desc.target
        set(value) { desc.target = value; apply() }

    var up: Vec3
        get() = desc.up
        set(value) { desc.up = value; apply() }

    var fov: Float
        get() = desc.fovDegrees
        set(value) { desc.fovDegrees = value; apply() }

    var focalLength: Float
        get() = desc.focalLength
        set(value) { desc.focalLength = value; apply() }

    var nearPlane: Float
        get() = desc.nearPlane
        set(value) { desc.nearPlane = value; apply() }

    var farPlane: Float
        get() = desc.farPlane
        set(value) { desc.farPlane = value; apply() }

    var focusDistance: Float
        get() = desc.focusDistance
        set(value) { desc.focusDistance = value; apply() }

    fun setExposure(aperture: Float = 16.0f, shutterSpeed: Float = 125.0f, iso: Float = 100.0f) {
        desc.aperture = aperture
        desc.shutterSpeed = shutterSpeed
        desc.sensitivityISO = iso
        apply()
    }

    fun setLensShift(shiftX: Float, shiftY: Float) {
        desc.shiftX = shiftX
        desc.shiftY = shiftY
        apply()
    }

    fun makeMain() {
        RaptorNative.setMainCamera(engine.handle, id)
    }

    fun update(desc: CameraDesc) {
        this.desc.projectionType = desc.projectionType
        this.desc.fovDegrees = desc.fovDegrees
        this.desc.focalLength = desc.focalLength
        this.desc.nearPlane = desc.nearPlane
        this.desc.farPlane = desc.farPlane
        this.desc.focusDistance = desc.focusDistance
        this.desc.position = desc.position
        this.desc.target = desc.target
        this.desc.up = desc.up
        this.desc.aperture = desc.aperture
        this.desc.shutterSpeed = desc.shutterSpeed
        this.desc.sensitivityISO = desc.sensitivityISO
        this.desc.shiftX = desc.shiftX
        this.desc.shiftY = desc.shiftY
        apply()
    }

    fun lookAt(px: Float, py: Float, pz: Float, tx: Float, ty: Float, tz: Float) {
        desc.position.set(px, py, pz)
        desc.target.set(tx, ty, tz)
        apply()
    }

    fun apply() {
        RaptorNative.updateCamera(
            engine.handle, id,
            desc.projectionType.ordinal, desc.fovDegrees, desc.focalLength, desc.nearPlane, desc.farPlane, desc.focusDistance,
            desc.position.x, desc.position.y, desc.position.z,
            desc.target.x, desc.target.y, desc.target.z,
            desc.up.x, desc.up.y, desc.up.z,
            desc.aperture, desc.shutterSpeed, desc.sensitivityISO,
            desc.shiftX, desc.shiftY,
            desc.orthoLeft, desc.orthoRight, desc.orthoBottom, desc.orthoTop
        )
    }

    fun updateRaw(
        fov: Float, near: Float, far: Float,
        px: Float, py: Float, pz: Float,
        tx: Float, ty: Float, tz: Float,
        ux: Float, uy: Float, uz: Float,
        aperture: Float = 16.0f, shutterSpeed: Float = 125.0f, sensitivityISO: Float = 100.0f,
        focalLength: Float = 0.0f, focusDistance: Float = 5.0f
    ) {
        desc.fovDegrees = fov
        desc.nearPlane = near
        desc.farPlane = far
        desc.position.set(px, py, pz)
        desc.target.set(tx, ty, tz)
        desc.up.set(ux, uy, uz)
        desc.aperture = aperture
        desc.shutterSpeed = shutterSpeed
        desc.sensitivityISO = sensitivityISO
        desc.focalLength = focalLength
        desc.focusDistance = focusDistance
        apply()
    }
}
