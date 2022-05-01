#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <mach/mach.h>
#include "IOUSBDeviceControllerLib.h"

#include "usb_hid_keys.h"

CFStringRef serial = CFSTR("Look, Custom Serial");

#define USB_EP_TYPE_CTRL (0)
#define USB_EP_TYPE_ISOC (1)
#define USB_EP_TYPE_BULK (2)
#define USB_EP_TYPE_INT  (3)

#define USB_EP_DIR_OUT (0)
#define USB_EP_DIR_IN (1)

const char* some_str = "keyboard_idk";

void make_a_keyboard()
{
    // TODO: A lot more error checking

    IOUSBDeviceControllerRef controller;
    while (IOUSBDeviceControllerCreate(kCFAllocatorDefault, &controller))
    {
        printf("Unable to get USB device controller\n");
        sleep(3);    
    }
    IOUSBDeviceDescriptionRef desc = IOUSBDeviceDescriptionCreateFromController(kCFAllocatorDefault, controller);
    IOUSBDeviceDescriptionSetSerialString(desc, serial);

    IOUSBDeviceDescriptionSetProductID(desc, 0x1234);

    IOUSBDeviceDescriptionRemoveAllConfigurations(desc);
    int configId = IOUSBDeviceDescriptionAppendConfiguration(desc, CFSTR("JustAKeyboard"), 0, 0);
    //int interfaceId1 = IOUSBDeviceDescriptionAppendInterfaceToConfiguration(desc, configId, CFSTR("AppleUSBMux"));
    int interfaceId2 = IOUSBDeviceDescriptionAppendInterfaceToConfiguration(desc, configId, CFSTR("MyUSBData"));
    //int interfaceId2 = IOUSBDeviceDescriptionAppendInterfaceToConfiguration(desc, configId, CFSTR("MyUSBControlAux"));
    //int interfaceId3 = IOUSBDeviceDescriptionAppendInterfaceToConfiguration(desc, configId, CFSTR("MyUSBData"));
    //int interfaceId4 = IOUSBDeviceDescriptionAppendInterfaceToConfiguration(desc, configId, CFSTR("MyUSBDataAux"));
    //CFDictionarySetValue(desc->info, , kCFBooleanTrue);

    IOReturn ret = IOUSBDeviceControllerSetDescription(controller, desc);
    printf("%x: system=%x subsystem=%x code=%x\n", ret, err_get_system(ret), err_get_sub(ret), err_get_code(ret));

    IOUSBDeviceControllerSetPreferredConfiguration(controller, configId);

    //IOUSBDeviceControllerSendCommand(controller, CFSTR("ShowDeviceDebug"), NULL); 

    CFMutableDictionaryRef match = IOServiceMatching("IOUSBDeviceInterface");
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(dict, CFSTR("USBDeviceFunction"), CFSTR("MyUSBData"));
    CFDictionarySetValue(match, CFSTR("IOPropertyMatch"), dict);
    io_service_t service;
    while(1) {
        CFRetain(match);
        service = IOServiceGetMatchingService(kIOMasterPortDefault, match);
        if(!service) {
            printf("Didn't find, trying again\n");
            sleep(1);
        } else {
            break;
        }
    }
    CFRelease(match);

    io_string_t            path;
    if (IORegistryEntryGetPath(service, kIOServicePlane, path) != KERN_SUCCESS) {

    }

    printf("Connecting to: '%s'\n", path);

    io_connect_t dataPort;
    kern_return_t open_ret = IOServiceOpen(service, mach_task_self(), 123, &dataPort);
    printf("open_ret %x %x\n", open_ret, dataPort);
    
    uint64_t output[10];
    uint32_t outputCount;

    uint64_t args[10] = { 0, 0, 0, 0, 0, 0,0,0,0,0 };

#if 0
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 20; j++) {
            kern_return_t s_ret = IOConnectCallScalarMethod(dataPort, j, args, i, output, &outputCount);
            printf("s_ret %x %x %x %x\n", s_ret, j, i, outputCount);
        }
    }
#endif
    // Open
    outputCount = 0;
    kern_return_t s_ret = IOConnectCallScalarMethod(dataPort, 0, args, 1, output, &outputCount);
    printf("open s_ret %x %x (%x %x)\n", s_ret, outputCount, output[0], output[1]);

    // SetClass
    outputCount = 0;
    args[0] = 3; // HID
    args[1] = configId;
    s_ret = IOConnectCallScalarMethod(dataPort, 3, args, 2, output, &outputCount);
    printf("s_ret %x %x (%x %x)\n", s_ret, outputCount, output[0], output[1]);

    // SetSubclass
    outputCount = 0;
    args[0] = 1; // boot if
    args[1] = configId;
    s_ret = IOConnectCallScalarMethod(dataPort, 4, args, 2, output, &outputCount);
    printf("s_ret %x %x (%x %x)\n", s_ret, outputCount, output[0], output[1]);

    // SetProtocol
    outputCount = 0;
    args[0] = 1; // keyboard
    args[1] = configId;
    s_ret = IOConnectCallScalarMethod(dataPort, 5, args, 2, output, &outputCount);
    printf("s_ret %x %x (%x %x)\n", s_ret, outputCount, output[0], output[1]);

#if 0
    mach_port_t wakePort;
    mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &wakePort);

    args[0] = 0;
    args[1] = (uint64_t) callback_2;
    args[2] = (uint64_t) 0; // refcon
    IOConnectCallAsyncMethod(dataPort, 8, wakePort, args, 3, NULL, 0, NULL, 0, NULL, 0, NULL, NULL);
    printf("async s_ret %x\n", s_ret, outputCount);
#endif

#if 0
    // CreatePipe
    outputCount = 1;
    output[0] = 0;
    args[0] = USB_EP_TYPE_BULK;
    args[1] = USB_EP_DIR_OUT;
    args[2] = -1;
    args[3] = 0;
    args[4] = 0;
    args[5] = configId;
    s_ret = IOConnectCallScalarMethod(dataPort, 10, args, 6, output, &outputCount);
    printf("s_ret %x %x (%x %x)\n", s_ret, outputCount, output[0], output[1]);

    // CreatePipe
    outputCount = 1;
    output[0] = 0;
    args[0] = USB_EP_TYPE_BULK; // type
    args[1] = USB_EP_DIR_IN; // direction
    args[2] = -1;
    args[3] = 0xff; // interval
    args[4] = 0;
    args[5] = configId;
    s_ret = IOConnectCallScalarMethod(dataPort, 10, args, 6, output, &outputCount);
    printf("s_ret %x %x (%x %x)\n", s_ret, outputCount, output[0], output[1]);

    // CreatePipe
    outputCount = 1;
    output[0] = 0;
    args[0] = USB_EP_TYPE_INT; // type
    args[1] = USB_EP_DIR_OUT; // direction
    args[2] = -1;
    args[3] = 0;
    args[4] = 0;
    args[5] = configId;
    s_ret = IOConnectCallScalarMethod(dataPort, 10, args, 6, output, &outputCount);
    printf("s_ret %x %x (%x %x)\n", s_ret, outputCount, output[0], output[1]);
#endif

    uint8_t hid_desc[] = {0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22, 0x5D, 0x00};
    uint8_t hid_report_desc[] = { 0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x05, 0x07, 0x19, 0xE0, 0x29, 0xE7, 0x15, 0x00, 0x25, 0x01,
  0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01, 0x75, 0x08, 0x81, 0x01, 0x95, 0x03, 0x75, 0x01,
  0x05, 0x08, 0x19, 0x01, 0x29, 0x03, 0x91, 0x02, 0x95, 0x05, 0x75, 0x01, 0x91, 0x01, 0x95, 0x06,
  0x75, 0x08, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x05, 0x07, 0x19, 0x00, 0x2A, 0xFF, 0x00, 0x81, 0x00,
  0xC0};

    // setDesc
    /*outputCount = 0;
    output[0] = some_str;
    args[0] = some_str; // idk
    s_ret = IOConnectCallMethod(dataPort, 2, NULL, 0, args, 1, NULL, NULL, NULL, NULL);
    printf("setdesc s_ret %x %x (%x %x)\n", s_ret, outputCount, output[0], output[1]);*/

    // appendStandardClassOrVendorDescriptor
    // This gets added after the interface and before endpoints
    outputCount = 0;
    args[0] = 0x21; // descriptor type
    args[1] = 0xF;
    s_ret = IOConnectCallMethod(dataPort, 6, args, 2, hid_desc, sizeof(hid_desc), output, &outputCount, NULL, NULL);
    printf("desc s_ret %x %x (%x %x)\n", s_ret, outputCount, output[0], output[1]);

    // appendNonstandardClassOrVendorDescriptor
    // this has to be asked for manually via the ctrl ep
    outputCount = 0;
    args[0] = 0x22; // class descriptor type
    args[1] = 0xF;
    s_ret = IOConnectCallMethod(dataPort, 7, args, 2, hid_report_desc, sizeof(hid_report_desc), output, &outputCount, NULL, NULL);
    printf("append s_ret %x %x (%x %x)\n", s_ret, outputCount, output[0], output[1]);

    // CreatePipe
    outputCount = 1;
    output[0] = 0;
    args[0] = USB_EP_TYPE_INT; // type
    args[1] = USB_EP_DIR_IN; // direction
    args[2] = 8;
    args[3] = 10;
    args[4] = 0;
    args[5] = configId;
    s_ret = IOConnectCallScalarMethod(dataPort, 10, args, 6, output, &outputCount);
    printf("s_ret %x %x (%x %x)\n", s_ret, outputCount, output[0], output[1]);

    uint64_t pipe_id = output[0];

    // CommitConfiguration
    outputCount = 0;
    s_ret = IOConnectCallScalarMethod(dataPort, 11, args, 0, output, &outputCount);
    printf("s_ret %x %x (%x %x)\n", s_ret, outputCount, output[0], output[1]);

    // createData
    outputCount = 3;
    args[0] = 0x1000;
    s_ret = IOConnectCallScalarMethod(dataPort, 18, args, 1, output, &outputCount);
    printf("s_ret %x %x (%x %x %x)\n", s_ret, outputCount, output[0], output[1], output[2]);

    void* dataptr = output[0];
    int64_t data_cap = output[1];
    uint64_t map_token = output[2];

    int enum_waits = 0;
    int idx = 0;
    uint8_t type_stuff[] = {KEY_M, KEY_Y, KEY_SPACE, KEY_L, KEY_A, KEY_P, KEY_T, KEY_O, KEY_P, KEY_SPACE, KEY_I, KEY_S, KEY_SPACE, KEY_A, KEY_SPACE, KEY_K, KEY_E, KEY_Y, KEY_B, KEY_O, KEY_A, KEY_R, KEY_D, KEY_DOT, KEY_SPACE};

    // Force the device to re-enumerate
    IOUSBDeviceControllerGoOffAndOnBus(controller, 1000);

    while (1)
    {
        
        uint8_t test_send[8] = {0x0, 0x0, type_stuff[idx], 0x0, 0x0, 0x0, 0x0, 0x0};

        memset(dataptr, 0, 0x10);
        memcpy(dataptr, test_send, sizeof(test_send));

        // 0xE0000001 = Device not enumerated
        // 0xE00002D6 = Send timed out
        // WritePort - press the key
        outputCount = 1; // bytes written
        args[0] = pipe_id;
        args[1] = map_token;
        args[2] = sizeof(test_send);
        args[3] = 100; // timeout ms?
        s_ret = IOConnectCallScalarMethod(dataPort, 14, args, 4, output, &outputCount);
        printf("write s_ret %x (%x)\n", s_ret, output[0]);

        if (s_ret == 0xE0000001) {
            printf("Waiting for device enumeration...\n");

            if (enum_waits++ > 5) {
                // Force the device to re-enumerate
                IOUSBDeviceControllerGoOffAndOnBus(controller, 1000);

                enum_waits = 0;
            }

            sleep (1);
            continue;
        }
        else if (s_ret == 0xE00002D6) {
            printf("Write timed out.\n");
            sleep (1);
            continue;
        }

        // Write Port - Un-press the key
        memset(dataptr, 0, 0x10);
        args[0] = pipe_id;
        args[1] = map_token;
        args[2] = sizeof(test_send);
        args[3] = 100; // timeout ms?
        s_ret = IOConnectCallScalarMethod(dataPort, 14, args, 4, output, &outputCount);
        printf("write s_ret %x (%x)\n", s_ret, output[0]);

        idx++;
        if (idx >= sizeof(type_stuff)) {
            idx = 0;
            sleep(1);
        }
    }

    // Close
    outputCount = 0;
    output[0] = 0;
    output[1] = 0;
    s_ret = IOConnectCallScalarMethod(dataPort, 1, args, 0, output, &outputCount);
    printf("close s_ret %x %x (%x %x)\n", s_ret, outputCount, output[0], output[1]);

    IOServiceClose(dataPort);

    //mach_port_mod_refs(mach_task_self(), wakePort, MACH_PORT_RIGHT_RECEIVE, -1);
}

int main()
{
    make_a_keyboard();
}