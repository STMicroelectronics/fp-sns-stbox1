## <b>BLEMLC Application Description for STEVAL-STWINBX1 (STWIN.box) evaluation board</b>

This firmware package includes Components Device Drivers, Board Support Package and example application for the following STMicroelectronics elements:

  - STEVAL-STWINBX1 (STWIN.box) evaluation board that contains the following components:
      - MEMS sensor devices: IIS2DLPC, IIS2MDC, IIS3DWB, ISM330DHCX,IIS2ICLX,ILPS22QS,STTS22H
	  - ST25dv 64K
      - analog/digital microphone 
 
The Example application provides one example on how to program the Machine Learning core and the Finite State Machine provided by the ISM330DHCX
The application provided also firwmare over the air update allowing also to change the Firwmare running on the board

### <b>Dependencies</b>

STM32Cube packages:

  - STM32U5xx drivers from STM32CubeU5 V1.3.0
  
STEVAL-MKSBOX1V1:

  - STEVAL-STWINBX1 V1.0.2
  
### <b>Hardware and Software environment</b>

- This example runs on STEVAL-STWINBX1 (STWIN.box) evaluation board and it can be easily tailored to any other supported device and development board.
- This example must be used with the related ST BLE Sensor Android/iOS application (Version 5.0.0 or higher) available on the Google Play or Apple App Store, in order to read the sent information by Bluetooth Low Energy protocol
	
### <b>How to use it?</b>

This package contains projects for 3 IDEs viz- IAR, Keil µVision 5 and Integrated Development Environment for STM32.
In order to make the  program work, you must do the following:

 - WARNING: before opening the project with any toolchain be sure your folder
   installation path is not too in-depth since the toolchain may report errors
   after building.

For IAR:

 - Open IAR toolchain (this firmware has been successfully tested with Embedded Workbench V9.20.1).
 - Open the IAR project file EWARM\BLELowPower.eww
 - Rebuild all files and Flash the binary on STEVAL-STWINBX1

For Keil µVision 5:

 - Open Keil µVision 5 toolchain (this firmware has been successfully tested with MDK-ARM Professional Version: 5.37.0).
 - Open the µVision project file MDK-ARM\Project.uvprojx
 - Rebuild all files and Flash the binary on STEVAL-STWINBX1
		
For Integrated Development Environment for STM32:

 - Open STM32CubeIDE (this firmware has been successfully tested with Version 1.13.2)
 - Set the default workspace proposed by the IDE (please be sure that there are not spaces in the workspace path).
 - Press "File" -> "Import" -> "Existing Projects into Workspace"; press "Browse" in the "Select root directory" and choose the path where the STM32CubeIDE project is located (it should be STM32CubeIDE\).
 - Rebuild all files and and Flash the binary on STEVAL-STWINBX1
   
### <b>Author</b>

SRA Application Team

### <b>License</b>

Copyright (c) 2023 STMicroelectronics.
All rights reserved.

This software is licensed under terms that can be found in the LICENSE file
in the root directory of this software component.
If no LICENSE file comes with this software, it is provided AS-IS.
