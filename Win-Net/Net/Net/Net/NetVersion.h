#pragma once
#define NetMajorVersion 3 // Re-Code - Library Changes
#define NetMinorVersion 4 // Function extension
#define NetRevision 4 // Issue fixing
#define NetKey CSTRING("1MFOm3a9as-xieg1iEMIf-pgKHPNlSMP-pgKHPNlSMP")

#include "NetString.h"

namespace Net
{
	namespace Version
	{
		short Major();
		short Minor();
		short Revision();
		NetString Key();
		NetString Version();
	}
}
