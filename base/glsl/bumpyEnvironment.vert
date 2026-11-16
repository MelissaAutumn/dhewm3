
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

Shader converted from: base/renderprogs/bumpyenvironment.vertex
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

// thx https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL
out VS_OUT
{
    vec4 texcoord[7];
    vec4 color;
} result;

#define PP_LOCAL_LIGHT_ORIGIN 2
#define PP_LOCAL_VIEW_ORIGIN 3

void main() {
    vec4 normal = in_normal * 2.0 - 1.0;
    vec4 tangent = in_tangent * 2.0 - 1.0;
    vec3 binormal = cross(normal.xyz, tangent.xyz) * tangent.w;

    gl_Position = uProjectionMatrix * uModelViewMatrix * in_position;

    result.texcoord[0] = in_texcoord;

    vec4 toEye = uProgramEnv[PP_VIEW_ORIGIN] - in_position;
    //result.texcoord1.x = dot3(toEye, rpModelMatrixX);
    //result.texcoord1.y = dot3(toEye, rpModelMatrixY);
    //result.texcoord1.z = dot3(toEye, rpModelMatrixZ);
    result.texcoord[1] = uModelViewMatrix * toEye;

    result.texcoord[2].x = dot3(tangent, uModelViewMatrix[0]);
    result.texcoord[3].x = dot3(tangent, uModelViewMatrix[1]);
    result.texcoord[4].x = dot3(tangent, uModelViewMatrix[2]);

    result.texcoord[2].y = dot3(binormal, uModelViewMatrix[0]);
    result.texcoord[3].y = dot3(binormal, uModelViewMatrix[1]);
    result.texcoord[4].y = dot3(binormal, uModelViewMatrix[2]);

    result.texcoord[2].z = dot3(normal, uModelViewMatrix[0]);
    result.texcoord[3].z = dot3(normal, uModelViewMatrix[1]);
    result.texcoord[4].z = dot3(normal, uModelViewMatrix[2]);

    result.color = uProgramEnv[PP_COLOR_GLSL];
}
