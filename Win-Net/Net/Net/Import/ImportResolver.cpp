/*
	MIT License

	Copyright (c) 2022 Tobias Staack

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#ifndef BUILD_LINUX
#include "ImportResolver.h"

typedef HMODULE(*_LoadLibraryA)(LPCSTR lpLibFileName);
typedef BOOL(*_FreeLibrary)(HMODULE hLibModule);
typedef FARPROC(*_GetProcAddress)(HMODULE hModule, LPCSTR lpProcName);

namespace Import
{
	namespace Resolver
	{
		static std::map<std::string, module_t> modules;

		bool isLoaded(const char* library)
		{
			return modules.find(library) != modules.end();
		}

		module_t getModule(const char* library)
		{
			return modules[library];
		}

		bool Load(const char* library, const char* path, type_t type)
		{
			if (isLoaded(library)) return true;

			module_t mod = { 0 };

			switch (type)
			{
			case type_t::RESOLVE_KERNEL32:
			{
				/* Load up Kernel32 Module if not done yet */
				if (!isLoaded(CSTRING("Kernel32")))
					if (!Load(CSTRING("Kernel32"), CSTRING(WINDOWS_MODULE_PATH"\\kernel32.dll"), Import::Resolver::type_t::RESOLVE_MEMORY)) break;

				auto ptr = Function(CSTRING("Kernel32"), CSTRING("LoadLibraryA"));
				if (!ptr.valid()) return false;

				auto fnc = (_LoadLibraryA)ptr.get();
				if (!fnc)
				{
					Unload(CSTRING("Kernel32"));
					break;
				}

				mod.module = fnc(path);
				break;
			}

			case type_t::RESOLVE_MEMORY:
			{
				/* load file from path */
				NET_FILEMANAGER fmanager(path, NET_FILE_READ);

				BYTE* data = nullptr;
				size_t size = 0;
				if (fmanager.read(data, size))
				{
					mod.module = ::MemoryLoadLibrary(data, size);
					FREE<byte>(data);
					break;
				}

				NET_LOG_ERROR(CSTRING("Unable to resolve memory from file '%s'"), path);
				return false;
			}

			default:
				NET_LOG_WARNING(CSTRING("Invalid Type"));
				return false;
			};

			if (mod.module.valid())
			{
				strcpy_s(mod.name, library);
				strcpy_s(mod.path, path);
				mod.type = type;
				modules.emplace(std::pair<std::string, module_t>(library, mod));
				return true;
			}
			else
			{
				/* load using loadlibrary from IAT */
				mod.module = LoadLibraryA(path);

				if (mod.module.valid())
				{
					strcpy_s(mod.name, library);
					strcpy_s(mod.path, path);
					mod.type = type_t::RESOLVE_IAT;
					modules.emplace(std::pair<std::string, module_t>(library, mod));
				}
			}

			return false;
		}

		bool Remove(const char* library)
		{
			return modules.erase(library) > 0;
		}

		bool Unload(const char* library)
		{
			if (isLoaded(library)) return false;
			auto mod = getModule(library);
			if (!mod.module.valid()) return Remove(library);

			switch (mod.type)
			{
			case type_t::RESOLVE_KERNEL32:
			{
				/* Load up Kernel32 Module if not done yet */
				if (!isLoaded(CSTRING("Kernel32")))
					if (!Load(CSTRING("Kernel32"), CSTRING(WINDOWS_MODULE_PATH"\\kernel32.dll"), Import::Resolver::type_t::RESOLVE_MEMORY)) break;

				auto ptr = Function(CSTRING("Kernel32"), CSTRING("FreeLibrary"));
				if (!ptr.valid()) return false;

				auto fnc = (_FreeLibrary)ptr.get();
				if (!fnc)
				{
					Unload(CSTRING("Kernel32"));
					break;
				}

				fnc((HMODULE)mod.module.get());
				break;
			}

			case type_t::RESOLVE_MEMORY:
				::MemoryFreeLibrary((HMEMORYMODULE)mod.module.get());
				break;

			default:
				NET_LOG_WARNING(CSTRING("Invalid Type"));
				return Remove(library);
			};

			return Remove(library);
		}

		NET_CPOINTER<void> Function(const char* library, const char* funcName)
		{
			if (!isLoaded(library)) return NET_CPOINTER<void>();
			auto mod = getModule(library);
			if (!mod.module.valid()) return NET_CPOINTER<void>();

			switch (mod.type)
			{
			case type_t::RESOLVE_KERNEL32:
			{
				/* Load up Kernel32 Module if not done yet */
				if (!isLoaded(CSTRING("Kernel32")))
					if (!Load(CSTRING("Kernel32"), CSTRING(WINDOWS_MODULE_PATH"\\kernel32.dll"), Import::Resolver::type_t::RESOLVE_MEMORY)) break;

				auto ptr = Function(CSTRING("Kernel32"), CSTRING("GetProcAddress"));
				if (!ptr.valid()) return NET_CPOINTER<void>();

				auto fnc = (_GetProcAddress)ptr.get();
				if (!fnc)
				{
					Unload(CSTRING("Kernel32"));
					break;
				}

				return NET_CPOINTER<void>(fnc((HMODULE)mod.module.get(), funcName));
			}

			case type_t::RESOLVE_MEMORY:
				return NET_CPOINTER<void>(::MemoryGetProcAddress((HMEMORYMODULE)mod.module.get(), funcName));


			case type_t::RESOLVE_IAT:
			{
				auto ptr = NET_CPOINTER<void>(GetProcAddress((HMODULE)mod.module.get(), CSTRING("GetProcAddress")));
				if (!ptr.valid()) return NET_CPOINTER<void>();

				auto fnc = (_GetProcAddress)ptr.get();
				if (!fnc)
				{
					Unload(CSTRING("Kernel32"));
					break;
				}

				return NET_CPOINTER<void>(fnc((HMODULE)mod.module.get(), funcName));
			}

			default:
				NET_LOG_WARNING(CSTRING("Invalid Type"));
				break;
			};

			return NET_CPOINTER<void>();
		}
	}
}
#endif
