# 多分辨率融合
-----------------------

## 流程

### 第二章: 多尺度岩心图像三维重建算法

1. 输入: 一张训练图像```A```(二值图)    
输出: 重建出一个统计分析意义上等效的三维体```3D_A```.(序列图,长宽高相等)     
注意: 重建出来的三维体并非是实际的三维体,只是统计意义上的三维体.

### 第三章: 二维融合三维

- 二维融合三维
1. 输入: 步骤1得到的```3D_A```(高分辨率局部小孔) + 低分辨率(大孔)的三维体(真实的三维体)```3D_LowRes```(CT二值序列图)    
输出: 将```3D_A```(高分辨率的局部小孔)融合到低分辨率三维体中```3D_LowRes```(序列图).     
注意: 融合之后得到的三维体,大孔是实际上的大孔,融合进去的小孔并非真实的小孔,只是统计等效的小孔.

- 三维融合三维
1. 输入: 高分辨率局部小孔三维体```3D_HighRes```(CT二值序列图,真实结构,较小) + 低分辨率大孔三维体```3D_LowRes```(CT二值序列图,真实结构,较大)        
输出: 将小孔三维体融合进大孔三维体,得到最终的三维体(```3D_LowRes'```)

最终计算参数: 将得到融合后的序列图,放入```3DModelAnalysis```中计算参数.

## 环境准备
1. QT: 5.9.8,5.9是长期支持版本,bug较少.
2. VS2017 + QT插件
3. 在VS2017上进行开发,方便调试.

## 界面设计
Ribbon风格: Ribbon文件夹是一个Ribbon的demo.


