#include "HidPlugin.h"

#include <hidapi.h>
#include <iostream>

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
    
}

HidPlugin::~HidPlugin() {
    
}

int HidPlugin::Init_Hid()
{
    return hid_init();
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
             for(;hid_info != nullptr;hid_info = hid_info->next){
                 std::cout << "interface_number: " << hid_info->interface_number << std::endl; // 设备接口号
                 std::cout << "manufacturer_string: " << hid_info->manufacturer_string << std::endl; // 厂商字符串
                 std::cout << "product_string: " << hid_info->product_string << std::endl; // 设备字符串
                 std::cout << "release_number: " << hid_info->release_number << std::endl; // 版本号
                 std::cout << "path: " << hid_info->path << std::endl; // 地址
             }
             /*释放链表*/
             hid_free_enumeration(hid_info);
             //Compare_Devices(_instance->m_pUsb_devices, devices);
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
