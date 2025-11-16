/*++
	Copyright (c) Microsoft Corporation. All Rights Reserved.
	Sample code. Dealpoint ID #843729.

	Module Name:

		ftinternal.h

	Abstract:

		Contains common types and defintions used internally
		by the multi touch screen driver.

	Environment:

		Kernel mode

	Revision History:

--*/

#pragma once

#include <wdm.h>
#include <wdf.h>
#include <controller.h>
#include <resolutions.h>
#include <Cross Platform Shim/bitops.h>
#include <Cross Platform Shim/hweight.h>
#include <report.h>

// Ignore warning C4152: nonstandard extension, function/data pointer conversion in expression
#pragma warning (disable : 4152)

// Ignore warning C4201: nonstandard extension used : nameless struct/union
#pragma warning (disable : 4201)

// Ignore warning C4201: nonstandard extension used : bit field types other than in
#pragma warning (disable : 4214)

// Ignore warning C4324: 'xxx' : structure was padded due to __declspec(align())
#pragma warning (disable : 4324)

typedef struct _NOVATEK_TOUCH_DATA
{
	BYTE PositionX_High : 4;
	BYTE Reserved0 : 2;
	BYTE EventFlag : 2;

	BYTE PositionX_Low;

	BYTE PositionY_High : 4;
	BYTE TouchId : 4;

	BYTE PositionY_Low;

	BYTE TouchWeight;

	BYTE Reserved1 : 4;
	BYTE TouchArea : 4;
} NOVATEK_TOUCH_DATA, * PNOVATEK_TOUCH_DATA;

typedef struct _NOVATEK_EVENT_DATA
{
	BYTE Reserved0 : 4;
	BYTE DeviceMode : 3;
	BYTE Reserved1 : 1;

	BYTE GestureId;

	BYTE NumberOfTouchPoints : 4;
	BYTE Reserved2 : 4;

	NOVATEK_TOUCH_DATA TouchData[6];
} NOVATEK_EVENT_DATA, * PNOVATEK_EVENT_DATA;

#define TOUCH_POOL_TAG_F12              (ULONG)'21oT'

//
// Driver structures
//

typedef struct _NT36XXX_CONTROLLER_CONTEXT
{
	WDFDEVICE FxDevice;
	WDFWAITLOCK ControllerLock;

	//
	// Power state
	//
	DEVICE_POWER_STATE DevicePowerState;

	UCHAR Data1Offset;

	BYTE MaxFingers;

    int HidQueueCount;
} NT36XXX_CONTROLLER_CONTEXT;

NTSTATUS
Ft5xConfigureFunctions(
	IN NT36XXX_CONTROLLER_CONTEXT* ControllerContext,
	IN SPB_CONTEXT* SpbContext
);

NTSTATUS
Nt36xxxServiceInterrupts(
	IN NT36XXX_CONTROLLER_CONTEXT* ControllerContext,
	IN SPB_CONTEXT* SpbContext,
	IN PREPORT_CONTEXT ReportContext
);

#define NVT_UPDATE_FW_ON_RESUME 1
