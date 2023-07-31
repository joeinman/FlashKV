#include "FlashKV/FlashKV.h"

namespace FlashKV
{
    FlashKV::FlashKV(FlashWriteFunc flashWriteFunc,
                     FlashReadFunc flashReadFunc,
                     FlashEraseFunc flashEraseFunc,
                     size_t flashPageSize,
                     size_t flashSectorSize,
                     size_t flashAddress,
                     size_t flashSize)
        : _flashWriteFunc(flashWriteFunc),
          _flashReadFunc(flashReadFunc),
          _flashEraseFunc(flashEraseFunc),
          _flashPageSize(flashPageSize),
          _flashSectorSize(flashSectorSize),
          _flashAddress(flashAddress),
          _flashSize(flashSize)
    {
    }

    FlashKV::~FlashKV()
    {
        saveStore();
    }

    int FlashKV::loadStore()
    {
        // Read the first few bytes from Flash memory.
        std::array<uint8_t, SIGNATURE_SIZE> signature;
        if (!_flashReadFunc(_flashAddress, signature.data(), SIGNATURE_SIZE))
            return false;

        // Check if the signature matches.
        if (signature == FLASHKV_SIGNATURE)
        {
            // The Flash memory contains a valid FlashKV Store
            _storeLoaded = true;
            return 0;

            // TODO: Read the store from Flash memory.
        }
        else
        {
            // The Flash memory does not contain a valid FlashKV store, Erase the memory here.
            if (!_flashEraseFunc(_flashAddress, _flashSize))
            {
                _storeLoaded = false;
                return 2;
            }

            // Write the signature to the first page of Flash memory.
            std::vector<uint8_t> buffer(_flashPageSize, 0);
            std::copy(FLASHKV_SIGNATURE.begin(), FLASHKV_SIGNATURE.end(), buffer.begin());
            if (!_flashWriteFunc(_flashAddress, buffer.data(), _flashPageSize))
            {
                _storeLoaded = false;
                return 2;
            }

            _storeLoaded = true;
            return 1;
        }
        return 2;
    }

    bool FlashKV::saveStore()
    {
        // If The Store Hasn't Been Loaded, Return False.
        if (!_storeLoaded)
            return false;

        // Erase Entire Flash Memory.
        if (!_flashEraseFunc(_flashAddress, _flashSize))
            return false;

        // TODO: Write the store to Flash memory.

        return true;
    }

    bool FlashKV::writeKey(std::string key, std::vector<uint8_t> value)
    {
        if (!_storeLoaded)
            return false;

        _keyValueStore[key] = value;
        return true;
    }

    std::optional<std::vector<uint8_t>> FlashKV::readKey(std::string key)
    {
        if (!_storeLoaded)
            return std::nullopt;

        auto it = _keyValueStore.find(key);
        if (it != _keyValueStore.end())
            return it->second;

        return std::nullopt;
    }

    bool FlashKV::eraseKey(std::string key)
    {
        if (!_storeLoaded)
            return false;

        size_t erased = _keyValueStore.erase(key);
        return erased > 0;
    }

} // namespace FlashKV
