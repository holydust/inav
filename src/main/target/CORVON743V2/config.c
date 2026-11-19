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

#include <stdint.h>
#include "platform.h"
#include "config/config_master.h"
#include "config/feature.h"
#include "fc/config.h"
#include "io/serial.h"

// Port-config indices follow the active serialPortIdentifier order for this
// target: 0=VCP, 1=UART1, 2=UART2, 3=UART3, 4=UART4, 5=UART5, 6=UART6,
// 7=UART7, 8=UART8.
void targetConfiguration(void)
{
    // UART1 -> MSP (TELEM1 connector)
    serialConfigMutable()->portConfigs[1].functionMask = FUNCTION_MSP;
    serialConfigMutable()->portConfigs[1].msp_baudrateIndex = BAUD_57600;

    // UART2 -> MSP DisplayPort (DJI O4 air-unit connector)
    serialConfigMutable()->portConfigs[2].functionMask = FUNCTION_MSP_OSD;

    // UART3 -> GPS
    serialConfigMutable()->portConfigs[3].functionMask = FUNCTION_GPS;

    // UART7 -> ESC telemetry (RX pad on board)
    serialConfigMutable()->portConfigs[7].functionMask = FUNCTION_ESCSERIAL;

    // PWM-driven passive piezo buzzer (PA7 / TIM14_CH1)
    beeperConfigMutable()->pwmMode = true;
}
