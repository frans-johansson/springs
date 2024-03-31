#include "raylib.h"
#include "raymath.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define FORCES_CAPACITY 64
#define SYSTEM_CAPACITY 128
#define MASS_RADIUS 5.0f
#define MASS_COLOR_SCALE 50.0f
#define TIME_SCALE 5.0f
#define GRAVITATIONAL_ACCELERATION                                             \
  (Vector2) { 0.0f, 9.8f }

Color color_lerp(Color c1, Color c2, float amount) {
  Vector4 v1 = (Vector4){c1.r, c1.g, c1.b, c1.a};
  Vector4 v2 = (Vector4){c2.r, c2.g, c2.b, c2.a};
  Vector4 lerped = Vector4Lerp(v1, v2, Clamp(amount, 0.0f, 1.0f));
  return (Color){lerped.x, lerped.y, lerped.z, lerped.w};
}

typedef struct {
  Vector2 position;
  Vector2 velocity;
  Vector2 acceleration;
  Vector2 forces[FORCES_CAPACITY];
  size_t force_count;
  float mass;
  _Bool fixed;
} Mass;

void mass_force_append(Mass *mass, Vector2 force) {
  if (mass->force_count >= FORCES_CAPACITY) {
    printf("ERROR: Cannot append any more forces to mass\n");
    return;
  }

  mass->forces[mass->force_count++] = force;
}

void mass_reset_forces(Mass *mass) { mass->force_count = 0; }

void mass_update(Mass *mass, float dt) {
  if (mass->fixed) {
    return;
  }
  mass->acceleration = GRAVITATIONAL_ACCELERATION;
  for (size_t i = 0; i < mass->force_count; ++i) {
    Vector2 acceleration_from_force =
        Vector2Scale(mass->forces[i], 1 / mass->mass);
    mass->acceleration =
        Vector2Add(mass->acceleration, acceleration_from_force);
  }
  mass->velocity =
      Vector2Add(mass->velocity, Vector2Scale(mass->acceleration, dt));
  mass->position = Vector2Add(mass->position, Vector2Scale(mass->velocity, dt));
}

void mass_draw(Mass *mass) {
  Color c =
      color_lerp(BLUE, RED, Vector2Length(mass->velocity) / MASS_COLOR_SCALE);
  DrawCircleV(mass->position, MASS_RADIUS, c);
}

typedef struct {
  Mass *first;
  Mass *second;
  float length;
  float strength;
  float dampening;
} Spring;

void spring_update(Spring *spring) {
  Vector2 span =
      Vector2Subtract(spring->second->position, spring->first->position);
  Vector2 force_direction = Vector2Normalize(span);

  // Spring force
  float displacement = spring->length - Vector2Length(span);
  mass_force_append(
      spring->first,
      Vector2Scale(force_direction, spring->strength * -displacement));
  mass_force_append(
      spring->second,
      Vector2Scale(force_direction, spring->strength * displacement));

  // Dampener force
  float displacement_rate_first =
      Vector2DotProduct(spring->first->velocity, force_direction);
  float displacement_rate_second =
      -Vector2DotProduct(spring->second->velocity, force_direction);
  float displacement_rate = displacement_rate_first + displacement_rate_second;
  mass_force_append(
      spring->first,
      Vector2Scale(force_direction, spring->dampening * -displacement_rate));
  mass_force_append(
      spring->second,
      Vector2Scale(force_direction, spring->dampening * displacement_rate));
}

void spring_draw(Spring *spring) {
  Vector2 span =
      Vector2Subtract(spring->second->position, spring->first->position);
  float relative_displacement =
      (spring->length - Vector2Length(span)) / Vector2Length(span);
  Color c;
  if (relative_displacement < 0.0f) {
    c = color_lerp(WHITE, RED, -relative_displacement);
  } else {
    c = color_lerp(WHITE, BLUE, relative_displacement);
  }

  DrawLineV(spring->first->position, spring->second->position, c);
}

typedef struct {
  Mass masses[SYSTEM_CAPACITY];
  Spring springs[SYSTEM_CAPACITY];
  size_t mass_count;
  size_t spring_count;
} System;

void system_add_mass(System *system, Mass mass) {
  if (system->mass_count >= SYSTEM_CAPACITY) {
    printf("ERROR: Cannot add more masses to the system\n");
    return;
  }

  system->masses[system->mass_count++] = mass;
}

void system_add_spring(System *system, Spring spring, size_t m1, size_t m2) {
  if (system->spring_count >= SYSTEM_CAPACITY) {
    printf("ERROR: Cannot add more springs to the system\n");
    return;
  }
  if (system->mass_count <= m1) {
    printf("ERROR: Index to first mass out of range\n");
    return;
  }
  if (system->mass_count <= m2) {
    printf("ERROR: Index to second mass out of range\n");
    return;
  }

  spring.first = &system->masses[m1];
  spring.second = &system->masses[m2];
  system->springs[system->spring_count++] = spring;
}

void system_draw(System *system) {
  for (size_t i = 0; i < system->spring_count; ++i) {
    spring_draw(&system->springs[i]);
  }
  for (size_t i = 0; i < system->mass_count; ++i) {
    mass_draw(&system->masses[i]);
  }
}

void system_spring_update(System *system) {
  for (size_t i = 0; i < system->spring_count; ++i) {
    spring_update(&system->springs[i]);
  }
}

void system_mass_update(System *system, float dt) {
  for (size_t i = 0; i < system->mass_count; ++i) {
    mass_update(&system->masses[i], dt);
  }
}

void system_mass_reset_forces(System *system) {
  for (size_t i = 0; i < system->mass_count; ++i) {
    mass_reset_forces(&system->masses[i]);
  }
}

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Springs");
  SetTargetFPS(60);

  _Bool running = false;

  System system = {0};

  Mass m1 = {0};
  m1.position = (Vector2){WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 3.0f};
  m1.fixed = true;

  Mass m2 = {0};
  m2.position = Vector2Add(m1.position, (Vector2){-150.0f, 150.0f});
  m2.mass = 10.0f;

  Mass m3 = {0};
  m3.position = Vector2Add(m2.position, (Vector2){50.0f, -50.0f});
  m3.mass = 10.0f;

  Mass m4 = {0};
  m4.position = Vector2Add(m2.position, (Vector2){-50.0f, 50.0f});
  m4.mass = 10.0f;

  system_add_mass(&system, m1);
  system_add_mass(&system, m2);
  system_add_mass(&system, m3);
  system_add_mass(&system, m4);

  system_add_spring(
      &system, (Spring){.length = 100.0f, .dampening = 1.0f, .strength = 10.0f},
      0, 1);
  system_add_spring(
      &system, (Spring){.length = 50.0f, .dampening = 1.0f, .strength = 10.0f},
      1, 2);
  system_add_spring(
      &system, (Spring){.length = 50.0f, .dampening = 1.0f, .strength = 10.0f},
      1, 3);

  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_SPACE)) {
      running = (running) ? false : true;
    }

    BeginDrawing();
    ClearBackground(BLACK);
    system_draw(&system);
    EndDrawing();

    float dt = TIME_SCALE * GetFrameTime();
    if (running) {
      system_spring_update(&system);
      system_mass_update(&system, dt);
      system_mass_reset_forces(&system);
    }
  }

  CloseWindow();
  return 0;
}