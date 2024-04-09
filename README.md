# 概要
DirectX12を用いて3Dモデルを描画するプログラム
![Image 1](https://github.com/Ryo-Takahashi-0422/RenderingDemo/blob/main/examples/intro_.png)
  
# 実行
  ...RenderingDemoRebuild\x64\Release\RenderingDemo.exeもしくは  
  ...RenderingDemoRebuild\x64\Debug\RenderingDemo.exeを実行 
  
# デモ動画(音声無し)
[https://www.youtube.com/watch?v=pzYBP06b1Pk](https://www.youtube.com/watch?v=jvIl-Ih2M1Q)  
  
# 操作方法
- 前進：wキー  
- 右回転：→キー  
- 左回転：←キー  
  
# パラメータ調整機能
### Imguiを介してプログラムのパラメータ調整が可能
![Imgui1](https://github.com/Ryo-Takahashi-0422/RenderingDemo/blob/main/examples/imgui.png)
- Sun Angle : 太陽の球面座標における位置計算に用いるradian(x:φ, y:Θ)を設定
- Sky Resolution : Sky描画パスの解像度を設定
- Sky LUT Resolution : SkyLUT描画パスの解像度を設定
- Shadow Resolution : シャドウ描画パスの解像度を設定
- Draw : 各対象の描画on/off。Sponza(キャラクター以外の3Dモデル)、 Collider(OBB)、Air(大気レンダリング)、  
SSAO(スクリーンスペースアンビエントオクルージョン)、DOF(被写界深度)  
- Max FPS : 最大FPS値の設定  
- Anti Aliasing : NVIDIA公開fxaa.hlslによるFXAA（Fast Approximate Anti-Aliasing）on/off  
- Air Parameter, SkyLUT Parameter : シングルスキャッタリング計算過程で用いる変数の値を設定  
  
# 開発環境
- OS : Windows 11 Home(ver.23H2)  
- CPU : Intel® Core™ i9 プロセッサー  
- GPU : NVIDIA GeForce RTX 4070 Ti  
- RAM : 64GB  
  
# 開発期間
- 2023/8/23 ～ 2024/4/4
- 2023/3/1～2023/8/22の期間に書籍「DirectX12の魔導書 3Dレンダリングの基礎からMMDモデルを踊らせるまで」からDirextX12を学習した
このリポジトリはその際に作成したリポジトリ「tutorial-GrimoireOfTheDirectX12」から派生したもの
  
# 使用ソフトウェア
- Visual Studio Community 2019  
- Blender 2.81  
- PIX 2305.10  
  
# デジタルアセット
- Sponza  
https://www.intel.com/content/www/us/en/developer/topic-technology/graphics-research/samples.html  
- Conan  
https://www.turbosquid.com/3d-models/conan-rig-character-3d-model-1182019
- Mixamo(アニメーションデータ)  
https://www.mixamo.com/#/  
  
# 事前処理
1. 事前に3Dモデルを設定(BlenderでSponzaのテクスチャ設定・コライダー追加等)して、Conanにはmixamoよりダウンロードしたアニメーション設定およびテクスチャ設定を行う。
2. FbxConverterToBinaryで各モデルをfbx形式からbin形式に変換する。マテリアルや頂点といった情報を抽出するのに開発環境で約5秒ほどかかるため、事前に変換する処理を準備した。  
https://github.com/Ryo-Takahashi-0422/FBXConvertToBinary  
  
# 処理フロー
1. DirectX12の各リソース設定  
2. 3Dデータファイル読み込み  
3. ShadowFactor描画パス実行(SkyLUT描画パスで利用)  
4. ゲームループ開始(12. 以外はメインスレッドが実行)  
5. Imgui描画パス実行  
6. Sun描画パス実行  
7. シャドウマップ(VSM)描画パス実行  
8. Air描画パス実行(コンピュートシェーダー)  
9. SkyLUT描画パス実行(Sky描画パスで利用)  
10. Sky描画パス実行  
11. シャドウマップのガウシアンブラー描画パス実行  
12. ワーカースレッド1,2がSponzaとConan、コライダーの描画パス実行(マルチレンダリングでカラー、法線、デプスを出力)  
13. 各スレッドが出力した描画結果(カラー、法線)を統合する描画パス実行  
14. 統合したカラー情報のガウシアンブラー描画パス実行(Depth Of Field処理で利用)  
15. 各スレッドが出力した描画結果(デプス)を統合する描画パス実行  
16. SSAO描画パス実行  
17. SSAOのガウシアンブラー描画パス実行  
18. 各出力結果の結合描画パス実行(カラー、imgui、SSAO、DOFをデプス値により処理)  
19. FXAA描画パス実行  
20. ダブルバッファリングで描画結果を交互に表示  
  
# 参考(一部のみ記載)
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
