#version 400
in vec4 vPosition;
in vec4 vColor;
in vec3 vNormal;
out vec4 color;

uniform vec4 AmbientProduct1, DiffuseProduct1, SpecularProduct1;
uniform vec4 Light1Position; // Sun

uniform vec4 AmbientProduct2, DiffuseProduct2, SpecularProduct2;
uniform vec4 Light2Position; // Indoor

uniform float Shininess;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

in vec2 vTexCoord;
out vec2 texCoord;

void main()
{	
	texCoord = vTexCoord;
	// Transform vertex position into eye coordinates
	vec3 pos = (View * Model * vPosition).xyz;
	vec3 N = normalize( View * Model * vec4(vNormal, 0.0) ).xyz;
	vec3 E = normalize( -pos );

	// --- Light 1: Sun (Directional) ---
	vec3 L1;
	if(Light1Position.w == 0.0) L1 = normalize(Light1Position.xyz);
	else L1 = normalize(Light1Position.xyz - pos);
	
	vec3 H1 = normalize(L1 + E);
	
	vec4 ambient1 = AmbientProduct1;
	float Kd1 = max( dot(L1, N), 0.0 );
	vec4 diffuse1 = Kd1 * DiffuseProduct1;
	float Ks1 = pow( max(dot(N, H1), 0.0), Shininess );
	vec4 specular1 = Ks1 * SpecularProduct1;
	if( dot(L1, N) < 0.0 ) specular1 = vec4(0.0, 0.0, 0.0, 1.0);

	// --- Light 2: Indoor (Point with Attenuation) ---
	vec3 L2_vec = Light2Position.xyz - pos;
	float dist = length(L2_vec);
	vec3 L2 = normalize(L2_vec);
	
	vec3 H2 = normalize(L2 + E);
	
	// Attenuation
	float att = 1.0 / (1.0 + 0.1 * dist + 0.01 * dist * dist);
	
	vec4 ambient2 = AmbientProduct2;
	float Kd2 = max( dot(L2, N), 0.0 );
	vec4 diffuse2 = Kd2 * DiffuseProduct2;
	float Ks2 = pow( max(dot(N, H2), 0.0), Shininess );
	vec4 specular2 = Ks2 * SpecularProduct2;
	if( dot(L2, N) < 0.0 ) specular2 = vec4(0.0, 0.0, 0.0, 1.0);

	// Combine
	color = (ambient1 + diffuse1 + specular1) + (ambient2 + diffuse2 + specular2) * att;
	
	// Preserve alpha
	color.a = DiffuseProduct1.a; 

    gl_Position = Projection * View * Model * vPosition/vPosition.w;
}
