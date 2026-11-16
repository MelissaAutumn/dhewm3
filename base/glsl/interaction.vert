/*

====================================================

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

Shader converted from: base/renderprogs/interaction.vertex
https://github.com/id-Software/DOOM-3-BFG/blob/master/neo/renderer/RenderProgs_GLSL.cpp
*/
#include "global.glsl"
// VS_IN - R_GLSL_SetVertexAttribs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_texcoord;
layout(location = 2) in vec4 in_normal;
layout(location = 3) in vec4 in_tangent;
layout(location = 4) in vec4 in_tangent_unk;
layout(location = 5) in vec4 in_color;

out VS_OUT
{
    vec4 texcoord[7];
    vec4 color;
} result;

#define PP_LOCAL_LIGHT_ORIGIN 2
#define PP_LOCAL_VIEW_ORIGIN 3

void main() {
    vec3 vNormal = in_normal.xyz * 2.0 - 1.0;
    vec4 vTangent = in_tangent * 2.0 - 1.0;
    vec3 vBinormal = cross(vNormal.xyz, vTangent.xyz) * vTangent.w;

    vec4 defaultTexCoord = vec4(0.0f, 0.5f, 0.0f, 1.0f);

    //calculate vector to light in R0
    vec4 toLight = uProgramEnv[PP_LIGHT_ORIGIN] - in_position;

    //texcoord0
    result.texcoord[0].x = dot3(vTangent.xyz, toLight);
    result.texcoord[0].y = dot3(vBinormal, toLight);
    result.texcoord[0].z = dot3(vNormal, toLight);
    result.texcoord[0].w = 1.0f;

    //textures 1 takes the base coordinates by the texture matrix
    result.texcoord[1] = defaultTexCoord;
    result.texcoord[1].x = dot4(in_texcoord.xy, uProgramEnv[PP_BUMP_MATRIX_S]);
    result.texcoord[1].y = dot4(in_texcoord.xy, uProgramEnv[PP_BUMP_MATRIX_T]);

    //# texture 2 has one texgen
    result.texcoord[2] = defaultTexCoord;
    result.texcoord[2].x = dot4(in_position, uProgramEnv[PP_LIGHT_FALLOFF_S]);

    //# texture 3 has three texgens
    result.texcoord[3].x = dot4(in_position, uProgramEnv[PP_LIGHT_PROJECT_S]);
    result.texcoord[3].y = dot4(in_position, uProgramEnv[PP_LIGHT_PROJECT_T]);
    result.texcoord[3].z = 0.0f;
    result.texcoord[3].w = dot4(in_position, uProgramEnv[PP_LIGHT_PROJECT_Q]);

    //# textures 4 takes the base coordinates by the texture matrix
    result.texcoord[4] = defaultTexCoord;
    result.texcoord[4].x = dot4(in_texcoord.xy, uProgramEnv[PP_DIFFUSE_MATRIX_S]);
    result.texcoord[4].y = dot4(in_texcoord.xy, uProgramEnv[PP_DIFFUSE_MATRIX_T]);

    //# textures 5 takes the base coordinates by the texture matrix
    result.texcoord[5] = defaultTexCoord;
    result.texcoord[5].x = dot4(in_texcoord.xy, uProgramEnv[PP_SPECULAR_MATRIX_S]);
    result.texcoord[5].y = dot4(in_texcoord.xy, uProgramEnv[PP_SPECULAR_MATRIX_T]);

    //# texture 6's texcoords will be the halfangle in texture space

    //# calculate normalized vector to light in R0
    toLight = normalize(toLight);

    //# calculate normalized vector to viewer in R1
    vec4 toView = normalize(uProgramEnv[PP_VIEW_ORIGIN] - in_position);

    //# add together to become the half angle vector in object space (non-normalized)
    vec4 halfAngleVector = toLight + toView;

    //# put into texture space
    result.texcoord[6].x = dot3(vTangent.xyz, halfAngleVector);
    result.texcoord[6].y = dot3(vBinormal, halfAngleVector);
    result.texcoord[6].z = dot3(vNormal, halfAngleVector);
    result.texcoord[6].w = 1.0f;

    //# generate the vertex color, which can be 1.0, color, or 1.0 - color
    //# for 1.0 : env[16] = 0, env[17] = 1
    //# for color : env[16] = 1, env[17] = 0
    //# for 1.0-color : env[16] = -1, env[17] = 1
    result.color = (in_color * uProgramEnv[PP_COLOR_MODULATE]) + uProgramEnv[PP_COLOR_ADD];

    gl_Position = uProjectionMatrix * uModelViewMatrix * in_position;
}
