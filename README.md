# RenderingDemo


# 利用したリソース
- Sponza  
https://www.intel.com/content/www/us/en/developer/topic-technology/graphics-research/samples.html  
- Conan  
https://www.turbosquid.com/3d-models/conan-rig-character-3d-model-1182019
  
# 処理概要
1. 3Dモデル設定。BlenderでSponzaのテクスチャ設定・コライダー追加を行う。Conanにはmixamoよりダウンロードしたアニメーション設定およびテクスチャ設定を行う。
2. FbxConverterで各モデルをfbx形式からbin形式に変換する。(変換には開発環境で約7秒ほどかかるため、事前に変換する処理を準備した)
https://github.com/Ryo-Takahashi-0422/FBXConvertToBinary
3.   
  
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
