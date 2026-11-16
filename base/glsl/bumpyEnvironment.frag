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

Shader converted from: base/renderprogs/bumpyenvironment.pixel
https://github.com/id-Software/DOOM-3-BFG/blob/master/neo/renderer/RenderProgs_GLSL.cpp
*/
#include "global.glsl"

layout(binding = 0) uniform samplerCube samp0; // texture 1 is the per-surface bump map
layout(binding = 1) uniform sampler2D samp1; // texture per-pixel reflection mapping with bump mapping

in VS_OUT
{
    vec4 texcoord[7];
    vec4 color;
} fragment;

layout(location = 0) out vec4 color;

void main()
{
    vec4 bump = texture(samp1, fragment.texcoord[0].xy) * 2.0f - 1.0f;
    vec3 localNormal = vec3(bump.wy, 0.0f);
    localNormal.z = sqrt(1.0f - dot3(localNormal, localNormal));

    vec3 globalNormal;
    globalNormal.x = dot3(localNormal, fragment.texcoord[2]);
    globalNormal.y = dot3(localNormal, fragment.texcoord[3]);
    globalNormal.z = dot3(localNormal, fragment.texcoord[4]);

    vec3 globalEye = normalize(fragment.texcoord[1].xyz);

    vec3 reflectionVector = globalNormal * dot3(globalEye, globalNormal);
    reflectionVector = (reflectionVector * 2.0f) - globalEye;

    vec4 envMap = texture(samp0, reflectionVector);

    color = vec4(envMap.xyz, 1.0f) * fragment.color;
}
