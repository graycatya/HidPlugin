#include "HidPlugin.h"

int main(int argv, char** argc) {
    HidPlugin plugin;
    plugin.Init_Hid();
    plugin.Register_Hotplug(0, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    plugin.Deregister_Hotplug();
    plugin.Exit_HId();
}
