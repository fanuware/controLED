# controLED

Controls a LED Matrix on the Raspberry Pi or another linux based system.

LED's can be light up on a Raspberry Pi 3 B.
It's also possible to use it on another systems(i.e Ubuntu), just that the LED's are held virtually.
- Websocket connections allow controlling the LED's from a html frontend.
- Multiple connections are allowed to connect and control simultaneously.
- Different animations are available and can be started/played (Snake Game, Light Show, Dijkstras shortest path)
- Two different LED displays are supported and either one can be plugged in; displays: WS2812(PWM), 74HC595(SPI)



## Getting Started

Use the package manager [APT](https://packages.debian.org/en/apt) to install dependencies of controLED.

### Install dependencies

```bash
apt-get install libboost-system-dev
apt-get install libssl-dev
```

### Configure LED device on your Raspberry

#### Configuration for WS2812 PWM
/etc/modprobe.d/snd-blacklist.conf
```bash
blacklist snd_bcm2835
```

/boot/config.txt
```bash
# Enable audio(loads snd_bcm2835)
#dtparam=audio=on
```

#### Configuration for 74HC595 SPI
/boot/config.txt
```bash
device_tree=bcm2710-rpi-3-b.dtb
dtparam=spi=on
```

### Build controLED from source
```bash
git clone <controLED-repo>
cd <controLED-project>
git submodule update --init
mkdir build
cd build
cmake ../
make
```


## Usage

### 1. Start server
```bash
sudo bin/controLED
```

### 2. Connect from your local browser
```bash
cd <project-root>
cd html
chromium-browser controled.html
```