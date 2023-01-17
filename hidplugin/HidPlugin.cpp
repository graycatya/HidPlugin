#include "HidPlugin.h"

#include <hidapi.h>
#include <iostream>
#include <stdlib.h>

// Headers needed for sleeping.
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

// Fallback/example
#ifndef HID_API_MAKE_VERSION
#define HID_API_MAKE_VERSION(mj, mn, p) (((mj) << 24) | ((mn) << 8) | (p))
#endif
#ifndef HID_API_VERSION
#define HID_API_VERSION HID_API_MAKE_VERSION(HID_API_VERSION_MAJOR, HID_API_VERSION_MINOR, HID_API_VERSION_PATCH)
#endif

//
// Sample using platform-specific headers
#if defined(__APPLE__) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <hidapi_darwin.h>
#endif

#if defined(_WIN32) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <hidapi_winapi.h>
#endif

#if defined(USING_HIDAPI_LIBUSB) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <hidapi_libusb.h>
#endif

HidPlugin::HidPlugin() {
    m_pMutex = new std::mutex;
    m_pMutex->lock();
    m_pHid_devices.clear();
    m_pMutex->unlock();
}

HidPlugin::~HidPlugin() {
    delete m_pMutex;
    m_pMutex = nullptr;
}

int HidPlugin::Init_Hid(unsigned short vendor_id, unsigned short product_id)
{
    int ret = hid_init();
    if(ret == 0)
    {
        hid_device_info *hid_info;//usb链表
        /*打开指定VID PID设备*/
        hid_info = hid_enumerate(vendor_id,product_id);
        //hid_info = hid_enumerate(0x3243,0x0122);
        /*遍历所有信息并打印*/
        std::map<std::string, HidDevice> devices;
        Copy_Device(hid_info, m_pHid_devices);
        /*释放链表*/
        hid_free_enumeration(hid_info);
    }
    return ret;
}

int HidPlugin::Exit_HId()
{
    return hid_exit();
}

void HidPlugin::Register_Hotplug(unsigned short vendor_id, unsigned short product_id)
{
    m_bHotplugThreadStop = false;
    m_pHotplug_thread = new std::thread([=]{
         while(!m_bHotplugThreadStop)
         {
             hid_device_info *hid_info;//usb链表
             /*打开指定VID PID设备*/
             hid_info = hid_enumerate(vendor_id,product_id);
             //hid_info = hid_enumerate(0x3243,0x0122);
             /*遍历所有信息并打印*/
             std::map<std::string, HidDevice> devices;
             Copy_Device(hid_info, devices);
             /*释放链表*/
             hid_free_enumeration(hid_info);
             Compare_Devices(m_pHid_devices, devices);
             std::this_thread::sleep_for(std::chrono::milliseconds(m_nHotplug_sleepMs));
         }
    });
}

void HidPlugin::Deregister_Hotplug()
{
    if(m_pHotplug_thread != nullptr)
    {
        m_bHotplugThreadStop = true;
        m_pHotplug_thread->join();
        delete m_pHotplug_thread;
        m_pHotplug_thread = nullptr;
    }
}

void HidPlugin::SetHotplug_SleepMs(int ms)
{
    m_nHotplug_sleepMs = ms;
}

int HidPlugin::GetHotplug_SleepMs()
{
    return m_nHotplug_sleepMs;
}

void HidPlugin::Register_Hotplug_Callback(std::function<void (std::list<HidDevice>, std::list<HidDevice>)> callback)
{
    if(m_pHotplug_callback == nullptr)
    {
        m_pHotplug_callback = callback;
    }
}

void HidPlugin::Deregister_Hotplug_Callback()
{
    if(m_pHotplug_callback)
    {
        m_pHotplug_callback = nullptr;
    }
}

//int HidPlugin::Open_Device(unsigned short vendor_id, unsigned short product_id, std::wstring serial_number)
//{
//    if(device != nullptr)
//    {
//        return -1;
//    }

//    device = hid_open(vendor_id, product_id, serial_number.c_str());

//    if(device == nullptr)
//    {
//        return -2;
//    }
//    /*阻塞*/
//    hid_set_nonblocking(device, 0);
//    Start_ReadData_Thread();

//    return 0;

//}

int HidPlugin::Open_Write_Device(std::string path)
{
    if(write_device != nullptr)
    {
        return -1;
    }

    write_device = hid_open_path(path.c_str());
    if(write_device == nullptr)
    {
        return -2;
    }

    return 0;
}

int HidPlugin::Close_Write_Device()
{
    if(write_device)
    {
        hid_close(write_device);
        write_device = nullptr;
        return 0;
    }

    return -1;
}

int HidPlugin::Open_Read_Device(int info_index)
{
    int ret = -1;
    m_pMutex->lock();
    std::map<std::string, HidDevice>::iterator it;
    int index = 0;
    for(it=m_pHid_devices.begin(); it!=m_pHid_devices.end();++it,index++)
    {
        if(info_index == index)
        {
            ret = Open_Read_Device(it->second.path);
            m_pMutex->unlock();
            return ret;
        }
    }
    m_pMutex->unlock();
    return ret;
}

int HidPlugin::Open_Read_Device(std::string path)
{
    if(read_device != nullptr)
    {
        return -1;
    }

    read_device = hid_open_path(path.c_str());
    if(read_device == nullptr)
    {
        return -2;
    }
    ///*阻塞*/
    std::cout << "set nonblocking: " << hid_set_nonblocking(read_device, 0) << std::endl;
    Start_ReadData_Thread();

    return 0;
}

int HidPlugin::Close_Read_Device()
{
    if(read_device)
    {
        Stop_ReadData_Thread();
        hid_close(read_device);
        read_device = nullptr;
        return 0;
    }

    return -1;
}

int HidPlugin::Write_Data(std::string data)
{
    if(write_device)
    {
        return hid_write(write_device, (const unsigned char *)(data.c_str()), data.length());
    }
    return -1;
}

int HidPlugin::Send_Feature_report(std::string data)
{
    if(write_device)
    {
        return hid_send_feature_report(write_device, (const unsigned char *)(data.c_str()), data.length());
    }
    return -1;
}

void HidPlugin::SetReadData_SleepMs(int ms)
{
    m_nReadData_sleepMs = ms;
}

int HidPlugin::GetReadData_SleepMs()
{
    return m_nReadData_sleepMs;
}

void HidPlugin::Register_ReadData_Callback(std::function<void (std::string)> callback)
{
    if(m_pReadData_callback == nullptr)
    {
        m_pReadData_callback = callback;
    }
}

void HidPlugin::Deregister_ReadData_Callback()
{
    if(m_pReadData_callback)
    {
        m_pReadData_callback = nullptr;
    }
}

std::map<std::string, HidDevice> HidPlugin::GetHidDevices()
{
    return m_pHid_devices;
}

void HidPlugin::Copy_Device(hid_device_info *hid_info, std::map<std::string, HidDevice> &devices)
{
    for(;hid_info != nullptr;hid_info = hid_info->next){
        HidDevice device;
        device.interface_number = hid_info->interface_number;
        device.manufacturer_string = hid_info->manufacturer_string;
        device.product_string = hid_info->product_string;
        device.release_number = hid_info->release_number;
        device.bus_type = hid_info->bus_type;
        device.serial_number = hid_info->serial_number;
        device.path = hid_info->path;

//        std::cout << "interface_number: " << hid_info->interface_number << std::endl; // 设备接口号
//        std::cout << "manufacturer_string: " << hid_info->manufacturer_string << std::endl; // 厂商字符串
//        std::cout << "product_string: " << hid_info->product_string << std::endl; // 设备字符串
//        std::cout << "release_number: " << hid_info->release_number << std::endl; // 版本号
//        std::cout << "bus_type: " << hid_info->bus_type << std::endl;
//        std::cout << "serial_number: " << hid_info->serial_number << std::endl;
//        std::cout << "path: " << hid_info->path << std::endl; // 地址
        devices[device.path] = device;
    }
}

void HidPlugin::Compare_Devices(std::map<std::string, HidDevice> &original, std::map<std::string, HidDevice> &current)
{
    std::map<std::string, HidDevice> t_original = original;
    std::map<std::string, HidDevice> t_current = current;
    std::list<HidDevice> adds;
    std::list<HidDevice> dels;

    std::list<std::string> delkeys;
    if(t_original.size() < t_current.size())
    {
        for(auto it = t_original.begin(); it != t_original.end(); ++it)
        {
            if(t_current.count(it->first) > 0)
            {
                delkeys.push_back(it->first);
            }
        }
    } else {
        for(auto it = t_current.begin(); it != t_current.end(); ++it)
        {
            if(t_current.count(it->first) > 0)
            {
                delkeys.push_back(it->first);
            }
        }
    }
    for(auto it = delkeys.begin(); it != delkeys.end(); ++it)
    {
        t_original.erase(it->data());
        t_current.erase(it->data());
    }
    for (auto it = t_original.begin(); it != t_original.end() ; ++it) {
        dels.push_back(it->second);
    }
    for (auto it = t_current.begin(); it != t_current.end() ; ++it) {
        adds.push_back(it->second);
    }
    if(m_pHotplug_callback)
    {
        m_pHotplug_callback(adds, dels);
    }
    m_pMutex->lock();
    original.clear();
    original = current;
    m_pMutex->unlock();
}

void HidPlugin::Start_ReadData_Thread()
{
    m_bReadDataThreadStop = false;
    m_pReadData_thread = new std::thread([=]{
         while(!m_bReadDataThreadStop)
         {
             int res;
             unsigned char buf[64+1];
             //buf[0] = 0x17;
             res = hid_read(read_device, buf, 64+1);
             std::cout << "read res: " << res << std::endl;
             if(res < 0){
                 /*返回值查看*/
                 std::cout << "err_string = " << std::wstring(hid_error(read_device)).c_str() << std::endl;
             }
//             hid_set_nonblocking(read_device, 0);
            for(int i = 0;i<8;i++){
                std::cout << "buf[" << i << "]: " << std::hex << (int)buf[i] << std::endl;
            }
             //std::cout << buf << std::endl;

             std::this_thread::sleep_for(std::chrono::milliseconds(m_nReadData_sleepMs));
         }
    });
}

void HidPlugin::Stop_ReadData_Thread()
{
    if(m_pReadData_thread != nullptr)
    {
        m_bReadDataThreadStop = true;
        m_pReadData_thread->join();
        delete m_pReadData_thread;
        m_pReadData_thread = nullptr;
    }
}

int HidPlugin::Open_Write_Device(int info_index)
{
    int ret = -1;
    m_pMutex->lock();
    std::map<std::string, HidDevice>::iterator it;
    int index = 0;
    for(it=m_pHid_devices.begin(); it!=m_pHid_devices.end();++it,index++)
    {
        if(info_index == index)
        {
            ret = Open_Write_Device(it->second.path);
            m_pMutex->unlock();
            return ret;
        }
    }
    m_pMutex->unlock();
    return ret;
}
