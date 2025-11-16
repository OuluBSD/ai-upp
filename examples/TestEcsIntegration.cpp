#include <Core/Core.h>
#include <GameEngine/GameEngine.h>

using namespace Upp;

void TestEcsIntegration() {
    std::cout << "Testing ECS Integration..." << std::endl;
    
    // Create a simple ECS integration instance
    GameEcsIntegration ecs;
    if (!ecs.Initialize()) {
        std::cout << "Failed to initialize ECS integration" << std::endl;
        return;
    }
    
    // Create a simple entity
    auto entity = ecs.CreateEntity("TestEntity");
    if (!entity) {
        std::cout << "Failed to create entity" << std::endl;
        return;
    }
    
    std::cout << "Successfully created entity: " << entity->GetName() << std::endl;
    
    // Create a game object with components
    auto gameObject = ecs.CreateGameObject("Player", Point3(1, 2, 3));
    if (!gameObject) {
        std::cout << "Failed to create game object" << std::endl;
        return;
    }
    
    std::cout << "Successfully created game object with tag: " << "Player" << std::endl;
    
    // Check if the entity has the expected components
    if (auto transform = gameObject->Find<TransformComponent>()) {
        if (transform->GetPosition() == Point3(1, 2, 3)) {
            std::cout << "Transform component working correctly" << std::endl;
        } else {
            std::cout << "Transform component not set correctly" << std::endl;
        }
    }
    
    if (auto tag = gameObject->Find<TagComponent>()) {
        if (tag->GetTag() == "Player") {
            std::cout << "Tag component working correctly" << std::endl;
        } else {
            std::cout << "Tag component not set correctly" << std::endl;
        }
    }
    
    // Test finding entities by tag
    if (auto ecsSystem = ecs.GetEcsSystem()) {
        auto entityManager = ecsSystem->GetEntityManager();
        auto foundEntity = entityManager.FindEntityByTag("Player");
        
        if (foundEntity) {
            std::cout << "Successfully found entity by tag" << std::endl;
        } else {
            std::cout << "Failed to find entity by tag" << std::endl;
        }
    }
    
    std::cout << "ECS Integration test completed successfully!" << std::endl;
}

CONSOLE_APP_MAIN {
    TestEcsIntegration();
}