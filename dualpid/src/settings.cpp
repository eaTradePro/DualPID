#include "settings.h"

#include <Arduino.h>
#include <nvs.h>

#include "config.h"
#include "tools-log.h"
#include "globals.h"

#include "inputdrv.h"
#include "outputdrv.h"

SettingsManager::SettingsManager(settings_t& settings) : NVSettings(&settings, sizeof(settings_t))
{

};

bool SettingsManager::set_defaults_since(const uint32_t data_version)
{
    settings_t *settings = static_cast<settings_t*>(_data);

    switch(data_version)
    {
        default:
            ERROR("Unknown data version: %d", data_version);
            return false;
        case 0: // empty blob
        case 1: // 
        case 2: //
        case 3: //
        case 4: //
        case 5: //
            DBG("Init settings v5: defaults");
            memset(_data, 0, _data_size);

            settings->sensor_loop_ms =              DEFAULT_SENSOR_LOOP_MS;
            settings->graph_delta =                 DEFAULT_GRAPH_DELTA_MS;

            settings->pid1.active =                 false;
            settings->pid1.output_drv =             OUTPUT_DRIVER_NONE;
            settings->pid1.looptime =               PID_DEFAULT_LOOPTIME_MS;
            settings->pid1.output.param[0] =        GPIO_NUM_NC;
            settings->pid1.output.param[1] =        GPIO_NUM_NC;
            settings->pid1.output.param[2] =        0;
            settings->pid1.min_output =             PID_DEFAULT_MIN_OUTPUT;
            settings->pid1.max_output =             PID_DEFAULT_MAX_OUTPUT;
            settings->pid1.fpid.kF =                PID_DEFAULT_F;
            settings->pid1.fpid.kF_offset =         PID_DEFAULT_F_OFFSET;
            settings->pid1.fpid.kP =                PID_DEFAULT_P;
            settings->pid1.fpid.kI =                PID_DEFAULT_I;
            settings->pid1.fpid.kD =                PID_DEFAULT_D;
            settings->pid1.fpid.setpoint =          PID_DEFAULT_SETPOINT;
            settings->pid1.fpid.output_filter =     PID_DEFAULT_OFILTER;
            settings->pid1.fpid.D_filter =          PID_DEFAULT_DFILTER;
            settings->pid1.fpid.takebackhalf =      PID_DEFAULT_TBH;
            settings->pid1.lock_window =            PID_DEFAULT_LOCK_WINDOW;
            settings->pid1.lock_time =              PID_DEFAULT_LOCK_TIME_MS;
            settings->pid1.input_filter =           PID_DEFAULT_INPUT_FILTER;
            settings->pid1.input_drv =              INPUT_DRIVER_NONE;

            // Channel 2 has the same defaults
            memcpy(&(settings->pid2), &(settings->pid1), sizeof(PIDLoop::settings_t));

        // End with the current version:
        case 6:
            _data_version = 6;
            return true;
    };

	return true;
};

bool SettingsManager::read_blob(void* blob, const size_t blob_size, const uint32_t blob_version)
{
    switch(blob_version)
    {
        default:
            ERROR("Unknown blob version: %d", blob_version);
            return false;

        // old, non convertible settings
        case 1: // 
        case 2: 
        case 3: 
        case 4: 
        case 5: 
            set_defaults_since(0);
            _dirty = true;
            gui.showMessage("INFO", "Default settings loaded.");
            return true;

        // latest
        case 6:
            if(blob_size != sizeof(settings_t))
            {
                ERROR("Settings blob size mismatch (version is ok).");
                return false;
            };
            memcpy(_data, blob, blob_size);
            _data_version = 6;
            _dirty = false;
            return true;
    };

	return true;
};
