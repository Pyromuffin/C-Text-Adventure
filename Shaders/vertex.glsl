#version 400 core

// float x0,y0,s0,t0; // top-left
// float x1,y1,s1,t1; // bottom-right

layout(location = 0) in vec4 one;
layout(location = 1) in vec4 two;

uniform vec2 toClip; // coordinates are sent in screen space.

out vec4 frag_color;

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