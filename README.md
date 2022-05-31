# m5stack-slack-status-updater

When I walk away from my desk for lunch, I always forget to update my Slack status. I could use my phone, but then I have to unlock it, open Slack, pick an emoji, etc... Too many buttons, too much work.

If only some piece of hardware existed to allowed me to update my Slack status with a pre-programmed physical button 🤔

https://user-images.githubusercontent.com/4955337/171285130-c45aa088-1eee-4d63-aa8a-8f8d57907cff.mp4

https://user-images.githubusercontent.com/4955337/171285139-b7cd4357-f014-461d-b78c-a4559674e83f.mp4

## Disclaimer
⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️
- This is literally my first time writing C-type code and will be the sloppiest thing you see today.
- Your Slack API token is basically your username and password in one. Please take that into consideration before using it here.
- Please don't commit the `arduino_secrets.h` file to source control!

## Hardware you need to purchase

You need an [M5Stack Core2](https://shop.m5stack.com/collections/m5-controllers/products/m5stack-core2-esp32-iot-development-kit). I believe an [M5Stack Core](https://shop.m5stack.com/collections/m5-controllers/products/esp32-basic-core-iot-development-kit-v2-6) would work as well (I haven't tested this), but some hardware-specific functions (e.g., brightness, vibration) won't work.

If you don't want to purchase from M5Stack's website (because of the long shipping times), you can try:

* [DigiKey](https://www.digikey.com/en/products/detail/m5stack-technology-co.-ltd./K010/13151126)
* [Mouser](https://www.mouser.com/ProductDetail/M5Stack/K010?qs=sGAEpiMZZMuqBwn8WqcFUj1SFkunHY10JxY66yV4ZZHgeixQGf7z4g%3D%3D)
* [Amazon](https://www.amazon.com/dp/B07RTKTJZB)

## Why not UI Flow or PlatformIO?

Both [UIFlow](https://flow.m5stack.com/) and [PlatformIO](https://platformio.org/) would work for this project, and I even believe UIFlow offers more features than the Arduino library. However, Arduino is the gold-standard and is the easiest to setup from scratch.

## Setup the Arduino IDE

1. Install the [Arduino IDE](https://www.arduino.cc/en/software#download). There are step-by-step instructions for Windows, Mac, and Linux [here](https://www.arduino.cc/en/Guide#install-the-arduino-desktop-ide). Once installed, open the Arduino IDE.

2. Add support for ESP32 boards by going to `File-->Preferences-->Additional Board Manager URLs` and then paste in the URL below. If you have a URL in the box already, separate them with a comma.

```
https://dl.espressif.com/dl/package_esp32_index.json
```
![](/screenshots/setup_001.png)

3. Go to `Tools-->Board-->Boards Manager`, search for `esp32` and install.
![](/screenshots/setup_002.png)

4. Go to `Tools-->Manage Libraries`, search for `m5core2` and install (be sure to install the entry by `M5Stack`).
![](/screenshots/setup_003.png)

5. Go to `Tools-->Manage Libraries`, search for `Arduino_JSON` and install (be sure to install the entry by `Arduino`).
![](/screenshots/setup_005.png)

6. Go to `Tools-->Board` and select `M5Stack-Core2`.
![](/screenshots/setup_004.png)

7. I also had to make the following changes:
  * Change `Tools-->Upload Speed` from `921600` to `115200`
  * Change `Tools-->Port` to `/dev/ttyACM0`
  * Install the `pyserial` package on my distrubtion

## Obtain Slack API token

The easiest way to create a Slack API token is to [create a single-use app](https://api.slack.com/tutorials/tracks/getting-a-token) for yourself, then get an API token for that app.

1. Click [here](https://api.slack.com/apps?new_app=1), then choose to create an app *from an app manifest*. 

2. Sign into the workspace that you want this app to access. This will be the workspace that you want the M5Stack to access.

3. Copy/paste [this](https://raw.githubusercontent.com/loganmarchione/m5stack-slack-status-updater/master/manifest.yaml) YAML code into the box. This is the app manifest with the minimum permissions needed to update your user's status.

4. Click *Create*.

5. Click *Install to Workspace*.

6. Navigate to *OAuth & Permissions*, then copy the *User OAuth Token*. It will begin with `xoxp-`. This token will be needed later.

## Flash ESP32

1. Clone this repo.

```
git clone https://github.com/loganmarchione/m5stack-slack-status-updater.git
cd m5stack-slack-status-updater/M5StackSlackStatusUpdater
```

2. Rename the `arduino_secrets_sample.h` file to `arduino_secrets.h`

```
mv arduino_secrets_sample.h arduino_secrets.h
```

3. Update the `arduino_secrets.h` file to include your variables.

* WiFi SSID
* WiFi Password
* Slack API token

4. Update the `variables` section of the `5StackSlackStatusUpdater.ino` file. This is where you will set your three Slack statuses and their corresponding emojis.

5. Click `Upload` in the IDE and wait for the program to compile and upload.

## TODO
- [X] Do some printing to the serial console with more detailed info
- [ ] Maybe switch to UIFlow since the Arduino library seems to be crippled?
- [ ] Do some error handling with API call
- [ ] Add buttons that can be combinations (e.g., ButtonA+ButtonB)
- [ ] Add ability to change presence (e.g., available, away)
