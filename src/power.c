/*++
    Copyright (c) Microsoft Corporation. All Rights Reserved.
    Copyright (c) Bingxing Wang. All Rights Reserved.
    Copyright (c) LumiaWoA authors. All Rights Reserved.

    Module Name:

        power.c

    Abstract:

        Contains NovaTek power-on and power-off functionality

    Environment:

        Kernel mode

    Revision History:

--*/

#include <Cross Platform Shim\compat.h>
#include <controller.h>
#include <spb.h>
#include <device.h>
#include <nt36xxx\ntinternal.h>
#include <nt36xxx\ntfwupdate.h>
#include <internal.h>
#include <touch_power\touch_power.h>
#include <power.tmh>

NTSTATUS
TchPowerSettingCallback(
    _In_ LPCGUID SettingGuid,
    _In_ PVOID Value,
    _In_ ULONG ValueLength,
    _Inout_opt_ PVOID Context
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION devContext = NULL;
    NT36XXX_CONTROLLER_CONTEXT* ControllerContext = NULL;
    SPB_CONTEXT* SpbContext = NULL;

    if (Context == NULL)
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_POWER,
            "TchPowerSettingCallback: Context is NULL"
        );

        status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

    devContext = (PDEVICE_EXTENSION)Context;
    ControllerContext = (NT36XXX_CONTROLLER_CONTEXT*)devContext->TouchContext;
    SpbContext = &(devContext->I2CContext);

    if (IsEqualGUID(&GUID_CONSOLE_DISPLAY_STATE, SettingGuid))
    {
        Trace(
            TRACE_LEVEL_INFORMATION,
            TRACE_POWER,
            "Monitor State Change Notification");

        if (ValueLength != sizeof(DWORD))
        {
            Trace(
                TRACE_LEVEL_ERROR,
                TRACE_POWER,
                "TchPowerSettingCallback: Unexpected value size."
            );

            status = STATUS_INVALID_DEVICE_REQUEST;
            goto exit;
        }

        DWORD DisplayState = *(DWORD*)Value;

        switch (DisplayState)
        {
        case 0:
            Trace(
                TRACE_LEVEL_INFORMATION,
                TRACE_POWER,
                "The Display is Off");
            break;
        case 1:
            Trace(
                TRACE_LEVEL_INFORMATION,
                TRACE_POWER,
                "The Display is On");
#if NVT_UPDATE_FW_ON_RESUME
            //Load firmware each time after display turned on
            status = NVTLoadFirmwareFile(ControllerContext->FxDevice, SpbContext);
#endif
            break;
        case 2:
            Trace(
                TRACE_LEVEL_INFORMATION,
                TRACE_POWER,
                "The Display is dimmed");

            break;
        default:
            Trace(
                TRACE_LEVEL_ERROR,
                TRACE_POWER,
                "Unknown display state - 0x%02X",
                DisplayState);
        }
    }

exit:
    return status;
}

NTSTATUS
TchWakeDevice(
   IN VOID *ControllerContext,
   IN SPB_CONTEXT *SpbContext
)
/*++

Routine Description:

   Enables multi-touch scanning

Arguments:

   ControllerContext - Touch controller context

   SpbContext - A pointer to the current i2c context

Return Value:

   NTSTATUS indicating success or failure

--*/
{
    UNREFERENCED_PARAMETER(SpbContext);
    NT36XXX_CONTROLLER_CONTEXT* controller;
    //NTSTATUS status;

    controller = (NT36XXX_CONTROLLER_CONTEXT*) ControllerContext;

    //
    // Check if we were already on
    //
    if (controller->DevicePowerState == PowerDeviceD0)
    {
        goto exit;
    }

    controller->DevicePowerState = PowerDeviceD0;

exit:

    return STATUS_SUCCESS;
}

NTSTATUS
TchStandbyDevice(
   IN VOID *ControllerContext,
   IN SPB_CONTEXT *SpbContext,
    IN VOID* ReportContext
)
/*++

Routine Description:

   Disables multi-touch scanning to conserve power

Arguments:

   ControllerContext - Touch controller context

   SpbContext - A pointer to the current i2c context

Return Value:

   NTSTATUS indicating success or failure

--*/
{
    UNREFERENCED_PARAMETER(SpbContext);
    NT36XXX_CONTROLLER_CONTEXT* controller;
    //NTSTATUS status;

    controller = (NT36XXX_CONTROLLER_CONTEXT*) ControllerContext;

    //
    // Interrupts are now disabled but the ISR may still be
    // executing, so grab the controller lock to ensure ISR
    // is finished touching HW and controller state.
    //
    WdfWaitLockAcquire(controller->ControllerLock, NULL);

    //
    // Put the chip in sleep mode
    //
    controller->DevicePowerState = PowerDeviceD3;

    //
    // Invalidate state
    //
    ((PREPORT_CONTEXT)ReportContext)->Cache.SlotValid = 0;
    ((PREPORT_CONTEXT)ReportContext)->Cache.SlotDirty = 0;
    ((PREPORT_CONTEXT)ReportContext)->Cache.DownCount = 0;



    WdfWaitLockRelease(controller->ControllerLock);

    return STATUS_SUCCESS;
}