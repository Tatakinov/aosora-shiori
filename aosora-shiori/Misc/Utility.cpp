#include <stdio.h>
#include "Misc/Platform.h"
#include <fstream>
#include <filesystem>
#include "Misc/Utility.h"

namespace sakura {

#if defined(WIN32) || defined(_WIN32)
	constexpr UINT SHIFT_JIS = 932;
#endif // WIN32 or _WIN32

	//統一で使用するランダム用オブジェクト
	std::mt19937 randomEngine((std::random_device())());

	//ファイル読み込み
	bool File::ReadAllText(const std::string& filename, std::string& result) {
		std::ifstream loadStream(Utf8ToFileSystem(filename), std::ios_base::in);
		if (!loadStream) {
			return false;
		}
		result = std::string(std::istreambuf_iterator<char>(loadStream), std::istreambuf_iterator<char>());
		return true;
	}

	//ファイル書き込み
	bool File::WriteAllText(const std::string& filename, const std::string& content) {
		std::ofstream saveStream(Utf8ToFileSystem(filename), std::ios_base::out);
		if (!saveStream) {
			return false;
		}
		saveStream << content;

		return true;
	}

	//ファイルまたはフォルダのコピー
	void File::Copy(const std::string& sourceFilename, const std::string& destFilename, bool overwrite, std::error_code& err)
	{
		std::filesystem::copy_options copyOptions = std::filesystem::copy_options::none;

		if (overwrite) {
			copyOptions |= std::filesystem::copy_options::overwrite_existing;
		}

		std::filesystem::copy(Utf8ToFileSystem(sourceFilename), Utf8ToFileSystem(destFilename), copyOptions, err);
	}

	//ファイルまたはフォルダの削除
	void File::Delete(const std::string& filename, std::error_code& err)
	{
		std::filesystem::remove_all(filename, err);
	}

	//ファイルの移動
	void File::Move(const std::string& sourceFilename, const std::string& destFilename, std::error_code& err)
	{
		std::filesystem::rename(Utf8ToFileSystem(sourceFilename), Utf8ToFileSystem(destFilename), err);
	}

	//ファイルの存在確認
	bool File::Exists(const std::string& filename, std::error_code& err)
	{
		return std::filesystem::exists(Utf8ToFileSystem(filename), err);
	}

	void File::CreateDirectories(const std::string& filename, std::error_code& err)
	{
		std::filesystem::create_directories(Utf8ToFileSystem(filename), err);
	}

	bool File::IsDirectory(const std::string& filename, std::error_code& err)
	{
		return std::filesystem::is_directory(Utf8ToFileSystem(filename), err);
	}

	bool File::IsFile(const std::string& filename, std::error_code& err)
	{
		return std::filesystem::is_regular_file(Utf8ToFileSystem(filename), err);
	}
	
	std::vector<std::string> File::GetFiles(const std::string& filename, bool isRecursive, std::error_code& err)
	{
		std::vector<std::string> result;

		if (isRecursive) {
			for (const auto& item : std::filesystem::recursive_directory_iterator(Utf8ToFileSystem(filename), err)) {
				result.push_back(item.path().string());
			}
		}
		else
		{
			for (const auto& item : std::filesystem::directory_iterator(Utf8ToFileSystem(filename), err)) {
				result.push_back(item.path().string());
			}
		}
		
		return result;
	}

#if defined(WIN32) || defined(_WIN32)
	//Sjift_JISからUTF8へ変換
	std::string ConvertEncoding(const std::string& input, UINT inputEncode, UINT outputEncode) {

		if (input.empty()) {
			return std::string();
		}

		//UTF16に変換
		int len = MultiByteToWideChar(inputEncode, 0, input.c_str(), -1, NULL, 0);
		if (len <= 0) {
			return std::string();
		}

		wchar_t* wstr = static_cast<wchar_t*>(malloc(sizeof(wchar_t) * (len + 1)));
		MultiByteToWideChar(inputEncode, 0, input.c_str(), -1, wstr, len);

		//UTF8に変換
		int resultLen = WideCharToMultiByte(outputEncode, 0, wstr, -1, NULL, 0, NULL, NULL);
		if (resultLen <= 0) {
			free(wstr);
			return std::string();
		}

		char* str = static_cast<char*>(malloc(sizeof(char) * (resultLen + 1)));
		WideCharToMultiByte(outputEncode, 0, wstr, -1, str, resultLen, NULL, NULL);
		str[resultLen] = '\0';

		std::string result(str);
		free(wstr);
		free(str);
		return result;
	}

	std::string SjisToUtf8(const std::string& input) {
		return ConvertEncoding(input, SHIFT_JIS, CP_UTF8);
	}

	std::string Utf8ToSjis(const std::string& input) {
		return ConvertEncoding(input, CP_UTF8, SHIFT_JIS);
	}

	//将来的にロケールによって文字コードを選ぶなどの対応むけにファイルパス用関数を分離
	std::string Utf8ToFileSystem(const std::string& input) {
		return Utf8ToSjis(input);
	}
#else
	std::string ConvertEncoding(const std::string& input, const char *inputEncode, const char *outputEncode) {
        // TODO stub
		return input;
	}

	std::string SjisToUtf8(const std::string& input) {
        // TODO stub
        return input;
	}

	std::string Utf8ToSjis(const std::string& input) {
        // TODO stub
        return input;
	}
#endif // WIN32 or _WIN32
}
