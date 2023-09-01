# FlashKV

![CMake](https://github.com/joeinman/FlashKV/actions/workflows/cmake.yml/badge.svg)

FlashKV is a lightweight, hardware agnostic key-value map for flash memory. It provides an easy-to-use interface for storing, reading, and erasing key-value pairs in flash memory.

## Features:

- **Simple Interface**: FlashKV provides a straightforward API for interacting with flash memory.
- **Customizable**: The library can be easily customized to work with different flash memory configurations by providing appropriate read, write, and erase functions.

## Basic Example:

```cpp
#include <FlashKV/FlashKV.h>
#include <iostream>
#include <optional>
#include <vector>

// Define the size of the FlashKV map
#define FLASHKV_SIZE 4096
#define FLASHKV_BASE 0

int main()
{
    // Create FlashKV Object
    FlashKV::FlashKV flashKV(
        [](uint32_t flashAddress, const uint8_t *data, size_t count) -> bool {
            // Implement Flash Write Function
            return true;
        },
        [](uint32_t flashAddress, uint8_t *data, size_t count) -> bool {
            // Implement Flash Read Function
            return true;
        },
        [](uint32_t flashAddress, size_t count) -> bool {
            // Implement Flash Erase Function
            return true;
        },
        FLASH_PAGE_SIZE,
        FLASH_SECTOR_SIZE,
        FLASHKV_BASE,
        FLASHKV_SIZE);
    );

    // Load The Map From Flash
    if (flashKV.loadMap())
        std::cout << "FlashKV Map Loaded Successfully" << std::endl;
    else
        std::cout << "Error Occurred While Loading The FlashKV Map" << std::endl;

    // Try To Read A Value From The Map
    std::string key = "test";
    std::optional<std::vector<uint8_t>> value = flashKV.readKey(key);
    if (value)
        std::cout << "Key [" << key << "] Found." << std::endl;
    else
        std::cout << "Key [" << key << "] Not Found." << std::endl;

    // Write A Value To The Map
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    flashKV.writeKey(key, data);
    std::cout << "Writing Key: [" << key << "]" << std::endl;

    // Save The Map To Flash
    if (flashKV.saveMap())
        std::cout << "FlashKV Map Saved Successfully" << std::endl;
    else
        std::cout << "Error Occurred While Saving The FlashKV Map" << std::endl;

    return 0;
}
```
