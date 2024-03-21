#pragma once

namespace Utility 
{
	//アライメントにそろえたサイズを返す
	//@param size 元のサイズ
	//@param alignment アライメントサイズ
	//@return アライメントを揃えたサイズ
	size_t AlignmentSize(size_t size, size_t alignment);

	void EnableDebugLayer();

	//モデルのパスとテクスチャのパスから合成パスを得る
	//@param modelPath アプリからみたpmdモデルのパス
	//@param texPath pmdモデルからみたテクスチャのパス
	//@param return アプリからみたテクスチャのパス
	std::string GetTexPathFromModeAndTexlPath(const std::string modelPath, const char* texPath);

	// std::string(マルチバイト文字列)→std::wstring(ワイド文字列)を得る
	// @param str マルチバイト文字列
	// return 変換されたワイド文字列
	std::wstring GetWideStringFromSring(const std::string& str);

	//ファイルから拡張子を取得する
	// @param path 対象パス文字列
	// @return 拡張子
	std::string GetExtension(const std::string& path);

	//テクスチャのパスをセパレータ文字で分離する
	// @param path 対象のパス
	// @param splitter 文字列
	// @return 分離前後の文字列ペア
	std::pair<std::string, std::string> SplitFileName(const std::string& path, const char splitter = '*');

	// ガウシアンぼかしの横方向用ウェイトを計算
	// @param count 計算回数
	// @param disp 分散
	// @return ウェイト
	std::vector<float> GetGaussianWeight(size_t count, float disp);

	// hlslのファイルパス作成
	std::pair<LPWSTR, LPWSTR> GetHlslFilepath(std::string _vsName, std::string _psName);

	// テクスチャフォルダへのパス作成
	std::string GetTextureFilepath();

	// モデルバイナリファイルへのパス作成
	std::string GetModelFilepath();
};