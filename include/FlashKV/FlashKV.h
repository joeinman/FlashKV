#pragma once

#include <cstdint>
#include <functional>

namespace FlashKV
{
    // Function type for reading from Flash memory.
    // Returns true if the read operation was successful, false otherwise.
    using FlashReadFunc = std::function<bool(uint32_t flash_offs, size_t count)>;

    // Function type for writing to Flash memory.
    // Returns true if the write operation was successful, false otherwise.
    using FlashWriteFunc = std::function<bool(uint32_t flash_offs, size_t count)>;

    // Function type for erasing from Flash memory.
    // Returns true if the erase operation was successful, false otherwise.
    using FlashEraseFunc = std::function<bool(uint32_t flash_offs)>;

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
         * @param pageSize Minimum size required for writing data to Flash memory. Should be equal to Flash page size.
         * @param sectorSize Minimum size required for erasing data from Flash memory. Should be equal to Flash sector size.
         * @param offset Starting offset in Flash memory where key-value pairs will be stored.
         * @param size Size of Flash memory region reserved for key-value store. Should be large enough to accommodate required number of key-value pairs.
         *
         * @note The provided Flash write and read functions should return true if the operation was successful, false otherwise.
         *       The minWriteSize and minEraseSize parameters should be chosen based on the Flash memory characteristics to ensure proper data storage and retrieval.
         */
        FlashKV(FlashWriteFunc flashWriteFunc,
                FlashReadFunc flashReadFunc,
                FlashEraseFunc flashEraseFunc,
                size_t pageSize,
                size_t sectorSize,
                size_t offset,
                size_t size);

        /**
         * @brief Destroys the FlashKV object.
         *
         * Releases any resources and performs necessary cleanup.
         */
        ~FlashKV();

        /**
         * @brief Writes a key-value pair to Flash memory.
         *
         * @param key The key to be written. Should be a uint8_t value.
         * @param value The value to be associated with the key. Should be a uint8_t value.
         *
         * @return True if the write operation was successful, false otherwise.
         */
        bool writeKey(uint8_t key, uint8_t value);

        /**
         * @brief Reads the value associated with a given key from Flash memory.
         *
         * @param key The key for which the value should be retrieved. Should be a uint8_t value.
         * @param[out] value The value associated with the given key. Will be updated if the key is found.
         *
         * @return True if the key was found, false otherwise. The value output parameter will only be valid if the key is found and the function returns true.
         */
        bool readKey(uint8_t key, uint8_t &value);

        /**
         * @brief Erases the key-value pair associated with a given key from Flash memory.
         *
         * @param key The key for which the key-value pair should be erased. Should be a uint8_t value.
         *
         * @return True if the key-value pair was successfully erased, false otherwise.
         */
        bool eraseKey(uint8_t key);

    private:
        FlashWriteFunc _flashWriteFunc; // Function for writing to Flash memory.
        FlashReadFunc _flashReadFunc;   // Function for reading from Flash memory.
        FlashEraseFunc _flashEraseFunc; // Function for erasing from Flash memory.
    };

} // namespace FlashKV
