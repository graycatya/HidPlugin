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
    std::wstring manufacturer_string;
    std::wstring product_string;
    unsigned short release_number;
    hid_bus_type bus_type;
    std::wstring serial_number;
    std::string path;


private:

};

class HidPlugin 
{
public:
    explicit HidPlugin();
    ~HidPlugin();

    int Init_Hid(unsigned short vendor_id = 0x0, unsigned short product_id = 0x0);
    int Exit_HId();

    void Register_Hotplug(unsigned short vendor_id, unsigned short product_id);
    void Deregister_Hotplug();

    void SetHotplug_SleepMs(int ms);
    int GetHotplug_SleepMs();

    void Register_Hotplug_Callback(std::function<void(std::list<HidDevice>, std::list<HidDevice>)> callback);
    void Deregister_Hotplug_Callback();

    int Open_Write_Device(int info_index);
    int Open_Write_Device(std::string path);
    int Close_Write_Device();
    int Open_Read_Device(int info_index);
    int Open_Read_Device(std::string path);
    int Close_Read_Device();
    int Write_Data(std::string data);
    int Send_Feature_report(std::string data);

    void SetReadData_SleepMs(int ms);
    int GetReadData_SleepMs();

    void Register_ReadData_Callback(std::function<void(std::string)> callback);
    void Deregister_ReadData_Callback();

    std::map<std::string, HidDevice> GetHidDevices();

private:
    void Copy_Device(hid_device_info *hid_info, std::map<std::string, HidDevice> &devices);
    void Compare_Devices(std::map<std::string, HidDevice> &original, std::map<std::string, HidDevice> &current);

    void Start_ReadData_Thread();
    void Stop_ReadData_Thread();

private:
    std::thread* m_pHotplug_thread = nullptr;
    int m_nHotplug_sleepMs = 1000;
    bool m_bHotplugThreadStop = true;
    //std::map<std::string, Usb_Device> m_pUsb_devices;
    std::map<std::string, HidDevice> m_pHid_devices;

    std::function<void(std::list<HidDevice>, std::list<HidDevice>)> m_pHotplug_callback = nullptr;

    std::thread* m_pReadData_thread = nullptr;
    int m_nReadData_sleepMs = 1000;
    bool m_bReadDataThreadStop = true;

    std::function<void(std::string)> m_pReadData_callback = nullptr;

    hid_device *write_device = nullptr;
    hid_device *read_device = nullptr;

    std::mutex* m_pMutex = nullptr;

    friend class Usb_Device;
};

#endif 
