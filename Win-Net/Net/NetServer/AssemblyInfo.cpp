#include "stdafx.h"
#include "Server.h"

#ifdef C_DLL
using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::CompilerServices;
using namespace System::Runtime::InteropServices;
using namespace System::Security::Permissions;

[assembly:AssemblyTitleAttribute(L"NetServer")];
[assembly:AssemblyDescriptionAttribute(L"")];
[assembly:AssemblyConfigurationAttribute(L"")];
[assembly:AssemblyCompanyAttribute(L"")];
[assembly:AssemblyProductAttribute(L"NetServer")];
[assembly:AssemblyCopyrightAttribute(L"Copyright (c)  2018")];
[assembly:AssemblyTrademarkAttribute(L"")];
[assembly:AssemblyCultureAttribute(L"")];

[assembly:AssemblyVersionAttribute(NetVersion)];

[assembly:ComVisible(false)];

[assembly:CLSCompliantAttribute(true)];
#endif