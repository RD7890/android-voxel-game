package com.mcme

object NativeLib {
    init { System.loadLibrary("mcme-engine") }

    external fun onSurfaceCreated(width: Int, height: Int)
    external fun onSurfaceChanged(width: Int, height: Int)
    external fun onDrawFrame()
    external fun onJoystick(rightAxis: Float, forwardAxis: Float)
    external fun onLook(dyaw: Float, dpitch: Float)
    external fun onJump()
    external fun onPause()
    external fun onResume()
    external fun setSensitivity(s: Float)
    external fun setRenderDistance(d: Int)
    external fun destroy()

    // v1.2 — Block interaction
    external fun breakBlock()
    external fun placeBlock(blockType: Int)
}
