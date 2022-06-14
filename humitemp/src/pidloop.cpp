
#include "pidloop.h"

#include <Arduino.h>
#include <FPID.h>

#include "config.h"
#include "globals.h"
#include "settings.h"
#include "tools-log.h"

PIDLoop::PIDLoop(pidloop_settings_t& s, const double& input) :
    _pid(&(s.fpid), (&input), (&_output)),
    _settings(s), _input_ref(input)
{
};

bool PIDLoop::begin()
{
    _pin_n = static_cast<gpio_num_t>(_settings.pin_n);
    _pin_p = static_cast<gpio_num_t>(_settings.pin_p);

    // config hardware
  	pinMode(_pin_n, OUTPUT);
  	pinMode(_pin_p, OUTPUT);
	digitalWrite(_pin_n, LOW);
	digitalWrite(_pin_p, LOW);

	_windowstarttime = millis();
    _pid_last = millis();

    switch(_settings.mode)
    {
        case MODE_NONE:
            set_active(false);
        	_pid.setOutputLimits(0,0);
            break;
        case MODE_NP:
        	_pid.setOutputLimits(0, _settings.windowtime);
            break;
        case MODE_ZP:
        	_pid.setOutputLimits(0, _settings.windowtime);
            break;
    };

    reset_output();

    set_active(_settings.active);

	return true;
};

void PIDLoop::reset_output()
{
    switch(_settings.mode)
    {
        case MODE_NONE:
            break;
        case MODE_NP:
        	_output = _settings.windowtime / 2;
            break;
        case MODE_ZP:
	        _output = 0;
            break;
    };
};

void PIDLoop::set_active(bool active)
{
    DBG("set_active(%s)", active ? "true":"false");

    if(_settings.mode == MODE_NONE)
    {
        WARNING("No pid output mode set: pid remains in-active.");
        active = false;
    };

    if(active)
    {
        _pid.alignOutput();
        reset_output();
    } else {
        if(_pin_n)
            digitalWrite(_pin_n, LOW);
        if(_pin_p)
            digitalWrite(_pin_p, LOW);
    };
    _settings.active = active;
    _active_last = active;
};

void PIDLoop::loop()
{
    if(_active_last != _settings.active)
    {
        set_active(_settings.active);
    };

    // See if its time to do another PID iteration
    // in in-active mode just update input value for display purposes
    time_t now = millis();
    if(now > _pid_last + _settings.looptime)
    {
        if(_settings.active)
        {
            if(!_pid.calculate())
                _output = NAN;

            DBG("PID: Input = %.2f, Setpoint = %.2f, Output = %.2f", _input_ref, _settings.fpid.setpoint, _output);
        }else{
            // DBG("PID: Input = %.2f, In-Active, Output = %.2f", _input_ref, _output);
        };

        // Queue next iteration
        _pid_last = now;
    };

    // turn of output if not active or if PID loop had an error
    if(!_settings.active || isnan(_output))
    {
        if(_pin_n)
            digitalWrite(_pin_n, LOW);
        if(_pin_p)
            digitalWrite(_pin_p, LOW);
        return;
    };

    // Process timewindow valve depending on PID Output
    if (now - _windowstarttime > _settings.windowtime)
    { //time to shift the Relay Window
        _windowstarttime += _settings.windowtime;
    };

    // Set output
    uint A = LOW;
    uint B = LOW;
    switch(_settings.mode)
    {
        case MODE_NONE:
            break;
        case MODE_NP:
            if (_output > (now - _windowstarttime))
                B = HIGH;
            else
                A = HIGH;
            break;
        case MODE_ZP:
            if (_output > (now - _windowstarttime))
                B = HIGH;
            break;
    };
    if(_pin_n)
        digitalWrite(_pin_n, A);
    if(_pin_p)
        digitalWrite(_pin_p, B);
};
