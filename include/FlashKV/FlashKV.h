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
    using FlashWriteFunc = std::function<bool(uint32_t flashAddress, const uint8_t *data, size_t count)>;
    using FlashReadFunc = std::function<bool(uint32_t flashAddress, uint8_t *data, size_t count)>;
    using FlashEraseFunc = std::function<bool(uint32_t flashAddress, size_t count)>;

    // Key-Value Store Type
    using KeyValueStore = std::unordered_map<std::string, std::vector<uint8_t>>;

    /**
     * @class FlashKV
     * @brief A class that provides a key-value store using Flash memory.
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
         * @param flashWriteFunc Function for writing data to Flash memory.
         * @param flashReadFunc Function for reading data from Flash memory.
         * @param flashEraseFunc Function for erasing data from Flash memory.
         * @param flashPageSize Minimum size required for writing data to Flash memory. Should be equal to Flash page size.
         * @param flashSectorSize Minimum size required for erasing data from Flash memory. Should be equal to Flash sector size.
         * @param flashAddress Starting address in Flash memory where key-value pairs will be stored.
         * @param flashSize Size of Flash memory region reserved for key-value store. Should be large enough to accommodate required number of key-value pairs.
         *
         * @note The provided Flash write and read functions should return true if the operation was successful, false otherwise.
         *       The minWriteSize and minEraseSize parameters should be chosen based on the Flash memory characteristics to ensure proper data storage and retrieval.
         */
        FlashKV(FlashWriteFunc flashWriteFunc,
                FlashReadFunc flashReadFunc,
                FlashEraseFunc flashEraseFunc,
                size_t flashPageSize,
                size_t flashSectorSize,
                size_t flashAddress,
                size_t flashSize);

        /**
         * @brief Destroys the FlashKV object.
         *
         * Saves the store, releases any resources and performs necessary cleanup.
         */
        ~FlashKV();

        /**
         * @brief Loads the key-value store from Flash memory.
         *
         * @return 0 If A Store was successfully loaded from Flash memory.
         * @return 1 If No Store was found in Flash memory.
         * @return -1 If An error occurred while loading the store.
         */
        int loadStore();

        /**
         * @brief Saves the key-value store to Flash memory.
         *
         * @return True if the save operation was successful, false otherwise.
         */
        bool saveStore();

        /**
         * @brief Writes a key-value pair to the store.
         *
         * @param key The key to be written. Should be a uint8_t value.
         * @param value The value to be associated with the key. Should be a uint8_t value.
         *
         * @return True if the write operation was successful, false otherwise.
         */
        bool writeKey(std::string key, std::vector<uint8_t> value);

        /**
         * @brief Reads a value associated with a key from the store.
         *
         * @param key The key to be read. Should be a uint8_t value.
         *
         * @return The value associated with the key if the read operation was successful, std::nullopt otherwise.
         */
        std::optional<std::vector<uint8_t>> readKey(std::string key);

        /**
         * @brief Erases a key-value pair from the store.
         *
         * @param key The key to be erased. Should be a uint8_t value.
         *
         * @return True if the erase operation was successful, false otherwise.
         */
        bool eraseKey(std::string key);

        /**
         * @brief Gets all keys in the store.
         *
         * @return An std::vector of all keys in the store.
         */
        std::vector<std::string> getAllKeys();

    private:
        FlashWriteFunc _flashWriteFunc; // Function for writing to Flash memory.
        FlashReadFunc _flashReadFunc;   // Function for reading from Flash memory.
        FlashEraseFunc _flashEraseFunc; // Function for erasing from Flash memory.

        KeyValueStore _keyValueStore; // In-memory key-value store.
        size_t _flashPageSize;        // Size of a page in Flash memory.
        size_t _flashSectorSize;      // Size of a sector in Flash memory.
        size_t _flashAddress;         // Address of the Flash memory to use for the key-value store.
        size_t _flashSize;            // Size of the Flash memory to use for the key-value store.
        bool _storeLoaded;            // Whether the key-value store has been loaded from Flash memory.
    };

} // namespace FlashKV
