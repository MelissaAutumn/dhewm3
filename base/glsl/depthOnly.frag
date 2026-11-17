/*
    *    ===========================================================================
    *
    *    Doom 3 BFG Edition GPL Source Code
    *    Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
    *
    *    This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").
    *
    *    Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
    *    it under the terms of the GNU General Public License as published by
    *    the Free Software Foundation, either version 3 of the License, or
    *    (at your option) any later version.
    *
    *    Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
    *    but WITHOUT ANY WARRANTY; without even the implied warranty of
    *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    *    GNU General Public License for more details.
    *
    *    You should have received a copy of the GNU General Public License
    *    along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.
    *
    *    In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.
    *
    *    If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
    *
    *    ===========================================================================
    */

#include "global.glsl"

layout(binding = 0) uniform sampler2D samp0;

in VS_OUT
{
    vec4 texcoord[1];
} fragment;

layout(location = 0) out vec4 colour;

// From RB_STD_DrawView
#define PP_MAIN_COLOR 0
#define PP_ALPHA_TEST 1

void main()
{
    // red channel contains the test
    if (uProgramEnv[PP_ALPHA_TEST].r <= 0.0f) {
        colour = uProgramEnv[PP_MAIN_COLOR];
        return;
    }

    // texture contains white image with alpha masking any transparencies
    vec4 tex = texture(samp0, fragment.texcoord[0].xy);
    // Alpha test contains a cutoff value, discard any values below it
    if (tex.a <= uProgramEnv[PP_ALPHA_TEST].r) {
        discard;
    }

    colour = uProgramEnv[PP_MAIN_COLOR] * tex;
}
