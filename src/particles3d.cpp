#include "particles3d.h"

// Helper function to get random float within a range
float ParticleSystem::GetRandomFloat(float min, float max) {
    return min + (float)GetRandomValue(0, 10000) / 10000.0f * (max - min);
}

// Constructor
ParticleSystem::ParticleSystem() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
    nextParticleIndex = 0;
}

// Emit particles from a source
void ParticleSystem::Emit(Vector3 position, int count, Color startColor, Color endColor,
                          float startSize, float endSize, float minLifetime, float maxLifetime,
                          Vector3 minVelocity, Vector3 maxVelocity) {
    for (int i = 0; i < count; i++) {
        // Find an inactive particle
        int particleIndex = -1;
        for (int j = 0; j < MAX_PARTICLES; j++) {
            if (!particles[nextParticleIndex].active) {
                particleIndex = nextParticleIndex;
                break;
            }
            nextParticleIndex = (nextParticleIndex + 1) % MAX_PARTICLES;
        }

        if (particleIndex != -1) {
            Particle* p = &particles[particleIndex];
            p->position = position;
            p->velocity = (Vector3){GetRandomFloat(minVelocity.x, maxVelocity.x),
                                    GetRandomFloat(minVelocity.y, maxVelocity.y),
                                    GetRandomFloat(minVelocity.z, maxVelocity.z)};
            p->startColor = startColor;
            p->endColor = endColor;
            p->currentColor = startColor;
            p->startSize = startSize;
            p->endSize = endSize;
            p->currentSize = startSize;
            p->lifetime = GetRandomFloat(minLifetime, maxLifetime);
            p->age = 0.0f;
            p->active = true;

            nextParticleIndex = (particleIndex + 1) % MAX_PARTICLES;
        }
    }
}

// Update all active particles
void ParticleSystem::Update(float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            Particle* p = &particles[i];
            p->age += dt;

            if (p->age >= p->lifetime) {
                p->active = false; // Particle dies
                continue;
            }

            // Apply gravity (simple for now)
            p->velocity.y -= 9.8f * dt;

            // Update position
            p->position = Vector3Add(p->position, Vector3Scale(p->velocity, dt));

            // Lerp color
            float t = p->age / p->lifetime;
            p->currentColor.r = (unsigned char)Lerp(p->startColor.r, p->endColor.r, t);
            p->currentColor.g = (unsigned char)Lerp(p->startColor.g, p->endColor.g, t);
            p->currentColor.b = (unsigned char)Lerp(p->startColor.b, p->endColor.b, t);
            p->currentColor.a = (unsigned char)Lerp(p->startColor.a, p->endColor.a, t);

            // Lerp size
            p->currentSize = Lerp(p->startSize, p->endSize, t);
        }
    }
}

// Draw all active particles
void ParticleSystem::Draw() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            Particle* p = &particles[i];
            // For now, drawing spheres. Will switch to billboards or custom shader later.
            DrawSphere(p->position, p->currentSize, p->currentColor);
        }
    }
}
