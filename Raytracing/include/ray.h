#ifndef RAY_H
#define RAY_H

#include "vec3.h"
class ray {
public:
	ray() {};
	ray(const point3& org, const vec3& dir) :orig(org),direct(dir){};
	const point3& origin() const { return orig; };
	const vec3& direction() const { return direct; };
	point3 at(double t) const {
		return orig + t * direct;
	}
private:
	point3 orig;
	vec3 direct;
};
#endif