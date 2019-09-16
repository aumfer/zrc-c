#include <zmath.h>
#define HANDMADE_MATH_IMPLEMENTATION
#include <HandmadeMath.h>

hmm_mat4 hmm_inverse(hmm_mat4 m) {
	float Coef00 = m.Elements[2][2] * m.Elements[3][3] - m.Elements[3][2] * m.Elements[2][3];
	float Coef02 = m.Elements[1][2] * m.Elements[3][3] - m.Elements[3][2] * m.Elements[1][3];
	float Coef03 = m.Elements[1][2] * m.Elements[2][3] - m.Elements[2][2] * m.Elements[1][3];

	float Coef04 = m.Elements[2][1] * m.Elements[3][3] - m.Elements[3][1] * m.Elements[2][3];
	float Coef06 = m.Elements[1][1] * m.Elements[3][3] - m.Elements[3][1] * m.Elements[1][3];
	float Coef07 = m.Elements[1][1] * m.Elements[2][3] - m.Elements[2][1] * m.Elements[1][3];

	float Coef08 = m.Elements[2][1] * m.Elements[3][2] - m.Elements[3][1] * m.Elements[2][2];
	float Coef10 = m.Elements[1][1] * m.Elements[3][2] - m.Elements[3][1] * m.Elements[1][2];
	float Coef11 = m.Elements[1][1] * m.Elements[2][2] - m.Elements[2][1] * m.Elements[1][2];

	float Coef12 = m.Elements[2][0] * m.Elements[3][3] - m.Elements[3][0] * m.Elements[2][3];
	float Coef14 = m.Elements[1][0] * m.Elements[3][3] - m.Elements[3][0] * m.Elements[1][3];
	float Coef15 = m.Elements[1][0] * m.Elements[2][3] - m.Elements[2][0] * m.Elements[1][3];

	float Coef16 = m.Elements[2][0] * m.Elements[3][2] - m.Elements[3][0] * m.Elements[2][2];
	float Coef18 = m.Elements[1][0] * m.Elements[3][2] - m.Elements[3][0] * m.Elements[1][2];
	float Coef19 = m.Elements[1][0] * m.Elements[2][2] - m.Elements[2][0] * m.Elements[1][2];

	float Coef20 = m.Elements[2][0] * m.Elements[3][1] - m.Elements[3][0] * m.Elements[2][1];
	float Coef22 = m.Elements[1][0] * m.Elements[3][1] - m.Elements[3][0] * m.Elements[1][1];
	float Coef23 = m.Elements[1][0] * m.Elements[2][1] - m.Elements[2][0] * m.Elements[1][1];

	hmm_vec4 Fac0 = HMM_Vec4(Coef00, Coef00, Coef02, Coef03);
	hmm_vec4 Fac1 = HMM_Vec4(Coef04, Coef04, Coef06, Coef07);
	hmm_vec4 Fac2 = HMM_Vec4(Coef08, Coef08, Coef10, Coef11);
	hmm_vec4 Fac3 = HMM_Vec4(Coef12, Coef12, Coef14, Coef15);
	hmm_vec4 Fac4 = HMM_Vec4(Coef16, Coef16, Coef18, Coef19);
	hmm_vec4 Fac5 = HMM_Vec4(Coef20, Coef20, Coef22, Coef23);

	hmm_vec4 Vec0 = HMM_Vec4(m.Elements[1][0], m.Elements[0][0], m.Elements[0][0], m.Elements[0][0]);
	hmm_vec4 Vec1 = HMM_Vec4(m.Elements[1][1], m.Elements[0][1], m.Elements[0][1], m.Elements[0][1]);
	hmm_vec4 Vec2 = HMM_Vec4(m.Elements[1][2], m.Elements[0][2], m.Elements[0][2], m.Elements[0][2]);
	hmm_vec4 Vec3 = HMM_Vec4(m.Elements[1][3], m.Elements[0][3], m.Elements[0][3], m.Elements[0][3]);

	hmm_vec4 Inv0 = HMM_AddVec4(HMM_SubtractVec4(HMM_MultiplyVec4(Vec1, Fac0), HMM_MultiplyVec4(Vec2, Fac1)), HMM_MultiplyVec4(Vec3, Fac2));
	hmm_vec4 Inv1 = HMM_AddVec4(HMM_SubtractVec4(HMM_MultiplyVec4(Vec0, Fac0), HMM_MultiplyVec4(Vec2, Fac3)), HMM_MultiplyVec4(Vec3, Fac4));
	hmm_vec4 Inv2 = HMM_AddVec4(HMM_SubtractVec4(HMM_MultiplyVec4(Vec0, Fac1), HMM_MultiplyVec4(Vec1, Fac3)), HMM_MultiplyVec4(Vec3, Fac5));
	hmm_vec4 Inv3 = HMM_AddVec4(HMM_SubtractVec4(HMM_MultiplyVec4(Vec0, Fac2), HMM_MultiplyVec4(Vec1, Fac4)), HMM_MultiplyVec4(Vec2, Fac5));

	hmm_vec4 SignA = HMM_Vec4(+1, -1, +1, -1);
	hmm_vec4 SignB = HMM_Vec4(-1, +1, -1, +1);
	hmm_vec4 Col0 = HMM_MultiplyVec4(Inv0, SignA);
	hmm_vec4 Col1 = HMM_MultiplyVec4(Inv1, SignB);
	hmm_vec4 Col2 = HMM_MultiplyVec4(Inv2, SignA);
	hmm_vec4 Col3 = HMM_MultiplyVec4(Inv3, SignB);
	hmm_mat4 Inverse = {
		.Elements[0] = { Col0.X, Col0.Y, Col0.Z, Col0.W },
		.Elements[1] = { Col1.X, Col1.Y, Col1.Z, Col1.W },
		.Elements[2] = { Col2.X, Col2.Y, Col2.Z, Col2.W },
		.Elements[3] = { Col3.X, Col3.Y, Col3.Z, Col3.W },
	};

	hmm_vec4 Row0 = HMM_Vec4(Inverse.Elements[0][0], Inverse.Elements[1][0], Inverse.Elements[2][0], Inverse.Elements[3][0]);
	
	hmm_vec4 Dot0 = HMM_MultiplyVec4(HMM_Vec4(m.Elements[0][0], m.Elements[0][1], m.Elements[0][2], m.Elements[0][3]), Row0);
	float Dot1 = (Dot0.X + Dot0.Y) + (Dot0.Z + Dot0.W);

	float OneOverDeterminant = 1.0f / Dot1;

	return HMM_MultiplyMat4f(Inverse, OneOverDeterminant);
}

hmm_vec3 hmm_unproject(hmm_vec3 win, hmm_mat4 proj, float viewport[4]) {
	hmm_mat4 Inverse = hmm_inverse(proj);

	hmm_vec4 tmp = HMM_Vec4(win.X, win.Y, win.Z, 1);
	tmp.X = (tmp.X - (viewport[0])) / (viewport[2]);
	tmp.Y = (tmp.Y - (viewport[1])) / (viewport[3]);
	tmp.X = tmp.X * 2 - 1;
	tmp.Y = tmp.Y * 2 - 1;

	hmm_vec4 obj = HMM_MultiplyMat4ByVec4(Inverse, tmp);
	obj = HMM_DivideVec4f(obj, obj.W);
	
	return obj.XYZ;
}

float isect_plane(hmm_vec3 ro, hmm_vec3 rd, hmm_vec4 p) {
	float t = -(HMM_DotVec3(ro, p.XYZ) + p.W) / HMM_DotVec3(rd, p.XYZ);
	return t;
}

// https://github.com/ninjin/liblinear/blob/master/blas/ddot.c
float sdot(const int *n, const float *sx, const int *incx, const float *sy, const int *incy) {
	long int i, m, nn, iincx, iincy;
	float stemp;
	long int ix, iy;

	/* forms the dot product of two vectors.
	   uses unrolled loops for increments equal to one.
	   jack dongarra, linpack, 3/11/78.
	   modified 12/3/93, array(1) declarations changed to array(*) */

	   /* Dereference inputs */
	nn = *n;
	iincx = *incx;
	iincy = *incy;

	stemp = 0.0;
	if (nn > 0)
	{
		if (iincx == 1 && iincy == 1) /* code for both increments equal to 1 */
		{
			m = nn - 4;
			for (i = 0; i < m; i += 5)
				stemp += sx[i] * sy[i] + sx[i + 1] * sy[i + 1] + sx[i + 2] * sy[i + 2] +
				sx[i + 3] * sy[i + 3] + sx[i + 4] * sy[i + 4];

			for (; i < nn; i++)        /* clean-up loop */
				stemp += sx[i] * sy[i];
		}
		else /* code for unequal increments or equal increments not equal to 1 */
		{
			ix = 0;
			iy = 0;
			if (iincx < 0)
				ix = (1 - nn) * iincx;
			if (iincy < 0)
				iy = (1 - nn) * iincy;
			for (i = 0; i < nn; i++)
			{
				stemp += sx[ix] * sy[iy];
				ix += iincx;
				iy += iincy;
			}
		}
	}

	return stemp;
}