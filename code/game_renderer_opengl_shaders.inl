constexpr static const char* RendererOpenGLVertexShader = R"###(
#version 450 core
layout(location=0) in vec3 aModelVtx; 
layout(location=1) in vec4 aColor;
layout(location=2) in vec2 aTexCoord[4];
layout(location=6) in mat4 aTransform;
out vec4 mColor;
out vec2 mTexCoord;
uniform mat4 uProjection;

void main(void) {
	gl_Position = uProjection * aTransform *  vec4(aModelVtx, 1.0);
	mColor = aColor;
	mTexCoord = aTexCoord[gl_VertexID];
})###";

#if 1
constexpr static const char* RendererOpenGLVFragmentShader = R"###(
#version 450 core
out vec4 fragColor;
in vec4 mColor;
in vec2 mTexCoord;
uniform sampler2D uTexture;

void main(void) {
	fragColor = texture(uTexture, mTexCoord) * mColor; 
})###";

#else

constexpr static const char* fragmentShader = R"###(
#version 450 core
out vec4 fragColor;
in vec4 mColor;
in vec2 mTexCoord; 
uniform sampler2D 
uTexture;

void main(void) {
	fragColor.x = mTexCoord.x; 
	fragColor.y = mTexCoord.y; 
	fragColor.z = 0.0; 
	fragColor.w = 1.0; 
})###";

#endif
    