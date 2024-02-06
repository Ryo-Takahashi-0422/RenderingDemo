#include <stdafx.h>
#include <PeraLayout.h>

PeraLayout::PeraLayout()
{
	inputLayout =
	{
		//座標
		{
			"POSITION",
			0, // 同じセマンティクスに対するインデックス
			DXGI_FORMAT_R32G32B32_FLOAT,
			0, // スロットインデックス
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0 // 一度に描画するインスタンス数
		},

		// このシェーダー設定は4点の一枚板ポリをDirectX12に生成させたデータを利用する。
		// そのため、PMDモデル出力設定のようにNORMAL(などのｾﾏﾝﾃｨｸｽ)を追加すると、想像ではあるがデータが存在しないために対応するシェーダー側に
		// 渡るデータポインタがずれてしまい、この場合(ここにNORMAL指定する)TEXCOORD(uv)データが渡らずサンプリングできなくなり表示がバグる。

		//uv
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0, // スロットインデックス
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		}
	};

}


PeraLayout::~PeraLayout()
{
}

std::vector<D3D12_INPUT_ELEMENT_DESC> PeraLayout::GetInputLayout()
{
	return inputLayout;
}

size_t PeraLayout::GetInputSize()
{
	return inputLayout.size();
}