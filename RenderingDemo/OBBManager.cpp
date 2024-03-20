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

	// �ײ�ް�p
	if (FAILED(collisionRootSignature->SetRootsignatureParam(dev)))
	{
		return false;
	}

	// �R���C�_�[�`��p
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

	// �R���C�_�[�p
	auto result = colliderGraphicsPipelineSetting->CreateGPStateWrapper(dev, collisionRootSignature, _vsCollisionBlob, _psCollisionBlob);

	return result;
}

// TODO : 1. �V�F�[�_�[�𕪂��āA�{�[���}�g���b�N�X�Ƃ̏�Z���Ȃ���&�G�b�W�̂ݒ��F�����{�b�N�X�Ƃ��ĕ\������A2. 8���_�̈ʒu�𐳂� 3. �����̃��b�V��(��Q��)�ƃL�����N�^�[���b�V���𔻕ʂ��ď����o����悤�ɂ���
void OBBManager::CreateInfo()
{
	auto vertmap1 = resourceManager[0]->/*GetIndiceAndVertexInfo()*/GetIndiceAndVertexInfoOfOBB();
	auto it = vertmap1.begin();
	std::map<std::string, std::vector<XMFLOAT3>> vertMaps;
	for (int i = 0; i < vertmap1.size(); ++i)
	{
		for (int j = 0; j < it->second.vertices.size(); ++j)
		{
			// �I�u�W�F�N�g�����_����I�t�Z�b�g���Ă���ꍇ�A�R���C�_�[��x���ɑ΂��đΏ̂̈ʒu�ɔz�u�����B����𒲐����ă��f���`��̈ʒu���R���C�_�[�Ɠ��ʒu�ɕς��Ă���B
			it->second.vertices[j].pos.z *= -1;
			vertMaps[it->first].push_back(it->second.vertices[j].pos);
		}
		it++;
	}

	boxes.resize(vertMaps.size());
	// �I�u�W�F�N�g�̒��_��񂩂炻�ꂼ���OBB�𐶐�����
	auto itVertMap = vertMaps.begin();

	// fbx���f����xyz���[�J����]�E���s�ړ��s��Q���擾
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

		// OBB�̒��_���t��]�����ĉ�]�����̏�Ԃɂ���B
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
	// �eOBB���N�H�[�^�j�I���ɂ���]�����AExtents�𒲐����A���̒��_�Q��`��ړI�Ŋi�[���Ă���
	for (int i = 0; i < vertMaps.size(); ++i)
	{
		// fbx���f����xyz���[�J����]�E���s�ړ��s�񂩂�N�H�[�^�j�I������
		XMVECTOR quaternion = XMQuaternionRotationMatrix(localTransitionAndRotation[itVertMap->first]);

		// OBB�̒��_����]������B�N�H�[�^�j�I���Ȃ̂�OBB���S�_�Ɋ�Â��p�����ω�����B���[���h��Ԍ��_�𒆐S�Ƃ�����]�ł͂Ȃ����Ƃɒ��ӁB
		XMFLOAT4 orientation;
		orientation.x = quaternion.m128_f32[0];
		orientation.y = -quaternion.m128_f32[1];
		orientation.z = quaternion.m128_f32[2];
		orientation.w = quaternion.m128_f32[3];
		boxes[i].GetCorners(oBBVertices[i].pos);

		++itVertMap;
	}
	// ���b�V���`��ɑ΂���x���Ώ̂̈ʒu�ɔz�u�����R���C�_�[�𒲐��������A���ɖ߂��ĕ`��ʒu�Ɠ����ʒu�ɃR���C�_�[��`�悳����B(�R���C�_�[�ʒu�ɂ͉e���������Ƃɒ���)
	for (int i = 0; i < vertMaps.size(); ++i)
	{
		for (int j = 0; j < sizeof(oBBVertices[i].pos) / sizeof(XMFLOAT3); ++j)
		{
			oBBVertices[i].pos[j].z *= -1;
		}
	}

	//// �L�����N�^�[�p�̃X�t�B�A�R���C�_�[����
	//auto vetmap2 = resourceManager[1]->GetIndiceAndVertexInfo();
	//for (int j = 0; j < 4454; ++j)
	//{
	//	input2.push_back(vetmap2.begin()->second.vertices[j].pos);
	//}
	//BoundingSphere::CreateFromPoints(bSphere, 4454, input2.data(), (size_t)sizeof(XMFLOAT3));
	//bSphere.Radius -= 0.2f; // ���̃R���C�_�[�̔��a��������B���l�K��

	//// ����L�����N�^�[�̋��̃R���C�_�[�쐬
	//CreateSpherePoints(bSphere.Center, bSphere.Radius);

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(oBBVertices[0]));

	//�� OBB�̐������\�[�X��p�ӂ��A�}�b�s���O����
	boxBuffs.resize(oBBVertices.size());
	boxVBVs.resize(oBBVertices.size());
	mappedOBBs.resize(oBBVertices.size());
	for (int i = 0; i < oBBVertices.size(); ++i)
	{
		// �o�b�t�@�[�쐬
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
		// �r���[�쐬
		boxVBVs[i].BufferLocation = boxBuffs[i]->GetGPUVirtualAddress();
		boxVBVs[i].SizeInBytes = sizeof(oBBVertices[i]);
		boxVBVs[i].StrideInBytes = sizeof(XMFLOAT3);
		// �}�b�s���O
		mappedOBBs[i] = nullptr;
		boxBuffs[i]->Map(0, nullptr, (void**)&mappedOBBs[i]);
		std::copy(std::begin(oBBVertices[i].pos), std::end(oBBVertices[i].pos), mappedOBBs[i]);
	}

	// ���_�C���f�b�N�X�𐶐����ă}�b�s���O����
	// �܂��͋������璸�_�̈ʒu�֌W��c������B���W�b�N�Ƃ��Ă�
	// 1. �eOBB��[0]���_�Ƒ����_�̋������Z�o���ď��������ɕ��ѕς���
	// 2. [0][1][2],[0][1][3],[0][2][3]��[0]���̃C���f�b�N�X�Ƃ��Ē��o����B
	// 3. �����ő�̂��̂������āA�����̑傫��3�_�͑Ίp�����`�����钸�_�ŁA���ꂼ��ɑ΂��Ď菇1,2���J��Ԃ��B
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
	//	// �|���S��1�̃C���f�b�N�X
	//	oBBIndices[i].push_back(res[i][0].second);
	//	oBBIndices[i].push_back(res[i][1].second);
	//	oBBIndices[i].push_back(res[i][2].second);

	//	// �|���S��2�̃C���f�b�N�X
	//	oBBIndices[i].push_back(res[i][0].second);
	//	oBBIndices[i].push_back(res[i][1].second);
	//	oBBIndices[i].push_back(res[i][3].second);

	//	// �|���S��3�̃C���f�b�N�X
	//	oBBIndices[i].push_back(res[i][0].second);
	//	oBBIndices[i].push_back(res[i][2].second);
	//	oBBIndices[i].push_back(res[i][3].second);

	//	// �ȉ��|���S��4��8�܂ŌJ��Ԃ�
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
			D3D12_RESOURCE_STATE_GENERIC_READ, // Upload�q�[�v�ł̃��\�[�X������Ԃ͂��̃^�C�v���������[��
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

	// �o�b�t�@�[�쐬2
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

	// �r���[�쐬
	boxVBV2.BufferLocation = boxBuff2->GetGPUVirtualAddress();
	boxVBV2.SizeInBytes = sizeof(output3);
	boxVBV2.StrideInBytes = sizeof(XMFLOAT3);

	// �}�b�s���O
	boxBuff2->Map(0, nullptr, (void**)&mappedBox2);
	std::copy(std::begin(output3), std::end(output3), mappedBox2);
	//boxBuff2->Unmap(0, nullptr);

	//// ���ޯ���ޯ̧�[�쐬
	//auto indexNum = sphereColliderIndices.size();
	//auto indiceBuffSize = indexNum * sizeof(unsigned int);
	//auto indicesDesc = CD3DX12_RESOURCE_DESC::Buffer(indiceBuffSize);

	//result = dev->CreateCommittedResource
	//(
	//	&heapProp,
	//	D3D12_HEAP_FLAG_NONE,
	//	&indicesDesc,
	//	D3D12_RESOURCE_STATE_GENERIC_READ, // Upload�q�[�v�ł̃��\�[�X������Ԃ͂��̃^�C�v���������[��
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
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {}; // SRV�p�f�B�X�N���v�^�q�[�v
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
		D3D12_RESOURCE_STATE_GENERIC_READ, // Upload�q�[�v�ł̃��\�[�X������Ԃ͂��̃^�C�v���������[��
		nullptr,
		IID_PPV_ARGS(matrixBuff.ReleaseAndGetAddressOf())
	);

	return result;
}

void OBBManager::CreateMatrixView()
{
	auto handle = matrixHeap->GetCPUDescriptorHandleForHeapStart();

	// frustum�pview
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

	//�v���~�e�B�u�^�Ɋւ�����ƁA���̓A�Z���u���[�X�e�[�W�̓��̓f�[�^���L�q����f�[�^�������o�C���h
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP/*D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP*/);

	//���_�o�b�t�@�[��CPU�L�q�q�n���h����ݒ�
	for (int i = 0; i < oBBVertices.size(); ++i)
	{
		_cmdList->IASetVertexBuffers(0, 1, &boxVBVs[i]);
		// �C���f�b�N�X�o�b�t�@�[�̃r���[��ݒ�
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

	// ���̓��l�ɕ��т̋K�����͕s���B[0][1][2]�̂悤��[0]��擪�ɕ��ׂ��OBB���Ƀ|���S�����`�悳��Ă��܂��B
	// �|���S��1�̃C���f�b�N�X
	oBBIndices[loopCnt].push_back(res2[loopCnt][2].second);
	oBBIndices[loopCnt].push_back(res2[loopCnt][0].second);
	oBBIndices[loopCnt].push_back(res2[loopCnt][1].second);	

	// �|���S��2�̃C���f�b�N�X
	oBBIndices[loopCnt].push_back(res2[loopCnt][3].second);
	oBBIndices[loopCnt].push_back(res2[loopCnt][0].second);
	oBBIndices[loopCnt].push_back(res2[loopCnt][1].second);	

	// �|���S��3�̃C���f�b�N�X
	oBBIndices[loopCnt].push_back(res2[loopCnt][3].second);
	oBBIndices[loopCnt].push_back(res2[loopCnt][0].second);
	oBBIndices[loopCnt].push_back(res2[loopCnt][2].second);
	
}
