



 // layout (set=M, binding=N) uniform sampler2D variableNameArray[I];

 layout (std140, binding = 0) uniform bufferVals {
    mat4 mvp;
} myBufferVals;

vec2 transformToClip( vec2 vert )
{
	vec2 clip = vert / toClip;
	clip.y = 1 - clip.y;
	clip -= 0.5f;
	clip = clip * 2;

	return clip;
}


void main()
{


	gl_Position = vec4(clip, 1.0f);
	frag_color = color;
}