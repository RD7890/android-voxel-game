package com.mcme

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class GameSurfaceView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null
) : GLSurfaceView(context, attrs) {

    private var lastLookX = 0f
    private var lastLookY = 0f

    val lookTouchListener = View.OnTouchListener { _, event ->
        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> {
                lastLookX = event.x
                lastLookY = event.y
            }
            MotionEvent.ACTION_MOVE -> {
                val dx = event.x - lastLookX
                val dy = event.y - lastLookY
                lastLookX = event.x
                lastLookY = event.y
                queueEvent { NativeLib.onLook(dx, dy) }
            }
        }
        true
    }

    init {
        setEGLContextClientVersion(3)
        setEGLConfigChooser(8, 8, 8, 8, 16, 0)
        setRenderer(MCMERenderer())
        renderMode = RENDERMODE_CONTINUOUSLY
    }

    private inner class MCMERenderer : Renderer {
        override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
            // width/height not available yet — called again in onSurfaceChanged
        }
        override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
            NativeLib.onSurfaceCreated(width, height)
            NativeLib.onSurfaceChanged(width, height)
        }
        override fun onDrawFrame(gl: GL10?) {
            NativeLib.onDrawFrame()
        }
    }
}
