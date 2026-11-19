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

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/pinio.h"
#include "drivers/sensor.h"

// IMU bus devices (ICM-42688P on DEVHW_ICM42605, plus the BMI088 gyro/acc
// split-CS pair) are registered automatically by common_hardware.c from the
// ICM42605_*/BMI088_* macros in target.h. Doing it here as well would create
// duplicate busdev_*_registry symbols (multiple-definition link error under
// the -fno-common default of the gcc-arm 13.2 toolchain).

timerHardware_t timerHardware[] = {
    // Motors 1-4 on TIM1 (DShot-capable)
    DEF_TIM(TIM1,  CH1, PE9,   TIM_USE_OUTPUT_AUTO, 0, 0), // M1
    DEF_TIM(TIM1,  CH2, PE11,  TIM_USE_OUTPUT_AUTO, 0, 1), // M2
    DEF_TIM(TIM1,  CH3, PE13,  TIM_USE_OUTPUT_AUTO, 0, 2), // M3
    DEF_TIM(TIM1,  CH4, PE14,  TIM_USE_OUTPUT_AUTO, 0, 3), // M4

    // Motors 5-6 on TIM3 (DShot-capable)
    DEF_TIM(TIM3,  CH3, PB0,   TIM_USE_OUTPUT_AUTO, 0, 4), // M5
    DEF_TIM(TIM3,  CH4, PB1,   TIM_USE_OUTPUT_AUTO, 0, 5), // M6

    // Motors 7-10 on TIM4 (DShot-capable on the streamed channels; M9/M10 no DMA)
    DEF_TIM(TIM4,  CH1, PD12,  TIM_USE_OUTPUT_AUTO, 0, 6), // M7
    DEF_TIM(TIM4,  CH2, PD13,  TIM_USE_OUTPUT_AUTO, 0, 7), // M8
    DEF_TIM(TIM4,  CH3, PD14,  TIM_USE_OUTPUT_AUTO, 0, 0), // M9  DMA_NONE
    DEF_TIM(TIM4,  CH4, PD15,  TIM_USE_OUTPUT_AUTO, 0, 0), // M10 DMA_NONE

    // Servos 1-2 on TIM12 (standard PWM only, no DMA available)
    DEF_TIM(TIM12, CH1, PB14,  TIM_USE_OUTPUT_AUTO, 0, 0), // S1
    DEF_TIM(TIM12, CH2, PB15,  TIM_USE_OUTPUT_AUTO, 0, 0), // S2

    // Auxiliary outputs (not counted in MAX_PWM_OUTPUT_PORTS)
    DEF_TIM(TIM2,  CH1, PA5,   TIM_USE_LED,         0, 8), // WS2812 LED strip
    DEF_TIM(TIM14, CH1, PA7,   TIM_USE_BEEPER,      0, 0), // Passive piezo buzzer
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
