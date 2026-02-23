#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "raymath.h"
#include <string>

// Forward declarations
class AudioManager; 
class Player;
class LevelManager;
class ParticleSystem;

// Define enemy states
typedef enum EnemyState {
    IDLE = 0,
    PATROL,
    CHASE,
    ATTACK,
    BANISHED,
    BANISHING // New state for animation
} EnemyState;

class Enemy {
public:
    Vector3 position;
    Vector3 spawnPosition; 
    Vector3 velocity;
    float doubtMeter;
    float maxDoubt;
    float speed;
    EnemyState state;
    float detectionRange; 
    float radius;         
    float flashTimer;     
    bool isAttacking;     

    float banishTimer;
    bool isBanishing;

    float stateTimer; // For AI decision making

    // Constructor
    Enemy(Vector3 startPos, float initialDoubt, float moveSpeed, float range, float rad, AudioManager& am);

    // Virtual destructor
    virtual ~Enemy() = default;

    // Pure virtual methods to be implemented by derived classes
    virtual void Update(float dt, Player& player, LevelManager& levelManager, ParticleSystem& particleSystem) = 0;
    virtual void Draw() = 0;

    // Common methods
    void TakeGloryHit(float damage);
    bool IsBanished(); // Returns true if doubt <= 0
    bool ReadyToRemove(); // Returns true if animation complete

protected: // Make audioManager protected so derived classes can access it
    AudioManager& audioManager;
};

// ShadowDrone specific class
class ShadowDrone : public Enemy {
public:
    float hoverAmplitude;
    float hoverSpeed;
    float attackRange; 

    float shootTimer;
    float shootCooldown;

    ShadowDrone(Vector3 startPos, AudioManager& am);
    ~ShadowDrone() = default;

    void Update(float dt, Player& player, LevelManager& levelManager, ParticleSystem& particleSystem) override;
    void Draw() override;
};

// Whisperer specific class (Ground melee rusher)
class Whisperer : public Enemy {
public:
    float chargeCooldown;
    float chargeTimer;

    Whisperer(Vector3 startPos, AudioManager& am);
    ~Whisperer() = default;

    void Update(float dt, Player& player, LevelManager& levelManager, ParticleSystem& particleSystem) override;
    void Draw() override;
};

// TemptationBeast specific class (Tanky charger)
class TemptationBeast : public Enemy {
public:
    float chargeWindup;
    float chargeDuration;
    Vector3 chargeDir;

    TemptationBeast(Vector3 startPos, AudioManager& am);
    ~TemptationBeast() = default;

    void Update(float dt, Player& player, LevelManager& levelManager, ParticleSystem& particleSystem) override;
    void Draw() override;
};

// Boss Types
typedef enum BossType {
    PRIDE,
    DESPAIR,
    DEATH
} BossType;

// Boss Class
class Boss : public Enemy {
public:
    BossType type;
    float actionTimer;
    float summonTimer;
    int phase; // 1, 2, 3

    Boss(Vector3 startPos, BossType bossType, AudioManager& am);
    ~Boss() = default;

    void Update(float dt, Player& player, LevelManager& levelManager, ParticleSystem& particleSystem) override;
    void Draw() override;
};

#endif // ENEMY_H
