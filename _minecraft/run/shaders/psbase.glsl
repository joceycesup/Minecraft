varying float alpha;
varying vec3 normal;
varying vec3 vertex;

uniform vec3 pos;
uniform vec3 sunPos;
uniform sampler2D Texture0;
uniform float elapsed;
uniform float ambientLevel;
uniform float underWater;
uniform vec3 skyColor;
uniform float skyColorFactor;

float taxicab (vec3 v1, vec3 v2) {
	return abs(v1.x-v2.x) + abs(v1.y-v2.y);
}

float unsmoothstep (float x) {
	return x * (2.0 * x*x - 3.0 * x + 2.0);
}

void main (void) {
	vec4 color = texture2D(Texture0, vec2(gl_TexCoord[0]).st);

	float dist = 0.0;
	if (underWater > 0.5) {
		dist = 1.0 - (-cos (-taxicab(vertex, pos) / 15.0 + elapsed * 2.0) + 1.0) / 2.0;
		dist = unsmoothstep (unsmoothstep (unsmoothstep (dist)));
		color *= vec4 (dist / 4.0 + 0.75, dist * 0.9 + 0.1, dist * 0.9 + 0.1, 1.0);
	}
	dist = 1.0 - (-sin (distance(vertex, pos) / 35.0 + elapsed * 4.0) + 1.0) / 2.0;
	dist = unsmoothstep (unsmoothstep (dist));
	//color *= vec4 (dist / 1.6 + 0.375, dist / 4.0 + 0.75, dist / 1.6 + 0.375, 1.0);

	color.a = alpha;

	gl_FragColor = color;
	if (underWater > 0.5) {
		gl_FragColor *= vec4( 0.2, 0.4, 1.0, 1.0);
	}
	//else
	{//*
		gl_FragColor *= vec4(
			1.0 - skyColorFactor + skyColorFactor * skyColor.x,
			1.0 - skyColorFactor + skyColorFactor * skyColor.y,
			1.0 - skyColorFactor + skyColorFactor * skyColor.z,
		1.0);//*/
	}
}
