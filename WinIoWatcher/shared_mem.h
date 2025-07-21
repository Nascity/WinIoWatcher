#pragma once

#include "../common.h"

BOOL
InitSharedMem(
	VOID
);

BOOL
InitEvent(
	VOID
);

VOID
MemCleanup(
	VOID
);

VOID
ReadLog(
	PLOG	pLog
);