#pragma once
using namespace DirectX;

#pragma pack(1)
struct VMDMotion
{
	char boneName[15]; // ボーン名
	unsigned int frameNo; // フレーム番号(読込時は現在のフレーム位置を0とした相対位置)
	XMFLOAT3 location; // 位置
	XMFLOAT4 quaternion; // Quaternion // 回転
	unsigned char bezier[64]; // [4][4][4] // 補完
};
#pragma pack()

// モーション構造体
struct KeyFrame
{
	unsigned int frameNo; // フレーム番号
	XMVECTOR quaternion; // クォータニオン
	XMFLOAT3 location; // IK初期位置からのオフセット情報
	//XMFLOAT3 offset; // IK初期位置からのオフセット情報
	DirectX::XMFLOAT2 p1, p2; // ベジェ曲線の中間制御点
	KeyFrame(unsigned int fno, XMVECTOR q, XMFLOAT2 ip1, XMFLOAT2 ip2) : frameNo(fno), quaternion(q), p1(ip1), p2(ip2)
	{} // 並行情報無しver
	KeyFrame(unsigned int fno, XMVECTOR q, XMFLOAT3 loc , XMFLOAT2 ip1, XMFLOAT2 ip2) : frameNo(fno), quaternion(q),location(loc), p1(ip1), p2(ip2)
	{} // 並行情報有りver
};

class VMDMotionInfo
{
private:
	unsigned int motionDataNum;
	std::vector<VMDMotion> vmdMotions;
	std::unordered_map<std::string, std::vector<KeyFrame>> _motionData;

public:
	HRESULT ReadVMDHeaderFile(std::string);
	std::unordered_map<std::string, std::vector<KeyFrame>> GetMotionData();
};