#include <GameEngine/GameEngine.h>

using namespace Upp;

class MyGame : public Game {
public:
    virtual void Initialize() override {
        Game::Initialize(); // Call parent initialization
        
        // Create some example entities
        if (auto ecs = GetEcsIntegration()) {
            // Create a camera entity
            auto camera = ecs->CreateGameObject("Camera", Point3(0, 1.6, -6));
            if (camera) {
                if (auto transform = camera->Find<TransformComponent>()) {
                    // Set initial camera position
                    transform->SetPosition(Point3(0, 1.6, -6));
                }
            }
            
            // Create a player entity
            auto player = ecs->CreateGameObject("Player", Point3(0, 0, 0));
            if (player) {
                if (auto transform = player->Find<TransformComponent>()) {
                    transform->SetPosition(Point3(0, 0, 0));
                }
            }
            
            // Create some game objects
            for (int i = 0; i < 5; i++) {
                String tag = "Object_" + IntStr(i);
                auto obj = ecs->CreateGameObject(tag, Point3(i * 2.0, 0, 0));
            }
        }
    }
    
    virtual void Update(double deltaTime) override {
        Game::Update(deltaTime); // Update ECS and other systems
        
        // Process game-specific logic here
        // You can access ECS entities and components here
        if (auto ecs = GetEcsIntegration()) {
            auto entityManager = ecs->GetEntityManager();
            if (entityManager) {
                // Example: Find and update all objects with tag "Player"
                auto players = entityManager->FindEntitiesByTag("Player");
                for (auto player : players) {
                    if (auto transform = player->Find<TransformComponent>()) {
                        auto pos = transform->GetPosition();
                        // Move player forward
                        transform->SetPosition(Point3(pos.x, pos.y, pos.z + 1.0 * deltaTime));
                    }
                }
            }
        }
    }
    
    virtual void Render(Draw& draw) override {
        // Render game content
        draw.DrawRect(GetMainWindow().GetSize(), Color(20, 20, 60)); // Dark blue background
        
        // You could integrate ECS rendering here if needed
    }
};

CONSOLE_APP_MAIN {
    MyGame game;
    game.Run();
}