#version 420
#extension GL_ARB_explicit_uniform_location : require
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

// As declared in SetupUniforms
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uTextureMatrix;

uniform vec4 uProgramEnv[32];
uniform vec4 uProgramLocal[8];

uniform uint uCustomPostValue;

// !GENERALLY! ProgramEnv values
#define PP_LIGHT_ORIGIN 4
#define PP_VIEW_ORIGIN 5
#define PP_LIGHT_PROJECT_S 6
#define PP_LIGHT_PROJECT_T 7
#define PP_LIGHT_PROJECT_Q 8
#define PP_LIGHT_FALLOFF_S 9
#define PP_BUMP_MATRIX_S 10
#define PP_BUMP_MATRIX_T 11
#define PP_DIFFUSE_MATRIX_S 12
#define PP_DIFFUSE_MATRIX_T 13
#define PP_SPECULAR_MATRIX_S 14
#define PP_SPECULAR_MATRIX_T 15
#define PP_COLOR_MODULATE 16
#define PP_COLOR_ADD 17
#define PP_AMBIENT_LEVEL 18
// rpColor
#define PP_COLOR_GLSL 19
#define PP_LIGHT_FALLOFF_TQ 20
#define PP_GAMMA_BRIGHTNESS 21
#define PP_CURDEPTH_RECIPR 22
#define PP_PARTICLE_RADIUS 23
#define PP_PARTICLE_COLCHAN_MASK 24
#define PP_INV_VIEWPROJ_X 25
#define PP_INV_VIEWPROJ_Y 26
#define PP_INV_VIEWPROJ_Z 27
#define PP_INV_VIEWPROJ_W 28
#define PP_PREV_VIEWPROJ_X 29
#define PP_PREV_VIEWPROJ_Y 30
#define PP_PREV_VIEWPROJ_Z 31
#define PP_PREV_VIEWPROJ_W 32

/*
uniform float4 rpScreenCorrectionFactor; //:	register(c0);
uniform float4 rpWindowCoord; //:	register(c1);
uniform float4 rpDiffuseModifier; //:	register(c2);
uniform float4 rpSpecularModifier; //:	register(c3);

uniform float4 rpLocalLightOrigin; //:	register(c4);
uniform float4 rpLocalViewOrigin; //:	register(c5);

uniform float4 rpLightProjectionS; //:	register(c6);
uniform float4 rpLightProjectionT; //:	register(c7);
uniform float4 rpLightProjectionQ; //:	register(c8);
uniform float4 rpLightFalloffS; //:	register(c9);

uniform float4 rpBumpMatrixS; //:	register(c10);
uniform float4 rpBumpMatrixT; //:	register(c11);

uniform float4 rpDiffuseMatrixS; //:	register(c12);
uniform float4 rpDiffuseMatrixT; //:	register(c13);

uniform float4 rpSpecularMatrixS; //:	register(c14);
uniform float4 rpSpecularMatrixT; //:	register(c15);

uniform float4 rpVertexColorModulate; //:	register(c16);
uniform float4 rpVertexColorAdd; //:	register(c17);

uniform float4 rpColor; //:	register(c18);
uniform float4 rpViewOrigin; //:	register(c19);
uniform float4 rpGlobalEyePos; //:	register(c20);

uniform float4 rpMVPmatrixX; //:	register(c21);
uniform float4 rpMVPmatrixY; //:	register(c22);
uniform float4 rpMVPmatrixZ; //:	register(c23);
uniform float4 rpMVPmatrixW; //:	register(c24);

uniform float4 rpModelMatrixX; //:	register(c25);
uniform float4 rpModelMatrixY; //:	register(c26);
uniform float4 rpModelMatrixZ; //:	register(c27);
uniform float4 rpModelMatrixW; //:	register(c28);

uniform float4 rpProjectionMatrixX; //:	register(c29);
uniform float4 rpProjectionMatrixY; //:	register(c30);
uniform float4 rpProjectionMatrixZ; //:	register(c31);
uniform float4 rpProjectionMatrixW; //:	register(c32);

uniform float4 rpModelViewMatrixX; //:	register(c33);
uniform float4 rpModelViewMatrixY; //:	register(c34);
uniform float4 rpModelViewMatrixZ; //:	register(c35);
uniform float4 rpModelViewMatrixW; //:	register(c36);

uniform float4 rpTextureMatrixS; //:	register(c37);
uniform float4 rpTextureMatrixT; //:	register(c38);

uniform float4 rpTexGen0S; //:	register(c39);
uniform float4 rpTexGen0T; //:	register(c40);
uniform float4 rpTexGen0Q; //:	register(c41);
uniform float4 rpTexGen0Enabled; //:	register(c42);

uniform float4 rpTexGen1S; //:	register(c43);
uniform float4 rpTexGen1T; //:	register(c44);
uniform float4 rpTexGen1Q; //:	register(c45);
uniform float4 rpTexGen1Enabled; //:	register(c46);

uniform float4 rpWobbleSkyX; //:	register(c47);
uniform float4 rpWobbleSkyY; //:	register(c48);
uniform float4 rpWobbleSkyZ; //:	register(c49);

uniform float4 rpOverbright; //:	register(c50);
uniform float4 rpEnableSkinning; //:	register(c51);
uniform float4 rpAlphaTest; //:	register(c52);
*/

/* This nonsense needs to go into a global.inc */
float dot2(vec2 a, vec2 b) {
    return dot(a, b);
}
float dot3(vec3 a, vec3 b) {
    return dot(a, b);
}
float dot3(vec3 a, vec4 b) {
    return dot(a, b.xyz);
}
float dot3(vec4 a, vec3 b) {
    return dot(a.xyz, b);
}
float dot3(vec4 a, vec4 b) {
    return dot(a.xyz, b.xyz);
}
float dot4(vec4 a, vec4 b) {
    return dot(a, b);
}
float dot4(vec2 a, vec4 b) {
    return dot(vec4(a, 0, 1), b);
}

vec4 idtex2Dproj(sampler2D samp, vec4 texCoords) {
    return textureProj(samp, texCoords);
}

// ----------------------
// YCoCg Color Conversion
// ----------------------
const vec4 matrixRGB1toCoCg1YX = vec4(0.50, 0.0, -0.50, 0.50196078); // Co
const vec4 matrixRGB1toCoCg1YY = vec4(-0.25, 0.5, -0.25, 0.50196078); // Cg
const vec4 matrixRGB1toCoCg1YZ = vec4(0.0, 0.0, 0.0, 1.0); // 1.0
const vec4 matrixRGB1toCoCg1YW = vec4(0.25, 0.5, 0.25, 0.0); // Y

const vec4 matrixCoCg1YtoRGB1X = vec4(1.0, -1.0, 0.0, 1.0);
const vec4 matrixCoCg1YtoRGB1Y = vec4(0.0, 1.0, -0.50196078, 1.0); // -0.5 * 256.0 / 255.0
const vec4 matrixCoCg1YtoRGB1Z = vec4(-1.0, -1.0, 1.00392156, 1.0); // +1.0 * 256.0 / 255.0

vec3 ConvertYCoCgToRGB(vec4 YCoCg) {
    vec3 rgbColor;

    YCoCg.z = (YCoCg.z * 31.875) + 1.0; //z = z * 255.0/8.0 + 1.0
    YCoCg.z = 1.0 / YCoCg.z;
    YCoCg.xy *= YCoCg.z;
    rgbColor.x = dot4(YCoCg, matrixCoCg1YtoRGB1X);
    rgbColor.y = dot4(YCoCg, matrixCoCg1YtoRGB1Y);
    rgbColor.z = dot4(YCoCg, matrixCoCg1YtoRGB1Z);
    return rgbColor;
}
/* end of nonsense */
