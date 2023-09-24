#include <stdafx.h>
#include <CollisionManager.h>

CollisionManager::CollisionManager(ComPtr<ID3D12Device> _dev, std::vector<ResourceManager*> _resourceManagers)
{
	dev = _dev;
	resourceManager = _resourceManagers;
	Init();
}


// TODO : 1. シェーダーを分けて、ボーンマトリックスとの乗算をなくす&エッジのみ着色したボックスとして表示する、2. 8頂点の位置を正す
void CollisionManager::Init()
{
	auto vetmap1 = resourceManager[0]->GetIndiceAndVertexInfo();
	auto vetmap2 = resourceManager[1]->GetIndiceAndVertexInfo();

	///
	auto loalRotation = resourceManager[0]->GetLocalRotationFloat();
	XMMATRIX localRotaionXMatrix = XMMatrixRotationX(loalRotation.x);
	XMMATRIX localRotaionYMatrix = XMMatrixRotationY(45); // blenderと符号逆
	XMMATRIX localRotaionZMatrix = XMMatrixRotationZ(loalRotation.z);
	XMMATRIX localRotaionMatrix = /*localRotaionXMatrix * */localRotaionYMatrix/* * localRotaionZMatrix*/;
	XMVECTOR quaternion = XMQuaternionRotationMatrix(localRotaionMatrix);
	///
	//resourceManager[0]->GetMappedMatrix()->world *= localRotaionMatrix; // モデルとコライダー描画はworld原点中心に回転するも、コライダー実態には影響無し。惜しい。これ+コライダーのworld原点回転か...?
	
	//XMVECTOR qu = XMQuaternionRotationAxis();
	for (int j = 0; j < resourceManager[0]->GetVertexTotalNum(); ++j)
	{
		//// 回転している場合も考慮する...以下コードは平行移動にしかならない。
		//XMVECTOR pos = XMLoadFloat3(&vetmap1.begin()->second.vertices[j].pos);
		//pos.m128_f32[3] = 1;
		//pos = XMVector3Rotate(pos, quaternion);//XMVector3Transform(pos, localRotaionMatrix);
		//vetmap1.begin()->second.vertices[j].pos.x = pos.m128_f32[0];
		//vetmap1.begin()->second.vertices[j].pos.y = pos.m128_f32[1];
		//vetmap1.begin()->second.vertices[j].pos.z = pos.m128_f32[2];

		// ★ オブジェクトが原点からオフセットしている場合、コライダーがx軸に対して対称の位置に配置される。これを調整してモデル描画の位置をコライダーと同位置に変えている。少しずれている？
		vetmap1.begin()->second.vertices[j].pos.z *= -1;
		input1.push_back(vetmap1.begin()->second.vertices[j].pos);
	}

	for (int j = 0; j < 4454; ++j)
	{
		input2.push_back(vetmap2.begin()->second.vertices[j].pos);
	}

	/*BoundingBox*/BoundingOrientedBox::CreateFromPoints(box1, input1.size(), input1.data(), (size_t)sizeof(XMFLOAT3));
	// fbxモデルデータからのxyz回転成分に基づきOBB回転
	XMVECTOR scale;
	XMVECTOR rotation;
	XMVECTOR transition;
	XMMatrixDecompose(&scale, &rotation, &transition, localRotaionMatrix);
	/*box1.Transform(box1, 1.0f, rotation, transition);*/

	/*resourceManager[0]->GetMappedMatrix()->world *= localRotaionMatrix;*/
	///

	BoundingBox::CreateFromPoints(box2, 4454, input2.data(), (size_t)sizeof(XMFLOAT3));
	BoundingSphere::CreateFromPoints(bSphere, 4454, input2.data(), (size_t)sizeof(XMFLOAT3));
	bSphere.Radius -= 0.2f; // 球体コライダーの半径を微調整。数値適当



	XMFLOAT3 temp1[8];
	XMFLOAT3 temp2[8];
	XMFLOAT3 temp3[8];
	box1.GetCorners(temp1);
	box2.GetCorners(temp2);
	
	//localRotaionMatrix.r[0].m128_f32[0] *= -1;
	//localRotaionMatrix.r[0].m128_f32[2] *= -1;
	//localRotaionMatrix.r[2].m128_f32[0] *= -1;
	//localRotaionMatrix.r[2].m128_f32[2] *= -1;
	//auto k = XMVector3Transform(XMLoadFloat3(&box1.Center), localRotaionMatrix);
	//box1.Center.x = k.m128_f32[0];
	//box1.Center.y = k.m128_f32[1];
	//box1.Center.z = k.m128_f32[2];

	for (int i = 0; i < 8; ++i)
	{
		output1[i] = temp1[i];
		output1[i].z *= -1; // x軸に対して対称の位置に配置されるコライダーを調整したが、元に戻して描画位置と同じ位置にコライダーを描画させる。
		output2[i] = temp2[i];
	}

	CreateSpherePoints(bSphere.Center, bSphere.Radius);

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(output1));
		
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
	boxVBV1.SizeInBytes = sizeof(output1);
	boxVBV1.StrideInBytes = sizeof(XMFLOAT3);

	// マッピング
	boxBuff1->Map(0, nullptr, (void**)&mappedBox1);
	std::copy(std::begin(output1), std::end(output1), mappedBox1);
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
	printf("%f\n", bSphere.Center.x);
	printf("%f\n", bSphere.Center.y);
	printf("%f\n", bSphere.Center.z);
	printf("\n");

	printf("%d\n", box1.Contains(bSphere));
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