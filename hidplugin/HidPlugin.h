#ifndef HIDPLUGIN_H
#define HIDPLUGIN_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <map>
#include <list>
#include <string>
#include <hidapi.h>

class HidDevice

{
public:
    int interface_number;
    std::string manufacturer_string;
    std::string product_string;
    unsigned short release_number;
    hid_bus_type bus_type;
    std::string serial_number;
    std::string path;


private:

};

class HidPlugin 
{
public:
    explicit HidPlugin();
    ~HidPlugin();

    int Init_Hid();
    int Exit_HId();

    void Register_Hotplug(unsigned short vendor_id, unsigned short product_id);
    void Deregister_Hotplug();

    void SetHotplug_SleepMs(int ms);
    int GetHotplug_SleepMs();

    void Register_Hotplug_Callback(std::function<void(std::list<HidDevice>, std::list<HidDevice>)> callback);
    void Deregister_Hotplug_Callback();

private:
    void Copy_Device(hid_device_info *hid_info, std::list<HidDevice> &devices);

private:
    std::thread* m_pHotplug_thread = nullptr;
    int m_nHotplug_sleepMs = 1000;
    bool m_bHotplugThreadStop = true;
    //std::map<std::string, Usb_Device> m_pUsb_devices;
    std::list<HidDevice> m_pHid_devices;

    std::function<void(std::list<HidDevice>, std::list<HidDevice>)> m_pHotplug_callback = nullptr;

    friend class Usb_Device;
};

#endif 
