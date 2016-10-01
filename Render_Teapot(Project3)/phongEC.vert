varying vec3 ec_vnormal, ec_vposition;
varying vec3 ec_vtangent, ec_vbitangent;
attribute vec3 tangent, bitangent;
varying vec4 shadow;

void main(){	
	ec_vnormal = gl_NormalMatrix*gl_Normal;
	ec_vtangent = gl_NormalMatrix*tangent;
	ec_vbitangent = gl_NormalMatrix*bitangent;
	
	shadow=gl_TextureMatrix[4]*gl_Vertex;

	ec_vposition = gl_ModelViewMatrix*gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ProjectionMatrix*gl_ModelViewMatrix*gl_Vertex;
}