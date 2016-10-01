varying vec3 ec_vnormal, ec_vposition, ec_vtangent, ec_vbitangent;
uniform int renderMode;
uniform sampler2D wood;
uniform sampler2D bubble_color;
uniform sampler2D normalmap;
uniform sampler2D shadowmap;
varying vec4 shadow;

void main(){

	mat3 tform;
	vec3 P, N, L, V, H; 
	vec3 mapN, tcolor;
	vec4 ambient_color = gl_LightSource[0].ambient*gl_FrontMaterial.ambient;
	vec4 diffuse_color = gl_LightSource[0].diffuse*gl_FrontMaterial.diffuse; 
	vec4 specular_color = gl_LightSource[0].specular*gl_FrontMaterial.specular; 
	float shininess = gl_FrontMaterial.shininess;

	P = ec_vposition;
	if(renderMode==0){
		tcolor = vec3(texture2D(wood,gl_TexCoord[0].st));
		N = normalize(ec_vnormal);
	}else {
		tform = mat3(ec_vtangent,ec_vbitangent,ec_vnormal);
		mapN = vec3(texture2D(normalmap,gl_TexCoord[0].st));
		mapN.xy = 2.0*mapN.xy - vec2(1.0,1.0);
		N = normalize(tform*normalize(mapN));
		tcolor = vec3(texture2D(bubble_color,gl_TexCoord[0].st));
	}
	L = normalize(gl_LightSource[0].position - P);
	V = normalize(-P);
	H = normalize(L+V);
	
	
	diffuse_color *= vec4(tcolor,1.0);
	diffuse_color *= max(dot(N,L),0.0);
	diffuse_color *= vec4(tcolor, 1.0);

	specular_color *= ((shininess+2)/(8*3.1415926));
	specular_color *= pow(max(dot(H,N),0.0),shininess);
	specular_color *= vec4(tcolor, 1.0);
	

	vec4 scoord;
	float depth,cl;
	scoord=shadow/shadow.w;
	depth=texture2D(shadowmap,scoord.st).z;
	cl=1.0;
	if(depth<scoord.z) cl=0.5;

	gl_FragColor = ambient_color + diffuse_color + specular_color;

	gl_FragColor *= cl;
	
}