#include "HidPlugin.h"
#include <iostream>

static void Hotplug_callback(std::list<HidDevice> adds, std::list<HidDevice> dels) {
    std::cout << "adds: " << std::to_string(adds.size()) << " dels: " << std::to_string(dels.size()) << std::endl;
    fflush(stdout);
}

int main(int argv, char** argc) {
    HidPlugin plugin;
    plugin.Init_Hid();

    plugin.Register_Hotplug_Callback(Hotplug_callback);
    plugin.SetHotplug_SleepMs(100);
    plugin.Register_Hotplug(0x4C4A, 0x4155);
    std::this_thread::sleep_for(std::chrono::milliseconds(40000));
    plugin.Deregister_Hotplug();
    plugin.Exit_HId();
}
