/*++
	Copyright (c) Microsoft Corporation. All Rights Reserved.
	Copyright (c) Bingxing Wang. All Rights Reserved.
	Copyright (c) LumiaWoA authors. All Rights Reserved.

	Module Name:

		report.c

	Abstract:

		Contains NovaTek specific code for reporting samples

	Environment:

		Kernel mode

	Revision History:

--*/

#pragma once

#include <Cross Platform Shim\compat.h>
#include <controller.h>
#include <resolutions.h>
#include <hid.h>
#include <HidCommon.h>
#include <spb.h>

#define MAX_TOUCHES                10

typedef struct _OBJECT_INFO
{
	int x;
	int y;
	UCHAR status;
} OBJECT_INFO;

typedef struct _OBJECT_CACHE
{
	OBJECT_INFO Slot[MAX_TOUCHES];
	UINT32 SlotValid;
	UINT32 SlotDirty;
	int DownOrder[MAX_TOUCHES];
	int DownCount;
	ULONG64 ScanTime;
} OBJECT_CACHE;

typedef struct _DETECTED_OBJECT_POSITION
{
	int X;
	int Y;
} DETECTED_OBJECT_POSITION;

typedef enum _OBJECT_STATE
{
	OBJECT_STATE_NOT_PRESENT = 0,
	OBJECT_STATE_FINGER_PRESENT_WITH_ACCURATE_POS = 1,
	OBJECT_STATE_FINGER_PRESENT_WITH_INACCURATE_POS = 2,
	OBJECT_STATE_RESERVED = 3
} OBJECT_STATE;

typedef struct _DETECTED_OBJECTS
{
	OBJECT_STATE States[MAX_TOUCHES];
	DETECTED_OBJECT_POSITION Positions[MAX_TOUCHES];
} DETECTED_OBJECTS;

typedef struct _REPORT_CONTEXT
{
	OBJECT_CACHE Cache;
	TOUCH_SCREEN_PROPERTIES Props;
	WDFQUEUE PingPongQueue;
} REPORT_CONTEXT, * PREPORT_CONTEXT;

NTSTATUS
ReportObjects(
	IN PREPORT_CONTEXT ReportContext,
	IN DETECTED_OBJECTS data
);

NTSTATUS
ReportConfigureContinuousSimulationTimer(
	IN WDFDEVICE DeviceHandle
);