#ifndef RAINBOWGAME_GAMEPLAY_COMPONENTS_PLAYER_H
#define RAINBOWGAME_GAMEPLAY_COMPONENTS_PLAYER_H

#include <Core/Core.h>
#include "Input/InputController.h"
#include "Gameplay/Components/ParasolComponent.h"

using namespace Upp;

// Forward declarations
class CollisionHandler;
class Enemy;

struct Rectangle {
    float x, y, width, height;
    
    Rectangle() : x(0), y(0), width(0), height(0) {}
    Rectangle(float x, float y, float width, float height) 
        : x(x), y(y), width(width), height(height) {}
};

struct Vector2 {
    float x, y;
    
    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}
    
    void SetZero() { x = 0; y = 0; }
};

class Player {
public:
    Player(float x, float y, float width, float height);
    ~Player();
    
    void Update(float delta, const InputState& inputState, CollisionHandler* collision);
    
    // Movement
    void MoveX(Rectangle& bounds, float amount);
    void MoveY(Rectangle& bounds, float amount);
    
    // Position
    void SetPosition(float x, float y);
    float GetX() const { return bounds.x; }
    float GetY() const { return bounds.y; }
    
    // Accessors
    Rectangle GetBounds() const { return bounds; }
    Vector2 GetVelocity() const { return velocity; }
    bool IsOnGround() const { return onGround; }
    ParasolComponent* GetParasol() { return &parasol; }
    
    // Score
    void AddScore(int value);
    int GetScore() const { return score; }
    
    // Lives
    int GetLives() const { return lives; }
    bool IsAlive() const { return lives > 0; }
    void AddLife();
    
    // Damage
    void Damage(int amount);
    void Damage(int amount, const Rectangle& sourceBounds);
    bool IsInvincible() const { return invincibleTimer > 0.0f; }
    bool IsRecovering() const { return knockbackTimer > 0.0f; }
    
    // Teleportation and reset
    void ResetPosition(float x, float y);
    void TeleportTo(float x, float y);
    
    // Facing direction  
    int GetFacing() const { return facing; }
    
    // Enemy capture
    bool CanCapture(Enemy* enemy);
    void CaptureEnemy(Enemy* enemy);
    bool HasCapturedEnemies() const;
    Enemy* ReleaseCapturedEnemy();
    int GetBlueBalls() const { return blueBalls; }
    void SetBlueBalls(int count);
    void AddBlueBall();
    
    // For physics integration
    void SetVelocity(float vx, float vy) { velocity.x = vx; velocity.y = vy; }
    float GetVelocityX() const { return velocity.x; }
    float GetVelocityY() const { return velocity.y; }
    
private:
    // Physics properties
    Rectangle bounds;
    Rectangle tmpBounds;
    Vector2 velocity;
    ParasolComponent parasol;
    
    // Physics constants
    static const float TILE_EPSILON;
    static const float GRID_SIZE;
    static const float MOVE_SPEED;
    static const float GRAVITY;
    static const float MAX_FALL_SPEED;
    static const float JUMP_VELOCITY;
    static const float MIN_JUMP_VELOCITY;
    static const float COLLISION_STEP;
    
    // State
    bool onGround;
    float coyoteTimer;
    float jumpBufferTimer;
    bool jumpHeld;
    bool jumpHoldQueued;
    float fallDistance;
    float maxFallDistance;
    int lives;
    float invincibleTimer;
    int score;
    int facing;  // 1 for right, -1 for left
    float knockbackTimer;
    float knockbackDuration;
    float knockbackHorizontal;
    float knockbackVertical;
    float maxCarryWeight;
    float carryWeight;
    
    // Carried enemies
    Vector<Enemy*> carriedEnemies;
    
    // Blue balls (collectibles)
    int blueBalls;
    
    // Constants
    static const float COYOTE_TIME;
    static const float JUMP_BUFFER_TIME;
    
    // Helper methods
    bool IsTouchingWallOnLeft(CollisionHandler* collision);
    bool IsTouchingWallOnRight(CollisionHandler* collision);
    bool IsTouchingWallTile(CollisionHandler* collision);
    bool IsStandingOnFloor(CollisionHandler* collision);
    double* ResolveLandingY(double currentY, double nextY, CollisionHandler* collision);
    void IntegrateHorizontal(float delta, CollisionHandler* collision);
    void IntegrateVertical(float delta, CollisionHandler* collision);
    void AlignCapturedEnemies();
    void AbandonCapturedEnemies();
    float GetRemainingCarryWeight();
};

#endif
