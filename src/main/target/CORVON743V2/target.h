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
// PE4 blue, PE5 red, PE6 green
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

// PB5 is AF7 for SPI3_MOSI (the default table assumes AF6)
#define SPI3_SCK_AF             GPIO_AF6_SPI3
#define SPI3_MISO_AF            GPIO_AF6_SPI3
#define SPI3_MOSI_AF            GPIO_AF7_SPI3

// ICM-42688P
#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN      CW90_DEG
#define ICM42605_SPI_BUS        BUS_SPI3
#define ICM42605_CS_PIN         PA15

// BMI088 is fitted but disabled, otherwise it is detected before the ICM-42688P
// #define USE_IMU_BMI088
#define IMU_BMI088_ALIGN        CW90_DEG
#define BMI088_SPI_BUS          BUS_SPI3
#define BMI088_GYRO_CS_PIN      PD5
#define BMI088_ACC_CS_PIN       PD4

// *************** I2C **********************
// I2C1: GPS connector, I2C2: onboard BMP581 + IST8310
#define USE_I2C
#define USE_I2C_DEVICE_1
#define USE_I2C_DEVICE_2
#define I2C1_SCL                PB6
#define I2C1_SDA                PB7
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11
#define DEFAULT_I2C_BUS         BUS_I2C1

#define USE_BARO
#define USE_BARO_BMP5XX
#define USE_BARO_DPS310
#define USE_BARO_SPL06
#define BARO_I2C_BUS            BUS_I2C2
#define BMP5XX_I2C_ADDR         (0x46)

#define USE_MAG
#ifdef CORVON743V2_EXTMAG
#define MAG_I2C_BUS             BUS_I2C1
#else
#define MAG_I2C_BUS             BUS_I2C2
#endif
#define USE_MAG_ALL

#define UG2864_I2C_BUS          BUS_I2C1

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

// *************** OPTICAL FLOW & RANGEFINDER **********************
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
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC_CHANNEL_1_PIN           PC0
#define ADC_CHANNEL_2_PIN           PC1
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define VBAT_SCALE_DEFAULT          2112
#define CURRENT_METER_SCALE         249     // 40.2 A/V

// *************** LED Strip **********************
#define USE_LED_STRIP
#define WS2811_PIN              PA5

#define DEFAULT_FEATURES        (FEATURE_VBAT | FEATURE_CURRENT_METER | FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_LED_STRIP | FEATURE_GPS | FEATURE_BLACKBOX)

#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff
#define TARGET_IO_PORTE 0xffff

#define MAX_PWM_OUTPUT_PORTS    12     // 10 motors + 2 servos
