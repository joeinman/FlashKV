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
        // Attempt To Read The Signature From Flash
        std::array<uint8_t, FLASHKV_SIGNATURE_SIZE> signature;
        if (!_flashReadFunc(_flashAddress, signature.data(), FLASHKV_SIGNATURE_SIZE))
            return false;

        // Check If The Signature Is Valid
        if (signature == FLASHKV_SIGNATURE)
        {
            // Flash Contains A Valid FlashKV Store.
            size_t offset = FLASHKV_SIGNATURE_SIZE;
            while (1)
            {
                // Read The Size Of The Key
                uint16_t keySize;
                _flashReadFunc(_flashAddress + offset, reinterpret_cast<uint8_t *>(&keySize), sizeof(uint16_t));
                offset += sizeof(uint16_t);

                // If The Key Size Is 0, We Have Reached The End Of The Store
                if (keySize == 0)
                    break;

                // Read The Key
                std::string key;
                key.resize(keySize);
                _flashReadFunc(_flashAddress + offset, reinterpret_cast<uint8_t *>(&key[0]), keySize);
                offset += keySize;

                // Read The Size Of The Value
                uint16_t valueSize;
                _flashReadFunc(_flashAddress + offset, reinterpret_cast<uint8_t *>(&valueSize), sizeof(uint16_t));
                offset += sizeof(uint16_t);

                // Read The Value
                std::vector<uint8_t> value;
                value.resize(valueSize);
                _flashReadFunc(_flashAddress + offset, reinterpret_cast<uint8_t *>(&value[0]), valueSize);
                offset += valueSize;

                // Add Key-Value Pair To The Store
                _keyValueStore[key] = value;
            }

            // Load Was Successful
            _storeLoaded = true;
            return 0;
        }
        else
        {
            // Flash Doesn't Contain A Valid FlashKV Store
            _storeLoaded = true;
            return 1;
        }
        return -1;
    }

    bool FlashKV::saveStore()
    {
        // If The Store Hasn't Been Loaded, Return False.
        if (!_storeLoaded)
            return false;

        // Erase Entire Flash Memory.
        if (!_flashEraseFunc(_flashAddress, _flashSize))
            return false;

        // Create A Buffer To Hold The Entire Store
        std::vector<uint8_t> buffer;

        // Add Signature To The Buffer
        buffer.insert(buffer.end(), FLASHKV_SIGNATURE.begin(), FLASHKV_SIGNATURE.end());

        // Add Each Key-Value Pair To The Buffer
        for (const auto &kv : _keyValueStore)
        {
            // Serialize The Key
            uint16_t keySize = kv.first.size();
            buffer.insert(buffer.end(), reinterpret_cast<uint8_t *>(&keySize), reinterpret_cast<uint8_t *>(&keySize) + sizeof(uint16_t));
            buffer.insert(buffer.end(), kv.first.begin(), kv.first.end());

            // Serialize The Value
            uint16_t valueSize = kv.second.size();
            buffer.insert(buffer.end(), reinterpret_cast<uint8_t *>(&valueSize), reinterpret_cast<uint8_t *>(&valueSize) + sizeof(uint16_t));
            buffer.insert(buffer.end(), kv.second.begin(), kv.second.end());
        }

        // Pad The Buffer With Zeros To Make It A Multiple Of The Page Size
        while (buffer.size() < _flashSize)
            buffer.push_back(0);

        // Write The Buffer To Flash
        if (!_flashWriteFunc(_flashAddress, buffer.data(), buffer.size()))
            return false;

        return true;
    }

    bool FlashKV::writeKey(std::string key, std::vector<uint8_t> value)
    {
        // If The Store Hasn't Been Loaded, Return False.
        if (!_storeLoaded)
            return false;

        // Write The Key-Value Pair To The Store.
        _keyValueStore[key] = value;
        return true;
    }

    std::optional<std::vector<uint8_t>> FlashKV::readKey(std::string key)
    {
        // If The Store Hasn't Been Loaded, Return Null.
        if (!_storeLoaded)
            return std::nullopt;

        // Find The Key If It Exists & Return It.
        auto it = _keyValueStore.find(key);
        if (it != _keyValueStore.end())
            return it->second;

        // Return Null If The Key Doesn't Exist.
        return std::nullopt;
    }

    bool FlashKV::eraseKey(std::string key)
    {
        if (!_storeLoaded)
            return false;

        return _keyValueStore.erase(key) > 0;
    }

} // namespace FlashKV
