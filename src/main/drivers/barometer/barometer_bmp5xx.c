/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute this software and/or
 * modify this software under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Driver for the Bosch BMP580 / BMP581 barometer family. Both chips share an
 * identical register map and need no calibration data — hardware applies
 * temperature and pressure compensation internally — so a single driver
 * covers the whole family. The naming follows Bosch's own BMP5_SensorAPI and
 * the Betaflight barometer_bmp5xx driver this port was derived from.
 *
 * References:
 *   - BMP581 datasheet (Bosch Sensortec, document BST-BMP581-DS004)
 *   - BMP5_SensorAPI:                https://github.com/boschsensortec/BMP5_SensorAPI
 *   - Betaflight barometer_bmp5xx:   PR betaflight/betaflight#14925 (merged 2026-02-27)
 *   - INAV barometer_bmp388 driver (structural template)
 */

#include <stdbool.h>
#include <stdint.h>

#include <platform.h>
#include "build/build_config.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/bus.h"
#include "drivers/barometer/barometer.h"
#include "drivers/barometer/barometer_bmp5xx.h"

#if defined(USE_BARO) && defined(USE_BARO_BMP5XX)

// I2C addresses are board-specific (SDO pin selects PRIM/SEC). Default to
// SECONDARY (SDO=GND) since that matches the CORVON743V2 wiring and the
// Bosch / Ardupilot datasheet default; the board can override.
#if !defined(BMP5XX_I2C_ADDR)
#define BMP5XX_I2C_ADDR             (0x46)
#endif

// Chip IDs. Bosch only labels them PRIM / SEC and does not bind either ID
// to a specific SKU — silicon revision matters more than 580 vs 581. In
// reported cases 0x50 has been seen on BMP580 hardware and 0x51 on BMP581,
// but the only safe rule is to accept both.
#define BMP5XX_CHIP_ID_PRIM         (0x50)
#define BMP5XX_CHIP_ID_SEC          (0x51)

// Register map (datasheet section 7)
#define BMP5XX_REG_CHIP_ID          (0x01)
#define BMP5XX_REG_INT_CONFIG       (0x14)
#define BMP5XX_REG_INT_SOURCE       (0x15)
#define BMP5XX_REG_TEMP_DATA_XLSB   (0x1D)    // first of 6 burst-readable data bytes
#define BMP5XX_REG_STATUS           (0x28)
#define BMP5XX_REG_DSP_CONFIG       (0x30)
#define BMP5XX_REG_DSP_IIR          (0x31)
#define BMP5XX_REG_OSR_CONFIG       (0x36)
#define BMP5XX_REG_ODR_CONFIG       (0x37)
#define BMP5XX_REG_CMD              (0x7E)

#define BMP5XX_CMD_SOFT_RESET       (0xB6)

// ODR_CONFIG bitfields: power-mode [1:0], ODR [6:2]
#define BMP5XX_MODE_STANDBY         (0x00)
#define BMP5XX_MODE_NORMAL          (0x01)
#define BMP5XX_ODR_50_HZ            (0x0F << 2)

// OSR_CONFIG bitfields: pressure OSR [2:0], temperature OSR [5:3], press_en bit 6
#define BMP5XX_OSR_PRESS_128X       (0x07)
#define BMP5XX_OSR_TEMP_4X          (0x02 << 3)
#define BMP5XX_OSR_PRESS_EN         (0x01 << 6)

// DSP_CONFIG bitfields: comp enables (press+temp = 0x03), shadow-IIR routing
#define BMP5XX_DSP_COMP_PRESS_TEMP  (0x03)
#define BMP5XX_DSP_SHDW_SEL_IIR     (0x01 << 3)

// DSP_IIR coefficient (separate 3-bit fields for press [2:0] and temp [5:3])
#define BMP5XX_IIR_COEF_15          (0x04)
#define BMP5XX_IIR_PRESS(x)         ((x) & 0x07)
#define BMP5XX_IIR_TEMP(x)          (((x) & 0x07) << 3)

#define BMP5XX_DATA_FRAME_SIZE      (6)

// All sensor state in the file scope mirrors the inav BMP388 driver shape.
static uint8_t sensor_data[BMP5XX_DATA_FRAME_SIZE];
static int32_t bmp5xx_temp_raw;     // signed 24-bit, sign-extended into int32
static uint32_t bmp5xx_press_raw;   // unsigned 24-bit

static bool bmp5xxStartUT(baroDev_t *baro)
{
    UNUSED(baro);
    // BMP5xx runs in NORMAL (continuous) mode - nothing to kick off per sample.
    return true;
}

static bool bmp5xxGetUT(baroDev_t *baro)
{
    UNUSED(baro);
    // Temperature is read in the same burst as pressure (see bmp5xxGetUP).
    return true;
}

static bool bmp5xxStartUP(baroDev_t *baro)
{
    UNUSED(baro);
    // Continuous mode - dummy, real read happens in bmp5xxGetUP after up_delay.
    return true;
}

static bool bmp5xxGetUP(baroDev_t *baro)
{
    if (!busReadBuf(baro->busDev, BMP5XX_REG_TEMP_DATA_XLSB, sensor_data, BMP5XX_DATA_FRAME_SIZE)) {
        return false;
    }

    // Bytes 0..2 = temperature, 24-bit two's complement, LSB-first.
    uint32_t tu = (uint32_t)sensor_data[0]
                | ((uint32_t)sensor_data[1] << 8)
                | ((uint32_t)sensor_data[2] << 16);
    if (tu & 0x800000U) {
        tu |= 0xFF000000U;          // sign-extend negative temperatures
    }
    bmp5xx_temp_raw = (int32_t)tu;

    // Bytes 3..5 = pressure, 24-bit unsigned, LSB-first.
    bmp5xx_press_raw = (uint32_t)sensor_data[3]
                     | ((uint32_t)sensor_data[4] << 8)
                     | ((uint32_t)sensor_data[5] << 16);

    return true;
}

static bool bmp5xxCalculate(baroDev_t *baro, int32_t *pressure, int32_t *temperature)
{
    UNUSED(baro);

    // Hardware already applied IIR + compensation. Raw scaling per datasheet:
    //   temperature = raw / 65536  [degC]      -> *100 / 65536 for centi-degC
    //   pressure    = raw / 64     [Pa]
    if (temperature) {
        *temperature = (int32_t)(((int64_t)bmp5xx_temp_raw * 100) / 65536);
    }
    if (pressure) {
        *pressure = (int32_t)(bmp5xx_press_raw / 64);
    }
    return true;
}

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(busDevice_t *busDev)
{
    for (int retry = 0; retry < DETECTION_MAX_RETRY_COUNT; retry++) {
        delay(20);

        uint8_t chipId = 0;
        bool ack = busReadBuf(busDev, BMP5XX_REG_CHIP_ID, &chipId, 1);
        if (ack && (chipId == BMP5XX_CHIP_ID_PRIM || chipId == BMP5XX_CHIP_ID_SEC)) {
            return true;
        }
    }
    return false;
}

bool bmp5xxDetect(baroDev_t *baro)
{
    baro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_BMP5XX, 0, OWNER_BARO);
    if (baro->busDev == NULL) {
        return false;
    }

    busSetSpeed(baro->busDev, BUS_SPEED_STANDARD);

    if (!deviceDetect(baro->busDev)) {
        busDeviceDeInit(baro->busDev);
        return false;
    }

    // Soft reset clears any leftover config from prior boots / warm resets.
    busWrite(baro->busDev, BMP5XX_REG_CMD, BMP5XX_CMD_SOFT_RESET);
    delay(5);

    // Route through IIR shadow registers and enable on-chip compensation for
    // both pressure and temperature.
    busWrite(baro->busDev, BMP5XX_REG_DSP_CONFIG,
             BMP5XX_DSP_COMP_PRESS_TEMP | BMP5XX_DSP_SHDW_SEL_IIR);

    // IIR coefficient 15 on both channels - matches Betaflight default and
    // gives the noise floor quoted on the BMP581 datasheet (~0.06 hPa RMS).
    busWrite(baro->busDev, BMP5XX_REG_DSP_IIR,
             BMP5XX_IIR_PRESS(BMP5XX_IIR_COEF_15) | BMP5XX_IIR_TEMP(BMP5XX_IIR_COEF_15));

    // 128X pressure / 4X temperature oversampling, pressure measurement enabled.
    busWrite(baro->busDev, BMP5XX_REG_OSR_CONFIG,
             BMP5XX_OSR_PRESS_128X | BMP5XX_OSR_TEMP_4X | BMP5XX_OSR_PRESS_EN);

    // Continuous 50 Hz sampling (NORMAL power mode).
    busWrite(baro->busDev, BMP5XX_REG_ODR_CONFIG,
             BMP5XX_ODR_50_HZ | BMP5XX_MODE_NORMAL);

    baro->ut_delay  = 0;
    baro->start_ut  = bmp5xxStartUT;
    baro->get_ut    = bmp5xxGetUT;

    // ODR 50 Hz = 20 ms per sample; pick exactly that as the inter-read gap.
    baro->up_delay  = 20000;
    baro->start_up  = bmp5xxStartUP;
    baro->get_up    = bmp5xxGetUP;

    baro->calculate = bmp5xxCalculate;

    return true;
}

#endif /* USE_BARO && USE_BARO_BMP5XX */
