#include "filemanager.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(manager)
filemanager::filemanager(const char* fname, const int Mode)
{
	this->fname = fname;
	this->Mode = Mode;
}

filemanager::~filemanager()
{
	if (file.is_open())
		file.close();
}

bool filemanager::openFile()
{
	file.open(fname.c_str(), Mode);
	return file.is_open();
}

bool filemanager::CanOpenFile()
{
	const auto success = openFile();
	file.close();
	return success;
}

bool filemanager::getFileBuffer(BYTE*& out_data, size_t& out_size)
{
	file.seekg(0, file.end);
	const auto size = static_cast<int>(file.tellg());
	file.seekg(0, file.beg);

	auto buffer = ALLOC<char>(size + 1);
	if (file.read(buffer, size))
	{
		buffer[size] = '\0';
		out_data = (BYTE*)buffer;
		out_size = static_cast<size_t>(size);

		file.close();
		return true;
	}

	FREE(buffer);
	file.close();
	return false;
}

bool filemanager::file_exists()
{
	return CanOpenFile();
}

bool filemanager::read(BYTE*& out_data, size_t& out_size)
{
	if (file_exists())
	{
		openFile();
		return getFileBuffer(out_data, out_size);
	}

	return false;
}

bool filemanager::read(char*& out_data)
{
	if (file_exists())
	{
		openFile();

		BYTE* out_byte = nullptr;
		size_t out_size = NULL;
		const auto res = getFileBuffer(out_byte, out_size);
		if (res && out_size > 0)
		{
			out_data = ALLOC<char>(out_size + 1);
			memcpy(out_data, out_byte, out_size);
			out_data[out_size] = '\0';
		}

		FREE(out_byte);

		return res;
	}

	return false;
}

bool filemanager::write(BYTE* data, const size_t size) const
{
	std::ofstream ofile { fname.c_str(), Mode };
	if (ofile.write(reinterpret_cast<char*>(data), size))
	{
		ofile.close();
		return true;
	}

	ofile.close();
	return false;
}

bool filemanager::write(const char* str) const
{
	std::ofstream ofile{ fname.c_str(), Mode };
	if (ofile.write(str, strlen(str)))
	{
		ofile.close();
		return true;
	}

	ofile.close();
	return false;
}

bool filemanager::write(const wchar_t* str) const
{
	std::wofstream wofile{ fname.c_str(), Mode };
	if (wofile.write(str, wcslen(str)))
	{
		wofile.close();
		return true;
	}

	wofile.close();
	return false;
}

void filemanager::clear() const
{
	std::ofstream ofile{ fname.c_str(), std::ofstream::out | std::ofstream::trunc };
	ofile.close();
}

void filemanager::close()
{
	if (file.is_open())
		file.close();
}
NET_NAMESPACE_END
NET_NAMESPACE_END
