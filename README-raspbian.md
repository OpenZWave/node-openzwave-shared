# Installation on raspbian Debian Wheezy for Raspberry PI 2 model B

## Install nodejs and libudev-dev
#### Get the 0.12 nodejs deb package for raspbian
```
wget http://node-arm.herokuapp.com/node_latest_armhf.deb
```
#### Install nodejs
```
sudo dpkg -i node_latest_armhf.deb
```
#### Install libudev-dev
```
sudo apt-get install -y libudev-dev
```
## Download and compile Open-Zwave library
#### Get the unix source at http://old.openzwave.com/downloads/ (tested with openzwave-1.2.919.tar.gz)
```
wget http://old.openzwave.com/downloads/openzwave-<version>.tar.gz
```

#### Untar
```
tar zxvf openzwave-*.gz
```

#### Compile Open-Zwave
```
cd openzwave-*
make && sudo make install
```

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

## Install node-openzwave-shared
#### Go to your project directory
```
cd /<project directory>
```

#### npm node-openzwave-shared
```
npm install openzwave-shared
```

#### You can test node-openzwave-shared
```
wget https://raw.githubusercontent.com/OpenZWave/node-openzwave-shared/master/test2.js
node test2.js
```
