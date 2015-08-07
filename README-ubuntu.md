# Installation on ubuntu server 15.04

## Install nodejs and libudev-dev
#### Add the nodejs 0.12 ppa
```
curl -sL https://deb.nodesource.com/setup_0.12 | sudo bash -
```

#### Install from packet manager
```
sudo apt-get install -y nodejs build-essential libudev-dev
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
export LD_LIBRARY_PATH=/usr/local/lib64
```
make it permanent by adding it to /etc/environment
```
sudo sed -i '$a LD_LIBRARY_PATH=/usr/local/lib64' /etc/environment
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
#### Copy options.xml to project directory
```
cp /usr/local/etc/openzwave/options.xml options.xml
```
#### You can test node-openzwave-shared
```
wget https://raw.githubusercontent.com/OpenZWave/node-openzwave-shared/master/test2.js
node test2.js
```
