# Crypto - A cryptocurrency watcher made with GTK+3

Crypto uses the CoinCap.io API to fetch up-to-date data about cryptocurrencies.

## Requirements

* gcc
* make
* gtk3
* json-glib
* libcurl

### Arch Linux (and derivatives)

```bash
sudo pacman -S gcc make gtk3 json-glib libcurl
```

### Ubuntu (and derivatives)

```bash
sudo apt install gcc make libgtk-3-dev libjson-glib-dev libcurl4-openssl-dev 
```

Currently requires network connection to start correctly

## Installation

```bash
git clone https://github.com/ErikWallstrom/Crypto.git
cd Crypto
make
sudo make install
```

## Screenshots

![](./screenshot1.png)
![](./screenshot2.png)



## License

![](http://www.gnu.org/graphics/gplv3-127x51.png "GPLv3")
