# 10.10 Python 驱动 Control Rig

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 20 分钟

---

## 一、Python 在 Control Rig 中的用途

```
UE 编辑器支持 Python 脚本自动化：
  批量创建控制器（为 200 根骨骼逐一创建 Control 太慢）
  自动配置 IK Rig 的骨骼链映射
  批量重定向动画（一键处理数百个动画文件）
  生成标准化 Control Rig 模板

启用：
  Edit → Plugins → Scripting → Python Editor Script Plugin ✅
  Edit → Plugins → Editor Scripting Utilities ✅
```

---

## 二、Python 批量创建控制器

```python
import unreal

# 获取 Control Rig 蓝图资产
rig_path = "/Game/Characters/CR_MyCharacter"
rig = unreal.load_asset(rig_path)
hierarchy = rig.get_controller_hierarchy()
controller = rig.get_controller()

# 要为哪些骨骼创建控制器
bones_to_control = [
    "spine_01", "spine_02", "spine_03",
    "neck_01", "head",
    "upperarm_l", "lowerarm_l", "hand_l",
    "upperarm_r", "lowerarm_r", "hand_r",
]

for bone_name in bones_to_control:
    bone_key = unreal.RigElementKey(
        type=unreal.RigElementType.BONE,
        name=bone_name
    )
    # 获取骨骼的当前变换（用于设置控制器初始位置）
    bone_transform = hierarchy.get_global_transform(bone_key)
    
    # 创建控制器
    ctrl_name = f"ctrl_{bone_name}"
    ctrl_settings = unreal.RigControlSettings()
    ctrl_settings.control_type = unreal.RigControlType.EULER_TRANSFORM
    ctrl_settings.shape_name = "Circle_Thin"  # 圆圈形状
    ctrl_settings.shape_color = unreal.LinearColor(0.0, 1.0, 0.5, 1.0)  # 绿色
    
    controller.add_control(
        ctrl_name,
        unreal.RigElementKey(type=unreal.RigElementType.NONE, name=""),
        ctrl_settings,
        unreal.RigControlValue(),
        unreal.Transform(),
        bone_transform  # 初始位置 = 骨骼位置
    )
    print(f"Created control: {ctrl_name}")

print("Done! All controls created.")
```

---

## 三、Python 批量重定向动画

```python
import unreal

# 批量重定向源目录下所有动画
source_dir = "/Game/Animations/Source/"
output_dir = "/Game/Animations/Retargeted/"
retargeter_path = "/Game/Characters/RTG_SourceToTarget"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
retargeter = unreal.load_asset(retargeter_path)

# 获取源目录下所有动画
source_anims = unreal.EditorAssetLibrary.list_assets(source_dir)
anim_assets = [
    unreal.load_asset(a) for a in source_anims
    if unreal.EditorAssetLibrary.find_asset_data(a).asset_class == "AnimSequence"
]

print(f"Found {len(anim_assets)} animations to retarget...")

# 批量重定向
for anim in anim_assets:
    output_name = f"{output_dir}{anim.get_name()}_Retargeted"
    unreal.IKRetargeterController.retarget_animation_assets(
        retargeter,
        [anim],
        output_dir
    )
    print(f"Retargeted: {anim.get_name()}")

print("Batch retarget complete!")
```

---

## 四、延伸阅读

- 🔗 [UE Python 脚本文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/scripting-the-unreal-editor-using-python)
- 🔗 [Control Rig Python API](https://dev.epicgames.com/documentation/en-us/unreal-engine/control-rig-python-scripting-in-unreal-engine)
