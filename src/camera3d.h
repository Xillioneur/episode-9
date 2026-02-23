#ifndef CAMERA_3D_H
#define CAMERA_3D_H

#include "raylib.h"
#include "raymath.h"

class Camera3D_Custom {
public:
    Camera camera;
    Vector3 targetOffset;
    float rotationSpeed;
    float distance;
    
    float yaw;   // Horizontal rotation
    float pitch; // Vertical rotation

    Camera3D_Custom();
    void InitCamera(Vector3 targetPos);
    void Update(Vector3 targetPos);
    void SetTargetOffset(Vector3 offset);
    void SetDistance(float dist);
    
    Vector3 GetForward(); // Get camera forward vector (horizontal)
};

#endif // CAMERA_3D_H
