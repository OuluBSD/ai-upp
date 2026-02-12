#include <Core/Core.h>
#include <Aria/Aria.h>
#include <Aria/NewsScraper.h>

using namespace Upp;

struct Command {
	virtual ~Command() {}
	virtual String GetName() const = 0;
	virtual Vector<String> GetAliases() const { return {}; }
	virtual String GetDescription() const = 0;
	virtual void ShowHelp() const = 0;
	virtual void Execute(const Vector<String>& args) = 0;
};

struct NewsCommand : Command {
	String GetName() const override { return "news"; }
	Vector<String> GetAliases() const override { return {"n"}; }
	String GetDescription() const override { return "Manage news items and scrapers"; }
	void ShowHelp() const override {
		Cout() << "usage: AriaCLI news <subcommand>\n\n"
		       << "subcommands:\n"
		       << "  scrape        Run scrapers for all sources\n"
		       << "  list          Show persisted news items\n"
		       << "  clear         Clear news database\n";
	}
	
	void Execute(const Vector<String>& args) override {
		if (args.GetCount() == 0) { ShowHelp(); return; }
		
		String sub = args[0];
		String path = ConfigFile("News.bin");
		
		if (sub == "scrape") {
			AriaNavigator navigator;
			SiteManager sm;
			NewsScraper scraper(navigator, sm);
			
			VectorMap<String, NewsItem> items;
			LoadFromFile(items, path);
			
			struct Site : Moveable<Site> {
				String name;
				String url;
				Site(const String& n, const String& u) : name(n), url(u) {}
				Site() {}
			};
			Vector<Site> sites;
			sites << Site("ZeroHedge", "https://www.zerohedge.com/")
			      << Site("HackerNews", "https://news.ycombinator.com/")
			      << Site("ForexFactory", "https://www.forexfactory.com/")
			      << Site("FXStreet", "https://www.fxstreet.com")
			      << Site("Investing", "https://www.investing.com")
			      << Site("Discord", "https://discord.com/channels/283274943754665984/1363100565890007041")
			      << Site("Discord", "https://discord.com/channels/283274943754665984/675830944371965953");
			
			for(const auto& s : sites) {
				Cout() << "Scraping " << s.name << "...\n";
				ValueArray res = scraper.ScrapeSite(s.name, s.url);
				for(const Value& v : res) {
					String u = v["url"];
					if(u.IsEmpty()) continue;
					if(items.Find(u) < 0) {
						NewsItem& n = items.Add(u);
						n.url = u;
						n.title = v["title"];
						n.source = v["source"];
						n.published = GetUtcTime();
						n.id = u;
					}
				}
			}
			StoreToFile(items, path);
			Cout() << "✓ Done. Total items: " << items.GetCount() << "\n";
		}
		else if (sub == "list") {
			bool all = args.GetCount() > 1 && args[1] == "all";
			VectorMap<String, NewsItem> items;
			if (LoadFromFile(items, path)) {
				Cout() << "Persisted News Items (" << items.GetCount() << "):\n";
				for(int i = 0; i < items.GetCount(); i++) {
					const auto& n = items[i];
					Cout() << "--------------------------------------------------\n";
					Cout() << "Source:    " << n.source << "\n";
					Cout() << "Title:     " << n.title << "\n";
					Cout() << "URL:       " << n.url << "\n";
					Cout() << "Published: " << n.published << "\n";
					
					if (!all && i >= 19) {
						Cout() << "--------------------------------------------------\n";
						Cout() << "... and " << (items.GetCount() - i - 1) << " more. Use 'list all' to see everything.\n";
						break;
					}
				}
				Cout() << "--------------------------------------------------\n";
			} else {
				Cout() << "No news data found at " << path << "\n";
			}
		}
		else if (sub == "clear") {
			FileDelete(path);
			Cout() << "News database cleared.\n";
		}
		else if (sub == "path") {
			Cout() << path << "\n";
		}
		else {
			Cout() << "Unknown news subcommand: " << sub << "\n";
		}
	}
};

struct NavigatorCommand : Command {
	String GetName() const override { return "navigator"; }
	Vector<String> GetAliases() const override { return {"nav"}; }
	String GetDescription() const override { return "Direct navigator control"; }
	void ShowHelp() const override {
		Cout() << "usage: AriaCLI nav <url> [eval <js>]\n";
	}
	void Execute(const Vector<String>& args) override {
		if (args.GetCount() < 1) { ShowHelp(); return; }
		
		try {
			AriaNavigator nav;
			Cout() << "Connecting to browser...\n";
			nav.Navigate(args[0]);
			Cout() << "Navigated to " << args[0] << ". Waiting for load...\n";
			Sleep(5000);
			
			if (args.GetCount() >= 3 && args[1] == "eval") {
				Cout() << "Evaluating: " << args[2] << "\n";
				Value res = nav.Eval(args[2]);
				Cout() << "Result: " << AsString(res) << "\n";
			}
		} catch (const Exc& e) {
			Cerr() << "Navigator Error: " << e << "\n";
		}
	}
};

struct DiscordCommand : Command {
	String GetName() const override { return "discord"; }
	Vector<String> GetAliases() const override { return {"d"}; }
	String GetDescription() const override { return "Discord interactions"; }
	void ShowHelp() const override {
		Cout() << "usage: AriaCLI discord [scrape]\n";
	}
	void Execute(const Vector<String>& args) override {
		if (args.GetCount() > 0 && args[0] == "scrape") {
			AriaNavigator navigator;
			SiteManager sm;
			DiscordScraper scraper(navigator, sm);
			Cout() << "Refreshing Discord...\n";
			if (scraper.Refresh()) {
				Cout() << "✓ Discord data refreshed.\n";
			} else {
				Cerr() << "Error: Discord refresh failed.\n";
			}
		} else {
			ShowHelp();
		}
	}
};

struct GoogleMessagesCommand : Command {
	String GetName() const override { return "messages"; }
	Vector<String> GetAliases() const override { return {"msg", "sms"}; }
	String GetDescription() const override { return "Google Messages interaction"; }
	void ShowHelp() const override {
		Cout() << "usage: AriaCLI messages [list]\n";
	}
	void Execute(const Vector<String>& args) override {
		SiteManager sm;
		Value data = sm.GetSiteData("google_messages", "messages");
		if (data.Is<ValueArray>()) {
			ValueArray va = data;
			Cout() << "Recent Google Messages:\n";
			for(int i = 0; i < min(va.GetCount(), 10); i++) {
				ValueMap m = va[i];
				Cout() << "[" << m["sender"] << "] " << m["text"] << " (" << m["time"] << ")\n";
			}
		} else {
			Cout() << "No Google Messages data available. Run scrape in Hub first.\n";
		}
	}
};

void MainHelp(const Array<Command>& commands) {
// ... (rest of MainHelp)
}

CONSOLE_APP_MAIN {
	SetConfigName("AriaHub");
	Array<Command> commands;
	commands.Create<NewsCommand>();
	commands.Create<NavigatorCommand>();
	commands.Create<DiscordCommand>();
	commands.Create<GoogleMessagesCommand>();
	
	const Vector<String>& raw_args = CommandLine();
	if (raw_args.GetCount() == 0) {
		MainHelp(commands);
		return;
	}
	
	String cmdName = raw_args[0];
	Vector<String> sub_args;
	for(int i = 1; i < raw_args.GetCount(); i++) sub_args.Add(raw_args[i]);
	
	Command* found = nullptr;
	for(int i = 0; i < commands.GetCount(); i++) {
		if (commands[i].GetName() == cmdName) { found = &commands[i]; break; }
		for(const auto& a : commands[i].GetAliases()) if (a == cmdName) { found = &commands[i]; break; }
	}
	
	if (found) {
		found->Execute(sub_args);
	} else {
		Cout() << "Unknown command: " << cmdName << "\n";
		MainHelp(commands);
	}
}