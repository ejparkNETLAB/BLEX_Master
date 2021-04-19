# BLEX

Welcome to BLEX on the Master side.

This repo demonstrates how to reproduce the results from
[_BLEX: Flexible Multi-Connection Scheduling for Bluetooth Low Energy_](https://drive.google.com/file/d/1txzinSNUsFuD-SCqadtUpBoXayle00vF/view) (to be published at IPSN 2021) on the
[Zephyr project](https://www.zephyrproject.org/). 

![BLEX_result](https://user-images.githubusercontent.com/62861175/114977049-fcacec80-9ec1-11eb-8bf4-992f851f4be2.png)

## Getting Started

This is a fork of [Zephyr Project](https://www.zephyrproject.org/).

### Experimental settings

Two UP2 boards were used to build the code.
One UP2 board each was used for one master and several slaves.
For the Bluetooth Low Energy (BLE) radio, We used five [Nordic nrf52840 DKs](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK).
Code has only been tested on Ubuntu 16.04.6.

### Install

#### 1. Install Zephyr project

Install the Zpehyr Project on the Ubuntu refering [Zephyr getting started v2.2.1](https://docs.zephyrproject.org/2.2.0/index.html).
Code has only been tested on Zephyr Project v2.2.1 and zephyr-sdk-0.11.2.

#### 2. Clone code

##### 1) On the Master node
```bash
git clone https://github.com/ejparkNETLAB/BLEX_Master.git
```

##### 1) On the Slave node
```bash
git clone https://github.com/ejparkNETLAB/BLEX_Slave.git
```

#### 3. Run grabserial

On the slave side, read the throughput using grabserial command.
Execute the following command in each new terminal window to get the message of 4 slaves.

```bash
grabserial -t -b 115200 -d /dev/ttyACM0
grabserial -t -b 115200 -d /dev/ttyACM1
grabserial -t -b 115200 -d /dev/ttyACM2
grabserial -t -b 115200 -d /dev/ttyACM3
```

### Evaluate

#### 1. Performance of BLEX when establising new connections with the same connection interval

On the Ubuntu server connected to the master node, run the following script file.
```bash
cd ~/zephyrproject/zephyr/
./BLEX_same_interval.sh 
```

On the Ubuntu server connected to the slave nodes, run the following scrip files according to the scenario.

##### 1) Increasing traffic scenario
```bash
cd ~/zephyrproject/zephyr/
./BLEX_increasing_traffic.sh
```

##### 2) Same traffic scenario
```bash
cd ~/zephyrproject/zephyr/
./BLEX_same_traffic.sh
```

##### 1) Decreasing traffic scenario
```bash
cd ~/zephyrproject/zephyr/
./BLEX_decreasing_traffic.sh
```


#### 2. Performance of BLEX when updating ongoing connections with the same connection interval

On the Ubuntu server connected to the master node, run the following script file.
```bash
cd ~/zephyrproject/zephyr/
./BLEX_real_update.sh
```

On the Ubuntu server connected to the slave nodes, run the following scrip file.
```bash
cd ~/zephyrproject/zephyr/
./BLEX_real_update.sh
```


#### 3. Performance of BLEX with the real applications

On the Ubuntu server connected to the master node, run the following script file.
```bash
cd ~/zephyrproject/zephyr/
./BLEX_real_app.sh
```

On the Ubuntu server connected to the slave nodes, run the following scrip file.
```bash
cd ~/zephyrproject/zephyr/
./BLEX_real_app.sh
```

* By default, the codes are executed BLEX controller.
* To compare BLEX with the other schemes, change the configurations in the code below, on the master side.
```bash
cd ~/zephyrproject/zephyr/include/bluetooth/conn.h
```
