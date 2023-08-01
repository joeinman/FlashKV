#pragma once

#include <unordered_map>
#include <functional>
#include <algorithm>
#include <optional>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace FlashKV
{
    // Define the signature and its size.
    const std::array<uint8_t, 4> FLASHKV_SIGNATURE = {'F', 'K', 'V', 'S'};
    const size_t SIGNATURE_SIZE = FLASHKV_SIGNATURE.size();

    using FlashWriteFunc = std::function<bool(uint32_t flashAddress, const uint8_t *data, size_t count)>;
    using FlashReadFunc = std::function<bool(uint32_t flashAddress, uint8_t *data, size_t count)>;
    using FlashEraseFunc = std::function<bool(uint32_t flashAddress, size_t count)>;

    using KeyValueStore = std::unordered_map<std::string, std::vector<uint8_t>>;

    class FlashKV
    {
    public:
        FlashKV(FlashWriteFunc flashWriteFunc,
                FlashReadFunc flashReadFunc,
                FlashEraseFunc flashEraseFunc,
                size_t flashPageSize,
                size_t flashSectorSize,
                size_t flashAddress,
                size_t flashSize);

        ~FlashKV();

        int loadStore();
        bool saveStore();

        bool writeKey(std::string key, std::vector<uint8_t> value);
        std::optional<std::vector<uint8_t>> readKey(std::string key);
        bool eraseKey(std::string key);

    private:
        FlashWriteFunc _flashWriteFunc; // Function for writing to Flash memory.
        FlashReadFunc _flashReadFunc;   // Function for reading from Flash memory.
        FlashEraseFunc _flashEraseFunc; // Function for erasing from Flash memory.

        KeyValueStore _keyValueStore; // In-memory key-value store.
        size_t _flashPageSize;        // Size of a page in Flash memory.
        size_t _flashSectorSize;      // Size of a sector in Flash memory.
        size_t _flashAddress;         // Address of the Flash memory to use for the key-value store.
        const size_t _flashSize;      // Size of the Flash memory to use for the key-value store.
        bool _storeLoaded;            // Whether the key-value store has been loaded from Flash memory.
    };

} // namespace FlashKV
