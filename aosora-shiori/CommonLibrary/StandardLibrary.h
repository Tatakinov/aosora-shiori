﻿#pragma once
#include "Interpreter/Interpreter.h"

//aosoraスタンダードライブラリ
//stdユニットに存在する機能群として作成
namespace sakura {

	//スクリプト向けjsonシリアライザ
	class ScriptJsonSerializer : public Object<ScriptJsonSerializer> {
	public:
		static ScriptValueRef StaticGet(const std::string& key, ScriptExecuteContext& executeContext);

		static void ScriptSerialize(const FunctionRequest& request, FunctionResponse& response);
		static void ScriptDeserialize(const FunctionRequest& request, FunctionResponse& response);
	};

	//ファイルアクセス
	class ScriptFileAccess : public Object<ScriptFileAccess> {
	private:
		enum class PathCheckMode {
			Exists,
			IsDirectory,
			IsFile
		};

		static bool ValidateFileAccess(const FunctionRequest& request, FunctionResponse& response);
		static Charset CharsetFromFunctionArgs(size_t index, const FunctionRequest& request);
		static void ConvertReadCharset(std::string& str, Charset charset);
		static void ConvertWriteCharset(std::string& str, Charset charset);
		static void PathCheck(const FunctionRequest& request, FunctionResponse& response, PathCheckMode mode);

	public:
		static ScriptValueRef StaticGet(const std::string& key, ScriptExecuteContext& executeContext);

		static void ReadAllText(const FunctionRequest& request, FunctionResponse& response);
		static void WriteAllText(const FunctionRequest& request, FunctionResponse& response);

		static void Delete(const FunctionRequest& request, FunctionResponse& response);
		static void Move(const FunctionRequest& request, FunctionResponse& response);
		static void Copy(const FunctionRequest& request, FunctionResponse& response);
		static void Exists(const FunctionRequest& request, FunctionResponse& response);
		static void IsDirectory(const FunctionRequest& request, FunctionResponse& response);
		static void IsFile(const FunctionRequest& request, FunctionResponse& response);
		static void CreateDirectory(const FunctionRequest& request, FunctionResponse& response);
	};

}
