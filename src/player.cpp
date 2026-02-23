#include "player.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h" 
#include "camera3d.h" 
#include <algorithm> 

// Helper to interpolate angles correctly (handling wrap-around)
float LerpAngle(float start, float end, float amount) {
    float difference = end - start;
    while (difference < -180) difference += 360;
    while (difference > 180) difference -= 360;
    return start + difference * amount;
}

// Player constructor
Player::Player() {
    position = (Vector3){ 0.0f, 0.5f, 0.0f }; 
    velocity = (Vector3){ 0.0f, 0.0f, 0.0f };
    baseSpeed = 12.0f; 
    sprintSpeedMultiplier = 3.0f; 
    currentSpeed = baseSpeed;
    jumpStrength = 18.0f; 
    isGrounded = true;
    canDoubleJump = false;
    isGliding = false;

    dashPower = 30.0f; 
    dashCooldown = 1.0f; 
    dashTimer = 0.0f;
    dashEffectTimer = 0.0f;

    glideGravityMultiplier = 0.1f; 

    faithMeter = 100.0f; 
    maxFaith = 100.0f;    
    radius = 0.5f; 
    rotationY = 0.0f;

    isShieldActive = false;
    shieldTimer = 0.0f;
    shieldCooldown = 0.0f;
    shieldDuration = 3.0f;

    isSwinging = false;
    swingTimer = 0.0f;
}

// Update player state
void Player::Update(Camera3D_Custom& camera) { 
    float dt = GetFrameTime();
    Vector3 input = { 0.0f, 0.0f, 0.0f };

    // Update timers
    if (dashTimer > 0) dashTimer -= dt;
    if (dashEffectTimer > 0) dashEffectTimer -= dt;
    if (shieldTimer > 0) {
        shieldTimer -= dt;
        if (shieldTimer <= 0) isShieldActive = false;
    }
    if (shieldCooldown > 0) shieldCooldown -= dt;
    if (swingTimer > 0) {
        swingTimer -= dt;
        if (swingTimer <= 0) isSwinging = false;
    }

    // Handle Faith Shield (Q)
    if (IsKeyPressed(KEY_Q) && shieldCooldown <= 0) {
        isShieldActive = true;
        shieldTimer = shieldDuration;
        shieldCooldown = 8.0f; 
    }

    // Handle sprinting
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        currentSpeed = baseSpeed * sprintSpeedMultiplier;
    } else {
        currentSpeed = baseSpeed;
    }

    // Get camera vectors
    Vector3 cameraForward = camera.GetForward();
    Vector3 cameraRight = Vector3CrossProduct(cameraForward, (Vector3){0.0f, 1.0f, 0.0f});
    cameraRight = Vector3Normalize(cameraRight);

    // Handle input (WASD) relative to camera
    if (IsKeyDown(KEY_W)) input = Vector3Add(input, cameraForward);
    if (IsKeyDown(KEY_S)) input = Vector3Subtract(input, cameraForward);
    if (IsKeyDown(KEY_A)) input = Vector3Subtract(input, cameraRight);
    if (IsKeyDown(KEY_D)) input = Vector3Add(input, cameraRight);

    // Normalize input
    bool isMoving = Vector3Length(input) > 0.1f;
    if (isMoving) {
        input = Vector3Normalize(input);
    }

    // --- Rotation Logic ---
    // If attacking (Swinging staff or firing beam), face camera forward
    if (isSwinging || IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        float targetRotation = atan2f(cameraForward.x, cameraForward.z) * RAD2DEG;
        rotationY = LerpAngle(rotationY, targetRotation, 20.0f * dt); // Snappy rotation for combat
    }
    // Else if moving, face movement direction
    else if (isMoving) {
        float targetRotation = atan2f(input.x, input.z) * RAD2DEG;
        rotationY = LerpAngle(rotationY, targetRotation, 10.0f * dt); // Smooth rotation for traversal
    }

    // Horizontal Movement
    Vector3 targetHorizontalVelocity = Vector3Scale(input, currentSpeed);
    if (dashEffectTimer <= 0.0f) {
        velocity.x = Lerp(velocity.x, targetHorizontalVelocity.x, 10.0f * dt);
        velocity.z = Lerp(velocity.z, targetHorizontalVelocity.z, 10.0f * dt);

        if (!isMoving) { 
            velocity.x = Lerp(velocity.x, 0.0f, 15.0f * dt); 
            velocity.z = Lerp(velocity.z, 0.0f, 15.0f * dt);
        }
    }

    // Vertical Physics
    float gravity = 22.0f; 
    if (!isGrounded) {
        float curGrav = gravity;
        if (velocity.y > 0 && !IsKeyDown(KEY_SPACE)) curGrav *= 1.5f; 

        if (IsKeyDown(KEY_SPACE) && velocity.y < 0) { 
            isGliding = true;
            velocity.y -= curGrav * dt * glideGravityMultiplier;
            velocity.x = Lerp(velocity.x, targetHorizontalVelocity.x * 1.5f, 5.0f * dt);
            velocity.z = Lerp(velocity.z, targetHorizontalVelocity.z * 1.5f, 5.0f * dt);
        } else {
            isGliding = false;
            velocity.y -= curGrav * dt; 
        }
        if (velocity.y < -30.0f) velocity.y = -30.0f;
    } else {
        velocity.y = 0.0f; 
        canDoubleJump = true; 
    }

    if (IsKeyPressed(KEY_SPACE)) {
        if (isGrounded) {
            velocity.y = jumpStrength;
            isGrounded = false;
            canDoubleJump = true; 
        } else if (canDoubleJump && !isGliding) { 
            velocity.y = jumpStrength * 0.9f; 
            canDoubleJump = false; 
            isGliding = false; 
        }
    }

    // Dash
    if (IsKeyPressed(KEY_C) && dashTimer <= 0) {
        Vector3 dashDir = isMoving ? input : cameraForward;
        velocity.x = dashDir.x * dashPower;
        velocity.z = dashDir.z * dashPower;
        dashTimer = dashCooldown;
        dashEffectTimer = 0.2f; 
    }

    position = Vector3Add(position, Vector3Scale(velocity, dt));

    if (position.y < 0.5f) { 
        position.y = 0.5f;
        if (!isGrounded) { isGrounded = true; canDoubleJump = true; isGliding = false; }
    } else {
        isGrounded = false;
    }
}

// Draw player
void Player::Draw(Camera3D_Custom& camera) {
    // We'll use rlgl to rotate the entire model based on rotationY
    rlPushMatrix();
    rlTranslatef(position.x, position.y, position.z);
    rlRotatef(rotationY, 0, 1, 0);

    Color robeColor = DARKBLUE;
    Color skinColor = BEIGE;
    Color goldTrim = GOLD;

    // --- Body ---
    DrawCylinder({0, -0.5f, 0}, 0.6f, 0.6f, 0.8f, 8, robeColor);
    DrawCylinderWires({0, -0.5f, 0}, 0.6f, 0.6f, 0.8f, 8, DARKGRAY);
    DrawCylinder({0, 0.3f, 0}, 0.5f, 0.5f, 0.7f, 8, robeColor);
    DrawCylinder({0, 0.25f, 0}, 0.52f, 0.52f, 0.1f, 8, goldTrim);

    // --- Head ---
    DrawSphere({0, 1.1f, 0}, 0.25f, skinColor);
    DrawSphere({0, 1.15f, 0}, 0.28f, robeColor); // Hood
    
    // --- Eyes (Facing forward in local space) ---
    DrawSphere({0.1f, 1.15f, 0.2f}, 0.05f, BLACK);
    DrawSphere({-0.1f, 1.15f, 0.2f}, 0.05f, BLACK);

    // --- Arms ---
    // In local space, forward is +Z
    Vector3 leftShoulder = {-0.3f, 0.9f, 0};
    Vector3 rightShoulder = {0.3f, 0.9f, 0};
    DrawSphere(leftShoulder, 0.15f, robeColor);
    DrawSphere(rightShoulder, 0.15f, robeColor);
    
    Vector3 leftHand = {-0.3f, 0.5f, 0.2f};
    DrawCylinderEx(leftShoulder, leftHand, 0.12f, 0.1f, 6, robeColor);
    DrawSphere(leftHand, 0.1f, skinColor);

    Vector3 rightHand = {0.3f, 0.6f, 0.3f};
    if (isSwinging) rightHand.y += 0.2f;
    DrawCylinderEx(rightShoulder, rightHand, 0.12f, 0.1f, 6, robeColor);
    DrawSphere(rightHand, 0.1f, skinColor);

    // --- Halo ---
    DrawCylinder({0, 1.5f, 0}, 0.2f, 0.2f, 0.02f, 16, Fade(GOLD, 0.3f));
    DrawCylinderWires({0, 1.5f, 0}, 0.2f, 0.2f, 0.02f, 16, Fade(GOLD, 0.8f));

    // --- The Glorious Staff ---
    rlPushMatrix();
    rlTranslatef(rightHand.x, rightHand.y, rightHand.z);
    
    if (isSwinging) {
        float t = 1.0f - (swingTimer / 0.3f); 
        float sweepAngle = sinf(t * PI) * 120.0f;
        rlRotatef(sweepAngle, 0, 1, 0); 
        rlRotatef(-30.0f, 1, 0, 0); 
    } else {
        rlRotatef(10.0f, 1, 0, 0); 
    }

    DrawCylinder({0, -0.5f, 0}, 0.04f, 0.04f, 2.0f, 8, DARKBROWN);
    DrawSphere({0, 1.5f, 0}, 0.15f, GOLD);
    DrawSphereWires({0, 1.5f, 0}, 0.18f, 8, 8, Fade(WHITE, 0.8f));
    DrawCube({0, 1.7f, 0}, 0.05f, 0.25f, 0.05f, WHITE);
    DrawCube({0, 1.75f, 0}, 0.2f, 0.05f, 0.05f, WHITE);
    rlPopMatrix();

    rlPopMatrix(); // End character rotation

    // --- Faith Shield ---
    if (isShieldActive) {
        DrawSphereWires(position, 1.2f, 6, 8, Fade(SKYBLUE, 0.6f));
        DrawSphere(position, 1.15f, Fade(BLUE, 0.15f));
    }
}

void Player::SwingSword() {
    isSwinging = true;
    swingTimer = 0.3f; 
}

void Player::SetPosition(Vector3 pos) { position = pos; }
Vector3 Player::GetPosition() { return position; }

void Player::TakeDamage(float amount) {
    if (isShieldActive) return; 
    faithMeter = std::clamp(faithMeter - amount, 0.0f, maxFaith);
}

void Player::Heal(float amount) {
    faithMeter = std::clamp(faithMeter + amount, 0.0f, maxFaith);
}
