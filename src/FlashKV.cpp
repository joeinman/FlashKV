/*
 * FlashKV - A Lightweight, Hardware-Agnostic Key-Value Map for Flash Memory
 * 
 * Copyright (C) 2023 Joe Inman
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the MIT License as published by
 * the Open Source Initiative.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * MIT License for more details.
 * 
 * You should have received a copy of the MIT License along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 * 
 * Author: Joe Inman
 * Email: joe.inman8@gmail.com
 * Version: 1.0
 * 
 * Description:
 * FlashKV is designed to provide a straightforward and customizable
 * interface for key-value storage in flash memory. This library
 * allows for easy reading, writing, and erasing of key-value pairs.
 * 
 */

#include "../include/FlashKV/FlashKV.h"

namespace FlashKV
{
    FlashKV::FlashKV(FlashWriteFunction flashWriteFunc,
                     FlashReadFunction flashReadFunc,
                     FlashEraseFunction flashEraseFunc,
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
    }

    int FlashKV::loadMap()
    {
        // Attempt To Read The Signature From Flash
        uint8_t signature[FLASHKV_SIGNATURE_SIZE];
        if (!_flashReadFunc(_flashAddress, signature, FLASHKV_SIGNATURE_SIZE))
            return -1;

        // Check If The Signature Is Valid
        if (std::memcmp(signature, FLASHKV_SIGNATURE, FLASHKV_SIGNATURE_SIZE) == 0)
        {
            // Flash Contains A Valid FlashKV Map.
            size_t offset = FLASHKV_SIGNATURE_SIZE;
            while (1)
            {
                // Read The Size Of The Key
                uint16_t keySize;
                _flashReadFunc(_flashAddress + offset, reinterpret_cast<uint8_t *>(&keySize), sizeof(uint16_t));

                // If The Key Size Is 0, We Have Reached The End Of The Map
                if (keySize == 0)
                    break;
                offset += sizeof(uint16_t);

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

                // Add Key-Value Pair To The Map
                _keyValueMap[key] = value;
            }
            _serialisedSize = offset;

            // Load Was Successful
            _mapLoaded = true;
            return 0;
        }
        else
        {
            // Flash Doesn't Contain A Valid FlashKV Map
            _mapLoaded = true;
            return 1;
        }
        return -1;
    }

    bool FlashKV::saveMap()
    {
        // If The Map Hasn't Been Loaded, Return False.
        if (!_mapLoaded)
            return false;

        // Erase Entire Flash Memory.
        if (!_flashEraseFunc(_flashAddress, _flashSize))
            return false;

        // Create A Buffer To Hold The Entire Map
        std::vector<uint8_t> buffer;

        // Add Signature To The Buffer
        buffer.insert(buffer.end(), std::begin(FLASHKV_SIGNATURE), std::end(FLASHKV_SIGNATURE));

        // Add Each Key-Value Pair To The Buffer
        for (const auto &kv : _keyValueMap)
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
        // If The Map Hasn't Been Loaded, Return False.
        if (!_mapLoaded)
            return false;

        // Write The Key-Value Pair To The Map If There Is Enough Space
        if (_serialisedSize + sizeof(uint16_t) + key.size() + sizeof(uint16_t) + value.size() <= _flashSize)
        {
            _keyValueMap[key] = value;
            _serialisedSize += sizeof(uint16_t) + key.size() + sizeof(uint16_t) + value.size();
            return true;
        }

        return false;
    }

    std::optional<std::vector<uint8_t>> FlashKV::readKey(std::string key)
    {
        // If The Map Hasn't Been Loaded, Return Null.
        if (!_mapLoaded)
            return std::nullopt;

        // Find The Key If It Exists & Return It.
        auto it = _keyValueMap.find(key);
        if (it != _keyValueMap.end())
            return it->second;

        // Return Null If The Key Doesn't Exist.
        return std::nullopt;
    }

    bool FlashKV::eraseKey(std::string key)
    {
        // If The Map Hasn't Been Loaded, Return False
        if (!_mapLoaded)
            return false;

        // Erase The Key If It Exists & Modify The Serialised Size
        auto it = _keyValueMap.find(key);
        if (it != _keyValueMap.end())
        {
            _serialisedSize -= sizeof(uint16_t) + key.size() + sizeof(uint16_t) + it->second.size();
            _keyValueMap.erase(it);
            return true;
        }

        return false;
    }

    std::vector<std::string> FlashKV::getAllKeys()
    {
        // Return A Vector Of All The Keys In The Map.
        std::vector<std::string> keys;
        for (const auto &kv : _keyValueMap)
            keys.push_back(kv.first);
        return keys;
    }

} // namespace FlashKV
