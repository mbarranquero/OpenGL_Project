#version 450 core // Minimal GL version support expected from the GPU
#define M_PI 3.1415926535897932384626433832795

int mode = 1;
uniform float changingToon;
uniform float xtoon;
uniform float cluster;

struct LightSource {
	vec3 position;
	vec3 color;
	float intensity;
	float Ac;
	float Al;
	float Aq;
};

uniform LightSource lightSource;
uniform LightSource lightSourceB;
uniform LightSource lightSourceBi;

struct Material {
	vec3 albedo;
	sampler2D albedoTex;
	sampler2D roughnessTex;
	sampler2D metallicTex;
	sampler2D occlusionTex;
	sampler2D xtoonTex;
	float s;
	float ks;
	float alpha;
	float f0;
};

uniform Material material;

in vec3 fPosition; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
in vec3 fNormal;
in vec2 fTexCoord;

out vec4 colorResponse; // Shader output: the color response attached to this fragment


float d1(float alpha, vec3 wh, vec3 n){
	return alpha*alpha/(M_PI*(1+(alpha*alpha-1)*dot(n,wh)*dot(n,wh))*(1+(alpha*alpha-1)*dot(n,wh)*dot(n,wh)));
}

float f1(float f0, vec3 wi, vec3 wh){
	return f0 +(1-f0)*pow(1-max(0,dot(wi,wh)),5);
}

float g1(float alpha, vec3 w, vec3 n){
	return 2*dot(n,w)/(dot(n,w)+sqrt(alpha*alpha+(1-alpha*alpha)*dot(n,w)*dot(n,w)));
}

float depthF(float z, float zmin, float zmax){
		return 1-log(z/zmin)/log(zmax/zmin);
}

void main() {
	/*if (changingToon==1.f) {
		mode = 1;
	}*/
	if(xtoon == 1.f) {
		mode = 2;
	}
	if (cluster == 1.f){
		mode = 3;
	}

//// Premiere méthode
/*
	vec3 n = normalize (fNormal); // Linear barycentric interpolation does not preserve unit vectors
	vec3 wi = normalize (lightSource.position - fPosition);
	vec3 wiB = normalize (lightSourceB.position - fPosition);
	vec3 wiBi = normalize (lightSourceBi.position - fPosition);

	vec3 wo = normalize (-fPosition);

	vec3 wr = normalize(-wi + 2*(max(dot(wi,n), 0.0))*n);
	vec3 wrB = normalize(-wiB + 2*(max(dot(wiB,n),0.0))*n);
	vec3 wrBi = normalize(-wiBi + 2*(max(dot(wiBi,n), 0.0))*n);

	float fs = pow(material.ks*max(dot(wr, wo),0.0), material.s);
	float fsB = pow(material.ks*max(dot(wrB, wo),0.0), material.s);
	float fsBi = pow(material.ks*max(dot(wrBi, wo),0.0), material.s);

	float d = length(lightSource.position - fPosition);
	float dB = length(lightSourceB.position - fPosition);
	float dBi = length(lightSourceBi.position - fPosition);

	vec3 fr = material.albedo;

	vec3 Li = lightSource.color * lightSource.intensity ;
	vec3 LiB = lightSourceB.color * lightSourceB.intensity ;
	vec3 LiBi = lightSourceBi.color * lightSourceBi.intensity ;

	vec3 Libis = Li / (lightSource.Ac + lightSource.Al*d + lightSource.Aq*(d*d));
	vec3 LiBbis = LiB / (lightSourceB.Ac + lightSourceB.Al*d + lightSourceB.Aq*(d*d));
	vec3 LiBibis = LiBi / (lightSourceBi.Ac + lightSourceBi.Al*d + lightSourceBi.Aq*(d*d));

	float fd = 1.0 / 3.141592654;
	vec3 radianceL = Libis * fr * (fd + fs)*max(dot(n,wi),0.0);
	vec3 radianceLB = LiBbis * fr * (fd + fsB)*max(dot(n,wiB),0.0);
	vec3 radianceLBi = LiBibis * fr * (fd + fsBi)*max(dot(n,wiBi),0.0);
	vec3 radiance = radianceL + radianceLB + radianceLBi;

  colorResponse = vec4 (radiance, 1.0); // Building an RGBA value from an RGB one.*/

//// Deuxieme méthode
	vec3 n = normalize (fNormal); // Linear barycentric interpolation does not preserve unit vectors
	vec3 wi = normalize (lightSource.position - fPosition);
	vec3 wiB = normalize (lightSourceB.position - fPosition);
	vec3 wiBi = normalize (lightSourceBi.position - fPosition);

	vec3 wo = normalize (-fPosition);

	vec3 wr = normalize(-wi + 2*(dot(wi,n))*n);
	vec3 wrB = normalize(-wiB + 2*(dot(wiB,n))*n);
	vec3 wrBi = normalize(-wiBi + 2*(dot(wiBi,n))*n);

	float d = length(lightSource.position - fPosition);
	float dB = length(lightSourceB.position - fPosition);
	float dBi = length(lightSourceBi.position - fPosition);

	//vec3 fr = material.albedo;
	vec3 albedo = texture (material.albedoTex, fTexCoord).rgb;
	float rough = texture (material.roughnessTex, fTexCoord).r;
	float metallic = texture (material.metallicTex, fTexCoord).r;
	float occlusion = texture (material.occlusionTex, fTexCoord).r;
	vec3 fr = albedo;
	float f0 = metallic;
	float alpha = rough*rough;

	float F = f1(f0, wi, wr);
	float FB = f1(f0, wiB, wrB);
	float FBi = f1(f0, wiBi, wrBi);

	float D = d1(alpha, wr, n);
	float DB = d1(alpha, wrB, n);
	float DBi = d1(alpha, wrBi, n);

	float G = g1(alpha,wi,n)*g1(alpha,wo,n);
	float GB = g1(alpha,wiB,n)*g1(alpha,wo,n);
	float GBi = g1(alpha,wiBi,n)*g1(alpha,wo,n);

	float fs = D * F * G / (4 * dot(n, wi) * dot(n, wo) );
	float fsB = DB * FB * GB / (4 * dot(n, wiB) * dot(n, wo) );
	float fsBi = DBi * FBi * GBi / (4 * dot(n, wiBi) * dot(n, wo) );

	vec3 Li = lightSource.color * lightSource.intensity ;
	vec3 LiB = lightSourceB.color * lightSourceB.intensity ;
	vec3 LiBi = lightSourceBi.color * lightSourceBi.intensity ;

	vec3 Libis = Li / (lightSource.Ac + lightSource.Al*d + lightSource.Aq*(d*d));
	vec3 LiBbis = LiB / (lightSourceB.Ac + lightSourceB.Al*d + lightSourceB.Aq*(d*d));
	vec3 LiBibis = LiBi / (lightSourceBi.Ac + lightSourceBi.Al*d + lightSourceBi.Aq*(d*d));

	float fd = 1.0 / 3.141592654;

	vec3 radianceL = Libis * fr * (fd + fs)*max(dot(n,wi),0.0);
	vec3 radianceLB = LiBbis * fr * (fd + fsB)*max(dot(n,wiB),0.0);
	vec3 radianceLBi = LiBibis * fr * (fd + fsBi)*max(dot(n,wiBi),0.0);
	vec3 radiance = radianceL + radianceLB + radianceLBi;

///Toon shading
	/*if (mode == 1){
		vec3 longueurOnde = vec3 (0,0.7,1);
		if (length(radiance) > 0.4){
			radiance = vec3(1.0, 1.0, 1.0);
		}
		else if (length(radiance) < 0.4){
			radiance = longueurOnde*1;
		}
		if(dot(wo, n)<= 0.4){
			radiance = longueurOnde*0;
		}
	}*/

//XToon
	if (mode == 2){
		float zmin = 0.2;
		float zmax = 5;
		float z = length(fPosition);
		float depth = clamp(depthF(z, zmin, zmax), 0.0, 0.99);
		float axis = clamp(abs(dot(n, wi)), 0.01, 0.99);
		radiance = texture(material.xtoonTex, vec2(axis, depth)).rgb;
		colorResponse = vec4 (radiance, 1.0);
	}

	colorResponse = vec4 (radiance, 1.0) ; //* occlusion; // Building an RGBA value from an RGB one.
}


//color = vec4 (lightSource.color, 1.0);
