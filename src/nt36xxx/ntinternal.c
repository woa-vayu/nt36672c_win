/*++
      Copyright (c) Microsoft Corporation. All Rights Reserved.
      Sample code. Dealpoint ID #843729.

      Module Name:

            rmiinternal.c

      Abstract:

            Contains Synaptics initialization code

      Environment:

            Kernel mode

      Revision History:

--*/

#include <Cross Platform Shim\compat.h>
#include <spb.h>
#include <report.h>
#include <device.h>
#include <nt36xxx\ntinternal.h>
#include <nt36xxx\ntfwupdate.h>
#include <ntinternal.tmh>

NTSTATUS
Ft5xBuildFunctionsTable(
      IN NT36XXX_CONTROLLER_CONTEXT* ControllerContext,
      IN SPB_CONTEXT* SpbContext
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);

      return STATUS_SUCCESS;
}

NTSTATUS
Ft5xChangePage(
      IN NT36XXX_CONTROLLER_CONTEXT* ControllerContext,
      IN SPB_CONTEXT* SpbContext,
      IN int DesiredPage
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);
      UNREFERENCED_PARAMETER(DesiredPage);

      return STATUS_SUCCESS;
}

NTSTATUS
Ft5xConfigureFunctions(
      IN NT36XXX_CONTROLLER_CONTEXT* ControllerContext,
      IN SPB_CONTEXT* SpbContext
)
{
    NT36XXX_CONTROLLER_CONTEXT* controller;
    controller = (NT36XXX_CONTROLLER_CONTEXT*)ControllerContext;
    //unsigned char value;
    
    LARGE_INTEGER delay = { 0 };

    unsigned char dataBuffer[7] = { 0, 0, 0, 0, 0, 0, 0 };
    
    delay.QuadPart = RELATIVE(MILLISECONDS(10));
    KeDelayExecutionThread(KernelMode, TRUE, &delay);
    
    //value = 0;
    //SetGPIO(devContext->ResetGpio, &value);

    nt36xxx_eng_reset(SpbContext);
    
    //value = 1;
    //SetGPIO(devContext->ResetGpio, &value);

    if (nt36xxx_bootloader_reset(SpbContext)) {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INTERRUPT,
            "Failed to reset the Novatek IC");
    }

    nt36xxx_set_page(SpbContext, NT36XXX_PAGE_CHIP_INFO);

    SpbReadDataSynchronously(SpbContext, SPI_READ_MASK(NT36XXX_PAGE_CHIP_INFO & NT36XXX_EVT_CHIPID), dataBuffer, sizeof(dataBuffer), FALSE);

    Trace(
        TRACE_LEVEL_INFORMATION,
        TRACE_INTERRUPT,
        "TEST READ: %X %X %X %X %X %X %X",
        dataBuffer[0], dataBuffer[1], dataBuffer[2], dataBuffer[3], dataBuffer[4], dataBuffer[5], dataBuffer[6]);

    NVTLoadFirmwareFile(controller->FxDevice, SpbContext);

    return STATUS_SUCCESS;
}

struct nt36xxx_abs_object {
    unsigned short x;
    unsigned short y;
    unsigned short z;
    unsigned char tm;
};

#define TOUCH_MAX_FINGER_NUM 10

NTSTATUS
Ft5xGetObjectStatusFromControllerF12(
      IN VOID* ControllerContext,
      IN SPB_CONTEXT* SpbContext,
      IN DETECTED_OBJECTS* Data
)
/*++

Routine Description:

      This routine reads raw touch messages from hardware. If there is
      no touch data available (if a non-touch interrupt fired), the
      function will not return success and no touch data was transferred.

Arguments:

      ControllerContext - Touch controller context
      SpbContext - A pointer to the current i2c context
      Data - A pointer to any returned F11 touch data

Return Value:

      NTSTATUS, where only success indicates data was returned

--*/
{
    NTSTATUS status;
    NT36XXX_CONTROLLER_CONTEXT* controller;
    controller = (NT36XXX_CONTROLLER_CONTEXT*)ControllerContext;

    struct nt36xxx_abs_object objd = { 0, 0, 0, 0 };
    struct nt36xxx_abs_object* obj = &objd;

    //enable TchTranslateToDisplayCoordinates in report.c

    unsigned char input_id = 0;
    unsigned char point[79];
    unsigned int ppos = 0;
    int i = 0;
    
    unsigned short max_x = 1080, max_y = 2400;

    status = SpbReadDataSynchronously(SpbContext, 0, point, sizeof(point), TRUE);

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INTERRUPT,
            "Error reading finger status data - 0x%08lX",
            status);

        goto exit;
    }

    for (i = 0; i < TOUCH_MAX_FINGER_NUM; i++) {
        ppos = 6 * i;
        input_id = point[ppos + 0] >> 3;
        if ((input_id == 0) || (input_id > TOUCH_MAX_FINGER_NUM))
            continue;

        if (((point[ppos] & 0x07) == 0x01) ||
            ((point[ppos] & 0x07) == 0x02)) {
            obj->x = (point[ppos + 1] << 4) +
                (point[ppos + 3] >> 4);
            obj->y = (point[ppos + 2] << 4) +
                (point[ppos + 3] & 0xf);
            if ((obj->x > max_x) ||
                (obj->y > max_y))
                continue;

            obj->tm = point[ppos + 4];
            if (obj->tm == 0)
                obj->tm = 1;

            obj->z = point[ppos + 5];
 
            Data->States[input_id-1] = OBJECT_STATE_FINGER_PRESENT_WITH_ACCURATE_POS;

            Trace(
                TRACE_LEVEL_ERROR,
                TRACE_INTERRUPT,
                "TOUCH x: %d, y:%d",
                obj->x, obj->y);

            Data->Positions[input_id-1].X = obj->x;
            Data->Positions[input_id-1].Y = obj->y;
            //printf("x:%d y:%d point:%d\n", (int)obj->x, (int)obj->y, finger_cnt);
        }
    }

exit:
    return status;
}

NTSTATUS
TchServiceObjectInterrupts(
      IN NT36XXX_CONTROLLER_CONTEXT* ControllerContext,
      IN SPB_CONTEXT* SpbContext,
      IN PREPORT_CONTEXT ReportContext
)
{
      NTSTATUS status = STATUS_SUCCESS;
      DETECTED_OBJECTS data;

      RtlZeroMemory(&data, sizeof(data));

      //
      // See if new touch data is available
      //
      status = Ft5xGetObjectStatusFromControllerF12(
            ControllerContext,
            SpbContext,
            &data
      );

      if (!NT_SUCCESS(status))
      {
            Trace(
                  TRACE_LEVEL_VERBOSE,
                  TRACE_SAMPLES,
                  "No object data to report - 0x%08lX",
                  status);

            goto exit;
      }

      status = ReportObjects(
            ReportContext,
            data);

      if (!NT_SUCCESS(status))
      {
            Trace(
                  TRACE_LEVEL_VERBOSE,
                  TRACE_SAMPLES,
                  "Error while reporting objects - 0x%08lX",
                  status);

            goto exit;
      }

exit:
      return status;
}


NTSTATUS
Nt36xxxServiceInterrupts(
      IN NT36XXX_CONTROLLER_CONTEXT* ControllerContext,
      IN SPB_CONTEXT* SpbContext,
      IN PREPORT_CONTEXT ReportContext
)
{
      NTSTATUS status = STATUS_SUCCESS;

      TchServiceObjectInterrupts(ControllerContext, SpbContext, ReportContext);

      return status;
}

NTSTATUS
Ft5xCheckInterrupts(
    IN NT36XXX_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext,
    IN ULONG* InterruptStatus
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);
      UNREFERENCED_PARAMETER(InterruptStatus);

      return STATUS_SUCCESS;
}

NTSTATUS
Ft5xConfigureInterruptEnable(
    IN NT36XXX_CONTROLLER_CONTEXT* ControllerContext,
    IN SPB_CONTEXT* SpbContext
)
{
      UNREFERENCED_PARAMETER(SpbContext);
      UNREFERENCED_PARAMETER(ControllerContext);

      return STATUS_SUCCESS;
}
