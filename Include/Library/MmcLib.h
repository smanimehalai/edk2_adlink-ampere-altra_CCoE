/** @file

**/

#ifndef __MMC_LIB_H__
#define __MMC_LIB_H__

#include <Uefi.h>
#include <Library/UefiLib.h>

#define CMD_BUFFER_SIZE           16 
#define SENSOR_RAW_DATA_OFFSET    33
#define X_BUFFER_SIZE             42
#define MAX_STRING_SIZE           64
#define MMC_RESPONSE_OFFSET       66
#define MMC_X_BUFFER_SIZE         105   
/**
  Sends a 32-bit value to a POST card.

  Sends the 32-bit value specified by Value to a POST card, and returns Value.
  Some implementations of this library function may perform I/O operations
  directly to a POST card device.  Other implementations may send Value to
  ReportStatusCode(), and the status code reporting mechanism will eventually
  display the 32-bit value on the status reporting device.

  MmcPostCode() must actively prevent recursion.  If MmcPostCode() is called while
  processing another Post Code Library function, then
  MmcPostCode() must return Value immediately.

  @param  Value  The 32-bit value to write to the POST card.

  @return The 32-bit value to write to the POST card.

**/
EFI_STATUS
MmcPostCode (
  IN UINT32  Value
  );

EFI_STATUS
MmcSetPowerOffType (
  IN UINT8  Value
  );

EFI_STATUS
MmcFirmwareVersion (
  IN UINT8 *Buffer,
  IN UINTN BufferSize
  );

EFI_STATUS
Get_Sensor_Reading(
    IN UINT8 *Buffer,
    IN UINTN BufferSize,
    IN UINT8 *IpmiCmdBuf,
    IN UINTN IpmiCmdBufSize
	);

EFI_STATUS
WolDisableCmd( 
  );
  
EFI_STATUS
WolEnableCmd( 
  );
#endif
