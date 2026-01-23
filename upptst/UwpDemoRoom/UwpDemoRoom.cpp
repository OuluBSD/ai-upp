#include "UwpDemoRoom.h"

using namespace DemoRoom;
using namespace Upp;
using namespace winrt::Windows::Graphics::Holographic;

namespace {
	struct DemoRoomApp final : ShellConnectorApp {
		DemoRoomMain main;

		void SetHolographicSpace(HolographicSpace const& holographicSpace) override
		{
			main.SetHolographicSpace(holographicSpace);
		}

		void Update() override
		{
			main.Update();
		}

		void SaveAppState() override
		{
			main.SaveAppState();
		}

		void LoadAppState() override
		{
			main.LoadAppState();
		}
	};
}

UWPVR_APP_MAIN
{
	return std::make_unique<DemoRoomApp>();
}
