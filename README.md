# Stealth Project

Stealth application to run through NS-3 Network Simulator

## Installation 

1. Install NS-3 simulator version 3.28, available [here](https://www.nsnam.org/releases/ns-3-28/)
2. Copy `node.cc` and `node.h` to folder `/HOMEPATH/ns-allinone-3.28/ns-3.28/src/network/model`
3. Copy traces file `ostermalm_003_1_new.tr` to `/HomePath/ns-allinone-3.28/ns-3.28/scratch`
4. Copy stealth files `StealthSimulation_3.cc` to `/HomePath/ns-allinone-3.28/ns-3.28/scratch`

## Usage

Run application from folder `/HomePath/ns-allinone-3.28/ns-3.28`

* Run without fixed nodes

`./waf --run scratch/StealthSimulation_5 > log.txt 2>&1`

* Run with fixed nodes

`./waf --run "scratch/StealthSimulation_5 --fixNode=3" > log.txt 2>&1`

## Results

* Results are stored in `/HomePath/ns-allinone-3.28/ns-3.28/stealth_traces`, inside a folder named **Date_Time**, like **03022019_1049**.
* The log file created `log.txt` is saved inside `/HomePath/ns-allinone-3.28/ns-3.28/`



