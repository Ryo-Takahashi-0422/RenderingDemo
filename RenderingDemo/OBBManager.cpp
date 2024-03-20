#include <stdafx.h>
#include <OBBManager.h>

OBBManager::OBBManager(ComPtr<ID3D12Device> _dev, std::vector<ResourceManager*> _resourceManagers)
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

OBBManager::~OBBManager()
{
	delete layout;
	layout = nullptr;

	delete colliderGraphicsPipelineSetting;
	colliderGraphicsPipelineSetting = nullptr;

	free(collisionRootSignature);
	collisionRootSignature = nullptr;

	collisionShaderCompile = nullptr;

	mappedBox2 = nullptr;
	mappedIdx.clear();
	mappedMatrix = nullptr;

	for (auto& buff : boxBuffs)
	{
		buff->Unmap(0, nullptr);
	}
	for (auto& obb : mappedOBBs)
	{
		obb = nullptr;
	}
}

HRESULT OBBManager::Init()
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
void OBBManager::CreateInfo()
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
			point.z *= -1;
			boxPoints[itVertMap->first].push_back(point);
		}

		++itVertMap;
	}

	auto itBoxPoints = boxPoints.begin();
	for (int i = 0; i < vertMaps.size(); ++i)
	{
		BoundingOrientedBox::CreateFromPoints(boxes[i], itBoxPoints->second.size(), itBoxPoints->second.data(), (size_t)sizeof(XMFLOAT3));
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

	//// キャラクター用のスフィアコライダー生成
	//auto vetmap2 = resourceManager[1]->GetIndiceAndVertexInfo();
	//for (int j = 0; j < 4454; ++j)
	//{
	//	input2.push_back(vetmap2.begin()->second.vertices[j].pos);
	//}
	//BoundingSphere::CreateFromPoints(bSphere, 4454, input2.data(), (size_t)sizeof(XMFLOAT3));
	//bSphere.Radius -= 0.2f; // 球体コライダーの半径を微調整。数値適当

	//// 操作キャラクターの球体コライダー作成
	//CreateSpherePoints(bSphere.Center, bSphere.Radius);

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

	//// ｲﾝﾃﾞｯｸｽﾊﾞｯﾌｧー作成
	//auto indexNum = sphereColliderIndices.size();
	//auto indiceBuffSize = indexNum * sizeof(unsigned int);
	//auto indicesDesc = CD3DX12_RESOURCE_DESC::Buffer(indiceBuffSize);

	//result = dev->CreateCommittedResource
	//(
	//	&heapProp,
	//	D3D12_HEAP_FLAG_NONE,
	//	&indicesDesc,
	//	D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadヒープでのリソース初期状態はこのタイプが公式ルール
	//	nullptr,
	//	IID_PPV_ARGS(sphereIbBuff.ReleaseAndGetAddressOf())
	//);
	////if (result != S_OK) return result;

	//result = sphereIbBuff->Map(0, nullptr, (void**)&mappedSphereIdx); // mapping
	//std::copy(std::begin(sphereColliderIndices), std::end(sphereColliderIndices), mappedSphereIdx);
	//sphereIbBuff->Unmap(0, nullptr);

	//sphereIBV.BufferLocation = sphereIbBuff->GetGPUVirtualAddress();
	//sphereIBV.SizeInBytes = indiceBuffSize;
	//sphereIBV.Format = DXGI_FORMAT_R32_UINT;
}


HRESULT OBBManager::CreateMatrixHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {}; // SRV用ディスクリプタヒープ
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 1; // matrix
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;

	auto result = dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(matrixHeap.ReleaseAndGetAddressOf()));
	return result;
}

HRESULT OBBManager::CreateMatrixResources()
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

void OBBManager::CreateMatrixView()
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

HRESULT OBBManager::MappingMatrix()
{
	auto result = matrixBuff->Map(0, nullptr, (void**)&mappedMatrix);
	return result;
}

void OBBManager::SetMatrix(XMMATRIX _world, XMMATRIX _view, XMMATRIX _proj)
{
	mappedMatrix->rotation = XMMatrixIdentity();
	mappedMatrix->world = _world;
	mappedMatrix->view = _view;
	mappedMatrix->proj = _proj;
}

void OBBManager::SetCharaPos(XMFLOAT3 _charaPos)
{
	charaPos = _charaPos;
}

void OBBManager::SetDraw(bool _isDraw, ID3D12GraphicsCommandList* _cmdList)
{
	mappedMatrix->isDraw = _isDraw;
	if (_isDraw)
	{
		Execution(_cmdList);
	}
}

void OBBManager::Execution(ID3D12GraphicsCommandList* _cmdList)
{
	_cmdList->SetGraphicsRootSignature(collisionRootSignature->GetRootSignature().Get());
	_cmdList->SetDescriptorHeaps(1, matrixHeap.GetAddressOf());
	auto gHandle = matrixHeap->GetGPUDescriptorHandleForHeapStart();
	_cmdList->SetGraphicsRootDescriptorTable(0, gHandle);

	_cmdList->SetPipelineState(colliderGraphicsPipelineSetting->GetPipelineState().Get());

	//プリミティブ型に関する情報と、入力アセンブラーステージの入力データを記述するデータ順序をバインド
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP/*D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP*/);

	//頂点バッファーのCPU記述子ハンドルを設定
	for (int i = 0; i < oBBVertices.size(); ++i)
	{
		_cmdList->IASetVertexBuffers(0, 1, &boxVBVs[i]);
		// インデックスバッファーのビューを設定
		_cmdList->IASetIndexBuffer(&boxIBVs[i]);
		_cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);

		//_cmdList->DrawInstanced(8, 1, 0, 0);
	}
	
}

void OBBManager::StoreIndiceOfOBB(std::map<int, std::vector<std::pair<float, int>>> res, int loopCnt, int index)
{
	std::map<int, std::vector<std::pair<float, int>>> res2;
	for (int j = 0; j < sizeof(oBBVertices[loopCnt].pos) / sizeof(XMFLOAT3); ++j)
	{
		auto v1 = XMVectorSubtract(XMLoadFloat3(&oBBVertices[loopCnt].pos[res[loopCnt][index].second]), XMLoadFloat3(&oBBVertices[loopCnt].pos[j]));
		std::pair<float, int> pair = { XMVector4Length(v1).m128_f32[0], j };
		res2[loopCnt].push_back(pair);
	}
	std::sort(res2[loopCnt].begin(), res2[loopCnt].end());

	// 球体同様に並びの規則性は不明。[0][1][2]のように[0]を先頭に並べるとOBB内にポリゴンが描画されてしまう。
	// ポリゴン1のインデックス
	oBBIndices[loopCnt].push_back(res2[loopCnt][2].second);
	oBBIndices[loopCnt].push_back(res2[loopCnt][0].second);
	oBBIndices[loopCnt].push_back(res2[loopCnt][1].second);	

	// ポリゴン2のインデックス
	oBBIndices[loopCnt].push_back(res2[loopCnt][3].second);
	oBBIndices[loopCnt].push_back(res2[loopCnt][0].second);
	oBBIndices[loopCnt].push_back(res2[loopCnt][1].second);	

	// ポリゴン3のインデックス
	oBBIndices[loopCnt].push_back(res2[loopCnt][3].second);
	oBBIndices[loopCnt].push_back(res2[loopCnt][0].second);
	oBBIndices[loopCnt].push_back(res2[loopCnt][2].second);
	
}
