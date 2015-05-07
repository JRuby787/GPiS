# GPiS
Project for Stevens Institute of Technology CPE-555 Real-Time and Embedded Systems

Raspberry Pi based GPS navigation system implemented in C++/Qt

To install required libraries:

```
sudo apt-get install qt4-dev-tools qtmobility-dev
sudo apt-get install gpsd gpsd-clients python-gps libgps-dev
```

To compile:

```
qmake-qt4
make
```

To run:

```
bin/GPiS
```
