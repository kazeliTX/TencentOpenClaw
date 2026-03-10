# 11.4 Live Link 实时动作捕捉

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、Live Link 概述

```
Live Link 是 UE 的实时数据输入系统：
  连接外部动捕设备/软件 → 实时驱动 UE 中的角色
  
支持的数据源：
  iPhone ARKit（面部捕捉）← 最常用，手机即可
  Rokoko Suit（全身动捕）
  Vicon / OptiTrack（专业动捕）
  Xsens MVN（惯性动捕）
  Maya Live Link（Maya 直接驱动）
  任何第三方通过 UDP 发送的数据

数据类型：
  Transform（骨骼变换）← 全身动捕
  BlendShape（表情权重）← 面部捕捉（ARKit）
  Camera（摄像机动画）
```

---

## 二、Live Link 配置流程

```
步骤 1：启用插件
  Edit → Plugins → Live Link ✅
  Edit → Plugins → Apple ARKit（iPhone 面部）✅
  Edit → Plugins → Rokoko Live Link（如需 Rokoko）✅

步骤 2：打开 Live Link 窗口
  Window → Live Link

步骤 3：添加数据源
  + Source → 选择数据类型：
    Message Bus Source（UDP，大多数设备用这个）
    ARKit Live Link（iPhone 专用）
    
步骤 4：验证数据接收
  Live Link 窗口中看到 Subject（如 iPhone_Face）
  Subject 旁边显示绿色 = 数据正常接收

步骤 5：在 AnimBP 中使用
  AnimGraph → 搜索 Live Link Pose 节点
  Subject Name: iPhone_Face
  → 直接驱动骨骼/BlendShape
```

---

## 三、iPhone ARKit 面部捕捉连接

```
需要在 iPhone 上安装 "Live Link Face" App（Epic 官方，免费）：

手机端配置：
  1. 安装 Live Link Face App（App Store 搜索）
  2. App 设置 → Target：输入 UE 编辑器所在电脑 IP
  3. 确保手机和电脑在同一 WiFi 网络

UE 编辑器端：
  1. Window → Live Link → + Source → ARKit Live Link
  2. 看到 "iPhone_Face" Subject 出现
  
  3. 在 Face AnimBP 中：
     Live Link Pose 节点
     Subject Name: "iPhone_Face"（或实际显示的名字）
     
  4. 实时预览：演员对着手机做表情 → MetaHuman 同步

校准：
  Live Link Face App → 校准按钮
  演员保持中性表情 → 校准
  避免"零点漂移"（眉毛轻微上挑 → 映射为 0）
```

---

## 四、录制 Live Link 数据

```
实时捕捉满意后，可以录制为动画序列：

方法 1：Sequencer 录制
  Sequencer → + Track → Live Link → 添加 Subject
  点击录制 → 演员表演 → 停止录制
  → 自动生成 Animation Sequence

方法 2：Live Link Recorder（UE5 插件）
  Edit → Plugins → Live Link Recording ✅
  Window → Live Link Recorder
  选择要录制的 Subject → 录制
  → 更精确的录制控制

录制建议：
  120fps 捕捉（iPhone 支持最高 60fps，建议 60fps 录制）
  录制前做 3 秒静止校准帧
  后处理：Sequencer 中对 BlendShape 曲线做平滑（减少抖动）
```

---

## 五、延伸阅读

- 📄 [11.5 ARKit 面部捕捉详解](./05-arkit-face.md)
- 📄 [代码示例：Live Link 控制器](./code/01_livelink_controller.cpp)
- 🔗 [Live Link 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/live-link-in-unreal-engine)
