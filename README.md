# FlashKV

FlashKV is a simple, lightweight key-value store library for Flash memory. It provides an easy-to-use interface for storing, reading, and erasing key-value pairs in Flash memory.

## Features:

- **Simple Interface**: FlashKV provides a straightforward API for interacting with Flash memory.
- **Customizable**: The library can be easily customized to work with different Flash memory configurations by providing appropriate read, write, and erase functions.

## Basic Example:

A basic example of using the library with an RP2040 microcontroller is shown below.

```cpp
// Includes
#include <hardware/flash.h>
#include <pico/stdlib.h>
#include <tusb.h>

#include <FlashKV/FlashKV.h>

#include <iostream>
#include <optional>
#include <iomanip>
#include <vector>

// Address Of The FlashKV Store In Flash Memory
#define FLASHKV_BASE XIP_BASE + (1024 * 1040)
#define FLASHKV_SIZE FLASH_SECTOR_SIZE * 2

int main()
{
    stdio_init_all();
    while (!tud_cdc_connected())
        tight_loop_contents();

    // Create a FlashKV object.
    FlashKV::FlashKV flashKV(
        [](uint32_t flashAddress, const uint8_t *data, size_t count) -> bool
        {
            flash_range_program(flashAddress - XIP_BASE, data, count);
            return true;
        },
        [](uint32_t flashAddress, uint8_t *data, size_t count) -> bool
        {
            memcpy(data, reinterpret_cast<uint8_t *>(flashAddress), count);
            return true;
        },
        [](uint32_t flashAddress, size_t count) -> bool
        {
            sleep_ms(100);
            flash_range_erase(flashAddress - XIP_BASE, count);
            return true;
        },
        FLASH_PAGE_SIZE,
        FLASH_SECTOR_SIZE,
        FLASHKV_BASE,
        FLASHKV_SIZE);

    // Load The Store From Flash
    int loadStatus = flashKV.loadStore();
    if (loadStatus == 0)
        std::cout << "FlashKV Store Loaded Successfully" << std::endl;
    else if (loadStatus == 1)
        std::cout << "No FlashKV Store Found. A New One Will Be Created Upon Saving" << std::endl;
    else
        std::cout << "Error Occurred While Loading The FlashKV Store" << std::endl;

    // Try To Read A Value From The Store
    std::string key = "test";
    std::optional<std::vector<uint8_t>> value = flashKV.readKey(key);
    if (value)
    {
        std::cout << "Read Key: [" << key << "], Value: [";
        for (auto it = value->begin(); it != value->end(); ++it)
        {
            if (it != value->begin())
                std::cout << " ";
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)*it;
        }
        std::cout << "]" << std::endl;
    }
    else
    {
        std::cout << "Key [" << key << "] Not Found." << std::endl;
    }

    // Write A Value To The Store
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    flashKV.writeKey(key, data);

    std::cout << "Writing Key: [" << key << "], Value: [";
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        if (it != data.begin())
            std::cout << " ";
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)*it;
    }
    std::cout << "]" << std::endl;

    // Save The Store To Flash
    if (flashKV.saveStore())
        std::cout << "FlashKV Store Saved Successfully" << std::endl;
    else
        std::cout << "Error Occurred While Saving The FlashKV Store" << std::endl;

    while (1)
    {
        tight_loop_contents();
    }
}
```
