!!permu FOG
#include "sys/fog.h"
varying vec2 tc;
varying vec2 lm;
varying vec4 vc;

#ifdef RTLIGHT
	varying vec3 lightvector;
//	#if defined(SPECULAR) || defined(OFFSETMAPPING)
//		varying vec3 eyevector;
//	#endif
	#if defined(PCF) || defined(CUBE) || defined(SPOT)
		varying vec4 vtexprojcoord;
	#endif
#endif





#ifdef VERTEX_SHADER

#ifdef RTLIGHT
	uniform vec3 l_lightposition;
//	#if defined(SPECULAR) || defined(OFFSETMAPPING)
//		uniform vec3 e_eyepos;
//	#endif
	#if defined(PCF) || defined(CUBE) || defined(SPOT)
		uniform mat4 l_cubematrix;
	#endif
	attribute vec3 v_normal;
	attribute vec3 v_svector;
	attribute vec3 v_tvector;
#endif

attribute vec2 v_texcoord;
attribute vec2 v_lmcoord;
attribute vec4 v_colour;

void main (void)
{
	tc = v_texcoord.st;
	lm = v_lmcoord.st;
	vc = v_colour;
	gl_Position = ftetransform();

	#ifdef RTLIGHT
		//light position is in model space, which is handy.
		vec3 lightminusvertex = l_lightposition - v_position.xyz;

		//no bumpmapping, so we can just use distance without regard for actual surface direction. we still do scalecos stuff. you might notice it on steep slopes.
		lightvector = lightminusvertex;
//		lightvector.x = dot(lightminusvertex, v_svector.xyz);
//		lightvector.y = dot(lightminusvertex, v_tvector.xyz);
//		lightvector.z = dot(lightminusvertex, v_normal.xyz);

//		#if defined(SPECULAR)||defined(OFFSETMAPPING)
//			vec3 eyeminusvertex = e_eyepos - v_position.xyz;
//			eyevector.x = dot(eyeminusvertex, v_svector.xyz);
//			eyevector.y = dot(eyeminusvertex, v_tvector.xyz);
//			eyevector.z = dot(eyeminusvertex, v_normal.xyz);
//		#endif
		#if defined(PCF) || defined(SPOT) || defined(CUBE)
			//for texture projections/shadowmapping on dlights
			vtexprojcoord = (l_cubematrix*vec4(v_position.xyz, 1.0));
		#endif
	#endif
}
#endif




#ifdef FRAGMENT_SHADER
//four texture passes
uniform sampler2D s_t0;
uniform sampler2D s_t1;
uniform sampler2D s_t2;
uniform sampler2D s_t3;

//mix values
uniform sampler2D s_t4;

#ifdef PCF
	uniform sampler2DShadow s_t5;
	#include "sys/pcf.h"
#endif
#ifdef CUBE
	uniform samplerCube s_t6;
#endif

//light levels
uniform vec4 e_lmscale;

#ifdef RTLIGHT
	uniform float l_lightradius;
	uniform vec3 l_lightcolour;
	uniform vec3 l_lightcolourscale;
#endif

void main (void)
{
	vec4 r;
	vec4 m = texture2D(s_t4, lm);

	r  = texture2D(s_t0, tc)*m.r;
	r += texture2D(s_t1, tc)*m.g;
	r += texture2D(s_t2, tc)*m.b;
	r += texture2D(s_t3, tc)*(1.0 - (m.r + m.g + m.b));

	//vertex colours provide a scaler that applies even through rtlights.
	r *= vc;

#ifdef RTLIGHT
	vec3 nl = normalize(lightvector);
	float colorscale = max(1.0 - (dot(lightvector, lightvector)/(l_lightradius*l_lightradius)), 0.0);
	vec3 diff;
//	#ifdef BUMP
//		colorscale *= (l_lightcolourscale.x + l_lightcolourscale.y * max(dot(bumps, nl), 0.0));
//	#else
		colorscale *= (l_lightcolourscale.x + l_lightcolourscale.y * max(dot(vec3(0.0, 0.0, 1.0), nl), 0.0));
//	#endif

//	#ifdef SPECULAR
//		vec3 halfdir = normalize(normalize(eyevector) + nl);
//		float spec = pow(max(dot(halfdir, bumps), 0.0), 32.0 * specs.a);
//		diff += l_lightcolourscale.z * spec * specs.rgb;
//	#endif



	#if defined(SPOT)
		if (vtexprojcoord.w < 0.0) discard;
		vec2 spot = ((vtexprojcoord.st)/vtexprojcoord.w);
		colorscale *= 1.0-(dot(spot,spot));
	#endif
	#ifdef PCF
		colorscale *= ShadowmapFilter(s_t5, vtexprojcoord);
	#endif

	r.rgb *= colorscale * l_lightcolour;

	#ifdef CUBE
		r.rgb *= textureCube(s_t6, vtexprojcoord.xyz).rgb;
	#endif

	gl_FragColor = fog4additive(r);
#else
	//lightmap is greyscale in m.a. probably we should just scale the texture mix, but precision errors when editing make me paranoid.
	r *= e_lmscale*vec4(m.aaa,1.0);
	gl_FragColor = fog4(r);
#endif
}
#endif