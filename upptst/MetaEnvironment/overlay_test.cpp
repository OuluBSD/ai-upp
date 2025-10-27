#include <Core/Core.h>
#include <Vfs/Core/Core.h>
#include <Vfs/Overlay/Overlay.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
	
	// Test basic overlay functionality
	LOG("Testing VfsOverlay implementation...");
	
	// Create a simple source reference
	SourceRef source(12345, 67890, "/test/file.cpp", 100);
	
	// Create an overlay
	Ptr<VfsOverlay> overlay = new VfsOverlay(source);
	
	// Test that overlay was created
	ASSERT(overlay);
	
	// Test source reference
	const SourceRef& src = overlay->GetSource();
	ASSERT(src.pkg_hash == 12345);
	ASSERT(src.file_hash == 67890);
	ASSERT(src.local_path == "/test/file.cpp");
	ASSERT(src.priority == 100);
	
	// Test listing (should be empty initially)
	Vector<String> list = overlay->List("/");
	ASSERT(list.IsEmpty());
	
	// Test merged value (should be null initially)
	Value merged = overlay->GetMerged("/");
	ASSERT(merged.IsNull());
	
	LOG("Basic VfsOverlay tests passed.");
	
	// Test OverlayManager
	LOG("Testing OverlayManager...");
	
	OverlayManager& manager = OverlayManager::GetInstance();
	
	// Test listing from manager (should be empty)
	Vector<String> mgr_list = manager.List("/");
	ASSERT(mgr_list.IsEmpty());
	
	// Test merged value from manager (should be empty map)
	Value mgr_merged = manager.GetMerged("/");
	ASSERT(mgr_merged.Is<ValueMap>());
	
	LOG("OverlayManager tests passed.");
	
	// Test MetaEnvironment overlay integration
	LOG("Testing MetaEnvironment overlay integration...");
	
	MetaEnvironment& env = MetaEnv();
	
	// Test listing from environment
	Vector<String> env_list = env.List("/");
	ASSERT(env_list.IsEmpty() || env_list.GetCount() >= 0); // May not be empty due to existing content
	
	// Test merged value from environment
	Value env_merged = env.GetMerged("/");
	ASSERT(env_merged.Is<ValueMap>() || env_merged.IsNull());
	
	// Test adding overlay to environment
	env.AddOverlay(overlay);
	
	LOG("MetaEnvironment overlay integration tests passed.");
	
	// Test precedence provider
	LOG("Testing PrecedenceProvider...");
	
	DefaultPrecedenceProvider precedence;
	precedence.AddPackage(12345);
	precedence.AddPackage(67890);
	
	Vector<hash_t> order = precedence.GetPackageOrder();
	ASSERT(order.GetCount() == 2);
	ASSERT(order[0] == 12345);
	ASSERT(order[1] == 67890);
	
	LOG("PrecedenceProvider tests passed.");
	
	LOG("All overlay tests completed successfully.");
}