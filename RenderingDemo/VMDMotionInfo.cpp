#include <stdafx.h>

HRESULT VMDMotionInfo::ReadVMDHeaderFile(std::string strMotionPath)
{
	// VMDヘッダファイルの読み込み
	auto fp = fopen(strMotionPath.c_str(), "rb");
	if (fp == nullptr) {
		//エラー処理
		assert(0);
		return ERROR_FILE_NOT_FOUND;
	}

	// 最初の50byteは不要なので飛ばす
	fseek(fp, 50, SEEK_SET);

	// データ数読み込み
	motionDataNum = 0;
	fread(&motionDataNum, sizeof(motionDataNum), 1, fp);
	vmdMotions.resize(motionDataNum);

	for (auto& motion : vmdMotions)
	{
		fread(motion.boneName, sizeof(motion.boneName), 1, fp);
		fread(&motion.frameNo, sizeof(motion.frameNo)
			+ sizeof(motion.location)
			+ sizeof(motion.quaternion)
			+ sizeof(motion.bezier), 1, fp);
	}

	// ボーン名とキーフレームのハッシュテーブルを生成
	for (auto& m : vmdMotions)
	{
		_motionData[m.boneName].emplace_back(
			KeyFrame(m.frameNo, XMLoadFloat4(&m.quaternion), m.location,
				XMFLOAT2((float)m.bezier[3] / 127.0f, (float)m.bezier[7] / 127.0f),
				XMFLOAT2((float)m.bezier[11] / 127.0f, (float)m.bezier[15] / 127.0f)));
	}
}

std::unordered_map<std::string, std::vector<KeyFrame>> VMDMotionInfo::GetMotionData()
{
	return _motionData;
}