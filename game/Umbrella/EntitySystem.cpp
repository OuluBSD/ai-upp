#include "Umbrella.h"

#include <CtrlLib/CtrlLib.h>
#include <Draw/Draw.h>
#include <Core/Core.h>

using namespace Upp;

// Forward declarations
class Entity;
class EntityManager;
class EntityFactory;

// Basic Entity Types
enum class EntityType {
    PLAYER,
    ENEMY,
    COLLECTIBLE,
    OBSTACLE,
    DECORATION
};

// Basic Entity Component
struct PositionComponent {
    double x, y;
    
    PositionComponent(double px = 0, double py = 0) : x(px), y(py) {}
};

struct RenderComponent {
    String spriteName;
    Color color;
    int zIndex;
    
    RenderComponent(const String& spr = "", Color c = White(), int z = 0) 
        : spriteName(spr), color(c), zIndex(z) {}
};

struct PhysicsComponent {
    double velocityX, velocityY;
    bool isSolid;
    
    PhysicsComponent(double vx = 0, double vy = 0, bool solid = true) 
        : velocityX(vx), velocityY(vy), isSolid(solid) {}
};

struct HealthComponent {
    int currentHealth;
    int maxHealth;
    
    HealthComponent(int health = 100) : currentHealth(health), maxHealth(health) {}
};

// Basic Entity Class
class Entity {
private:
    int id;
    String name;
    EntityType type;
    bool isActive;
    
    // Components
    PositionComponent* position;
    RenderComponent* render;
    PhysicsComponent* physics;
    HealthComponent* health;
    
public:
    Entity(int entityId, const String& entityName, EntityType entityType);
    ~Entity();
    
    // Getters
    int GetId() const { return id; }
    const String& GetName() const { return name; }
    EntityType GetType() const { return type; }
    bool IsActive() const { return isActive; }
    
    // Component accessors
    PositionComponent* GetPosition() { return position; }
    RenderComponent* GetRender() { return render; }
    PhysicsComponent* GetPhysics() { return physics; }
    HealthComponent* GetHealth() { return health; }
    
    // Setters
    void SetActive(bool active) { isActive = active; }
    
    // Component setters
    void SetPosition(double x, double y);
    void SetRender(const String& sprite, Color color = White(), int zIndex = 0);
    void SetPhysics(double velX, double velY, bool isSolid = true);
    void SetHealth(int current, int max);
    
    // Update entity
    void Update(double deltaTime);
    
    // Render entity
    void Render(Draw& draw, const Rect& viewPort);
};

// Implementation of Entity
Entity::Entity(int entityId, const String& entityName, EntityType entityType)
    : id(entityId), name(entityName), type(entityType), isActive(true) {
    position = new PositionComponent();
    render = new RenderComponent();
    physics = new PhysicsComponent();
    health = new HealthComponent();
}

Entity::~Entity() {
    delete position;
    delete render;
    delete physics;
    delete health;
}

void Entity::SetPosition(double x, double y) {
    if(position) {
        position->x = x;
        position->y = y;
    }
}

void Entity::SetRender(const String& sprite, Color color, int zIndex) {
    if(render) {
        render->spriteName = sprite;
        render->color = color;
        render->zIndex = zIndex;
    }
}

void Entity::SetPhysics(double velX, double velY, bool isSolid) {
    if(physics) {
        physics->velocityX = velX;
        physics->velocityY = velY;
        physics->isSolid = isSolid;
    }
}

void Entity::SetHealth(int current, int max) {
    if(health) {
        health->currentHealth = current;
        health->maxHealth = max;
    }
}

void Entity::Update(double deltaTime) {
    // Update position based on physics
    if(position && physics) {
        position->x += physics->velocityX * deltaTime;
        position->y += physics->velocityY * deltaTime;
    }
}

void Entity::Render(Draw& draw, const Rect& viewPort) {
    if(render && position) {
        // Only render if entity is within viewport
        if(position->x >= viewPort.left && position->x <= viewPort.right &&
           position->y >= viewPort.top && position->y <= viewPort.bottom) {
            
            // Draw a simple representation of the entity
            Color entityColor = render->color;
            if(type == EntityType::PLAYER) {
                entityColor = Blue();
            } else if(type == EntityType::ENEMY) {
                entityColor = Red();
            } else if(type == EntityType::COLLECTIBLE) {
                entityColor = Yellow();
            }
            
            // Draw a simple rectangle representing the entity
            draw.DrawRect(Rect(round(position->x), round(position->y), 32, 32), entityColor);
            
            // Draw entity name
            draw.DrawText(round(position->x), round(position->y) - 15, name, Arial(10), Black());
        }
    }
}

// Entity Manager
class EntityManager {
private:
    Vector<Entity*> entities;
    int nextId;
    
public:
    EntityManager();
    ~EntityManager();
    
    // Create a new entity
    Entity* CreateEntity(const String& name, EntityType type);
    
    // Remove an entity
    void RemoveEntity(int entityId);
    void RemoveEntity(Entity* entity);
    
    // Get entity by ID
    Entity* GetEntity(int entityId);
    
    // Update all entities
    void UpdateAll(double deltaTime);
    
    // Render all entities
    void RenderAll(Draw& draw, const Rect& viewPort);
    
    // Get all entities
    const Vector<Entity*>& GetAllEntities() const { return entities; }
    
    // Clear all entities
    void Clear();
};

// Implementation of EntityManager
EntityManager::EntityManager() : nextId(0) {}

EntityManager::~EntityManager() {
    Clear();
}

Entity* EntityManager::CreateEntity(const String& name, EntityType type) {
    Entity* newEntity = new Entity(nextId++, name, type);
    entities.Add(newEntity);
    return newEntity;
}

void EntityManager::RemoveEntity(int entityId) {
    for(int i = 0; i < entities.GetCount(); i++) {
        if(entities[i]->GetId() == entityId) {
            delete entities[i];
            entities.Remove(i);
            break;
        }
    }
}

void EntityManager::RemoveEntity(Entity* entity) {
    if(entity) {
        RemoveEntity(entity->GetId());
    }
}

Entity* EntityManager::GetEntity(int entityId) {
    for(Entity* entity : entities) {
        if(entity->GetId() == entityId) {
            return entity;
        }
    }
    return nullptr;
}

void EntityManager::UpdateAll(double deltaTime) {
    for(Entity* entity : entities) {
        if(entity->IsActive()) {
            entity->Update(deltaTime);
        }
    }
}

void EntityManager::RenderAll(Draw& draw, const Rect& viewPort) {
    // Sort entities by z-index for proper rendering order
    struct EntityZIndex {
        Entity* entity;
        int zIndex;
    };
    
    Vector<EntityZIndex> renderQueue;
    for(Entity* entity : entities) {
        if(entity->IsActive() && entity->GetRender()) {
            EntityZIndex ez;
            ez.entity = entity;
            ez.zIndex = entity->GetRender()->zIndex;
            renderQueue.Add(ez);
        }
    }
    
    // Sort by z-index
    Sort(renderQueue, [](const EntityZIndex& a, const EntityZIndex& b) {
        return a.zIndex < b.zIndex;
    });
    
    // Render in sorted order
    for(const EntityZIndex& ez : renderQueue) {
        ez.entity->Render(draw, viewPort);
    }
}

void EntityManager::Clear() {
    for(Entity* entity : entities) {
        delete entity;
    }
    entities.Clear();
    nextId = 0;
}

// Entity Factory
class EntityFactory {
private:
    EntityManager* entityManager;
    
public:
    EntityFactory(EntityManager* mgr) : entityManager(mgr) {}
    
    Entity* CreatePlayer(const String& name, double x, double y);
    Entity* CreateEnemy(const String& name, double x, double y, const String& enemyType = "");
    Entity* CreateCollectible(const String& name, double x, double y, const String& collectibleType = "");
    Entity* CreateObstacle(const String& name, double x, double y);
    Entity* CreateDecoration(const String& name, double x, double y);
    
private:
    Entity* CreateBasicEntity(const String& name, EntityType type, double x, double y);
};

// Implementation of EntityFactory
Entity* EntityFactory::CreateBasicEntity(const String& name, EntityType type, double x, double y) {
    Entity* entity = entityManager->CreateEntity(name, type);
    if(entity) {
        entity->SetPosition(x, y);
    }
    return entity;
}

Entity* EntityFactory::CreatePlayer(const String& name, double x, double y) {
    Entity* player = CreateBasicEntity(name, EntityType::PLAYER, x, y);
    if(player) {
        player->SetRender("player_sprite", Blue(), 10);
        player->SetPhysics(0, 0, true);
        player->SetHealth(100, 100);
    }
    return player;
}

Entity* EntityFactory::CreateEnemy(const String& name, double x, double y, const String& enemyType) {
    Entity* enemy = CreateBasicEntity(name, EntityType::ENEMY, x, y);
    if(enemy) {
        enemy->SetRender(enemyType, Red(), 5);
        enemy->SetPhysics(0, 0, true);
        enemy->SetHealth(50, 50);
    }
    return enemy;
}

Entity* EntityFactory::CreateCollectible(const String& name, double x, double y, const String& collectibleType) {
    Entity* collectible = CreateBasicEntity(name, EntityType::COLLECTIBLE, x, y);
    if(collectible) {
        collectible->SetRender(collectibleType, Yellow(), 3);
        collectible->SetPhysics(0, 0, false);  // Not solid
        collectible->SetHealth(1, 1);  // Just exists
    }
    return collectible;
}

Entity* EntityFactory::CreateObstacle(const String& name, double x, double y) {
    Entity* obstacle = CreateBasicEntity(name, EntityType::OBSTACLE, x, y);
    if(obstacle) {
        obstacle->SetRender("obstacle", Brown(), 2);
        obstacle->SetPhysics(0, 0, true);
        obstacle->SetHealth(999, 999);  // Indestructible
    }
    return obstacle;
}

Entity* EntityFactory::CreateDecoration(const String& name, double x, double y) {
    Entity* decoration = CreateBasicEntity(name, EntityType::DECORATION, x, y);
    if(decoration) {
        decoration->SetRender("decoration", Green(), 1);
        decoration->SetPhysics(0, 0, false);  // Not solid
        decoration->SetHealth(1, 1);  // Just exists
    }
    return decoration;
}

// Global entity manager instance
EntityManager& GetEntityManager() {
    static EntityManager entityManager;
    return entityManager;
}