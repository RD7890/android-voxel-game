package com.mcme

import android.os.Bundle
import android.view.View
import android.view.WindowManager
import androidx.appcompat.app.AppCompatActivity
import com.mcme.databinding.ActivityGameBinding

class GameActivity : AppCompatActivity() {

    private lateinit var binding: ActivityGameBinding
    private lateinit var gameView: GameSurfaceView

    // Hotbar block types matching C++ BlockType enum:
    // GRASS=1, DIRT=2, STONE=3, WOOD=4, LEAVES=5, SAND=6
    private val hotbarBlocks = intArrayOf(1, 2, 3, 4, 5, 6)
    private var selectedSlot = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        makeFullscreen()

        binding = ActivityGameBinding.inflate(layoutInflater)
        setContentView(binding.root)

        gameView = binding.gameSurface

        // Standard controls
        binding.btnBack.setOnClickListener { finish() }
        binding.btnJump.setOnClickListener { NativeLib.onJump() }

        // Place selected block
        binding.btnPlace.setOnClickListener {
            gameView.queueEvent { NativeLib.placeBlock(hotbarBlocks[selectedSlot]) }
        }

        // Joystick movement
        binding.joystick.onMove = { rx, ry ->
            NativeLib.onJoystick(rx, -ry)
        }

        // Look + tap-to-break
        binding.lookZone.setOnTouchListener(gameView.lookTouchListener)

        // Hotbar slot selection
        val slots = listOf(
            binding.slot0, binding.slot1, binding.slot2,
            binding.slot3, binding.slot4, binding.slot5
        )
        slots.forEachIndexed { i, slot ->
            slot.isClickable = true
            slot.setOnClickListener { selectSlot(i, slots) }
        }
        selectSlot(0, slots)
    }

    private fun selectSlot(index: Int, slots: List<View>) {
        selectedSlot = index
        slots.forEachIndexed { i, slot ->
            slot.alpha  = if (i == index) 1.0f else 0.5f
            slot.scaleX = if (i == index) 1.15f else 1.0f
            slot.scaleY = if (i == index) 1.15f else 1.0f
        }
    }

    private fun makeFullscreen() {
        @Suppress("DEPRECATION")
        window.decorView.systemUiVisibility = (
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            or View.SYSTEM_UI_FLAG_FULLSCREEN
            or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
            or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
            or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
        )
    }

    override fun onResume() {
        super.onResume()
        makeFullscreen()
        gameView.onResume()
        NativeLib.onResume()
    }

    override fun onPause() {
        super.onPause()
        gameView.onPause()
        NativeLib.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        NativeLib.destroy()
    }
}
