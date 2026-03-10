# 10.6 流体表面波纹

> **难度**：⭐⭐⭐⭐☆

```
流体表面波纹的三种实现方案：

1. Gerstner Wave（Water Plugin 内置）
   多层正弦波叠加
   材质中通过 WaterWaveFunction 节点计算顶点偏移
   性能好，适合大面积水体

2. Render Target 波纹
   角色进入水中 → 写入 RT 中的涟漪信息（圆形向外扩散）
   法线贴图采样 RT → 水面法线扭曲
   → 精细的入水涟漪，可叠加多个
   
   工作流：
     创建 RenderTarget2D（256×256 足够）
     创建 Material Function 读取 RT → 转为法线偏移
     在 Character Tick 中：DrawMaterialToRenderTarget(RT, WaveMaterial, Loc)
     
3. Niagara Fluid + RenderTarget 输出
   最精确，见 02-niagara-fluids.md
```
