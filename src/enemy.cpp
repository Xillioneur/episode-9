#include "enemy.h"
#include "raylib.h"
#include "raymath.h"
#include "audio_manager.h" 
#include "player.h"        
#include "level_manager.h"
#include "particles3d.h" 

// Enemy base class constructor
Enemy::Enemy(Vector3 startPos, float initialDoubt, float moveSpeed, float range, float rad, AudioManager& am)
    : audioManager(am) { 
    position = startPos;
    spawnPosition = startPos; 
    velocity = (Vector3){ 0.0f, 0.0f, 0.0f };
    doubtMeter = initialDoubt;
    maxDoubt = initialDoubt;
    speed = moveSpeed;
    state = IDLE; 
    detectionRange = range;
    radius = rad;
    flashTimer = 0.0f;
    isAttacking = false;
    banishTimer = 0.0f;
    isBanishing = false;
    stateTimer = 0.0f;
}

void Enemy::TakeGloryHit(float damage) {
    doubtMeter -= damage;
    if (doubtMeter < 0) doubtMeter = 0;
    flashTimer = 0.2f; 
    audioManager.PlaySFX("enemy_hit", position); 
}

bool Enemy::IsBanished() {
    return doubtMeter <= 0;
}

bool Enemy::ReadyToRemove() {
    return state == BANISHED;
}

// ---------------------------------------------------------------------------------------------
// ShadowDrone Implementation (Modified: Hitscan Beam)
// ---------------------------------------------------------------------------------------------

ShadowDrone::ShadowDrone(Vector3 startPos, AudioManager& am)
    : Enemy(startPos, 100.0f, 8.0f, 20.0f, 0.5f, am) { 
    hoverAmplitude = 0.5f;
    hoverSpeed = 2.0f;
    attackRange = 15.0f; 
    state = IDLE; 
    shootCooldown = 2.0f; 
    shootTimer = 0.0f;
}

void ShadowDrone::Update(float dt, Player& player, LevelManager& levelManager, ParticleSystem& particleSystem) {
    if (flashTimer > 0) flashTimer -= dt; 
    if (stateTimer > 0) stateTimer -= dt;

    if (IsBanished()) {
        if (!isBanishing) {
            isBanishing = true;
            state = BANISHING;
            banishTimer = 1.0f;
            audioManager.PlaySFX("enemy_banished", position);
        }
    }

    if (state == BANISHING) {
        banishTimer -= dt;
        particleSystem.Emit(position, 5, PURPLE, GOLD, 0.2f, 0.5f, 0.5f, 1.0f, (Vector3){-1, 1, -1}, (Vector3){1, 2, 1});
        if (banishTimer <= 0) state = BANISHED;
        return;
    }

    if (state == BANISHED) return;

    Vector3 playerPos = player.GetPosition();
    Vector3 toPlayer = Vector3Subtract(playerPos, position);
    float dist = Vector3Length(toPlayer);

    if (dist < (radius + player.radius)) {
        Vector3 push = Vector3Normalize(Vector3Subtract(position, playerPos));
        position = Vector3Add(position, Vector3Scale(push, (radius + player.radius) - dist));
    }

    // Hitscan Beam Logic
    if (shootTimer > 0) shootTimer -= dt;
    if (dist < detectionRange && shootTimer <= 0) {
        // Particle charge effect
        particleSystem.Emit(position, 5, RED, ORANGE, 0.1f, 0.2f, 0.1f, 0.3f, (Vector3){-0.5f,-0.5f,-0.5f}, (Vector3){0.5f,0.5f,0.5f});
        
        if (shootTimer < -0.5f) { // 0.5s windup
            Vector3 shootDir = Vector3Normalize(toPlayer);
            Ray ray = { position, shootDir };
            RayCollision col = GetRayCollisionSphere(ray, playerPos, player.radius);
            if (col.hit && col.distance < detectionRange) {
                player.TakeDamage(12.0f);
                for(float i=0; i<col.distance; i+=1.0f) {
                    particleSystem.Emit(Vector3Add(position, Vector3Scale(shootDir, i)), 1, RED, YELLOW, 0.1f, 0.2f, 0.1f, 0.2f, (Vector3){0,0,0}, (Vector3){0,0,0});
                }
            }
            shootTimer = shootCooldown;
            audioManager.PlaySFX("enemy_attack", position);
        }
    }

    float targetHeight = 4.0f;

    switch (state) {
        case IDLE:
            position.y = targetHeight + sin(GetTime() * hoverSpeed) * hoverAmplitude;
            if (dist < detectionRange) state = CHASE;
            break;

        case CHASE: {
            Vector3 targetPos = { playerPos.x, targetHeight, playerPos.z };
            Vector3 moveDir = Vector3Normalize(Vector3Subtract(targetPos, position));
            moveDir = Vector3Add(moveDir, Vector3Scale(Vector3CrossProduct(moveDir, {0,1,0}), sinf(GetTime()*2)*0.5f));
            position = Vector3Add(position, Vector3Scale(Vector3Normalize(moveDir), speed * dt));
            if (dist < 10.0f) { state = ATTACK; stateTimer = 3.0f; }
        } break;

        case ATTACK: {
            Vector3 dir = Vector3Normalize(toPlayer);
            Vector3 right = Vector3CrossProduct(dir, (Vector3){0, 1, 0});
            Vector3 moveVec = Vector3Scale(right, speed * 0.8f * dt);
            if (dist > 12.0f) moveVec = Vector3Add(moveVec, Vector3Scale(dir, speed * 0.5f * dt));
            if (dist < 8.0f) moveVec = Vector3Subtract(moveVec, Vector3Scale(dir, speed * 0.5f * dt));
            position = Vector3Add(position, moveVec);
            position.y = (playerPos.y + 4.0f) + sinf(GetTime() * 2.0f) * 1.0f;
            if (stateTimer <= 0) state = CHASE;
        } break;
        default: break;
    }
}

void ShadowDrone::Draw() {
    if (state == BANISHED) return;
    if (state == BANISHING) {
        float alpha = banishTimer;
        DrawSphereWires(position, radius * (2.0f - alpha), 8, 8, Fade(GOLD, alpha));
        return;
    }
    Color color = (flashTimer > 0) ? WHITE : ((doubtMeter < maxDoubt / 2) ? PURPLE : DARKPURPLE);
    DrawSphere(position, radius, color);
    DrawSphereWires(position, radius, 6, 6, BLACK);
    float wingOffset = sinf(GetTime() * 15.0f) * 0.2f;
    DrawCube(Vector3Add(position, {0.6f, wingOffset, 0}), 0.8f, 0.1f, 0.4f, color);
    DrawCube(Vector3Subtract(position, {0.6f, -wingOffset, 0}), 0.8f, 0.1f, 0.4f, color);

    Vector3 meterPos = Vector3Add(position, (Vector3){0.0f, radius + 0.3f, 0.0f});
    DrawCube(meterPos, 1.0f, 0.1f, 0.1f, BLACK);
    DrawCube((Vector3){meterPos.x - (1.0f - (doubtMeter/maxDoubt)) / 2.0f, meterPos.y, meterPos.z}, (doubtMeter/maxDoubt), 0.08f, 0.08f, RED);
}

// ---------------------------------------------------------------------------------------------
// Whisperer Implementation
// ---------------------------------------------------------------------------------------------

Whisperer::Whisperer(Vector3 startPos, AudioManager& am)
    : Enemy(startPos, 60.0f, 11.0f, 15.0f, 0.5f, am) { 
    chargeCooldown = 2.5f; 
    chargeTimer = 0.0f;
    state = IDLE;
}

void Whisperer::Update(float dt, Player& player, LevelManager& levelManager, ParticleSystem& particleSystem) {
    if (flashTimer > 0) flashTimer -= dt; 
    if (stateTimer > 0) stateTimer -= dt;

    if (IsBanished()) {
        if (!isBanishing) {
            isBanishing = true;
            state = BANISHING;
            banishTimer = 1.0f; 
            audioManager.PlaySFX("enemy_banished", position);
        }
    }

    if (state == BANISHING) {
        banishTimer -= dt;
        particleSystem.Emit(position, 5, LIME, WHITE, 0.2f, 0.5f, 0.5f, 1.0f, (Vector3){-1, 1, -1}, (Vector3){1, 2, 1});
        if (banishTimer <= 0) state = BANISHED;
        return; 
    }

    if (state == BANISHED) return;
    if (chargeTimer > 0) chargeTimer -= dt;

    Vector3 playerPos = player.GetPosition();
    Vector2 pos2D = { position.x, position.z };
    Vector2 pPos2D = { playerPos.x, playerPos.z };
    float dist2D = Vector2Distance(pos2D, pPos2D);

    if (dist2D < (radius + player.radius) && fabsf(playerPos.y - position.y) < 1.0f) {
        Vector2 push = Vector2Normalize(Vector2Subtract(pos2D, pPos2D));
        position.x += push.x * ((radius + player.radius) - dist2D);
        position.z += push.y * ((radius + player.radius) - dist2D);
    }

    if (position.y > 0.5f) position.y -= 15.0f * dt;
    if (position.y < 0.5f) position.y = 0.5f;

    switch (state) {
        case IDLE:
            if (dist2D < detectionRange) { state = CHASE; stateTimer = 1.0f; }
            break;

        case CHASE: {
            Vector3 target = { playerPos.x, position.y, playerPos.z };
            Vector3 dir = Vector3Normalize(Vector3Subtract(target, position));
            position = Vector3Add(position, Vector3Scale(dir, speed * dt));
            if (dist2D < 4.0f && chargeTimer <= 0) {
                state = ATTACK;
                chargeTimer = 0.4f; 
                isAttacking = false;
            }
        } break;

        case ATTACK:
            if (chargeTimer > 0) {
                position.x += sinf(GetTime() * 30.0f) * 0.05f;
            } else {
                Vector3 lungeDir = Vector3Normalize(Vector3Subtract(playerPos, position));
                position = Vector3Add(position, Vector3Scale(lungeDir, speed * 2.5f * dt));
                if (!isAttacking && Vector3Distance(position, playerPos) < 1.2f) {
                    player.TakeDamage(15.0f); 
                    isAttacking = true;
                    audioManager.PlaySFX("enemy_attack", position);
                }
                if (chargeTimer < -0.3f) { 
                    state = CHASE;
                    chargeTimer = chargeCooldown;
                }
            }
            break;
        default: break;
    }
}

void Whisperer::Draw() {
    if (state == BANISHED) return;
    if (state == BANISHING) {
        float alpha = banishTimer;
        DrawCylinderWires(position, radius * (2.0f - alpha), radius * (2.0f - alpha), 1.5f, 8, Fade(GOLD, alpha));
        return;
    }
    Color color = (flashTimer > 0) ? WHITE : ((doubtMeter < maxDoubt / 2) ? LIME : DARKGREEN);
    DrawCylinder(position, radius, radius * 0.8f, 1.5f, 8, color);
    DrawSphere(Vector3Add(position, {0, 0.8f, 0}), 0.3f, color);
    DrawSphereWires(Vector3Add(position, {0, 0.8f, 0}), 0.3f, 6, 6, BLACK);

    Vector3 shoulderL = Vector3Add(position, {0.3f, 0.8f, 0});
    Vector3 shoulderR = Vector3Add(position, {-0.3f, 0.8f, 0});
    DrawCylinderEx(shoulderL, Vector3Add(shoulderL, {0, -0.6f, 0.2f}), 0.08f, 0.05f, 4, color);
    DrawCylinderEx(shoulderR, Vector3Add(shoulderR, {0, -0.6f, 0.2f}), 0.08f, 0.05f, 4, color);

    Vector3 meterPos = Vector3Add(position, (Vector3){0.0f, 2.0f, 0.0f});
    DrawCube(meterPos, 1.0f, 0.1f, 0.1f, BLACK);
    DrawCube((Vector3){meterPos.x - (1.0f - (doubtMeter/maxDoubt)) / 2.0f, meterPos.y, meterPos.z}, (doubtMeter/maxDoubt), 0.08f, 0.08f, RED);
}

// ---------------------------------------------------------------------------------------------
// TemptationBeast Implementation
// ---------------------------------------------------------------------------------------------

TemptationBeast::TemptationBeast(Vector3 startPos, AudioManager& am)
    : Enemy(startPos, 250.0f, 5.5f, 25.0f, 1.5f, am) { 
    chargeWindup = 0.0f;
    chargeDuration = 0.0f;
    state = IDLE;
}

void TemptationBeast::Update(float dt, Player& player, LevelManager& levelManager, ParticleSystem& particleSystem) {
    if (flashTimer > 0) flashTimer -= dt; 
    if (IsBanished()) {
        if (!isBanishing) {
            isBanishing = true;
            state = BANISHING;
            banishTimer = 1.5f; 
            audioManager.PlaySFX("enemy_banished", position);
        }
    }
    if (state == BANISHING) {
        banishTimer -= dt;
        particleSystem.Emit(position, 10, RED, GOLD, 0.5f, 1.0f, 0.5f, 1.5f, (Vector3){-2, 1, -2}, (Vector3){2, 3, 2});
        if (banishTimer <= 0) state = BANISHED; 
        return; 
    }
    if (state == BANISHED) return;
    Vector3 playerPos = player.GetPosition();
    Vector3 toPlayer = Vector3Subtract(playerPos, position);
    float dist = Vector3Length(toPlayer);
    if (dist < (radius + player.radius)) {
        Vector3 push = Vector3Normalize(Vector3Subtract(position, playerPos));
        position = Vector3Add(position, Vector3Scale(push, (radius + player.radius) - dist));
    }
    if (position.y > 1.5f) position.y -= 20.0f * dt;
    if (position.y < 1.5f) position.y = 1.5f;
    switch (state) {
        case IDLE: if (dist < detectionRange) state = CHASE; break;
        case CHASE: {
            Vector3 dir = Vector3Normalize(toPlayer);
            position = Vector3Add(position, Vector3Scale(dir, speed * dt));
            if (dist < 10.0f) { state = ATTACK; chargeWindup = 1.5f; isAttacking = false; }
        } break;
        case ATTACK: {
            if (chargeWindup > 0) {
                chargeWindup -= dt;
                Vector3 predictedPos = Vector3Add(playerPos, Vector3Scale(player.velocity, 0.4f));
                chargeDir = Vector3Normalize(Vector3Subtract(predictedPos, position));
                position.x += sinf(GetTime() * 40.0f) * 0.1f * dt;
                if (chargeWindup <= 0) { chargeDuration = 1.2f; audioManager.PlaySFX("enemy_attack", position); }
            } else if (chargeDuration > 0) {
                chargeDuration -= dt;
                position = Vector3Add(position, Vector3Scale(chargeDir, speed * 4.0f * dt)); 
                if (!isAttacking && dist < (radius + 1.2f)) {
                    player.TakeDamage(25.0f); 
                    isAttacking = true;
                    particleSystem.Emit(playerPos, 20, RED, GRAY, 0.5f, 1.0f, 0.3f, 0.6f, (Vector3){-2, 2, -2}, (Vector3){2, 4, 2});
                }
            } else { state = CHASE; stateTimer = 2.0f; }
        } break;
        default: break;
    }
}

void TemptationBeast::Draw() {
    if (state == BANISHED) return;
    if (state == BANISHING) {
        float alpha = banishTimer / 1.5f;
        DrawCubeWires(position, 3.0f, 3.0f, 3.0f, Fade(GOLD, alpha));
        return;
    }
    Color color = (flashTimer > 0) ? WHITE : ((doubtMeter < maxDoubt / 2) ? PINK : MAROON);
    if (state == ATTACK && chargeWindup > 0) {
        float pulse = (sinf(GetTime() * 15.0f) + 1.0f) / 2.0f;
        color = ColorLerp(MAROON, RED, pulse);
    }
    DrawCube(position, 3.0f, 2.0f, 3.0f, color);
    DrawCubeWires(position, 3.0f, 2.0f, 3.0f, BLACK);
    DrawCylinderEx(Vector3Add(position, {1.0f, 1.0f, 1.0f}), Vector3Add(position, {1.5f, 2.0f, 1.5f}), 0.2f, 0.05f, 6, GRAY);
    DrawCylinderEx(Vector3Add(position, {-1.0f, 1.0f, 1.0f}), Vector3Add(position, {-1.5f, 2.0f, 1.5f}), 0.2f, 0.05f, 6, GRAY);
    Vector3 meterPos = Vector3Add(position, (Vector3){0.0f, 2.5f, 0.0f});
    DrawCube(meterPos, 2.0f, 0.2f, 0.1f, BLACK);
    DrawCube((Vector3){meterPos.x - (2.0f - (2.0f * (doubtMeter/maxDoubt))) / 2.0f, meterPos.y, meterPos.z}, (2.0f * (doubtMeter/maxDoubt)), 0.15f, 0.15f, RED);
}

// ---------------------------------------------------------------------------------------------
// Boss Implementation
// ---------------------------------------------------------------------------------------------

Boss::Boss(Vector3 startPos, BossType bossType, AudioManager& am)
    : Enemy(startPos, 500.0f, 3.5f, 50.0f, 3.0f, am) { 
    type = bossType;
    phase = 1;
    actionTimer = 2.0f;
    summonTimer = 5.0f;
    if (type == PRIDE) { maxDoubt = 600.0f; speed = 3.5f; }
    else if (type == DESPAIR) { maxDoubt = 800.0f; speed = 4.5f; radius = 4.0f; }
    else if (type == DEATH) { maxDoubt = 1200.0f; speed = 7.0f; radius = 2.0f; }
    doubtMeter = maxDoubt;
    state = CHASE;
}

void Boss::Update(float dt, Player& player, LevelManager& levelManager, ParticleSystem& particleSystem) {
    if (flashTimer > 0) flashTimer -= dt; 
    if (IsBanished()) {
        if (!isBanishing) {
            isBanishing = true;
            state = BANISHING;
            banishTimer = 3.0f; 
            audioManager.PlaySFX("enemy_banished", position);
        }
    }
    if (state == BANISHING) {
        banishTimer -= dt;
        particleSystem.Emit(position, 20, GOLD, WHITE, 0.5f, 2.0f, 1.0f, 3.0f, (Vector3){-5, 0, -5}, (Vector3){5, 10, 5});
        if (banishTimer <= 0) state = BANISHED;
        return;
    }
    if (state == BANISHED) return;
    Vector3 playerPos = player.GetPosition();
    float dist = Vector3Distance(position, playerPos);
    if (dist < (radius + player.radius)) player.TakeDamage(2.0f); 

    if (type == PRIDE) {
        position.y = 3.0f; 
        if (actionTimer > 0) actionTimer -= dt;
        else {
            if (dist < 12.0f) {
                if (dist < 10.0f) player.TakeDamage(25.0f);
                particleSystem.Emit(position, 60, DARKPURPLE, BLACK, 1.0f, 4.0f, 0.5f, 1.2f, (Vector3){-6, 0, -6}, (Vector3){6, 1, 6});
                audioManager.PlaySFX("enemy_attack", position);
                actionTimer = 2.5f;
            } else {
                Vector3 dir = Vector3Normalize(Vector3Subtract(playerPos, position));
                position = Vector3Add(position, Vector3Scale(dir, speed * dt));
            }
        }
        if (summonTimer > 0) summonTimer -= dt;
        else {
            Vector3 shootDir = Vector3Normalize(Vector3Subtract(playerPos, position));
            levelManager.AddProjectile(position, Vector3Scale(shootDir, 20.0f), 0.6f, 5.0f, false);
            summonTimer = 6.0f;
        }
    } else if (type == DESPAIR) {
        position.y = 5.0f;
        if (actionTimer > 0) actionTimer -= dt;
        else {
            for (int i=0; i<12; i++) {
                float angle = i * 30.0f * DEG2RAD;
                Vector3 dir = { sinf(angle), -0.2f, cosf(angle) };
                levelManager.AddProjectile(position, Vector3Scale(dir, 10.0f), 0.5f, 5.0f, false);
            }
            audioManager.PlaySFX("enemy_attack", position);
            actionTimer = 2.0f;
        }
        Vector3 dir = Vector3Normalize(Vector3Subtract(playerPos, position));
        position = Vector3Add(position, Vector3Scale(dir, speed * dt));
    } else if (type == DEATH) {
        if (state == CHASE) {
            Vector3 dir = Vector3Normalize(Vector3Subtract(playerPos, position));
            position = Vector3Add(position, Vector3Scale(dir, speed * dt));
            if (dist < 10.0f) { state = ATTACK; actionTimer = 0.4f; }
        } else if (state == ATTACK) {
            actionTimer -= dt;
            if (actionTimer <= 0) {
                Vector3 dir = Vector3Normalize(Vector3Subtract(playerPos, position));
                position = Vector3Add(position, Vector3Scale(dir, speed * 5.0f * dt));
                if (dist < 3.5f) { player.TakeDamage(50.0f); state = CHASE; }
                if (dist > 20.0f) state = CHASE;
            }
        }
    }
}

void Boss::Draw() {
    if (state == BANISHED) return;
    if (state == BANISHING) {
        float alpha = banishTimer / 3.0f;
        DrawSphereWires(position, radius * (2.0f - alpha), 16, 16, Fade(GOLD, alpha));
        return;
    }
    Color color = (flashTimer > 0) ? WHITE : DARKGRAY;
    if (type == PRIDE) color = (flashTimer > 0) ? WHITE : PURPLE;
    if (type == DESPAIR) color = (flashTimer > 0) ? WHITE : DARKBLUE;
    if (type == DEATH) color = (flashTimer > 0) ? WHITE : BLACK;
    if (type == PRIDE) {
        DrawCylinder(position, radius, radius, 6.0f, 8, color);
        DrawSphere(Vector3Add(position, {0, 6.5f, 0}), 2.0f, color);
        DrawCylinderWires(Vector3Add(position, {0, 3.8f, 0}), 1.0f, 1.5f, 0.8f, 6, GOLD);
    } else if (type == DESPAIR) {
        DrawSphere(position, radius * 0.5f, Fade(color, 0.9f));
        for(int i=0; i<12; i++) {
            float t = GetTime() * 2.0f + i;
            Vector3 orbPos = Vector3Add(position, {sinf(t)*3.5f, cosf(t*0.5f)*2.0f, cosf(t)*3.5f});
            DrawCube(orbPos, 0.8f, 0.8f, 0.8f, Fade(SKYBLUE, 0.6f));
        }
    } else if (type == DEATH) {
        DrawCylinder(Vector3Subtract(position, {0, 3.0f, 0}), 1.5f, 2.0f, 6.0f, 8, color);
        DrawSphere(Vector3Add(position, {0, 2.5f, 0}), 1.2f, color);
        Vector3 hand = Vector3Add(position, {1.5f, 1.0f, 1.0f});
        DrawCylinderEx(hand, Vector3Add(hand, {0, 4.0f, 1.0f}), 0.1f, 0.1f, 4, GRAY);
        Vector3 bladeBase = Vector3Add(hand, {0, 3.5f, 1.0f});
        DrawCylinderEx(bladeBase, Vector3Add(bladeBase, {-1.5f, 0.5f, 0}), 0.1f, 0.4f, 4, LIGHTGRAY);
    }
    Vector3 meterPos = Vector3Add(position, (Vector3){0.0f, 8.0f, 0.0f});
    DrawCube(meterPos, 5.0f, 0.5f, 0.1f, BLACK);
    DrawCube((Vector3){meterPos.x - (5.0f - (5.0f * (doubtMeter/maxDoubt))) / 2.0f, meterPos.y, meterPos.z}, (5.0f * (doubtMeter/maxDoubt)), 0.4f, 0.15f, RED);
}
