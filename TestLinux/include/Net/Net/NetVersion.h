#pragma once
#define NetMajorVersion 4 // Re-Code - Library Changes
#define NetMinorVersion 4 // Function extension
#define NetRevision 5 // Issue fixing
#define NetKey CSTRING("1MFOm3a9as-xieg1iEMIf-pgKHPNlSMP-pgKHPNlDEP")

#include "NetString.h"

namespace Net
{
	namespace Version
	{
		short Major();
		short Minor();
		short Revision();
		Net::String Key();
		Net::String Version();
	}
}
