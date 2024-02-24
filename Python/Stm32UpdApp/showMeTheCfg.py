#!/usr/bin/env python
#
#===============================================================================
#
#                         OOOO
#                       OOOOOOOO
#      PPPPPPPPPPPPP   OOO    OOO   PPPPPPPPPPPPP
#    PPPPPPPPPPPPPP   OOO      OOO   PPPPPPPPPPPPPP
#   PPP         PPP   OOO      OOO   PPP         PPP
#  PPP          PPP   OOO      OOO   PPP          PPP
#  PPP          PPP   OOO      OOO   PPP          PPP
#  PPP          PPP   OOO      OOO   PPP          PPP
#   PPP         PPP   OOO      OOO   PPP         PPP
#    PPPPPPPPPPPPPP   OOO      OOO   PPPPPPPPPPPPPP
#     PPPPPPPPPPPPP   OOO      OOO   PPP
#               PPP   OOO      OOO   PPP
#               PPP   OOO      OOO   PPP
#               PPP   OOO      OOO   PPP
#               PPP    OOO    OOO    PPP
#               PPP     OOOOOOOO     PPP
#              PPPPP      OOOO      PPPPP
#
# @file:   showMeTheCfg.py
# @author: Hugh Spahr
# @date:   2/5/2024
#
# @note:   Open Pinball Project
#          Copyright 2024, Hugh Spahr
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#===============================================================================
#
# Read the configuration sector and give detailed report about the config.
# Should work on Windows/Ubuntu based hosts and Python 2.7.x and Python 3.12.x
#
#===============================================================================

# Support Python 3 print(,end="") functionality so py2.7 and py3.x work
from __future__ import print_function

import sys
import os
import struct
import serial
import time
import platform
if (platform.system() == 'Windows'):
  import subprocess
else:
  import usb

import rs232BIntf

IHEX_DATA = 0x00
IHEX_END = 0x01
IHEX_EXT_ADDR = 0x04
IHEX_START_ADDR = 0x05

CFG_MEM_LEN = 0x100
CFG_MEM_ADDR = 0x0800fc00

# application commands
BOARD_ADDR = 0x20
GET_VERS_CMD = 0x02
GO_BOOT_CMD = 0x05
EOM_CMD = 0xff

# bootloader commands
BOOT_STX = 0x02
BOOT_CMD_WRITE_PROG_MEM = 0x00
BOOT_CMD_READ_PROG_MEM = 0x01
BOOT_CMD_GET_BOOT_VERS = 0x05
BOOT_CMD_REBOOT = 0x80

CFG_MEM_CRC8_OFF = 0x00
CFG_MEM_WING_CFG_OFF = 0x04
CFG_MEM_INP_CFG_OFF = 0x10
CFG_MEM_SOL_CFG_OFF = 0x30
CFG_MEM_NEO_CFG_OFF_NO_SOL = 0x30
CFG_MEM_NEO_CFG_OFF_WITH_SOL = 0x60

FIRST_SERVO_POS = 8
INP_WING_MAX_NUM_SERVO = 8
SOL_WING_MAX_NUM_SERVO = 4

CRC8ByteLookup = \
  [ 0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d, \
    0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d, \
    0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd, \
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd, \
    0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea, \
    0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a, \
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a, \
    0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a, \
    0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4, \
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4, \
    0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44, \
    0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34, \
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63, \
    0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, \
    0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83, \
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3 ]

#calculate a crc8
def calcCrc8(msgInts):
  crc8Byte = 0xff
  for indInt in msgInts:
    crc8Byte = CRC8ByteLookup[crc8Byte ^ indInt];
  return (crc8Byte)

## Main
#
# Read passed arguments
#
# @param  argv     [in] Passed in arguments
# @return None
def main(argv=None):
  end = False
  portName = "/dev/ttyACM0"
  inHexName = ""
  outHexName = ""
  inCfgName = ""

  if argv is None:
    argv = sys.argv
  for arg in argv:
    if arg.startswith('-?'):
      end = True
    elif arg.startswith('-port='):
      # Remove "-port=" from front
      portName = arg.replace('-port=','',1)
    elif arg.startswith('-inHex='):
      # Remove "-inHex=" from front
      inHexName = arg.replace('-inHex=','',1)
    elif arg.startswith('-outHex='):
      # Remove "-outHex=" from front
      outHexName = arg.replace('-outHex=','',1)
    elif arg.startswith('-inCfg='):
      # Remove "-inCfg=" from front
      inCfgName = arg.replace('-inCfg=','',1)

  if ('COM' in portName) and (not (platform.system() == 'Windows')):
    print ("Windows detected and serial port does not contain COM.  ({})".format(portName))
    print ("This is probably wrong.\n")
    end = True
  if ((inHexName == "") and (inCfgName == "") and (platform.system() != 'Windows') and (os.geteuid() != 0)):
    print ("Linux detected, need to jump to bootloader, and user doesn't seem to have sudo priveleges.")
    print ("Please rerun script with sudo.\n")
    end = True
  if (inHexName != "") and (not os.path.isfile(inHexName)):
    print ("Input Hex File - {} does not exist".format(inHexName))
    return 1
  if (inCfgName != "") and (not os.path.isfile(inCfgName)):
    print ("Input Cfg File - {} does not exist".format(inCfgName))
    return 1
  if (inCfgName != "") and (inHexName != ""):
    print ("Which input do you want me to use?  The cfgFile or the hexFile?")
    return 1

  if (end):
    print ("python showMeTheCfg.py [OPTIONS]")
    print ("       -?  Options help")
    print ("       -port=portName      serial port, defaults to /dev/ttyACM0")
    print ("       -inHex=inpHexName   input hexadecimal file created by script to avoid re-reading each time")
    print ("       -outHex=outHexName  output hexadecimal that can be used as inHex file")
    print ("       -inCfg=inpCfgName   input configuration file used for simple configuration checking")
    return 0

  bootBusNumLst = []
  if (inHexName == "") and (inCfgName == "") and (platform.system() != 'Windows'):
    # Get the USB port number (needed for the unbind/rebind for reconnecting to bootloader
    #  Note:  Finding all USB buses containing vendorId/productId because correlating
    #    the serial port to a USB bus would require another installed library
    print ("Locating USB bus number")
    bootFoundBusNum = False
    busses = usb.busses()
    for bus in busses:
      devices = bus.devices
      for dev in devices:
        if (dev.dev.idVendor == 0x0483) and (dev.dev.idProduct == 0x5740):
          bootBusNumLst.append("usb{}".format(bus.location))
          bootFoundBusNum = True
    if not bootFoundBusNum:
      print ("Could not locate USB bus number")
      return 1

    # Remove duplicates if they exist
    bootBusNumLst = list(set(bootBusNumLst))

  if (inHexName == "") and (inCfgName == ""):
    currFirm = ""
    # Open the serial port
    try:
      ser=serial.Serial(portName, baudrate=115200, bytesize=serial.EIGHTBITS, \
        parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=.1)
    except serial.SerialException:
      print ("Could not open app serial port: {}".format(portName))
      return 1

    # Send get version command to see current app version, this also
    # synchronizes the serial port if it contains data from previous commands
    ser.read(100)
    cmdArr = [BOARD_ADDR, GET_VERS_CMD, 0x00, 0x00, 0x00, 0x00]
    cmdArr.append(calcCrc8(cmdArr))
    cmdArr.append(EOM_CMD)
    numTries = 0
    commsGood = False
    print ("Synchronizing with app serial port: {}".format(portName))
    while ((numTries < 10) and (not commsGood)):
      ser.write(cmdArr)
      resp = b'\x00'
      try:
        resp = ser.read(100)
      except serial.SerialException:
        # Second read from a newly opened serial port can throw a
        # SerialException: device reports readiness to read but returned no data
        #   (device disconnected or multiple access on port?)
        # Ignore the exception
        print ("Ignoring thrown exception")
      respInt = struct.unpack('{}B'.format(len(resp)), resp)
      if ((len(respInt) >= 8) and (respInt[0] == BOARD_ADDR) and (respInt[1] == GET_VERS_CMD) and (respInt[7] == EOM_CMD)):
        tmpData = [ respInt[0], respInt[1], respInt[2], respInt[3], respInt[4], respInt[5] ]
        crc8 = calcCrc8(tmpData)
        if (respInt[6] == crc8):
          currFirm = "{0}.{1}.{2}.{3}".format(respInt[2], respInt[3], respInt[4], respInt[5])
          commsGood = True
      if (not commsGood):
        numTries += 1
        sys.stdout.write(".")
        sys.stdout.flush()
        time.sleep(.5)
    if (not commsGood):
      print ("\nCould not synchronize with app serial port")
      ser.close()
      return 1
    print ("\nFirmware is currently version: {}".format(currFirm))

    # Transition to bootloader
    print ("Transitioning to bootloader")
    cmdArr = [BOARD_ADDR, GO_BOOT_CMD]
    cmdArr.append(calcCrc8(cmdArr))
    ser.write(cmdArr)
    ser.close()
    time.sleep(1)

    # Unbind the USB hub port
    print ("Unbinding USB port")
    if (platform.system() != 'Windows'):
      for usbBusNum in bootBusNumLst:
        try:
          with open("/sys/bus/usb/drivers/usb/unbind", 'w') as unbindFile:
            unbindFile.write(usbBusNum)
        except IOError:
          print ("Could not open unbind file")
          return 1
      
      time.sleep(1)

      # Bind the USB hub port
      print ("Binding USB port")
      for usbBusNum in bootBusNumLst:
        try:
          with open("/sys/bus/usb/drivers/usb/bind", 'w') as bindFile:
            bindFile.write(usbBusNum)
        except IOError:
          print ("Could not open bind file")
          return 1
    else:
      # Windows unbind/rebind using pnputil.exe
      print ("Finding USB parent device id")
      outputStr = subprocess.check_output(['pnputil.exe', '/enum-devices', '/connected', \
        '/class', 'ports', '/relations']).decode('UTF-8')
      outputStr = outputStr.replace('\r', '')
      outputLines = outputStr.split('\n')
      parentDeviceId = ""
      for line in outputLines:
        if line.startswith("Device Description:"):
          # Check if correct COM port
          if ("(" + portName + ")") in line:
            foundDevice = True
          else:
            foundDevice = False
        elif line.startswith("Parent:") and foundDevice:
          # Store parent device id
          parentDeviceId = line.strip().split()[1]
      if (parentDeviceId == ""):
        print ("Could not find parent device Id")
        return 1
      print ("Restarting USB parent")
      outputStr = subprocess.check_output(['pnputil.exe', '/restart-device', parentDeviceId])
      time.sleep(3)
      print ("Finding redescribed port number")
      outputStr = subprocess.check_output(['pnputil.exe', '/enum-devices', '/connected', \
        '/class', 'ports', '/relations']).decode('UTF-8')
      outputStr = outputStr.replace('\r', '')
      outputLines = outputStr.split('\n')
      currDeviceDesc = ""
      for line in outputLines:
        if line.startswith("Device Description:"):
          currDeviceDesc = line.strip()
        elif line.startswith("Parent:"):
          if (line.strip().split()[1] == parentDeviceId):
            # Find the new COM port by extracting between parenthesis
            portName = currDeviceDesc.split('(')[1].split(')')[0]
            print ("Found new port name {}".format(portName))          
  
    time.sleep(5)

    # Opening boot serial port 
    print ("Opening bootloader serial port")
    try:
      ser=serial.Serial(portName, baudrate=115200, bytesize=serial.EIGHTBITS, \
        parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=1)
    except serial.SerialException:
      print ("Could not open bootloader serial port: {}".format(portName))
      return 1

    # Send get boot version command to synch serial port
    # linux likes to send v.25bis commands on new ports
    numTries = 0
    commsGood = False
    goodResp = 0
    while ((numTries < 30) and (not commsGood)):
      ser.write([BOOT_STX, BOOT_CMD_GET_BOOT_VERS])
      resp = b'\x00'
      try:
        resp = ser.read(32)
      except serial.SerialException:
        # Second read from a newly opened serial port can throw a
        # SerialException: device reports readiness to read but returned no data
        #   (device disconnected or multiple access on port?)
        # Ignore the exception
        print ("Ignoring thrown exception")
      if ((len(resp) == 8) and (resp == b"Boot0.0\r")):
        goodResp += 1
        if (goodResp > 3):
          commsGood = True
      else:
        goodResp = 0
        numTries += 1
        sys.stdout.write(".")
        sys.stdout.flush()
        time.sleep(.5)
    if (not commsGood):
      print ("\nCould not synchronize with bootloader serial port")
      ser.close()
      return 1

  # Allocate maximum memory for allocation
  cfgMem = bytearray(b'\xff') * CFG_MEM_LEN

  # If reading actual board
  if (inHexName == "") and (inCfgName == ""):
    print ("\nReading configuration sector from board...")
    for addr in range(CFG_MEM_ADDR, CFG_MEM_ADDR + CFG_MEM_LEN, 8):
      if ((addr & 0xff) == 0):
        sys.stdout.write(".")
        sys.stdout.flush()      
      cmdArr = [BOOT_STX, BOOT_CMD_READ_PROG_MEM, \
        (addr >> 24) & 0xff, (addr >> 16) & 0xff, \
        (addr >> 8) & 0xff, addr & 0xff]
      ser.write(cmdArr)
      resp = ser.read(15)
      respInt = struct.unpack('{}B'.format(len(resp)), resp)
      if (len(respInt) != 15) or (respInt[0] != BOOT_STX) or (respInt[1] != BOOT_CMD_READ_PROG_MEM) or \
        (respInt[2] != (addr >> 24) & 0xff) or \
        (respInt[3] != (addr >> 16) & 0xff) or \
        (respInt[4] != (addr >> 8) & 0xff) or \
        (respInt[5] != (addr & 0xff)):
        print ("Received bad read memory response: \"{}\"" \
          .format(''.join(struct.pack('B', byte).hex() for byte in respInt)))
        ser.close()
        return 1
      crc8 = calcCrc8(respInt[1:-1])
      if (respInt[-1] != crc8):
        print ("Received bad read memory CRC: \"{}\"" \
          .format(''.join(struct.pack('B', byte).hex() for byte in respInt)))
        ser.close()
        return 1
      for index in range(8):
        # Big endian order to make everyone's life easier
        cfgMem[addr - CFG_MEM_ADDR + index] = respInt[(index & 0x04) + 6 + (3 - (index & 0x03))]
  else:
    if (inHexName != ""):
      print ("\nReading configuration sector from {}...".format(inHexName))
      hexFile = open(inHexName, 'rb')
      for index in range(CFG_MEM_LEN):
        cfgMem[index] = ord(hexFile.read(1))
      hexFile.close()
    elif (inCfgName != ""):
      print ("\nLoading configuration file from {}...".format(inCfgName))
      shouldHaveSolCfg = False
      shouldHaveColorCfg = False
      cfgFile = __import__(inCfgName.replace('.py','',1))
      # If necessary convert old char style cfg file to ints
      if (hasattr(cfgFile, 'wingCfg') and not isinstance(cfgFile.wingCfg[0][0], int)):
        for loop in range(rs232BIntf.NUM_G2_WING_PER_BRD):
          cfgFile.wingCfg[0][loop] = ord(cfgFile.wingCfg[0][loop])
      # Copy into cfgMem
      if (hasattr(cfgFile, 'wingCfg')):
        for loop in range(rs232BIntf.NUM_G2_WING_PER_BRD):
          cfgMem[loop + CFG_MEM_WING_CFG_OFF] = cfgFile.wingCfg[0][loop]
          if (cfgMem[loop + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_SOL) or \
                (cfgMem[loop + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_NEO_SOL):
            shouldHaveSolCfg = True
          if (cfgMem[loop + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_NEO) or \
                (cfgMem[loop + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_NEO_SOL):
            shouldHaveColorCfg = True
      else:
        print ("\nConfiguration file does not have a wing configuration.")
        print ("This really makes absolutely no sense.  What does the board do?\n")
      if (hasattr(cfgFile, 'inpCfg') and not isinstance(cfgFile.inpCfg[0][0], int)):
        for loop in range(rs232BIntf.NUM_G2_INP_PER_BRD):
          cfgFile.inpCfg[0][loop] = ord(cfgFile.inpCfg[0][loop])
      if (hasattr(cfgFile, 'inpCfg')):
        for loop in range(rs232BIntf.NUM_G2_INP_PER_BRD):
          cfgMem[loop + CFG_MEM_INP_CFG_OFF] = cfgFile.inpCfg[0][loop]
      else:
        print ("\nConfiguration file does not have an input configuration.")
        print ("All OPP board require an inpCfg.\n")
      if (hasattr(cfgFile, 'solCfg') and not isinstance(cfgFile.solCfg[0][0], int)):
        for loop in range(rs232BIntf.NUM_G2_SOL_PER_BRD * rs232BIntf.CFG_BYTES_PER_SOL):
          cfgFile.solCfg[0][loop] = ord(cfgFile.solCfg[0][loop])
      if (hasattr(cfgFile, 'solCfg')):
        if (not shouldHaveSolCfg):
          print ("\nConfiguration file has solenoid configuration, but no wings need it.")
          print ("The solCfg is ignored and not sent to board.\n")
        else:
          for loop in range(rs232BIntf.NUM_G2_SOL_PER_BRD * rs232BIntf.CFG_BYTES_PER_SOL):
            cfgMem[loop + CFG_MEM_SOL_CFG_OFF] = cfgFile.solCfg[0][loop]
      if (hasattr(cfgFile, 'colorCfg') and not isinstance(cfgFile.colorCfg[0][0], int)):
        for loop in range((rs232BIntf.NUM_COLOR_TBL * 3) + 1):
          cfgFile.colorCfg[0][loop] = ord(cfgFile.colorCfg[0][loop])
      if (hasattr(cfgFile, 'colorCfg')):
        if (not shouldHaveColorCfg):
          print ("\nConfiguration file has color configuration, but no wings need it.")
          print ("The colorCfg is sent to the board even if not needed.\n")
          if (shouldHaveSolCfg):
            cfgMem[loop + CFG_MEM_NEO_CFG_OFF_WITH_SOL] = cfgFile.solCfg[0][loop]
          else:
            cfgMem[loop + CFG_MEM_NEO_CFG_OFF_NO_SOL] = cfgFile.solCfg[0][loop]

  if (inHexName == "") and (inCfgName == ""):
    # Go back to the application to clean up after ourselves
    print ("\nSending reboot command to go back to application")
    ser.write([BOOT_STX, BOOT_CMD_REBOOT])
    ser.close()
    time.sleep(1)

    # Unbind the USB hub port
    print ("Unbinding USB port")
    if (platform.system() != 'Windows'):
      for usbBusNum in bootBusNumLst:
        try:
          with open("/sys/bus/usb/drivers/usb/unbind", 'w') as unbindFile:
            unbindFile.write(usbBusNum)
        except IOError:
          print ("Could not open unbind file")
          return 1
    
      time.sleep(1)

      # Bind the USB hub port
      print ("Binding USB port")
      for usbBusNum in bootBusNumLst:
        try:
          with open("/sys/bus/usb/drivers/usb/bind", 'w') as bindFile:
            bindFile.write(usbBusNum)
        except IOError:
          print ("Could not open bind file")
          return 1
    else:
      # Windows unbind/rebind using pnputil.exe
      print ("Restarting USB parent")
      outputStr = subprocess.check_output(['pnputil.exe', '/restart-device', parentDeviceId])
      time.sleep(3)
      print ("Finding redescribed port number")
      outputStr = subprocess.check_output(['pnputil.exe', '/enum-devices', '/connected', \
        '/class', 'ports', '/relations']).decode('UTF-8')
      outputStr = outputStr.replace('\r', '')
      outputLines = outputStr.split('\n')
      currDeviceDesc = ""
      for line in outputLines:
        if line.startswith("Device Description:"):
          currDeviceDesc = line.strip()
        elif line.startswith("Parent:"):
          if (line.strip().split()[1] == parentDeviceId):
            # Find the new COM port by extracting between parenthesis
            portName = currDeviceDesc.split('(')[1].split(')')[0]
            print ("Found new port name {}".format(portName))          

    time.sleep(5)

    # Opening application serial port 
    print ("Opening application serial port")
    try:
      ser=serial.Serial(portName, baudrate=115200, bytesize=serial.EIGHTBITS, \
        parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=1)
    except serial.SerialException:
      print ("Could not open app serial port: {}".format(portName))
      return 1

    # Reconnect with application
    # Send get version command to see current app version, this also
    # synchronizes the serial port if it contains data from previous commands
    resp = ser.read(100)
    cmdArr = [BOARD_ADDR, GET_VERS_CMD, 0x00, 0x00, 0x00, 0x00]
    cmdArr.append(calcCrc8(cmdArr))
    cmdArr.append(EOM_CMD)
    numTries = 0
    commsGood = False
    print ("Synchronizing with app serial port: {}".format(portName))
    while ((numTries < 10) and (not commsGood)):
      ser.write(cmdArr)
      resp = b'\x00'
      try:
        resp = ser.read(100)
      except serial.SerialException:
        # Second read from a newly opened serial port can throw a
        # SerialException: device reports readiness to read but returned no data
        #   (device disconnected or multiple access on port?)
        # Ignore the exception
        print ("Ignoring thrown exception")
      respInt = struct.unpack('{}B'.format(len(resp)), resp)
      if ((len(respInt) == 8) and (respInt[0] == BOARD_ADDR) and (respInt[1] == GET_VERS_CMD) and (respInt[7] == EOM_CMD)):
        tmpData = [ respInt[0], respInt[1], respInt[2], respInt[3], respInt[4], respInt[5] ]
        crc8 = calcCrc8(tmpData)
        if (respInt[6] == crc8):
          commsGood = True
      if (not commsGood):
        numTries += 1
        sys.stdout.write(".")
        sys.stdout.flush()
        time.sleep(.5)
    if (not commsGood):
      print ("\nCould not synchronize with app serial port")
      ser.close()
      return 1
    ser.close()
    print ("Reboot complete - analyzing configuration")

  if (outHexName != ""):
    print ("\nWriting {}...".format(outHexName))
    hexFile = open(outHexName, 'wb')
    for index in range(CFG_MEM_LEN):
       hexFile.write(struct.pack('{}B'.format(1), cfgMem[index]))
    hexFile.close()

  inpPinBitMask = 0
  solPinBitMask = 0
  solBitMask = 0
  servoPinBitMask = 0
  lampMtrxRowPinMask = 0
  lampMtrxColPinMask = 0
  swMtrxOutPinMask = 0
  swMtrxInPinMask = 0
  spiClkBitMask = 0
  neoBitMask = 0
  incandMask = 0
  error = False
  if (inCfgName == ""):
    print ("\n\nNV Cfg Mem CRC8 = {}".format("0x%02x" % cfgMem[CFG_MEM_CRC8_OFF]))
  for index in range(rs232BIntf.NUM_G2_WING_PER_BRD):
    outStr = "W[%d]:" % index
    if cfgMem[index + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_SOL:
      outStr += "SOL_WING"
      inpPinBitMask |= (0x0f << (index * 8))
      solPinBitMask |= (0xf0 << (index * 8))
      solBitMask |= (0xf << (index * 4))
    elif cfgMem[index + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_INP:
      outStr += "INP_WING"
      inpPinBitMask |= 0xff << (index * 8)
    elif cfgMem[index + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_INCAND:
      outStr += "INCAND_WING"
      incandMask |= 0xff << (index * 8)
    elif cfgMem[index + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_SW_MATRIX_OUT:
      if (index == 2):
        swMtrxOutPinMask = 0x00ff0000
        outStr += "SW_MATRIX_OUT_WING"
      else:
        print ("!!! Illegal Config !!! Switch matrix output wings must be wing 2.")
        error = True
      if (cfgMem[3 + CFG_MEM_WING_CFG_OFF] != rs232BIntf.WING_SW_MATRIX_IN):
        print ("!!! Illegal Config !!! Switch matrix output wings need switch matrix inputs as wing 3.")
        error = True
    elif cfgMem[index + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_SW_MATRIX_IN:
      if (index == 3):
        outStr += "SW_MATRIX_IN_WING"
        swMtrxInPinMask = 0xff000000
      else:
        print ("!!! Illegal Config !!! Switch matrix input wings must be wing 3.")
        error = True
      if (cfgMem[2 + CFG_MEM_WING_CFG_OFF] != rs232BIntf.WING_SW_MATRIX_OUT) and \
         (cfgMem[2 + CFG_MEM_WING_CFG_OFF] != rs232BIntf.WING_SW_MATRIX_OUT_LOW):
        print ("!!! Illegal Config !!! Switch matrix input wings need switch matrix outputs as wing 2.")
        error = True
    elif cfgMem[index + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_NEO:
      if (index == 0):
        outStr += "NEO_WING"
        inpPinBitMask |= 0x000000ef
        neoBitMask |= 0x00000010
      else:
        print ("!!! Illegal Config !!! Neo wings can only be wing 0.")
        error = True
    elif cfgMem[index + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_HI_SIDE_INCAND:
      outStr += "INCAND_HI_WING"
    elif cfgMem[index + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_NEO_SOL:
      if (index == 0):
        outStr += "NEO_SOL_WING"
        inpPinBitMask |= 0x0000000e
        neoBitMask |= 0x00000010

        #  Note:  the first solenoid output is relocated to bit 0 of the port
        #  since Neopixels need to be at bit 4 of the wing
        solPinBitMask |= (0xe1 << (index * 8))
        solBitMask |= (0xf << (index * 4))
      else:
        print ("!!! Illegal Config !!! Neo sol wing can only be wing 0.")
        error = True
    elif cfgMem[index + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_SPI:
      outStr += "SPI_WING"
      print ("!!! Illegal Config !!! SPI wing was written and tested, yet never enabled.")
      print ("No support for OPP SPI in MPF so it was abandoned")
      error = True
    elif cfgMem[index + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_SW_MATRIX_OUT_LOW:
      if (index == 2):
        swMtrxOutPinMask = 0x00ff0000
        outStr += "SW_MATRIX_OUT_LOW_WING"
      else:
        print ("!!! Illegal Config !!! Switch matrix output low wings must be wing 2.")
        error = True
      if (cfgMem[3 + CFG_MEM_WING_CFG_OFF] != rs232BIntf.WING_SW_MATRIX_IN):
        print ("!!! Illegal Config !!! Switch matrix output wings need switch matrix inputs as wing 3.")
        error = True
    elif cfgMem[index + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_LAMP_MATRIX_COL:
      if (index == 0):
        outStr += "LAMP_MATRIX_COL_WING"
        lampMtrxColPinMask = 0x000000ff
      else:
        print ("!!! Illegal Config !!! Lamp matrix column must be wing 0.")
        error = True
    elif cfgMem[index + CFG_MEM_WING_CFG_OFF] == rs232BIntf.WING_LAMP_MATRIX_ROW:
      if (index == 1):
        outStr += "LAMP_MATRIX_ROW_WING"
        lampMtrxRowPinMask = 0x0000ff00
      else:
        print ("!!! Illegal Config !!! Lamp matrix row must be wing 1.")
        error = True
    else:
      outStr += "Error"
      print ("!!! Illegal Config !!! Don't understand wing type = {}".format(cfgMem[index + CFG_MEM_WING_CFG_OFF]))
      error = True
    if index < rs232BIntf.NUM_G2_WING_PER_BRD - 1:
      outStr += ","
      print (outStr, end=" ")
    else:
      print (outStr)
  if (error):
    return 1

  # Check for servo configurations (can only be wing 1)
  if (cfgMem[CFG_MEM_WING_CFG_OFF + 1] == rs232BIntf.WING_INP):
    for index in range(INP_WING_MAX_NUM_SERVO):
      if (cfgMem[index + CFG_MEM_INP_CFG_OFF + FIRST_SERVO_POS] >= 64):
        inpPinBitMask &= ~(1 << (FIRST_SERVO_POS + index))
        servoPinBitMask |= (1 << (FIRST_SERVO_POS + index))
  if (cfgMem[CFG_MEM_WING_CFG_OFF + 1] == rs232BIntf.WING_SOL):
    for index in range(SOL_WING_MAX_NUM_SERVO):
      if (cfgMem[index + CFG_MEM_INP_CFG_OFF + FIRST_SERVO_POS] >= 64):
        inpPinBitMask &= ~(1 << (FIRST_SERVO_POS + index))
        servoPinBitMask |= (1 << (FIRST_SERVO_POS + index))

  # Find location of Neopixel config if it exist
  CFG_MEM_NEO_CFG_OFF = 0x30
  if (neoBitMask):
    if (solPinBitMask != 0):
      CFG_MEM_NEO_CFG_OFF = 0x60

  # Check if configured for SPI pixels
  pinCfgDict = {}
  if (neoBitMask) and (cfgMem[CFG_MEM_NEO_CFG_OFF + 1] == 3) and (cfgMem[CFG_MEM_NEO_CFG_OFF + 5] == 0xa5):
    # SPI pixels use PB13 as the clock output
    inpPinBitMask &= ~(0x00000004)
    spiClkBitMask |= 0x00000004

  for index in range(rs232BIntf.NUM_G2_INP_PER_BRD):
    pinCfgStr = ""
    if ((1 << index) & inpPinBitMask != 0):
      if (cfgMem[index + CFG_MEM_INP_CFG_OFF] == rs232BIntf.CFG_INP_STATE):
        pinCfgStr += "    Input state pin\n"
      else:
        pinCfgStr += "!!! Illegal Config !!! Input pin configuration should only be CFG_INP_STATE\n"
    if ((1 << index) & solPinBitMask != 0):
      # Special case for the NeoSol board which moves first sol output to bit 0
      neoSolSpecialCase = False
      if (index == 0):
        solCfgIndex = 0
        neoSolSpecialCase = True
      else:
        solCfgIndex = ((index >> 3) * 4) + (index & 0x3)
      solCfgMemIndex = CFG_MEM_SOL_CFG_OFF + (solCfgIndex * rs232BIntf.CFG_BYTES_PER_SOL)
      pinCfgStr += "    Solenoid pin - initial kick = %dms\n" % cfgMem[solCfgMemIndex + 1]
      if (cfgMem[solCfgMemIndex] == rs232BIntf.CFG_SOL_DISABLE):
        pinCfgStr += "        Disabled in white wood mode\n"
      else:
        outStr = "        Config bits set: "
        if ((cfgMem[solCfgMemIndex] & rs232BIntf.CFG_SOL_USE_SWITCH) != 0):
          outStr += "CFG_SOL_USE_SWITCH | "
          if (neoSolSpecialCase):
            pinCfgStr += "!!! Illegal Config !!! NeoSol wing first solenoid does not have an input switch available\n"
            error = True
        if ((cfgMem[solCfgMemIndex] & rs232BIntf.CFG_SOL_AUTO_CLR) != 0):
          outStr += "CFG_SOL_AUTO_CLR | "
          if ((cfgMem[solCfgMemIndex] & rs232BIntf.CFG_SOL_ON_OFF) != 0):
            pinCfgStr += "!!! Questionable Config !!! Solenoid is on/off and auto clear is set.\n"
            pinCfgStr += "    Host won't be able to properly control this solenoid (but you might not care)\n"
            error = True
        if ((cfgMem[solCfgMemIndex] & rs232BIntf.CFG_SOL_ON_OFF) != 0):
          outStr += "CFG_SOL_ON_OFF | "
        if ((cfgMem[solCfgMemIndex] & rs232BIntf.CFG_SOL_DLY_KICK) != 0):
          outStr += "CFG_SOL_DLY_KICK | "
          pinCfgStr += "        Delay kick time set to %dms\n" % ((cfgMem[solCfgMemIndex + 2] & 0x0f) * 2)
        if ((cfgMem[solCfgMemIndex] & rs232BIntf.CFG_SOL_USE_MTRX_INP) != 0):
          outStr += "CFG_SOL_USE_MTRX_INP | "
          if (swMtrxInPinMask != 0):
            pinCfgStr += "        Solenoid input is switch matrix input 0x%02x\n" % cfgMem[solCfgMemIndex + 2]
          else:
            pinCfgStr += "!!! Illegal Config !!! Solenoid is configured to use matrix input, but no input matrix wings\n"
            error = True
        if ((cfgMem[solCfgMemIndex] & rs232BIntf.CFG_SOL_CAN_CANCEL) != 0):
          outStr += "CFG_SOL_CAN_CANCEL | "
        pinCfgStr += (outStr[:-3] + "\n")

        # If this is a normal solenoid, print out delay and timeouts
        if ((cfgMem[solCfgMemIndex] & (rs232BIntf.CFG_SOL_ON_OFF | \
            rs232BIntf.CFG_SOL_DLY_KICK | rs232BIntf.CFG_SOL_USE_MTRX_INP) == 0)):
          if ((cfgMem[solCfgMemIndex + 2] & 0x0f) != 0):
            pinCfgStr += "        PWM for solenoid hold set to %.2f%%\n" % \
                ((float(cfgMem[solCfgMemIndex + 2] & 0x0f) * 100.0)/16.0)
          if ((cfgMem[solCfgMemIndex + 2] & 0x70) != 0):
            pinCfgStr += "        Minimum off time set to %dms\n" % \
                (((cfgMem[solCfgMemIndex + 2] & 0x70) >> 4) * cfgMem[solCfgMemIndex + 1])
    if ((1 << index) & servoPinBitMask != 0):
      servoCfg = cfgMem[index + CFG_MEM_INP_CFG_OFF]
      servoOutput = float(servoCfg)/100.0
      pinCfgStr += "    Servo pin - initial pulse set to = %.2fms\n" % servoCfg
    if ((1 << index) & lampMtrxRowPinMask != 0):
      pinCfgStr += "    Lamp matrix row pin\n"
    if ((1 << index) & lampMtrxColPinMask != 0):
      pinCfgStr += "    Lamp matrix column pin\n"
    if ((1 << index) & swMtrxOutPinMask != 0):
      pinCfgStr += "    Switch matrix strobe pin\n"
    if ((1 << index) & swMtrxInPinMask != 0):
      pinCfgStr += "    Switch matrix strobe pin\n"
    if ((1 << index) & spiClkBitMask != 0):
      pinCfgStr += "    SPI pixel clock out pin\n"
    if ((1 << index) & neoBitMask != 0):
      pinCfgStr += "    Pixel data output pin\n"
    if ((1 << index) & incandMask != 0):
      pinCfgStr += "    Incandescent output pin\n"
    if pinCfgStr in pinCfgDict:
      pinCfgDict[pinCfgStr].append(index)
    else:
      pinCfgDict[pinCfgStr] = [index]

  for index in range(rs232BIntf.NUM_G2_INP_PER_BRD):
    for pinCfgStr, pinLst in pinCfgDict.items():
      if (index == pinLst[0]):
        pinLstStr = ""
        for pin in pinLst:
          pinLstStr += "%d, " % pin
        print ("Pin[{}]:".format(pinLstStr[:-2]))
        print (pinCfgStr)


  # Print color table information if it exists
  if (neoBitMask):
    bytesPerPixel = cfgMem[CFG_MEM_NEO_CFG_OFF]
    if (bytesPerPixel == 0x00) or (bytesPerPixel == 0xff):
      bytesPerPixel = 3
    print ("\nLED information:")
    if (spiClkBitMask == 0):
      print ("    Neopixel style LEDs configured (ws2812 and such)")
    else:
      print ("    SPI style LEDs configured (SK9822 and APA102)")      
    print ("    Bytes per pixel = {}".format(bytesPerPixel))
    numPixels = cfgMem[CFG_MEM_NEO_CFG_OFF + 1]
    if (numPixels == 0xff):
       numPixels = 256
    print ("    {} Pixels are configured".format(numPixels))
    dfltColor = True
    pixelColorStr = "" 
    for index in range(bytesPerPixel):
      pixelColorStr += "0x%02x " % cfgMem[CFG_MEM_NEO_CFG_OFF + index + 2]
      if (cfgMem[CFG_MEM_NEO_CFG_OFF + index + 2] != 0xff):
        dfltColor = False
    if (not dfltColor):
      print ("    Initially each pixel value is {}".format(pixelColorStr))
    else:
      if (bytesPerPixel == 3):
        print ("    Default initial colors such as green, red, blue, green, red, ...")
      else:
        print ("    Default initial colors such as green, red, blue, white, green, red, ...")
      
  return 0

if __name__ == "__main__":
  sys.exit(main())

