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

/*
Thx learnopengl.com
*/

layout(location = 0) out vec4 color;

layout(binding = 0) uniform sampler2D samp; // texture 0 is the normalization cube map for the vector towards the light

in VS_OUT
{
    vec4 texcoord[1];
    float gamma;
    float applyTonemapping; // bool
    float exposure;
} fragment;

void main()
{
    vec2 texcoord = fragment.texcoord[0].xy;
    vec3 sceneColor = texture(samp, texcoord).rgb;

    // reinhard tonemapping
    if (fragment.applyTonemapping == 1.0) {
        vec3 result = vec3(1.0) - exp(-sceneColor * fragment.exposure);
        result = pow(result, vec3(1.0 / fragment.gamma));
        color = vec4(result, 1.0);
        return;
    }

    vec3 result = pow(sceneColor.rgb, vec3(1.0 / fragment.gamma));
    color = vec4(result, 1.0);
}
