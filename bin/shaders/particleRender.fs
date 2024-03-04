#version 430
in vec4 Color;
in vec4 fragWorldPos;
in vec4 posAndScale;
out vec4 FragColor;

layout(std430, binding = 0) readonly buffer Positions
{
    vec4 ReadPosAndScale[];
};


void main()
{
   if (length(fragWorldPos.xyz - posAndScale.xyz) > (posAndScale.w * 0.25f))
      discard;

   FragColor = Color;
}