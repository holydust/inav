/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define TARGET_BOARD_IDENTIFIER "C7V2"

#define USBD_PRODUCT_STRING  "CORVON743V2"

#define USE_TARGET_CONFIG

// *************** LED **********************
// V2 silkscreen: PE4 = blue, PE5 = red, PE6 = green (active LOW)
#define LED0                    PE4
#define LED1                    PE5
#define LED2                    PE6

// *************** Beeper **********************
#define BEEPER                  PA7
#define BEEPER_INVERTED
#define BEEPER_PWM_FREQUENCY    2500

// *************** SPI: Gyro & ACC **********************
#define USE_SPI
#define USE_SPI_DEVICE_3

#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PB5

// STM32H743: PB5 as SPI3_MOSI is AF7, while SCK/MISO on PB3/PB4 are AF6.
// INAV's global SPI3 table hardcodes AF6 for all three (only correct when MOSI
// sits on PC12). Without this per-target override the MOSI line is misconfigured,
// so the FC cannot clock commands out to the SPI3 IMUs and they all fail to
// detect (SCK/MISO still work, which is why baro/mag on I2C are unaffected).
// AF values verified against the STM32H743 datasheet.
#define SPI3_SCK_AF             GPIO_AF6_SPI3
#define SPI3_MISO_AF            GPIO_AF6_SPI3
#define SPI3_MOSI_AF            GPIO_AF7_SPI3

// Primary IMU: ICM-42688P on SPI3 (uses the ICM-42605-family driver).
// Both IMUs are registered by common_hardware.c (EXTI=NONE, polled). The DRDY
// lines are wired on the PCB (ICM PD10, BMI088 gyro PD11, BMI088 acc PD6) but
// INAV reads these sensors by polling, so no EXTI macros are defined.
#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN      CW90_DEG
#define ICM42605_SPI_BUS        BUS_SPI3
#define ICM42605_CS_PIN         PA15

// Secondary IMU: BMI088 on SPI3 (split chip-select).
// DISABLED so the primary ICM-42688P is used. INAV has no gyro-model selector
// (only gyro_to_use, a same-model instance index), and AUTO-detect walks the
// gyroSensor_e enum in order where BMI088 sits *before* ICM42605 and would win.
// The BMI088 is still on the board - re-enable USE_IMU_BMI088 to fall back to it.
// #define USE_IMU_BMI088
#define IMU_BMI088_ALIGN        CW90_DEG
#define BMI088_SPI_BUS          BUS_SPI3
#define BMI088_GYRO_CS_PIN      PD5
#define BMI088_ACC_CS_PIN       PD4

// *************** I2C: BARO & MAG ****************************
// I2C1 (PB6/PB7) = external connector (GPS / external compass / airspeed)
// I2C2 (PB10/PB11) = onboard sensors (BMP581 baro + IST8310 mag)
#define USE_I2C
#define USE_I2C_DEVICE_1
#define USE_I2C_DEVICE_2
#define I2C1_SCL                PB6
#define I2C1_SDA                PB7
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11
#define DEFAULT_I2C_BUS         BUS_I2C2

// Barometer: on-board chip is BMP581 on I2C2 @ 0x46 (SDO=GND). Driven by the
// BMP5xx family driver, which also covers BMP580 hardware in case of a SKU
// swap. Other Bosch baros are kept enabled as a safety net for variant boards.
#define USE_BARO
#define USE_BARO_BMP5XX
#define USE_BARO_DPS310
#define USE_BARO_SPL06
#define BARO_I2C_BUS            BUS_I2C2
#define BMP5XX_I2C_ADDR         (0x46)

#define USE_MAG
#ifdef CORVON743V2_EXTMAG
// External compass on the GPS connector
#define MAG_I2C_BUS             BUS_I2C1
#else
// On-board IST8310 on internal bus
#define MAG_I2C_BUS             BUS_I2C2
#endif
#define USE_MAG_ALL

// *************** UART *****************************
#define USE_VCP

#define USE_UART1
#define UART1_RX_PIN            PA10
#define UART1_TX_PIN            PA9

#define USE_UART2
#define UART2_RX_PIN            PA3
#define UART2_TX_PIN            PA2

#define USE_UART3
#define UART3_RX_PIN            PD9
#define UART3_TX_PIN            PD8

#define USE_UART4
#define UART4_RX_PIN            PA1
#define UART4_TX_PIN            PA0

#define USE_UART5
#define UART5_RX_PIN            PB12
#define UART5_TX_PIN            PB13

#define USE_UART6
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6

#define USE_UART7
#define UART7_RX_PIN            PE7
#define UART7_TX_PIN            PE8

#define USE_UART8
#define UART8_RX_PIN            PE0
#define UART8_TX_PIN            PE1

#define SERIAL_PORT_COUNT       9      // VCP, UART1..UART8

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART6

// *************** ENABLE OPTICAL FLOW & RANGEFINDER *****************************
#define USE_RANGEFINDER
#define USE_RANGEFINDER_MSP
#define USE_OPFLOW
#define USE_OPFLOW_MSP

// *************** SDIO SD BLACKBOX *******************
#define USE_SDCARD
#define USE_SDCARD_SDIO
#define SDCARD_SDIO_DEVICE      SDIODEV_1
#define SDCARD_SDIO_4BIT
#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// *************** ADC *****************************
// PC0 = BATT1 voltage (21:1 divider), PC1 = BATT1 current
// PC4/PC5 = BATT2 voltage/current pads, PA4 = airspeed, PA6 = RSSI
// (only BATT1 wired into the inav defaults; secondary battery + airspeed +
//  analog RSSI can be added later when inav grows the matching plumbing.)
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC_CHANNEL_1_PIN           PC0
#define ADC_CHANNEL_2_PIN           PC1
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define VBAT_SCALE_DEFAULT          2100
#define CURRENT_METER_SCALE         402

// *************** LED Strip **********************
#define USE_LED_STRIP
#define WS2811_PIN              PA5

#define DEFAULT_FEATURES        (FEATURE_VBAT | FEATURE_CURRENT_METER | FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_LED_STRIP)

#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff
#define TARGET_IO_PORTE 0xffff

#define MAX_PWM_OUTPUT_PORTS    12     // 10 motors + 2 servos
