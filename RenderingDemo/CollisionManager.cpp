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

	//★xyz max,min test BattleField.fbxの壁のcenterがNanになる。原因は頂点数？最小データでOBB作成したい
	std::map<std::string, std::vector<float>> xContainer, yContainer, zContainer;
	int containerSize = vertMaps.begin()->second.size();
	for (int i = 0; i < vertMaps.size(); ++i)
	{
		for (int j = 0; j < itVertMap->second.size(); ++j)
		{
			xContainer[itVertMap->first].push_back(itVertMap->second[j].x);
			yContainer[itVertMap->first].push_back(itVertMap->second[j].y);
			zContainer[itVertMap->first].push_back(itVertMap->second[j].z);
		}
		++itVertMap;
	}

	std::map<std::string, std::vector<XMFLOAT3>> boxPoints;
	itVertMap = vertMaps.begin();
	//std::vector<float> xMax, xMin, yMax, yMin, zMax, zMin;
	for (int i = 0; i < vertMaps.size(); ++i)
	{
		auto xMax = *std::max_element(xContainer[itVertMap->first].begin(), xContainer[itVertMap->first].end());
		auto xMin = *std::min_element(xContainer[itVertMap->first].begin(), xContainer[itVertMap->first].end());
		auto yMax = *std::max_element(yContainer[itVertMap->first].begin(), yContainer[itVertMap->first].end());
		auto yMin = *std::min_element(yContainer[itVertMap->first].begin(), yContainer[itVertMap->first].end());
		auto zMax = *std::max_element(zContainer[itVertMap->first].begin(), zContainer[itVertMap->first].end());
		auto zMin = *std::min_element(zContainer[itVertMap->first].begin(), zContainer[itVertMap->first].end());

		XMFLOAT3 xMaxYMaxZmax = { xMax ,yMax ,zMax };
		XMFLOAT3 xMaxYMinZmax = { xMax ,yMin ,zMax };
		XMFLOAT3 xMaxYMaxZmin = { xMax ,yMax ,zMin };
		XMFLOAT3 xMaxYMinZmin = { xMax ,yMin ,zMin };

		XMFLOAT3 xMinYMaxZmax = { xMin ,yMax ,zMax };
		XMFLOAT3 xMinYMinZmax = { xMin ,yMin ,zMax };
		XMFLOAT3 xMinYMaxZmin = { xMin ,yMax ,zMin };
		XMFLOAT3 xMinYMinZmin = { xMin ,yMin ,zMin };

		boxPoints[itVertMap->first].push_back(xMaxYMaxZmax);
		boxPoints[itVertMap->first].push_back(xMaxYMinZmax);
		boxPoints[itVertMap->first].push_back(xMaxYMaxZmin);
		boxPoints[itVertMap->first].push_back(xMaxYMinZmin);
		boxPoints[itVertMap->first].push_back(xMinYMaxZmax);
		boxPoints[itVertMap->first].push_back(xMinYMinZmax);
		boxPoints[itVertMap->first].push_back(xMinYMaxZmin);
		boxPoints[itVertMap->first].push_back(xMinYMinZmin);

		++itVertMap;
	}

	auto itBoxPoints = boxPoints.begin();
	for (int i = 0; i < vertmap1.size(); ++i)
	{
		BoundingOrientedBox::CreateFromPoints(boxes[i], itBoxPoints->second.size(), itBoxPoints->second.data(), (size_t)sizeof(XMFLOAT3));
		++itBoxPoints;
	}
	//★///

	//itVertMap = vertMaps.begin();
	//for (int i = 0; i < vertmap1.size(); ++i)
	//{
	//	BoundingOrientedBox::CreateFromPoints(boxes[i], itVertMap->second.size(), itVertMap->second.data(), (size_t)sizeof(XMFLOAT3));
	//	++itVertMap;
	//}

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
		//boxes[i].Extents.x = yLen; //★★★要修正 BattleField壁など長方形OBBの形が崩れる原因。ただし、現状はOBBが正六面体であることを前提とした衝突実装になっており、これをコメントアウトすると「壁だけ」衝突時にバグる...岩は問題無しに見える...
		//boxes[i].Extents.z = yLen; //★★★要修正 BattleField壁など長方形OBBの形が崩れる原因。ただし、現状はOBBが正六面体であることを前提とした衝突実装になっており、これをコメントアウトすると「壁だけ」衝突時にバグる...岩は問題無しに見える...
		
		//★Extentsを調整した結果、boxesは問題ないがBattleFieldは角めり込み発生する。無しの方向でいくか...
		//float adjustExtents = abs(localTransitionAndRotation[i].r[0].m128_f32[0]) + abs(localTransitionAndRotation[i].r[0].m128_f32[2]);
		//boxes[i].Extents.x /= adjustExtents;
		//boxes[i].Extents.z /= adjustExtents;

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

bool CollisionManager::OBBCollisionCheck()
{
	bool result = true;
	for (auto& box : boxes)
	{
		if (box.Contains(bSphere) != 0)
		{
			result = false;
			collidedOBB = box;
			break;
		}
	}

	return result;
}

void CollisionManager::OBBTransrationWithCollision(float forwardSpeed, XMMATRIX characterDirection, int fbxIndex)
{
	XMFLOAT3 boxCenterPos = collidedOBB.Center;
	XMVECTOR boxCenterVec = XMLoadFloat3(&collidedOBB.Center);
	BoundingSphere reserveSphere = bSphere; // 操作キャラクターコリジョンを動かす前の情報を残しておく
	auto sCenter = bSphere.Center;
	MoveCharacterBoundingBox(forwardSpeed, characterDirection); // move collider

	if (OBBCollisionCheck()/*collisionManager->GetBoundingBox1()[debugNum].Contains(collisionManager->GetBoundingSphere()) == 0*/)
	{
		resourceManager[fbxIndex]->GetMappedMatrix()->world *= XMMatrixTranslation(0, 0, -forwardSpeed);
	}

	else
	{
		auto moveMatrix = XMMatrixIdentity(); // 滑らせように初期化して使いまわし
		XMFLOAT3 boxVertexPos[8];
		collidedOBB.GetCorners(boxVertexPos);
		bSphere.Center = reserveSphere.Center; // 衝突したのでキャラクターコリジョンの位置を元に戻す

		// ★面滑らせ実装
		std::vector<std::pair<float, int>> distances;
		distances.resize(8);
		for (int i = 0; i < 8; ++i)
		{
			distances[i].first = 1000.0f;
			distances[i].second = 0;
		}
		// キャラクターのコライダー中心に対して、障害物ボックスコライダーからの距離を計算して、近いものを4つ選出する。これらが衝突面を構成する点となる。

		for (int h = 0; h < 8; ++h)
		{
			float distance = powf(sCenter.x - boxVertexPos[h].x, 2.0f) + powf(sCenter.y - boxVertexPos[h].y, 2.0f) + powf(sCenter.z - boxVertexPos[h].z, 2.0f);
			for (int i = 0; i < 8; ++i)
			{
				if (distance < distances[i].first)
				{
					distances[i].first = distance;
					distances[i].second = h;
					std::sort(distances.rbegin(), distances.rend());
					break;
				}
			}
		}
		std::sort(distances.begin(), distances.end()); // 距離の降順になっているので昇順にする
		// 選出した最も近い3点のXYZ座標を抽出する。
		float epsilon = 0.00005f;
		auto it = distances.begin();
		std::vector<XMFLOAT3> boxPoint4Cal;
		boxPoint4Cal.push_back(boxVertexPos[it->second]); // 最初の頂点を格納してイテレータを進める
		++it;
		for (int i = 0; i < 2; ++i)
		{
			// 衝突面の中心では各頂点までの距離が重複する可能性があるため、最初の頂点に基づき座標の差で二つ目以降を求めていく
			float x = abs(boxPoint4Cal[0].x) - abs(boxVertexPos[it->second].x);
			float y = abs(boxPoint4Cal[0].y) - abs(boxVertexPos[it->second].y);
			float z = abs(boxPoint4Cal[0].z) - abs(boxVertexPos[it->second].z);
			if (abs(x) < epsilon && abs(z) < epsilon && abs(y) > epsilon)
			{
				boxPoint4Cal.push_back(boxVertexPos[it->second]);
				// 3つ目の頂点を格納する。3つ目のイテレータまでに要素が決定するためi = 1で処理終了する。
				if (i == 0)
				{
					++it;
					boxPoint4Cal.push_back(boxVertexPos[it->second]);
				}
				else if (i == 1)
				{
					--it;
					boxPoint4Cal.push_back(boxVertexPos[it->second]);
				}
			}
			++it;
		}
		// 2点目もしくは3点目から4点目を決定する
		auto fourthPoint = CalculateForthPoint(boxPoint4Cal, boxVertexPos);
		boxPoint4Cal.push_back(fourthPoint);
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

		// キャラクターコライダーが角にぶつかっているか判定する
		// キャラクターに近いもう片方の面を構成する座標を格納する。こちらは角との衝突判定に使う法線を求めるために利用する。
		it = distances.begin();
		std::vector<XMFLOAT3> boxPoint4NextPlaneCal;
		boxPoint4NextPlaneCal.push_back(boxPoint4Cal[0]); // 最初の要素を格納しておく
		boxPoint4NextPlaneCal.push_back(boxPoint4Cal[1]); // 二つ目の要素を格納しておく
		// 3つ目の要素を抽出する。	
		it += 4; // distancesの上位4番目まで、もしくは上位1,2,5,6位が選定された状態。どちらか分からないので一旦上位5番目と予想される要素に基づき値を調べていく				
		// 角ではbox4Calの頂点とかぶるので対処する
		float x1 = abs(boxPoint4Cal[2].x) - abs(boxVertexPos[it->second].x);
		float z1 = abs(boxPoint4Cal[2].z) - abs(boxVertexPos[it->second].z);

		float x2 = abs(boxPoint4Cal[3].x) - abs(boxVertexPos[it->second].x);
		float z2 = abs(boxPoint4Cal[3].z) - abs(boxVertexPos[it->second].z);

		// distancesのitretorは[4]つまり5番目に近い頂点を指している状態。ただし、角に衝突した時は5番目に近いとは限らず4番目に近い頂点の可能性がある。
		// そこで、[5]を指すことで3番目に近い線を構成する片方の頂点を確実に選定することが可能。この頂点に基づき4つ目の頂点を計算する。
		if (abs(x1) < epsilon && abs(z1) < epsilon) // 既にbox4Calに上位5番目と推測される要素が格納されている場合は実際の上位5か6番目である[6]を利用する。
		{
			++it;
			boxPoint4NextPlaneCal.push_back(boxVertexPos[it->second]);
		}

		else if (abs(x2) < epsilon && abs(z2) < epsilon) // 既にbox4Calに上位5番目と推測される要素が格納されている場合は実際の上位5か6番目である[6]を利用する。
		{
			++it;
			boxPoint4NextPlaneCal.push_back(boxVertexPos[it->second]);
		}
		else // box4Calに上位5番目と推測される要素が格納されていない場合はこのまま格納する
		{
			boxPoint4NextPlaneCal.push_back(boxVertexPos[it->second]);
		}

		// 2点目もしくは3点目から4点目を決定する
		fourthPoint = CalculateForthPoint(boxPoint4NextPlaneCal, boxVertexPos);
		boxPoint4NextPlaneCal.push_back(fourthPoint);
		// 法線ベクトルとスライド方向を算出する
		auto normalAndSlideVectorOfNextPlane = CalcurateNormalAndSlideVector(boxPoint4NextPlaneCal, boxCenterPos);
		auto normalOfNextPlane = normalAndSlideVectorOfNextPlane.first;

		auto centerToCenter = XMVectorSubtract(XMLoadFloat3(&sCenter), boxCenterVec); // 衝突したOBB中心座標→キャラクターコライダーまでのベクトル
		centerToCenter = XMVector3Normalize(centerToCenter);
		auto dot1 = XMVector3Dot(centerToCenter, normal);
		auto dot2 = XMVector3Dot(centerToCenter, normalOfNextPlane); // 0.5以上のときは角と衝突している

		// 角に接触していない場合
		if (dot2.m128_f32[0] < 0.5f)
		{
			// Z軸がFBX(-Z前方)モデルに対して反転している。モデルの向きを+Z前方にしたいが一旦このままで実装を進める
			for (int i = 0; i < boxes.size(); ++i)
			{
				boxes[i].Center.x += moveMatrix.r[3].m128_f32[0];
				boxes[i].Center.y += moveMatrix.r[3].m128_f32[1];
				boxes[i].Center.z -= moveMatrix.r[3].m128_f32[2];
			}
			//collidedOBB.Center.x += moveMatrix.r[3].m128_f32[0];
			//collidedOBB.Center.y += moveMatrix.r[3].m128_f32[1];
			//collidedOBB.Center.z -= moveMatrix.r[3].m128_f32[2];

			// オブジェクトはシェーダーで描画されているのでworld変換行列の影響を受けている。moveMatrixはキャラクターの進行方向と逆にするため-1掛け済なので、
			// 更にキャラクターの向きを掛けてwolrd空間におきてキャラクターの逆の向きにオブジェクトが流れるようにする
			moveMatrix *= characterDirection;
			moveMatrix.r[0].m128_f32[0] = 1;
			moveMatrix.r[0].m128_f32[2] = 0;
			moveMatrix.r[2].m128_f32[0] = 0;
			moveMatrix.r[2].m128_f32[2] = 1;

			resourceManager[fbxIndex]->GetMappedMatrix()->world *= moveMatrix;
		}

		// 角に接触している場合
		else if (dot2.m128_f32[0] >= 0.5f)
		{
			// 衝突面の法線方向と隣面の法線方向の合成ベクトルを利用して、角から弾かれるようにしてコライダー同士の交差を防ぐ
			// ガタついた動きになるのがネック
			float normalWeight = 1.0f - dot2.m128_f32[0];
			float nextNormalWeight = dot2.m128_f32[0];
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
			moveMatrix.r[3].m128_f32[0] *= -1;
			//moveMatrix.r[3].m128_f32[1] += centerToCenter.m128_f32[1];
			moveMatrix.r[3].m128_f32[2] = slideCol.m128_f32[2];
			// Z軸がFBX(-Z前方)モデルに対して反転している。モデルの向きを+Z前方にしたいが一旦このままで実装を進める
			for (int i = 0; i < boxes.size(); ++i)
			{
				boxes[i].Center.x += moveMatrix.r[3].m128_f32[0];
				boxes[i].Center.y += moveMatrix.r[3].m128_f32[1];
				boxes[i].Center.z -= moveMatrix.r[3].m128_f32[2];
			}
			//collidedOBB.Center.x += moveMatrix.r[3].m128_f32[0];
			//collidedOBB.Center.y += moveMatrix.r[3].m128_f32[1];
			//collidedOBB.Center.z -= moveMatrix.r[3].m128_f32[2];

			// オブジェクトはシェーダーで描画されているのでworld変換行列の影響を受けている。moveMatrixはキャラクターの進行方向と逆にするため-1掛け済なので、
			// 更にキャラクターの向きを掛けてwolrd空間におきてキャラクターの逆の向きにオブジェクトが流れるようにする
			moveMatrix *= characterDirection;
			moveMatrix.r[0].m128_f32[0] = 1;
			moveMatrix.r[0].m128_f32[2] = 0;
			moveMatrix.r[2].m128_f32[0] = 0;
			moveMatrix.r[2].m128_f32[2] = 1;

			resourceManager[fbxIndex]->GetMappedMatrix()->world *= moveMatrix;
		}
	}
	
}

XMFLOAT3 CollisionManager::CalculateForthPoint(std::vector<XMFLOAT3> storedPoints, XMFLOAT3 boxPoints[8])
{
	float epsilon = 0.00005f;
	float xVal = abs(storedPoints[0].x) - abs(storedPoints[1].x);
	float yVal = abs(storedPoints[0].y) - abs(storedPoints[1].y);
	float zVal = abs(storedPoints[0].z) - abs(storedPoints[1].z);
	XMFLOAT3 result;

	// 3点目から4点目を決定する
	if (abs(xVal) < epsilon && abs(zVal) < epsilon && abs(yVal) > epsilon)
	{
		for (int i = 0; i < 8; ++i)
		{
			float xVal = abs(storedPoints[2].x) - abs(boxPoints[i].x);
			float yVal = abs(storedPoints[2].y) - abs(boxPoints[i].y);
			float zVal = abs(storedPoints[2].z) - abs(boxPoints[i].z);
			if (abs(xVal) < epsilon && abs(zVal) < epsilon && abs(yVal) > epsilon)
			{
				result = boxPoints[i];
			}
		}
	}

	// 2点目から4点目を決定する
	else
	{
		for (int i = 0; i < 8; ++i)
		{
			float xVal = abs(storedPoints[1].x) - abs(boxPoints[i].x);
			float yVal = abs(storedPoints[1].y) - abs(boxPoints[i].y);
			float zVal = abs(storedPoints[1].z) - abs(boxPoints[i].z);
			if (abs(xVal) < epsilon && abs(zVal) < epsilon && abs(yVal) > epsilon)
			{
				result = boxPoints[i];
			}
		}
	}

	return result;
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
	//auto boxCenter = collidedOBB.Center;
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