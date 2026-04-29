package com.danvexteam.raptorv.types

data class Vec3(var x: Float = 0f, var y: Float = 0f, var z: Float = 0f)

data class Transform(
    var position: Vec3 = Vec3(),
    var rotation: Vec3 = Vec3(),
    var scale: Vec3 = Vec3(1f, 1f, 1f)
)

data class RigidBodyDesc(
    val halfExtent: Vec3 = Vec3(0.5f, 0.5f, 0.5f),
    val mass: Float = 1f
)

data class CameraDesc(
    var fovDegrees: Float = 60f,
    var nearPlane: Float = 0.1f,
    var farPlane: Float = 1000f,
    var position: Vec3 = Vec3(0f, 0f, 0f),
    var target: Vec3 = Vec3(0f, 0f, -1f),
    var up: Vec3 = Vec3(0f, 1f, 0f)
)

data class Vec4(var x: Float = 0f, var y: Float = 0f, var z: Float = 0f, var w: Float = 0f)

enum class LightType { Directional, Point, Spot }

data class LightDesc(
    var type: LightType = LightType.Directional,
    var color: Vec3 = Vec3(1f, 1f, 1f),
    var intensity: Float = 100000f,
    var direction: Vec3 = Vec3(0f, -1f, 0f),
    var position: Vec3 = Vec3(0f, 0f, 0f),
    var falloffRadius: Float = 10f,
    var castShadows: Boolean = true
)

data class RenderSettings(
    var bloom: Boolean = true,
    var ssao: Boolean = true,
    var fxaa: Boolean = true,
    var taa: Boolean = false,
    var clearColor: Vec4 = Vec4(0.0f, 0.0f, 0.02f, 1.0f)
)
