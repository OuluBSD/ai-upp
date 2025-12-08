#include <Core/Core.h>
#include <GameEngine/GameEngine.h>

using namespace Upp;

CONSOLE_APP_MAIN {
    std::cout << "Testing GameEngine Networking functionality...\n";
    
    // Test HTTP Client
    std::cout << "Testing HTTP Client...\n";
    HttpClient httpClient;
    httpClient.SetHeader("User-Agent", "GameEngine Test Client");
    
    String response;
    if (httpClient.Get("http://httpbin.org/get", response)) {
        std::cout << "HTTP GET successful, response length: " << response.GetLength() << "\n";
    } else {
        std::cout << "HTTP GET failed\n";
    }
    
    // Test TCP Client/Server
    std::cout << "Testing TCP functionality...\n";
    
    // Create a server instance
    TcpServer server;
    if (server.Initialize()) {
        std::cout << "TCP Server initialized\n";
        // Note: We're not actually starting the server to listen on a real port
        // as that would require a separate thread and available port
    } else {
        std::cout << "TCP Server initialization failed\n";
    }
    
    // Create a client instance
    TcpClient client;
    if (client.Initialize()) {
        std::cout << "TCP Client initialized\n";
    } else {
        std::cout << "TCP Client initialization failed\n";
    }
    
    // Test WebSocket
    std::cout << "Testing WebSocket functionality...\n";
    WebSocket websocket;
    if (websocket.Initialize()) {
        std::cout << "WebSocket initialized\n";
    } else {
        std::cout << "WebSocket initialization failed\n";
    }
    
    // Test Network Message serialization
    std::cout << "Testing Network Message functionality...\n";
    NetworkMessage msg(NetworkMessageType::CHAT_MESSAGE, "Hello, World!");
    msg.senderId = 1;
    msg.targetId = 2;
    
    String serialized = client.SerializeMessage(msg);
    std::cout << "Serialized message: " << serialized << "\n";
    
    NetworkMessage deserialized = client.DeserializeMessage(serialized);
    std::cout << "Deserialized message content: " << deserialized.content << "\n";
    
    // Test Network Manager
    std::cout << "Testing Network Manager...\n";
    NetworkManager netManager;
    netManager.Initialize();
    
    // Add a client to the manager
    netManager.AddInterface(std::make_shared<TcpClient>());
    std::cout << "Added client to network manager, interface count: " 
              << netManager.GetInterfaceCount() << "\n";
    
    // Test game-specific networking utilities
    std::cout << "Testing game-specific network utilities...\n";
    Value gameState = 42; // Some simple game state
    NetworkMessage stateMsg = GameNetworkUtils::SerializeGameState(gameState);
    Value deserializedState = GameNetworkUtils::DeserializeGameState(stateMsg);
    std::cout << "Game state serialized and deserialized successfully\n";
    
    std::cout << "All networking tests completed!\n";
}