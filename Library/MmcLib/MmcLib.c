/** @file
  MMC Library.

  Copyright (c) 2022, ADLink. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/PcdLib.h>
#include <Library/PL011UartLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MmcLib.h>
#include <Library/NVLib.h>

UINT8 CmdBuffer_MmcPostCode[]         = {"[C0 00 80 11]\r\n"};
UINT8 CmdBuffer_MmcSetPowerOffType[]  = {"[C0 00 15 01]\r\n"};
UINT8 CmdBuffer_MmcFirmwareVersion[]  = {"[18 00 01]\r\n"};
UINT8 CmdBuffer_wolCmdBuf_D[]         = {"[C0 00 3C 00]\r\n"};   // cmd to disable WOL
UINT8 CmdBuffer_wolCmdBuf_E[]         = {"[C0 00 3C 01]\r\n"};   // cmd to enable WOL
 
EFI_STATUS
MmcPostCode(
    IN UINT32 Value)
{
  UINTN NumberOfBytes;
  UINT8 IpmiCmdBuf[CMD_BUFFER_SIZE];

  AsciiSPrint((CHAR8 *)IpmiCmdBuf, CMD_BUFFER_SIZE, "[C0 00 80 %2X]\r\n", (UINT8)Value);

  NumberOfBytes = PL011UartWrite((UINTN)PcdGet64(PcdSerialDbgRegisterBase), IpmiCmdBuf, CMD_BUFFER_SIZE);

  if (NumberOfBytes == 0)
  {
    DEBUG((DEBUG_ERROR, "%a Failed to Write MMC POST code data\n", __FUNCTION__));
    return EFI_NO_RESPONSE;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
MmcSetPowerOffType(
    IN UINT8 Value)
{
  UINTN numofbytes;
  UINT8 IpmiCmdBuf[CMD_BUFFER_SIZE];

  AsciiSPrint((CHAR8 *)IpmiCmdBuf, CMD_BUFFER_SIZE, "[C0 00 80 %02X]\r\n", Value);

  DEBUG((DEBUG_INFO, "%a Write MMC Power off type %d\n", __FUNCTION__, Value));
  numofbytes = PL011UartWrite((UINTN)PcdGet64(PcdSerialDbgRegisterBase), IpmiCmdBuf, CMD_BUFFER_SIZE);

  if (numofbytes == 0)
  {
    DEBUG((DEBUG_ERROR, "%a Failed to Write MMC Power off type\n", __FUNCTION__));
    return EFI_INVALID_PARAMETER;
  }
  return EFI_SUCCESS;
}

/**
  Retrieves the MMC firmware version by sending an IPMI command over UART and processing the response.

  This function sends a predefined IPMI command (`[18 00 01]\r\n`) over UART to request the MMC firmware version.
  It then reads the response and extracts the relevant firmware version information into the provided buffer.

  @param[out]  Buffer            Pointer to the buffer where the MMC firmware version will be stored.
  @param[in]   BufferSize        Size of the Buffer in bytes.

  @retval      EFI_SUCCESS       The MMC firmware version was successfully retrieved and stored in Buffer.
  @retval      EFI_UNSUPPORTED   The A1 firmware version is not supported.
  @retval      EFI_NO_RESPONSE   No response was received from the MMC device.
**/

EFI_STATUS
MmcFirmwareVersion(
    IN UINT8 *Buffer,
    IN UINTN BufferSize)
{
  UINTN NumberOfBytes;
  UINT8 xBuffer[MMC_X_BUFFER_SIZE];
  if (GetFirmwareMajorVersion()==0xA1) {
    DEBUG((DEBUG_INFO, "%a A1 is not supported\n", __FUNCTION__));
    return EFI_UNSUPPORTED;
  }
  // Send command to IPMI device
  NumberOfBytes = PL011UartWrite((UINTN)PcdGet64(PcdSerialDbgRegisterBase), CmdBuffer_MmcFirmwareVersion, sizeof(CmdBuffer_MmcFirmwareVersion));

  if (NumberOfBytes == 0)
  {
    DEBUG((DEBUG_ERROR, "%a Failed to Get MMC Version\n", __FUNCTION__));
    return EFI_NO_RESPONSE;
  }
  // Read the response from the IPMI device
  NumberOfBytes = PL011UartRead((UINTN)PcdGet64(PcdSerialDbgRegisterBase), xBuffer, MMC_X_BUFFER_SIZE);

  if (NumberOfBytes == 0)
  {

    DEBUG((DEBUG_ERROR, "%a Failed to Get MMC Version\n", __FUNCTION__));

    return EFI_NO_RESPONSE;
  }

  DEBUG((DEBUG_INFO, "%a [18 00 01]--->", __FUNCTION__));
  for (int i=0; i< MMC_X_BUFFER_SIZE; i++) {
    DEBUG((DEBUG_INFO, "%02d:%02x ", i, xBuffer[i]));
  }
  DEBUG((DEBUG_INFO, "\n"));

  DEBUG((DEBUG_INFO, "%a [18 00 01]--->", __FUNCTION__));
  for (int i=0; i< MMC_X_BUFFER_SIZE; i++) {
    DEBUG((DEBUG_INFO, "%c", xBuffer[i]));
  }
  DEBUG((DEBUG_INFO, "\n"));

  CopyMem(Buffer, xBuffer + MMC_RESPONSE_OFFSET, BufferSize - 1);

  Buffer[2] = '.';

  return EFI_SUCCESS;
}

/**
  Retrieves the sensor reading by sending an IPMI command over UART and processing the response.

  This function sends a command to an IPMI device via UART and reads the response.
  The relevant sensor data is extracted from the received buffer and stored in the provided output buffer.

  @param[out]  Buffer           Pointer to the buffer where the extracted sensor data will be stored.
  @param[in]   BufferSize       Size of the Buffer in bytes.
  @param[in]   IpmiCmdBuf       Pointer to the IPMI command buffer to be sent.
  @param[in]   IpmiCmdBufSize   Size of the IPMI command buffer.

  @retval      EFI_SUCCESS      The sensor data was successfully retrieved and stored in Buffer.
  @retval      EFI_NO_RESPONSE  No response was received from the IPMI device.
**/
EFI_STATUS
Get_Sensor_Reading(
    IN UINT8 *Buffer,
    IN UINTN BufferSize,
    IN UINT8 *IpmiCmdBuf,
    IN UINTN IpmiCmdBufSize
	)
{
    UINTN NumberOfBytes;
    UINT8 xBuffer[X_BUFFER_SIZE];

    // Send command to IPMI device
    NumberOfBytes = PL011UartWrite((UINTN)PcdGet64(PcdSerialDbgRegisterBase), IpmiCmdBuf, IpmiCmdBufSize);

    if (NumberOfBytes == 0)
    {
        DEBUG((DEBUG_ERROR, "%a Failed to send command to get sensor reading\n", __FUNCTION__));
        
        return EFI_NO_RESPONSE;
    }

    // Read the response from the IPMI device
    NumberOfBytes = PL011UartRead((UINTN)PcdGet64(PcdSerialDbgRegisterBase), xBuffer, X_BUFFER_SIZE);

    if (NumberOfBytes == 0)
    {
        DEBUG((DEBUG_ERROR, "%a Failed to receive sensor reading\n", __FUNCTION__));
        
        return EFI_NO_RESPONSE;
    }

    // Debug output
    DEBUG((DEBUG_INFO, "%a Command --->", __FUNCTION__));
    for (int i = 0; i < X_BUFFER_SIZE; i++) {
        DEBUG((DEBUG_INFO, "%02d:%02x ", i, xBuffer[i]));
    }
    DEBUG((DEBUG_INFO, "\n"));

    DEBUG((DEBUG_INFO, "%a Command Output --->", __FUNCTION__));
    for (int i = 0; i < X_BUFFER_SIZE; i++) {
        DEBUG((DEBUG_INFO, "%c", xBuffer[i]));
    }
    DEBUG((DEBUG_INFO, "\n"));


    CopyMem(Buffer, xBuffer + SENSOR_RAW_DATA_OFFSET, BufferSize);

  return EFI_SUCCESS;
}
EFI_STATUS
WolDisableCmd( )
{
  UINTN NumberOfBytes;

  NumberOfBytes = PL011UartWrite((UINTN)PcdGet64(PcdSerialDbgRegisterBase), CmdBuffer_wolCmdBuf_D, sizeof(CmdBuffer_wolCmdBuf_D));

  if (NumberOfBytes == 0)
  {
    return EFI_NO_RESPONSE;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
WolEnableCmd( )
{
  UINTN NumberOfBytes;

  NumberOfBytes = PL011UartWrite((UINTN)PcdGet64(PcdSerialDbgRegisterBase), CmdBuffer_wolCmdBuf_E, sizeof(CmdBuffer_wolCmdBuf_E));

  if (NumberOfBytes == 0)
  {
    return EFI_NO_RESPONSE;
  }

  return EFI_SUCCESS;
}
