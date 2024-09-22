
#define PI 3.14159265359
vec3 HemisphereSampling(vec2 u, inout float pdf){
	float z = u.x;
	float r = sqrt(max(0.0, 1.0 - z*z));
    float phi = 2.0 * PI * u.y;
    pdf = 1.0 / (2.0 * PI);

    return vec3(r * cos(phi), z, r * sin(phi));
}

vec2 DiskConcentric(vec2 u){
    vec2 xi = 2.0 * u - vec2(1.0);
    float phi, r;

    if (xi.x == 0.0 && xi.y == 0.0) {
		return vec2(0.0);
    }

	if (abs(xi.x) > abs(xi.y)) {
		r = xi.x;
		phi = PI / 4.0 * (xi.y / xi.x);
	} else {
		r = xi.y;
		phi = PI / 2.0 - PI / 4.0 * (xi.x / xi.y);
	}

	return r * vec2(cos(phi), sin(phi));
}

vec3 CosineSampling(vec2 uv,inout float pdf){
		vec2 xi = DiskConcentric(uv);
		float z = sqrt(max(0.0, 1.0 - xi.x*xi.x - xi.y*xi.y));
		pdf = z / PI;
		return vec3(xi.x,z,xi.y);
}


vec3 LambertBSDF_Sample(vec3 wo, vec3 basecolor,vec2 xi,inout vec3 wi, inout float pdf){
//    wi = HemisphereSampling(xi,pdf);
    wi = CosineSampling(xi,pdf);
    vec3 bsdf = basecolor / PI;
    return bsdf;
}
