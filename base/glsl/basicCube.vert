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
*/
#include "global.glsl"

layout(location = 0) in vec4 in_position; // : POSITION;
layout(location = 1) in vec4 in_texcoord; // : TEXCOORD0;
layout(location = 2) in vec4 in_normal; // : NORMAL;
layout(location = 3) in vec4 in_tangent; // : TANGENT;
layout(location = 4) in vec4 in_tangent_unk; // : TANGENT; // ??
layout(location = 5) in vec4 in_color; // : COLOR0;

out VS_OUT
{
    vec4 texcoord[1];
    vec4 color;
} result;

void main() {
    result.texcoord[0] = uTextureMatrix * in_texcoord;
    result.color = uProgramEnv[PP_COLOR_GLSL] * ((in_color * uProgramEnv[PP_COLOR_MODULATE]) + uProgramEnv[PP_COLOR_ADD]);
    gl_Position = uProjectionMatrix * uModelViewMatrix * in_position;
}
