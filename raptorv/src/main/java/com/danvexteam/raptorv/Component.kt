package com.danvexteam.raptorv

abstract class Component {
    lateinit var entity: Entity
        internal set

    open fun onCreate() {}

    open fun onUpdate(deltaTime: Float) {}

    open fun onDestroy() {}
}
