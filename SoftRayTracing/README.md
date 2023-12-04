### 一.Screen to Viewport

![img](img/1.jpg)

对于屏幕空间中的像素[i,j],需要映射到世界空间中的顶点才能投射出光线。

定义世界空间中的viewport:

```c++
struct camera
{
    float focal_length;
    vec3f campos;
    float fov;
};
```

上面camera的定义还是简单了一点，毕竟没有考虑方向或者上面默认定义的lookat就是看向-z, up 为+y，right为+x.

根据定义，viewport的x,y为：
$$
y = 2 \times focal\_length \times tan(\frac{fov}{2}) \\
x = \frac{width}{height} \times y
$$
对于屏幕空间中的[i,j]，通过平移 和scale两步可以转换为world space中的坐标，变换矩阵为：
$$
\begin{bmatrix}
\frac{x}{width} & 0 & 0\\
0 & \frac{y}{height} & 0\\
0 & 0 & 1\\
\end{bmatrix}
\times
\begin{bmatrix}
1 & 0 & -\frac{width}{2} + campos.x\\
0 & 1 & -\frac{height}{2} + campos.y\\
0 & 0 & 1\\
\end{bmatrix}
$$

###  二、求交

![img](img/3.PNG)



![img](img/4.PNG)

(图源：games101)

实现求交之后就可以实现直接投射光线的结果了，和光栅化结果相同：

![img](img/5.PNG)



### 三、Ray Tracing

光线追踪的关键：rendering equation
$$
L(p0,wo) = E(p0) + \int_\Omega BRDF(p0,wi,wo)L(p0,wi)cos\theta dw
$$
简单模拟情况下考虑三种材质：

1. diffuse matrial 
2. reflectance 
3. refractance



