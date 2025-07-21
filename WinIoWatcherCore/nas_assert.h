#pragma once

#include <wdm.h>

// Prints "[WIW] Assert failure at main.c:45"
// and returns 'status'
#define NAS_ASSERT(status)	do { NT_ASSERT(NT_SUCCESS(status)); if (!NT_SUCCESS(status)) { DbgPrint("[WIW] Assert failure at %s:%d.\n", __FILE__, __LINE__); return status; } } while (0)