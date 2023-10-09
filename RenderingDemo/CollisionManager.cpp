#include <stdafx.h>
#include <CollisionManager.h>

CollisionManager::CollisionManager(ComPtr<ID3D12Device> _dev, std::vector<ResourceManager*> _resourceManagers)
{
	dev = _dev;
	resourceManager = _resourceManagers;
	Init();
}

// TODO : 1. シェーダーを分けて、ボーンマトリックスとの乗算をなくす&エッジのみ着色したボックスとして表示する、2. 8頂点の位置を正す 3. 複数のメッシュ(障害物)とキャラクターメッシュを判別して処理出来るようにする
void CollisionManager::Init()
{
	auto vertmap1 = resourceManager[0]->GetIndiceAndVertexInfo();
	boxes.resize(vertmap1.size());
	auto it = vertmap1.begin();
	std::map<std::string, std::vector<XMFLOAT3>> vertMaps;
	for (int i = 0; i < vertmap1.size(); ++i)
	{
		for (int j = 0; j < it->second.vertices.size(); ++j)
		{
			// オブジェクトが原点からオフセットしている場合、コライダーがx軸に対して対称の位置に配置される。これを調整してモデル描画の位置をコライダーと同位置に変えている。
			it->second.vertices[j].pos.z *= -1;
			vertMaps[it->first].push_back(it->second.vertices[j].pos);
		}
		it++;
	}

	// オブジェクトの頂点情報からそれぞれのOBBを生成する
	auto itVertMap = vertMaps.begin();
	for (int i = 0; i < vertmap1.size(); ++i)
	{
		BoundingOrientedBox::CreateFromPoints(boxes[i], itVertMap->second.size(), itVertMap->second.data(), (size_t)sizeof(XMFLOAT3));
		++itVertMap;
	}

	// fbxモデルのxyzローカル回転・平行移動行列群を取得
	auto localTransitionAndRotation = resourceManager[0]->GetLocalMatrix();
	//output1.resize(8 * vertMaps.size());
	oBBVertices.resize(vertMaps.size());

	// 各OBBをクォータニオンにより回転させ、Extentsを調整し、その頂点群を描画目的で格納していく
	for (int i = 0; i < vertMaps.size(); ++i)
	{
		// fbxモデルのxyzローカル回転・平行移動行列からクォータニオン生成
		XMVECTOR quaternion = XMQuaternionRotationMatrix(localTransitionAndRotation[i]);

		// OBBの頂点を回転させる。クォータニオンなのでOBB中心点に基づき姿勢が変化する。ワールド空間原点を中心とした回転ではないことに注意。
		XMFLOAT4 orientation;
		orientation.x = quaternion.m128_f32[0];
		orientation.y = -quaternion.m128_f32[1];
		orientation.z = quaternion.m128_f32[2];
		orientation.w = quaternion.m128_f32[3];
		boxes[i].Orientation = orientation;
		// ExtentsはY軸回転により変化する→signθ+cosθ　これによりOBBが肥大化するため調整する。現状はY軸変化のみ対応しているので、Y軸長さをコピーして対応する。
		auto yLen = boxes[i].Extents.y;
		boxes[i].Extents.x = yLen;
		boxes[i].Extents.z = yLen;
		boxes[i].GetCorners(oBBVertices[i].pos);
	}
	// メッシュ描画に対してx軸対称の位置に配置されるコライダーを調整したが、元に戻して描画位置と同じ位置にコライダーを描画させる。(コライダー位置には影響無いことに注意)
	for (int i = 0; i < vertMaps.size(); ++i)
	{
		for (int j = 0; j < sizeof(oBBVertices[i].pos) / sizeof(XMFLOAT3); ++j)
		{
			oBBVertices[i].pos[j].z *= -1;
		}
	}


	// キャラクター用のスフィアコライダー生成
	auto vetmap2 = resourceManager[1]->GetIndiceAndVertexInfo();
	for (int j = 0; j < 4454; ++j)
	{
		input2.push_back(vetmap2.begin()->second.vertices[j].pos);
	}
	BoundingSphere::CreateFromPoints(bSphere, 4454, input2.data(), (size_t)sizeof(XMFLOAT3));
	bSphere.Radius -= 0.2f; // 球体コライダーの半径を微調整。数値適当

	// 操作キャラクターの球体コライダー作成
	CreateSpherePoints(bSphere.Center, bSphere.Radius);

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(oBBVertices));
		
	// バッファー作成1
	auto result = dev->CreateCommittedResource
	(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(boxBuff1.ReleaseAndGetAddressOf())
	);

	// ビュー作成
	boxVBV1.BufferLocation = boxBuff1->GetGPUVirtualAddress();
	boxVBV1.SizeInBytes = sizeof(oBBVertices);
	boxVBV1.StrideInBytes = sizeof(XMFLOAT3);

	// マッピング
	boxBuff1->Map(0, nullptr, (void**)&mappedBox1);
	for (int i = 0; i < oBBVertices.size(); ++i)
	{
		for(int j = 0; j < sizeof(oBBVertices[i].pos) / sizeof(XMFLOAT3); ++j)
		{
			mappedBox1 = &oBBVertices[i].pos[j];
			++mappedBox1;
		}
	}
	//boxBuff1->Unmap(0, nullptr);

	// バッファー作成2
	resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(/*output2*/output3));
	result = dev->CreateCommittedResource
	(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(boxBuff2.ReleaseAndGetAddressOf())
	);

	// ビュー作成
	boxVBV2.BufferLocation = boxBuff2->GetGPUVirtualAddress();
	boxVBV2.SizeInBytes = sizeof(output3);
	boxVBV2.StrideInBytes = sizeof(XMFLOAT3);

	// マッピング
	boxBuff2->Map(0, nullptr, (void**)&mappedBox2);
	std::copy(std::begin(output3), std::end(output3), mappedBox2);
	//boxBuff2->Unmap(0, nullptr);
}

void CollisionManager::MoveCharacterBoundingBox(double speed, XMMATRIX charaDirection)
{
	auto tempCenterPos = XMLoadFloat3(&bSphere.Center);
	tempCenterPos.m128_f32[3] = 1;

	// 平行移動成分にキャラクターの向きから回転成分を乗算して方向変え。これによる回転移動成分は不要なので、1と0にする。Y軸回転のみ対応している。
	auto moveMatrix = XMMatrixMultiply(XMMatrixTranslation(0, 0, -speed), charaDirection);
	moveMatrix.r[0].m128_f32[0] = 1;
	moveMatrix.r[0].m128_f32[2] = 0;
	moveMatrix.r[2].m128_f32[0] = 0;
	moveMatrix.r[2].m128_f32[2] = 1;
	tempCenterPos = XMVector4Transform(tempCenterPos, moveMatrix); // 符号注意

	bSphere.Center.x = tempCenterPos.m128_f32[0];
	bSphere.Center.y = tempCenterPos.m128_f32[1];
	bSphere.Center.z = tempCenterPos.m128_f32[2];

	// Debug
	//printf("%f\n", bSphere.Center.x);
	//printf("%f\n", bSphere.Center.y);
	//printf("%f\n", bSphere.Center.z);
	//printf("\n");

	//printf("%d\n", box1.Contains(bSphere));
}

void CollisionManager::CreateSpherePoints(const XMFLOAT3& center, float Radius)
{
	int div = 8;
	int loopStartCnt = 2;
	int loopEndCnt = 2 + div;
	
	// 天
	output3[0].x = center.x;
	output3[0].y = center.y + Radius;
	output3[0].z = center.z;

	// 地
	output3[1].x = center.x;
	output3[1].y = center.y - Radius;
	output3[1].z = center.z;

	// 水平
	for (int i = loopStartCnt; i < loopEndCnt; ++i)
	{		
		output3[i].x = center.x + Radius * cosf(XMConvertToRadians(360 / div * i));
		output3[i].y = center.y;
		output3[i].z = center.z + Radius * sinf(XMConvertToRadians(360 / div * i));
	}
	loopStartCnt += div;
	loopEndCnt += div;

	float halfR = Radius * cosf(XMConvertToRadians(45));

	// 半天	
	for (int i = loopStartCnt; i < loopEndCnt; ++i)
	{
		output3[i].x = center.x + halfR * cosf(XMConvertToRadians(360 / div * i));
		output3[i].y = center.y + halfR;
		output3[i].z = center.z + halfR * sinf(XMConvertToRadians(360 / div * i));
	}
	loopStartCnt += div;
	loopEndCnt += div;

	// 半地
	for (int i = loopStartCnt; i < loopEndCnt; ++i)
	{
		output3[i].x = center.x + halfR * cosf(XMConvertToRadians(360 / div * i));
		output3[i].y = center.y - halfR;
		output3[i].z = center.z + halfR * sinf(XMConvertToRadians(360 / div * i));
	}
}