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
            // The Flash memory contains a valid FlashKV store.
            // Start reading after the signature.
            uint32_t address = _flashAddress + _flashPageSize;
            while (address < _flashAddress + _flashSize)
            {
                // Read the key size.
                uint32_t keySize;
                if (!_flashReadFunc(address, reinterpret_cast<uint8_t *>(&keySize), sizeof(keySize)))
                    return false;
                address += sizeof(keySize);

                if (keySize == 0)
                    break;

                // Read the key.
                std::string key(keySize, '\0');
                if (!_flashReadFunc(address, reinterpret_cast<uint8_t *>(&key[0]), keySize))
                    return false;
                address += keySize;

                // Read the value size.
                uint32_t valueSize;
                if (!_flashReadFunc(address, reinterpret_cast<uint8_t *>(&valueSize), sizeof(valueSize)))
                    return false;
                address += sizeof(valueSize);

                // Read the value.
                std::vector<uint8_t> value(valueSize);
                if (!_flashReadFunc(address, value.data(), valueSize))
                    return false;
                address += valueSize;

                // Add the key-value pair to the store.
                _keyValueStore[key] = value;
            }

            _storeLoaded = true;
            return 0;
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
        // If the store hasn't been loaded, return false.
        if (!_storeLoaded)
            return false;

        // Erase the entire Flash memory.
        if (!_flashEraseFunc(_flashAddress, _flashSize))
            return false;

        // Create a buffer for the signature and the serialized key-value store.
        std::vector<uint8_t> buffer;

        // Copy the signature into the start of the buffer.
        buffer.insert(buffer.end(), FLASHKV_SIGNATURE.begin(), FLASHKV_SIGNATURE.end());

        // Serialize the key-value pairs into the buffer.
        for (const auto &pair : _keyValueStore)
        {
            // Write the key size.
            uint32_t keySize = pair.first.size();
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(&keySize), reinterpret_cast<const uint8_t *>(&keySize) + sizeof(keySize));

            // Write the key.
            buffer.insert(buffer.end(), pair.first.begin(), pair.first.end());

            // Write the value size.
            uint32_t valueSize = pair.second.size();
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t *>(&valueSize), reinterpret_cast<const uint8_t *>(&valueSize) + sizeof(valueSize));

            // Write the value.
            buffer.insert(buffer.end(), pair.second.begin(), pair.second.end());
        }

        // Write the buffer to the Flash memory in page-sized chunks.
        size_t numFullPages = buffer.size() / _flashPageSize;
        for (size_t i = 0; i < numFullPages; ++i)
        {
            if (!_flashWriteFunc(_flashAddress + i * _flashPageSize, &buffer[i * _flashPageSize], _flashPageSize))
                return false;
        }

        // Write any remaining data and pad the final page with zeros.
        size_t remainingDataSize = buffer.size() % _flashPageSize;
        if (remainingDataSize > 0)
        {
            std::vector<uint8_t> finalPage(_flashPageSize, 0);
            std::copy(buffer.end() - remainingDataSize, buffer.end(), finalPage.begin());

            if (!_flashWriteFunc(_flashAddress + numFullPages * _flashPageSize, finalPage.data(), _flashPageSize))
                return false;
        }

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
