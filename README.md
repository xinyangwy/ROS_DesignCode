# ROS_DesignCode
 ROS 机器人系统课程设计（自主导航+YOLO目标检测+语音播报）
+ 注：YOLO_weights网盘下载链接：https://pan.baidu.com/s/1wQHcdP5kQWm8qL_qxqhPdw?pwd=o7fp
提取码：o7fp --来自百度网盘超级会员V2的分享

+ 对应的CSDN博客地址：https://wuzilong.blog.csdn.net/article/details/125803475



----
# 注意事项：
实验的过程中遇到了许多的bug和问题，例如：
+ YOLO模块检测物体的时候检测框的左上角不出现文字（后来发现是缺少labels文件）；
+ gazebo打开的时候可能会崩溃（后来发现是虚拟机的问题，需要关闭vmware的3D图形加速选项）；
+ 关闭gazebo的时候尽量使用ctrl+C去关闭，直接关闭窗口可能不能完全关闭gazebo的全部进程（此时需要重启）；
+ 使用语音模块去订阅YOLO模块发送的消息的时候要处理好/darknet_ros/bounding boxes话题信息，在回调函数中提取其中的识别结果。
+ 安装ros可参考:
[Ubuntu18.04上安装ROS的详细教程](https://blog.csdn.net/qq_43313298/article/details/124187225)
[Ubuntu18.04安装Ros(最新最详细亲测)](https://blog.csdn.net/KIK9973/article/details/118755045)
+ 本文末尾有源码 和 虚拟机环境（可复制克隆）
#  一、总体功能设计
完成自主导航功能，并在小车移动至指定地点的过程中，通过机器人的摄像头检测导航途中所见物体，识别出物体的种类（框选出不同物体并标注识别结果），结合科大讯飞语音合成模块对识别结果进行语音播报。

> （注：目标检测，也叫目标提取，是一种基于目标几何和统计特征的图像分割。它将目标的分割和识别合二为一，其准确性和实时性是整个系统的一项重要能力。）

#  二、实验环境（文末提供虚拟机克隆）
Windows10
VMware虚拟机 ubuntu18.04 
Ros melodic
# 三、演示
演示地址：[https://www.bilibili.com/video/BV1tB4y1Y7UH](https://www.bilibili.com/video/BV1tB4y1Y7UH)

> 其他演示视频：链接：[https://pan.baidu.com/s/1nbP4XJRjF-rSvISvY1fkFA?pwd=nyrf](https://pan.baidu.com/s/1nbP4XJRjF-rSvISvY1fkFA?pwd=nyrf)
> 提取码：nyrf
> 视频内容基本相似，多了一点可以参考的视角

# 四、场景搭建、建图与导航模块
## 4.1 场景搭建
+ gazebo Building Editor
  ![image](https://github.com/user-attachments/assets/4a2477f4-79cd-4441-affd-c0d34acc4729)

+ 场景概览
![在这里插入图片描述](https://i-blog.csdnimg.cn/blog_migrate/278436a8b639b3b15f54c62e592bb5ce.png)
由于官方的模型库中没有我所使用的YOLO模型中能够检测到的特征鲜明的物体，如猫、狗、大象、马、苹果、香蕉等，所以部分物体（如图中的香蕉）放在相框中的图片中进行识别。
+ 设置的小车主要识别的区域
![在这里插入图片描述](https://i-blog.csdnimg.cn/blog_migrate/a6a9a1ee278050379e3038f485d961d0.png)
## 4.2 小车模型
本次课设使用的是TurtleBot3中的Waffle小车，下图中的中间一个（含有雷达与摄像头）
![image](https://github.com/user-attachments/assets/d623708e-08b7-4fc6-8aed-19a209692889)

小车模型在rviz中展示
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/a754c1867af2f5c7cb80393c036d4ab4.png)
## 4.3 导航模块
+ **注：对应github文件中的`test_ws`工作空间**
> gmapping 通过粒子滤波将激光距离数据转化为珊格地图。
> + 优点：在长廊及低特征场景中建图效果好；构建小场景地图所需的计算量较小且精度较高。缺点：依赖里程计，无法适用无人机及地面小车不平坦区域；无回环；大的场景，粒子较多的情况下，特别消耗资源。

主要参考：我在github上传的这个文件：[导航模块参考.pdf](https://github.com/xinyangwy/ROS_DesignCode/tree/main/%E5%AF%BC%E8%88%AA%E6%A8%A1%E5%9D%97%E5%8F%82%E8%80%83) 

### （1）安装依赖
在开始该部分设计之前，我们先安装相关支持的功能包，包括建图、导航以及机器人仿真的功能包（克隆下载后仅保留turtlebot3_gazebo文件夹），相关命令如下:

```
sudo apt-get update  
sudo apt-get install ros-melodic-turtlebot3-*  
sudo apt-get install ros-melodic-navigation  
sudo apt-get install ros-melodic-slam-gmapping  
git clone https://github.com/ROBOTIS-GIT/turtlebot3_simulations.git  
sudo apt-get install ros-melodic-dwa-local-planner  
```

+ 在使用仿真机器人之前先利用以下命令确定机器人类型，在 test_ws 目录下运行： 
```
echo “export TURTLEBOT3_MODEL=waffle” >>~/.bashrc
```
### （2）从github下载的文件中的test_ws目录下
先 catkin_make编译然后刷新工作空间，最后在运行 launch文件即可完成整体功能的启动，建图流程:
在test_ws目录下运行以下两条命令
```
roslaunch turtlebot3_gazebo turtlebot3_world.launch  
roslaunch my_navi build_map.launch  
```
运行完上述前两条命令之后会出现一个仿真的场景，然后rviz中会显示当前建图结果，然后运行下面的命令会出现键盘控制界面，

```
rosrun turtlebot3_teleop turtlebot3_teleop_key  
```
可以利用“w”，“a”,“s”，“d”来控制机器人移动，这四个按键分别控制向前移动、向左自旋、停止和向右自旋，合理的控制机器人移动来建立环境地图，结果如图3所示，最后利用下面的命令保存建图结果，
```
rosrun map_server map_saver -f map2  
```

产生图片名为“map2.pgm”和名为“map2.yaml”的地图参数，相关建图结果如图所示
+ 开始建图的时候
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/12d32387094e9a8a3380695045744bc9.png)
+ 建图完成
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/e468098258295c56a4056f077838f70b.png)
+ 控制小车建图的时候各个节点之间的通信情况
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/d3e287ea03416ac7e1539709394e7f04.png)
### （3）运行导航功能 
导航相关命令如下，在 test_ws 目录下执行命令之后会出现仿真场景和 rviz，点击 rviz 导 航栏的“2D Nav Goal”，然后点击地图上的任意空白地点之后机器人会自动导航至目标点， 并且 rviz 和仿真地图中机器人的动作在没有系统延时的时候会保持一致。

```
roslaunch turtlebot3_gazebo turtlebot3_world.launch  
roslaunch my_navi turtlebot_navigation.launch  
```
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/3c37ae061cbc30f191f7a6d550c89f85.png)
# 五、YOLO目标检测模块
**注：对应github文件中的yolo_ws工作空间**
主要参考：[小车yolo机械臂（一）ros下gazebo搭建小车（可键盘控制）安装摄像头仿真 加载yolo检测识别标记物体](https://blog.csdn.net/WhiffeYF/article/details/109187804)
## 5.1 YOLO介绍
YOLO（You Only Look Once） 是一种基于深度神经网络的对象识别和定位算法，其最大的特点是运行速度很快，可以用于实时系统。
## 5.2 本课设尝试使用过的YOLO模型
注：本课设使用的是yolov2_tiny.weights（能识别80种类别的物体）
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/f49f57c18be7dc973dc6c10b08f708c5.png)
> 注：YOLO_weights网盘下载链接：[https://pan.baidu.com/s/1wQHcdP5kQWm8qL_qxqhPdw?pwd=o7fp](https://pan.baidu.com/s/1wQHcdP5kQWm8qL_qxqhPdw?pwd=o7fp)
> 提取码：o7fp --来自百度网盘超级会员V2的分享
# 六、语音合成模块
+ **注：对应github文件中的`catkin_ws`工作空间**
+ 主要参考：
	+ [ROS实战( 三 )利用科大讯飞tts实现ROS下语音合成播报](https://blog.csdn.net/weixin_40522162/article/details/80525758)
	+ [ROS实战（二） 科大讯飞语音合成模块](https://blog.csdn.net/weixin_40522162/article/details/80489808)
可直接用文章（三）；注意第二个参考中的以下两个需要执行一些下：

```python
sudo apt-get install sox
为了不要每次运行都souce,把x64目录下的共享库文件拷贝到/usr/lib/下,先cd到voice/libs/x64目录下
cp libmsc.so /usr/lib/
```
# 七、小车摄像头与YOLO模块的通信
## 7.1 小车摄像头模块发布话题消息
找到导航模块中小车的摄像头对应的话题名称：
打开小车导航模块的时候在rviz中添加camera（可以用来查看小车摄像头的实时画面），然后选择话题名称：/camera/rgb/image_raw
（也可以在rqt_image_view中查看话题名称）
+ rviz中添加camera
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/5c8941695442b9965ae073088b9009b5.png)
+ 选择topic
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/6c7c3266e0c822e362f71e05f6aac1be.png)
+ rqt_image_view中查看摄像头话题名称
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/9e8b1112a16a02408bd97731280eb4c0.png)
## 7.2 YOLO模块模块订阅话题消息
在YOLO模块中订阅该话题：
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/d7071d16600466a9d4e2fe14078464de.png)
+ 通信过程
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/137fc8c75c715afdd5f2f743c49497f0.png)
# 八、YOLO模块与语音合成模块的通信
## 8.1 YOLO模块模块发布话题消息
YOLO检测模块中的ros.yaml文件中，发布话题/darknet_ros/bounding_boxes
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/f5d7772cd0dbaa6cdd08034b549dbf5e.png)
`rostopic echo /darknet_ros/bounding boxes`，展示发布到topic上的消息数据，Class是需要获取到的识别结果
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/77d18666c5e64ed328ae6669eaa2f3d9.png)
## 8.2 语音合成模块订阅话题消息
+ 语音合成模块节点
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/4287e1d4c6e9510b96f8f315067f8484.png)
+  在回调函数中获取话题/darknet_ros/bounding boxes消息中的Class内容：
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/5b846376cebe0ec3c64e58d14bf34d5e.png)
+ 通信过程
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/4028c57170ec549edfbe77956382102a.png)
# 九、演示时各模块的启动命令

```python
（在test_ws路径下）启动gazebo：roslaunch my_navi xinyang.launch
（在test_ws路径下）启动导航模块：roslaunch my_navi turtlebot_navigation.launch
（在yolo_ws路径下）启动YOLO模块：roslaunch darknet_ros darknet_ros.launch
（在catkin_ws路径下）启动语音合成模块：rosrun voice_system xf_tts_node
```
# 十、全局概览
![在这里插入图片描述](https://i-blog.csdnimg.cn/blog_migrate/f338d5696682b818a5e641d692b1b475.png)

+ 全局：各节点之间的通信情况
![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/1e0b002651040d5796ab6d2aa3944a85.png)
# 源码：
github源码地址：[https://github.com/xinyangwy/ROS_DesignCode](https://github.com/xinyangwy/ROS_DesignCode)
# 想要复制虚拟机环境看这里：
+ **遇到bug的朋友看这里**：虚拟机环境，想要复制的可以直接复制到自己的电脑上运行。网盘里面有OVF文件　和　整个的虚拟机环境【开机密码是123】
链接：[https://pan.baidu.com/s/15CAt5WjC3VvyTBE5eK2Dpw?pwd=wzla](https://pan.baidu.com/s/15CAt5WjC3VvyTBE5eK2Dpw?pwd=wzla) 
提取码：wzla
