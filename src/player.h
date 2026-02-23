#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "raymath.h"
#include "camera3d.h" // Include custom camera header

class Player {
public:
    Vector3 position;
    Vector3 velocity;
    float baseSpeed; // Base movement speed
    float sprintSpeedMultiplier;
    float currentSpeed; // Effective speed including sprint
    float jumpStrength;
    bool isGrounded;
    bool canDoubleJump;
    bool isGliding;

    float dashPower;
    float dashCooldown;
    float dashTimer; // Timer for dash cooldown
    float dashEffectTimer; // Timer for how long the dash impulse lasts

    float glideGravityMultiplier; // To reduce gravity while gliding

    float faithMeter;
    float maxFaith;
    float radius; // For collision detection

    // Faith Shield
    bool isShieldActive;
    float shieldTimer;
    float shieldCooldown;
    float shieldDuration;

    Player();
    void Update(Camera3D_Custom& camera);
    void Draw(Camera3D_Custom& camera); // Now takes camera for orientation
    void SetPosition(Vector3 pos);
    Vector3 GetPosition();

    void TakeDamage(float amount);
    void Heal(float amount);

    // Sword Animation
    bool isSwinging;
    float swingTimer;
    void SwingSword();
};

#endif // PLAYER_H
