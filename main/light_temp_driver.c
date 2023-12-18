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

#include "esp_log.h"
#include "driver/ledc.h"
#include "light_temp_driver.h"

#define LEDC_TIMER_TEMP         LEDC_TIMER_1
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO_WW       (2)  // Define the output GPIO for warm white
#define LEDC_OUTPUT_IO_CW       (3)  // Define the output GPIO form cold white
#define LEDC_CHANNEL_WW         LEDC_CHANNEL_3
#define LEDC_CHANNEL_CW         LEDC_CHANNEL_4
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY_MAX		    (8192)
#define LEDC_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz


static const char *TAG = "ESP_ZB_COLOR_DIMMABLE_LIGHT";
static uint8_t s_level = 255;
static float s_temp = 0.5;

void set_temps() {
	float ratio = (float)s_level / 255;
	float dc_cw = s_temp * 2.f;
	float dc_ww = 2.f - dc_cw;
	if (dc_cw > 1.f) dc_cw = 1.f;
	if (dc_ww > 1.f) dc_ww = 1.f;
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_CW, (int)(6554.f * ratio * dc_cw)));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_CW));
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_WW, (int)(6554.f * ratio * dc_ww)));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_WW));
	ESP_LOGI(TAG, "Light temp change to ratio=%f c=%f w=%f l=%d", ratio, dc_cw, dc_ww, s_level);
}

void light_temp_driver_set_color_temperature(uint16_t color_temperature)
{
	float temp = 1000000 / color_temperature;
	s_temp = (float)(temp - 2500) / (6500 - 2500);

	set_temps();
}

void light_temp_driver_set_power(bool power)
{
	if (power) {
		set_temps();	
		return;
	}
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_WW, 0));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_WW));
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_CW, 0));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_CW));
}

void light_temp_driver_set_level(uint8_t level)
{
	s_level = level;
	ESP_LOGI(TAG, "Light level changes to %d", s_level);

	set_temps();
}

void light_temp_driver_init(bool power)
{
	// Prepare and then apply the LEDC PWM timer configuration
	ledc_timer_config_t ledc_timer = {
		.speed_mode       = LEDC_MODE,
		.duty_resolution  = LEDC_DUTY_RES,
		.timer_num        = LEDC_TIMER_TEMP,
		.freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4 kHz
		.clk_cfg          = LEDC_AUTO_CLK
	};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	ledc_channel_config_t ledc_channel_cw = {
		.speed_mode     = LEDC_MODE,
		.channel        = LEDC_CHANNEL_CW,
		.timer_sel      = LEDC_TIMER_TEMP,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = LEDC_OUTPUT_IO_CW,
		.duty           = 0,
		.hpoint         = 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_cw));

	ledc_channel_config_t ledc_channel_ww = {
		.speed_mode     = LEDC_MODE,
		.channel        = LEDC_CHANNEL_WW,
		.timer_sel      = LEDC_TIMER_TEMP,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = LEDC_OUTPUT_IO_WW,
		.duty           = 0,
		.hpoint         = 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_ww));

	light_temp_driver_set_power(power);
}
