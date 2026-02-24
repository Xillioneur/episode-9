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

void Enemy::HandlePlayerCollision(Player& player, float dt) {
    Vector3 playerPos = player.GetPosition();
    Vector3 toPlayer = Vector3Subtract(playerPos, position);
    float dist = Vector3Length(toPlayer);
    float combinedRadius = radius + player.radius;

    if (dist < combinedRadius + 0.3f) {
        Vector3 pushDir = Vector3Normalize(toPlayer);
        if (Vector3LengthSqr(pushDir) < 0.001f) pushDir = {0, 0, 1};
        
        float overlap = (combinedRadius + 0.3f) - dist;
        player.SetPosition(Vector3Add(playerPos, Vector3Scale(pushDir, overlap)));
        
        player.velocity = Vector3Add(player.velocity, Vector3Scale(pushDir, 30.0f));
        player.knockbackTimer = 0.4f;
        
        if (dist < combinedRadius) {
            player.TakeDamage(10.0f * dt * 60.0f); 
        }
    }
}

// ---------------------------------------------------------------------------------------------
// ShadowDrone Implementation
// ---------------------------------------------------------------------------------------------

ShadowDrone::ShadowDrone(Vector3 startPos, AudioManager& am)
    : Enemy(startPos, 120.0f, 9.0f, 25.0f, 0.6f, am) { 
    hoverAmplitude = 0.5f;
    hoverSpeed = 2.0f;
    attackRange = 18.0f; 
    state = IDLE; 
    shootCooldown = 1.8f; 
    shootTimer = 0.0f;
}

void ShadowDrone::Update(float dt, Player& player, LevelManager& levelManager, ParticleSystem& particleSystem) {
    if (flashTimer > 0) flashTimer -= dt; 
    if (stateTimer > 0) stateTimer -= dt;

    if (IsBanished()) {
        if (!isBanishing) { isBanishing = true; state = BANISHING; banishTimer = 1.0f; audioManager.PlaySFX("enemy_banished", position); }
    }

    if (state == BANISHING) {
        banishTimer -= dt;
        particleSystem.Emit(position, 5, PURPLE, GOLD, 0.2f, 0.5f, 0.5f, 1.0f, (Vector3){-1, 1, -1}, (Vector3){1, 2, 1});
        if (banishTimer <= 0) state = BANISHED;
        return;
    }

    if (state == BANISHED) return;

    HandlePlayerCollision(player, dt);

    Vector3 playerPos = player.GetPosition();
    Vector3 toPlayer = Vector3Subtract(playerPos, position);
    float dist = Vector3Length(toPlayer);

    if (shootTimer > 0) shootTimer -= dt;
    if (dist < detectionRange && shootTimer <= 0) {
        Vector3 shootDir = Vector3Normalize(toPlayer);
        levelManager.AddProjectile(position, Vector3Scale(shootDir, 18.0f), 0.3f, 4.0f, false);
        shootTimer = shootCooldown;
        audioManager.PlaySFX("enemy_attack", position);
    }

    float droneHeight = 4.5f;
    switch (state) {
        case IDLE:
            position.y = droneHeight + sin(GetTime() * hoverSpeed) * hoverAmplitude;
            if (dist < detectionRange) { state = CHASE; stateTimer = 2.0f; }
            break;
        case CHASE: {
            // Variation based on spawn position
            float offset = (spawnPosition.x + spawnPosition.z) * 0.5f;
            float angle = (float)GetTime() * 0.5f + offset;
            Vector3 targetPos = { playerPos.x + cosf(angle) * 12.0f, droneHeight, playerPos.z + sinf(angle) * 12.0f };
            Vector3 moveDir = Vector3Normalize(Vector3Subtract(targetPos, position));
            position = Vector3Add(position, Vector3Scale(moveDir, speed * dt));
            if (stateTimer <= 0) { state = ATTACK; stateTimer = 3.0f; }
        } break;
        case ATTACK: {
            Vector3 dir = Vector3Normalize(toPlayer);
            Vector3 right = Vector3CrossProduct(dir, (Vector3){0, 1, 0});
            float strafeDir = (sinf(GetTime() * 2.0f) > 0) ? 1.0f : -1.0f;
            Vector3 moveVec = Vector3Scale(right, speed * 1.5f * strafeDir * dt);
            if (dist > 15.0f) moveVec = Vector3Add(moveVec, Vector3Scale(dir, speed * dt));
            if (dist < 10.0f) moveVec = Vector3Subtract(moveVec, Vector3Scale(dir, speed * dt));
            position = Vector3Add(position, moveVec);
            position.y = Lerp(position.y, droneHeight + sinf(GetTime() * 3.0f) * 0.8f, 2.0f * dt);
            if (stateTimer <= 0) state = CHASE;
        } break;
        default: break;
    }
}

void ShadowDrone::Draw() {
    if (state == BANISHED) return;
    if (state == BANISHING) { float alpha = banishTimer; DrawSphereWires(position, radius * (2.0f - alpha), 8, 8, Fade(GOLD, alpha)); return; }
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
    : Enemy(startPos, 80.0f, 12.0f, 20.0f, 0.5f, am) { 
    chargeCooldown = 2.0f; 
    chargeTimer = 0.0f;
    state = IDLE;
}

void Whisperer::Update(float dt, Player& player, LevelManager& levelManager, ParticleSystem& particleSystem) {
    if (flashTimer > 0) flashTimer -= dt; 
    if (stateTimer > 0) stateTimer -= dt;
    if (IsBanished()) {
        if (!isBanishing) { isBanishing = true; state = BANISHING; banishTimer = 1.0f; audioManager.PlaySFX("enemy_banished", position); }
    }
    if (state == BANISHING) {
        banishTimer -= dt;
        particleSystem.Emit(position, 5, LIME, WHITE, 0.2f, 0.5f, 0.5f, 1.0f, (Vector3){-1, 1, -1}, (Vector3){1, 2, 1});
        if (banishTimer <= 0) state = BANISHED;
        return; 
    }
    if (state == BANISHED) return;
    if (chargeTimer > 0) chargeTimer -= dt;

    HandlePlayerCollision(player, dt);

    Vector3 playerPos = player.GetPosition();
    Vector2 pos2D = { position.x, position.z };
    Vector2 pPos2D = { playerPos.x, playerPos.z };
    float dist2D = Vector2Distance(pos2D, pPos2D);

    if (position.y > 0.5f) position.y -= 15.0f * dt;
    if (position.y < 0.5f) position.y = 0.5f;

    switch (state) {
        case IDLE: if (dist2D < detectionRange) { state = CHASE; stateTimer = 1.0f; } break;
        case CHASE: {
            if (dist2D > 3.5f) {
                Vector3 toP = Vector3Normalize(Vector3Subtract(playerPos, position));
                Vector3 right = Vector3CrossProduct(toP, {0, 1, 0});
                float side = (spawnPosition.x > 0) ? 1.0f : -1.0f;
                Vector3 targetDir = Vector3Add(toP, Vector3Scale(right, side * 1.5f));
                position = Vector3Add(position, Vector3Scale(Vector3Normalize(targetDir), speed * dt));
            }
            if (dist2D < 6.0f && chargeTimer <= 0) { state = ATTACK; chargeTimer = 0.35f; isAttacking = false; }
        } break;
        case ATTACK:
            if (chargeTimer > 0) { position.x += sinf(GetTime() * 40.0f) * 0.08f; }
            else {
                Vector3 lungeDir = Vector3Normalize(Vector3Subtract(playerPos, position));
                position = Vector3Add(position, Vector3Scale(lungeDir, speed * 2.8f * dt));
                if (!isAttacking && Vector3Distance(position, playerPos) < 1.5f) { player.TakeDamage(18.0f); isAttacking = true; audioManager.PlaySFX("enemy_attack", position); }
                if (chargeTimer < -0.35f) { state = CHASE; chargeTimer = chargeCooldown; }
            }
            break;
        default: break;
    }
}

void Whisperer::Draw() {
    if (state == BANISHED) return;
    if (state == BANISHING) { float alpha = banishTimer; DrawCylinderWires(position, radius * (2.0f - alpha), radius * (2.0f - alpha), 1.5f, 8, Fade(GOLD, alpha)); return; }
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
    : Enemy(startPos, 350.0f, 6.0f, 30.0f, 1.5f, am) { 
    chargeWindup = 0.0f;
    chargeDuration = 0.0f;
    state = IDLE;
}

void TemptationBeast::Update(float dt, Player& player, LevelManager& levelManager, ParticleSystem& particleSystem) {
    if (flashTimer > 0) flashTimer -= dt; 
    if (IsBanished()) {
        if (!isBanishing) { isBanishing = true; state = BANISHING; banishTimer = 1.5f; audioManager.PlaySFX("enemy_banished", position); }
    }
    if (state == BANISHING) {
        banishTimer -= dt;
        particleSystem.Emit(position, 10, RED, GOLD, 0.5f, 1.0f, 0.5f, 1.5f, (Vector3){-2, 1, -2}, (Vector3){2, 3, 2});
        if (banishTimer <= 0) state = BANISHED; 
        return; 
    }
    if (state == BANISHED) return;
    HandlePlayerCollision(player, dt);
    Vector3 playerPos = player.GetPosition();
    Vector3 toPlayer = Vector3Subtract(playerPos, position);
    float dist = Vector3Length(toPlayer);
    if (position.y > 1.5f) position.y -= 20.0f * dt;
    if (position.y < 1.5f) position.y = 1.5f;
    switch (state) {
        case IDLE: if (dist < detectionRange) state = CHASE; break;
        case CHASE: {
            if (dist > 4.5f) {
                Vector3 dir = Vector3Normalize(toPlayer);
                position = Vector3Add(position, Vector3Scale(dir, speed * dt));
            }
            if (dist < 12.0f) { state = ATTACK; chargeWindup = 1.2f; isAttacking = false; }
        } break;
        case ATTACK: {
            if (chargeWindup > 0) {
                chargeWindup -= dt;
                Vector3 predictedPos = Vector3Add(playerPos, Vector3Scale(player.velocity, 0.5f));
                chargeDir = Vector3Normalize(Vector3Subtract(predictedPos, position));
                position.x += sinf(GetTime() * 50.0f) * 0.15f * dt;
                if (chargeWindup <= 0) { chargeDuration = 1.5f; audioManager.PlaySFX("enemy_attack", position); }
            } else if (chargeDuration > 0) {
                chargeDuration -= dt;
                position = Vector3Add(position, Vector3Scale(chargeDir, speed * 4.5f * dt)); 
                if (!isAttacking && dist < (radius + 1.5f)) { player.TakeDamage(35.0f); isAttacking = true; }
            } else { state = CHASE; stateTimer = 2.0f; }
        } break;
        default: break;
    }
}

void TemptationBeast::Draw() {
    if (state == BANISHED) return;
    if (state == BANISHING) { float alpha = banishTimer / 1.5f; DrawCubeWires(position, 3.0f, 3.0f, 3.0f, Fade(GOLD, alpha)); return; }
    Color color = (flashTimer > 0) ? WHITE : ((doubtMeter < maxDoubt / 2) ? PINK : MAROON);
    if (state == ATTACK && chargeWindup > 0) { float pulse = (sinf(GetTime() * 20.0f) + 1.0f) / 2.0f; color = ColorLerp(MAROON, RED, pulse); }
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
    : Enemy(startPos, 1000.0f, 4.0f, 60.0f, 4.0f, am) { 
    type = bossType; phase = 1; actionTimer = 2.0f; summonTimer = 5.0f;
    if (type == PRIDE) { maxDoubt = 1200.0f; speed = 4.0f; }
    else if (type == DESPAIR) { maxDoubt = 1500.0f; speed = 5.0f; radius = 5.0f; }
    else if (type == DEATH) { maxDoubt = 2500.0f; speed = 8.0f; radius = 2.5f; }
    doubtMeter = maxDoubt; state = CHASE;
}

void Boss::Update(float dt, Player& player, LevelManager& levelManager, ParticleSystem& particleSystem) {
    if (flashTimer > 0) flashTimer -= dt; 
    if (IsBanished()) {
        if (!isBanishing) { isBanishing = true; state = BANISHING; banishTimer = 3.0f; audioManager.PlaySFX("enemy_banished", position); }
    }
    if (state == BANISHING) {
        banishTimer -= dt;
        particleSystem.Emit(position, 20, GOLD, WHITE, 0.5f, 2.0f, 1.0f, 3.0f, (Vector3){-5, 0, -5}, (Vector3){5, 10, 5});
        if (banishTimer <= 0) state = BANISHED;
        return;
    }
    if (state == BANISHED) return;
    HandlePlayerCollision(player, dt);
    Vector3 playerPos = player.GetPosition();
    float dist = Vector3Distance(position, playerPos);
    if (type == PRIDE) {
        position.y = 3.0f; 
        if (actionTimer > 0) actionTimer -= dt;
        else {
            if (dist < 15.0f) { player.TakeDamage(30.0f); particleSystem.Emit(position, 100, PURPLE, BLACK, 1.0f, 5.0f, 0.5f, 1.5f, (Vector3){-8, 0, -8}, (Vector3){8, 1, 8}); audioManager.PlaySFX("enemy_attack", position); actionTimer = 2.5f; }
            else { Vector3 dir = Vector3Normalize(Vector3Subtract(playerPos, position)); position = Vector3Add(position, Vector3Scale(dir, speed * dt)); }
        }
        if (summonTimer > 0) summonTimer -= dt;
        else {
            for(int i=0; i<3; i++) { float a = (float)i * 120.0f * DEG2RAD; Vector3 sd = {cosf(a), 0, sinf(a)}; levelManager.AddProjectile(position, Vector3Scale(sd, 25.0f), 0.8f, 5.0f, false); }
            summonTimer = 5.0f;
        }
    } else if (type == DESPAIR) {
        position.y = 6.0f;
        if (actionTimer > 0) actionTimer -= dt;
        else {
            for (int i=0; i<16; i++) { float a = i * 22.5f * DEG2RAD; Vector3 d = { sinf(a), -0.3f, cosf(a) }; levelManager.AddProjectile(position, Vector3Scale(d, 12.0f), 0.6f, 6.0f, false); }
            audioManager.PlaySFX("enemy_attack", position); actionTimer = 1.8f;
        }
        Vector3 dir = Vector3Normalize(Vector3Subtract(playerPos, position)); position = Vector3Add(position, Vector3Scale(dir, speed * dt));
    } else if (type == DEATH) {
        if (state == CHASE) {
            Vector3 dir = Vector3Normalize(Vector3Subtract(playerPos, position)); position = Vector3Add(position, Vector3Scale(dir, speed * dt));
            if (dist < 12.0f) { state = ATTACK; actionTimer = 0.3f; }
        } else if (state == ATTACK) {
            actionTimer -= dt;
            if (actionTimer <= 0) {
                Vector3 d = Vector3Normalize(Vector3Subtract(playerPos, position)); position = Vector3Add(position, Vector3Scale(d, speed * 6.0f * dt));
                if (dist < 4.0f) { player.TakeDamage(60.0f); state = CHASE; }
                if (dist > 25.0f) state = CHASE;
            }
        }
    }
}

void Boss::Draw() {
    if (state == BANISHED) return;
    if (state == BANISHING) { float alpha = banishTimer / 3.0f; DrawSphereWires(position, radius * (2.0f - alpha), 16, 16, Fade(GOLD, alpha)); return; }
    Color color = (flashTimer > 0) ? WHITE : DARKGRAY;
    if (type == PRIDE) color = (flashTimer > 0) ? WHITE : PURPLE;
    if (type == DESPAIR) color = (flashTimer > 0) ? WHITE : DARKBLUE;
    if (type == DEATH) color = (flashTimer > 0) ? WHITE : BLACK;
    if (type == PRIDE) {
        DrawCylinder(position, radius, radius, 8.0f, 8, color);
        DrawSphere(Vector3Add(position, {0, 8.5f, 0}), 3.0f, color);
        DrawCylinderWires(Vector3Add(position, {0, 5.0f, 0}), 2.0f, 3.0f, 1.5f, 6, GOLD);
    } else if (type == DESPAIR) {
        DrawSphere(position, radius * 0.6f, Fade(color, 0.9f));
        for(int i=0; i<16; i++) { float t = GetTime() * 2.5f + i; Vector3 op = Vector3Add(position, {sinf(t)*5.0f, cosf(t*0.5f)*3.0f, cosf(t)*5.0f}); DrawCube(op, 1.2f, 1.2f, 1.2f, Fade(SKYBLUE, 0.7f)); }
    } else if (type == DEATH) {
        DrawCylinder(Vector3Subtract(position, {0, 4.0f, 0}), 2.0f, 3.0f, 10.0f, 8, color);
        DrawSphere(Vector3Add(position, {0, 4.5f, 0}), 2.0f, color);
        Vector3 hand = Vector3Add(position, {2.5f, 2.0f, 1.5f});
        DrawCylinderEx(hand, Vector3Add(hand, {0, 6.0f, 1.5f}), 0.15f, 0.15f, 4, GRAY);
        Vector3 bb = Vector3Add(hand, {0, 5.5f, 1.5f}); DrawCylinderEx(bb, Vector3Add(bb, {-2.5f, 1.0f, 0}), 0.1f, 0.6f, 4, LIGHTGRAY);
    }
    Vector3 mp = Vector3Add(position, (Vector3){0.0f, 10.0f, 0.0f});
    DrawCube(mp, 8.0f, 0.8f, 0.1f, BLACK);
    DrawCube((Vector3){mp.x - (8.0f - (8.0f * (doubtMeter/maxDoubt))) / 2.0f, mp.y, mp.z}, (8.0f * (doubtMeter/maxDoubt)), 0.7f, 0.15f, RED);
}
