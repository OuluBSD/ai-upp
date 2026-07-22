#include "AMP.h"

NAMESPACE_UPP

Vector<AmpDeviceInfo> EnumerateAmpDevices()
{
	Vector<AmpDeviceInfo> result;
#ifdef HAVE_SYSTEM_AMP
	std::vector<concurrency::accelerator> devices = concurrency::accelerator::get_all();
	result.SetCount((int)devices.size());
	for(int i = 0; i < (int)devices.size(); i++) {
		const concurrency::accelerator& device = devices[i];
		AmpDeviceInfo& item = result[i];
		item.index = i;
		item.description = WString(device.description.c_str()).ToString();
		item.device_path = WString(device.device_path.c_str()).ToString();
		item.is_emulated = device.is_emulated;
		item.has_display = device.has_display;
		item.dedicated_memory = (int64)device.dedicated_memory;
	}
#endif
	return result;
}

static ValueMap AmpDeviceToValue(const AmpDeviceInfo& device)
{
	ValueMap value;
	value.Add("index", device.index);
	value.Add("description", device.description);
	value.Add("device_path", device.device_path);
	value.Add("is_emulated", device.is_emulated);
	value.Add("has_display", device.has_display);
	value.Add("dedicated_memory", device.dedicated_memory);
	return value;
}

bool SaveAmpDeviceInventory(const String& path, const Vector<AmpDeviceInfo>& devices)
{
	ValueArray array;
	for(const AmpDeviceInfo& device : devices)
		array.Add(AmpDeviceToValue(device));
	ValueMap root;
	root.Add("devices", array);
	root.Add("count", array.GetCount());
	root.Add("format", "amp-device-inventory-v1");
	if(!SaveFile(path, AsJSON(root, true))) {
		COUTLOG(Format("amp_inventory_write_failed path=%s", ~path));
		return false;
	}
	COUTLOG(Format("amp_inventory_written path=%s count=%d", ~path, array.GetCount()));
	return true;
}

bool LoadAmpDeviceInventory(const String& path, Vector<AmpDeviceInfo>& devices, String& error)
{
	String content = LoadFile(path);
	if(content.IsEmpty()) {
		error = "inventory file is empty or unreadable";
		return false;
	}
	Value root_value = ParseJSON(content);
	if(root_value.IsError() || !root_value.Is<ValueMap>()) {
		error = "inventory root is not a JSON object";
		return false;
	}
	ValueArray array = ValueMap(root_value).Get("devices", ValueArray());
	devices.Clear();
	for(int i = 0; i < array.GetCount(); i++) {
		Value value = array[i];
		if(!value.Is<ValueMap>()) {
			error = Format("inventory device %d is not an object", i);
			return false;
		}
		ValueMap item = value;
		AmpDeviceInfo device;
		device.index = (int)item.Get("index", i);
		device.description = AsString(item.Get("description", Value()));
		device.device_path = AsString(item.Get("device_path", Value()));
		device.is_emulated = (bool)item.Get("is_emulated", false);
		device.has_display = (bool)item.Get("has_display", false);
		device.dedicated_memory = (int64)item.Get("dedicated_memory", (int64)0);
		if(device.device_path.IsEmpty()) {
			error = Format("inventory device %d has no device path", i);
			return false;
		}
		devices.Add(device);
	}
	COUTLOG(Format("amp_inventory_loaded path=%s count=%d", ~path, devices.GetCount()));
	return true;
}

bool SelectAmpDevice(const Vector<AmpDeviceInfo>& devices, int index, AmpDeviceInfo& selected, String& error)
{
	if(index < 0 || index >= devices.GetCount()) {
		error = Format("device index %d is outside inventory size %d", index, devices.GetCount());
		return false;
	}
	selected = devices[index];
	COUTLOG(Format("amp_device_selected index=%d path=%s description=%s", index,
	               ~selected.device_path, ~selected.description));
	return true;
}

END_UPP_NAMESPACE
