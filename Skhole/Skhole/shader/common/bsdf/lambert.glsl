
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


float GGX_D(vec3 wm,float ax,float ay){
	float term1 = wm.x * wm.x / (ax * ax) + wm.z * wm.z / (ay * ay) + wm.y * wm.y;
	float term2 = PI * ax * ay * term1 * term1;
	return 1.0f / term2;
}

float GGX_Lambda(vec3 w, float ax, float ay)
{
	float term = 1.0 + (ax * ax * w.x * w.x + ay * ay * w.z * w.z) / (w.y * w.y);
	return (-1.0 + sqrt(term)) * 0.5;
}

float GGX_G1(vec3 w, float ax, float ay){
	return 1.0 / (1.0 + GGX_Lambda(w, ax, ay));
}

float GGX_G2(vec3 wo, vec3 wi, float ax, float ay)
{
	return GGX_G1(wo,ax,ay) * GGX_G1(wi,ax,ay);
}

float GGX_G2_HeightCorrelate(vec3 wo, vec3 wi, float ax, float ay)
{
	return 1.0 / (1.0 + GGX_Lambda(wo,ax,ay) + GGX_Lambda(wi,ax,ay));
}

vec3 Shlick_Fresnel(vec3 F0, vec3 v, vec3 n)
{
	return F0 + (1.0 - F0) * pow(1.0 - dot(v, n), 5.0);
}

struct GGX_Params
{
	vec3 F0;
	float roughness;
	float anisotropic;
};


vec3 sampleVisibleNormal(vec2 uv, vec3 wo, vec2 alpha) {
	vec3 strech_wo = normalize(vec3(wo.x * alpha.x, wo.y, wo.z * alpha.y));

	float phi = 2.0 * PI * uv.x;
	float z = fma((1.0 - uv.y), (1.0 + strech_wo.y), -strech_wo.y);
	float sinTheta = sqrt(clamp(1.0f - z * z, 0.0f, 1.0f));
	float x = sinTheta * cos(phi);
	float y = sinTheta * sin(phi);

	vec3 c = vec3(x, z, y);

	vec3 h = c + strech_wo;

	vec3 wm = normalize(vec3(h.x * alpha.x, h.y, h.z * alpha.y));

	return wm;
}

vec2 RoughnessToAlpha(float roughness, float anistropic){
	float r2 = clamp(roughness * roughness,0.0001,1.0);
	float aspect = sqrt(1.0 - anistropic * 0.9);
	vec2 alpha;

	alpha.x = r2 / aspect;
	alpha.y = r2 * aspect;

	return alpha;
}

vec3 GGX_Sample(vec3 wo, vec2 xi,GGX_Params param, inout vec3 wi, inout float pdf){

	vec3 bsdf = vec3(0.0);

	vec2 alpha = RoughnessToAlpha(param.roughness, param.anisotropic);

	vec3 wm = sampleVisibleNormal(xi,wo,alpha);
	wi = reflect(-wo, wm);

	if(wi.y <= 0.0){
		pdf = 1.0;
		return bsdf;
	}

	float ggx_D = GGX_D(wm, alpha.x, alpha.y);
	float ggx_G = GGX_G2(wo, wi, alpha.x, alpha.y);
	vec3 ggx_F = Shlick_Fresnel(param.F0, wo, wm);

	float ggx_G1 = GGX_G1(wo, alpha.x, alpha.y);
	float jacobian = 0.25 / dot(wo, wm);

	pdf = ggx_D * ggx_G1 * dot(wo,wm) * jacobian / abs(wo.y);

	bsdf = ggx_D * ggx_G * ggx_F / (4.0 * wo.y * wi.y);
	return bsdf;
}

vec3 GGX_Evaluate(vec3 wo, vec3 wi, GGX_Params param)
{
	vec2 alpha = RoughnessToAlpha(param.roughness, param.anisotropic);

	vec3 wm = normalize(wo + wi);

	float ggx_D = GGX_D(wm, alpha.x, alpha.y);
	float ggx_G = GGX_G2(wo, wi, alpha.x, alpha.y);
	vec3 ggx_F = Shlick_Fresnel(param.F0, wo, wm);

	return  ggx_D * ggx_G * ggx_F / (4.0 * wo.y * wi.y);
}

