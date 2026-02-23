**Glory’s Triumph 3D – Lenten Warfare**  
**Game Design Document (GDD)**

**Document Version:** 1.1 (Gamepad Support Removed)  
**Date:** February 21, 2026  
**Prepared by:** Grok (Lead Designer & Tutorial Architect)  
**Based on:** GRS v1.4 (Locked)  
**Status:** Approved for Raylib C++ Implementation – Episode 9 Tutorial Series

-----

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [Game Overview](#game-overview)
3. [Story and Narrative](#story-and-narrative)
4. [Gameplay Mechanics](#gameplay-mechanics)
5. [World and Level Design](#world-and-level-design)
6. [Art and Visual Style](#art-and-visual-style)
7. [Audio Design](#audio-design)
8. [User Interface and Experience](#user-interface-and-experience)
9. [Progression and Replayability](#progression-and-replayability)
10. [Technical Implementation Guide](#technical-implementation-guide)
11. [Monetization and Distribution](#monetization-and-distribution)
12. [Episode 9 Tutorial Learning Goals](#episode-9-tutorial-learning-goals)
13. [Appendix: Key Assets List](#appendix)

-----

## Executive Summary

*Glory’s Triumph 3D* is a fully-featured 3D Action-RPG that immerses players in a 15-day Lenten journey of spiritual warfare and divine glory. As the Light Bearer, players explore breathtaking 3D realms, engage in non-violent combat against manifestations of doubt, and unleash spectacular Glory abilities that transform the world in real-time — blooming flowers, piercing heavenly beams, and triumphant particle cascades all declaring God’s mighty power.

Built exclusively with Raylib and C++ for Episode 9 of the tutorial series, the game delivers 2–4 hours of polished, family-safe gameplay with RPG progression, a hub world, save system, and multiple endings. Combat feels action-packed through fluid movement, combos, and awe-inspiring visuals, but remains 100% violence-free: enemies are banished by light, never harmed.

**High Concept:** Spiritual Zelda meets Journey — light conquers darkness in a Lenten odyssey of faith.  
**Target Playtime:** 2–4 hours (core) + replay for collectibles.  
**Unique Selling Points:** Biblically inspired narrative, non-violent spectacle combat, Raylib-powered 3D glory effects.

-----

## Game Overview

|**Aspect**      |**Details**                                                           |
|----------------|----------------------------------------------------------------------|
|**Genre**       |3D Third-Person Action-RPG (Spiritual Warfare)                        |
|**Perspective** |Third-person over-the-shoulder                                        |
|**Platforms**   |PC (Windows/Linux/macOS via Raylib)                                   |
|**Player Count**|Single-player                                                         |
|**Playtime**    |2–4 hours                                                             |
|**Rating**      |Everyone (E) – 100% family-safe                                       |
|**Core Theme**  |God’s Glory & Might through Lent: Prayer, Fasting, Repentance, Victory|

**Tagline:** “Banished by Light, Transformed by Grace – Walk the Path of Eternal Triumph!”

-----

## Story and Narrative

### Narrative Arc

The game unfolds over **15 Lenten Days**, each themed around a biblical virtue or trial (e.g., Day 1: Repentance, Day 7: Temptation, Day 15: Resurrection).

- **Prologue:** Angelic voice-over summons the player as the **Light Bearer** to restore a fading world shrouded in “Shadows of Doubt.”
- **Rising Action:** Traverse darkening realms, banish spiritual foes, collect Scripture Fragments revealing God’s promises.
- **Climax:** Day 15 Easter Boss – the **Spirit of Death** – defeated in a multi-phase glory symphony.
- **Endings:** 3 based on Grace Score: “Faithful Victory” (full glory), “Humble Journey” (partial), “Call to Return” (low – replay encouraged).

### Key Characters

- **Player (Light Bearer):** Silent protagonist, robed figure with glowing staff.
- **Angelic Guide (Seraphina):** Companion NPC – provides hints, voice-overs, follows in hub.
- **Converted Allies:** Defeated enemies who repent and aid (e.g., former Whisperers sing praises).
- **Bosses:** Pride (arrogant giant), Despair (storm cloud entity), Flesh (tempting beast) – narrate temptations via ethereal whispers.

**Delivery:** World-space subtitles, cinematic particle cutscenes, collectible scrolls with KJV-inspired verses (e.g., John 1:5 – “The light shines in the darkness, and the darkness has not overcome it.”).

-----

## Gameplay Mechanics

### Core Loop

1. **Explore** 3D realm (fluid movement, platforming).
2. **Engage** spiritual enemies blocking paths.
3. **Combat** – Chain Glory abilities to fill Doubt Meters.
4. **Transform** – Abilities reshape environment (paths open, lights bloom).
5. **Progress** – Reach Altar for Virtue upgrade + next Day.

### Controls

|**Input**     |**Keyboard / Mouse**|**Action**              |
|--------------|--------------------|------------------------|
|Move          |WASD                |3D Acceleration/Friction|
|Camera/Aim    |Mouse               |Smooth Orbit/Lock-on    |
|Jump/Double   |Space               |Leap of Faith + Glide   |
|Sprint/Dash   |Shift / LShift      |Speed Burst (Cooldown)  |
|Primary Attack|LMB                 |Glory Beam              |
|Melee         |RMB                 |Light Blade Sweep       |
|Area Burst    |E                   |Prayer Burst            |
|Defense       |Q                   |Faith Shield            |
|Lock-on       |Tab                 |Target Nearest Enemy    |
|Interact      |F                   |Pickups/Altar           |

### Spiritual Combat System

- **Enemy AI:** Patrol → Detect → Attack patterns (projectiles, rushes, summons). Simple state machine: Idle, Chase, Attack, Flee (low Doubt).
- **Doubt Meter:** Visible above enemies (fills with Glory hits → shatter → banish/convert).
- **Player Faith Meter:** Drains from hits → 0 = respawn at Prayer Stone + scripture encouragement. No permadeath.
- **Combos:** Chain hits (e.g., Beam → Blade → Burst) for Glory Multiplier → bigger particles/lights.
- **Enemy Types:**
1. **Shadow Drones** (Fly, shoot doubt orbs – weak to Beam).
2. **Whisperers** (Ground rush – convert easily to allies).
3. **Temptation Beasts** (Charge – high Doubt, drop Offerings).
4. **Doubt Towers** (Spawn minions – Burst clears).
5. **Storm Wraiths** (Aerial, wind knockback).
6. **Strongholds** (Mini-boss: multi-phase).
- **Boss Fights:** Pattern-based (dodge temptations, counter with charged Ultimate).

### Glory Ability System

|**Ability**    |**Unlock**|**Effect**                                    |**Spectacle**                      |
|---------------|----------|----------------------------------------------|-----------------------------------|
|Glory Beam     |Start     |Ranged chain light projectile                 |Piercing beam + particle trail     |
|Prayer Burst   |Day 3     |Charged AoE sphere                            |Expanding light dome + screen flash|
|Light Blade    |Day 5     |Melee arc sweep                               |Golden slash + enemy stagger       |
|Blessing Wave  |Day 7     |Ground ripple (lifts platforms, converts weak)|Flower bloom chain + ally cheers   |
|Faith Shield   |Day 10    |Block/reflect (10s duration)                  |Radiant dome + reflect bursts      |
|Transfiguration|Day 12    |Ultimate (full meter) – 20s god-mode          |Screen heaven + mega particles     |

**Cooldowns:** 2–10s, reduced by Virtues.

### Movement & Platforming

- Acceleration: 10 units/s² to 20 max speed.
- Jump: 8 units high; Double: +glide (air control).
- Environmental Puzzles: Use Wave to raise islands, Beam to light dark paths.

-----

## World and Level Design

- **Hub:** Garden of Reflection (open 3D area) – fast-travel to Days, view gallery of glory moments, talk to allies.
- **15 Days:** Modular arenas (100x100x50 units).
  
  |**Day Range**|**Theme**|**Key Features**             |
  |-------------|---------|-----------------------------|
  |1–5          |Awakening|Basic combat, open meadows   |
  |6–10         |Trials   |Platforms, enemy waves       |
  |11–15        |Triumph  |Boss arenas, combo challenges|
- **Dynamic World:** Cleared areas stay lit/bloomed (saved persistently).
- **Scale:** Player ~2 units tall; levels load/unload seamlessly.

-----

## Art and Visual Style

- **Style:** Low-poly vibrant fantasy (inspired by *Wind Waker* + *Journey*). Bright palettes shift from dim grays to golden radiance.
- **Key Effects:**
  - 3D Particles: 1000+ billboards (velocity, gravity, color lerp).
  - Dynamic Lights: 5–10 per scene (sun + ability spots).
  - Post-Process: Simple bloom shader (rlgl).
  - Animations: 8–12 frame cycles (idle, walk, attack).
- **Models:** OBJ/IQM (player: 1k tris; enemies: 500–2k tris).

-----

## Audio Design

- **Music:** Dynamic orchestral worship (4 layers: ambient → tension → glory swell → triumph choir). Raylib music streaming.
- **SFX:** 50+ (whoosh beams, shatter doubts, bloom chimes) – 3D spatial.
- **Voice:** Angelic whispers/subtitles (text-to-speech optional or recorded).
- **Dynamic:** Pitch rise on charges; volume swell on combos.

-----

## User Interface and Experience

- **HUD:** Minimal – Faith bar, Ability icons (cooldown pies), Enemy lock-on reticle, Day counter.
- **Menus:**
  - Main: New Game, Load, Options, Credits (raygui).
  - Pause: Resume, Map, Stats, Quit.
  - Post-Day: Virtue selection wheel.
- **Accessibility:** Remap keys, color-blind shaders, subtitles, reduced motion.
- **Tutorials:** Contextual pop-ups (e.g., “Hold to charge Glory!”).

-----

## Progression and Replayability

- **Grace Level:** 1–15 (XP from combats/collectibles).
- **Virtue Tree:** 3 branches x 4 tiers (e.g., Faith: +Beam range).
- **Collectibles:** 150+ (Scriptures unlock lore; Orbs boost score).
- **Replay:** New Game+, hub challenges, endings hunt.

-----

## Technical Implementation Guide

- **Raylib Features:**
  - 3D: LoadModel, DrawModelEx, Camera3D (custom follow/lock).
  - Collision: RayCollision, BoundingBox.
  - Particles: Custom struct array (update/draw loop).
  - Shaders: Basic glow (pass 1: scene, pass 2: additive).
  - States: Enum (MENU=0, HUB=1, DAY_X=2+X).
- **Code Tips:**
  
  ```cpp
  // EPISODE 9: Update particles
  for(int i=0; i<MAX_PARTICLES; i++) {
      if(particles[i].active) {
          particles[i].pos = Vector3Add(particles[i].pos, particles[i].vel);
          // ... fade logic
      }
  }
  ```
- **Build:** CMake, assets in /resources/.

-----

## Monetization and Distribution

- **Free Release:** Itch.io, GitHub (source + binary).
- **Optional:** Donations for expansions (40 Days DLC). No MTX/ads.

-----

## Episode 9 Tutorial Learning Goals

1. Advanced 3D camera/lock-on.
2. Enemy AI & combat state machines.
3. Dynamic particles/lighting.
4. Save persistence & hubs.
5. Polish: Menus, audio, effects.

**Total LOC Estimate:** 5k–8k (modular classes).

-----

## Appendix: Key Assets List

- **Models (15):** player.iqm, shadow_drone.obj, etc.
- **Textures (30):** diffuse/normal PBR packs.
- **Audio (50):** sfx/ogg, music/ogg.
- **Levels:** JSON for spawn positions.