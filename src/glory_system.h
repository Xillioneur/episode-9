#ifndef GLORY_SYSTEM_H
#define GLORY_SYSTEM_H

#include "raylib.h"
#include "raymath.h"
#include "enemy.h" // To interact with enemies
#include "camera3d.h" // To get camera information for raycasting
#include "particles3d.h" // Include ParticleSystem
#include <vector>
#include <memory> // For unique_ptr

// Forward declaration
class AudioManager; 

class GlorySystem {
public:
    float gloryBeamCooldown;
    float gloryBeamTimer;
    float gloryBeamDamage;

    float prayerBurstCooldown;
    float prayerBurstTimer;
    float prayerBurstDamage;
    float prayerBurstRadius;

    float lightBladeCooldown;
    float lightBladeTimer;
    float lightBladeDamage;
    float lightBladeRange;
    float lightBladeArcAngle; // In degrees

    GlorySystem(ParticleSystem& ps, AudioManager& am); // Constructor now takes ParticleSystem and AudioManager reference
    void Update(float dt, Camera3D_Custom& camera, Player& player, std::vector<std::unique_ptr<Enemy>>& enemies);
    void Draw(Camera3D_Custom& camera); // Pass camera for 3D drawing

private:
    ParticleSystem& particleSystem; // Reference to the particle system
    AudioManager& audioManager;     // Reference to the audio manager

    // For Glory Beam visual feedback
    bool drawingGloryBeam;
    Vector3 gloryBeamStartPos;
    Vector3 gloryBeamEndPos;
    float gloryBeamDrawTimer; // How long to draw the beam

    // For Light Blade visual feedback
    bool drawingLightBlade;
    Vector3 lightBladeDrawPos;
    Vector3 lightBladeDrawForward; // Direction of the blade arc
    float lightBladeDrawTimer; // How long to draw the arc
};

#endif // GLORY_SYSTEM_H
