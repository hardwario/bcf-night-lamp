<a href="https://www.hardwario.com/"><img src="https://www.hardwario.com/ci/assets/hw-logo.svg" width="200" alt="HARDWARIO Logo" align="right"></a>

# LED Night Lamp

[![Travis](https://img.shields.io/travis/bigclownlabs/bcf-skeleton/master.svg)](https://travis-ci.org/bigclownlabs/bcf-skeleton)
[![Release](https://img.shields.io/github/release/bigclownlabs/bcf-skeleton.svg)](https://github.com/bigclownlabs/bcf-skeleton/releases)
[![License](https://img.shields.io/github/license/bigclownlabs/bcf-skeleton.svg)](https://github.com/bigclownlabs/bcf-skeleton/blob/master/LICENSE)
[![Twitter](https://img.shields.io/twitter/follow/hardwario_en.svg?style=social&label=Follow)](https://twitter.com/hardwario_en)

This night lamp can be operated alone, but also can be extended by other modules and tags:

* Encoder Module can be added to the Night Lamp to control the intensity of lamp
* Button Module can be added so you don't have to press tiny button on the Core Module
* Button Module can be paired and control the lamp
* Motion Detector can be paired with Night Lamp and turn the lamp on for amount of time
* Lux Meter Tag can be added to the Night Lamp so the Motion Detector turn the LED on only when is dark in the room

# Pairing:

1. Use HARDWARIO Playground and flash other Core Module with Radio Button or Radio Motion Detector
2. Power on the Night Lamp, pres the button for 3 seconds - the LED Strip turnes RED and LED blinks on Core Module
3. Remove USB or batteries, wait 5 seconds and connect batteries to the Button or Motion detector
4. After pairing one module, the red color turns off. Repeat step 2 for adding more devices

# Clear paired nodes

1. Power the Night Lamp
2. Immediatelly after connecting the power press and hold the button. You have to do it at least 200ms after power-up. Don't press button together with power-up.
3. The modules are unpaired


If you want to get more information about Core Module, firmware and how to work with it, please follow this link:

**https://developers.hardwario.com/firmware/basic-overview/**

User's application code (business logic) goes into `app/application.c`.
The default content works as a *Hello World* example.
When flashed into Core Module, it toggles LED state with each button press.

## License

This project is licensed under the [MIT License](https://opensource.org/licenses/MIT/) - see the [LICENSE](LICENSE) file for details.

---

Made with &#x2764;&nbsp; by [**HARDWARIO s.r.o.**](https://www.hardwario.com/) in the heart of Europe.
