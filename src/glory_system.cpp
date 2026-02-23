#include "glory_system.h"
#include "raylib.h"
#include "raymath.h"
#include "particles3d.h" // Include ParticleSystem
#include "audio_manager.h" // Include AudioManager

GlorySystem::GlorySystem(ParticleSystem& ps, AudioManager& am)
    : particleSystem(ps), audioManager(am) { // Initialize references
    gloryBeamCooldown = 0.5f; // Half a second cooldown
    gloryBeamTimer = 0.0f;    // Initially ready
    gloryBeamDamage = 100.0f; // Increased for 1-hit banish in testing

    prayerBurstCooldown = 2.0f; // 2 seconds cooldown
    prayerBurstTimer = 0.0f;
    prayerBurstDamage = 100.0f; 
    prayerBurstRadius = 5.0f;  // 5 units radius for the burst

    lightBladeCooldown = 0.8f; 
    lightBladeTimer = 0.0f;
    lightBladeDamage = 75.0f; 
    lightBladeRange = 4.0f; // Increased melee range
    lightBladeArcAngle = 120.0f; // Increased arc angle

    // Initialize visual feedback variables
    drawingGloryBeam = false;
    gloryBeamDrawTimer = 0.0f; 
    gloryBeamStartPos = {0.0f, 0.0f, 0.0f};
    gloryBeamEndPos = {0.0f, 0.0f, 0.0f};

    drawingLightBlade = false;
    lightBladeDrawTimer = 0.0f; 
    lightBladeDrawPos = {0.0f, 0.0f, 0.0f};
    lightBladeDrawForward = {0.0f, 0.0f, 1.0f}; 
}

void GlorySystem::Update(float dt, Camera3D_Custom& camera, Vector3 playerPosition, std::vector<std::unique_ptr<Enemy>>& enemies) {
    // Update timers
    if (gloryBeamTimer > 0) {
        gloryBeamTimer -= dt;
    }
    if (prayerBurstTimer > 0) {
        prayerBurstTimer -= dt;
    }
    if (lightBladeTimer > 0) {
        lightBladeTimer -= dt;
    }

    // Decrement visual feedback timers
    if (gloryBeamDrawTimer > 0) {
        gloryBeamDrawTimer -= dt;
        if (gloryBeamDrawTimer <= 0) drawingGloryBeam = false;
    }
    if (lightBladeDrawTimer > 0) {
        lightBladeDrawTimer -= dt;
        if (lightBladeDrawTimer <= 0) drawingLightBlade = false;
    }


    // Glory Beam activation (LMB)
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && gloryBeamTimer <= 0) {
        audioManager.PlaySFX("glory_beam_activate", playerPosition); 

        gloryBeamTimer = gloryBeamCooldown; 

        // Set visual feedback for Glory Beam
        gloryBeamDrawTimer = 0.3f; 
        gloryBeamStartPos = camera.camera.position;
        
        // Ray directly from camera forward vector
        Ray ray;
        ray.position = camera.camera.position;
        ray.direction = Vector3Normalize(Vector3Subtract(camera.camera.target, camera.camera.position));
        
        gloryBeamEndPos = Vector3Add(gloryBeamStartPos, Vector3Scale(ray.direction, 100.0f)); 

        float closestHitDist = 1000.0f;
        Enemy* closestEnemy = nullptr;
        RayCollision closestCollision = {0};

        // Check for collisions with all enemies to find the closest hit
        for (auto& enemy : enemies) {
            if (enemy->state != BANISHED) { 
                // Make the beam more forgiving by increasing the effective collision radius for the check
                RayCollision collision = GetRayCollisionSphere(ray, enemy->position, enemy->radius * 2.0f); 
                if (collision.hit && collision.distance < closestHitDist) {
                    closestHitDist = collision.distance;
                    closestEnemy = enemy.get();
                    closestCollision = collision;
                }
            }
        }

        if (closestEnemy != nullptr) {
            closestEnemy->TakeGloryHit(gloryBeamDamage);
            particleSystem.Emit(closestCollision.point, 25, YELLOW, GOLD, 0.1f, 0.5f, 0.2f, 0.5f, (Vector3){-1.0f, -1.0f, -1.0f}, (Vector3){1.0f, 1.0f, 1.0f});
            gloryBeamEndPos = closestCollision.point; 
        }
        drawingGloryBeam = true; 
    }

    // Prayer Burst activation (KEY_E)
    if (IsKeyPressed(KEY_E) && prayerBurstTimer <= 0) {
        audioManager.PlaySFX("prayer_burst_activate", playerPosition); 
        particleSystem.Emit(playerPosition, 60, BLUE, SKYBLUE, 0.3f, 1.5f, 0.5f, 1.0f, (Vector3){-3.0f, 2.0f, -3.0f}, (Vector3){3.0f, 5.0f, 3.0f});

        for (auto& enemy : enemies) {
            if (enemy->state != BANISHED) {
                float distance = Vector3Distance(playerPosition, enemy->position);
                if (distance < (prayerBurstRadius + enemy->radius)) { // Account for enemy radius
                    enemy->TakeGloryHit(prayerBurstDamage);
                }
            }
        }
        prayerBurstTimer = prayerBurstCooldown; 
    }

    // Light Blade activation (RMB)
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && lightBladeTimer <= 0) {
        audioManager.PlaySFX("light_blade_activate", playerPosition); 
        lightBladeTimer = lightBladeCooldown; 

        lightBladeDrawTimer = 0.3f; 
        lightBladeDrawPos = playerPosition;
        lightBladeDrawForward = Vector3Normalize(Vector3Subtract(camera.camera.target, camera.camera.position));
        lightBladeDrawForward.y = 0; 
        lightBladeDrawForward = Vector3Normalize(lightBladeDrawForward);
        drawingLightBlade = true;

        Vector3 playerForward = lightBladeDrawForward; 
        particleSystem.Emit(playerPosition, 40, WHITE, RAYWHITE, 0.2f, 0.8f, 0.3f, 0.6f, Vector3Scale(playerForward, 4.0f), Vector3Scale(playerForward, 8.0f));

        for (auto& enemy : enemies) {
            if (enemy->state != BANISHED) {
                Vector3 enemyToPlayer = Vector3Subtract(enemy->position, playerPosition);
                float distance = Vector3Length(enemyToPlayer);

                if (distance < (lightBladeRange + enemy->radius)) { // Account for enemy radius
                    Vector3 enemyDirection = Vector3Normalize(enemyToPlayer);
                    float dotProduct = Vector3DotProduct(playerForward, enemyDirection);
                    float angle = acos(dotProduct) * RAD2DEG; 

                    if (angle < (lightBladeArcAngle / 2.0f)) {
                        enemy->TakeGloryHit(lightBladeDamage);
                        particleSystem.Emit(enemy->position, 20, WHITE, GOLD, 0.1f, 0.3f, 0.1f, 0.3f, (Vector3){-1.0f, 1.0f, -1.0f}, (Vector3){1.0f, 2.0f, 1.0f});
                    }
                }
            }
        }
    }
}

void GlorySystem::Draw(Camera3D_Custom& camera) {
    if (drawingGloryBeam) {
        DrawLine3D(gloryBeamStartPos, gloryBeamEndPos, YELLOW);
    }

    if (drawingLightBlade) {
        // Draw a fan-shaped arc for light blade
        // This is a simplified visual representation
        Vector3 playerPos = lightBladeDrawPos;
        Vector3 forward = lightBladeDrawForward;
        Vector3 right = Vector3CrossProduct(forward, (Vector3){0.0f, 1.0f, 0.0f});
        right = Vector3Normalize(right);

        float halfArc = lightBladeArcAngle / 2.0f;
        int segments = 10;
        float angleStep = lightBladeArcAngle / segments;

        // Start point of the fan (player position)
        for (int i = 0; i < segments; i++) {
            float currentAngle = -halfArc + (angleStep * i);
            float nextAngle = -halfArc + (angleStep * (i + 1));

            Vector3 p1Dir = Vector3RotateByAxisAngle(forward, (Vector3){0.0f, 1.0f, 0.0f}, currentAngle * DEG2RAD);
            Vector3 p2Dir = Vector3RotateByAxisAngle(forward, (Vector3){0.0f, 1.0f, 0.0f}, nextAngle * DEG2RAD);
            
            Vector3 p1 = Vector3Add(playerPos, Vector3Scale(p1Dir, lightBladeRange));
            Vector3 p2 = Vector3Add(playerPos, Vector3Scale(p2Dir, lightBladeRange));

            DrawLine3D(playerPos, p1, GOLD);
            DrawLine3D(p1, p2, GOLD);
        }
        DrawLine3D(playerPos, Vector3Add(playerPos, Vector3Scale(Vector3RotateByAxisAngle(forward, (Vector3){0.0f, 1.0f, 0.0f}, -halfArc * DEG2RAD), lightBladeRange)), GOLD);
        DrawLine3D(playerPos, Vector3Add(playerPos, Vector3Scale(Vector3RotateByAxisAngle(forward, (Vector3){0.0f, 1.0f, 0.0f}, halfArc * DEG2RAD), lightBladeRange)), GOLD);
    }
}
