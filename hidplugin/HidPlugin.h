#ifndef HIDPLUGIN_H
#define HIDPLUGIN_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <map>
#include <list>
#include <string>

class HidPlugin 
{
public:
    explicit HidPlugin();
    ~HidPlugin();

    void Register_Hotplug(unsigned short vendor_id, unsigned short product_id);
    void Deregister_Hotplug();

    void SetHotplug_SleepMs(int ms);
    int GetHotplug_SleepMs();

//    void Register_Hotplug_Callback(std::function<void(std::list<Usb_Device>, std::list<Usb_Device>)> callback);
//    void Deregister_Hotplug_Callback();

private:
    std::thread* m_pHotplug_thread = nullptr;
    int m_nHotplug_sleepMs = 1;
    bool m_bHotplugThreadStop = true;
    //std::map<std::string, Usb_Device> m_pUsb_devices;
};

#endif 
