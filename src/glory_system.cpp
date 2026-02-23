#include "glory_system.h"
#include "raylib.h"
#include "raymath.h"
#include "particles3d.h" // Include ParticleSystem
#include "audio_manager.h" // Include AudioManager
#include "player.h"

GlorySystem::GlorySystem(ParticleSystem& ps, AudioManager& am)
    : particleSystem(ps), audioManager(am) { 
    gloryBeamCooldown = 0.5f; 
    gloryBeamTimer = 0.0f;    
    gloryBeamDamage = 100.0f; 

    prayerBurstCooldown = 2.0f; 
    prayerBurstTimer = 0.0f;
    prayerBurstDamage = 100.0f; 
    prayerBurstRadius = 5.0f;  

    lightBladeCooldown = 0.8f; 
    lightBladeTimer = 0.0f;
    lightBladeDamage = 75.0f; 
    lightBladeRange = 4.0f; 
    lightBladeArcAngle = 120.0f; 

    drawingGloryBeam = false;
    gloryBeamDrawTimer = 0.0f; 
    gloryBeamStartPos = {0.0f, 0.0f, 0.0f};
    gloryBeamEndPos = {0.0f, 0.0f, 0.0f};

    drawingLightBlade = false;
    lightBladeDrawTimer = 0.0f; 
    lightBladeDrawPos = {0.0f, 0.0f, 0.0f};
    lightBladeDrawForward = {0.0f, 0.0f, 1.0f}; 
}

void GlorySystem::Update(float dt, Camera3D_Custom& camera, Player& player, std::vector<std::unique_ptr<Enemy>>& enemies) {
    Vector3 playerPosition = player.GetPosition();

    // Update timers
    if (gloryBeamTimer > 0) gloryBeamTimer -= dt;
    if (prayerBurstTimer > 0) prayerBurstTimer -= dt;
    if (lightBladeTimer > 0) lightBladeTimer -= dt;

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

        gloryBeamDrawTimer = 0.3f; 
        gloryBeamStartPos = Vector3Add(playerPosition, {0, 1.0f, 0}); // Fire from chest height
        
        Ray ray;
        ray.position = camera.camera.position;
        ray.direction = Vector3Normalize(Vector3Subtract(camera.camera.target, camera.camera.position));
        
        gloryBeamEndPos = Vector3Add(gloryBeamStartPos, Vector3Scale(ray.direction, 100.0f)); 

        float closestHitDist = 1000.0f;
        Enemy* closestEnemy = nullptr;
        RayCollision closestCollision = {0};

        for (auto& enemy : enemies) {
            if (enemy->state != BANISHED && enemy->state != BANISHING) { 
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
            particleSystem.Emit(closestCollision.point, 25, YELLOW, GOLD, 0.1f, 0.5f, 0.2f, 0.5f, (Vector3){-1,-1,-1}, (Vector3){1,1,1});
            gloryBeamEndPos = closestCollision.point; 
        }
        drawingGloryBeam = true; 
    }

    // Prayer Burst activation (KEY_E)
    if (IsKeyPressed(KEY_E) && prayerBurstTimer <= 0) {
        audioManager.PlaySFX("prayer_burst_activate", playerPosition); 
        particleSystem.Emit(playerPosition, 60, BLUE, SKYBLUE, 0.3f, 1.5f, 0.5f, 1.0f, (Vector3){-3, 2, -3}, (Vector3){3, 5, 3});

        for (auto& enemy : enemies) {
            if (enemy->state != BANISHED && enemy->state != BANISHING) {
                float distance = Vector3Distance(playerPosition, enemy->position);
                if (distance < (prayerBurstRadius + enemy->radius)) {
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
        player.SwingSword(); // Trigger Animation!

        lightBladeDrawTimer = 0.3f; 
        lightBladeDrawPos = playerPosition;
        lightBladeDrawForward = camera.GetForward();
        drawingLightBlade = true;

        Vector3 playerForward = lightBladeDrawForward; 
        particleSystem.Emit(playerPosition, 40, WHITE, RAYWHITE, 0.2f, 0.8f, 0.3f, 0.6f, Vector3Scale(playerForward, 4.0f), Vector3Scale(playerForward, 8.0f));

        for (auto& enemy : enemies) {
            if (enemy->state != BANISHED && enemy->state != BANISHING) {
                Vector3 enemyToPlayer = Vector3Subtract(enemy->position, playerPosition);
                float distance = Vector3Length(enemyToPlayer);

                if (distance < (lightBladeRange + enemy->radius)) {
                    Vector3 enemyDirection = Vector3Normalize(enemyToPlayer);
                    float dotProduct = Vector3DotProduct(playerForward, enemyDirection);
                    float angle = acos(dotProduct) * RAD2DEG; 

                    if (angle < (lightBladeArcAngle / 2.0f)) {
                        enemy->TakeGloryHit(lightBladeDamage);
                        particleSystem.Emit(enemy->position, 20, WHITE, GOLD, 0.1f, 0.3f, 0.1f, 0.3f, (Vector3){-1, 1, -1}, (Vector3){1, 2, 1});
                    }
                }
            }
        }
    }
}

void GlorySystem::Draw(Camera3D_Custom& camera) {
    if (drawingGloryBeam) {
        // Realistic Beam: Concentric glowing cylinders
        float alpha = gloryBeamDrawTimer / 0.3f;
        DrawCylinderEx(gloryBeamStartPos, gloryBeamEndPos, 0.05f, 0.05f, 8, Fade(WHITE, alpha));
        DrawCylinderEx(gloryBeamStartPos, gloryBeamEndPos, 0.15f, 0.15f, 8, Fade(GOLD, alpha * 0.5f));
        DrawCylinderEx(gloryBeamStartPos, gloryBeamEndPos, 0.3f, 0.3f, 8, Fade(YELLOW, alpha * 0.2f));
    }

    if (drawingLightBlade) {
        Vector3 playerPos = lightBladeDrawPos;
        playerPos.y += 0.5f;
        Vector3 forward = lightBladeDrawForward;
        
        float halfArc = lightBladeArcAngle / 2.0f;
        int segments = 12;
        float angleStep = lightBladeArcAngle / segments;

        float alpha = lightBladeDrawTimer / 0.3f;

        for (int i = 0; i < segments; i++) {
            float currentAngle = -halfArc + (angleStep * i);
            float nextAngle = -halfArc + (angleStep * (i + 1));

            Vector3 p1Dir = Vector3RotateByAxisAngle(forward, (Vector3){0, 1, 0}, currentAngle * DEG2RAD);
            Vector3 p2Dir = Vector3RotateByAxisAngle(forward, (Vector3){0, 1, 0}, nextAngle * DEG2RAD);
            
            Vector3 p1 = Vector3Add(playerPos, Vector3Scale(p1Dir, lightBladeRange));
            Vector3 p2 = Vector3Add(playerPos, Vector3Scale(p2Dir, lightBladeRange));

            DrawLine3D(playerPos, p1, Fade(GOLD, alpha * 0.3f));
            DrawLine3D(p1, p2, Fade(WHITE, alpha));
        }
    }
}
