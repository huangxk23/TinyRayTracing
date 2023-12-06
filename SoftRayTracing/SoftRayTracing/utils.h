#pragma once
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
#include <random>
#include <math.h>
#include <cmath>

#define M_PI 3.1415926535

// 0-1 随机数生成
std::uniform_real_distribution<> dis(0.0, 1.0);
std::random_device rd;
std::mt19937 gen(rd());
double randf()
{
	return dis(gen);
}

//sample light
Eigen::Vector3f sampleLight()
{
	float r1 = randf(), r2 = randf();
	return { static_cast<float>(0.4 * r1 - 0.4 * (1 - r1)),1.24,static_cast<float>(0.4 * r2 - 0.4 * (1 - r2))};
}

// 单位球内的随机向量
Eigen::Vector3f randomVec3()
{

	Eigen::Vector3f d;
	do
	{
		d = 2.0f * Eigen::Vector3f(randf(), randf(), randf()) - Eigen::Vector3f(1, 1, 1);
	} while (d.dot(d) > 1.0);
	return d.normalized();
}

Eigen::Vector3f randomDirection(Eigen::Vector3f n)
{
	return (randomVec3() + n).normalized();
}

inline float deg2rad(const float& deg)
{
	return deg * M_PI / 180.0;
}

inline float clamp(const float& lo, const float& hi, const float& v)
{
	return std::max(lo, std::min(hi, v));
}

// Compute reflection direction
Eigen::Vector3f reflect(const Eigen::Vector3f& I, const Eigen::Vector3f& N)
{
	return -I + 2 * I.dot(N) * N;
}

// Compute refraction direction using Snell's law
//
// We need to handle with care the two possible situations:
//
//    - When the ray is inside the object
//
//    - When the ray is outside.
//
// If the ray is outside, you need to make cosi positive cosi = -N.I
//
// If the ray is inside, you need to invert the refractive indices and negate the normal N
Eigen::Vector3f refract(const Eigen::Vector3f& I, const Eigen::Vector3f& N, const float& ior)
{
	float cosi = clamp(-1, 1, I.dot(N));
	float etai = 1, etat = ior;
	Eigen::Vector3f n = N;
	if (cosi < 0) { cosi = -cosi; }
	else { std::swap(etai, etat); n = -N; }
	float eta = etai / etat;
	float k = 1 - eta * eta * (1 - cosi * cosi);
	Eigen::Vector3f v = { 0,0,0 };
	return k < 0 ? v : eta* I + (eta * cosi - sqrtf(k)) * n;
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
