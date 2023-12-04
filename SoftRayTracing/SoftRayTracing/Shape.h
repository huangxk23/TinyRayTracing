#pragma once
#include <eigen3/Eigen/Eigen>

struct HitRes
{
	bool isHit = false;
	Eigen::Vector3f HitPos;
	
};

struct Material
{
	bool isEmiss = false;
	Eigen::Vector3f albedo;
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
		HitRes res;
		return res;
	}

	~Triangle() {}
private:
	Eigen::Vector3f a, b, c;
	Material m;
};

class Sphere
{
public:
	Sphere(Eigen::Vector3f oo,float rr,Material mm):o(oo),r(rr),m(mm) {}
	~Sphere() {}

	HitRes intersect(Ray &ray)
	{
		HitRes res;
		return res;
	}

private:
	Eigen::Vector3f o;
	float r;
	Material m;
};
