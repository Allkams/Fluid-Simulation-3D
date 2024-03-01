#version 430

uniform mat4 ViewProj;
uniform mat4 BillBoardViewProj;

uniform int ParticleOffset;

out vec4 Color;

layout(std430, binding = 0) readonly buffer Positions
{
    vec3 ReadPosAndScale[];
};

layout(std430, binding = 1) readonly buffer ReadBlockColor
{
    vec4 ReadColors[];
};

const vec3 TriBaseVerts[] = {
	vec3(0.5, 0.5, 0),
	vec3(-0.5, 0.5, 0),
	vec3(-0.5, -0.5, 0),
	vec3(0.5, 0.5, 0),
	vec3(-0.5, -0.5, 0),
	vec3(0.5, -0.5, 0)
}; 

void main()
{
    int localIndex = gl_VertexID % 6;
    int index1D = ParticleOffset + gl_VertexID / 6;
    vec4 translation = vec4(ReadPosAndScale[index1D].xyz , 1);
    float scale = 0.25;
    vec4 projectVertexPos = BillBoardViewProj * vec4(TriBaseVerts[localIndex] * scale, 0);
    gl_Position = ViewProj * translation + projectVertexPos;
    Color = ReadColors[index1D];
}