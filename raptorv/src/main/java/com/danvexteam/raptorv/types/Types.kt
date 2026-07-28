package com.danvexteam.raptorv.types

data class Transform(
    var position: Vec3 = Vec3(),
    var rotation: Vec3 = Vec3(),
    var scale: Vec3 = Vec3(1f, 1f, 1f)
)

enum class ColliderType { BOX, SPHERE, CAPSULE }

data class RigidBodyDesc(
    val type: ColliderType = ColliderType.BOX,
    val halfExtent: Vec3 = Vec3(0.5f, 0.5f, 0.5f),
    val radius: Float = 0.5f,
    val halfHeight: Float = 0.5f,
    val mass: Float = 1.0f
)

enum class ProjectionType { PERSPECTIVE, ORTHOGRAPHIC }

data class CameraDesc(
    var projectionType: ProjectionType = ProjectionType.PERSPECTIVE,
    var fovDegrees: Float = 60f,
    var focalLength: Float = 0.0f,
    var nearPlane: Float = 0.1f,
    var farPlane: Float = 1000f,
    var focusDistance: Float = 5.0f,

    var position: Vec3 = Vec3(0f, 2f, 5f),
    var target: Vec3 = Vec3(0f, 0f, 0f),
    var up: Vec3 = Vec3(0f, 1f, 0f),

    var aperture: Float = 16.0f,
    var shutterSpeed: Float = 125.0f,
    var sensitivityISO: Float = 100.0f,

    var shiftX: Float = 0.0f,
    var shiftY: Float = 0.0f,

    var orthoLeft: Float = -10.0f,
    var orthoRight: Float = 10.0f,
    var orthoBottom: Float = -10.0f,
    var orthoTop: Float = 10.0f
)

enum class LightType { Directional, Point, Spot, Sun }

data class LightDesc(
    var type: LightType = LightType.Directional,
    var color: Vec3 = Vec3(1f, 1f, 1f),
    var intensity: Float = 100000f,
    var direction: Vec3 = Vec3(0f, -1f, 0f),
    var position: Vec3 = Vec3(0f, 0f, 0f),
    var falloffRadius: Float = 10f,
    var castShadows: Boolean = true,
    var enableContactShadows: Boolean = false,
    var innerConeAngle: Float = 0.5f,
    var outerConeAngle: Float = 0.6f,
    var shadowMapSize: Int = 2048,
    var shadowCascades: Int = 1,
    var shadowConstantBias: Float = 0.005f,
    var shadowNormalBias: Float = 0.4f,
    var sunAngularRadius: Float = 0.53f,
    var sunHaloSize: Float = 10.0f,
    var sunHaloFalloff: Float = 80.0f,
    var cascadeSplit1: Float = 0.1f,
    var cascadeSplit2: Float = 0.25f,
    var cascadeSplit3: Float = 0.50f
)

enum class ToneMapping { LINEAR, ACES, FILMIC }

enum class MinFilter { NEAREST, LINEAR, NEAREST_MIPMAP_NEAREST, LINEAR_MIPMAP_NEAREST, NEAREST_MIPMAP_LINEAR, LINEAR_MIPMAP_LINEAR }
enum class MagFilter { NEAREST, LINEAR }
enum class WrapMode { CLAMP_TO_EDGE, REPEAT, MIRRORED_REPEAT }
