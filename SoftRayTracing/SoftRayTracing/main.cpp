#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <vector>
#include <omp.h>

#include "Shape.h"
#include "utils.h"

#define M_PI 3.1415926535

int width = 400;
int height = 400;
int spp = 128;
Eigen::Vector3f lightnormal = {0,-1,0};

struct Camera
{
	float focal_length = 1.0f;
	Eigen::Vector3f position = {0.0f,0.0f,0.0f};
	float fov = 90.0f;
	Camera(float f, Eigen::Vector3f pos, float v) :focal_length(f), position(pos), fov(v) {}
};
HitRes closest_hit(Ray& ray, std::vector<Shape*> scene)
{
	HitRes hitpoint;
	for (auto item : scene)
	{
		HitRes res = item->intersect(ray);
		if (res.isHit && res.distance < hitpoint.distance) hitpoint = res;
	}

	return hitpoint;
}


Eigen::Vector3f shade(HitRes & hit,Ray& ray, std::vector<Shape*> scene,int depth)
{
	//if (depth >= 5) return{ 0,0,0 };
	//命中光源 ray的radiance就是光源的albedo
	if (hit.m.isEmit) return hit.m.albedo;
	

	if (hit.m.mtype == material_type::diffuse)
	{
		//direct light sample from light
		Eigen::Vector3f pl = sampleLight();
		//std::cout <<"pl: "<<pl << std::endl;
		//hit.HitPos = { -1,1,0 };
		//std::cout << "hit: "<<hit.HitPos << std::endl;
		Ray direct;
		direct.position = hit.HitPos;
		direct.direction = (pl - hit.HitPos).normalized();
		//std::cout << "dir: "<<direct.direction << std::endl;
		//std::cout << "-dir:"<< -direct.direction << std::endl;
		//std::cout << (-direct.direction.dot(lightnormal)) << std::endl;
		HitRes middle = closest_hit(direct,scene);
		//if (middle.m.isEmit) std::cout << "hit " << middle.HitPos << std::endl;
		bool blocked = false;
		if ((middle.HitPos - pl).norm() < 1e-4) blocked = false;
		else blocked = true;
		//blocked = false;
		Eigen::Vector3f dir_l;
		if (blocked) dir_l = { 0,0,0 };
		else
		{
			float r2 = (hit.HitPos - pl).norm();
			float cos1 = std::max(0.0f,direct.direction.dot(hit.normal));
			float cos2 = std::max(0.0f, (-direct.direction).dot(lightnormal));
			//std::cout << r2 << " " << cos1 << " " << cos2 << " " << std::endl;
			Eigen::Vector3f color = { 1,1,1 };
			color = {color.x() * hit.m.albedo.x(),color.y() * hit.m.albedo.y(),color.z() * hit.m.albedo.z()};
			dir_l = color * cos1 * cos2 * (0.8 * 0.8) / M_PI / r2;
		}
		//std::cout << dir_l << std::endl;
		return dir_l;
		
		float p_rr = 0.8;
		float r = randf();
		if (r > p_rr) return dir_l;
		//indirect light
		Ray wi;
		wi.position = hit.HitPos;
		wi.direction = randomDirection(hit.normal);
		Eigen::Vector3f indir_l = { 0,0,0 };
		HitRes p = closest_hit(wi,scene);
		if (p.isHit == false) return dir_l;
		else
		{
			float cosine = std::max(0.0f, wi.direction.dot(hit.normal)); 
			indir_l = shade(p, wi, scene, depth + 1);
			indir_l = { indir_l.x() * hit.m.albedo.x(),indir_l.y() * hit.m.albedo.y(),indir_l.z() * hit.m.albedo.z() };
			indir_l = 2 *  indir_l * cosine / p_rr ;
		}

		return dir_l + dir_l;
		
	}
}

Eigen::Vector3f cast_ray(Ray & ray,std::vector<Shape*> scene)
{
	HitRes hit = closest_hit(ray, scene);
	if (!hit.isHit) return { 0,0,0 };
	//return { 0,0,0 };
	else
		return shade(hit, ray,scene,0);
}



int main()
{
	std::vector<Eigen::Vector3f> frame_buffer;

	frame_buffer.resize(width * height, { 255.0f,255.0f,255.0f });

	Camera cam(1.0f, { 0.0f,0.0f,2.2f }, 90.0f);

	//build the scene
	std::vector<Shape*> scene;
	//right
	Material m1(false, { 0,0,1 }, material_type::diffuse);
	Triangle t1({ 1,1.25,0.9 }, { 1,1.25,-0.9 }, { 1,-1.25,0.9 }, { -1,0,0 }, m1);
	Triangle t2({ 1,1.25,-0.9 }, { 1,-1.25,0.9 }, { 1,-1.25,-0.9 }, { -1,0,0 }, m1);
	scene.emplace_back(&t1);
	scene.emplace_back(&t2);

	//up
	Material m2(false, { 0.5,0.5,0.5 }, material_type::diffuse);
	Triangle t3({ -1,1.25,-0.9 }, { 1,1.25,-0.9 }, { 1,1.25,0.9 }, { 0,-1,0 }, m2);
	Triangle t4({ -1,1.25,-0.9 }, { 1,1.25,0.9 }, { -1,1.25,0.9 }, { 0,-1,0 }, m2);
	scene.emplace_back(&t3);
	scene.emplace_back(&t4);

	//left
	Material m3(false, { 1, 0, 0 }, material_type::diffuse);
	Triangle t5({ -1,1.25,0.9 }, { -1,1.25,-0.9 }, { -1,-1.25,0.9 }, { 1,0,0 }, m3);
	Triangle t6({ -1,-1.25,-0.9 }, { -1,1.25,-0.9 }, { -1,-1.25,0.9 }, { 1,0,0 }, m3);
	scene.emplace_back(&t5);
	scene.emplace_back(&t6);

	//down
	Material m4(false, { 0.5,0.5,0.5 }, material_type::diffuse);
	Triangle t7({ -1,-1.25,0.9 }, { -1,-1.25,-0.9 }, { 1,-1.25,0.9 }, { 0,1,0 }, m4);
	Triangle t8({ 1,-1.25,-0.9 }, { -1,-1.25,-0.9 }, { 1,-1.25,0.9 }, { 0,1,0 }, m4);
	scene.emplace_back(&t7);
	scene.emplace_back(&t8);

	//back
	Material m5(false, { 0.1,1,1 }, material_type::diffuse);
	Triangle t9({ -1,-1.25,-0.9 }, { -1,1.25,-0.9 }, { 1,-1.25,-0.9 }, { 0,0,1 }, m5);
	Triangle t10({ 1,1.25,-0.9 }, { -1,1.25,-0.9 }, { 1,-1.25,-0.9 }, { 0,0,1 }, m5);
	scene.emplace_back(&t9);
	scene.emplace_back(&t10);

	//light
	Material m_l(true, { 1,1,1 });
	Triangle t11({ 0.4, 1.24, 0.4 }, { -0.4, 1.24, -0.4 }, { -0.4, 1.24, 0.4 }, { 0,-1,0 }, m_l);
	Triangle t12({ 0.4, 1.24, 0.4 }, { 0.4, 1.24, -0.4 }, { -0.4, 1.24, -0.4 }, {0,-1,0},m_l);
	scene.emplace_back(&t11);
	scene.emplace_back(&t12);

	//ball
	Material m6(false, { 0.4,0.4,0.4 },material_type::diffuse);
	Sphere s1({ 0,0,0 }, 0.3, m6);
	scene.emplace_back(&s1);

	while (1)	
	{
		float y = 2.0f * cam.focal_length * tan(deg2rad(cam.fov / 2.0f));
		float x = static_cast<float>(width) / static_cast<float>(height) * y;
		float gamma = 2.2;
		#pragma omp parallel for num_threads(50)
		for (int i = width - 1; i >= 0; --i)
		{
			//std::cout <<i << std::endl;
			for (int j = 0; j < height; ++j)
			{
				//std::cout << i << " " << j << std::endl;
				float viewport_x = x / static_cast<float>(width) * j - x / 2.0f + x / static_cast<float>(width) * cam.position.x();
				float viewport_y = y / static_cast<float>(height) * i - y / 2.0f + y / static_cast<float>(height) * cam.position.y();
				//std::cout << viewport_x << " " << viewport_y << std::endl;
				Eigen::Vector3f view_coord(viewport_x, viewport_y, cam.position.z() - 1);
				Eigen::Vector3f direction = (view_coord - cam.position).normalized();
				//std::cout << direction << std::endl;
				Ray ray(cam.position, direction);

				Eigen::Vector3f color = { 0,0,0 };
				for (int k = 0; k < spp; ++k)
					color += cast_ray(ray, scene);
				color /= spp;

				frame_buffer[i * height + j].x() = pow(clamp(0.0f, 1.0f, color.x()), 1.0 / gamma) * 255.0f;
				frame_buffer[i * height + j].y() = pow(clamp(0.0f, 1.0f, color.y()), 1.0 / gamma) * 255.0f;
				frame_buffer[i * height + j].z() = pow(clamp(0.0f, 1.0f, color.z()), 1.0 / gamma) * 255.0f;
			}
		}
		
		cv::Mat img = cv::Mat::ones(height, width, CV_8UC3);

		write_Mat(img, frame_buffer);

		cv::imwrite("1.jpg", img);

		cv::imshow("SoftRayTracing", img);
		
		int k = cv::waitKey(10);
		
		//break;
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