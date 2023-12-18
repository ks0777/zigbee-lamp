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
#include "light_color_driver.h"

#define LEDC_TIMER_COLOR        LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO_R        (22) // Define the output GPIO for red
#define LEDC_OUTPUT_IO_G        (21) // Define the output GPIO for green
#define LEDC_OUTPUT_IO_B        (20) // Define the output GPIO for blue
#define LEDC_CHANNEL_R          LEDC_CHANNEL_0
#define LEDC_CHANNEL_G          LEDC_CHANNEL_1
#define LEDC_CHANNEL_B          LEDC_CHANNEL_2
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY_MAX		(8192)
#define LEDC_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz


static const char *TAG = "ESP_ZB_COLOR_DIMMABLE_LIGHT";
static uint8_t s_red = 255, s_green = 255, s_blue = 255, s_level = 255;

void set_colors() {
	float ratio = (float)s_level / 255;
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_R, (int)(8192.f * ratio * s_red / 255)));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_R));
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_G, (int)(8192.f * ratio * s_green / 255)));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_G));
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B, (int)(8192.f * ratio * s_blue / 255)));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B));
	ESP_LOGI(TAG, "Light colors change to r=%d g=%d b=%d l=%d", s_red, s_green, s_blue, s_level);
}

void light_color_driver_set_color_xy(uint16_t color_current_x, uint16_t color_current_y)
{
	float red_f = 0, green_f = 0, blue_f = 0, color_x, color_y;
	color_x = (float)color_current_x / 65535;
	color_y = (float)color_current_y / 65535;
	/* assume color_Y is full light level value 1  (0-1.0) */
	float color_X = color_x / color_y;
	float color_Z = (1 - color_x - color_y) / color_y;
	/* change from xy to linear RGB NOT sRGB */
	XYZ_to_RGB(color_X, 1, color_Z, red_f, green_f, blue_f);
	s_red = (uint8_t)(red_f * (float)255);
	s_green = (uint8_t)(green_f * (float)255);
	s_blue = (uint8_t)(blue_f * (float)255);

	set_colors();
}

void light_color_driver_set_power(bool power)
{
	if (power) {
		set_colors();	
		return;
	}
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_R, 0));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_R));
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_G, 0));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_G));
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_B, 0));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_B));
}

void light_color_driver_set_level(uint8_t level)
{
	s_level = level;
	ESP_LOGI(TAG, "Light level changes to %d", s_level);

	set_colors();
}

void light_color_driver_init(bool power)
{
	// Prepare and then apply the LEDC PWM timer configuration
	ledc_timer_config_t ledc_timer = {
		.speed_mode       = LEDC_MODE,
		.duty_resolution  = LEDC_DUTY_RES,
		.timer_num        = LEDC_TIMER_COLOR,
		.freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4 kHz
		.clk_cfg          = LEDC_AUTO_CLK
	};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	ledc_channel_config_t ledc_channel_r = {
		.speed_mode     = LEDC_MODE,
		.channel        = LEDC_CHANNEL_R,
		.timer_sel      = LEDC_TIMER_COLOR,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = LEDC_OUTPUT_IO_R,
		.duty           = 0,
		.hpoint         = 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_r));

	ledc_channel_config_t ledc_channel_g = {
		.speed_mode     = LEDC_MODE,
		.channel        = LEDC_CHANNEL_G,
		.timer_sel      = LEDC_TIMER_COLOR,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = LEDC_OUTPUT_IO_G,
		.duty           = 0,
		.hpoint         = 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_g));

	ledc_channel_config_t ledc_channel_b = {
		.speed_mode     = LEDC_MODE,
		.channel        = LEDC_CHANNEL_B,
		.timer_sel      = LEDC_TIMER_COLOR,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = LEDC_OUTPUT_IO_B,
		.duty           = 0,
		.hpoint         = 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_b));

	light_color_driver_set_power(power);
}
