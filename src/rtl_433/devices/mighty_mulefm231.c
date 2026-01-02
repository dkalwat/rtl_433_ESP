/** @file
    Mighty Mule FM231 Driveway Alarm decoder.

    Copyright (C) 2025

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

/**
Mighty Mule FM231 Driveway Alarm from GTO Inc.

FCC ID: I6HGTOFM231
FCC Test Report: https://fccid.io/I6HGTOFM231/Test-Report/Test-Report-1214140.pdf

Product info:
- Wireless driveway alarm system
- 4-position DIP switch for device ID configuration
- Battery operated transmitter

Data format:
- 9 bits total
- Bit 4: Battery status (inverted: 0=OK, 1=Low)
- Bits 5-8: Device ID from DIP switches

Note: The DIP switches are labeled 1-4 from left to right on the device,
but appear in the data stream in reverse order (4-3-2-1).

Data layout:

    ???? B IIII

- ?: 4 bits unknown/preamble
- B: 1 bit battery status (0=OK, 1=Low Battery)
- I: 4 bits device ID (from DIP switches, reversed order)

*/

#include "decoder.h"

#define MIGHTYMULE_FM231_BITLEN 9

static int mightymule_fm231_decode(r_device *decoder, bitbuffer_t *bitbuffer)
{
    // Expect a single row with 9 bits
    if (bitbuffer->num_rows != 1) {
        return DECODE_ABORT_EARLY;
    }

    if (bitbuffer->bits_per_row[0] != MIGHTYMULE_FM231_BITLEN) {
        return DECODE_ABORT_LENGTH;
    }

    uint8_t *b = bitbuffer->bb[0];

    // Extract the 9 bits (stored in first 2 bytes, but only 9 bits used)
    uint16_t data = (b[0] << 1) | (b[1] >> 7);

    // Bit 4 (from left, 0-indexed) = battery status
    // In the data stream: 0 = OK, 1 = Low Battery
    // We invert this to match Accurite convention (battery_ok: 1=OK, 0=Low)
    int battery_raw = (data >> 4) & 0x01;
    int battery_ok = !battery_raw;  // Invert: 0->1 (OK), 1->0 (Low)

    // Bits 5-8 (from left, 0-indexed) = device ID
    int id = data & 0x0F;

    // Log for debugging
    decoder_logf(decoder, 2, __func__, "Data: %03x, Battery raw: %d, Battery OK: %d, ID: %d",
                 data, battery_raw, battery_ok, id);

    // Build output data
    /* clang-format off */
    data_t *output = data_make(
            "model",        "",                 DATA_STRING, "MightyMule-FM231",
            "id",           "ID",               DATA_INT,    id,
            "battery_ok",   "Battery",          DATA_INT,    battery_ok,
            NULL);
    /* clang-format on */

    decoder_output_data(decoder, output);

    return 1;
}

static char const *const output_fields[] = {
        "model",
        "id",
        "battery_ok",
        NULL,
};

r_device const mightymule_fm231 = {
        .name        = "Mighty Mule FM231 Driveway Alarm",
        .modulation  = OOK_PULSE_PWM,
        .short_width = 650,
        .long_width  = 1200,
        .sync_width  = 3800,
        .gap_limit   = 1100,
        .reset_limit = 1100,
        .tolerance   = 200,
        .decode_fn   = &mightymule_fm231_decode,
        .fields      = output_fields,
};