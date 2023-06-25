# dses-interferometer-controller
A repository for interferometer controller code, hardware, and simulations. 

See dses.science for more information on the Deep Space Exploration Society.

## System Overview

![System Overview Diagram](https://github.com/sailedeer/dses-interferometer-controller/blob/main/Documentation/Images/System-Overview.png?raw=true)

## Hardware
![Dish Control PCB](https://github.com/sailedeer/dses-interferometer-controller/blob/main/Documentation/Images/Dish-Control-PCB.png?raw=true)

![Encoder PCB](https://github.com/sailedeer/dses-interferometer-controller/blob/main/Documentation/Images/Encoder-PCB.png?raw=true)

Both the dish control PCB and encoder PCB have been sent out to fab. The bill of materials for each of these PCBs, as well as the system as a whole, can be found [here](https://docs.google.com/spreadsheets/d/1BclJA9Vqp6U5VNtGhfOCnUhMfViFTNw0S3vL4EeDwUc/edit?usp=sharing).

## Hardware Setup
To view/edit the KiCAD schematic, with KiCAD 6 open, go to Preferences -> Configure Paths, and add a new environment variable called "EWB" that points to dses-interferometer-controller\Hardware\lib.

The Raspberry Pi needs to be flashed with 64 bit Raspbian to work with Radioconda

## Simulation Setup
Use Python 3.9.
