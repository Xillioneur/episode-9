#include "camera3d.h"
#include "raylib.h"
#include "raymath.h"

// Constructor
Camera3D_Custom::Camera3D_Custom() {
    camera = { 0 };
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 75.0f;                                
    camera.projection = CAMERA_PERSPECTIVE;

    targetOffset = (Vector3){ 0.0f, 2.0f, 0.0f };
    rotationSpeed = 0.15f; 
    distance = 6.0f;
    
    yaw = 0.0f;
    pitch = 20.0f; 
}

// Initialize camera
void Camera3D_Custom::InitCamera(Vector3 targetPos) {
    camera.target = Vector3Add(targetPos, targetOffset);
    // Initial position will be set in the first Update call
}

// Update camera position and target using Yaw/Pitch
void Camera3D_Custom::Update(Vector3 targetPos) {
    // Update camera target to follow the player with an offset
    camera.target = Vector3Add(targetPos, targetOffset);

    // Camera rotation based on mouse input
    Vector2 mouseDelta = GetMouseDelta();
    
    yaw -= mouseDelta.x * rotationSpeed;
    pitch += mouseDelta.y * rotationSpeed;

    // Clamp pitch
    if (pitch > 80.0f) pitch = 80.0f;
    if (pitch < -40.0f) pitch = -40.0f; 

    // Calculate new position using spherical coordinates
    // We use yaw and pitch to determine the position relative to the target
    float horizontalDist = distance * cosf(pitch * DEG2RAD);
    float verticalDist = distance * sinf(pitch * DEG2RAD);

    float theta = yaw * DEG2RAD;
    
    camera.position.x = camera.target.x + horizontalDist * sinf(theta);
    camera.position.z = camera.target.z + horizontalDist * cosf(theta);
    camera.position.y = camera.target.y + verticalDist;
}

// Set target offset
void Camera3D_Custom::SetTargetOffset(Vector3 offset) {
    targetOffset = offset;
}

// Set distance from target
void Camera3D_Custom::SetDistance(float dist) {
    distance = dist;
}

Vector3 Camera3D_Custom::GetForward() {
    Vector3 forward = Vector3Subtract(camera.target, camera.position);
    forward.y = 0;
    return Vector3Normalize(forward);
}
