#pragma once

#include "../common.h"

#define LOG_FILE_NAME	L"log.csv"

BOOL
InitLogFile(
	VOID
);

VOID
FileCleanup(
	VOID
);

VOID
WriteLog(
	PLOG	pLog
);