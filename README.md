# 概要
DirectX12を用いて3Dモデルを描画するプログラムです。  
  
# 紹介動画
  
# 開発環境
- OS : Windows 11 Home(ver.23H2)  
- CPU : Intel® Core™ i9 プロセッサー  
- GPU : NVIDIA GeForce RTX 4070 Ti  
- RAM : 64GB  
  
# 開発期間
- 2023/3/1 ～ 2024/4/4
  
# 使用ソフトウェア
- Visual Studio Community 2019  
- Blender 2.81  
- PIX 2305.10  
  
# デジタルアセット
- Sponza  
https://www.intel.com/content/www/us/en/developer/topic-technology/graphics-research/samples.html  
- Conan  
https://www.turbosquid.com/3d-models/conan-rig-character-3d-model-1182019  
  
# 事前処理
1. 事前に3Dモデルを設定しています。BlenderでSponzaのテクスチャ設定・コライダー追加等を行い、Conanにはmixamoよりダウンロードしたアニメーション設定およびテクスチャ設定を行います。  
2. FbxConverterToBinaryで各モデルをfbx形式からbin形式に変換します。変換処理ではマテリアルや頂点といった情報を抽出するため開発環境で約7秒ほどかかるため、事前に変換する処理を準備しました。  
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
12. ワーカースレッド1,2がSponzaとConanの描画パス実行(マルチレンダリングでカラー、法線、デプスを出力)  
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
