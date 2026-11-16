/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================

Shader converted from: base/renderprogs/interaction.pixel
https://github.com/id-Software/DOOM-3-BFG/blob/master/neo/renderer/RenderProgs_GLSL.cpp
*/
#include "global.glsl"
layout(binding = 0) uniform samplerCube samp_cubemap; // texture 0 is the normalization cube map for the vector towards the light
layout(binding = 1) uniform sampler2D samp0; // texture 1 is the per-surface bump map
layout(binding = 2) uniform sampler2D samp1; // texture 2 is the light falloff texture
layout(binding = 3) uniform sampler2D samp2; // texture 3 is the light projection texture
layout(binding = 4) uniform sampler2D samp3; // texture 4 is the per-surface diffuse map
layout(binding = 5) uniform sampler2D samp4; // texture 5 is the per-surface specular map
layout(binding = 6) uniform sampler2D samp5_specular_lookup; // texture 6 is the specular lookup table

in VS_OUT
{
    vec4 texcoord[7];
    vec4 color;
} fragment;

layout(location = 0) out vec4 color;

// ProgramEnv strays
#define PP_DIFFUSE_COLOUR 0
#define PP_SPECULAR_COLOUR 1

void main()
{
    vec4 bumpMap = texture(samp0, fragment.texcoord[1].xy);
    vec4 lightFalloff = idtex2Dproj(samp1, fragment.texcoord[2]);
    vec4 lightProj = idtex2Dproj(samp2, fragment.texcoord[3]);
    vec3 diffuseMap = texture(samp3, fragment.texcoord[4].xy).xyz;
    vec4 specMap = texture(samp4, fragment.texcoord[5].xy);

    vec3 lightVector = normalize(fragment.texcoord[0].xyz);
    //vec3 diffuseMap = ConvertYCoCgToRGB(YCoCG);

    vec3 localNormal;
    localNormal.xy = bumpMap.wy - 0.5;
    localNormal.z = sqrt(abs(dot(localNormal.xy, localNormal.xy) - 0.25));
    localNormal = normalize(localNormal);

    const float specularPower = 10.0f;
    float hDotN = dot3(normalize(fragment.texcoord[6].xyz), localNormal);
    vec3 specularContribution = vec3(pow(hDotN, specularPower));

    vec3 diffuseColor = diffuseMap * uProgramEnv[PP_DIFFUSE_COLOUR].xyz;
    vec3 specularColor = specMap.xyz * specularContribution * uProgramEnv[PP_SPECULAR_COLOUR].xyz;
    vec3 lightColor = dot3(lightVector, localNormal) * lightProj.xyz * lightFalloff.xyz;

    color.rgb = (diffuseColor + specularColor) * lightColor * fragment.color.xyz;
    color.a = 1.0;
}
