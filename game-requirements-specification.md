**Game Requirements Specification (GRS) Document – Episode 9 Final Edition**

**Project Title:** Glory’s Triumph 3D – Lenten Warfare  
**Document Version:** 1.4 (Fully Featured Edition)  
**Date:** February 21, 2026  
**Prepared by:** Grok (Team Lead)  
**Status:** Approved baseline for Game Design Document (GDD) and Raylib C++ implementation

-----

### 1. Introduction

#### 1.1 Purpose

This GRS establishes the complete, fully-featured requirements for **Glory’s Triumph 3D**, a 3D Action-RPG built exclusively with Raylib and C++ as Episode 9 of the Raylib C++ tutorial series.

The game is **heavily inspired by God and the Lenten season**. It is **action-packed** with real-time spiritual combat, enemy encounters, and boss battles — all expressed **100 % non-violently** to glorify God’s mighty power, light overcoming darkness, mercy, and victory through faith. Enemies are manifestations of doubt, temptation, and spiritual strongholds; they are **banished by divine light and praise**, never harmed with blood, weapons, or death animations. When defeated they dissolve into radiant particles or repent and become allies.

This version restores every “good part of a game” while remaining tutorial-appropriate for intermediate-to-advanced learners (Episode 9).

#### 1.2 Scope

- Full campaign: **15 Lenten “Days”** (≈ 2–4 hours playtime, modular for expansion to 40).
- Complete RPG systems: progression, abilities, collectibles, light inventory, hub world.
- Spiritual combat system with 6 enemy types + 3 bosses.
- 3D third-person exploration, platforming, and spectacle-driven glory moments.
- Save system, main menu, options, credits, pause, multiple endings (based on Grace score).

**Out of scope:** Blood/gore, permanent death, multiplayer, advanced physics (Raylib built-in is sufficient).

-----

### 2. Game Overview

#### 2.1 Concept

You are the **Light Bearer**, summoned by an angelic companion to walk the 15 symbolic days of Lent. The world begins shrouded in shadows of doubt. Through fluid 3D movement, charged Glory abilities, and real-time spiritual combat you flood the land with God’s light — rivers sparkle, dead trees bloom instantly, mountains shine with heavenly beams. Every banishment and every glory burst is a visible declaration of God’s glory and might. The journey culminates in a triumphant Easter sunrise on Day 15.

Tone: Joyful reverence, empowering awe, triumphant worship, and victorious faith.

#### 2.2 Genre

3D Third-Person Action-RPG with non-violent spiritual warfare.

#### 2.3 Target Audience

Christian gamers, families, youth groups, ages 10+. Perfect for church events or inspirational play.

#### 2.4 Platforms

Windows, Linux, macOS (Raylib native).

-----

### 3. Technical Requirements (Episode 9 – Fully Featured)

- **Language:** C++20 (classes, smart pointers, enums, structs).
- **Engine:** Pure Raylib 5.5+ (rlgl, raymath, raygui for menus).
- **Project Structure (tutorial-ready):**
  - `main.cpp` + state machine
  - `player.h/cpp`, `camera3d.h/cpp`, `enemy.h/cpp`, `glory_system.h/cpp`, `level_manager.h/cpp`, `particles3d.h/cpp`, `ui.h/cpp`, `save.h/cpp`, `audio_manager.h/cpp`
- **Graphics:**
  - 1280×720 base, scalable to 4K.
  - Third-person camera (smooth follow, shoulder switch, lock-on during combat).
  - Low-poly models + basic skeletal animation (player, enemies, altars).
  - Dynamic day/night cycle + glory lighting (multiple point/spot lights + simple custom shader for radiant glow).
- **Audio:** Full 3D spatial sound, dynamic music (peace → swelling orchestral + choir during glory/combat peaks).
- **Input:** Keyboard + full gamepad + mouse aim support.
- **Performance:** 60 FPS locked on mid-range PCs.
- **Save:** JSON binary save with Raylib file I/O (progress, Grace, collectibles).

-----

### 4. Functional Requirements – Fully Featured Game

#### 4.1 Core Loop

Explore beautiful 3D Lenten realms → Encounter spiritual enemies → Engage in action-packed non-violent combat → Unleash Glory abilities → Witness environment transformation (blooming, light beams, repentant enemies) → Reach Altar → Scripture + Virtue upgrade → Next Day.

#### 4.2 Player Controls & Movement (Action-Packed)

- Fluid 3D movement with acceleration, sprint, dodge roll.
- Leap of Faith (double-jump + glide).
- Spirit Dash (short cooldown burst).
- Combat lock-on (tab or right-stick click).

#### 4.3 Glory Ability System (6 unlockable, combo-driven)

1. **Glory Beam** (hold + aim) – ranged light projectile, chains between enemies.
2. **Prayer Burst** (charge & release) – expanding 3D sphere banishes groups.
3. **Light Blade** (melee swing) – sweeping arc of holy light, no blood.
4. **Blessing Wave** (area) – lifts platforms, heals land, converts weak enemies.
5. **Faith Shield** (defensive) – blocks doubt projectiles, reflects as light.
6. **Ultimate Transfiguration** (meter fill) – temporary god-mode with screen-filling heavenly effects.

All abilities trigger 500–1000+ 3D particles, dynamic lights, screen flash, and environmental reactions.

#### 4.4 Spiritual Combat System (Non-Violent)

- **Enemies (6 types):**
  - Shadow Drones (flying, shoot doubt orbs).
  - Whisperers (melee rush, apply slow).
  - Temptation Beasts (charge, knockback).
  - Doubt Towers (stationary, spawn minions).
  - Storm Wraiths (aerial bosses-lite).
  - Final strongholds (mini-bosses).
- **Combat Mechanics:**
  - Enemies have “Doubt Meter”. Fill with Glory hits → meter breaks → enemy dissolves into golden particles or kneels and becomes an ally that follows and praises.
  - No health bars on player; “Faith Meter” drains from doubt attacks. At 0 you are gently returned to last Prayer Stone with a short encouraging scripture (“Be strong and courageous…”).
  - Combo system: chain abilities for multipliers and bigger glory spectacles.
- **Bosses (3):** Spirit of Pride, Spirit of Despair, Spirit of Flesh – each defeated through unique faith patterns + massive glory finale.

#### 4.5 Progression & RPG Systems

- **Grace Level** (1–15, one per Lenten Day).
- **Virtue Tree** (Faith, Hope, Love branches) – 12 unlockable passives & ability upgrades.
- **Collectibles:** 80+ Scripture Fragments, Praise Orbs, Offering Chests.
- **Hub World:** Central Garden of Reflection – return anytime to replay days or view collected glory highlights.
- **Light Inventory:** 8 slots (healing manna, temporary power scrolls).

#### 4.6 World & Levels

- 15 hand-crafted 3D levels (each 100×100+ units) + 1 open hub.
- Progressive complexity: early days = exploration & simple combat; later days = multi-stage arenas requiring ability combos and environmental interaction.
- Real-time environment changes: cleared areas stay bright forever (persistent world state saved).

#### 4.7 UI / Menus / Polish

- Elegant 3D HUD (Grace meter, ability icons, lock-on reticle, day counter).
- Full main menu, options (graphics, audio, controls rebinding), credits with scripture.
- Pause menu with map and collected verses.
- Cinematic glory cutscenes (particle-driven, no video files needed).

#### 4.8 Audio & Visual Spectacle

- Every combat victory and ability use must trigger at least one awe-inspiring glory moment (heavenly beams, screen shake of joy, choir swell, petal rain).
- Original epic worship-orchestral score with dynamic layering.

-----

### 5. Non-Functional Requirements

- 100 % family-safe, zero violence, biblically accurate text (no denominational bias).
- Every 5 minutes of gameplay contains at least one spectacular divine manifestation.
- Code is heavily commented with “// EPISODE 9 LEARNING GOAL: …” notes.
- Full accessibility: color-blind modes, large text, controller remapping.
- Must compile from clean Raylib template.

-----

**Episode 9 Learning Outcomes (Fully Featured)**

- Advanced 3D camera & lock-on
- Entity-component style enemy AI
- Real-time combat loop & combo system
- Dynamic environment interaction & persistence
- Particle & lighting mastery
- Save system & state management
- Menu systems with raygui
- Professional polish & spectacle design