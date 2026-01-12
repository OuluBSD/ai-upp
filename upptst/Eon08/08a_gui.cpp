#include "Eon08.h"
#include <EonRouterSupport/EonRouterSupport.h>

/*

net gui.test:
	center.customer
	center.gui.filesrc:
		file = "share/eon/forms/test_form.form"
	upp.gui.sink:
		close_machine = true
		sizeable = true
		env = "/event/register"

	center.customer.0 -> center.gui.filesrc.0
	center.gui.filesrc.0 -> upp.gui.sink.0
	upp.gui.sink.0 -> center.customer.0

*/

NAMESPACE_UPP

void Run08aGui(Engine& eng, int method) {
	LOG(Format("Run08aGui: starting with method %d", method));
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 3: {
		RouterNetContext net("gui.test");

		auto& customer = net.AddAtom("customer0", "center.customer");
		int customer_src0 = net.AddPort(customer.id, RouterPortDesc::Direction::Source, "0").index;
		int customer_sink0 = net.AddPort(customer.id, RouterPortDesc::Direction::Sink, "0").index;

		auto& filesrc = net.AddAtom("filesrc0", "center.gui.filesrc");
		filesrc.args.GetAdd("file") = "share/eon/forms/test_form.form";
		int filesrc_sink0 = net.AddPort(filesrc.id, RouterPortDesc::Direction::Sink, "0").index;
		int filesrc_src0 = net.AddPort(filesrc.id, RouterPortDesc::Direction::Source, "0").index;

		auto& guisink = net.AddAtom("guisink0", "upp.gui.sink");
		guisink.args.GetAdd("close_machine") = true;
		guisink.args.GetAdd("sizeable") = true;
		guisink.args.GetAdd("env") = "/event/register";
		int guisink_sink0 = net.AddPort(guisink.id, RouterPortDesc::Direction::Sink, "0").index;
		int guisink_src0 = net.AddPort(guisink.id, RouterPortDesc::Direction::Source, "0").index;

		net.Connect("customer0", customer_src0, "filesrc0", filesrc_sink0);
		net.Connect("filesrc0", filesrc_src0, "guisink0", guisink_sink0);
		net.Connect("guisink0", guisink_src0, "customer0", customer_sink0);

		LOG("Run08aGui: calling BuildLegacyLoop");
		if (!net.BuildLegacyLoop(eng)) {
			LOG("Run08aGui: BuildLegacyLoop failed!");
			Exit(1);
		}
		LOG("Run08aGui: BuildLegacyLoop succeeded");
		break;
	}
	case 1:
	case 2:
		LOG(Format("warning: Run08aGui: method %d not implemented yet", method));
	case 0:
		LOG("Run08aGui: calling PostLoadFile");
		sys->PostLoadFile(ShareDirFile("eon/tests/08a_gui.eon"));
		LOG("Run08aGui: PostLoadFile returned");
		break;
	default:
		throw Exc(Format("Run08aGui: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
