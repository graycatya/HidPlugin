#include "HidPlugin.h"
#include <iostream>

static void Hotplug_callback(std::list<HidDevice> adds, std::list<HidDevice> dels) {
    //std::cout << "adds: " << std::to_string(adds.size()) << " dels: " << std::to_string(dels.size()) << std::endl;
    fflush(stdout);
}

int main(int argv, char** argc) {
    HidPlugin plugin;
    plugin.Init_Hid(0x4C4A, 0x4155);
    //plugin.Init_Hid(0x3243, 0x0122);

    plugin.Register_Hotplug_Callback(Hotplug_callback);
    plugin.SetHotplug_SleepMs(100);
    plugin.Register_Hotplug(0x4C4A, 0x4155);
    //plugin.Register_Hotplug(0x3243, 0x0122);
    // 000001377F1476D0
    std::cout << "open device: " << plugin.Open_Write_Device(2) << std::endl;
    std::cout << "open device: " << plugin.Open_Read_Device(2) << std::endl;
    char buf[63]={0};
    buf[0] = 0x03;//Report ID（自定义）
    //buf[0] = 0x0D;//Report ID（自定义）
    buf[1] = 0x0b;//随便一点东西
    buf[2] = 0xf0;
    buf[3] = 0x0f;
    buf[4] = 0xf0;
    buf[5] = 0x00;
    buf[6] = 0x01;
    buf[7] = 0x00;
    buf[8] = 0x00;
    std::cout << "write device: " << plugin.Write_Data(buf) << std::endl;
    //std::cout << "Send_Feature_report: " << plugin.Send_Feature_report(buf) << std::endl;
    //std::cout << "open device: " << plugin.Open_Device(0x4C4A, 0x4155, L"5E321385364D1546") << std::endl;
    //std::cout << "open device: " << plugin.Open_Device("HID#VID_4C4A&PID_4155&MI_01&Col02");
    std::this_thread::sleep_for(std::chrono::milliseconds(40000));
    plugin.Deregister_Hotplug();
    plugin.Exit_HId();
}
