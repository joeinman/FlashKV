#pragma once

#include <cstdint>
#include <functional>

#include <string>
#include <vector>

namespace FlashKV
{
    using FlashReadFunc = std::function<bool(uint32_t flash_offs, size_t count)>;
    using FlashWriteFunc = std::function<bool(uint32_t flash_offs, size_t count)>;
    using FlashEraseFunc = std::function<bool(uint32_t flash_offs)>;

    class FlashKV
    {
    public:
        FlashKV(FlashWriteFunc flashWriteFunc,
                FlashReadFunc flashReadFunc,
                FlashEraseFunc flashEraseFunc,
                size_t pageSize,
                size_t sectorSize,
                size_t offset,
                size_t size);

        ~FlashKV();

        bool writeKey(uint8_t key, uint8_t value);
        bool readKey(uint8_t key, uint8_t &value);
        bool eraseKey(uint8_t key);

    private:
        FlashWriteFunc _flashWriteFunc; // Function for writing to Flash memory.
        FlashReadFunc _flashReadFunc;   // Function for reading from Flash memory.
        FlashEraseFunc _flashEraseFunc; // Function for erasing from Flash memory.
    };

} // namespace FlashKV
