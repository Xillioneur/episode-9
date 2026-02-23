#ifndef PARTICLES_3D_H
#define PARTICLES_3D_H

#include "raylib.h"
#include "raymath.h"
#include <vector>

#define MAX_PARTICLES 1000 // As specified in GDD (1000+ billboards)

// Particle structure
struct Particle {
    Vector3 position;
    Vector3 velocity;
    Color startColor;
    Color endColor;
    Color currentColor;
    float startSize;
    float endSize;
    float currentSize;
    float lifetime;
    float age;
    bool active;
};

// ParticleSystem class
class ParticleSystem {
public:
    Particle particles[MAX_PARTICLES];
    int nextParticleIndex; // To efficiently find inactive particles

    ParticleSystem();

    // Emit particles from a source
    void Emit(Vector3 position, int count, Color startColor, Color endColor,
              float startSize, float endSize, float minLifetime, float maxLifetime,
              Vector3 minVelocity, Vector3 maxVelocity);

    // Update all active particles
    void Update(float dt);

    // Draw all active particles
    void Draw();

private:
    // Helper to get random float within a range
    float GetRandomFloat(float min, float max);
};

#endif // PARTICLES_3D_H
