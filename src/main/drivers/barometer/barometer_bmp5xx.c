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
 * Bosch BMP580 / BMP581 driver, based on the Betaflight barometer_bmp5xx
 * driver and the Bosch BMP5_SensorAPI register definitions.
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

// 0x46 with SDO to GND, 0x47 with SDO to VDDIO
#if !defined(BMP5XX_I2C_ADDR)
#define BMP5XX_I2C_ADDR             (0x46)
#endif

// Both IDs are seen on BMP580 and BMP581 parts
#define BMP5XX_CHIP_ID_PRIM         (0x50)
#define BMP5XX_CHIP_ID_SEC          (0x51)

// Registers
#define BMP5XX_REG_CHIP_ID          (0x01)
#define BMP5XX_REG_INT_CONFIG       (0x14)
#define BMP5XX_REG_INT_SOURCE       (0x15)
#define BMP5XX_REG_TEMP_DATA_XLSB   (0x1D)    // first of 6 burst-readable data bytes
#define BMP5XX_REG_INT_STATUS       (0x27)
#define BMP5XX_REG_STATUS           (0x28)
#define BMP5XX_REG_DSP_CONFIG       (0x30)
#define BMP5XX_REG_DSP_IIR          (0x31)
#define BMP5XX_REG_OSR_CONFIG       (0x36)
#define BMP5XX_REG_ODR_CONFIG       (0x37)
#define BMP5XX_REG_OSR_EFF          (0x38)
#define BMP5XX_REG_CMD              (0x7E)

#define BMP5XX_CMD_SOFT_RESET       (0xB6)

// INT_STATUS / STATUS bits
#define BMP5XX_INT_POR_COMPLETE     (0x10)
#define BMP5XX_STATUS_NVM_RDY       (0x02)
#define BMP5XX_STATUS_NVM_ERR       (0x04)

// ODR_CONFIG bitfields: power-mode [1:0], ODR [6:2], deep_dis bit 7
#define BMP5XX_MODE_STANDBY         (0x00)
#define BMP5XX_MODE_NORMAL          (0x01)
#define BMP5XX_ODR_50_HZ            (0x0F << 2)
#define BMP5XX_DEEP_DISABLE         (0x01 << 7)

// OSR_CONFIG bitfields: temperature OSR [2:0], pressure OSR [5:3], press_en bit 6
#define BMP5XX_OSR_1X               (0x00)
#define BMP5XX_OSR_4X               (0x02)
#define BMP5XX_OSR_128X             (0x07)
#define BMP5XX_OSR_TEMP(x)          ((x) & 0x07)
#define BMP5XX_OSR_PRESS(x)         (((x) & 0x07) << 3)
#define BMP5XX_OSR_PRESS_EN         (0x01 << 6)

// OSR_EFF: odr_is_valid bit 7
#define BMP5XX_ODR_IS_VALID         (0x01 << 7)

// DSP_CONFIG: IIR-filtered temperature (bit 3) and pressure (bit 5) to data registers
#define BMP5XX_DSP_SHDW_IIR_TEMP    (0x01 << 3)
#define BMP5XX_DSP_SHDW_IIR_PRESS   (0x01 << 5)

// DSP_IIR coefficient: temperature [2:0], pressure [5:3]
#define BMP5XX_IIR_COEF_15          (0x04)
#define BMP5XX_IIR_TEMP(x)          ((x) & 0x07)
#define BMP5XX_IIR_PRESS(x)         (((x) & 0x07) << 3)

#define BMP5XX_DATA_FRAME_SIZE      (6)

static uint8_t sensor_data[BMP5XX_DATA_FRAME_SIZE];
static int32_t bmp5xx_temp_raw;     // signed 24-bit, sign-extended into int32
static uint32_t bmp5xx_press_raw;   // unsigned 24-bit

static bool bmp5xxStartUT(baroDev_t *baro)
{
    UNUSED(baro);
    return true;
}

static bool bmp5xxGetUT(baroDev_t *baro)
{
    UNUSED(baro);
    // Temperature is read together with pressure in bmp5xxGetUP()
    return true;
}

static bool bmp5xxStartUP(baroDev_t *baro)
{
    UNUSED(baro);
    return true;
}

static bool bmp5xxGetUP(baroDev_t *baro)
{
    if (!busReadBuf(baro->busDev, BMP5XX_REG_TEMP_DATA_XLSB, sensor_data, BMP5XX_DATA_FRAME_SIZE)) {
        return false;
    }

    // Temperature: 24-bit signed, LSB first
    uint32_t tu = (uint32_t)sensor_data[0]
                | ((uint32_t)sensor_data[1] << 8)
                | ((uint32_t)sensor_data[2] << 16);
    if (tu & 0x800000U) {
        tu |= 0xFF000000U;          // sign-extend negative temperatures
    }
    bmp5xx_temp_raw = (int32_t)tu;

    // Pressure: 24-bit unsigned, LSB first
    bmp5xx_press_raw = (uint32_t)sensor_data[3]
                     | ((uint32_t)sensor_data[4] << 8)
                     | ((uint32_t)sensor_data[5] << 16);

    return true;
}

static bool bmp5xxCalculate(baroDev_t *baro, int32_t *pressure, int32_t *temperature)
{
    UNUSED(baro);

    // Compensated output: temperature = raw / 65536 degC, pressure = raw / 64 Pa
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

static bool writeAndVerify(busDevice_t *busDev, uint8_t reg, uint8_t value)
{
    uint8_t readback = 0;
    return busWrite(busDev, reg, value)
        && busRead(busDev, reg, &readback)
        && readback == value;
}

static bool deviceReset(busDevice_t *busDev)
{
    if (!busWrite(busDev, BMP5XX_REG_CMD, BMP5XX_CMD_SOFT_RESET)) {
        return false;
    }
    delay(5);   // datasheet: 2 ms

    uint8_t intStatus = 0;
    uint8_t status = 0;
    if (!busRead(busDev, BMP5XX_REG_INT_STATUS, &intStatus) || !(intStatus & BMP5XX_INT_POR_COMPLETE)) {
        return false;
    }
    if (!busRead(busDev, BMP5XX_REG_STATUS, &status)
        || !(status & BMP5XX_STATUS_NVM_RDY) || (status & BMP5XX_STATUS_NVM_ERR)) {
        return false;
    }
    return true;
}

static bool deviceConfigure(busDevice_t *busDev)
{
    // Standby with deep standby disabled; IIR config is only kept and writable here
    if (!writeAndVerify(busDev, BMP5XX_REG_ODR_CONFIG, BMP5XX_DEEP_DISABLE | BMP5XX_MODE_STANDBY)) {
        return false;
    }
    delay(3);   // datasheet: 2.5 ms

    uint8_t dspConfig = 0;
    if (!busRead(busDev, BMP5XX_REG_DSP_CONFIG, &dspConfig)) {
        return false;
    }
    dspConfig |= BMP5XX_DSP_SHDW_IIR_TEMP | BMP5XX_DSP_SHDW_IIR_PRESS;
    if (!writeAndVerify(busDev, BMP5XX_REG_DSP_CONFIG, dspConfig)) {
        return false;
    }

    if (!writeAndVerify(busDev, BMP5XX_REG_DSP_IIR,
                        BMP5XX_IIR_TEMP(BMP5XX_IIR_COEF_15) | BMP5XX_IIR_PRESS(BMP5XX_IIR_COEF_15))) {
        return false;
    }

    // 50 Hz normal mode with the highest pressure OSR the sensor accepts
    for (int pressOsr = BMP5XX_OSR_128X; pressOsr >= BMP5XX_OSR_1X; pressOsr--) {
        const uint8_t osrConfig = BMP5XX_OSR_TEMP(BMP5XX_OSR_4X) | BMP5XX_OSR_PRESS(pressOsr) | BMP5XX_OSR_PRESS_EN;
        if (!writeAndVerify(busDev, BMP5XX_REG_OSR_CONFIG, osrConfig)
            || !writeAndVerify(busDev, BMP5XX_REG_ODR_CONFIG, BMP5XX_DEEP_DISABLE | BMP5XX_ODR_50_HZ | BMP5XX_MODE_NORMAL)) {
            return false;
        }

        uint8_t osrEff = 0;
        if (!busRead(busDev, BMP5XX_REG_OSR_EFF, &osrEff)) {
            return false;
        }
        if (osrEff & BMP5XX_ODR_IS_VALID) {
            return true;
        }

        // Back to standby; the mode only reads back once the conversion ends
        const uint8_t odrStandby = BMP5XX_DEEP_DISABLE | BMP5XX_ODR_50_HZ | BMP5XX_MODE_STANDBY;
        uint8_t odrReadback = 0;
        if (!busWrite(busDev, BMP5XX_REG_ODR_CONFIG, odrStandby)) {
            return false;
        }
        for (int retry = 0; retry < 10 && odrReadback != odrStandby; retry++) {
            delay(3);
            if (!busRead(busDev, BMP5XX_REG_ODR_CONFIG, &odrReadback)) {
                return false;
            }
        }
        if (odrReadback != odrStandby) {
            return false;
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

    if (!deviceDetect(baro->busDev)
        || !deviceReset(baro->busDev)
        || !deviceConfigure(baro->busDev)) {
        busDeviceDeInit(baro->busDev);
        return false;
    }

    // Two 10 ms steps per read, i.e. 50 Hz
    baro->ut_delay  = 10000;
    baro->start_ut  = bmp5xxStartUT;
    baro->get_ut    = bmp5xxGetUT;

    baro->up_delay  = 10000;
    baro->start_up  = bmp5xxStartUP;
    baro->get_up    = bmp5xxGetUP;

    baro->calculate = bmp5xxCalculate;

    return true;
}

#endif /* USE_BARO && USE_BARO_BMP5XX */
