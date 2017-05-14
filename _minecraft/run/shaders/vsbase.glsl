varying vec3 normal;
varying vec3 vertex;
varying vec4 color;
varying float alpha;

uniform vec3 pos;
uniform float elapsed;
uniform float underWater;
uniform mat4 invertView;
uniform mat4 view;

float taxicab (vec3 v1, vec3 v2) {
	return abs(v1.x-v2.x) + abs(v1.y-v2.y);
}

float hdistance (vec3 v1, vec3 v2) {
	v2.z = v1.z;
	return distance(v1, v2);
}

float vertexHeight (vec2 position) {
	return ( 
			  sin( position.x * 1.0  + position.y * 1.3  + elapsed * 3.0)
			+ sin( position.x * 0.2  + position.y * 1.9  + elapsed * 7.0 + 1.5)
			+ cos(-position.x * 0.5  - position.y * 2.7  - elapsed * 2.0 + 3.14)
			+ sin( position.x * 1.3  - position.y * 0.2  + elapsed * 6.0 + 4.5)
			+ cos(-position.x * 1.5  + position.y * 0.5  - elapsed * 4.0)
			+ sin( position.x * 1.9  - position.y * 0.4  + elapsed)
			+ cos( position.x * 0.1  + position.y * 0.4  - elapsed * 9.0)
			+ cos( position.x * 0.02 + position.y * 0.07 - elapsed * 1.3)
			) * (3.0 / 11.0);
}

void main () {
	vec4 worldCoord = invertView * gl_ModelViewMatrix * gl_Vertex;
	float dist = hdistance(vec3(worldCoord), pos);

 	if (gl_Color.b > 0.9 && gl_Color.r < 0.1) // water cube
 	{
 		float tmp = vertexHeight(worldCoord.xy);
		if (underWater < 0.5) {
			tmp += dist;
		}

		gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * (gl_Vertex + vec4(0, 0, -1 + tmp, 0));

 		alpha = (tmp + 3.0) / 4.0;
 	} else if (gl_Color.r > 0.9 && gl_Color.g > 0.9 && gl_Color.b < 0.1) {
 		gl_Position = gl_ProjectionMatrix * view * worldCoord;
 		alpha = 0.2;
 	} else {
		if (underWater > 0.5) {
			float zAdd = 1.0 - (-cos (-dist / 15.0 + elapsed * 2.0) + 1.0) / 2.0;
			worldCoord.z += zAdd * 10.0;
		} else {
			worldCoord.z += dist;
		}

 		gl_Position = gl_ProjectionMatrix * view * worldCoord;

		if (underWater < 0.5) {
 			dist = 1.0 - (-cos (dist / 10.0 + elapsed * 1.0) + 1.0) / 2.0;
			gl_Position.z += dist * (sin(elapsed) / 2.0 + 0.5);
		}

 		alpha = 1.0;
 	}

	gl_TexCoord[0] = gl_MultiTexCoord0;

	vertex = vec3(worldCoord);
	normal = gl_Normal;
}