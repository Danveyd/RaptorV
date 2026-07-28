package com.danvexteam.sandbox

import android.os.Bundle
import android.os.Handler
import android.os.Looper
import androidx.appcompat.app.AppCompatActivity
import com.danvexteam.raptorv.Camera
import com.danvexteam.raptorv.Component
import com.danvexteam.raptorv.Engine
import com.danvexteam.raptorv.Entity
import com.danvexteam.raptorv.RaptorView
import com.danvexteam.raptorv.pipeline.LumenConfig
import com.danvexteam.raptorv.pipeline.enableLumenGI
import com.danvexteam.raptorv.types.*
import kotlin.math.cos
import kotlin.math.sin

class StreetCameraController(
    private val camera: Camera,
    private val helmetEntity: Entity?
) : Component() {
    private var angle = 0f

    override fun onUpdate(deltaTime: Float) {
        angle += deltaTime * 0.2f

        val camX = sin(angle.toDouble()).toFloat() * 2.0f
        val camZ = cos(angle.toDouble()).toFloat() * 2.0f
        val camY = 1.6f

        camera.updateRaw(
            fov = 60f, near = 0.1f, far = 1000f,
            px = camX + 9f, py = camY, pz = camZ,
            tx = 9f, ty = 1.0f, tz = 0f,
            ux = 0f, uy = 1f, uz = 0f
        )

        helmetEntity?.setTransformRaw(
            px = 9f, py = 1.0f, pz = 0f,
            rx = 0f, ry = angle * 0.8f, rz = 0f,
            sx = 0.35f, sy = 0.35f, sz = 0.35f
        )
    }
}

class MainActivity : AppCompatActivity() {

    private lateinit var engine: Engine

    private var frameCount = 0
    private var lastFpsTimestamp = System.currentTimeMillis()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val raptorView = findViewById<RaptorView>(R.id.raptorView)
        engine = Engine(this)
        raptorView.attachEngine(engine)

        raptorView.onTick = { deltaTime ->
            frameCount++
            val now = System.currentTimeMillis()
            if (now - lastFpsTimestamp >= 1000) {
                val fps = frameCount
                frameCount = 0
                lastFpsTimestamp = now
                runOnUiThread { title = "Raptor V Next-Gen - $fps FPS" }
                android.util.Log.i("RaptorV_FPS", "Real FPS: $fps | dt: ${(deltaTime * 1000).toInt()}ms")
            }
        }

        engine.enableLumenGI(
            LumenConfig(
                giQuality = QualityLevel.HIGH,
                enableSSR = true,
                ssrThickness = 0.2f,
                enableSSCT = true,
                enableFSR = true,
                fsrRenderScale = 0.45f,
                fsrSharpness = 0.8f
            )
        )


        engine.setColorGrading(
            ColorGradingOptions(
                toneMapping = ToneMappingMode.ACES,
                exposure = 0.85f,
                contrast = 1.22f,
                vibrance = 1.15f,
                whiteBalanceTemp = 0.05f
            )
        )

        engine.setBloom(
            BloomOptions(
                enabled = true,
                strength = 0.08f,
                lensFlare = true
            )
        )

        engine.setShadowType(ShadowType.PCF)

        val scene = engine.createScene()
        engine.setActiveScene(scene)

        try {
            scene.setEnvironment(
                this,
                iblPath = "default_env_ibl.ktx",
                skyboxPath = "default_env_skybox.ktx"
            )
            scene.setIblIntensity(35000f)
        } catch (e: Exception) {
            e.printStackTrace()
        }

        val sunDesc = LightDesc(
            type = LightType.Sun,
            color = Vec3(1.0f, 0.95f, 0.85f),
            intensity = 110000f,
            direction = Vec3(-0.6f, -0.8f, -0.4f).normalize(),
            castShadows = true,
            shadowCascades = 1,
            shadowMapSize = 2048
        )
        engine.createLight(sunDesc)

        val camera = engine.createCamera(
            CameraDesc(
                position = Vec3(9f, 1.6f, 2.0f),
                target = Vec3(9f, 1.0f, 0f)
            )
        )
        camera.makeMain()

        Handler(Looper.getMainLooper()).postDelayed({
            val streetMeshId = engine.loadMesh("after_the_rain..._-_vr__sound.glb")
            if (streetMeshId != 0L) {
                val streetEntity = scene.createEntity("StreetLevel")
                streetEntity.attachMesh(streetMeshId)
                streetEntity.transform = Transform(
                    position = Vec3(0f, 0f, 0f),
                    scale = Vec3(1.0f, 1.0f, 1.0f)
                )
                streetEntity.receiveShadows = true
                streetEntity.castShadows = false
            }

            var helmetMeshId = engine.loadMesh("DamagedHelmet.glb")
            if (helmetMeshId == 0L) {
                helmetMeshId = engine.loadMesh("ClearCoatCarPaint.glb")
            }

            var heroHelmet: Entity? = null
            if (helmetMeshId != 0L) {
                heroHelmet = scene.createEntity("HeroHelmet")
                heroHelmet.attachMesh(helmetMeshId)
                heroHelmet.transform = Transform(
                    position = Vec3(9f, 1.0f, 0f),
                    scale = Vec3(0.35f, 0.35f, 0.35f)
                )
                heroHelmet.castShadows = true
                heroHelmet.receiveShadows = true
            }

            heroHelmet?.material?.emissiveColor = Vec4(5.0f, 0.0f, 0.0f, 1.0f)

            val controllerEntity = scene.createEntity("CameraController")
            controllerEntity.addScript(StreetCameraController(camera, heroHelmet))
        }, 500)
    }

    override fun onDestroy() {
        super.onDestroy()
        engine.close()
    }
}

