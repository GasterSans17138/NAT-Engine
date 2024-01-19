#version 450

layout(location = 0) in VertexOutput
{
	vec3 outUVW;
} vOut;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPosition;

layout(binding = 1) uniform FragmentUniform
{
	double time;
} ubo;

layout(binding = 2) uniform samplerCube skySampler;
layout(binding = 3) uniform sampler2D normSampler;
layout(binding = 4) uniform sampler2D heightSampler;
//layout(binding = 3) uniform sampler2DShadow shadowMap;

// Adapted from this shader : 
// https://www.shadertoy.com/view/4tGczc
// Created by mathmasterzach

precision highp float;

#define M_PI        3.1415926535897932
#define M_1_4PI     0.0795774715459476 // 1 / (4pi)

#define LIGHT_YEAR 9.4607e+15 // One light year in meters.
#define STELLAR_RADIUS 6.957e+8 // Radius of the sun in meters.
#define SIGMA 5.670367e-8 // Stefan-Boltzmann constant.

struct Ray
{
    vec3 pos;
    vec3 dir;
};

/* Hash by David_Hoskins */
#define UI0 1597334673U
#define UI1 3812015801U
#define UI2 uvec2(UI0, UI1)
#define UI3 uvec3(UI0, UI1, 2798796415U)
#define UIF (1.0 / float(0xffffffffU))

vec3 hash33(vec3 p)
{
	uvec3 q = uvec3(ivec3(p)) * UI3;
	q = (q.x ^ q.y ^ q.z)*UI3;
	return vec3(q) * UIF;
}

/* Source: https://www.shadertoy.com/view/3dVXDc */
vec4 worley(vec3 uv)
{    
    vec3 id = floor(uv);
    vec3 p = fract(uv);
    vec4 v = vec4(10000, 0.0, 0.0, 0.0);

    for (float x = -1.; x <= 1.; ++x)
    {
        for(float y = -1.; y <= 1.; ++y)
        {
            for(float z = -1.; z <= 1.; ++z)
            {
              vec3 offset = vec3(x, y, z);
              vec3 h = hash33((id + offset));
              h += offset;
              vec3 d = p - h;

              vec3 col = hash33(offset+id);

              float s = smoothstep(-1.0, 1.0, (v.x-length(d))/0.2);
              v.yzw = mix(v.yzw, hash33((id + offset)), s);
              v.x = min(v.x, length(d));
            }
        }
    }
    return v;
}

vec3 star_color(float temp) {
  float pt = pow(temp,-1.5)*1e5,
        lt = log(temp);
  return clamp(vec3(
      561. * pt + 148.,
      temp > 6500. ? 352. * pt + 184. : 100.04 * lt - 623.6,
      194.18 * lt - 1448.6)/255., 0., 1.);
}

vec3 draw_stars(Ray ray) 
{
  vec3 L = vec3(0.0);
  
  for (float i = 1.0; i < 3.0; i++) {

      vec4 n = worley(ray.dir*250.*i);

      float t = mix(1000.0, 44000.0, n.y); // Absolute Temperature (K)
      float d = mix(1.0, 2.0, n.z) * LIGHT_YEAR;
      float r = mix(1.0, 10.0, n.w) * STELLAR_RADIUS;

      float radiant_flux = 4.0 * M_PI * SIGMA * pow(t, 4.0) * r*r; // W
      float radiant_intensity = radiant_flux * M_1_4PI / (d*d); // W * sr^-1

      L += vec3(exp(-n.x*40.)) * star_color(t) * radiant_intensity;
  }

  return L;
}

void main()
{
    Ray ray;
    ray.pos = vec3(0.0);
    ray.dir = normalize(vOut.outUVW);
    
    vec3 atmo = mix(vec3(.31,.32,.75)*0.0001, vec3(0.0), .7+0.46*ray.dir.y);
    vec3 col = (draw_stars(ray)*2.)+atmo;
    
    // Output to screen
    outColor = vec4(col,1.0);
	outNormal = vec4(0);
	outPosition = vec4(0);
}