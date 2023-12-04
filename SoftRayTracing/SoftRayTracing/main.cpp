#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <vector>

#include "Shape.h"

#define M_PI 3.1415926535

int width = 400;
int height = 400;
int spp = 1;

struct Camera
{
	float focal_length = 1.0f;
	Eigen::Vector3f position = {0.0f,0.0f,0.0f};
	float fov = 90.0f;
	Camera(float f, Eigen::Vector3f pos, float v) :focal_length(f), position(pos), fov(v) {}
};

inline float deg2rad(const float& deg)
{
	return deg * M_PI / 180.0;
}

inline float clamp(const float& lo, const float& hi, const float& v)
{
	return std::max(lo, std::min(hi, v));
}

//This function is used to convert the pixels' rgb value into opencv::Mat
void write_Mat(cv::Mat& img, std::vector<Eigen::Vector3f> frame_buf)
{
	int channels = img.channels(), rows = img.rows, cols = img.cols * channels;

	//the Mat may be stored in a one dimension formation
	//if (img.isContinuous())
	//{
	//	cols = rows * cols;
	//	rows = 1;
	//}

	uchar* p;
	int idx = 0;
	for (int i = rows - 1; i >= 0; --i)
	{
		p = img.ptr<uchar>(i);
		for (int j = 0; j < cols; j += 3, idx += 1)
		{
			//the value in Mat is not RGB but BGR
			p[j] = frame_buf[idx].z();
			p[j + 1] = frame_buf[idx].y();
			p[j + 2] = frame_buf[idx].x();
		}
	}
}

HitRes closest_hit(Ray& ray, std::vector<Shape*> scene)
{
	HitRes hitpoint;
	for (auto item : scene)
	{
		HitRes res = item -> intersect(ray);
		if (res.isHit && res.distance < hitpoint.distance) hitpoint = res;
	}

	return hitpoint;
}


int main()
{
	std::vector<Eigen::Vector3f> frame_buffer;
	
	frame_buffer.resize(width * height, {255.0f,255.0f,255.0f});
	
	Camera cam(1.0f, {0.0f,0.0f,2.05f},90.0f);

	//build the scene
	std::vector<Shape*> scene;
	//right
	Material m1(false,{0.5,0.5,1});
	Triangle t1({ 1,1,1 }, { 1,1,-1 }, { 1,-1,1 }, m1);
	Triangle t2({ 1,1,-1 }, { 1,-1,1 }, {1,-1,-1}, m1);
	scene.emplace_back(&t1);
	scene.emplace_back(&t2);

	//up
	Material m2(false, {0.5,0.5,0.5});
	Triangle t3({ -1,1,-1 }, { 1,1,-1 }, {1,1,1},m2);
	Triangle t4({ -1,1,-1 }, { 1,1,1 }, {-1,1,1},m2);
	scene.emplace_back(&t3);
	scene.emplace_back(&t4);

	//left
	Material m3(false, { 1, 0.5, 0.5 });
	Triangle t5({ -1,1,1 }, { -1,1,-1 }, {-1,-1,1},m3);
	Triangle t6({ -1,-1,-1 }, { -1,1,-1 }, {-1,-1,1},m3);
	scene.emplace_back(&t5);
	scene.emplace_back(&t6);
	
	//down
	Material m4(false, {0.5,0.5,0.5});
	Triangle t7({ -1,-1,1 }, { -1,-1,-1 }, {1,-1,1},m4);
	Triangle t8({ 1,-1,-1 }, {-1,-1,-1}, {1,-1,1},m4);
	scene.emplace_back(&t7);
	scene.emplace_back(&t8);

	//back
	Material m5(false, { 0.1,1,1 });
	Triangle t9({ -1,-1,-1 }, { -1,1,-1 }, {1,-1,-1},m5);
	Triangle t10({ 1,1,-1 }, { -1,1,-1 }, {1,-1,-1},m5);
	scene.emplace_back(&t9);
	scene.emplace_back(&t10);

	//ball
	Material m6(false, { 0.7,0.3,0.3 });
	Sphere s1({ 0,0,3 }, 0.5, m6);
	scene.emplace_back(&s1);



	while (1)
	{
		float y = 2.0f * cam.focal_length * tan(deg2rad(cam.fov / 2.0f));
		float x = static_cast<float>(width) / static_cast<float>(height) * y;
		for(int i = width - 1;i >= 0;--i)
			for (int j = 0; j < height; ++j)
			{
				float viewport_x = x / static_cast<float>(width) * j - x / 2.0f + x / static_cast<float>(width) * cam.position.x();
				float viewport_y = y / static_cast<float>(height) * i - y / 2.0f + y / static_cast<float>(height) * cam.position.y();
				//std::cout << viewport_x << " " << viewport_y << std::endl;
				Eigen::Vector3f view_coord(viewport_x,viewport_y,cam.position.z() - 1);
				Eigen::Vector3f direction = (view_coord - cam.position).normalized();
				//std::cout << direction << std::endl;
				Ray ray(cam.position,direction);

				Eigen::Vector3f color = { 0,0,0 };
				for (int k = 0; k < spp; ++k)
				{
					HitRes res = closest_hit(ray, scene);
					if (res.isHit) color += res.m.albedo;
				}
				color /= spp;
				
				frame_buffer[i * height + j].x() = clamp(0.0f, 1.0f, color.x()) * 255.0f;
				frame_buffer[i * height + j].y() = clamp(0.0f, 1.0f, color.y()) * 255.0f;
				frame_buffer[i * height + j].z() = clamp(0.0f, 1.0f, color.z()) * 255.0f;
			}
		
		cv::Mat img = cv::Mat::ones(height, width, CV_8UC3);

		write_Mat(img, frame_buffer);

		cv::imshow("SoftRayTracing", img);
		
		int k = cv::waitKey(10);
		
		//in soft ray tracing moving camera seems to be useless hhhh
		if (k == 'q') break;
		else if (k == 'w') cam.position.z() -= 1;
		else if (k == 's') cam.position.z() += 1;
		else if (k == 'a') cam.position.x() -= 1;
		else if (k == 'd') cam.position.x() += 1;
		else if (k == 'e') cam.position.y() -= 1;
		else if (k == 'q') cam.position.y() += 1;
		//std::cout << cam.position << std::endl;

	}
	return 0;
}