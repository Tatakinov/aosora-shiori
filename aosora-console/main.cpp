#include <Windows.h>
#include <filesystem>
#include <iostream>
#include "Shiori.h"
#include "Misc/Message.h"
#include "Misc/ProjectParser.h"

/**
* コンソール版 蒼空
*/
int main(int argc, char* argv[])
{
	//UTF-8を設定
	SetConsoleOutputCP(CP_UTF8);

	//SHIORI同様にaosoraを起動
	sakura::Shiori* aosoraShiori = new sakura::Shiori();
	aosoraShiori->Load(std::filesystem::current_path().string() + "\\");

	//エラーがあればコンソール出力
	if (aosoraShiori->HasScriptLoadError()) {
		for (const auto& item : aosoraShiori->GetScriptLoadErrors()) {
			std::cerr << item.MakeConsoleErrorString() << std::endl;
		}
		return 1;
	}

	if (aosoraShiori->HasBootExecuteError()) {
		std::cerr << aosoraShiori->GetBootingExecuteErrorLog() << std::endl;
		return 1;
	}

	//リクエストを作成して実行
	sakura::ShioriRequest request;
	sakura::ShioriResponse response;

	//アプリケーション本体、イベントID、Reference.. と続くので読む
	//TODO: 実行側の引数をつけられるようにしたい。たとえばセーブデータ書き込みたくないとき aosora --nosave -- OnHelloWorld など
	bool hasEventId = argc >= 2;
	if (hasEventId) {
		request.SetEventId(argv[1]);
		for (int i = 2; i < argc; i++) {
			request.SetReference(i - 2, argv[i]);
		}
	}

	//イベントIDが渡らないなら初期実行までで打ち切りになる
	if (hasEventId) {
		aosoraShiori->Request(request, response);

		if (response.HasError()) {
			for (const auto& item : response.GetErrorCollection()) {
				std::cerr << item.GetErrorMessage() << std::endl;
			}
			return 1;
		}

		//戻り値があればコンソールに流す
		if (!response.GetValue().empty()) {
			std::cout << response.GetValue() << std::endl;
		}
	}

	//アンロード
	aosoraShiori->Unload();
	delete aosoraShiori;

	return 0;
}