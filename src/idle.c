/*++
    Copyright (c) Microsoft Corporation. All Rights Reserved.
    Copyright (c) Bingxing Wang. All Rights Reserved.
    Copyright (c) LumiaWoA authors. All Rights Reserved.

    Module Name:

        idle.c

    Abstract:

        This file contains the declarations for Power Idle specific callbacks
        and function definitions

    Environment:

        Kernel mode

    Revision History:

--*/

#include <internal.h>
#include <controller.h>
#include <idle.h>
#include <idle.tmh>

NTSTATUS
TchProcessIdleRequest(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request,
    OUT BOOLEAN* Pending
)
/*++

Routine Description:

   Handles HIDClass's idle notification request.

   This request is provided to a HID miniport, and provides a callback
   routine typically used by HID miniports to self-manage power.

   The callback routine is invoked by the miniport to indicate that all
   peripherals are idle, and in response the HID class driver will:
     1) Queue a wait/wake IRP to the device, and
     2) Set the device to D3

   In the case of this touch miniport, we are using the HID class driver's
   enhanced power management functionality, whereby invoking the callback
   results in an immediate exit from D0, powering off touch.

   The Request will be completed when either HIDCLASS cancels it or
   there is a device wake signal that will cause us to complete it.

Arguments:

   Device - Handle to WDF Device Object

   Request - Handle to request object

   Pending - flag to monitor if the request was sent down the stack

Return Value:

   On success, the function returns STATUS_SUCCESS
   On failure it passes the relevant error code to the caller.

--*/
{
    PDEVICE_EXTENSION devContext;
    PHID_SUBMIT_IDLE_NOTIFICATION_CALLBACK_INFO idleCallbackInfo;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status = STATUS_SUCCESS;

    devContext = GetDeviceContext(Device);

    NT_ASSERT(Pending != NULL);
    *Pending = FALSE;

    //
    // Retrieve request parameters and validate
    //
    irp = WdfRequestWdmGetIrp(Request);
    irpSp = IoGetCurrentIrpStackLocation(irp);

    if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
        sizeof(HID_SUBMIT_IDLE_NOTIFICATION_CALLBACK_INFO))
    {
        status = STATUS_INVALID_BUFFER_SIZE;

        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_HID,
            "Error: Input buffer is too small to process idle request - 0x%08lX", 
            status);

        goto exit;
    }

    //
    // Grab the callback
    //
    idleCallbackInfo = (PHID_SUBMIT_IDLE_NOTIFICATION_CALLBACK_INFO)
        irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

    NT_ASSERT(idleCallbackInfo != NULL);

    if (idleCallbackInfo == NULL || idleCallbackInfo->IdleCallback == NULL)
    {
        status = STATUS_NO_CALLBACK_ACTIVE;
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_HID,
            "Error: Idle Notification request %p has no idle callback info - 0x%08lX",
            Request,
            status);
        goto exit;
    }

exit:

    return status;
}