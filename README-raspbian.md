# Installation on Raspbian (Debian for Raspberry Pi)

You'll need a couple of well-supported packages (NodeJS and libudev-dev)

## 1. Install Node.js and libudev-dev

#### Download and install Node.js for Raspbian
```wget http://node-arm.herokuapp.com/node_latest_armhf.deb && sudo dpkg -i node_latest_armhf.deb```

#### Install libudev-dev
```sudo apt-get install -y libudev-dev```


## 2. Install the Open-Zwave library on your Raspberry:

You can take the easy route:

### (New!) Download and install the precompiled Raspbian/ARMv6 packages for Raspberry 1:

```
wget "https://github.com/ekarak/openzwave-debs-raspbian/raw/master/v1.4.79/openzwave_1.4.79.gfaea7dd_armhf.deb"
wget "https://github.com/ekarak/openzwave-debs-raspbian/raw/master/v1.4.79/libopenzwave1.3_1.4.79.gfaea7dd_armhf.deb"
wget "https://github.com/ekarak/openzwave-debs-raspbian/raw/master/v1.4.79/libopenzwave1.3-dev_1.4.79.gfaea7dd_armhf.deb"
sudo dpkg -i *openzwave*.deb
```

Or you can choose Hercules' path of blight and
### Download the OpenZWave Unix source from [the official website](http://old.openzave.com/downloads/)
```wget http://old.openzwave.com/downloads/openzwave-1.4.1.tar.gz```

#### Untar
```tar zxvf openzwave-*.gz```

#### Compile Open-Zwave
```cd openzwave-* && make && sudo make install```

#### Update the environment variable
```
export LD_LIBRARY_PATH=/usr/local/lib
```
make it permanent by adding it to /etc/environment
```
sudo sed -i '$a LD_LIBRARY_PATH=/usr/local/lib' /etc/environment
```
At this step you can ensure Open-Zwave library is correctly installed with
```
MinOZW
```

## 3. Install node-openzwave-shared

#### Go to your project directory
`cd /<project directory>`

#### npm node-openzwave-shared
`npm install openzwave-shared`

#### You can test node-openzwave-shared
```
wget https://raw.githubusercontent.com/OpenZWave/node-openzwave-shared/master/test2.js
node test2.js
```
