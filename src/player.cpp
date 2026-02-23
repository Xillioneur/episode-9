#include "player.h"
#include "raylib.h"
#include "raymath.h"
#include "camera3d.h" // Include custom camera header
#include <algorithm> // For std::clamp

// Player constructor
Player::Player() {
    position = (Vector3){ 0.0f, 0.5f, 0.0f }; // Start on the ground
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

    isShieldActive = false;
    shieldTimer = 0.0f;
    shieldCooldown = 0.0f;
    shieldDuration = 3.0f;
}

// Update player state
void Player::Update(Camera3D_Custom& camera) { 
    float dt = GetFrameTime();
    Vector3 input = { 0.0f, 0.0f, 0.0f };

    if (dashTimer > 0) dashTimer -= dt;
    if (dashEffectTimer > 0) dashEffectTimer -= dt;
    if (shieldTimer > 0) {
        shieldTimer -= dt;
        if (shieldTimer <= 0) isShieldActive = false;
    }
    if (shieldCooldown > 0) shieldCooldown -= dt;

    if (IsKeyPressed(KEY_Q) && shieldCooldown <= 0) {
        isShieldActive = true;
        shieldTimer = shieldDuration;
        shieldCooldown = 8.0f; 
    }

    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        currentSpeed = baseSpeed * sprintSpeedMultiplier;
    } else {
        currentSpeed = baseSpeed;
    }

    Vector3 cameraForward = camera.GetForward();
    Vector3 cameraRight = Vector3CrossProduct(cameraForward, (Vector3){0.0f, 1.0f, 0.0f});
    cameraRight = Vector3Normalize(cameraRight);

    if (IsKeyDown(KEY_W)) input = Vector3Add(input, cameraForward);
    if (IsKeyDown(KEY_S)) input = Vector3Subtract(input, cameraForward);
    if (IsKeyDown(KEY_A)) input = Vector3Subtract(input, cameraRight);
    if (IsKeyDown(KEY_D)) input = Vector3Add(input, cameraRight);

    if (Vector3Length(input) > 0.1f) {
        input = Vector3Normalize(input);
    }

    Vector3 targetHorizontalVelocity = Vector3Scale(input, currentSpeed);
    if (dashEffectTimer <= 0.0f) {
        velocity.x = Lerp(velocity.x, targetHorizontalVelocity.x, 10.0f * dt);
        velocity.z = Lerp(velocity.z, targetHorizontalVelocity.z, 10.0f * dt);

        if (Vector3LengthSqr(input) < 0.01f) { 
            velocity.x = Lerp(velocity.x, 0.0f, 15.0f * dt); 
            velocity.z = Lerp(velocity.z, 0.0f, 15.0f * dt);
        }
    }

    float gravity = 22.0f; 
    if (!isGrounded) {
        float currentGravity = gravity;
        if (velocity.y > 0 && !IsKeyDown(KEY_SPACE)) {
            currentGravity *= 1.5f; 
        }

        if (IsKeyDown(KEY_SPACE) && velocity.y < 0) { 
            isGliding = true;
            velocity.y -= currentGravity * dt * glideGravityMultiplier;
            velocity.x = Lerp(velocity.x, targetHorizontalVelocity.x * 1.5f, 5.0f * dt);
            velocity.z = Lerp(velocity.z, targetHorizontalVelocity.z * 1.5f, 5.0f * dt);
        } else {
            isGliding = false;
            velocity.y -= currentGravity * dt; 
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

    if (IsKeyPressed(KEY_C) && dashTimer <= 0) {
        Vector3 dashDirection = input; 
        if (Vector3Length(dashDirection) < 0.1f) dashDirection = cameraForward; 
        dashDirection = Vector3Normalize(dashDirection);

        velocity.x = dashDirection.x * dashPower;
        velocity.z = dashDirection.z * dashPower;
        dashTimer = dashCooldown;
        dashEffectTimer = 0.2f; 
    }

    position = Vector3Add(position, Vector3Scale(velocity, dt));

    if (position.y < 0.5f) { 
        position.y = 0.5f;
        if (!isGrounded) { 
            isGrounded = true;
            canDoubleJump = true;
            isGliding = false;
        }
    } else {
        isGrounded = false;
    }
}

// Draw player - Refined humanoid model with a sword
void Player::Draw(Camera3D_Custom& camera) {
    Vector3 forward = camera.GetForward();
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, (Vector3){0, 1, 0}));

    // --- Humanoid Body (Robed Figure) ---
    // Torso/Robe
    DrawCylinderEx(Vector3Subtract(position, {0, 0.5f, 0}), Vector3Add(position, {0, 0.8f, 0}), 0.5f, 0.3f, 8, DARKBLUE);
    // Head
    DrawSphere(Vector3Add(position, {0, 1.1f, 0}), 0.25f, BEIGE);
    // Eyes (indicator)
    Vector3 eyePos = Vector3Add(position, Vector3Scale(forward, 0.2f));
    eyePos.y += 1.15f;
    DrawSphere(Vector3Add(eyePos, Vector3Scale(right, 0.1f)), 0.05f, BLACK);
    eyePos = Vector3Subtract(eyePos, Vector3Scale(right, 0.1f));
    DrawSphere(eyePos, 0.05f, BLACK);

    // --- The Sword ---
    // Position sword relative to the player
    Vector3 handPos = Vector3Add(position, Vector3Scale(right, 0.45f));
    handPos = Vector3Add(handPos, Vector3Scale(forward, 0.2f));
    handPos.y += 0.3f;

    // Sword rotation (facing forward)
    float rotation = camera.yaw; 
    
    // Blade
    Vector3 bladePos = Vector3Add(handPos, Vector3Scale(forward, 0.8f));
    bladePos.y += 0.2f;
    // Draw blade as a long thin silver box
    // We use DrawCubeEx or rotate it manually. Let's compose it.
    // Blade
    Vector3 bladeDir = Vector3Normalize(Vector3Subtract(Vector3Add(handPos, Vector3Scale(forward, 2.0f)), handPos));
    for (float i = 0; i < 1.5f; i += 0.1f) {
        DrawCube(Vector3Add(handPos, Vector3Scale(forward, i)), 0.05f, 0.1f + i*0.02f, 0.1f, LIGHTGRAY);
    }
    
    // Hilt/Handle
    DrawCube(handPos, 0.1f, 0.1f, 0.4f, BROWN);
    // Crossguard
    Vector3 guardPos = Vector3Add(handPos, Vector3Scale(forward, 0.2f));
    DrawCube(guardPos, 0.5f, 0.1f, 0.1f, GOLD);

    // --- Faith Shield ---
    if (isShieldActive) {
        DrawSphereWires(position, 1.2f, 10, 10, Fade(SKYBLUE, 0.5f));
        DrawSphere(position, 1.1f, Fade(BLUE, 0.2f));
    }
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
