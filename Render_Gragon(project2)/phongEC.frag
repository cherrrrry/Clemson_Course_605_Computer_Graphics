varying vec3 ec_vnormal, ec_vposition;
uniform int renderMode;

void main(){
	vec3 P, N, L_key, L_fill, L_back, V;
	vec4 diffuse_color = gl_FrontMaterial.diffuse; 
	vec4 specular_color = gl_FrontMaterial.specular; 
	float shininess = gl_FrontMaterial.shininess;

	P = ec_vposition;
	N = normalize(ec_vnormal);
	
	L_key = normalize(gl_LightSource[0].position - P);
	L_fill = normalize(gl_LightSource[1].position - P);
	L_back = normalize(gl_LightSource[2].position - P);

	V = normalize(-P);				// eye position is (0,0,0)!
	

	vec4 diffuse_color_key = gl_FrontMaterial.diffuse; 
	vec4 diffuse_color_fill = gl_FrontMaterial.diffuse; 
	vec4 diffuse_color_back = gl_FrontMaterial.diffuse; 
	
	diffuse_color_key *= max(dot(N,L_key),0.0);
	diffuse_color_fill *= max(dot(N,L_fill),0.0);
	diffuse_color_back *= max(dot(N,L_back),0.0);
	
	vec4 specular_color_key = gl_FrontMaterial.specular; 
	vec4 specular_color_fill = gl_FrontMaterial.specular; 
	vec4 specular_color_back = gl_FrontMaterial.specular; 
	
	if(!renderMode){
		vec3 H_key, H_fill, H_back;
		H_key = normalize(L_key+V);
		H_fill = normalize(L_fill+V);
		H_back = normalize(L_back+V);
		
		specular_color_key *= ((shininess+2)/(8*3.1415926))*pow(max(dot(H_key,N),0.0),shininess);
		specular_color_fill *= ((shininess+2)/(8*3.1415926))*pow(max(dot(H_fill,N),0.0),shininess);
		specular_color_back *= ((shininess+2)/(8*3.1415926))*pow(max(dot(H_back,N),0.0),shininess);
	}else{

		vec3 R_key, R_fill, R_back;
		R_key = 2*dot(L_key,N)*N-L_key;
		R_fill = 2*dot(L_fill,N)*N-L_fill;
		R_back = 2*dot(L_back,N)*N-L_back;

		specular_color_key *= pow(max(dot(R_key,V),0.0),shininess);
		specular_color_fill *= pow(max(dot(R_fill,V),0.0),shininess);
		specular_color_back *= pow(max(dot(R_back,V),0.0),shininess);
	}

	/*
	specular_color_key *= pow(max(dot(H_key,N),0.0),shininess);
	specular_color_fill *= pow(max(dot(H_fill,N),0.0),shininess);
	specular_color_back *= pow(max(dot(H_back,N),0.0),shininess);
	*/

	gl_FragColor = 0.5*(diffuse_color_key + specular_color_key)+
			0.3*(diffuse_color_fill + specular_color_fill)+
			0.2*(diffuse_color_back + specular_color_back);
}