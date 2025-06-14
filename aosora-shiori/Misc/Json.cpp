﻿#include <vector>
#include <map>
#include <memory>
#include <string>
#include <regex>
#include <cassert>
#include "Misc/Utility.h"
#include "Misc/Json.h"

//簡単なjsonシリアライズ・デシリアライズ

namespace sakura {

	//解析用正規表現
	const std::regex JSON_NUMBER_PATTERN(R"(^\s*(\-?[0-9.]+))");
	const std::regex JSON_STRING_PATTERN("^\\s*\"(([^\\\\\"]+|\\\\.)*)\"");
	const std::regex JSON_TRUE_PATTERN(R"(^\s*true)");
	const std::regex JSON_FALSE_PATTERN(R"(^\s*false)");
	const std::regex JSON_NULL_PATTERN(R"(^\s*null)");
	const std::regex JSON_OBJECT_BEGIN_PATTERN(R"(^\s*\{)");
	const std::regex JSON_OBJECT_END_PATTERN(R"(^\s*\})");
	const std::regex JSON_ARRAY_BEGIN_PATTERN(R"(^\s*\[)");
	const std::regex JSON_ARRAY_END_PATTERN(R"(^\s*\])");
	const std::regex JSON_OBJECT_COMMA_PATTERN(R"(^\s*\,)");
	const std::regex JSON_OBJECT_COLON_PATTERN(R"(^\s*\:)");

	//先頭にだけマッチさせ不要な処理を行わないよう明示的に指定するフラグ
	const auto TOKEN_MATCH_FLAGS = std::regex_constants::match_continuous | std::regex_constants::format_first_only | std::regex_constants::format_no_copy;
	
	class JsonParseContext {
	private:
		const std::string& document;
		std::string_view currentView;
		size_t offset;
		bool hasError;

	public:
		JsonParseContext(const std::string& json) :
			document(json),
			currentView(std::string_view(json)),
			offset(0),
			hasError(false) {
		}

		std::string_view GetCurrent() const {
			return currentView;
		}

		void Offset(size_t offsetSize) {
			offset += offsetSize;
			currentView = std::string_view(document.c_str() + offset, document.size() - offset);
		}

		void Error() {
			hasError = true;
		}

		bool HasError() const {
			return hasError;
		}
	};

	std::shared_ptr<JsonArray> DeserializeArray(JsonParseContext& parseContext);
	std::shared_ptr<JsonObject> DeserializeObject(JsonParseContext& parseContext);
	std::shared_ptr<JsonTokenBase> DeserializeToken(JsonParseContext& parseContext);
	void SerializeArray(const std::shared_ptr<JsonArray>& token, std::string& result);
	void SerializeObject(const std::shared_ptr<JsonObject>& token, std::string& result);

	//エスケープの追加
	inline void AddEscape(std::string& str) {
		for (size_t i = 0; i < str.size(); i++) {
			switch (str[i]) {
			case '"':
			case '\\':
			case '\n':
			case '\r':
			case '\t':
				//エスケープを挿入
				str.insert(i, 1, '\\');
				i++;
				break;
			}
		}
	}

	//エスケープの除去
	inline void RemoveEscape(std::string& str) {
		size_t pos = 0;
		size_t offset = 0;

		while (offset < str.size() && (pos = str.find('\\', offset)) != std::string::npos) {
			str.erase(pos, 1);
			offset = pos + 1;	//エスケープの次の文字は無視
		}
	}

	std::shared_ptr<JsonTokenBase> DeserializeToken(JsonParseContext& parseContext) {

		std::string_view str = parseContext.GetCurrent();
		std::match_results<std::string_view::const_iterator> match;

		if (std::regex_search(str.cbegin(), str.cend(), match, JSON_NUMBER_PATTERN, TOKEN_MATCH_FLAGS)) {
			//数値
			double v = std::stod(match[1].str());
			parseContext.Offset(match.length());
			return std::shared_ptr<JsonPrimitive>(new JsonPrimitive(v));
		}
		else if (std::regex_search(str.cbegin(), str.cend(), match, JSON_STRING_PATTERN, TOKEN_MATCH_FLAGS)) {
			//文字列
			parseContext.Offset(match.length());
			std::string body = match[1].str();
			
			//エスケープを解除
			RemoveEscape(body);
			return std::shared_ptr<JsonString>(new JsonString(body));
		}
		else if (std::regex_search(str.cbegin(), str.cend(), match, JSON_TRUE_PATTERN, TOKEN_MATCH_FLAGS)) {
			//true
			parseContext.Offset(match.length());
			return std::shared_ptr<JsonPrimitive>(new JsonPrimitive(true));
		}
		else if (std::regex_search(str.cbegin(), str.cend(), match, JSON_FALSE_PATTERN, TOKEN_MATCH_FLAGS)) {
			//false
			parseContext.Offset(match.length());
			return std::shared_ptr<JsonPrimitive>(new JsonPrimitive(false));
		}
		else if (std::regex_search(str.cbegin(), str.cend(), match, JSON_NULL_PATTERN, TOKEN_MATCH_FLAGS)) {
			//null
			parseContext.Offset(match.length());
			return std::shared_ptr<JsonPrimitive>(new JsonPrimitive());
		}
		else if (std::regex_search(str.cbegin(), str.cend(), match, JSON_ARRAY_BEGIN_PATTERN, TOKEN_MATCH_FLAGS)) {
			parseContext.Offset(match.length());
			return DeserializeArray(parseContext);
		}
		else if (std::regex_search(str.cbegin(), str.cend(), match, JSON_OBJECT_BEGIN_PATTERN, TOKEN_MATCH_FLAGS)) {
			parseContext.Offset(match.length());
			return DeserializeObject(parseContext);
		}
		else {
			//不正
			parseContext.Error();
			return nullptr;
		}
	}


	std::shared_ptr<JsonArray> DeserializeArray(JsonParseContext& parseContext) {

		if (parseContext.HasError()) {
			return nullptr;
		}

		std::shared_ptr<JsonArray> result(new JsonArray());

		//からっぽの場合
		std::match_results<std::string_view::const_iterator> match;
		if (std::regex_search(parseContext.GetCurrent().cbegin(), parseContext.GetCurrent().cend(), match, JSON_ARRAY_END_PATTERN, TOKEN_MATCH_FLAGS)) {
			parseContext.Offset(match.length());
			return result;
		}

		while (!parseContext.HasError()) {

			//アイテム
			std::shared_ptr<JsonTokenBase> token = DeserializeToken(parseContext);
			result->Add(token);

			//カンマもしくは終端
			if (std::regex_search(parseContext.GetCurrent().cbegin(), parseContext.GetCurrent().cend(), match, JSON_ARRAY_END_PATTERN, TOKEN_MATCH_FLAGS)) {
				parseContext.Offset(match.length());
				return result;
			}
			else if (std::regex_search(parseContext.GetCurrent().cbegin(), parseContext.GetCurrent().cend(), match, JSON_OBJECT_COMMA_PATTERN, TOKEN_MATCH_FLAGS)) {
				parseContext.Offset(match.length());
				continue;
			}

			//ヒットしないとおかしい
			break;
		}

		parseContext.Error();
		return nullptr;
	}

	std::shared_ptr<JsonObject> DeserializeObject(JsonParseContext& parseContext) {

		std::shared_ptr<JsonObject> result(new JsonObject());
		std::match_results<std::string_view::const_iterator> match;

		//からっぽの場合
		if (std::regex_search(parseContext.GetCurrent().cbegin(), parseContext.GetCurrent().cend(), match, JSON_OBJECT_END_PATTERN, TOKEN_MATCH_FLAGS)) {
			parseContext.Offset(match.length());
			return result;
		}

		while (!parseContext.HasError()) {

			//key
			auto key = DeserializeToken(parseContext);

			if (parseContext.HasError()) {
				return nullptr;
			}

			//文字列じゃないとエラー
			if (key->GetType() != JsonTokenType::String) {
				parseContext.HasError();
				return nullptr;
			}

			//カンマ
			if (!std::regex_search(parseContext.GetCurrent().cbegin(), parseContext.GetCurrent().cend(), match, JSON_OBJECT_COLON_PATTERN, TOKEN_MATCH_FLAGS)) {
				//カンマじゃないとエラー
				parseContext.Error();
				return nullptr;
			}
			parseContext.Offset(match.length());

			//value
			auto value = DeserializeToken(parseContext);

			if (parseContext.HasError()) {
				return nullptr;
			}

			//内容を追加
			result->Add(std::static_pointer_cast<JsonString>(key)->GetString(), value);

			if (std::regex_search(parseContext.GetCurrent().cbegin(), parseContext.GetCurrent().cend(), match, JSON_OBJECT_END_PATTERN, TOKEN_MATCH_FLAGS)) {
				parseContext.Offset(match.length());
				return result;
			}
			else if (std::regex_search(parseContext.GetCurrent().cbegin(), parseContext.GetCurrent().cend(), match, JSON_OBJECT_COMMA_PATTERN, TOKEN_MATCH_FLAGS)) {
				parseContext.Offset(match.length());
				continue;
			}

			//ヒットしないとエラー
			parseContext.Error();
			return nullptr;

		}
		return result;
	}

	//jsonシリアライズ
	void SerializeJson(const std::shared_ptr<JsonTokenBase>& token, std::string& result) {
		switch (token->GetType()) {
			case JsonTokenType::Number:
				result.append(std::to_string(std::static_pointer_cast<JsonPrimitive>(token)->GetNumber()));
				break;
			case JsonTokenType::Boolean:
				result.append(std::static_pointer_cast<JsonPrimitive>(token)->GetBoolean() ? "true" : "false");
				break;
			case JsonTokenType::Null:
				result.append("null");
				break;
			case JsonTokenType::String:
				result.push_back('"');
				{
					//ダブルクォーテーションのエスケープ処理
					std::string body = std::static_pointer_cast<JsonString>(token)->GetString();
					AddEscape(body);
					result.append(body);
				}
				result.push_back('"');
				break;
			case JsonTokenType::Object:
				SerializeObject(std::static_pointer_cast<JsonObject>(token), result);
				break;
			case JsonTokenType::Array:
				SerializeArray(std::static_pointer_cast<JsonArray>(token), result);
				break;
		}
	}

	//Arrayシリアライズ
	void SerializeArray(const std::shared_ptr<JsonArray>& token, std::string& result) {

		if (token->GetCount() == 0) {
			result.append("[]");
		}
		else {
			result.push_back('[');

			bool requireComma = false;
			for (const auto& item : token->GetCollection()) {
				if (requireComma) {
					result.push_back(',');
				}
				SerializeJson(item, result);
				requireComma = true;
			}

			result.push_back(']');
		}
	}

	//Objectシリアライズ
	void SerializeObject(const std::shared_ptr<JsonObject>& token, std::string& result) {
		if (token->GetCount() == 0) {
			result.append("{}");
		}
		else {
			result.push_back('{');

			bool requireComma = false;
			for (const auto& item : token->GetCollection()) {
				if (requireComma) {
					result.push_back(',');
				}
				requireComma = true;
				result.push_back('"');
				result.append(item.first);
				result.push_back('"');
				result.push_back(':');
				SerializeJson(item.second, result);
			}

			result.push_back('}');
		}
	}

	//jsonデシリアライズ
	JsonDeserializeResult JsonSerializer::Deserialize(const std::string& json) {
		JsonParseContext context(json);
		JsonDeserializeResult result;
		auto token = DeserializeToken(context);

		if (!context.HasError()) {
			result.success = true;
			result.token = token;
		}
		else {
			result.success = false;
			result.token = nullptr;
		}
		return result;
	}

	std::string JsonSerializer::Serialize(const std::shared_ptr<JsonTokenBase>& token) {
		std::string result;
		SerializeJson(token, result);
		return result;
	}

	bool JsonSerializer::As(const std::shared_ptr<JsonTokenBase>& token, std::string& value) {
		if (token->GetType() == JsonTokenType::String) {
			value = std::static_pointer_cast<JsonString>(token)->GetString();
			return true;
		}
		return false;
	}

	bool JsonSerializer::As(const std::shared_ptr<JsonTokenBase>& token, bool& value) {
		if (token->GetType() == JsonTokenType::Boolean) {
			value = std::static_pointer_cast<JsonPrimitive>(token)->GetBoolean();
			return true;
		}
		return false;
	}

	bool JsonSerializer::As(const std::shared_ptr<JsonTokenBase>& token, double& value) {
		if (token->GetType() == JsonTokenType::Number) {
			value = std::static_pointer_cast<JsonPrimitive>(token)->GetNumber();
			return true;
		}
		return false;
	}

	bool JsonSerializer::As(const std::shared_ptr<JsonTokenBase>& token, uint32_t& value) {
		if (token->GetType() == JsonTokenType::Number) {
			number v = std::static_pointer_cast<JsonPrimitive>(token)->GetNumber();
			value = static_cast<uint32_t>(v);
			return true;
		}
		return false;
	}

	bool JsonSerializer::As(const std::shared_ptr<JsonTokenBase>& token, std::shared_ptr<JsonArray>& value) {
		if (token->GetType() == JsonTokenType::Array) {
			value = std::static_pointer_cast<JsonArray>(token);
			return true;
		}
		return false;
	}

	bool JsonSerializer::As(const std::shared_ptr<JsonTokenBase>& token, std::shared_ptr<JsonObject>& value) {
		if (token->GetType() == JsonTokenType::Object) {
			value = std::static_pointer_cast<JsonObject>(token);
			return true;
		}
		return false;
	}

	//てすと
	void JsonTest() {
		const std::string json = R"({"test": "test"})";

		auto deserialized = JsonSerializer::Deserialize(json);
		std::string serialized;
		SerializeJson(deserialized.token, serialized);
		printf("%s", serialized.c_str());
	}

}