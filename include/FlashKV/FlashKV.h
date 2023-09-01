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

#pragma once

#include <unordered_map>
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <cstring>

namespace FlashKV
{
    // FlashKV Signature
    const uint8_t FLASHKV_SIGNATURE[4] = {'F', 'K', 'V', 'S'};
    const uint8_t FLASHKV_SIGNATURE_SIZE = sizeof(FLASHKV_SIGNATURE);

    // Function Types For Flash Access
    using FlashWriteFunction = std::function<bool(uint32_t flashAddress, const uint8_t *data, size_t count)>;
    using FlashReadFunction = std::function<bool(uint32_t flashAddress, uint8_t *data, size_t count)>;
    using FlashEraseFunction = std::function<bool(uint32_t flashAddress, size_t count)>;

    // Key-Value Map Type
    using KeyValue = std::pair<std::string, std::vector<uint8_t>>;
    using KeyValueMap = std::unordered_map<KeyValue::first_type, KeyValue::second_type>;

    /**
     * @class FlashKV
     * @brief A class that provides a key-value map using Flash memory.
     *
     * FlashKV is a class that allows for storing, reading, and erasing key-value pairs in Flash memory.
     * It is designed to work with uint8_t keys and values.
     *
     * @note Ensure that the Flash memory is initialized and accessible before using this class.
     */
    class FlashKV
    {
    public:
        /**
         * @brief Constructs a new FlashKV object.
         *
         * @param flashWriteFunction Function for writing data to Flash memory.
         * @param flashReadFunction Function for reading data from Flash memory.
         * @param flashWriteFunction Function for erasing data from Flash memory.
         * @param flashPageSize Minimum size required for writing data to Flash memory. Should be equal to Flash page size.
         * @param flashSectorSize Minimum size required for erasing data from Flash memory. Should be equal to Flash sector size.
         * @param flashAddress Starting address in Flash memory where key-value pairs will be stored.
         * @param flashSize Size of Flash memory region reserved for key-value map. Should be large enough to accommodate required number of key-value pairs.
         *
         * @note The provided Flash write and read functions should return true if the operation was successful, false otherwise.
         *       The minWriteSize and minEraseSize parameters should be chosen based on the Flash memory characteristics to ensure proper data storage and retrieval.
         */
        FlashKV(FlashWriteFunction flashWriteFunction,
                FlashReadFunction flashReadFunction,
                FlashEraseFunction flashEraseFunction,
                size_t flashPageSize,
                size_t flashSectorSize,
                size_t flashAddress,
                size_t flashSize);

        /**
         * @brief Destroys the FlashKV object.
         *
         * Releases any resources and performs necessary cleanup.
         */
        ~FlashKV();

        /**
         * @brief Loads the key-value map from Flash memory.
         *
         * @return 0 If A Map was successfully loaded from Flash memory.
         * @return 1 If No Map was found in Flash memory.
         * @return -1 If An error occurred while loading the map.
         */
        int loadMap();

        /**
         * @brief Saves the key-value map to Flash memory.
         *
         * @return True if the save operation was successful, false otherwise.
         */
        bool saveMap();

        /**
         * @brief Writes a key-value pair to the map.
         *
         * @param key The key to be written. Should be a uint8_t value.
         * @param value The value to be associated with the key. Should be a uint8_t value.
         *
         * @return True if the write operation was successful, false otherwise.
         */
        bool writeKey(std::string key, std::vector<uint8_t> value);

        /**
         * @brief Reads a value associated with a key from the map.
         *
         * @param key The key to be read. Should be a uint8_t value.
         *
         * @return The value associated with the key if the read operation was successful, std::nullopt otherwise.
         */
        std::optional<std::vector<uint8_t>> readKey(std::string key);

        /**
         * @brief Erases a key-value pair from the map.
         *
         * @param key The key to be erased. Should be a uint8_t value.
         *
         * @return True if the erase operation was successful, false otherwise.
         */
        bool eraseKey(std::string key);

        /**
         * @brief Gets all keys in the map.
         *
         * @return An std::vector of all keys in the map.
         */
        std::vector<std::string> getAllKeys();

    private:
        FlashWriteFunction flashWriteFunction; // Function for writing to Flash memory.
        FlashReadFunction flashReadFunction;   // Function for reading from Flash memory.
        FlashEraseFunction flashEraseFunction; // Function for erasing from Flash memory.

        bool verifySignature();                                                                                // Verifies The FlashKV Signature.
        std::vector<uint8_t> serialiseKeyValuePair(const std::string &key, const std::vector<uint8_t> &value); // Serialises A Key-Value Pair.

        KeyValueMap keyValueMap;   // In-memory key-value map.
        size_t flashPageSize;      // Size of a page in Flash memory.
        size_t flashSectorSize;    // Size of a sector in Flash memory.
        size_t flashAddress;       // Address of the Flash memory to use for the key-value map.
        size_t flashSize;          // Size of the Flash memory to use for the key-value map.
        size_t serialisedSize = 0; // Size of the serialised key-value map.
        bool mapLoaded;            // Whether the key-value map has been loaded from Flash memory.
    };

} // namespace FlashKV
