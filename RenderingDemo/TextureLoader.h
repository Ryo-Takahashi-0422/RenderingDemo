#pragma once

class TextureLoader
{
private:
	std::map<std::string, LoadLambda_t> loadLambdaTable;
	void SetTable();

public:
	//ファイル形式毎のテクスチャロード処理
	void LoadTexture();
	std::map<std::string, LoadLambda_t> GetTable();
};