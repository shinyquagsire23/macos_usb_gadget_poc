# macos_usb_gadget_poc

This repository contains a proof-of-concept for interfacing with IOUSBDeviceFamily on ARM64 MacBooks. Currently it implements a HID keyboard which types "my laptop is a keyboard. " over and over.

**Requirements:**

 - SIP must be disabled (`sudo spctl --master-disable`)
 - Set `amfi_get_out_of_my_way=1` in the boot args
 - Maybe disable library verification?
 - I also had `-arm64e_preview_abi` in my boot args but idk if that's required

**Caveats:**

 - `amfi_get_out_of_my_way=1` breaks a lot of stuff, proceed with caution. I cannot launch Firefox nor Discord with it on.
 - This probably will also break network sharing on the port after use. It's fixable via reboot, or by calling `IOUSBDeviceDescriptionAppendInterfaceToConfiguration` with all the original NCM functions.

**Compiling/Running:**

 - `make clean && make && ./usb_device`