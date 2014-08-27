#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

#define TYPE_1_CMD_1	1
#define TYPE_1_CMD_2	2
#define TYPE_1_CMD_3	3
#define TYPE_1_CMD_4	4

#define CMD_TYPE_1		1

#define OT_CMD_HEADER_SIZE  9

    typedef uint8_t BYTE;
    typedef uint16_t USHORT;

    union u_header
    {
        BYTE buf[OT_CMD_HEADER_SIZE];
        struct {
            BYTE     type_id;    // 1 byte
            BYTE     command_id; // 1 byte
            BYTE     filler[2];  // 2 bytes padding
            uint32_t size;       // 4 bytes
            BYTE     checksum;   // 1 byte
        } fields;                // 9 bytes total
    };

#ifdef __cplusplus
}
#endif

// NOTE: Turns out moneypunct kind of sucks.
// As a result, for internationalization purposes,
// these values have to be set here before compilation.
//
#define OT_THOUSANDS_SEP ","
#define OT_DECIMAL_POINT "."
