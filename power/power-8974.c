/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 * Copyright (c) 2014, The CyanogenMod Project
 * Copyright (c) 2017, Dr. Ramm 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * *    * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define LOG_NIDEBUG 0

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>

#define LOG_TAG "QCOM PowerHAL"
#include <utils/Log.h>
#include <hardware/hardware.h>
#include <hardware/power.h>

#include "utils.h"
#include "metadata-defs.h"
#include "hint-data.h"
#include "performance.h"
#include "power-common.h"

#define USINSEC 1000000L
#define NSINUS 1000L

static struct timespec s_previous_boost_timespec;

static int display_hint_sent;
static int display_hint2_sent;
static int first_display_off_hint;
extern int display_boost;

static int current_power_profile = PROFILE_BALANCED;

int get_number_of_profiles() {
    return 5;
}

inline static void set_thermal_profile(int profile) {
        if (profile == PROFILE_HIGH_PERFORMANCE){
			/* change thermal zones*/
			sysfs_write( "/sys/kernel/msm_thermal/enabled", "0");
			sysfs_write( "/sys/kernel/msm_thermal/zone0", "2265600 45 5");
			sysfs_write( "/sys/kernel/msm_thermal/zone1", "2265600 50 46");
			sysfs_write( "/sys/kernel/msm_thermal/zone2", "2265600 55 51");
			sysfs_write( "/sys/kernel/msm_thermal/zone3", "2265600 60 56");
			sysfs_write( "/sys/kernel/msm_thermal/zone4", "2265600 65 61");
			sysfs_write( "/sys/kernel/msm_thermal/zone5", "2265600 70 66");
			sysfs_write( "/sys/kernel/msm_thermal/zone6", "1728000 95 71");
			sysfs_write( "/sys/kernel/msm_thermal/zone7", "652800  99 96");
			sysfs_write( "/sys/kernel/msm_thermal/enabled", "1");
			
			/* adreno part*/
			sysfs_write( "/sys/module/adreno_idler/parameters/adreno_idler_active", "N");			
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/force_bus_on", "1");
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/bus_split", "0");
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/force_rail_on", "1");
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/force_clk_on", "1");
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/devfreq/governor", "performance");
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/idle_timer", "1000000");
			
			sysfs_write( "/sys/devices/system/cpu/sched_mc_power_savings", "0");
			
			ALOGD("Power tweaks by Dr. Ramm: power");

        } 
		else if (profile == PROFILE_BIAS_POWER || profile == PROFILE_POWER_SAVE){
			/* change thermal zones*/
			sysfs_write( "/sys/kernel/msm_thermal/enabled", "0");
			sysfs_write( "/sys/kernel/msm_thermal/zone0", "1728000 45 5");
			sysfs_write( "/sys/kernel/msm_thermal/zone1", "1497600 50 46");
			sysfs_write( "/sys/kernel/msm_thermal/zone2", "1267200 55 51");
			sysfs_write( "/sys/kernel/msm_thermal/zone3", "1036800 60 56");
			sysfs_write( "/sys/kernel/msm_thermal/zone4", "883200 65 61");
			sysfs_write( "/sys/kernel/msm_thermal/zone5", "729600 70 66");
			sysfs_write( "/sys/kernel/msm_thermal/zone6", "652800 75 71");
			sysfs_write( "/sys/kernel/msm_thermal/zone7", "652800 99 76");
			sysfs_write( "/sys/kernel/msm_thermal/enabled", "1");
			
			/* adreno part*/
			sysfs_write( "/sys/module/adreno_idler/parameters/adreno_idler_active", "Y");			
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/force_bus_on", "0");
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/bus_split", "1");
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/force_rail_on", "0");
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/force_clk_on", "0");
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/devfreq/governor", "msm-adreno-tz");
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/idle_timer", "80");
			
			sysfs_write( "/sys/devices/system/cpu/sched_mc_power_savings", "1");
			
			ALOGD("Power tweaks by Dr. Ramm: eco");
				
		}
		else {
			/* change thermal zones*/
			sysfs_write( "/sys/kernel/msm_thermal/enabled", "0");
			sysfs_write( "/sys/kernel/msm_thermal/zone0", "1958400 45 5");
			sysfs_write( "/sys/kernel/msm_thermal/zone1", "1728000 50 46");
			sysfs_write( "/sys/kernel/msm_thermal/zone2", "1497600 55 51");
			sysfs_write( "/sys/kernel/msm_thermal/zone3", "1267200 60 56");
			sysfs_write( "/sys/kernel/msm_thermal/zone4", "1036800 65 61");
			sysfs_write( "/sys/kernel/msm_thermal/zone5", "883200 70 66");
			sysfs_write( "/sys/kernel/msm_thermal/zone6", "729600 75 71");
			sysfs_write( "/sys/kernel/msm_thermal/zone7", "652800 99 76");
			sysfs_write( "/sys/kernel/msm_thermal/enabled", "1");
			
			/* adreno part*/
			sysfs_write( "/sys/module/adreno_idler/parameters/adreno_idler_active", "Y");			
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/force_bus_on", "0");
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/bus_split", "1");
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/force_rail_on", "0");
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/force_clk_on", "0");
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/devfreq/governor", "msm-adreno-tz");
			sysfs_write( "/sys/class/kgsl/kgsl-3d0/idle_timer", "80");
			
			sysfs_write( "/sys/devices/system/cpu/sched_mc_power_savings", "1");
			
			ALOGD("Power tweaks by Dr. Ramm: balanced");
		}		
}

static void set_power_profile(int profile) {

    if (profile == current_power_profile)
        return;

    ALOGV("%s: profile=%d", __func__, profile);

    if (current_power_profile != PROFILE_BALANCED) {
        undo_hint_action(DEFAULT_PROFILE_HINT_ID);
        ALOGV("%s: hint undone", __func__);
    }

    if (profile == PROFILE_HIGH_PERFORMANCE) {
        int resource_values[] = { CPUS_ONLINE_MIN_4, 0x0901,
            CPU0_MIN_FREQ_TURBO_MAX, CPU1_MIN_FREQ_TURBO_MAX,
            CPU2_MIN_FREQ_TURBO_MAX, CPU3_MIN_FREQ_TURBO_MAX };
        perform_hint_action(DEFAULT_PROFILE_HINT_ID,
            resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
        ALOGD("%s: set performance mode", __func__);
    } else if (profile == PROFILE_BIAS_PERFORMANCE) {
        int resource_values[] = {
            CPU0_MIN_FREQ_NONTURBO_MAX + 1, CPU1_MIN_FREQ_NONTURBO_MAX + 1,
            CPU2_MIN_FREQ_NONTURBO_MAX + 1, CPU2_MIN_FREQ_NONTURBO_MAX + 1 };
        perform_hint_action(DEFAULT_PROFILE_HINT_ID,
            resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
        ALOGD("%s: set bias perf mode", __func__);
    } else if (profile == PROFILE_BIAS_POWER) {
        int resource_values[] = { 0x0A03,
            CPU0_MAX_FREQ_NONTURBO_MAX, CPU1_MAX_FREQ_NONTURBO_MAX,
            CPU1_MAX_FREQ_NONTURBO_MAX, CPU2_MAX_FREQ_NONTURBO_MAX };
        perform_hint_action(DEFAULT_PROFILE_HINT_ID,
            resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
        ALOGD("%s: set bias power mode", __func__);
    } else if (profile == PROFILE_POWER_SAVE) {
        int resource_values[] = { 0x0A03, CPUS_ONLINE_MAX_LIMIT_2,
            CPU0_MAX_FREQ_NONTURBO_MAX, CPU1_MAX_FREQ_NONTURBO_MAX,
            CPU2_MAX_FREQ_NONTURBO_MAX, CPU3_MAX_FREQ_NONTURBO_MAX };
        perform_hint_action(DEFAULT_PROFILE_HINT_ID,
            resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
        ALOGD("%s: set powersave", __func__);
    }

    set_thermal_profile(profile);
    current_power_profile = profile;
}

static long long calc_timespan_us(struct timespec start, struct timespec end) {
    long long diff_in_us = 0;
    diff_in_us += (end.tv_sec - start.tv_sec) * USINSEC;
    diff_in_us += (end.tv_nsec - start.tv_nsec) / NSINUS;
    return diff_in_us;
}

extern void interaction(int duration, int num_args, int opt_list[]);

int power_hint_override(__attribute__((unused)) struct power_module *module,
        power_hint_t hint, void *data)
{
    if (hint == POWER_HINT_SET_PROFILE) {
        set_power_profile(*(int32_t *)data);
        return HINT_HANDLED;
    }

    // Skip other hints in high/low power modes
    if (current_power_profile == PROFILE_POWER_SAVE ||
            current_power_profile == PROFILE_HIGH_PERFORMANCE) {
        return HINT_HANDLED;
    }

    if (hint == POWER_HINT_LAUNCH) {
        int duration = 2000;
        int resources[] = { CPUS_ONLINE_MIN_3,
            0x211, 0x311, 0x411, 0x511 };

        interaction(duration, sizeof(resources)/sizeof(resources[0]), resources);

        return HINT_HANDLED;
    }

    if (hint == POWER_HINT_CPU_BOOST) {
        int duration = *(int32_t *)data / 1000;
        int resources[] = { CPUS_ONLINE_MIN_2,
            0x20B, 0x30B, 0x40B, 0x50B };

        if (duration)
            interaction(duration, sizeof(resources)/sizeof(resources[0]), resources);

        return HINT_HANDLED;
    }

    if (hint == POWER_HINT_INTERACTION) {
        int duration = 500, duration_hint = 0;

        if (data) {
            duration_hint = *((int *)data);
        }

        duration = duration_hint > 0 ? duration_hint : 500;

        struct timespec cur_boost_timespec;
        clock_gettime(CLOCK_MONOTONIC, &cur_boost_timespec);
        long long elapsed_time = calc_timespan_us(s_previous_boost_timespec, cur_boost_timespec);

        if (elapsed_time > 750000)
            elapsed_time = 750000;
        // don't hint if it's been less than 250ms since last boost
        // also detect if we're doing anything resembling a fling
        // support additional boosting in case of flings
        else if (elapsed_time < 250000 && duration <= 750)
            return HINT_HANDLED;

        s_previous_boost_timespec = cur_boost_timespec;

        int resources[] = { (duration >= 2000 ? CPUS_ONLINE_MIN_3 : CPUS_ONLINE_MIN_2),
            0x20B, 0x30B, 0x40B, 0x50B };

        if (duration)
            interaction(duration, sizeof(resources)/sizeof(resources[0]), resources);

        return HINT_HANDLED;
    }


    return HINT_NONE;
}

int set_interactive_override(struct power_module *module __unused, int on)
{
    char governor[80];

    if (get_scaling_governor(governor, sizeof(governor)) == -1) {
        ALOGE("Can't obtain scaling governor.");

        return HINT_NONE;
    }

    if (!on) {
        /* Display off. */
        /*
         * We need to be able to identify the first display off hint
         * and release the current lock holder
         */
        if (display_boost) {
            if (!first_display_off_hint) {
                undo_initial_hint_action();
                first_display_off_hint = 1;
            }
            /* used for all subsequent toggles to the display */
            if (!display_hint2_sent) {
                undo_hint_action(DISPLAY_STATE_HINT_ID_2);
                display_hint2_sent = 1;
            }
        }

        if ((strncmp(governor, ONDEMAND_GOVERNOR, strlen(ONDEMAND_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(ONDEMAND_GOVERNOR))) {
            int resource_values[] = {MS_500, SYNC_FREQ_600, OPTIMAL_FREQ_600, THREAD_MIGRATION_SYNC_OFF};

            if (!display_hint_sent) {
                perform_hint_action(DISPLAY_STATE_HINT_ID,
                        resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
                display_hint_sent = 1;
            }

            return HINT_HANDLED;
        }
    } else {
        /* Display on */
        if (display_boost && display_hint2_sent) {
            int resource_values2[] = {CPUS_ONLINE_MIN_2};
            perform_hint_action(DISPLAY_STATE_HINT_ID_2,
                    resource_values2, sizeof(resource_values2)/sizeof(resource_values2[0]));
            display_hint2_sent = 0;
        }

        if ((strncmp(governor, ONDEMAND_GOVERNOR, strlen(ONDEMAND_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(ONDEMAND_GOVERNOR))) {
            undo_hint_action(DISPLAY_STATE_HINT_ID);
            display_hint_sent = 0;

            return HINT_HANDLED;
        }
    }

    return HINT_NONE;
}
