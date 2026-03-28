#include "Core/Core.h"

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);

	// Try loading from reference/YAML/test.yaml
	String s = LoadFile("/common/active/sblo/Dev/ai-upp/reference/YAML/test.yaml");
	if(s.IsEmpty()) {
		// Try relative path
		s = LoadFile("reference/YAML/test.yaml");
	}
	if(s.IsEmpty()) {
		// Try GetDataFile
		s = LoadFile(GetDataFile("test.yaml"));
	}
	
	LOG("=== Original YAML file ===");
	LOG("File size: " << s.GetCount() << " bytes");
	LOG(s.Left(200));
	
	LOG("\n=== Parsing YAML ===");
	Value yml = ParseYAML(s);
	DUMP(yml.GetTypeName());
	DUMP(yml.GetCount());
	
	LOG("\n=== First person ===");
	Value person = yml[0];
	DUMP(person["firstName"]);
	DUMP(person["lastName"]);
	DUMP(person["age"]);
	DUMP(person["address"]["city"]);
	
	LOG("\n=== Phone numbers ===");
	Value phone_number = person["phoneNumber"];
	DUMP(phone_number.GetTypeName());
	DUMP(phone_number.GetCount());
	for(int i = 0; i < phone_number.GetCount(); i++) {
		DUMP(phone_number[i]["type"]);
		DUMP(phone_number[i]["number"]);
	}
	
	LOG("\n=== Generate YAML (pretty) ===");
	String yaml_pretty = AsYAML(yml, true);
	LOG(yaml_pretty);
	
	LOG("\n=== Generate YAML (compact) ===");
	String yaml_compact = AsYAML(yml);
	LOG(yaml_compact);
	
	LOG("\n=== Composing YAML using support classes ===");
	Yaml yml_builder;
	yml_builder
		("firstName", "Andrew")
		("lastName", "Smith")
		("age", 28)
		("address", Yaml("streetAddress", "23 3rd Street")("city", "New York"))
	;
	YamlArray pn;
	pn << Yaml("type", "home")("number", "12312345")
	   << Yaml("type", "work")("number", "87126388");
	yml_builder("phoneNumber", pn);
	
	String composed = ~yml_builder;
	LOG(composed);
	
	LOG("\n=== Verify round-trip ===");
	Value parsed_back = ParseYAML(composed);
	DUMP(parsed_back["firstName"]);
	DUMP(parsed_back["age"]);
	DUMP(parsed_back["phoneNumber"].GetCount());
	
	LOG("\n=== Test null/bool values ===");
	DUMP(person["nulltest"].IsNull());
	DUMP((bool)person["truetest"]);
	DUMP(!(bool)person["falsetest"]);
}
