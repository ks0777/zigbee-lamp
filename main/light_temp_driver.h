/*
 * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: LicenseRef-Included
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Espressif Systems
 *    integrated circuit in a product or a software update for such product,
 *    must reproduce the above copyright notice, this list of conditions and
 *    the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * 4. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* light intensity level */
#define LIGHT_DEFAULT_ON  1
#define LIGHT_DEFAULT_OFF 0

#define COLOR_MIN_TEMP 154
#define COLOR_MAX_TEMP 400


/**
* @brief Set light power (on/off).
*
* @param  power  The light power to be set
*/
void light_temp_driver_set_power(bool power);


/**
* @brief Set light color temperature
*
* @param  temperature  The color temperature to be set
*/
void light_temp_driver_set_color_temperature(uint16_t color_temperature);

/**
* @brief Set light level (0-255).
*
* @param  level  The light level to be set
*/
void light_temp_driver_set_level(uint8_t level);

/**
* @brief color light driver init, be invoked where you want to use color light
*
* @param power power on/off
*/
void light_temp_driver_init(bool power);

#ifdef __cplusplus
} // extern "C"
#endif
