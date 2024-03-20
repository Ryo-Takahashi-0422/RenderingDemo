#include <stdafx.h>
#include <CollisionManager.h>

CollisionManager::CollisionManager(ComPtr<ID3D12Device> _dev, std::vector<ResourceManager*> _resourceManagers)
{
	dev = _dev;
	resourceManager = _resourceManagers;
	Init();
	CreateInfo();
	CreateMatrixHeap();
	CreateMatrixResources();
	CreateMatrixView();
	MappingMatrix();
}

CollisionManager::~CollisionManager()
{
	delete layout;
	layout = nullptr;

	delete colliderGraphicsPipelineSetting;
	colliderGraphicsPipelineSetting = nullptr;

	free(collisionRootSignature);
	collisionRootSignature = nullptr;

	collisionShaderCompile = nullptr;

	mappedSphereIdx = nullptr;
	mappedBox2 = nullptr;
	mappedIdx.clear();
	mappedMatrix = nullptr;
}

HRESULT CollisionManager::Init()
{
	layout = new PeraLayout;
	collisionRootSignature = new CollisionRootSignature;
	colliderGraphicsPipelineSetting = new ColliderGraphicsPipelineSetting(layout);
	collisionShaderCompile = new SettingShaderCompile;

	// ｺﾗｲﾀﾞｰ用
	if (FAILED(collisionRootSignature->SetRootsignatureParam(dev)))
	{
		return false;
	}

	// コライダー描画用
	std::string collisionVs = "CollisionVertex.hlsl";
	std::string collisionPs = "CollisionPixel.hlsl";
	auto collisionPathPair = Utility::GetHlslFilepath(collisionVs, collisionPs);
	auto colliderBlobs = collisionShaderCompile->CollisionSetShaderCompile(collisionRootSignature, _vsCollisionBlob, _psCollisionBlob,
		collisionPathPair.first, "vs",
		collisionPathPair.second, "ps");
	if (colliderBlobs.first == nullptr or colliderBlobs.second == nullptr) return false;
	_vsCollisionBlob = colliderBlobs.first;
	_psCollisionBlob = colliderBlobs.second;
	delete collisionShaderCompile;

	// コライダー用
	auto result = colliderGraphicsPipelineSetting->CreateGPStateWrapper(dev, collisionRootSignature, _vsCollisionBlob, _psCollisionBlob);

	return result;
}

// TODO : 1. シェーダーを分けて、ボーンマトリックスとの乗算をなくす&エッジのみ着色したボックスとして表示する、2. 8頂点の位置を正す 3. 複数のメッシュ(障害物)とキャラクターメッシュを判別して処理出来るようにする
void CollisionManager::CreateInfo()
{
	auto vertmap1 = resourceManager[0]->/*GetIndiceAndVertexInfo()*/GetIndiceAndVertexInfoOfOBB();
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

	boxes.resize(vertMaps.size());
	// オブジェクトの頂点情報からそれぞれのOBBを生成する
	auto itVertMap = vertMaps.begin();

	// fbxモデルのxyzローカル回転・平行移動行列群を取得
	auto localTransitionAndRotation = resourceManager[0]->GetLocalMatrixOfOBB();
	std::map<std::string, std::vector<XMFLOAT3>> boxPoints;
	itVertMap = vertMaps.begin();
	//std::vector<float> xMax, xMin, yMax, yMin, zMax, zMin;
	for (int i = 0; i < vertMaps.size(); ++i)
	{
		XMVECTOR CenterOfMass = XMVectorZero();
		int totalCount = 0;
		// Compute the center of mass and inertia tensor of the points.
		for (auto& point : itVertMap->second)
		{
			CenterOfMass = XMVectorAdd(CenterOfMass, XMLoadFloat3(&point));
			++totalCount;
		}

		CenterOfMass = XMVectorMultiply(CenterOfMass, XMVectorReciprocal(XMVectorReplicate(float(totalCount))));

		// OBBの頂点を逆回転させて回転無しの状態にする。
		for (auto& point : itVertMap->second)
		{
			//XMStoreFloat3(&point, XMVectorAdd(CenterOfMass, XMVector3Transform(XMVectorSubtract(XMLoadFloat3(&point), CenterOfMass), localTransitionAndRotation[itVertMap->first])));
			XMStoreFloat3(&point, XMVector3Transform(XMLoadFloat3(&point), localTransitionAndRotation[itVertMap->first]));
			//point.z *= -1;
			boxPoints[itVertMap->first].push_back(point);
		}

		++itVertMap;
	}

	auto itBoxPoints = boxPoints.begin();
	for (int i = 0; i < vertMaps.size(); ++i)
	{
		BoundingOrientedBox::CreateFromPoints(boxes[i], itBoxPoints->second.size(), itBoxPoints->second.data(), (size_t)sizeof(XMFLOAT3));
		//boxes[i].Center.x *= -1;
		++itBoxPoints;
	}

	oBBVertices.resize(vertMaps.size());
	itVertMap = vertMaps.begin();
	// 各OBBをクォータニオンにより回転させ、Extentsを調整し、その頂点群を描画目的で格納していく
	for (int i = 0; i < vertMaps.size(); ++i)
	{
		// fbxモデルのxyzローカル回転・平行移動行列からクォータニオン生成
		XMVECTOR quaternion = XMQuaternionRotationMatrix(localTransitionAndRotation[itVertMap->first]);

		// OBBの頂点を回転させる。クォータニオンなのでOBB中心点に基づき姿勢が変化する。ワールド空間原点を中心とした回転ではないことに注意。
		XMFLOAT4 orientation;
		orientation.x = quaternion.m128_f32[0];
		orientation.y = -quaternion.m128_f32[1];
		orientation.z = quaternion.m128_f32[2];
		orientation.w = quaternion.m128_f32[3];
		boxes[i].GetCorners(oBBVertices[i].pos);

		++itVertMap;
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
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(oBBVertices[0]));
		
	//★ OBBの数分リソースを用意し、マッピングする
	boxBuffs.resize(oBBVertices.size());
	boxVBVs.resize(oBBVertices.size());
	mappedOBBs.resize(oBBVertices.size());
	for (int i = 0; i < oBBVertices.size(); ++i)
	{
		// バッファー作成
		boxBuffs[i] = nullptr;
		auto result = dev->CreateCommittedResource
		(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(boxBuffs[i].ReleaseAndGetAddressOf())
		);
		// ビュー作成
		boxVBVs[i].BufferLocation = boxBuffs[i]->GetGPUVirtualAddress();
		boxVBVs[i].SizeInBytes = sizeof(oBBVertices[i]);
		boxVBVs[i].StrideInBytes = sizeof(XMFLOAT3);
		// マッピング
		mappedOBBs[i] = nullptr;
		boxBuffs[i]->Map(0, nullptr, (void**)&mappedOBBs[i]);
		std::copy(std::begin(oBBVertices[i].pos), std::end(oBBVertices[i].pos), mappedOBBs[i]);
	}

	// 頂点インデックスを生成してマッピングする
	// まずは距離から頂点の位置関係を把握する。ロジックとしては
	// 1. 各OBBの[0]頂点と他頂点の距離を算出して小さい順に並び変える
	// 2. [0][1][2],[0][1][3],[0][2][3]を[0]回りのインデックスとして抽出する。
	// 3. 距離最大のものを除いて、距離の大きな3点は対角線を形成する頂点で、それぞれに対して手順1,2を繰り返す。
	//std::map<int, std::vector<std::pair<float, int>>> res;
	//for (int i = 0; i < oBBVertices.size(); ++i)
	//{
	//	for (int j = 0; j < sizeof(oBBVertices[i].pos) / sizeof(XMFLOAT3); ++j)
	//	{
	//		auto v1 = XMVectorSubtract(XMLoadFloat3(&oBBVertices[i].pos[0]), XMLoadFloat3(&oBBVertices[i].pos[j]));
	//		std::pair<float, int> pair = {XMVector4Length(v1).m128_f32[0], j};
	//		res[i].push_back(pair);
	//	}
	//	std::sort(res[i].begin(), res[i].end());
	//	
	//	// ポリゴン1のインデックス
	//	oBBIndices[i].push_back(res[i][0].second);
	//	oBBIndices[i].push_back(res[i][1].second);
	//	oBBIndices[i].push_back(res[i][2].second);

	//	// ポリゴン2のインデックス
	//	oBBIndices[i].push_back(res[i][0].second);
	//	oBBIndices[i].push_back(res[i][1].second);
	//	oBBIndices[i].push_back(res[i][3].second);

	//	// ポリゴン3のインデックス
	//	oBBIndices[i].push_back(res[i][0].second);
	//	oBBIndices[i].push_back(res[i][2].second);
	//	oBBIndices[i].push_back(res[i][3].second);

	//	// 以下ポリゴン4→8まで繰り返し
	//	StoreIndiceOfOBB(res, i, 4);
	//	StoreIndiceOfOBB(res, i, 5);
	//	StoreIndiceOfOBB(res, i, 6);
	//}

	boxIBVs.resize(oBBVertices.size());
	boxIbBuffs.resize(oBBVertices.size());
	mappedIdx.resize(oBBVertices.size());
	for (int i = 0; i < oBBVertices.size(); ++i)
	{
		auto indexNum = vertmap1[i].second.indices.size();
		auto indiceBuffSize = indexNum * sizeof(unsigned int);
		auto indicesDesc = CD3DX12_RESOURCE_DESC::Buffer(indiceBuffSize);
		boxIbBuffs[i] = nullptr;
		auto result = dev->CreateCommittedResource
		(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&indicesDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadヒープでのリソース初期状態はこのタイプが公式ルール
			nullptr,
			IID_PPV_ARGS(boxIbBuffs[i].ReleaseAndGetAddressOf())
		);
		//if (result != S_OK) return result;

		mappedIdx[i] = nullptr;
		result = boxIbBuffs[i]->Map(0, nullptr, (void**)&mappedIdx[i]); // mapping
		std::copy(std::begin(vertmap1[i].second.indices), std::end(vertmap1[i].second.indices), mappedIdx[i]);
		boxIbBuffs[i]->Unmap(0, nullptr);

		boxIBVs[i].BufferLocation = boxIbBuffs[i]->GetGPUVirtualAddress();
		boxIBVs[i].SizeInBytes = indiceBuffSize;
		boxIBVs[i].Format = DXGI_FORMAT_R32_UINT;
	}

	// バッファー作成2
	resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(/*output2*/output3));
	auto result = dev->CreateCommittedResource
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

	// ｲﾝﾃﾞｯｸｽﾊﾞｯﾌｧー作成
	auto indexNum = sphereColliderIndices.size();
	auto indiceBuffSize = indexNum * sizeof(unsigned int);
	auto indicesDesc = CD3DX12_RESOURCE_DESC::Buffer(indiceBuffSize);

	result = dev->CreateCommittedResource
	(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&indicesDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadヒープでのリソース初期状態はこのタイプが公式ルール
		nullptr,
		IID_PPV_ARGS(sphereIbBuff.ReleaseAndGetAddressOf())
	);
	//if (result != S_OK) return result;

	result = sphereIbBuff->Map(0, nullptr, (void**)&mappedSphereIdx); // mapping
	std::copy(std::begin(sphereColliderIndices), std::end(sphereColliderIndices), mappedSphereIdx);
	sphereIbBuff->Unmap(0, nullptr);

	sphereIBV.BufferLocation = sphereIbBuff->GetGPUVirtualAddress();
	sphereIBV.SizeInBytes = indiceBuffSize;
	sphereIBV.Format = DXGI_FORMAT_R32_UINT;
}

void CollisionManager::MoveCharacterBoundingBox(double speed, XMMATRIX charaDirection)
{
	auto tempCenterPos = XMLoadFloat3(&bSphere.Center);
	tempCenterPos.m128_f32[3] = 1;

	// 平行移動成分にキャラクターの向きから回転成分を乗算して方向変え。これによる回転移動成分は不要なので、1と0にする。Y軸回転のみ対応している。
	auto moveMatrix = XMMatrixMultiply(XMMatrixTranslation(0, 0, speed), charaDirection);
	moveMatrix.r[0].m128_f32[0] = 1;
	moveMatrix.r[0].m128_f32[2] = 0;
	moveMatrix.r[2].m128_f32[0] = 0;
	moveMatrix.r[2].m128_f32[2] = 1;
	tempCenterPos = XMVector4Transform(tempCenterPos, moveMatrix); // 符号注意

	bSphere.Center.x = tempCenterPos.m128_f32[0];
	bSphere.Center.y = tempCenterPos.m128_f32[1];
	bSphere.Center.z = tempCenterPos.m128_f32[2];
}

void CollisionManager::CreateSpherePoints(const XMFLOAT3& center, float Radius)
{
	int div = 8;
	int loopStartCnt = 1;
	int loopEndCnt = 1 + div;
	
	// 北極点
	output3[0].x = center.x;
	output3[0].y = center.y + Radius;
	output3[0].z = center.z;

	float halfR = Radius * cosf(XMConvertToRadians(45));

	// 北極点と赤道の間
	for (int i = 0; loopStartCnt < loopEndCnt; ++i)
	{
		output3[loopStartCnt].x = center.x + halfR * cosf(XMConvertToRadians(360 / div * i));
		output3[loopStartCnt].y = center.y + halfR;
		output3[loopStartCnt].z = center.z + halfR * sinf(XMConvertToRadians(360 / div * i));
		++loopStartCnt;
	}

	loopEndCnt += div;
	// 赤道
	for (int i = 0; loopStartCnt < loopEndCnt; ++i)
	{		
		output3[loopStartCnt].x = center.x + Radius * cosf(XMConvertToRadians(360 / div * i));
		output3[loopStartCnt].y = center.y;
		output3[loopStartCnt].z = center.z + Radius * sinf(XMConvertToRadians(360 / div * i));
		++loopStartCnt;
	}

	loopEndCnt += div;
	// 南極点と赤道の間
	for (int i = 0; loopStartCnt < loopEndCnt; ++i)
	{
		output3[loopStartCnt].x = center.x + halfR * cosf(XMConvertToRadians(360 / div * i));
		output3[loopStartCnt].y = center.y - halfR;
		output3[loopStartCnt].z = center.z + halfR * sinf(XMConvertToRadians(360 / div * i));
		++loopStartCnt;
	}

	// 南極点
	output3[loopEndCnt].x = center.x;
	output3[loopEndCnt].y = center.y - Radius;
	output3[loopEndCnt].z = center.z;

	// インデックス作成
	// 北極点～北極点と赤道の間
	for (int j = 1; j < 8; ++j)
	{
		sphereColliderIndices.push_back(0);
		sphereColliderIndices.push_back(j);
		sphereColliderIndices.push_back(j + 1);
	}

	sphereColliderIndices.push_back(0);
	sphereColliderIndices.push_back(8);
	sphereColliderIndices.push_back(1);

	// 北極点と赤道の間～赤道
	for (int k = 1; k < 8; ++k)
	{
		sphereColliderIndices.push_back(k);
		sphereColliderIndices.push_back(k + 8);
		sphereColliderIndices.push_back(k + 1);
		sphereColliderIndices.push_back(k + 8);
		sphereColliderIndices.push_back(k + 1);		
		sphereColliderIndices.push_back(k + 9);

		// 以下並びでは一部のラインが生成されない。原因不明。赤道～南極点と赤道の間を上記・下記と同様の並びにすると一部ラインが描画されなくなる。
		//sphereColliderIndices.push_back(k);
		//sphereColliderIndices.push_back(k + 1);
		//sphereColliderIndices.push_back(k + 8);
		//sphereColliderIndices.push_back(k + 1);
		//sphereColliderIndices.push_back(k + 8);
		//sphereColliderIndices.push_back(k + 9);
	}

	sphereColliderIndices.push_back(8);
	sphereColliderIndices.push_back(1);
	sphereColliderIndices.push_back(16);
	sphereColliderIndices.push_back(1);
	sphereColliderIndices.push_back(16);
	sphereColliderIndices.push_back(9);

	// 赤道～南極点と赤道の間

	for (int k = 9; k < 16; ++k)
	{
		// 並びの規則性については謎。ライン生成は並びに影響される。ポリゴン表示は影響されない。
		sphereColliderIndices.push_back(k + 8);
		sphereColliderIndices.push_back(k);
		sphereColliderIndices.push_back(k + 1);		
		sphereColliderIndices.push_back(k + 9);
		sphereColliderIndices.push_back(k + 1);
		sphereColliderIndices.push_back(k + 8);
		
	}

	sphereColliderIndices.push_back(16);
	sphereColliderIndices.push_back(9);
	sphereColliderIndices.push_back(24);
	sphereColliderIndices.push_back(9);
	sphereColliderIndices.push_back(24);
	sphereColliderIndices.push_back(17);

	// 南極点と赤道の間～南極
	for (int j = 17; j < 24; ++j)
	{
		sphereColliderIndices.push_back(25);
		sphereColliderIndices.push_back(j);
		sphereColliderIndices.push_back(j + 1);
	}
	sphereColliderIndices.push_back(25);
	sphereColliderIndices.push_back(24);
	sphereColliderIndices.push_back(17);
}

HRESULT CollisionManager::CreateMatrixHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {}; // SRV用ディスクリプタヒープ
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 1; // matrix
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;

	auto result = dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(matrixHeap.ReleaseAndGetAddressOf()));
	return result;
}

HRESULT CollisionManager::CreateMatrixResources()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(CollisionMatrix) + 0xff) & ~0xff);

	auto result = dev->CreateCommittedResource
	(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadヒープでのリソース初期状態はこのタイプが公式ルール
		nullptr,
		IID_PPV_ARGS(matrixBuff.ReleaseAndGetAddressOf())
	);

	return result;
}

void CollisionManager::CreateMatrixView()
{
	auto handle = matrixHeap->GetCPUDescriptorHandleForHeapStart();

	// frustum用view
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
	desc.BufferLocation = matrixBuff->GetGPUVirtualAddress();
	desc.SizeInBytes = matrixBuff->GetDesc().Width;
	dev->CreateConstantBufferView
	(
		&desc,
		handle
	);
}

HRESULT CollisionManager::MappingMatrix()
{
	auto result = matrixBuff->Map(0, nullptr, (void**)&mappedMatrix);
	return result;
}

void CollisionManager::SetMatrix(XMMATRIX _world, XMMATRIX _view, XMMATRIX _proj)
{
	mappedMatrix->world = _world;
	mappedMatrix->view = _view;
	mappedMatrix->proj= _proj;
}

void CollisionManager::SetCharaPos(XMFLOAT3 _charaPos)
{
	charaPos = _charaPos;
}

void CollisionManager::SetRotation(XMMATRIX _rotation)
{
	mappedMatrix->rotation = _rotation;
}

void CollisionManager::SetDraw(bool _isDraw, ID3D12GraphicsCommandList* _cmdList)
{
	mappedMatrix->isDraw = _isDraw;
	if (_isDraw)
	{
		Execution(_cmdList);
	}
}

void CollisionManager::Execution(ID3D12GraphicsCommandList* _cmdList)
{
	_cmdList->SetGraphicsRootSignature(collisionRootSignature->GetRootSignature().Get());
	_cmdList->SetDescriptorHeaps(1, matrixHeap.GetAddressOf());
	auto gHandle = matrixHeap->GetGPUDescriptorHandleForHeapStart();
	_cmdList->SetGraphicsRootDescriptorTable(0, gHandle);

	_cmdList->SetPipelineState(colliderGraphicsPipelineSetting->GetPipelineState().Get());

	//プリミティブ型に関する情報と、入力アセンブラーステージの入力データを記述するデータ順序をバインド
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP/*D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP*/);

	mappedMatrix->world.r[3].m128_f32[0] = charaPos.x;
	//mappedMatrix->world.r[3].m128_f32[1] = charaPos.y;
	mappedMatrix->world.r[3].m128_f32[2] = charaPos.z;
	_cmdList->IASetVertexBuffers(0, 1, &boxVBV2);
	_cmdList->IASetIndexBuffer(&sphereIBV);
	_cmdList->DrawIndexedInstanced(144, 1, 0, 0, 0);
}

//void CollisionManager::StoreIndiceOfOBB(std::map<int, std::vector<std::pair<float, int>>> res, int loopCnt, int index)
//{
//	std::map<int, std::vector<std::pair<float, int>>> res2;
//	for (int j = 0; j < sizeof(oBBVertices[loopCnt].pos) / sizeof(XMFLOAT3); ++j)
//	{
//		auto v1 = XMVectorSubtract(XMLoadFloat3(&oBBVertices[loopCnt].pos[res[loopCnt][index].second]), XMLoadFloat3(&oBBVertices[loopCnt].pos[j]));
//		std::pair<float, int> pair = { XMVector4Length(v1).m128_f32[0], j };
//		res2[loopCnt].push_back(pair);
//	}
//	std::sort(res2[loopCnt].begin(), res2[loopCnt].end());
//
//	// 球体同様に並びの規則性は不明。[0][1][2]のように[0]を先頭に並べるとOBB内にポリゴンが描画されてしまう。
//	// ポリゴン1のインデックス
//	oBBIndices[loopCnt].push_back(res2[loopCnt][2].second);
//	oBBIndices[loopCnt].push_back(res2[loopCnt][0].second);
//	oBBIndices[loopCnt].push_back(res2[loopCnt][1].second);	
//
//	// ポリゴン2のインデックス
//	oBBIndices[loopCnt].push_back(res2[loopCnt][3].second);
//	oBBIndices[loopCnt].push_back(res2[loopCnt][0].second);
//	oBBIndices[loopCnt].push_back(res2[loopCnt][1].second);	
//
//	// ポリゴン3のインデックス
//	oBBIndices[loopCnt].push_back(res2[loopCnt][3].second);
//	oBBIndices[loopCnt].push_back(res2[loopCnt][0].second);
//	oBBIndices[loopCnt].push_back(res2[loopCnt][2].second);
//	
//}

bool CollisionManager::OBBCollisionCheck()
{
	bool result = true;

	// 各OBB中心からキャラクターコライダー中心までの距離を測定し、OBB.Extents.x * 2(この数値の決定根拠はない)とキャラコライダー半径を足した数値を減算する。
	bool isUpperMargin = true;
	auto sCenter = bSphere.Center;
	BoundingOrientedBox targetOBB;

	// 20231021 TODO 直方体2つの内青いのが認識されない
	//for (int i = 0; i < boxes.size(); ++i)
	//{
	//	float maxExtents = boxes[i].Extents.x;
	//	if (maxExtents < boxes[i].Extents.y)
	//	{
	//		maxExtents = boxes[i].Extents.y;
	//	}
	//	if (maxExtents < boxes[i].Extents.z)
	//	{
	//		maxExtents = boxes[i].Extents.z;
	//	}
	//	auto dist = XMVectorSubtract(XMLoadFloat3(&boxes[i].Center), XMLoadFloat3(&sCenter));
	//	auto len = XMVector3Length(dist);
	//	len.m128_f32[0] -= (bSphere.Radius + maxExtents * 2);
	//	if (len.m128_f32[0] < 0)
	//	{
	//		isUpperMargin = false;
	//		targetOBB = boxes[i]; // マージン以下にまで近づいてきたOBBを格納する
	//	}
	//}
	//// 各OBBとキャラクターコライダーの間にある程度距離がある場合は当たり判定を行わない。
	//if (isUpperMargin)
	//{
	//	return result;
	//}

	// 総当たりの場合
	for (auto& box : boxes)
	{
		if (box.Contains(bSphere) != 0)
		{
			result = false;
			collidedOBB = box;
			break;
		}
	}

	//// 総当たりではなくtargetOBBにのみ当たり判定を行う
	//if (targetOBB.Contains(bSphere) != 0)
	//{
	//	result = false;
	//	collidedOBB = targetOBB;
	//}

	return result;
}

XMFLOAT3 CollisionManager::OBBCollisionCheckAndTransration(float forwardSpeed, XMMATRIX characterDirection, int fbxIndex, XMVECTOR v, XMFLOAT3 charaPos)
{
	XMFLOAT3 boxCenterPos = collidedOBB.Center;
	XMVECTOR boxCenterVec = XMLoadFloat3(&collidedOBB.Center);
	BoundingSphere reserveSphere = bSphere; // 操作キャラクターコリジョンを動かす前の情報を残しておく
	bSphere.Center.x = charaPos.x;
	bSphere.Center.z = charaPos.z;
	auto sCenter = bSphere.Center;
	MoveCharacterBoundingBox(forwardSpeed, characterDirection); // move collider for collision test

	if (OBBCollisionCheck()/*collisionManager->GetBoundingBox1()[debugNum].Contains(collisionManager->GetBoundingSphere()) == 0*/)
	{
		//resourceManager[fbxIndex]->GetMappedMatrix()->world *= XMMatrixTranslation(0, 0, -forwardSpeed);
		resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[0] = v.m128_f32[0];
		resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[1] = v.m128_f32[1];
		resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[2] = v.m128_f32[2];

		charaPos.x = resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[0];
		charaPos.z = resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[2];

		resourceManager[fbxIndex]->GetMappedMatrix()->charaPos = charaPos;
		bSphere.Center.x = charaPos.x;
		bSphere.Center.z = charaPos.z;
		return charaPos;
	}

	else
	{
		bSphere.Center = reserveSphere.Center; // 衝突したのでキャラクターコリジョンの位置を元に戻す
		auto moveMatrix = XMMatrixIdentity(); // 滑らせように初期化して使いまわし
		XMFLOAT3 boxVertexPos[8];
		collidedOBB.GetCorners(boxVertexPos);
		//  1. 頂点p0, p6(g_BoxOffset[0], [6])とそれぞれに隣り合う3点を抽出し、辺を求める。点はGetCorners()でg_BoxOffset[8]の順番で取得可能。
		//	0→1, 0→3, 0→4
		//	6→2, 6→5, 6→7
		//	で0と隣り合う3つの頂点への辺および、6と隣り合う3つの頂点への辺が取得できる。
		XMVECTOR vZeroOne = XMVectorSubtract(XMLoadFloat3(&boxVertexPos[1]), XMLoadFloat3(&boxVertexPos[0]));
		XMVECTOR vZeroThree = XMVectorSubtract(XMLoadFloat3(&boxVertexPos[3]), XMLoadFloat3(&boxVertexPos[0]));
		XMVECTOR vZeroFour = XMVectorSubtract(XMLoadFloat3(&boxVertexPos[4]), XMLoadFloat3(&boxVertexPos[0]));
		XMVECTOR vSixTwo = XMVectorSubtract(XMLoadFloat3(&boxVertexPos[2]), XMLoadFloat3(&boxVertexPos[6]));
		XMVECTOR vSixFive = XMVectorSubtract(XMLoadFloat3(&boxVertexPos[5]), XMLoadFloat3(&boxVertexPos[6]));
		XMVECTOR vSixSeven = XMVectorSubtract(XMLoadFloat3(&boxVertexPos[7]), XMLoadFloat3(&boxVertexPos[6]));
		//	2. 以下ベクトルの外積 = 面の法線を算出する。DirectX = 左手系に注意
		//	0→1, 0→3
		XMVECTOR n1 = XMVector3Cross(vZeroOne, vZeroThree);
		//	0→1, 0→4
		XMVECTOR n2 = XMVector3Cross(vZeroFour, vZeroOne);
		//	0→3, 0→4
		XMVECTOR n3 = XMVector3Cross(vZeroThree, vZeroFour);
		//	6→2, 6→5
		XMVECTOR n4 = XMVector3Cross(vSixTwo, vSixFive);
		//	6→2, 6→7
		XMVECTOR n5 = XMVector3Cross(vSixSeven, vSixTwo);
		//	6→5, 6→7
		XMVECTOR n6 = XMVector3Cross(vSixFive, vSixSeven);

		//	3. XMPlaneFromPointNormal(p0, n1 / 2 / 3), XMPlaneFromPointNormal(p6, n4 / 5 / 6)によってOBBの面を求める
		XMVECTOR plane1 = XMVector4Normalize(XMPlaneFromPointNormal(XMLoadFloat3(&boxVertexPos[0]), n1));
		XMVECTOR plane2 = XMVector4Normalize(XMPlaneFromPointNormal(XMLoadFloat3(&boxVertexPos[0]), n2));
		XMVECTOR plane3 = XMVector4Normalize(XMPlaneFromPointNormal(XMLoadFloat3(&boxVertexPos[0]), n3));
		XMVECTOR plane4 = XMVector4Normalize(XMPlaneFromPointNormal(XMLoadFloat3(&boxVertexPos[6]), n4));
		XMVECTOR plane5 = XMVector4Normalize(XMPlaneFromPointNormal(XMLoadFloat3(&boxVertexPos[6]), n5));
		XMVECTOR plane6 = XMVector4Normalize(XMPlaneFromPointNormal(XMLoadFloat3(&boxVertexPos[6]), n6));

		//	4. XMPlaneDotCoord(plane1 / 2 / 3 / 4 / 5 / 6, bSphere.center)の結果から衝突面を求める
		XMVECTOR sphereVec = XMLoadFloat3(&sCenter);
		sphereVec.m128_f32[3] = 1;
		XMVECTOR dotPlane1Sphere = XMPlaneDotCoord(plane1, sphereVec);
		XMVECTOR dotPlane2Sphere = XMPlaneDotCoord(plane2, sphereVec);
		XMVECTOR dotPlane3Sphere = XMPlaneDotCoord(plane3, sphereVec);
		XMVECTOR dotPlane4Sphere = XMPlaneDotCoord(plane4, sphereVec);
		XMVECTOR dotPlane5Sphere = XMPlaneDotCoord(plane5, sphereVec);
		XMVECTOR dotPlane6Sphere = XMPlaneDotCoord(plane6, sphereVec);

		int collisionPlaneCount = 0;
		std::vector<std::pair<float, int>> collidedPlanes;
		if (dotPlane1Sphere.m128_f32[0] > 0)
		{
			collidedPlanes.resize(collisionPlaneCount + 1);
			collidedPlanes[collisionPlaneCount].first = dotPlane1Sphere.m128_f32[0];
			collidedPlanes[collisionPlaneCount].second = 1;
			++collisionPlaneCount;
		}
		if (dotPlane2Sphere.m128_f32[0] > 0)
		{
			collidedPlanes.resize(collisionPlaneCount + 1);
			collidedPlanes[collisionPlaneCount].first = dotPlane2Sphere.m128_f32[0];
			collidedPlanes[collisionPlaneCount].second = 2;
			++collisionPlaneCount;
		}
		if (dotPlane3Sphere.m128_f32[0] > 0)
		{
			collidedPlanes.resize(collisionPlaneCount + 1);
			collidedPlanes[collisionPlaneCount].first = dotPlane3Sphere.m128_f32[0];
			collidedPlanes[collisionPlaneCount].second = 3;
			++collisionPlaneCount;
		}
		if (dotPlane4Sphere.m128_f32[0] > 0)
		{
			collidedPlanes.resize(collisionPlaneCount + 1);
			collidedPlanes[collisionPlaneCount].first = dotPlane4Sphere.m128_f32[0];
			collidedPlanes[collisionPlaneCount].second = 4;
			++collisionPlaneCount;
		}
		if (dotPlane5Sphere.m128_f32[0] > 0)
		{
			collidedPlanes.resize(collisionPlaneCount + 1);
			collidedPlanes[collisionPlaneCount].first = dotPlane5Sphere.m128_f32[0];
			collidedPlanes[collisionPlaneCount].second = 5;
			++collisionPlaneCount;
		}
		if (dotPlane6Sphere.m128_f32[0] > 0)
		{
			collidedPlanes.resize(collisionPlaneCount + 1);
			collidedPlanes[collisionPlaneCount].first = dotPlane6Sphere.m128_f32[0];
			collidedPlanes[collisionPlaneCount].second = 6;
			++collisionPlaneCount;
		}

		std::vector<XMFLOAT3> boxPoint4Cal;
		std::vector<XMFLOAT3> boxPoint4NextPlaneCal;
		// not corner collision && collided a plane
		if (collisionPlaneCount == 1 && collidedPlanes[0].first > 0)
		{
			// plane1 points
			if (collidedPlanes[0].second == 1)
			{
				boxPoint4Cal.push_back(boxVertexPos[0]);
				boxPoint4Cal.push_back(boxVertexPos[1]);
				boxPoint4Cal.push_back(boxVertexPos[2]);
				boxPoint4Cal.push_back(boxVertexPos[3]);
			}
			// plane2 points
			else if (collidedPlanes[0].second == 2)
			{
				boxPoint4Cal.push_back(boxVertexPos[0]);
				boxPoint4Cal.push_back(boxVertexPos[1]);
				boxPoint4Cal.push_back(boxVertexPos[4]);
				boxPoint4Cal.push_back(boxVertexPos[5]);
			}
			// plane3 points
			else if (collidedPlanes[0].second == 3)
			{
				boxPoint4Cal.push_back(boxVertexPos[0]);
				boxPoint4Cal.push_back(boxVertexPos[3]);
				boxPoint4Cal.push_back(boxVertexPos[4]);
				boxPoint4Cal.push_back(boxVertexPos[7]);
			}
			// plane4 points
			else if (collidedPlanes[0].second == 4)
			{
				boxPoint4Cal.push_back(boxVertexPos[1]);
				boxPoint4Cal.push_back(boxVertexPos[2]);
				boxPoint4Cal.push_back(boxVertexPos[5]);
				boxPoint4Cal.push_back(boxVertexPos[6]);
			}
			// plane5 points
			else if (collidedPlanes[0].second == 5)
			{
				boxPoint4Cal.push_back(boxVertexPos[2]);
				boxPoint4Cal.push_back(boxVertexPos[3]);
				boxPoint4Cal.push_back(boxVertexPos[6]);
				boxPoint4Cal.push_back(boxVertexPos[7]);
			}
			// plane6 points
			else if (collidedPlanes[0].second == 6)
			{
				boxPoint4Cal.push_back(boxVertexPos[4]);
				boxPoint4Cal.push_back(boxVertexPos[5]);
				boxPoint4Cal.push_back(boxVertexPos[6]);
				boxPoint4Cal.push_back(boxVertexPos[7]);
			}
		}

		// 2つの場合内積から割合計算できないか試す
		else if (collisionPlaneCount == 2 && collidedPlanes[0].first > 0 && collidedPlanes[1].first > 0)
		{
			// 内積の値が大きいものから計算するため降順に並び変える
			std::sort(collidedPlanes.rbegin(), collidedPlanes.rend());

			// 衝突面1
			// plane1 points
			if (collidedPlanes[0].second == 1)
			{
				boxPoint4Cal.push_back(boxVertexPos[0]);
				boxPoint4Cal.push_back(boxVertexPos[1]);
				boxPoint4Cal.push_back(boxVertexPos[2]);
				boxPoint4Cal.push_back(boxVertexPos[3]);
			}
			// plane2 points
			else if (collidedPlanes[0].second == 2)
			{
				boxPoint4Cal.push_back(boxVertexPos[0]);
				boxPoint4Cal.push_back(boxVertexPos[1]);
				boxPoint4Cal.push_back(boxVertexPos[4]);
				boxPoint4Cal.push_back(boxVertexPos[5]);
			}
			// plane3 points
			else if (collidedPlanes[0].second == 3)
			{
				boxPoint4Cal.push_back(boxVertexPos[0]);
				boxPoint4Cal.push_back(boxVertexPos[3]);
				boxPoint4Cal.push_back(boxVertexPos[4]);
				boxPoint4Cal.push_back(boxVertexPos[7]);
			}
			// plane4 points
			else if (collidedPlanes[0].second == 4)
			{
				boxPoint4Cal.push_back(boxVertexPos[1]);
				boxPoint4Cal.push_back(boxVertexPos[2]);
				boxPoint4Cal.push_back(boxVertexPos[5]);
				boxPoint4Cal.push_back(boxVertexPos[6]);
			}
			// plane5 points
			else if (collidedPlanes[0].second == 5)
			{
				boxPoint4Cal.push_back(boxVertexPos[2]);
				boxPoint4Cal.push_back(boxVertexPos[3]);
				boxPoint4Cal.push_back(boxVertexPos[6]);
				boxPoint4Cal.push_back(boxVertexPos[7]);
			}
			// plane6 points
			else if (collidedPlanes[0].second == 6)
			{
				boxPoint4Cal.push_back(boxVertexPos[4]);
				boxPoint4Cal.push_back(boxVertexPos[5]);
				boxPoint4Cal.push_back(boxVertexPos[6]);
				boxPoint4Cal.push_back(boxVertexPos[7]);
			}

			// 衝突面2(隣面)
			// plane1 points
			if (collidedPlanes[1].second == 1)
			{
				boxPoint4NextPlaneCal.push_back(boxVertexPos[0]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[1]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[2]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[3]);
			}
			// plane2 points
			else if (collidedPlanes[1].second == 2)
			{
				boxPoint4NextPlaneCal.push_back(boxVertexPos[0]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[1]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[4]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[5]);
			}
			// plane3 points
			else if (collidedPlanes[1].second == 3)
			{
				boxPoint4NextPlaneCal.push_back(boxVertexPos[0]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[3]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[4]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[7]);
			}
			// plane4 points
			else if (collidedPlanes[1].second == 4)
			{
				boxPoint4NextPlaneCal.push_back(boxVertexPos[1]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[2]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[5]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[6]);
			}
			// plane5 points
			else if (collidedPlanes[1].second == 5)
			{
				boxPoint4NextPlaneCal.push_back(boxVertexPos[2]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[3]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[6]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[7]);
			}
			// plane6 points
			else if (collidedPlanes[1].second == 6)
			{
				boxPoint4NextPlaneCal.push_back(boxVertexPos[4]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[5]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[6]);
				boxPoint4NextPlaneCal.push_back(boxVertexPos[7]);
			}
		}

		// 法線ベクトルとスライド方向を算出する
		auto normalAndSlideVector = CalcurateNormalAndSlideVector(boxPoint4Cal, boxCenterPos);
		auto normal = normalAndSlideVector.first;
		auto slideVector = normalAndSlideVector.second;
		auto reverseSlideVector = -slideVector; // スライド方向ベクトル候補2(1の逆方向)
		slideVector.m128_f32[3] = 1;
		reverseSlideVector.m128_f32[3] = 1;

		// キャラクターの進行方向に対して、衝突面法線ベクトル・スライド候補1・スライド候補2との各内積を求める。
		XMVECTOR characterZDir = { characterDirection.r[2].m128_f32[0], characterDirection.r[0].m128_f32[1], characterDirection.r[0].m128_f32[0], 1 };
		auto dotboxNormalAndCharacterDir = XMVector3Dot(normal, characterZDir).m128_f32[0];
		auto dotslideVectorAndCharacterDir = XMVector3Dot(slideVector, characterZDir).m128_f32[0];
		auto dotReverseSlideVectorAndCharacterDir = XMVector3Dot(reverseSlideVector, characterZDir).m128_f32[0];
		XMVECTOR determinedSlideVector; // 決定したスライド方向(キャラクターコライダーが角にぶつかっているか判定するのにも使う)
		float adjustSpeed = 0.02f;

		// キャラクターが衝突面に対して垂直ではない場合、キャラクターを衝突面に対してスライドさせる
		if (dotboxNormalAndCharacterDir != -1.0f)
		{
			// キャラクターの進行方向との内積が小さいスライド方向が正解。このスライド方向はコライダーの頂点座標に基づいているが、fbxモデルはZ成分の符号を逆転させている。fbxモデル同様にZ成分を-1乗算して
			// ワールド空間におけるZ方向をFBXと同様の画面下方向に変換している。そのため、次の処理でコライダーにスライド方向で調整したmoveMAtrixを適用する際にはZ成分の符号をまた-1掛けしている...
			if (dotslideVectorAndCharacterDir < dotReverseSlideVectorAndCharacterDir)
			{
				determinedSlideVector = slideVector;
				moveMatrix.r[3].m128_f32[0] += determinedSlideVector.m128_f32[0] * adjustSpeed;
				moveMatrix.r[3].m128_f32[2] -= determinedSlideVector.m128_f32[2] * adjustSpeed;
			}
			else
			{
				determinedSlideVector = reverseSlideVector;
				moveMatrix.r[3].m128_f32[0] += determinedSlideVector.m128_f32[0] * adjustSpeed;
				moveMatrix.r[3].m128_f32[2] -= determinedSlideVector.m128_f32[2] * adjustSpeed;
			}
		}

		auto centerToCenter = XMVectorSubtract(XMLoadFloat3(&sCenter), boxCenterVec); // 衝突したOBB中心座標→キャラクターコライダーまでのベクトル
		centerToCenter = XMVector3Normalize(centerToCenter);

		//printf("%f\n", dotPlane1Sphere.m128_f32[0]);
		//printf("%f\n", dotPlane2Sphere.m128_f32[0]);
		//printf("%f\n", dotPlane3Sphere.m128_f32[0]);
		//printf("%f\n", dotPlane4Sphere.m128_f32[0]);
		//printf("%f\n", dotPlane5Sphere.m128_f32[0]);
		//printf("%f\n", dotPlane6Sphere.m128_f32[0]);
		//printf("\n");

		// 角に接触していない場合
		if (collidedPlanes.size() == 1)
		{
			// Z軸がFBX(-Z前方)モデルに対して反転している。モデルの向きを+Z前方にしたいが一旦このままで実装を進める
			//for (int i = 0; i < boxes.size(); ++i)
			//{
			//	boxes[i].Center.x += moveMatrix.r[3].m128_f32[0];
			//	boxes[i].Center.y += moveMatrix.r[3].m128_f32[1];
			//	boxes[i].Center.z -= moveMatrix.r[3].m128_f32[2];
			//}

			// オブジェクトはシェーダーで描画されているのでworld変換行列の影響を受けている。moveMatrixはキャラクターの進行方向と逆にするため-1掛け済なので、
			// 更にキャラクターの向きを掛けてwolrd空間におきてキャラクターの逆の向きにオブジェクトが流れるようにする
			//moveMatrix.r[3].m128_f32[0] *= -1;
			moveMatrix.r[3].m128_f32[2] *= -1;
			//moveMatrix *= characterDirection;

			moveMatrix.r[0].m128_f32[0] = 1;
			moveMatrix.r[0].m128_f32[2] = 0;
			moveMatrix.r[2].m128_f32[0] = 0;
			moveMatrix.r[2].m128_f32[2] = 1;

			//v = XMVector4Transform(v, moveMatrix);
			//resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[0] = v.m128_f32[0];
			//resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[1] = v.m128_f32[1];
			//resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[2] = v.m128_f32[2];
			resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[0] += moveMatrix.r[3].m128_f32[0];
			resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[1] += moveMatrix.r[3].m128_f32[1];
			resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[2] += moveMatrix.r[3].m128_f32[2];

			charaPos.x = resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[0];
			charaPos.z = resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[2];

			resourceManager[fbxIndex]->GetMappedMatrix()->charaPos = charaPos;
			bSphere.Center.x = charaPos.x;
			bSphere.Center.z = charaPos.z;
			return charaPos;
		}

		// 角に接触している場合
		else if (collidedPlanes.size() == 2)
		{
			// もう一方の面の法線ベクトルとスライド方向を算出する
			auto normalAndSlideVectorOfNextPlane = CalcurateNormalAndSlideVector(boxPoint4NextPlaneCal, boxCenterPos);
			auto normalOfNextPlane = normalAndSlideVectorOfNextPlane.first;

			// 衝突面の法線方向と隣面の法線方向の合成ベクトルを利用して、角から弾かれるようにしてコライダー同士の交差を防ぐ
			// ガタついた動きになるのがネック
			float totalWeight = collidedPlanes[0].first + collidedPlanes[1].first;
			float normalWeight = /*1.0f - dot2.m128_f32[0]*/collidedPlanes[0].first / totalWeight;
			float nextNormalWeight = /*dot2.m128_f32[0]*/collidedPlanes[1].first / totalWeight;
			normal *= normalWeight;
			normalOfNextPlane *= nextNormalWeight;
			XMVECTOR slideCol;
			slideCol.m128_f32[0] = normal.m128_f32[0] + normalOfNextPlane.m128_f32[0];
			slideCol.m128_f32[1] = normal.m128_f32[1] + normalOfNextPlane.m128_f32[1];
			slideCol.m128_f32[2] = normal.m128_f32[2] + normalOfNextPlane.m128_f32[2];
			slideCol.m128_f32[3] = 1.0f;
			slideCol = XMVector3Normalize(slideCol);
			slideCol *= adjustSpeed;

			moveMatrix.r[3].m128_f32[0] = slideCol.m128_f32[0];
			//moveMatrix.r[3].m128_f32[0] *= -1;
			//moveMatrix.r[3].m128_f32[1] += centerToCenter.m128_f32[1];
			moveMatrix.r[3].m128_f32[2] = slideCol.m128_f32[2];
			// Z軸がFBX(-Z前方)モデルに対して反転している。モデルの向きを+Z前方にしたいが一旦このままで実装を進める
			//for (int i = 0; i < boxes.size(); ++i)
			//{
			//	boxes[i].Center.x += moveMatrix.r[3].m128_f32[0];
			//	boxes[i].Center.y += moveMatrix.r[3].m128_f32[1];
			//	boxes[i].Center.z -= moveMatrix.r[3].m128_f32[2];
			//}

			// オブジェクトはシェーダーで描画されているのでworld変換行列の影響を受けている。moveMatrixはキャラクターの進行方向と逆にするため-1掛け済なので、
			// 更にキャラクターの向きを掛けてwolrd空間におきてキャラクターの逆の向きにオブジェクトが流れるようにする
			//moveMatrix.r[3].m128_f32[0] *= -1;
			//moveMatrix.r[3].m128_f32[2] *= -1;
			//moveMatrix *= characterDirection;
			moveMatrix.r[0].m128_f32[0] = 1;
			moveMatrix.r[0].m128_f32[2] = 0;
			moveMatrix.r[2].m128_f32[0] = 0;
			moveMatrix.r[2].m128_f32[2] = 1;

			//resourceManager[fbxIndex]->GetMappedMatrix()->world *= moveMatrix;
			//resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[0] = v.m128_f32[0];
			//resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[1] = v.m128_f32[1];
			//resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[2] = v.m128_f32[2];
			resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[0] += moveMatrix.r[3].m128_f32[0];
			resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[1] += moveMatrix.r[3].m128_f32[1];
			resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[2] += moveMatrix.r[3].m128_f32[2];

			charaPos.x = resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[0];
			charaPos.z = resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[2];

			resourceManager[fbxIndex]->GetMappedMatrix()->charaPos = charaPos;
			bSphere.Center.x = charaPos.x;
			bSphere.Center.z = charaPos.z;
			return charaPos;
		}
	}
	
}

std::pair<XMVECTOR, XMVECTOR> CollisionManager::CalcurateNormalAndSlideVector(std::vector<XMFLOAT3> points, XMFLOAT3 boxCenter)
{
	XMVECTOR line1, line2, line3;
	std::vector<XMVECTOR> lines;
	line1 = XMVectorSubtract(XMLoadFloat3(&points[0]), XMLoadFloat3(&points[1]));
	line2 = XMVectorSubtract(XMLoadFloat3(&points[0]), XMLoadFloat3(&points[2]));
	line3 = XMVectorSubtract(XMLoadFloat3(&points[0]), XMLoadFloat3(&points[3]));
	lines.push_back(line1);
	lines.push_back(line2);
	lines.push_back(line3);
	auto lengthL1 = pow(abs(line1.m128_f32[0]) + abs(line1.m128_f32[1]) + abs(line1.m128_f32[2]), 2);
	auto lengthL2 = pow(abs(line2.m128_f32[0]) + abs(line2.m128_f32[1]) + abs(line2.m128_f32[2]), 2);
	auto lengthL3 = pow(abs(line3.m128_f32[0]) + abs(line3.m128_f32[1]) + abs(line3.m128_f32[2]), 2);
	std::vector<double> lineLength;
	lineLength.push_back(lengthL1);
	lineLength.push_back(lengthL2);
	lineLength.push_back(lengthL3);
	auto iter = std::max_element(lineLength.begin(), lineLength.end());
	size_t index = std::distance(lineLength.begin(), iter);
	lines.erase(lines.cbegin() + index); // 最大値の要素は四角形面の対角線なので削除する
	// 削除処理の前に衝突面の法線を求めておく
	float normXPos = 0;
	float normYPos = 0;
	float normZPos = 0;
	if (index == 0)
	{
		normXPos = (points[0].x + points[1].x) / 2.0f;
		normYPos = (points[0].y + points[1].y) / 2.0f;
		normZPos = (points[0].z + points[1].z) / 2.0f;
	}
	else if (index == 1)
	{
		normXPos = (points[0].x + points[2].x) / 2.0f;
		normYPos = (points[0].y + points[2].y) / 2.0f;
		normZPos = (points[0].z + points[2].z) / 2.0f;
	}
	else if (index == 2)
	{
		normXPos = (points[0].x + points[3].x) / 2.0f;
		normYPos = (points[0].y + points[3].y) / 2.0f;
		normZPos = (points[0].z + points[3].z) / 2.0f;
	}
	XMVECTOR normPos = { normXPos, normYPos, normZPos, 1 };
	XMVECTOR boxCenterVec = { boxCenter.x, boxCenter.y, boxCenter.z, 1 };
	auto normal = XMVectorSubtract(normPos, boxCenterVec);// XMVector3Cross(lines[0], lines[1]);
	normal = XMVector4Normalize(normal); // 衝突面法線の算出完了

	// y要素が大きい成分は流し方向ではない線分なので削除する
	if (abs(lines[0].m128_f32[1]) < abs(lines[1].m128_f32[1]))
	{
		lines.erase(lines.cbegin() + 1);
	}
	else
	{
		lines.erase(lines.cbegin());
	}
	auto slideVector = XMVector3Normalize(lines[0]); // スライド方向ベクトル候補1

	std::pair<XMVECTOR, XMVECTOR> result;
	result.first = normal;
	result.second = slideVector;

	return result;
}