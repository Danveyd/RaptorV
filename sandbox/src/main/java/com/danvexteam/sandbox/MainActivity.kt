package com.danvexteam.sandbox

import android.os.Bundle
import android.os.Handler
import android.os.Looper
import androidx.appcompat.app.AppCompatActivity
import com.danvexteam.raptorv.Engine
import com.danvexteam.raptorv.Entity
import com.danvexteam.raptorv.RaptorView
import com.danvexteam.raptorv.types.CameraDesc
import com.danvexteam.raptorv.types.LightDesc
import com.danvexteam.raptorv.types.LightType
import com.danvexteam.raptorv.types.RenderSettings
import com.danvexteam.raptorv.types.Transform
import com.danvexteam.raptorv.types.Vec3
import com.danvexteam.raptorv.types.Vec4
import kotlin.math.cos
import kotlin.math.sin

class MainActivity : AppCompatActivity() {

    private lateinit var engine: Engine
    private var playerEntity: Entity? = null

    private var subRotationY = 0f
    private var angle = 0f

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val raptorView = findViewById<RaptorView>(R.id.raptorView)
        engine = Engine(this)
        raptorView.attachEngine(engine)


        val settings = RenderSettings(
            clearColor = Vec4(0.0f, 0.0f, 0.02f, 1f)
        )
        engine.setRenderSettings(settings)

        val scene = engine.createScene()
        engine.setActiveScene(scene)


        val sunDesc = LightDesc(
            type = LightType.Directional,
            color = Vec3(1f, 0.95f, 0.9f),
            intensity = 110000f,
            direction = Vec3(0.5f, -1f, -0.5f),
            castShadows = true
        )
        val sun = engine.createLight(sunDesc)

        val ambientDesc = LightDesc(
            type = LightType.Directional,
            color = Vec3(0.4f, 0.6f, 0.9f),
            intensity = 20000f,
            direction = Vec3(-0.5f, 1f, 0.5f),
            castShadows = false
        )
        val ambient = engine.createLight(ambientDesc)

        val camDesc = CameraDesc(
            position = Vec3(0f, 3f, 8f),
            target = Vec3(0f, 0f, 0f)
        )
        val camera = engine.createCamera(camDesc)
        camera.makeMain()

        Handler(Looper.getMainLooper()).postDelayed({
            val meshId = engine.loadMesh("atlantic_explorer_submarineglb.glb")
            playerEntity = scene.createEntity("Player")
            playerEntity?.attachMesh(meshId)
            playerEntity?.transform = Transform(position = Vec3(0f, 0f, 0f))
        }, 500)


        raptorView.onTick = { deltaTime ->
            subRotationY += deltaTime * 1.0f
            playerEntity?.setTransformRaw(
                0f, 0f, 0f,          // Position
                0f, subRotationY, 0f, // Rotation
                1f, 1f, 1f           // Scale
            )

            angle += deltaTime * 0.5f
            val camX = sin(angle.toDouble()).toFloat() * 12f
            val camZ = -10f + cos(angle.toDouble()).toFloat() * 12f

            camera.updateRaw(
                60f, 0.1f, 1000f,  // FOV, Near, Far
                camX, 3f, camZ,      // Position (X, Y, Z)
                0f, 0f, 0f,           // Target
                0f, 1f, 0f           // Up vector
            )
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        engine.close()
    }
}
