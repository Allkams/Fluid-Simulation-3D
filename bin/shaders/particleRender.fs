#version 430
in vec4 Color;
in vec4 Position;
out vec4 FragColor;

float particleRadius = 0.05;

vec2 res = vec2(1920, 1080);

void main()
{

   // vec2 center = vec2(0.0, 0.0); // Center of the circle in local space
   // float radius = 0.05; // Radius of the circle

   vec2 fragPosScreen = gl_FragCoord.xy / res;

   // // Transform the fragment's position to the local space of the billboard
   // vec2 fragPosLocal = fragPosScreen;

   // // Calculate the distance from the fragment to the center of the circle in local space
   // float distanceToCenter = length(fragPosLocal);

   // // If the distance is less than the radius, the fragment is inside the circle
   // if (distanceToCenter <= radius) {
   //    // Set the fragment color to whatever color you want for the circle
   //    // For example, let's set it to red
   //    FragColor = Color; // Red
   // } else {
   //    // Fragment is outside the circle, so discard it
   //    discard;
   // }

   float lengthofPos = length(fragPosScreen - Position.xy);
   if (lengthofPos > particleRadius)
      discard;

   FragColor = Color;
}