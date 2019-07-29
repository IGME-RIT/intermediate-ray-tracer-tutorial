Documentation Author: Niko Procopi 2019

This tutorial was designed for Visual Studio 2017 / 2019
If the solution does not compile, retarget the solution
to a different version of the Windows SDK. If you do not
have any version of the Windows SDK, it can be installed
from the Visual Studio Installer Tool

Welcome to the Shadow Ray Tracing Tutorial!
Prerequesites: Geometry Ray Tracing

In this tutorial, the only file we are changing is the
fragment shader. All lights will be hard-coded into the
shader, just like all the triangles.

For us to have lighting and shadows, we adjust the "trace"
function, and we a new function called "addLightColorToPixColor",
where Pix means pixel, of course. Don't try to shorten the name,
becuase we will be making more similar functions later.

This tutorial is significantly easier than the Shadow
Mapping tutorials that were in earlier graphics tutorials.

Just like the last tutorial, "trace" function will check
for collision between a ray, and all the polygons in the scene,
and if no triangles are hit, it returns vec4(0), which sets
the background color to black. 

In the last tutorial, trace(...) returned the color of the 
triangle that it hit. In this tutorial, if a ray hits a
triangle, we then have to do lighting calculations on the
triangle that the ray hit.

When we start the pixel shader, we cast a ray from the eye, 
into the scene. If the ray hits a triangle, then we cast 4 
rays from the triangle (one ray per light), each ray goes
from the point on the triangle that the eye's ray hit, to
each of the 4 lights. If these 4 rays successfully connect
the light to the triangle, then the triangle should be lit.
If these rays from the light to the triangle, have other
geometry colliding with them (which means if geometry is 
between the triangle that was hit by the eye's ray, and the light,
then that means the triangle that was hit by the eye's ray
is being blocked from the light, and then the color should be dark.
If there is nothing in between the triangle that the eye's ray hit,
and each of the lights, then add brightness to the pixel for each
light that successfully hits the triangle.

First, we create 4 lights at 4 positions, these will
all be white lights, I'll explain color later:
		vec3 light[4];
		light[0] = vec3(5.0, 3.0, 0.0);
		light[1] = vec3(-8.0, 5.0, 5.0);
		light[2] = vec3(5.0, 8.0, -5.0);
		light[3] = vec3(-5.0, 5.0, -5.0);
		
We make a float for light intensity, which is 6.0
in this simple example.

if the eye's ray hits a polygon, we set the pixel color 
to 10% of the triangle's color by default:
	vec3 pixColor = triangles[i.index].color * 0.1;
This is where we can set ambient occlusion. This is the 
color that the pixel will be if no lights are touching the
pixel. 0.1 can be changed to a vec3, to be a color, like 
vec3(0.3, 0.2, 0.1), to give the scene an orange glow.
If no lights touch this pixel, then this pixColor will be
the final color that is put on the screen.

Next, we loop through the four lights:
	for(int j = 0; j < 4; j++)
	
We call addLightColorToPixColor, which returns the amount of brightness
that the light gives to this pixel. If this light does not touch
the pixel (either due to other geometry blocking this pixel from 
the light, or due to the pixel being too far away), then
addLightColorToPixColor returns 0, and no color is added to this pixel 
from the light. If a light should add brightness to the pixel,
then we add to pixColor:
	pixColor += addLightColorToPixColor(light[j], dir, i, lightIntensity);
If you want lights to have color, here is how you do it. Let's say
your light is green, the vec3 RGB value of green is vec3(0, 1, 0),
so then you'd change this line to 
	pixColor += vec3(0, 1, 0) * addLightColorToPixColor(light[j], dir, i, lightIntensity);
This will make all your lights green, you can also make an array of colors, then 
you can do something like this:
	pixColor += lightCol[j] * addLightColorToPixColor(light[j], dir, i, lightIntensity);
To give each light a color. If you have no interest in changing the color
of the lights, then just leave it the way it is originally:
	pixColor += addLightColorToPixColor(light[j], dir, i, lightIntensity);
When we're done checking all the lights, we return the pixel color:
	return vec4(pixColor.rgb, 1.0);

The addLightColorToPixColor function is what checks to see if a light touches a pixel
lightPos is the position of the light,
pointToLight is the direction from the point where the eye's ray hit a triangle, to the light,
dir is the direction from the eye to the point that they eye's ray hit on the triangle.
eyeHitPoint gives the index in the triangle array that the eye's ray hit, letting us know which triangle 
	was hit by the eye's ray, and the point where the eye's ray hit the polygon
lightIntensity is how strong the light should be, if it hits the object directly, without any distance.

vec3 addLightColorToPixColor(vec3 lightPos, vec3 dir, hitinfo eyeHitPoint, float lightIntensity)

First, we get the direction from the point to the light
	vec3 pointToLight = lightPos - eyeHitPoint.point;

In this function, we make a hitInfo to see if any geometry stands between the light we are processing,
and the point at which the eye's ray hit a polygon. We have to do this for 4 lights, so that means
rays have to loop through all the geometry 5 times, it is clear how this can become expensive without
optimizations
	hitInfo render;
	
We use intersectTriangles to check all polygons in the scene, to see if any of them 
collide with the ray that shoots off the geometry that was hit by the eye's ray, in the 
direction of the light that is currrently being processed in the "for(j)" loop in the "trace" function

If this ray hits geometry, we check distance to see which is closer.
If the light    is closer to the point that was hit by the eye's ray, then the light shines on the pixel.
If the geometry is closer to the point that was hit by the eye's ray, then the light is blocked, 
	so return 0 and do not add brightness from this light to the pixel.
	
If nothing blocks the light from the point that was hit by the eye's ray, we need to calculate how much
brightness and should be applied to this pixel.

We normalize the ray from the point the eye hit, to the light
	vec3 normalPTL = pointToLight / dist;
	
we calculate the reflection vector, which will be used for calculating specular light,
	vec3 r = normalize((2 * dot(triangles[eyeHitPoint.index].normal, normalPTL) * triangles[eyeHitPoint.index].normal) - normalPTL);
	
This method is similar to previous tutorials with specular lighting in the "More Graphics" section. 
Please refer to the Specular Lighting tutorial in that section, if you would like more details.

With Diffuse, we use something similar to NdotL, to make it so the light only adds brightness 
to one side of the triangle (with the triangle's normal vector), this makes it so light does not 
go through triangles.
	max(0, dot(triangles[eyeHitPoint.index].normal, normalPTL)) ... (read next line)
	
Then we divide by distance ^ 2, which makes the light darker when the light is farther away,
this gives us a sense of attenuation. Close lights are bright, far lights are dark
	max(0, dot(triangles ... )) / pow(dist, 2);

With diffuse, specular, intensity of the light, and the color of the triangle, we
can finally return pixColor, which is not the color of the pixel, but the amount
that we are adding to the color of the pixel, from this particular light

How to improve:
Reflection (next tutorial). Reflect light and faces off of other faces.
