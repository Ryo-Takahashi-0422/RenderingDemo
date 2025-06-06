# 概要
DirectX12を用いて3Dモデルを描画するプログラムです。  
以下は実装した機能の一例になります。  
・3D モデルのアニメーションの実装  
・論文に基づくシングルスキャッタリングによるスカイシミュレーションの実装  
・大気の実装  
・太陽の実装  
・アンビエントオクルージョン(SSAO、RTAO)の実装  
・オクルージョンカリングの実装  
・BRDFベースのライティング実装  
  
![Image 1](https://github.com/Ryo-Takahashi-0422/RenderingDemo/blob/main/examples/intro_.png)
  
# 実行方法
  ...RenderingDemoRebuild\x64\Release\RenderingDemo.exeもしくは  
  ...RenderingDemoRebuild\x64\Debug\RenderingDemo.exeを実行下さい。 
  
# 苦労した点
  ・DirectXに触れたことが無い状態から、DirectX12ライブラリの利用方法を独学で学んだこと  
  ・FBXモデルの事前バイナリ化  
  ・キャラクターモデルを、テクスチャ情報やアニメーション含め全て正しく想定通りに描画すること  
  ・衝突判定の実装(購入した分厚い参考書は読まず、力技で設計して実装)  
  ・衝突判定用の球体ポリゴンを描画するための頂点インデクス設定  
  ・視錐台の上下左右端部の座標算出方法を理解すること  
  ・カーテンの影を表現すべく導入したAOは、何度試しても納得のいく結果にはなりませんでした。  
  ・空のシミュレーションに関する海外論文の理解(丸一ヵ月かかりました...)  
  ・ボリュームレンダリング(大気の3Dテクスチャ作成に使用しました)について、名前しか聞いたことが無い状態からスタートしたこと  
  ・空のシミュレーションに関する実装全般(DirectX11で実装された方のプログラムを参照させていただきました。  
    ボリュームレンダリングやコンピュートシェーダーの利用方法などはそこから学びました)  
  ・太陽の描画において、太陽が瞬間移動したり形が楕円になるなどの計算ミス対応  
  ・pixでのデバッグ方法。Youtube動画などを見て見よう見まねで、根気よく触っているうちにピクセル毎のデバッグを理解しました。  
  
# 工夫した点
  ・FBXモデルの解析を別の自作プログラムにより事前に行うことで、解析にかかる時間を本プログラムの起動時間から取り除きました。  
  ・キャラクターのピーターパニング現象を出来るだけ抑えました(隠しきれておりませんが...)  
  ・太陽が建物の影から出てくるタイミングと、大気のボリュームレンダリングが自然に見えるように調整  
  ・立体感を増すためにAOを実装しました。  
  ・カーテンの微細なテクスチャにノイズが生じないよう、LODを実装しました。  
  ・衝突判定を可視化するため、衝突用ポリゴンをワイヤーフレームで描画するモードを用意しました。  
  ・建物モデル、大気、AO、DOF、BRDFの描画切り替えモードも用意しました。  
  ・影や空のテクスチャの解像度を動的に変更できるようにして、描画負荷や見た目のチェックを可能としました。  
  ・処理負荷の軽減を目的とし、オクルージョンカリングを実装しました。また、各種計算の効率化に努めました。  
    
# 実行確認済GPU
  NVIDIA GeForce RTX 4070 Ti  
  NVIDIA RTX A2000
  
# デモ動画(音声無し)
https://www.youtube.com/watch?v=J42AyHKEepc  
  
# 操作方法
- 前進：wキー  
- 右回転：→キー  
- 左回転：←キー  
  
# パラメータ調整機能
### Imguiを介してプログラムのパラメータ調整が可能
![Imgui1](https://github.com/Ryo-Takahashi-0422/RenderingDemo/blob/main/examples/imgui_.png)  
- Sun Angle : 太陽の球面座標における位置計算に用いるradian(x:φ, y:Θ)を設定  
- Air Parameter, SkyLUT Parameter : シングルスキャッタリング計算過程で用いる変数の値を設定  
- Sky Resolution : Sky描画パスの解像度を設定  
- Sky LUT Resolution : SkyLUT描画パスの解像度を設定  
- Shadow Resolution : シャドウ描画パスの解像度を設定  
- Draw : 各対象の描画on/off。Sponza(キャラクター以外の3Dモデル)、 Collider(OBB)、Air(大気レンダリング)、  
AO(アンビエントオクルージョン)、DOF(被写界深度)、鏡面反射BRDF  
- Max FPS : 最大FPS値の設定  
- Point Light Position : 鏡面反射BRDFの計算に使用するポイントライト位置を設定  
- AO Type : SSAO(スクリーンスペースAO)とRTAO(レイトレーシングAO)の切り替え  
- Anti Aliasing : NVIDIA公開fxaa.hlslによるFXAA（Fast Approximate Anti-Aliasing）on/off  
  
# 開発環境
- OS : Windows 11 Home(ver.23H2)  
- CPU : Intel® Core™ i9 13900K  
- GPU : NVIDIA GeForce RTX 4070 Ti  
- RAM : 64GB  
  
# 開発期間
- 2023/8/23 ～ 2024/04/30  
- 2023/3/1～2023/8/22の期間に書籍「DirectX12の魔導書 3Dレンダリングの基礎からMMDモデルを踊らせるまで」からDirextX12を学習、
このリポジトリはその際に作成したリポジトリ「tutorial-GrimoireOfTheDirectX12」から派生したもの
  
# 使用ソフトウェア
- Visual Studio Community 2019  
- Blender 2.81  
- PIX 2305.10
  
# 外部ライブラリ
- DirectX12  
- DirectXMath  
- DirectXTex  
- ImGui  
- WinPixEventRuntime  
  
# デジタルアセット
- Sponza(オリジナルをアレンジしたデータ、テクスチャを利用)  
https://www.intel.com/content/www/us/en/developer/topic-technology/graphics-research/samples.html  
- Conan  
https://www.turbosquid.com/3d-models/conan-rig-character-3d-model-1182019
- Mixamo(アニメーションデータ)  
https://www.mixamo.com/#/  
  
# 事前処理
1. 事前に3Dモデルを設定(BlenderでSponzaのテクスチャ設定・コライダー追加・オクルージョンカリング用モデル追加等)して、Conanにはmixamoよりダウンロードしたアニメーション設定およびテクスチャ設定を行う。
2. FbxConverterToBinaryで各モデルをfbx形式からbin形式に変換する。マテリアルや頂点といった情報を抽出するのに開発環境で約5秒ほどかかるため、事前に変換する処理を準備した。  
https://github.com/Ryo-Takahashi-0422/FBXConvertToBinary  
  
# 処理フロー
1. DirectX12の各リソース設定  
2. 3Dデータファイル読み込み  
3. ミップマップ生成(5レベル)  
4. ShadowFactor描画パス実行(SkyLUT描画パスで利用)  
5. ゲームループ開始(12. 以外はメインスレッドが実行)  
6. Imgui描画パス実行  
7. Sun描画パス実行  
8. シャドウマップ(VSM)描画パス実行  
9. Air描画パス実行(コンピュートシェーダー)  
10. SkyLUT描画パス実行(Sky描画パスで利用)  
11. Sky描画パス実行  
12. シャドウマップのガウシアンブラー描画パス実行  
13. オクルージョンカリング用モデルによる深度マップを描画  
14. ワーカースレッド1,2がSponzaとConan、コライダーの描画パス実行(マルチレンダリングでカラー、法線、デプスを出力)  
15. 各スレッドが出力した描画結果(カラー、法線)を統合する描画パス実行  
16. 統合したカラー情報のガウシアンブラー描画パス実行(Depth Of Field処理で利用)  
17. 各スレッドが出力した描画結果(デプス)を統合する描画パス実行  
18. AO描画パス実行  
19. AOのガウシアンブラー描画パス実行  
20. 各出力結果の結合描画パス実行(カラー、imgui、SSAO、DOFをデプス値により処理)  
21. FXAA描画パス実行  
22. ダブルバッファリングで描画結果を交互に表示  
  
# 参考(一部)
## 図書、論文  
- DirectX12の魔導書 3Dレンダリングの基礎からMMDモデルを踊らせるまで  
https://www.shoeisha.co.jp/book/detail/9784798161938  
- HLSLシェーダーの魔導書  
https://www.shoeisha.co.jp/book/detail/9784798164281  
- ゲームエンジンアーキテクチャ第3版  
https://www.borndigital.co.jp/book/19115/  
- リアルタイムレンダリング第4版  
https://www.borndigital.co.jp/book/15291/  
- A Scalable and Production Ready Sky and Atmosphere Rendering Technique  
https://sebh.github.io/publications/egsr2020.pdf  
  
## 実装  
- MicroSoft's project 「DirectX-Graphics-Samples」  
https://github.com/microsoft/DirectX-Graphics-Samples  
- AirGuanZ's project 「AtmosphereRenderer」  
https://github.com/AirGuanZ/AtmosphereRenderer  
  
## 動画、ブログ  
- コンピュータグラフィックスの空  
https://www.youtube.com/watch?v=YrQ4ACuvM68  
- A Minimal Ray-Tracer  
https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html  
- 下町のナポレオン(Hatena Blog)  
https://hikita12312.hatenablog.com/  
  
## メモ  
- 2024/04/30 オクルージョンカリング実装により最大で0.9msの描画コスト削減とした。  
