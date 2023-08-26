#pragma once

class PMDActor
{
private:
	DWORD _startTime; // アニメーション開始時のミリ秒
	DWORD elapsedTime; // 経過ミリ秒
	unsigned int frameNo; // 現在のフレームNo
	std::vector<PMDIK> ikData; // ikデータ群
	PMDMaterialInfo* pmdMaterialInfo = nullptr;
	VMDMotionInfo* vmdMotionInfo = nullptr;
	size_t pmdBonesNum;
	std::vector<DirectX::XMMATRIX> boneMatrices;
	std::map<std::string, BoneNode> bNodeTable;
	void RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat);
	unsigned int _duration; // アニメーションの最大フレーム番号
	std::vector<BoneNode*> boneNodeAddressArray;
	float epsilon;

	// CCD-IKによりボーン方向を決定 node構成：root-interminiate(2個以上)-target
	// @param ik 対象IKオブジェクト
	void SolveCCDIK(const PMDIK& ik);
	std::vector<XMVECTOR> bonePositions; // IK構成点を保存

	// 余弦定理IKによりボーン方向を決定 node構成：root-interminiate-target
	// @param ik 対象IKオブジェクト
	void SolveCosineIK(const PMDIK& ik);
	std::vector<XMVECTOR> positions; // IK構成点を保存
	std::array<float, 2> edgeLens; // IKの各ボーン間の距離を保存
	std::vector<uint32_t> kneeIdxs;

	// LOOKAT行列によりボーン方向を決定 node構成：root-target
	// @param ik 対象IKオブジェクト
	void SolveLookAtIK(const PMDIK& ik);
	std::vector<unsigned int> lookAtSolvedIK;

	XMMATRIX beforeIKMat;
	float befTheta1;

public:
	//std::vector<DirectX::XMMATRIX> boneMatrices;
	// コピーコンストラクタ
	PMDActor(PMDMaterialInfo* _pmdMatInfo, VMDMotionInfo* _vmdMotionInfo);
	unsigned int GetFrameNo();
	void PlayAnimation();
	void MotionUpdate(unsigned int maxFrameNum);
	float GetYFromXOnBezier(float x,const XMFLOAT2& a, const XMFLOAT2& b, uint8_t n);
	std::vector<DirectX::XMMATRIX>* GetMatrices();
	unsigned int GetDuration();


	// アニメーション情報を更新してボーン(親)の位置や角度を変化させる
	void UpdateVMDMotion();

	// 描画前にアニメーションによる親ボーンの角度変化を再帰的に子ボーンへ伝達する
	void RecursiveMatrixMultiply(const DirectX::XMMATRIX& mat);

	// z軸を特定の方向に向かせる行列を返す関数
	// @param lookat 向かせたい方向ベクトル
	// @param up 上ベクトル
	// @param right 右ベクトル
	XMMATRIX LookAtMatrix(const XMVECTOR& lookat, XMFLOAT3& up, XMFLOAT3& right);

	///特定のベクトルを特定の方向に向けるための行列を返す
	///@param origin 特定のベクトル
	///@param lookat 向かせたい方向
	///@param up 上ベクトル
	///@param right 右ベクトル
	///@retval 特定のベクトルを特定の方向に向けるための行列
	XMMATRIX LookAtMatrix(const XMVECTOR& origin, const XMVECTOR& lookat, XMFLOAT3& up, XMFLOAT3& right);
	
	// IKの導出パターンを決定する
	void IKSolve();
};