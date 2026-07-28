package com.danvexteam.raptorv.types

import kotlin.math.*

object MathUtils {
    const val PI = kotlin.math.PI.toFloat()
    const val TWO_PI = PI * 2f
    const val HALF_PI = PI / 2f
    const val DEG2RAD = PI / 180f
    const val RAD2DEG = 180f / PI
    const val EPSILON = 1e-6f

    fun degToRad(deg: Float): Float = deg * DEG2RAD
    fun radToDeg(rad: Float): Float = rad * RAD2DEG

    fun clamp(v: Float, min: Float, max: Float): Float = v.coerceIn(min, max)
    fun lerp(a: Float, b: Float, t: Float): Float = a + (b - a) * clamp(t, 0f, 1f)

    fun smoothstep(edge0: Float, edge1: Float, x: Float): Float {
        val t = clamp((x - edge0) / (edge1 - edge0), 0f, 1f)
        return t * t * (3f - 2f * t)
    }
}

data class Vec2(var x: Float = 0f, var y: Float = 0f) {
    fun set(x: Float, y: Float): Vec2 { this.x = x; this.y = y; return this }

    operator fun plus(v: Vec2) = Vec2(x + v.x, y + v.y)
    operator fun minus(v: Vec2) = Vec2(x - v.x, y - v.y)
    operator fun times(s: Float) = Vec2(x * s, y * s)
    operator fun times(v: Vec2) = Vec2(x * v.x, y * v.y)
    operator fun div(s: Float) = Vec2(x / s, y / s)
    operator fun unaryMinus() = Vec2(-x, -y)

    fun lengthSq() = x * x + y * y
    fun length() = sqrt(lengthSq())

    fun normalize(): Vec2 {
        val len = length()
        return if (len > MathUtils.EPSILON) Vec2(x / len, y / len) else Vec2()
    }

    fun dot(v: Vec2) = x * v.x + y * v.y

    companion object {
        val ZERO get() = Vec2(0f, 0f)
        val ONE get() = Vec2(1f, 1f)
        fun lerp(a: Vec2, b: Vec2, t: Float) = Vec2(
            MathUtils.lerp(a.x, b.x, t),
            MathUtils.lerp(a.y, b.y, t)
        )
    }
}

data class Vec3(var x: Float = 0f, var y: Float = 0f, var z: Float = 0f) {
    fun set(x: Float, y: Float, z: Float): Vec3 {
        this.x = x; this.y = y; this.z = z
        return this
    }

    operator fun plus(v: Vec3) = Vec3(x + v.x, y + v.y, z + v.z)
    operator fun minus(v: Vec3) = Vec3(x - v.x, y - v.y, z - v.z)
    operator fun times(s: Float) = Vec3(x * s, y * s, z * s)
    operator fun times(v: Vec3) = Vec3(x * v.x, y * v.y, z * v.z)
    operator fun div(s: Float) = Vec3(x / s, y / s, z / s)
    operator fun unaryMinus() = Vec3(-x, -y, -z)

    fun lengthSq() = x * x + y * y + z * z
    fun length() = sqrt(lengthSq())

    fun distanceSq(v: Vec3): Float {
        val dx = x - v.x; val dy = y - v.y; val dz = z - v.z
        return dx * dx + dy * dy + dz * dz
    }
    fun distance(v: Vec3) = sqrt(distanceSq(v))

    fun normalize(): Vec3 {
        val len = length()
        return if (len > MathUtils.EPSILON) Vec3(x / len, y / len, z / len) else Vec3()
    }

    fun dot(v: Vec3) = x * v.x + y * v.y + z * v.z

    fun cross(v: Vec3) = Vec3(
        y * v.z - z * v.y,
        z * v.x - x * v.z,
        x * v.y - y * v.x
    )

    fun reflect(normal: Vec3): Vec3 {
        val d = this.dot(normal)
        return this - (normal * (2f * d))
    }

    fun toFloatArray(): FloatArray = floatArrayOf(x, y, z)

    companion object {
        val ZERO get() = Vec3(0f, 0f, 0f)
        val ONE get() = Vec3(1f, 1f, 1f)
        val UP get() = Vec3(0f, 1f, 0f)
        val DOWN get() = Vec3(0f, -1f, 0f)
        val FORWARD get() = Vec3(0f, 0f, -1f)
        val BACK get() = Vec3(0f, 0f, 1f)
        val RIGHT get() = Vec3(1f, 0f, 0f)
        val LEFT get() = Vec3(-1f, 0f, 0f)

        fun lerp(a: Vec3, b: Vec3, t: Float) = Vec3(
            MathUtils.lerp(a.x, b.x, t),
            MathUtils.lerp(a.y, b.y, t),
            MathUtils.lerp(a.z, b.z, t)
        )
    }
}

data class Vec4(var x: Float = 0f, var y: Float = 0f, var z: Float = 0f, var w: Float = 0f) {
    fun set(x: Float, y: Float, z: Float, w: Float): Vec4 {
        this.x = x; this.y = y; this.z = z; this.w = w
        return this
    }

    operator fun plus(v: Vec4) = Vec4(x + v.x, y + v.y, z + v.z, w + v.w)
    operator fun minus(v: Vec4) = Vec4(x - v.x, y - v.y, z - v.z, w - v.w)
    operator fun times(s: Float) = Vec4(x * s, y * s, z * s, w * s)
    operator fun div(s: Float) = Vec4(x / s, y / s, z / s, w / s)

    fun dot(v: Vec4) = x * v.x + y * v.y + z * v.z + w * v.w

    fun toFloatArray(): FloatArray = floatArrayOf(x, y, z, w)

    companion object {
        val WHITE get() = Vec4(1f, 1f, 1f, 1f)
        val BLACK get() = Vec4(0f, 0f, 0f, 1f)
        val RED get() = Vec4(1f, 0f, 0f, 1f)
        val GREEN get() = Vec4(0f, 1f, 0f, 1f)
        val BLUE get() = Vec4(0f, 0f, 1f, 1f)
        val CLEAR get() = Vec4(0f, 0f, 0f, 0f)

        fun fromHex(hex: Long, alpha: Float = 1.0f): Vec4 {
            val r = ((hex shr 16) and 0xFF) / 255.0f
            val g = ((hex shr 8) and 0xFF) / 255.0f
            val b = (hex and 0xFF) / 255.0f
            return Vec4(r, g, b, alpha)
        }
    }
}

data class Quat(var x: Float = 0f, var y: Float = 0f, var z: Float = 0f, var w: Float = 1f) {

    fun normalize(): Quat {
        val len = sqrt(x * x + y * y + z * z + w * w)
        return if (len > MathUtils.EPSILON) {
            Quat(x / len, y / len, z / len, w / len)
        } else IDENTITY
    }

    fun conjugate(): Quat = Quat(-x, -y, -z, w)

    operator fun times(q: Quat): Quat = Quat(
        w * q.x + x * q.w + y * q.z - z * q.y,
        w * q.y - x * q.z + y * q.w + z * q.x,
        w * q.z + x * q.y - y * q.x + z * q.w,
        w * q.w - x * q.x - y * q.y - z * q.z
    )

    operator fun times(v: Vec3): Vec3 {
        val u = Vec3(x, y, z)
        val s = w
        return u * (2.0f * u.dot(v)) + v * (s * s - u.dot(u)) + u.cross(v) * (2.0f * s)
    }

    fun toEuler(): Vec3 {
        val sinr_cosp = 2f * (w * x + y * z)
        val cosr_cosp = 1f - 2f * (x * x + y * y)
        val roll = atan2(sinr_cosp, cosr_cosp)

        val sinp = 2f * (w * y - z * x)
        val pitch = if (abs(sinp) >= 1f) sign(sinp) * MathUtils.HALF_PI else asin(sinp)

        val siny_cosp = 2f * (w * z + x * y)
        val cosy_cosp = 1f - 2f * (y * y + z * z)
        val yaw = atan2(siny_cosp, cosy_cosp)

        return Vec3(
            MathUtils.radToDeg(roll),
            MathUtils.radToDeg(pitch),
            MathUtils.radToDeg(yaw)
        )
    }

    companion object {
        val IDENTITY get() = Quat(0f, 0f, 0f, 1f)

        fun fromEuler(pitchDeg: Float, yawDeg: Float, rollDeg: Float): Quat {
            val p = MathUtils.degToRad(pitchDeg) / 2f
            val y = MathUtils.degToRad(yawDeg) / 2f
            val r = MathUtils.degToRad(rollDeg) / 2f

            val c1 = cos(y); val s1 = sin(y)
            val c2 = cos(p); val s2 = sin(p)
            val c3 = cos(r); val s3 = sin(r)

            return Quat(
                x = s1 * c2 * c3 + c1 * s2 * s3,
                y = c1 * s2 * c3 - s1 * c2 * s3,
                z = c1 * c2 * s3 - s1 * s2 * c3,
                w = c1 * c2 * c3 + s1 * s2 * s3
            ).normalize()
        }

        fun fromAxisAngle(axis: Vec3, angleDeg: Float): Quat {
            val rad = MathUtils.degToRad(angleDeg) / 2f
            val normAxis = axis.normalize()
            val s = sin(rad)
            return Quat(normAxis.x * s, normAxis.y * s, normAxis.z * s, cos(rad)).normalize()
        }

        fun slerp(q1: Quat, q2: Quat, t: Float): Quat {
            var cosHalfTheta = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w
            var targetQ = q2

            if (cosHalfTheta < 0f) {
                targetQ = Quat(-q2.x, -q2.y, -q2.z, -q2.w)
                cosHalfTheta = -cosHalfTheta
            }

            if (abs(cosHalfTheta) >= 1.0f) return q1

            val halfTheta = acos(cosHalfTheta)
            val sinHalfTheta = sqrt(1.0f - cosHalfTheta * cosHalfTheta)

            if (abs(sinHalfTheta) < 0.001f) {
                return Quat(
                    q1.x * (1f - t) + targetQ.x * t,
                    q1.y * (1f - t) + targetQ.y * t,
                    q1.z * (1f - t) + targetQ.z * t,
                    q1.w * (1f - t) + targetQ.w * t
                ).normalize()
            }

            val ratioA = sin((1f - t) * halfTheta) / sinHalfTheta
            val ratioB = sin(t * halfTheta) / sinHalfTheta

            return Quat(
                q1.x * ratioA + targetQ.x * ratioB,
                q1.y * ratioA + targetQ.y * ratioB,
                q1.z * ratioA + targetQ.z * ratioB,
                q1.w * ratioA + targetQ.w * ratioB
            )
        }
    }
}

class Mat4(val m: FloatArray = FloatArray(16)) {

    init {
        if (m.all { it == 0f }) setIdentity()
    }

    fun setIdentity(): Mat4 {
        for (i in 0..15) m[i] = 0f
        m[0] = 1f; m[5] = 1f; m[10] = 1f; m[15] = 1f
        return this
    }

    operator fun times(other: Mat4): Mat4 {
        val res = Mat4(FloatArray(16))
        for (r in 0..3) {
            for (c in 0..3) {
                var sum = 0f
                for (i in 0..3) {
                    sum += m[r + i * 4] * other.m[i + c * 4]
                }
                res.m[r + c * 4] = sum
            }
        }
        return res
    }

    fun transformPoint(v: Vec3): Vec3 {
        val x = m[0] * v.x + m[4] * v.y + m[8]  * v.z + m[12]
        val y = m[1] * v.x + m[5] * v.y + m[9]  * v.z + m[13]
        val z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14]
        val w = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15]
        return if (w != 0f && w != 1f) Vec3(x / w, y / w, z / w) else Vec3(x, y, z)
    }

    companion object {
        fun identity() = Mat4().setIdentity()

        fun translation(v: Vec3): Mat4 {
            val res = identity()
            res.m[12] = v.x; res.m[13] = v.y; res.m[14] = v.z
            return res
        }

        fun scaling(v: Vec3): Mat4 {
            val res = identity()
            res.m[0] = v.x; res.m[5] = v.y; res.m[10] = v.z
            return res
        }

        fun rotation(q: Quat): Mat4 {
            val res = identity()
            val normQ = q.normalize()
            val x = normQ.x; val y = normQ.y; val z = normQ.z; val w = normQ.w

            res.m[0] = 1f - 2f * y * y - 2f * z * z
            res.m[1] = 2f * x * y + 2f * w * z
            res.m[2] = 2f * x * z - 2f * w * y

            res.m[4] = 2f * x * y - 2f * w * z
            res.m[5] = 1f - 2f * x * x - 2f * z * z
            res.m[6] = 2f * y * z + 2f * w * x

            res.m[8] = 2f * x * z + 2f * w * y
            res.m[9] = 2f * y * z - 2f * w * x
            res.m[10] = 1f - 2f * x * x - 2f * y * y

            return res
        }
    }
}
