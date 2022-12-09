#include "HidPlugin.h"
#include <iostream>

static void Hotplug_callback(std::list<HidDevice> adds, std::list<HidDevice> dels) {
    //std::cout << "adds: " << std::to_string(adds.size()) << " dels: " << std::to_string(dels.size()) << std::endl;
    fflush(stdout);
}

int main(int argv, char** argc) {
    HidPlugin plugin;
    plugin.Init_Hid();

    plugin.Register_Hotplug_Callback(Hotplug_callback);
    plugin.SetHotplug_SleepMs(100);
    plugin.Register_Hotplug(0x4C4A, 0x4155);
    // 000001377F1476D0
    //std::cout << "open device: " << plugin.Open_Device(0x4C4A, 0x4155, L"5E321385364D1546") << std::endl;
    std::cout << "open device: " << plugin.Open_Device("\\?\HID#VID_4C4A&PID_4155&MI_01&Col02#7&3865c1dd&0&0001#{4d1e55b2-f16f-11cf-88cb-001111000030}");
    std::this_thread::sleep_for(std::chrono::milliseconds(40000));
    plugin.Deregister_Hotplug();
    plugin.Exit_HId();
}
