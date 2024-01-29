#!/usr/bin/env python
import sys
import os
import struct
import serial
import time
import usb

IHEX_DATA = 0x00
IHEX_END = 0x01
IHEX_EXT_ADDR = 0x04
IHEX_START_ADDR = 0x05

APP_START_ADDR = 0x08001000

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
def calcCrc8Chars(msgChars):
  crc8Byte = 0xff
  for indChar in msgChars:
    indInt = ord(indChar)
    crc8Byte = CRC8ByteLookup[crc8Byte ^ indInt];
  return (chr(crc8Byte))

def calcCrc8Ints(msgInts):
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
  hexFileName = ""
  portName = "/dev/ttyACM0"
  inBoot = False

  if argv is None:
    argv = sys.argv
  for arg in argv:
    if arg.startswith('-?'):
      end = True
    elif arg.startswith('-hex='):
      # Remove "-hex=" from front
      hexFileName = arg.replace('-hex=','',1)
    elif arg.startswith('-port='):
      # Remove "-port=" from front
      portName = arg.replace('-port=','',1)
    elif arg.startswith('-boot'):
      inBoot = True

  if (end or (hexFileName == "")):
    print ("python updApp.py [OPTIONS]")
    print ("       -?  Options help")
    print ("       -hex=hexFileName  Input intel hex file name (probably ends in .hex)")
    print ("       -port=portName    COM port number, defaults to /dev/ttyACM0")
    print ("       -boot             Board is in bootloader, so don't switch to boot from app")
    return 0

  if (not os.path.isfile(hexFileName)):
    print ("HexFile - {} does not exist".format(hexFileName))
    return 1

  # Get the USB port number (needed for the unbind/rebind for reconnecting to bootloader
  #  Note:  Finding all USB buses containing vendorId/productId because correlating
  #    the serial port to a USB bus would require another installed library
  print ("Locating USB bus number")
  bootFoundBusNum = False
  bootBusNumLst = []
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


  currFirm = ""
  if (not inBoot):
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
    cmdArr.append(calcCrc8Ints(cmdArr))
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
        # SerialException: device reports readiness to read but returned no data (device disconnected or multiple access on port?)
        # Ignore the exception
        print ("Ignoring thrown exception")
      respInt = struct.unpack('{}B'.format(len(resp)), resp)
      if ((len(respInt) >= 8) and (respInt[0] == BOARD_ADDR) and (respInt[1] == GET_VERS_CMD) and (respInt[7] == EOM_CMD)):
        tmpData = [ respInt[0], respInt[1], respInt[2], respInt[3], respInt[4], respInt[5] ]
        crc8 = calcCrc8Ints(tmpData)
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

  # Allocate maximum memory for allocation
  appMem = bytearray(b'\xff') * 0xf000
  maxAddr = 0

  print ("\nProcessing {}...".format(hexFileName))
  hexfile = open(hexFileName, 'r')
  lines = hexfile.readlines()
  hexfile.close()
  lineNum = 0
  extAddr = 0
  maxOff = 0
  for line in lines:
    lineNum += 1
    # ignore lines that don't start with ':'
    if line[0] == ':':
      # Convert hex digits to bytearray
      byteArr = bytearray.fromhex(line[1:].rstrip())
      hexLen = byteArr[0]
      hexAddr = (byteArr[1] << 8) | byteArr[2]
      hexCmd = byteArr[3]
      
      # Verify checksum
      hexSum = (~(sum(byteArr[:-1]) & 0xff) + 1) & 0xff
      if (hexSum != byteArr[-1]):
        print ("Bad CRC on line {0}.  File has {1}, but calculated {2}.  Line = \"{3}\"" \
          .format(lineNum, "0x%02x" % byteArr[-1], "0x%02x" % hexSum, line))
        if (not inBoot):
          ser.close()
        return 1

      # Process ihex commands
      if (hexCmd == IHEX_EXT_ADDR):
        extAddr = (byteArr[4] << 24) | (byteArr[5] << 16)
      elif (hexCmd == IHEX_DATA):
        currOff = (extAddr | (byteArr[1] << 8) | byteArr[2]) - APP_START_ADDR

        # Check if this is application code (bootloader will have negative offsets
        if (currOff >= 0):
          if (maxOff < currOff + hexLen):
            maxOff = currOff + hexLen
          for x in range(hexLen):
            appMem[currOff + x] = byteArr[x + 4]
      elif ((hexCmd == IHEX_END) or (hexCmd == IHEX_START_ADDR)):
        continue
      else:
        print ("Unknown ihex cmd on line {0} of {1}.  Line = \"{2}\"" \
          .format(lineNum, "0x%02x" % hexCmd, line))
        if (not inBoot):
          ser.close()
        return 1

  # Extract new app firmware version
  newFirm = "{0}.{1}.{2}.{3}".format(appMem[35], appMem[34], appMem[33], appMem[32])
  print ("New firmware version: {}".format(newFirm))

  if (not inBoot):
    # Transition to bootloader
    print ("Transitioning to bootloader")
    cmdArr = [BOARD_ADDR, GO_BOOT_CMD]
    cmdArr.append(calcCrc8Ints(cmdArr))
    ser.write(cmdArr)
    ser.close()
    time.sleep(1)

    # Unbind the USB hub port
    print ("Unbinding USB port")
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

    time.sleep(5)

  # Opening boot serial port 
  print ("Opening bootloader serial port")
  try:
    ser=serial.Serial(portName, baudrate=115200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=1)
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
      # SerialException: device reports readiness to read but returned no data (device disconnected or multiple access on port?)
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

  # Update the application
  print ("\nUpdating application")
  for x in range(0, maxOff, 8):
    # Print a '.' when starting to write a flash sector
    if ((x & 0x3ff) == 0):
      sys.stdout.write(".")
      sys.stdout.flush()
    cmdArr = [BOOT_STX, BOOT_CMD_WRITE_PROG_MEM, \
      ((APP_START_ADDR + x) >> 24) & 0xff, ((APP_START_ADDR + x) >> 16) & 0xff, \
      ((APP_START_ADDR + x) >> 8) & 0xff, (APP_START_ADDR + x) & 0xff, \
      appMem[x + 3], appMem[x + 2], appMem[x + 1], appMem[x], \
      appMem[x + 7], appMem[x + 6], appMem[x + 5], appMem[x + 4]]
    cmdArr.append(calcCrc8Ints(cmdArr[1:]))
    ser.write(cmdArr)
    resp = ser.read(6)
    respInt = struct.unpack('{}B'.format(len(resp)), resp)
    if (len(respInt) != 6) or (respInt[0] != BOOT_STX) or (respInt[1] != BOOT_CMD_WRITE_PROG_MEM) or \
      (respInt[2] != ((APP_START_ADDR + x) >> 24) & 0xff) or \
      (respInt[3] != ((APP_START_ADDR + x) >> 16) & 0xff) or \
      (respInt[4] != ((APP_START_ADDR + x) >> 8) & 0xff) or \
      (respInt[5] != (APP_START_ADDR + x) & 0xff):
      print ("Received bad write memory response: \"{}\"" \
        .format(''.join(struct.pack('B', byte).hex() for byte in respInt)))
      ser.close()
      return 1

  # Verify application
  print ("\nVerifying application")

  # Send reboot command
  print ("Sending reboot command to boot into updated application")
  ser.write([BOOT_STX, BOOT_CMD_REBOOT])
  ser.close()
  time.sleep(1)

  # Unbind the USB hub port
  print ("Unbinding USB port")
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

  time.sleep(5)

  # Opening boot serial port 
  print ("Opening application serial port")
  try:
    ser=serial.Serial(portName, baudrate=115200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=1)
  except serial.SerialException:
    print ("Could not open app serial port: {}".format(portName))
    return 1

  # Reconnect with application
  # Send get version command to see current app version, this also
  # synchronizes the serial port if it contains data from previous commands
  resp = ser.read(100)
  cmdArr = [BOARD_ADDR, GET_VERS_CMD, 0x00, 0x00, 0x00, 0x00]
  cmdArr.append(calcCrc8Ints(cmdArr))
  cmdArr.append(EOM_CMD)
  numTries = 0
  commsGood = False
  currFirm = ""
  print ("Synchronizing with app serial port: {}".format(portName))
  while ((numTries < 10) and (not commsGood)):
    ser.write(cmdArr)
    resp = b'\x00'
    try:
      resp = ser.read(100)
    except serial.SerialException:
      # Second read from a newly opened serial port can throw a
      # SerialException: device reports readiness to read but returned no data (device disconnected or multiple access on port?)
      # Ignore the exception
      print ("Ignoring thrown exception")
    respInt = struct.unpack('{}B'.format(len(resp)), resp)
    if ((len(respInt) == 8) and (respInt[0] == BOARD_ADDR) and (respInt[1] == GET_VERS_CMD) and (respInt[7] == EOM_CMD)):
      tmpData = [ respInt[0], respInt[1], respInt[2], respInt[3], respInt[4], respInt[5] ]
      crc8 = calcCrc8Ints(tmpData)
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

  # Verify new firmware version
  if (newFirm != currFirm):
    print ("Failure - Expected firmware to be {}, but new application is {}\n".format(newFirm, currFirm))
    ser.close()
    return 1

  ser.close()
  print ("Success - Finished updating {}\n".format(hexFileName))
  return 0

if __name__ == "__main__":
  sys.exit(main())

