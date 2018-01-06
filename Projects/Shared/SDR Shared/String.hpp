#pragma once
#include <string>

using namespace std::string_literals;

namespace SDR::String
{
	std::string ToUTF8(const std::wstring& input);
	std::string ToUTF8(const wchar_t* input);

	std::wstring FromUTF8(const std::string& input);
	std::wstring FromUTF8(const char* input);

	template <typename... Args>
	inline std::string Format(const char* format, Args&&... args)
	{
		std::string ret;

		auto size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
		ret.resize(size + 1);

		std::snprintf(ret.data(), size + 1, format, std::forward<Args>(args)...);

		/*
			Remove null terminator that above function adds.
		*/
		ret.pop_back();

		return ret;
	}

	template <typename T>
	bool StartsWith(const T* str, const T* begin)
	{
		auto sourcelength = std::char_traits<T>::length(str);
		auto beginlength = std::char_traits<T>::length(begin);

		if (sourcelength < beginlength)
		{
			return false;
		}

		auto start = str;
		return std::char_traits<T>::compare(start, begin, beginlength) == 0;
	}

	template <typename T>
	bool EndsWith(const T* str, const T* end)
	{
		auto sourcelength = std::char_traits<T>::length(str);
		auto endlength = std::char_traits<T>::length(end);

		if (sourcelength < endlength)
		{
			return false;
		}

		auto start = str + sourcelength - endlength;
		return std::char_traits<T>::compare(start, end, endlength) == 0;
	}

	template <typename T>
	bool IsSpace(T character)
	{
		return character == ' ';
	}

	inline bool IsEqual(const char* first, const char* other)
	{
		return std::strcmp(first, other) == 0;
	}
}
