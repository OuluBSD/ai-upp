#include "Core/Core.h"

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);

	// Load test YAML file
	String s = LoadFile("/common/active/sblo/Dev/ai-upp/reference/YAML/test.yaml");
	if(s.IsEmpty())
		s = LoadFile("reference/YAML/test.yaml");
	
	LOG("=== Basic YAML Parsing Test ===");
	Value yml = ParseYAML(s);
	LOG("Root type: " << yml.GetTypeName());
	LOG("Root count: " << yml.GetCount());
	
	// Test basic list parsing (people array)
	Value people = yml["people"];
	if(people.GetType() == VALUEARRAY_V) {
		ValueArray va = people;
		LOG("\n=== First Person ===");
		Value person = va[0];
		LOG("firstName: " << person["firstName"]);
		LOG("lastName: " << person["lastName"]);
		LOG("age: " << person["age"]);
		LOG("address.city: " << person["address"]["city"]);
		
		LOG("\n=== Phone Numbers ===");
		Value phone = person["phoneNumber"];
		LOG("Count: " << phone.GetCount());
		for(int i = 0; i < phone.GetCount(); i++) {
			LOG("  [" << i << "] type=" << phone[i]["type"] << ", number=" << phone[i]["number"]);
		}
		
		LOG("\n=== Null/Bool Tests ===");
		LOG("nulltest.IsNull(): " << person["nulltest"].IsNull());
		LOG("truetest: " << (bool)person["truetest"]);
		LOG("falsetest: " << (bool)person["falsetest"]);
	}
	
	// Test multi-line strings
	LOG("\n=== Multi-line String Tests ===");
	Value ml_test = yml["multi_line_test"];
	if(!ml_test.IsVoid()) {
		LOG("literal (|):\n" << ml_test["literal"]);
		LOG("---");
		LOG("folded (>): " << ml_test["folded"]);
	}
	
	// Test anchors and aliases
	LOG("\n=== Anchors & Aliases Tests ===");
	Value anchor_test = yml["anchors_test"];
	if(!anchor_test.IsVoid()) {
		LOG("defaults: timeout=" << anchor_test["defaults"]["timeout"] 
		                         << ", retries=" << anchor_test["defaults"]["retries"]);
		LOG("config1 (alias): timeout=" << anchor_test["config1"]["timeout"] 
		                                << ", retries=" << anchor_test["config1"]["retries"]);
		LOG("config2 (alias): timeout=" << anchor_test["config2"]["timeout"] 
		                                << ", retries=" << anchor_test["config2"]["retries"]);
		LOG("config1 == defaults: " << (anchor_test["config1"] == anchor_test["defaults"]));
		LOG("config2 == config1: " << (anchor_test["config2"] == anchor_test["config1"]));
	}
	
	// Test flow style
	LOG("\n=== Flow Style Tests ===");
	Value flow_test = yml["flow_style_test"];
	if(!flow_test.IsVoid()) {
		LOG("point {x: 10, y: 20}: x=" << flow_test["point"]["x"] << ", y=" << flow_test["point"]["y"]);
		LOG("list [1,2,3,4,5]: count=" << flow_test["list"].GetCount());
		Value list = flow_test["list"];
		for(int i = 0; i < list.GetCount(); i++)
			LOG("  [" << i << "]=" << list[i]);
		
		LOG("nested {items: [a,b,c], count: 3}:");
		LOG("  count=" << flow_test["nested"]["count"]);
		Value items = flow_test["nested"]["items"];
		LOG("  items: " << items.GetCount() << " elements");
		for(int i = 0; i < items.GetCount(); i++)
			LOG("    [" << i << "]=" << items[i]);
		
		LOG("empty_map {}: count=" << flow_test["empty_map"].GetCount());
		LOG("empty_list []: count=" << flow_test["empty_list"].GetCount());
		
		LOG("mixed: name=" << flow_test["mixed"]["name"] 
		                   << ", active=" << (bool)flow_test["mixed"]["active"]);
	}
	
	// Test combined features
	LOG("\n=== Combined Features Test ===");
	Value combined = yml["combined_test"];
	if(!combined.IsVoid()) {
		LOG("description:\n" << combined["description"]);
		LOG("settings: mode=" << combined["settings"]["mode"]);
		LOG("copy1 (alias): mode=" << combined["copy1"]["mode"]);
		LOG("copy1 == settings: " << (combined["copy1"] == combined["settings"]));
		LOG("inline: key=" << combined["inline"]["key"] << ", num=" << combined["inline"]["num"]);
	}
	
	// Test YAML generation (round-trip)
	LOG("\n=== YAML Generation Test ===");
	String generated = AsYAML(yml, true);
	LOG("Generated YAML (first 500 chars):\n" << generated.Left(500));
	
	// Test flow style generation
	LOG("\n=== Flow Style Parsing Verification ===");
	String flow_yaml = "point: {x: 100, y: 200}\nlist: [10, 20, 30]\n";
	Value flow_parsed = ParseYAML(flow_yaml);
	LOG("Parsed flow YAML:");
	LOG("  point.x=" << flow_parsed["point"]["x"] << ", point.y=" << flow_parsed["point"]["y"]);
	LOG("  list count=" << flow_parsed["list"].GetCount());
	for(int i = 0; i < flow_parsed["list"].GetCount(); i++)
		LOG("    [" << i << "]=" << flow_parsed["list"][i]);
	
	// Test multi-line string generation
	LOG("\n=== Multi-line String Generation ===");
	Yaml yb;
	yb("literal_test", "Line1\nLine2\nLine3");
	yb("folded_test", "This is a folded line that should wrap");
	String yaml_out = ~yb;
	LOG("Generated:\n" << yaml_out);
	
	LOG("\n=== All Tests Complete ===");
}
