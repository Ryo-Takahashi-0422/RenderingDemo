#include <stdafx.h>
#include <Utility.h>

void Utility::EnableDebugLayer() {
	ComPtr<ID3D12Debug> debuglayer = nullptr;
	auto result = D3D12GetDebugInterface(IID_PPV_ARGS(debuglayer.ReleaseAndGetAddressOf()));

	debuglayer->EnableDebugLayer();
	debuglayer->Release();//有効化後にインターフェイスを解放する
}

size_t
Utility::AlignmentSize(size_t size, size_t alignment)
{
	// sizeがalignmentの倍数で、alignment以上ならば調整不要
	if (size >= alignment && size % alignment == 0)
	{
		return size;
	}
	// sizeがアラインメント256byte未満なら256byteに調整する。
	// またはsizeがアラインメント256byteより大きければ、それより1つ分大きな256byteの倍数に調整する
	else
	{
		return size + alignment - size % alignment;
	}
}

std::string Utility::GetTexPathFromModeAndTexlPath
(
	const std::string modelPath,
	const char* texPath
)
{
	int pathIndex1 = modelPath.rfind('/');
	int pathIndex2 = modelPath.rfind('\\');
	//auto pathIndex = max(pathIndex1, pathIndex2);
	int pathIndex;
	if (pathIndex1 > pathIndex2) pathIndex = pathIndex1;
	else pathIndex = pathIndex2;

	auto folderPath = modelPath.substr(0, pathIndex + 1); // 末尾の\も取得するため +1

	return folderPath + texPath;
}

std::wstring Utility::GetWideStringFromSring(const std::string& str)
{
	//呼び出し一回目(文字列数を得る)
	auto num1 = MultiByteToWideChar // 関数成功でlpWideCharStrが指すバッファに書き込まれたワイド文字の数が返る
	(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), // 変換する文字列へのポインタを指定
		-1, // pMultiByteStr が指す文字列のサイズをバイト単位で渡す。-1でNULL終端と見なされ、長さが自動的に計算される
		nullptr, // 変換後の文字列を受け取るバッファへのポインタを指定
		0 // lpWideCharStrが指すバッファサイズをワイド文字数の単位で指定。 0で必要なバッファのサイズ(ワイド文字数)が返る
	);

	std::wstring wstr; // stringのwchar_t版
	wstr.resize(num1);

	auto num2 = MultiByteToWideChar
	(
		CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(),
		-1,
		&wstr[0], // wstr[0]からnum1分のワイド文字書き込み
		num1
	);

	assert(num1 == num2);
	return wstr;
}

std::string Utility::GetExtension(const std::string& path)
{
	int idx = path.rfind('.');
	return path.substr(idx + 1, path.length() - idx - 1);
}

std::pair<std::string, std::string> Utility::SplitFileName(
	const std::string& path, const char splitter)
{
	int idx = path.find(splitter);
	std::pair<std::string, std::string> ret;
	ret.first = path.substr(0, idx);
	ret.second = path.substr(idx + 1, path.length() - idx - 1);

	return ret;
}

std::vector<float> Utility::GetGaussianWeight(size_t count, float disp)
{
	std::vector<float> weight(count); // ウェイト配列返却用
	float x = 0.0f;
	float total = 0.0f;
	for (auto& wgt : weight)
	{
		wgt = expf(-(x * x) / (2 * disp * disp));
		total += wgt;
		x += 1.0f;
	}

	total = total * 2 - 1;

	// 足して1になるようにする
	for (auto& wgt : weight)
	{
		wgt /= total;
	}

	return weight;
}