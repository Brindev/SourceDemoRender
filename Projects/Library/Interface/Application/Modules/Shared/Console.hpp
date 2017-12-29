#pragma once
#include <memory>
#include <cstdint>
#include <SDR Shared\ConsoleTypes.hpp>

namespace SDR::Console
{
	struct Variable
	{
		Variable() = default;
		Variable(const Variable& other) = delete;
		Variable(Variable&& other) = default;

		Variable& operator=(const Variable& other) = delete;
		Variable& operator=(Variable&& other) = default;

		explicit operator bool() const;

		static Variable Find(const char* name);

		bool GetBool() const;
		int GetInt() const;
		float GetFloat() const;
		const char* GetString() const;

		void SetValue(const char* value);
		void SetValue(float value);
		void SetValue(int value);

		void* Opaque = nullptr;
	};

	struct CommandArgs
	{
		CommandArgs(const void* ptr);

		int Count() const;
		const char* At(int index) const;
		const char* FullArgs() const;
		const char* FullValue() const;

		const void* Ptr;
	};

	struct Command
	{
		void* Opaque = nullptr;
	};

	void Load();
	bool IsOutputToGameConsole();

	void MakeCommand(const char* name, Types::CommandCallbackVoidType callback);
	void MakeCommand(const char* name, Types::CommandCallbackArgsType callback);

	Variable MakeBool(const char* name, const char* value);

	Variable MakeNumber(const char* name, const char* value);
	Variable MakeNumber(const char* name, const char* value, float min);
	Variable MakeNumber(const char* name, const char* value, float min, float max);
	Variable MakeNumberWithString(const char* name, const char* value, float min, float max);

	Variable MakeString(const char* name, const char* value);
}
