#include <Core/Core.h>
#include <GameEngine/GameEngine.h>

using namespace Upp;

CONSOLE_APP_MAIN {
    // Basic test to verify GameEngine compiles and basic functionality works
    std::cout << "Running GameEngine tests..." << std::endl;
    
    // Test 1: AssetManager functionality
    AssetManager assetManager;
    ASSERT(assetManager.GetMemoryUsage() == 0);
    LOG("✓ AssetManager initialization test passed");
    
    // Set a small memory budget for testing
    assetManager.SetMemoryBudget(1024 * 1024); // 1MB
    ASSERT(assetManager.GetMemoryBudget() == 1024 * 1024);
    LOG("✓ AssetManager memory budget test passed");
    
    // Test 2: Scene management functionality
    SceneManager sceneManager;
    
    // Create a test scene
    auto testScene = std::make_shared<Scene>();
    testScene->SetName("TestScene");
    sceneManager.AddScene(testScene);
    
    // Switch to the scene
    sceneManager.SwitchToScene("TestScene");
    
    auto currentScene = sceneManager.GetCurrentScene();
    ASSERT(currentScene != nullptr);
    ASSERT(currentScene->GetName() == "TestScene");
    ASSERT(currentScene->IsActive());
    LOG("✓ Scene management test passed");
    
    // Test 3: GameWindow creation and basic functionality
    GameWindow window;
    window.Title("Test Window");
    window.SetRect(0, 0, 800, 600);
    
    // Verify window properties
    ASSERT(window.GetRect().GetWidth() == 800);
    ASSERT(window.GetRect().GetHeight() == 600);
    LOG("✓ GameWindow creation test passed");
    
    // Test 4: Game class functionality
    Game game;
    LOG("✓ Game object creation test passed");
    
    // Test 5: Matrix and geometry operations
    Matrix4 identity = Matrix4::Identity();
    ASSERT(identity[0][0] == 1.0 && identity[1][1] == 1.0 && 
           identity[2][2] == 1.0 && identity[3][3] == 1.0);
    LOG("✓ Matrix operations test passed");
    
    // Test 6: Vector operations
    Point3 point(1.0, 2.0, 3.0);
    ASSERT(point.x == 1.0 && point.y == 2.0 && point.z == 3.0);
    LOG("✓ Vector operations test passed");
    
    std::cout << "All tests passed!" << std::endl;
    LOG("GameEngine library is working correctly");
}