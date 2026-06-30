#include <Core/Core.h>

using namespace Upp;

struct User : Moveable<User> {
	int id;
	String name;
	String role;
	bool active;

	void Jsonize(JsonIO& io) {
		io("id", id)
		  ("name", name)
		  ("role", role)
		  ("active", active);
	}
};

struct TabularConfig {
	Vector<User> users;

	void Jsonize(JsonIO& io) {
		io("users", users);
	}
};

struct VisUser : Moveable<VisUser> {
	int id;
	String name;
	String role;
	bool active;

	void Visit(Visitor& v) {
		v("id", id)
		 ("name", name)
		 ("role", role)
		 ("active", active);
	}
};

struct VisConfig : Moveable<VisConfig> {
	Vector<VisUser> users;

	void Visit(Visitor& v) {
		v("users", users, VISIT_VECTOR);
	}
};

struct Address : Moveable<Address> {
	String city;
	String zip;

	void Jsonize(JsonIO& io) {
		io("city", city)
		  ("zip", zip);
	}
};

struct Employee : Moveable<Employee> {
	String name;
	Address address;

	void Jsonize(JsonIO& io) {
		io("name", name)
		  ("address", address);
	}
};

struct Company {
	String name;
	Vector<Employee> employees;

	void Jsonize(JsonIO& io) {
		io("name", name)
		  ("employees", employees);
	}
};

struct VisAddress : Moveable<VisAddress> {
	String city;
	String zip;

	void Visit(Visitor& v) {
		v("city", city)
		 ("zip", zip);
	}
};

struct VisEmployee : Moveable<VisEmployee> {
	String name;
	VisAddress address;

	void Visit(Visitor& v) {
		v("name", name);
		v.Visit("address", address);
	}
};

struct VisCompany {
	String name;
	Vector<VisEmployee> employees;

	void Visit(Visitor& v) {
		v("name", name)
		 ("employees", employees, VISIT_VECTOR);
	}
};

struct ContainerTest {
	Vector<String> vec;
	Array<int> arr;
	Index<String> idx;
	VectorMap<String, int> vec_map;
	ArrayMap<String, double> arr_map;

	void Jsonize(JsonIO& io) {
		io("vec", vec)
		  ("arr", arr)
		  ("idx", idx)
		  ("vec_map", vec_map)
		  ("arr_map", arr_map);
	}
};

void RunParserTest(const char* name, const String& toon, bool strict = true, bool expandPaths = false)
{
	LOG("--------------------------------------------------");
	LOG("Test: " << name);
	LOG("TOON Input:\n" << toon);
	
	Value val = ParseTOON(toon, strict, 2, expandPaths);
	if (val.IsError()) {
		LOG("Error: " << GetErrorText(val));
	} else {
		LOG("Parsed JSON structure:");
		LOG(AsJSON(val, true));
	}
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);

	LOG("==================================================");
	LOG("TOON Format parser & writer Reference Example");
	LOG("==================================================");

	// 1. Basic Object
	RunParserTest("Basic Object",
		"id: 123\n"
		"name: Ada\n"
		"active: true"
	);

	// 2. Nested Objects
	RunParserTest("Nested Objects",
		"user:\n"
		"  id: 123\n"
		"  name: Ada"
	);

	// 3. Primitive Inline Arrays
	RunParserTest("Primitive Inline Arrays",
		"tags[3]: admin,ops,dev"
	);

	// 4. Tabular Arrays
	RunParserTest("Tabular Arrays",
		"items[2]{sku,qty,price}:\n"
		"  A1,2,9.99\n"
		"  B2,1,14.5"
	);

	// 5. Mixed Arrays
	RunParserTest("Mixed Arrays",
		"items[3]:\n"
		"  - 1\n"
		"  - a: 1\n"
		"  - text"
	);

	// 6. Objects as List Items
	RunParserTest("Objects as List Items",
		"items[2]:\n"
		"  - id: 1\n"
		"    name: First\n"
		"  - id: 2\n"
		"    name: Second\n"
		"    extra: true"
	);

	// 7. Path Expansion (unquoted keys with dots)
	RunParserTest("Path Expansion (expandPaths=off)",
		"server.host: localhost\n"
		"database.connection.username: admin",
		true, false
	);
	RunParserTest("Path Expansion (expandPaths=safe)",
		"server.host: localhost\n"
		"database.connection.username: admin",
		true, true
	);

	// 8. Delimiter Variations (Pipe)
	RunParserTest("Delimiter Variations (Pipe)",
		"tags[3|]: reading|gaming|coding"
	);

	// 9. Edge case: Empty Array & Quoting
	RunParserTest("Empty Array & Quoting",
		"name: \"\"\n"
		"tags: []\n"
		"message: Hello 世界 👋"
	);

	// 10. Strict Mode Validation Tests (should produce errors)
	RunParserTest("Strict Mode Validation (Array Count Mismatch - Expected Error)",
		"tags[5]: a,b,c",
		true // strict
	);
	
	RunParserTest("Non-Strict Mode (Array Count Mismatch - Should Pass)",
		"tags[5]: a,b,c",
		false // non-strict
	);

	LOG("==================================================");
	LOG("Serialization & Deserialization (Store/Load) Tests");
	LOG("==================================================");

	TabularConfig cfg;
	User& u1 = cfg.users.Add();
	u1.id = 1;
	u1.name = "Alice";
	u1.role = "admin";
	u1.active = true;

	User& u2 = cfg.users.Add();
	u2.id = 2;
	u2.name = "Bob";
	u2.role = "developer";
	u2.active = true;

	User& u3 = cfg.users.Add();
	u3.id = 3;
	u3.name = "Charlie";
	u3.role = "designer";
	u3.active = false;

	// Serialize cfg to TOON (Tabular Array format)
	String toon_output = StoreAsTOON(cfg);
	LOG("Serialized TOON:\n" << toon_output);

	// De-serialize back to a new config
	TabularConfig loaded_cfg;
	if (LoadFromTOON(loaded_cfg, toon_output)) {
		LOG("Successfully loaded back from TOON!");
		LOG("Loaded users count: " << loaded_cfg.users.GetCount());
		for (int i = 0; i < loaded_cfg.users.GetCount(); i++) {
			const User& u = loaded_cfg.users[i];
			LOG("User [" << u.id << "]: name=" << u.name << ", role=" << u.role << ", active=" << u.active);
		}
	} else {
		LOG("Failed to load back from TOON");
	}

	// Key Folding Test
	ValueMap nested_vm;
	ValueMap server_vm;
	server_vm.Add("host", "localhost");
	nested_vm.Add("server", server_vm);

	LOG("\nEncoding nested maps with Key Folding:");
	LOG("No folding:\n" << AsTOON(nested_vm, 2, ',', false));
	LOG("With key folding:\n" << AsTOON(nested_vm, 2, ',', true));

	LOG("==================================================");
	LOG("Polymorphic Visitor TOON Tests");
	LOG("==================================================");

	VisConfig vcfg;
	VisUser& vu1 = vcfg.users.Add();
	vu1.id = 100;
	vu1.name = "Alice (Visitor)";
	vu1.role = "admin";
	vu1.active = true;

	VisUser& vu2 = vcfg.users.Add();
	vu2.id = 200;
	vu2.name = "Bob (Visitor)";
	vu2.role = "developer";
	vu2.active = false;

	// Serialize via Visitor
	String vis_toon = VisitToTOON(vcfg);
	LOG("Visitor Serialized TOON:\n" << vis_toon);

	// Deserialize via Visitor
	VisConfig loaded_vcfg;
	if (VisitFromTOON(loaded_vcfg, vis_toon)) {
		LOG("Successfully loaded back via Visitor!");
		LOG("Loaded users count: " << loaded_vcfg.users.GetCount());
		for (int i = 0; i < loaded_vcfg.users.GetCount(); i++) {
			const VisUser& u = loaded_vcfg.users[i];
			LOG("VisUser [" << u.id << "]: name=" << u.name << ", role=" << u.role << ", active=" << u.active);
		}
	} else {
		LOG("Failed to load back via Visitor");
	}

	LOG("==================================================");
	LOG("Nested Class Tests");
	LOG("==================================================");

	Company comp;
	comp.name = "Google DeepMind";
	Employee& emp1 = comp.employees.Add();
	emp1.name = "Dennis";
	emp1.address.city = "London";
	emp1.address.zip = "N1C";

	Employee& emp2 = comp.employees.Add();
	emp2.name = "Demis";
	emp2.address.city = "London";
	emp2.address.zip = "EC1Y";

	// 1. Test standard nested classes
	String comp_toon = StoreAsTOON(comp);
	LOG("Serialized Nested Class (Company):\n" << comp_toon);

	Company loaded_comp;
	if (LoadFromTOON(loaded_comp, comp_toon)) {
		LOG("Successfully loaded back Nested Class Company!");
		LOG("Company: " << loaded_comp.name << ", Employees count: " << loaded_comp.employees.GetCount());
		for(int i = 0; i < loaded_comp.employees.GetCount(); i++) {
			const Employee& emp = loaded_comp.employees[i];
			LOG("  Employee: " << emp.name << ", City: " << emp.address.city << ", Zip: " << emp.address.zip);
		}
	} else {
		LOG("Failed to load back Nested Class Company");
	}

	// 2. Test polymorphic nested classes via Visitor
	VisCompany vcomp;
	vcomp.name = "OpenAI";
	VisEmployee& vemp1 = vcomp.employees.Add();
	vemp1.name = "Sam";
	vemp1.address.city = "San Francisco";
	vemp1.address.zip = "94103";

	String vcomp_toon = VisitToTOON(vcomp);
	LOG("Visitor Serialized Nested Class (VisCompany):\n" << vcomp_toon);

	VisCompany loaded_vcomp;
	if (VisitFromTOON(loaded_vcomp, vcomp_toon)) {
		LOG("Successfully loaded back VisCompany via Visitor!");
		LOG("Company: " << loaded_vcomp.name << ", Employees count: " << loaded_vcomp.employees.GetCount());
		for(int i = 0; i < loaded_vcomp.employees.GetCount(); i++) {
			const VisEmployee& emp = loaded_vcomp.employees[i];
			LOG("  Employee: " << emp.name << ", City: " << emp.address.city << ", Zip: " << emp.address.zip);
		}
	} else {
		LOG("Failed to load back VisCompany via Visitor");
	}

	LOG("==================================================");
	LOG("Container (Vector, Array, Index, VectorMap, ArrayMap) Tests");
	LOG("==================================================");

	ContainerTest ct;
	ct.vec.Add("apple");
	ct.vec.Add("banana");

	ct.arr.Add(42);
	ct.arr.Add(100);

	ct.idx.Add("first");
	ct.idx.Add("second");

	ct.vec_map.Add("k1", 1000);
	ct.vec_map.Add("k2", 2000);

	ct.arr_map.Add("d1", 1.23);
	ct.arr_map.Add("d2", 4.56);

	String ct_toon = StoreAsTOON(ct);
	LOG("Serialized Containers TOON:\n" << ct_toon);

	ContainerTest loaded_ct;
	if (LoadFromTOON(loaded_ct, ct_toon)) {
		LOG("Successfully loaded back Containers!");
		LOG("Vector count: " << loaded_ct.vec.GetCount() << " (" << Join(loaded_ct.vec, ",") << ")");
		LOG("Array count: " << loaded_ct.arr.GetCount() << " (" << AsString(loaded_ct.arr[0]) << "," << AsString(loaded_ct.arr[1]) << ")");
		LOG("Index count: " << loaded_ct.idx.GetCount() << " (" << loaded_ct.idx[0] << "," << loaded_ct.idx[1] << ")");
		LOG("VectorMap count: " << loaded_ct.vec_map.GetCount());
		for (int i = 0; i < loaded_ct.vec_map.GetCount(); i++) {
			LOG("  " << loaded_ct.vec_map.GetKey(i) << " => " << loaded_ct.vec_map[i]);
		}
		LOG("ArrayMap count: " << loaded_ct.arr_map.GetCount());
		for (int i = 0; i < loaded_ct.arr_map.GetCount(); i++) {
			LOG("  " << loaded_ct.arr_map.GetKey(i) << " => " << loaded_ct.arr_map[i]);
		}
	} else {
		LOG("Failed to load back Containers");
	}
}
