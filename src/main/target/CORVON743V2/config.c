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
#include "sensors/compass.h"

// portConfigs index: 0 = VCP, 1..8 = UART1..UART8
void targetConfiguration(void)
{
    // UART1: MSP (TELEM1)
    serialConfigMutable()->portConfigs[1].functionMask = FUNCTION_MSP;
    serialConfigMutable()->portConfigs[1].msp_baudrateIndex = BAUD_57600;

    // UART2: MSP DisplayPort (DJI connector)
    serialConfigMutable()->portConfigs[2].functionMask = FUNCTION_MSP_OSD;

    // UART3: GPS
    serialConfigMutable()->portConfigs[3].functionMask = FUNCTION_GPS;

    // UART7: ESC telemetry
    serialConfigMutable()->portConfigs[7].functionMask = FUNCTION_ESCSERIAL;

    // Passive buzzer
    beeperConfigMutable()->pwmMode = true;

#ifndef CORVON743V2_EXTMAG
    // Onboard IST8310
    compassConfigMutable()->mag_align = CW90_DEG;
#endif
}
