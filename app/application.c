#include <application.h>
#include <bcl.h>
#include <bc_ws2812b.h>

#define LED_STRIP_COUNT 144
#define LED_STRIP_TYPE BC_LED_STRIP_TYPE_RGBW
#define LED_STRIP_DEFAULT_BRIGHTNESS 80 /* 1 - 255 */

// Time when the light turn off after motion detection
#define LED_STRIP_MOTION_DETECTOR_TIMEOUT (60 * 1000)

// Lux Meter Tag - optional
#define LUX_METER_TAG_UPDATE_INTERVAL 1000
// Threshold in lux. When the luminosity is below this value and Motion detector sends event, teh LED strip is turned on
#define LUX_METER_TAG_LUX_THRESHOLD 500

bc_led_t led;
bc_button_t button;
bc_tag_lux_meter_t lux_meter;

bool radio_pairing_mode = false;
uint32_t lux_meter_value = 0;

uint8_t led_strip_brightness = LED_STRIP_DEFAULT_BRIGHTNESS;
bool led_strip_on = false;

bc_scheduler_task_id_t task_motion_timeout_id;

void task_motion_timeout();

// Led strip
static uint32_t _bc_module_power_led_strip_dma_buffer[LED_STRIP_COUNT * LED_STRIP_TYPE * 2];
const bc_led_strip_buffer_t led_strip_buffer =
{
    .type = LED_STRIP_TYPE,
    .count = LED_STRIP_COUNT,
    .buffer = _bc_module_power_led_strip_dma_buffer
};

bc_led_strip_t led_strip;

bool driver_led_strip_init(const bc_led_strip_buffer_t *led_strip)
{
    (void) led_strip;
    return true;
}

bool driver_led_strip_write()
{
    bc_led_strip_write(&led_strip);
    return true;
}

const bc_led_strip_driver_t led_strip_driver =
{
    .init = driver_led_strip_init,
    .write = driver_led_strip_write,
    .set_pixel = bc_ws2812b_set_pixel_from_uint32,
    .set_pixel_rgbw = bc_ws2812b_set_pixel_from_rgb,
    .is_ready = bc_ws2812b_is_ready
};

void led_strip_update()
{
    if (led_strip_on)
    {
        uint32_t color = led_strip_brightness << 24 | led_strip_brightness << 16 | led_strip_brightness << 8 | led_strip_brightness;
        bc_led_strip_fill(&led_strip, color);
    }
    else
    {
        bc_led_strip_fill(&led_strip, 0);
    }

    bc_led_strip_write(&led_strip);
}

void led_strip_set_state(bool state)
{
    led_strip_on = state;
    led_strip_update();
}

bool led_strip_get_state()
{
    return led_strip_on;
}

void led_strip_toggle_state(void)
{
    // Disable motion detector timeout
    bc_scheduler_plan_absolute(task_motion_timeout_id, BC_TICK_INFINITY);

    led_strip_on = !led_strip_on;
    led_strip_update();
}

void bc_radio_pub_on_event_count(uint64_t *id, uint8_t event_id, uint16_t *event_count)
{
    (void) id;
    (void) event_count;

    bc_led_pulse(&led, 10);

    if (event_id == BC_RADIO_PUB_EVENT_PUSH_BUTTON)
    {
        led_strip_toggle_state();
    }

    if (event_id == BC_RADIO_PUB_EVENT_PIR_MOTION && led_strip_get_state() == false && lux_meter_value < LUX_METER_TAG_LUX_THRESHOLD)
    {
        led_strip_set_state(true);
        bc_scheduler_plan_from_now(task_motion_timeout_id, LED_STRIP_MOTION_DETECTOR_TIMEOUT);
    }
}

void radio_pairing_start()
{
    radio_pairing_mode = true;
    bc_radio_pairing_mode_start();
    bc_led_set_mode(&led, BC_LED_MODE_BLINK_FAST);

    bc_led_strip_fill(&led_strip, 0x55000000);
    bc_led_strip_write(&led_strip);
}

void radio_pairing_stop()
{
    radio_pairing_mode = false;
    bc_radio_pairing_mode_stop();
    bc_led_set_mode(&led, BC_LED_MODE_OFF);

    led_strip_update();
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_CLICK)
    {
        led_strip_toggle_state();
    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        if (radio_pairing_mode)
        {
            radio_pairing_stop();
        }
        else
        {
            radio_pairing_start();
        }
    }
}

void radio_event_handler(bc_radio_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_RADIO_EVENT_ATTACH)
    {
        bc_led_pulse(&led, 1000);
        radio_pairing_stop();

        //uint64_t radio_id = bc_radio_get_event_id();
        //bc_radio_is_peer_device()
    }
    else if (event == BC_RADIO_EVENT_ATTACH_FAILURE)
    {
        bc_led_pulse(&led, 5000);
        radio_pairing_stop();
    }
}


void encoder_event_handler(bc_module_encoder_event_t event, void *param)
{
    (void) param;

    // Disable encoder when light is off
    if (led_strip_get_state() == false)
    {
        return;
    }

    if (event == BC_MODULE_ENCODER_EVENT_ROTATION)
    {
        int32_t check = led_strip_brightness + (bc_module_encoder_get_increment() * 5);
        if (check > 255)
        {
            check = 255;
        }
        else if (check < 0)
        {
            check = 1;
        }

        led_strip_brightness = check;
        led_strip_update();
    }
}

void lux_meter_event_handler(bc_tag_lux_meter_t *self, bc_tag_lux_meter_event_t event, void *event_param)
{
    float value;
    (void) event_param;

    if (event == BC_TAG_LUX_METER_EVENT_UPDATE)
    {
        if (bc_tag_lux_meter_get_illuminance_lux(self, &value))
        {
            lux_meter_value = (uint32_t)value;
        }
    }
}

void application_init(void)
{
    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_OFF);

    bc_led_strip_init(&led_strip, bc_module_power_get_led_strip_driver(), &led_strip_buffer);
    bc_led_strip_fill(&led_strip, 0);

    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    bc_radio_init(BC_RADIO_MODE_GATEWAY);
    bc_radio_set_event_handler(radio_event_handler, NULL);

    // Encoder
	bc_module_encoder_init();
	bc_module_encoder_set_event_handler(encoder_event_handler, NULL);

    // Luxmeter
    bc_tag_lux_meter_init(&lux_meter, BC_I2C_I2C0, BC_TAG_LUX_METER_I2C_ADDRESS_DEFAULT);
    bc_tag_lux_meter_set_update_interval(&lux_meter, LUX_METER_TAG_UPDATE_INTERVAL);
    bc_tag_lux_meter_set_event_handler(&lux_meter, lux_meter_event_handler, NULL);

    bc_led_pulse(&led, 2000);

    task_motion_timeout_id = bc_scheduler_register(task_motion_timeout, NULL, BC_TICK_INFINITY);

    // Purge all the paired nodes when encoder/button is hold during power-up
    if (bc_gpio_get_input(BC_GPIO_BUTTON))
    {
        bc_radio_peer_device_purge_all();
    }
}

void task_motion_timeout()
{
    // Turn off LED after motion detector timeout
    led_strip_set_state(false);
}
