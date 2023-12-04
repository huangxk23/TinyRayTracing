#pragma once
#include <eigen3/Eigen/Eigen>
#include <limits>
#include <cmath>

struct Material
{
	bool isEmit = false;
	Eigen::Vector3f albedo = { 0,0,0 };
	Material(bool emit, Eigen::Vector3f al) :isEmit(emit), albedo(al) {}
	Material() {}
};

struct HitRes
{
	bool isHit = false;
	Eigen::Vector3f HitPos;
	float distance = std::numeric_limits<float>::max();
	Material m;
	HitRes(){}
};

struct Ray
{
	Eigen::Vector3f position = { 0.0f,0.0f,0.0f };
	Eigen::Vector3f direction = { 0.0f,0.0f,0.0f };
	Ray(Eigen::Vector3f pos, Eigen::Vector3f dir) :position(pos), direction(dir) {}
};


class Shape
{
public:
	Shape() {}
	
	virtual HitRes intersect(Ray& ray) = 0;

	virtual ~Shape() {}
};

class Triangle :public Shape
{
public:

	Triangle(Eigen::Vector3f pa, Eigen::Vector3f pb, Eigen::Vector3f pc, Material pm) :a(pa), b(pb), c(pc), m(pm) {}
	
	HitRes intersect(Ray& ray)
	{
		//Moller Trumbore Algorithm
		HitRes res;
		Eigen::Vector3f E1 = b - a, E2 = c - a;
		Eigen::Vector3f S = ray.position - a;
		Eigen::Vector3f S1 = ray.direction.cross(E2), S2 = S.cross(E1);
		
		float tmp = S1.dot(E1);
		float t = S2.dot(E2) / tmp;
		float b1 = S1.dot(S) / tmp;
		float b2 = S2.dot(ray.direction) / tmp;

		if (t > 0 && b1 >= 0 && b2 >= 0 && 1 - b1 - b2 >= 0) res.isHit = true;
		else res.isHit = false;

		res.m = m;
		res.HitPos = (1 - b1 - b2) * a + b1 * b + b2 * c;
		res.distance = (res.HitPos - ray.position).norm();
		
		return res;
	}

	~Triangle() {}
private:
	Eigen::Vector3f a, b, c;
	Material m;
};

class Sphere:public Shape
{
public:
	Sphere(Eigen::Vector3f oo,float rr,Material mm):o(oo),r(rr),m(mm) {}
	~Sphere() {}

	HitRes intersect(Ray &ray)
	{
		
		HitRes res;

		float a = ray.direction.dot(ray.direction);
		float b = 2.0f * (ray.position - o).dot(ray.direction);
		float c = (ray.position - o).dot(ray.position - o) - r * r;
		float discriminant = b * b - 4.0f * a * c;
		
		if (discriminant < 0) res.isHit = false;
		else
		{
			float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
			if (t > 0)
			{
				res.isHit = true;
				res.HitPos = ray.position + t * ray.direction;
				res.distance = (ray.position - res.HitPos).norm();
				res.m = m;
			}
			else res.isHit = false;
		}
		return res;
	}

private:
	Eigen::Vector3f o;
	float r;
	Material m;
};
