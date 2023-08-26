#include <stdafx.h>
#pragma comment(lib, "winmm.lib")

namespace
{
	enum class BoneType
	{
		Rotation,		// 回転
		RotAndMove,		// 回転＆移動
		IK,				// IK
		Undefine,		// 未定義
		IKChild,		// IK影響ボーン
		RotationChild,	// 回転影響ボーン
		IKDestination,	// IK接続先
		Invisible		// 不可視ボーン
	};
}

PMDActor::PMDActor(PMDMaterialInfo* _pmdMatInfo, VMDMotionInfo* _vmdMotionInfo)
{
	pmdMaterialInfo = new PMDMaterialInfo;
	vmdMotionInfo = new VMDMotionInfo;

	pmdMaterialInfo = _pmdMatInfo;
	vmdMotionInfo = _vmdMotionInfo;

	//boneMatrices = pmdMaterialInfo->GetBoneMatrices();
	pmdBonesNum = pmdMaterialInfo->GetNumberOfBones();
	boneMatrices.resize(pmdBonesNum);
	std::fill(boneMatrices.begin(), boneMatrices.end(), XMMatrixIdentity());
	bNodeTable = pmdMaterialInfo->GetBoneNode();
	kneeIdxs = pmdMaterialInfo->GetKneeIdx();
	ikData = pmdMaterialInfo->GetpPMDIKData();
	boneNodeAddressArray = pmdMaterialInfo->GetBoneNodeAddressArray();
	for (auto& ik : ikData) 
	{
		if (ik.nodeIdx.size() == 1)
		{
			lookAtSolvedIK.push_back(ik.nodeIdx[0]);
		}		
	}	

	epsilon = 0.005f;
}

void PMDActor::UpdateVMDMotion()
{
	_duration = 0;
	//行列情報クリア(してないと前フレームのポーズが重ね掛けされてモデルが壊れる)
	std::fill(boneMatrices.begin(), boneMatrices.end(), XMMatrixIdentity());
	
	for (auto& boneMotion : vmdMotionInfo->GetMotionData())
	{
		// キーフレームの順番を昇順に並び替える
		std::sort(
			boneMotion.second.begin(), boneMotion.second.end(),
			[](const KeyFrame& lval, const KeyFrame& rval)
			{
				return lval.frameNo <= rval.frameNo;
			}
		);
		
		// 最大フレーム番号取得
		_duration = std::max<unsigned int>(_duration, boneMotion.second[boneMotion.second.size() - 1].frameNo);
		auto itBoneNode = bNodeTable.find(boneMotion.first);
		if (itBoneNode == bNodeTable.end())
		{
			continue;
		}

		// 合致するものを探す

		//auto node = bNodeTable[boneMotion.first];
		auto node = itBoneNode->second;
		auto keyFrames = boneMotion.second;
		auto rit = std::find_if(
			keyFrames.rbegin(), keyFrames.rend(),
			[this](const KeyFrame keyFrame)
			{
				return keyFrame.frameNo <= frameNo;
			});

		if (rit == keyFrames.rend())
		{
			continue;
		}

		XMMATRIX rotation = XMMatrixIdentity();
		XMVECTOR offset = XMLoadFloat3(&rit->location);

		auto it = rit.base();
		if (it != keyFrames.end())
		{
			auto t = static_cast<float>(frameNo - rit->frameNo)
				/ static_cast<float>(it->frameNo - rit->frameNo);

			t = GetYFromXOnBezier(t, it->p1, it->p2, 12);
			// 線形補間
			//rotation = XMMatrixRotationQuaternion(rit->quaternion) * (1 - t)
			//	+ XMMatrixRotationQuaternion(it->quaternion) * t;

			// 球面線形補間
			rotation = XMMatrixRotationQuaternion(XMQuaternionSlerp(rit->quaternion, it->quaternion, t));
			offset = XMVectorLerp(offset, XMLoadFloat3(&it->location), t);
		}
		else
		{
			rotation = XMMatrixRotationQuaternion(rit->quaternion);
		}

		auto& pos = node.startPos;
		auto mat = XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
			* rotation
			* XMMatrixTranslation(pos.x, pos.y, pos.z);
		auto offsetMat = XMMatrixTranslationFromVector(offset);
		boneMatrices[node.boneIdx] = mat * offsetMat;
		//boneMatrices[node.boneIdx] = mat * XMMatrixTranslationFromVector(offset);

		// debug
		//if (node.boneIdx == 83 & frameNo > 65)
		//{
		//	int i = 0;
		//}
	}
	RecursiveMatrixMultiply(&bNodeTable["センター"], XMMatrixIdentity());
	IKSolve();
}

void PMDActor::RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat)
{
	boneMatrices[node->boneIdx] *= mat;

	for (auto cnode : node->children)
	{
		RecursiveMatrixMultiply(cnode, boneMatrices[node->boneIdx]);
	}
}

XMMATRIX PMDActor::LookAtMatrix(const XMVECTOR& lookat, XMFLOAT3& up, XMFLOAT3& right)
{
	// 向かせたい方向(Z軸ベクトル)
	XMVECTOR vz = lookat;

	// (向かせたい方向を向かせたときの)仮Y軸
	XMVECTOR vy = XMVector3Normalize(XMLoadFloat3(&up));

	// 向かせたい方向を向かせたときのX軸を計算
	XMVECTOR vx = XMVector3Normalize(XMVector3Cross(vy, vz));

	// 真のY軸を計算
	vy = XMVector3Cross(vz, vx);

	// もしLookAtとupが同じ方向を向いていたらrightを基準にして作り直す
	if (std::abs(XMVector3Dot(vy, vz).m128_f32[0]) == 1.0f)
	{
		// (向かせたい方向を向かせたときの)仮Xベクトル
		vx = XMVector3Normalize(XMLoadFloat3(&right));

		// 向かせたい方向を向かせたときのY軸を計算
		vy = XMVector3Normalize(XMVector3Cross(vz, vx));

		//真のX軸を計算
		vx = XMVector3Normalize(XMVector3Cross(vy, vz));
	}

	XMMATRIX ret = XMMatrixIdentity();
	ret.r[0] = vx;
	ret.r[1] = vy;
	ret.r[2] = vz;
	return ret;
}

XMMATRIX PMDActor::LookAtMatrix(const XMVECTOR& origin, const XMVECTOR& lookat, XMFLOAT3& up, XMFLOAT3& right) {
	// 逆行列は前回のLookAt行列による回転を打ち消すため。そして改めて導出した行列でLookAtしている。
	return XMMatrixTranspose(LookAtMatrix(origin, up, right)) *	LookAtMatrix(lookat, up, right);
}

void PMDActor::IKSolve()
{
	// Y軸移動のみ、Z軸移動のみに対応。(これ以上は回転軸の改善(クォータニオン対応?)が必要か...)
	for (auto& ik : ikData)
	{
		auto childrenNodeCount = ik.nodeIdx.size();
		switch (childrenNodeCount)
		{
		case 0: // 間のボーンが0(ケース無し)
			continue;
		case 1: // 間のボーンが1の時はLookAt
			SolveLookAtIK(ik);
			break;
		case 2: // 間のボーンが2の時は余弦定理
			SolveCosineIK(ik);
			break;
		default: // 間のボーンが3の時はCCD-IK
			SolveCCDIK(ik);
		}
	}
}

void PMDActor::SolveCCDIK(const PMDIK& ik)
{
	// IKボーン(動かすボーン)の各変数
	auto targetBoneNode = boneNodeAddressArray[ik.boneidx];
	auto targetOriginPos = XMLoadFloat3(&targetBoneNode->startPos);

	auto parentMat = boneMatrices[boneNodeAddressArray[ik.boneidx]->ikParentBone];
	XMVECTOR det;
	auto invParentMat = XMMatrixInverse(&det, parentMat);

	// invParentsMatによる変位だとIKボーンの変位が皆無になる。これは"初音ミク.pmd"におけるリグ構成では髪とﾈｸﾀｲIKボーンに
	// 対してセンターボーンが親となっており、このセンターボーンの変位による影響を逆行列乗算により打ち消すことになり、その結果を
	// bornMatrices[]に上書きしているから。この関数に入る前に、Recursive...により乗算された行列が格納されている)。
	// boneMatrices[0] * invParentMatによる変位は0である。=targetOriginPosであるが、bornMatrices[]と結びついていないため
	// Recursive...()による演算によりセンターボーンの子であるIKボーンは変位する。さらにいうとIK children達もセンターボーン
	// による変位と分離されている
	//auto targetNextPos = XMVector3Transform(targetOriginPos, invParentMat);
	//auto targetNextPos = XMVector3Transform(targetOriginPos, boneMatrices[0] * invParentMat);
	auto targetNextPos = targetOriginPos;

	// 末端ボーンの初期位置
	auto endPos = XMLoadFloat3(&boneNodeAddressArray[ik.targetidx]->startPos);

	bonePositions.clear();
	// 中間ボーンの初期位置
	for (auto& cidx : ik.nodeIdx)
	{
		bonePositions.push_back(XMLoadFloat3(&boneNodeAddressArray[cidx]->startPos));
	}

	// 回転行列の記録用。4*4の単位行列で埋めておく。
	std::vector<XMMATRIX> mats(bonePositions.size());
	fill(mats.begin(), mats.end(), XMMatrixIdentity());

	auto ikLimit = ik.limit * XM_PI; // PMDデータでは角度=度数法/180°で表現されているので調整している。

	// ikに設定されている試行回数繰り返しCCD計算を行う
	for (int c = 0; c < ik.iterations; ++c)
	{
		// 末端ボーンとターゲット(IKボーン)が一致したらイテレーションを抜ける
		if (XMVector3Length(
			XMVectorSubtract(endPos, targetNextPos)).m128_f32[0] <= epsilon)
		{
			break;
		}

		// それぞれのボーンを遡りながら角度制限に抵触しないように曲げていく
		// bonePositionsはCCD-IKにおける各ノードの座標をベクタ配列にしたもの
		for (int bidx = 0; bidx < bonePositions.size(); ++bidx)
		{
			const auto& pos = bonePositions[bidx];
			// 対象ボーンから末端ボーンまでと対象ベクトルから
			// ターゲットまでのベクトル作成
			auto vecToEnd = XMVectorSubtract(endPos, pos); // 対象ボーン→末端ボーン
			auto vecToTarget = XMVectorSubtract(targetNextPos, pos); // 対象ボーン→ターゲット(IK)

			// 正規化
			vecToEnd = XMVector3Normalize(vecToEnd);
			vecToTarget = XMVector3Normalize(vecToTarget);

			// ほぼ同じベクトルになった場合は外積が計算できないので次の処理へ
			if (XMVector3Length(XMVectorSubtract(vecToEnd, vecToTarget)).m128_f32[0] <= epsilon)
			{
				continue;
			}

			// 外積及び角度計算
			auto cross = XMVector3Cross(vecToEnd, vecToTarget);
			cross = XMVector3Normalize(cross);

			float angle = XMVector2AngleBetweenVectors(vecToEnd, vecToTarget).m128_f32[0]; // 0°～90°、-90°～0°の判別不能

			// 回転限界を超えたときの補正
			//angle = min(angle, ikLimit);
			if (angle < ikLimit) angle = angle;
			else angle = ikLimit;

			// 外積と角度から回転行列の算出
			XMMATRIX rot = XMMatrixRotationAxis(cross, angle);

			// 原点ではなくpos中心に回転させるため、回転行列の前後に並行移動行列の逆行列、行列を乗算する。
			// 例えばﾈｸﾀｲ3の処理時に-posすると、ﾈｸﾀｲ3の座標が原点に移動することになり、これによってpos中心の回転という意味になる。
			auto mat = 
				XMMatrixTranslationFromVector(-pos)
				* rot
				* XMMatrixTranslationFromVector(pos);

			// 行列の記録
			mats[bidx] *= mat;

			// 行列の適用は現在のボーンからみて末端側の中間ボーンが対象
			for (int idx = bidx - 1; idx >= 0; --idx)
			{
				bonePositions[idx] = XMVector3Transform(bonePositions[idx], mat);
			}

			// 末端ボーンにも行列を適用
			endPos = XMVector3Transform(endPos, mat);

			// 末端→IKのベクトル長が閾値未満なら処理終了
			if (XMVector3Length(XMVectorSubtract(endPos, targetNextPos)).m128_f32[0] <= epsilon)
			{
				break;
			}				
		}
	}

	// 計算過程で格納しておいた行列をboneMatricesの各アドレスに上書きしていく
	// センターボーンによる影響を考慮していない結果になっている。
	int idx = 0;
	for (auto& cidx : ik.nodeIdx)
	{
		boneMatrices[cidx] = mats[idx];
		++idx;
	}
	
	// ルートボーンから末端ボーンへIKボーンの親ボーン("初音ミク.pmd"では「センター」ボーン)行列を乗算して、
	// その結果を再帰的に乗算していく。つまり、センターボーンを考慮しない結果をセンターボーンによる影響と結びつけることで、
	// あるべき結果(行列)をボーンに適用していく。
	auto rootNode = pmdMaterialInfo->GetBoneNodeAddressArray()[ik.nodeIdx.back()];
	RecursiveMatrixMultiply(rootNode, parentMat);
}

void PMDActor::SolveCosineIK(const PMDIK& ik)
{
	// ターゲットはIKボーン(末端ボーンではなく、末端ボーンが近づく目標ボーン(IKボーン)の座標を取得)
	auto& targetNode = boneNodeAddressArray[ik.boneidx];
	// IKボーンの変位後座標を格納
	auto targetPos = XMVector3Transform(XMLoadFloat3(&targetNode->startPos), (boneMatrices[ik.boneidx]));

	// 末端ボーン
	auto endNode = boneNodeAddressArray[ik.targetidx];
	positions.clear();
	positions.emplace_back(XMLoadFloat3(&endNode->startPos));
	//positions.emplace_back(XMVector3Transform(XMLoadFloat3(&endNode->startPos), boneMatrices[ik.boneidx]));

	// 教科書に無かったので追加。末端ボーン行列には関数に入る前にセンターボーンの回転移動行列を乗算されているので、相殺しておく。
	auto parentMat = boneMatrices[0];
	XMVECTOR det;
	auto invParentMat = XMMatrixInverse(&det, parentMat);
	boneMatrices[ik.targetidx] *= invParentMat;

	auto targetNextPos = XMVector3Transform(XMLoadFloat3(&boneNodeAddressArray[ik.boneidx]->startPos), boneMatrices[ik.boneidx]);
	//targetPos = targetNextPos;

	// 中間及びルートノード(MMDデータの構成上、参照は中間→ルートの順になる)
	for (auto& chainBoneNode : ik.nodeIdx)
	{
		auto boneNode = boneNodeAddressArray[chainBoneNode];
		positions.emplace_back(XMLoadFloat3(&boneNode->startPos));
	}

	// positions内部のデータ順が末端→中間→ルートノードなので、分かりやすくするため逆順にする
	std::reverse(positions.begin(), positions.end()); // ルート→中間→末端の順に変更
	// 後で使うので初期値保存しておく
	auto p0s = positions[0];
	auto p2s = positions[2];

	// 元の長さを測定
	edgeLens[0] = XMVector3Length(XMVectorSubtract(positions[1], positions[0])).m128_f32[0]; // ルート→中間ベクトルの長さ
	edgeLens[1] = XMVector3Length(XMVectorSubtract(positions[2], positions[1])).m128_f32[0]; // 中間→末端ベクトルの長さ

	// ルートボーン座標変換
	positions[0] = XMVector3Transform(positions[0], boneMatrices[ik.nodeIdx[1]]); // ik.nodeIdx[1]はルートボーンを指している
	// positions[1]は自動的に導出される
	// 
	// 末端ボーン
	positions[2] = XMVector3Transform(positions[2], boneMatrices[ik.boneidx]); // 末端ボーンをikボーン行列で座標変換して、目的位置をIKと同様にする
	//positions[2] = XMVector3Transform(positions[2], boneMatrices[ik.targetidx]); // ik.nodeIdx[2]は存在しないのでは？→targetIdxに

	// ルートから末端へのベクトル(座標変換後。初期位置ではないことに注意)を作っておく
	auto linearVec = XMVectorSubtract(positions[2], positions[0]);
	
	//float A = XMVector3Length(linearVec).m128_f32[0]; // 座標変換後のルート→末端ベクトル長さ
	float B = edgeLens[0]; // ルート→中間ベクトルの長さ
	float C = edgeLens[1]; // 中間→末端ベクトルの長さ
	//float A = min(B + C - 0.01f, XMVector3Length(linearVec).m128_f32[0]); // 座標変換後のルート→末端ベクトル長さ
	float A;
	if (B + C - 0.01f < XMVector3Length(linearVec).m128_f32[0]) A = B + C - 0.01f;
	else A = XMVector3Length(linearVec).m128_f32[0];
	// 座標変換後にルート→中間ベクトルとルート→末端ベクトルの成す角度θ1
	float x1 = (A * A + B * B - C * C) / (2 * A * B);
	float theta1 = acosf(x1);

	// 座標変換後にルート→中間ベクトルと中間→末端ベクトルの成す角度θ2
	float x2 = (B * B + C * C - A * A) / (2 * B * C);
	float theta2 = acosf(x2);

	// IKボーンにZ方向の変位がある場合の処理
    // 教科書ベースではikのz方向移動有りモーションが再現出来ないので独自処理として追加
	XMMATRIX rotationOfIKzMove = XMMatrixIdentity();
	XMVECTOR root2IK;
	XMVECTOR root2TargetStart;
	XMVECTOR rootMinusroot2IK;
	float theta3 = 0;
	if (frameNo > 1)
	{
		// 中間ノードにikボーンが近づいてきたら、ルートボーンを原点としてルート→IKの角度分ルート・中間・末端ボーンを回転させる
		float f = beforeIKMat.r[3].m128_f32[2] - boneMatrices[ik.boneidx].r[3].m128_f32[2];
		if (f > 0) 
		{
			root2IK = XMVectorSubtract(targetPos, p0s);//positions[0]);
			root2TargetStart = XMVectorSubtract(XMLoadFloat3(&targetNode->startPos), p0s);//positions[0]);
			rootMinusroot2IK = XMVectorSubtract(root2IK, root2TargetStart);
			float D = XMVector3Length(root2IK).m128_f32[0];
			float E = XMVector3Length(rootMinusroot2IK).m128_f32[0];
			float y = (B * B + D * D - E * E) / (2 * B * D);
			//y = min(1, y); // 1超えるとarccos値が計算できない

			if (1 < y) y = 1;

			theta3 = acosf(y);
		}

		// 中間ノードからikボーンが遠ざかったら、ルートボーンを原点としてルート→IKの角度分ルート・中間・末端ボーンを逆回転させる
		else if(f < 0)
		{
			root2IK = XMVectorSubtract(targetPos, p0s);//positions[0]);
			root2TargetStart = XMVectorSubtract(XMLoadFloat3(&targetNode->startPos), p0s);//positions[0]);
			rootMinusroot2IK = XMVectorSubtract(root2IK, root2TargetStart);
			float D = XMVector3Length(root2IK).m128_f32[0];
			float E = XMVector3Length(rootMinusroot2IK).m128_f32[0];
			float y = (B * B + D * D - E * E) / (2 * B * D);
			//y = min(1, y); // 1超えるとarccos値が計算できない
			if (1 < y) y = 1;
			theta3 = -acosf(y);
		}
	}

	// 軸を求める
	// もし真ん中が「ひざ」であった場合には強制的にX軸とする
	XMVECTOR axis;
	if (find(kneeIdxs.begin(),
		kneeIdxs.end(),
		ik.nodeIdx[0]) == kneeIdxs.end())
	{
		auto vm = XMVector3Normalize(XMVectorSubtract(positions[2], positions[0])); // ルート→末端ベクトル
		auto vt = XMVector3Normalize(XMVectorSubtract(targetPos, positions[0])); // ルート→IKボーンベクトル
		axis = XMVector3Cross(vt, vm);
	}

	else
	{
		auto right = XMFLOAT3(1, 0, 0);
		axis = XMLoadFloat3(&right);
	}

	// Z方向にIKが移動した場合の行列
	rotationOfIKzMove = XMMatrixTranslationFromVector(-positions[0]);
	rotationOfIKzMove *= XMMatrixRotationAxis(axis, theta3);
	rotationOfIKzMove *= XMMatrixTranslationFromVector(positions[0]);

	// Debug
	//if (frameNo > 79)
	//{
	//	int i = 0;
	//}

	auto mat1 = XMMatrixTranslationFromVector(-positions[0]);
	mat1 *= XMMatrixRotationAxis(axis, theta1);
	mat1 *= XMMatrixTranslationFromVector(positions[0]);

	auto mat2 = XMMatrixTranslationFromVector(-positions[1]);
	mat2 *= XMMatrixRotationAxis(axis, theta2 - XM_PI*1.001);  // 中間ボーン=ひじと考えると分かりやすい。曲げた角度は曲がった後の角度-180°
	mat2 *= XMMatrixTranslationFromVector(positions[1]);

	boneMatrices[ik.nodeIdx[1]] *= rotationOfIKzMove * mat1; // ルートボーン行列はルートボーンの回転行列
	boneMatrices[ik.nodeIdx[0]] = mat2 * boneMatrices[ik.nodeIdx[1]]; // 中間ボーン行列は中間ボーン及びルートボーンの回転行列を乗算したもので
	
	// "初音ミク.pmd"専用処理。足IKの移動にしたがってつま先IKを移動させる。
	// IKボーンの子ボーンに行列適用
	for (auto& childIK : boneNodeAddressArray[ik.boneidx]->children)
	{
		boneMatrices[childIK->boneIdx] *= boneMatrices[ik.boneidx];

	}

	 // 実際は末端ボーン行列はルート・中間ボーン行列の影響を受ける。扱っているPMDモデルのリグの組み方によって、足首ボーンは
	 // それぞれ2つのIKチェーンに組み込まれている。それぞれLookAtIKとCosineIK対象である。LookAt側での結果にCosineでの以下計算結果が
	 // 上書きされることで、足首ボーンが望まない動きをしてしまうため、処理を省くことで整合性を合わせている。独自の、その場しのぎの
	 // やり方で改善が必要と思われる。
	auto isExsist = find(lookAtSolvedIK.begin(), lookAtSolvedIK.end(), ik.targetidx) != lookAtSolvedIK.end();
	if (!isExsist)
	{
		boneMatrices[ik.targetidx] = boneMatrices[ik.nodeIdx[0]];
	}
	else
	{
		//boneMatrices[ik.targetidx] = boneMatrices[ik.nodeIdx[0]];// *XMMatrixTranspose(XMMatrixRotationAxis(axis, theta1) * XMMatrixRotationAxis(axis, theta2 - XM_PI));
		boneMatrices[ik.targetidx] *= boneMatrices[ik.boneidx];//boneMatrices[ik.nodeIdx[0]] * XMMatrixTranslationFromVector(targetNextPos);
	}
	
	// 次の処理に向けて情報の保存
	befTheta1 = theta1;
	beforeIKMat = boneMatrices[ik.boneidx];

	// 末端ボーンの子ボーンに行列適用
	for (auto& childIK : boneNodeAddressArray[ik.targetidx]->children)
	{
		boneMatrices[childIK->boneIdx] *= boneMatrices[ik.boneidx];		
	}	
}

void PMDActor::SolveLookAtIK(const PMDIK& ik) 
{
	// root：ルートボーン、target：末端ボーン
	auto rootNode = boneNodeAddressArray[ik.nodeIdx[0]];
	auto targetNode = boneNodeAddressArray[ik.targetidx];

	// 特定方向ベクトル(ルート→末端への方向ベクトルを導出して単位ベクトル化)
	auto rpos1 = XMLoadFloat3(&rootNode->startPos);
	auto tpos1 = XMLoadFloat3(&targetNode->startPos);
	auto originVec = XMVectorSubtract(tpos1, rpos1);
	originVec = XMVector3Normalize(originVec);

	// 任意方向ベクトル(ルート/末端の座標に各々のノードが保持する現在の回転・並行移動行列を乗算した変位後の座標から、
	// 方向ベクトルを導出して単位ベクトル化)
	auto rpos2 = XMVector3TransformCoord(rpos1, boneMatrices[ik.nodeIdx[0]]);
	auto tpos2 = XMVector3TransformCoord(tpos1, boneMatrices[ik.boneidx]); // 末端をIKに移動させたい(ルート→IKベクトルを作りたい)からIK行列を乗算する	
	auto targetVec = XMVectorSubtract(tpos2, rpos2);
	targetVec = XMVector3Normalize(targetVec);

	XMFLOAT3 up = XMFLOAT3(0, 1, 0); // 上ベクトル
	XMFLOAT3 right = XMFLOAT3(1, 0, 0); // 右ベクトル
	auto mat = 
		XMMatrixTranslationFromVector(-rpos2)
		* LookAtMatrix(originVec, targetVec, up, right)
		* XMMatrixTranslationFromVector(rpos2);

	boneMatrices[ik.nodeIdx[0]] *= mat;

	// IKボーンの子ボーンに行列適用
	for (auto& childIK : boneNodeAddressArray[ik.boneidx]->children)
	{
		boneMatrices[childIK->boneIdx] *= boneMatrices[ik.boneidx];

	}
}

void PMDActor::PlayAnimation() 
{
	_startTime = timeGetTime();
}

void PMDActor::MotionUpdate(unsigned int maxFrameNum)
{
	elapsedTime = timeGetTime() - _startTime; // 経過時間を測定して格納
	frameNo = 30 * (elapsedTime / 1000.0f);

	if (frameNo > maxFrameNum)
	{
		PlayAnimation();
		frameNo = 0;
	}
}

unsigned int PMDActor::GetFrameNo()
{
	return frameNo;
}

unsigned int PMDActor::GetDuration()
{
	return _duration;
}

float PMDActor::GetYFromXOnBezier(float x, const XMFLOAT2& a, const XMFLOAT2& b, uint8_t n)
{
	if (a.x == a.y && b.x == b.y)
	{
		return x; // 計算不要
	}

	float t = x;
	const float k0 = 1 + 3 * a.x - 3 * b.x; // t^3係数
	const float k1 = 3 * b.x - 6 * a.x; // t^2係数
	const float k2 = 3 * a.x; // t係数

	// 誤差の範囲内かどうか判定するための定数
	constexpr float epsilon = 0.0005f;

	// tを近似で求める
	for (int i = 0; i < n; ++i) 
	{
		// f(t)を求める
		auto ft = k0 * t * t * t + k1 * t * t + k2 * t - x;

		// もし結果が0に近い(誤差の範囲内)なら打ち切る
		if (ft <= epsilon && ft >= -epsilon)
		{
			break;
		}

		t -= ft / 2; // 半減させる
	}

	// 求めたいtは既に求めているのでyを計算する
	auto r = 1 - t;
	return t * t * t + 3 * t * t * r * b.x + 3 * t * r * r * a.x;
}

std::vector<DirectX::XMMATRIX>* PMDActor::GetMatrices()
{
	return &boneMatrices;
}