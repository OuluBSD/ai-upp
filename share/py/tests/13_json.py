import json

print("JSON module tests:")
data = {
    "name": "Uppy",
    "version": 0.1,
    "features": ["fast", "simple", "U++ based"],
    "active": True,
    "none_val": None
}

json_str = json.dumps(data)
print("JSON string:", json_str)

parsed_data = json.loads(json_str)
print("Parsed name:", parsed_data["name"])
print("Parsed version:", parsed_data["version"])
print("Parsed features:", parsed_data["features"])
print("Parsed active:", parsed_data["active"])
print("Parsed none_val:", parsed_data["none_val"])

# Test with list
data_list = [1, 2, {"a": 3}]
json_list = json.dumps(data_list)
print("JSON list:", json_list)
parsed_list = json.loads(json_list)
print("Parsed list second element:", parsed_list[1])
print("Parsed list third element key 'a':", parsed_list[2]["a"])

# Test file dump/load
test_json_file = "test_json.json"
json.dump(data, test_json_file)
print("JSON dumped to file")
loaded_from_file = json.load(test_json_file)
print("Loaded from file name:", loaded_from_file["name"])

import os
os.remove(test_json_file)
print("Test JSON file removed")

print("JSON tests completed successfully")
