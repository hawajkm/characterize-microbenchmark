/* math.h
 *
 * Author: Khalid Al-Hawaj
 * Date  : 30 Dec. 2023
 *
 * This file contains optimized math functions.
*/

/* SIMD header file  */
#if defined(__amd64__) || defined(__x86_64__)
#include <immintrin.h>
#elif defined(__aarch__) || defined(__aarch64__) || defined(__arm64__)
#include <arm_neon.h>
#endif

#ifndef __COMMON_VMATH_H_
#define __COMMON_VMATH_H_

#if defined(__amd64__) || defined(__x86_64__)

/* ********************************************** *
 * Based on the SSE/SSE2 implementation of log_ps *
 * by\ Julien Pommier                             *
 * Website\ http://gruntthepeon.free.fr/ssemath/  *
 * ********************************************** */
__m256 _mm256_log_ps(__m256 x)
{
  /* log(x) = NAN, where x is less-than-or-equal to zero */
  __m256 invalid_mask = _mm256_cmp_ps(x, _mm256_setzero_ps(), _CMP_LE_OS);
  x = _mm256_max_ps(x, _mm256_castsi256_ps(_mm256_set1_epi32(0x00800000)));

  /* Get the exponent */
  __m256i imm0 = _mm256_srli_epi32(_mm256_castps_si256(x), 23);

  /* keep only the fractional part */
  x = _mm256_and_ps(x, _mm256_castsi256_ps(_mm256_set1_epi32(~0x7f800000)));
  x = _mm256_or_ps(x, _mm256_set1_ps(0.5f));

  imm0 = _mm256_sub_epi32(imm0, _mm256_set1_epi32(0x7f));
  __m256 e = _mm256_cvtepi32_ps(imm0);
  e = _mm256_add_ps(e, _mm256_set1_ps(1.0f));

  /* part2:
     if( x < SQRTHF ) {
       e -= 1;
       x = x + x - 1.0;
     } else { x = x - 1.0; }
  */
  __m256 mask = _mm256_cmp_ps(x, _mm256_set1_ps(0.707106781186547524), _CMP_LT_OS);
  __m256 tmp = _mm256_and_ps(x, mask);
  x = _mm256_sub_ps(x, _mm256_set1_ps(1.0f));
  e = _mm256_sub_ps(e, _mm256_and_ps(_mm256_set1_ps(1.0f), mask));
  x = _mm256_add_ps(x, tmp);

  __m256 z = _mm256_mul_ps(x, x);

  __m256 y = _mm256_set1_ps(7.0376836292e-2f);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, _mm256_set1_ps(-1.1514610310e-1f));
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, _mm256_set1_ps( 1.1676998740e-1f));
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, _mm256_set1_ps(-1.2420140846e-1f));
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, _mm256_set1_ps( 1.4249322787e-1f));
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, _mm256_set1_ps(-1.6668057665e-1f));
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, _mm256_set1_ps( 2.0000714765e-1f));
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, _mm256_set1_ps(-2.4999993993e-1f));
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, _mm256_set1_ps( 3.3333331174e-1f));
  y = _mm256_mul_ps(y, x);

  y = _mm256_mul_ps(y, z);

  tmp = _mm256_mul_ps(e, _mm256_set1_ps(-2.12194440e-4f));
  y = _mm256_add_ps(y, tmp);

  tmp = _mm256_mul_ps(z, _mm256_set1_ps(0.5f));
  y = _mm256_sub_ps(y, tmp);

  tmp = _mm256_mul_ps(e, _mm256_set1_ps(0.693359375f));
  x = _mm256_add_ps(x, y);
  x = _mm256_add_ps(x, tmp);
  x = _mm256_or_ps(x, invalid_mask);
  return x;
}

__m256 _mm256_approx_log_ps(__m256 x)
{
  /* Perform an approximation */
  /* log(x) = 2 * (sum(0, inf)((1 / 2n + 1) * ((x + 1) / (x - 1)) ^ 2n+1) */

  __m256 ret = _mm256_setzero_ps();

  /* Constants */
  __m256 one = _mm256_set1_ps(1.0f);
  __m256 two = _mm256_set1_ps(2.0f);
  __m256 rN  = _mm256_sub_ps(x, one);
  __m256 rD  = _mm256_add_ps(x, one);
  __m256 r   = _mm256_div_ps(rN, rD);
  __m256 r2  = _mm256_mul_ps(r, r);

  __m256 e   = r;

  for (int i = 0; i < 4; i++) {
    __m256 c  = _mm256_div_ps(two, _mm256_set1_ps((float)((2 * i) + 1)));
    ret = _mm256_add_ps(ret, _mm256_mul_ps(c, e));
    e = _mm256_mul_ps(e, r2);
  }

  return ret;
}

/* ********************************************** *
 * Based on the SSE/SSE2 implementation of exp_ps *
 * by\ Julien Pommier                             *
 * Website\ http://gruntthepeon.free.fr/ssemath/  *
 * ********************************************** */
__m256 _mm256_exp_ps(__m256 x)
{
  __m256 tmp = _mm256_setzero_ps(), fx;
  __m256i imm0;

  x = _mm256_min_ps(x, _mm256_set1_ps( 88.3762626647949f));
  x = _mm256_max_ps(x, _mm256_set1_ps(-88.3762626647949f));

  /* express exp(x) as exp(g + n*log(2)) */
  fx = _mm256_mul_ps(x, _mm256_set1_ps(1.44269504088896341f));
  fx = _mm256_add_ps(fx, _mm256_set1_ps(0.5f));

  /* if greater, substract 1 */
  tmp = _mm256_floor_ps(fx);
  __m256 mask = _mm256_cmp_ps(tmp, fx, _CMP_GT_OS);
  mask = _mm256_and_ps(mask, _mm256_set1_ps(1.0f));
  fx = _mm256_sub_ps(tmp, mask);

  tmp = _mm256_mul_ps(fx, _mm256_set1_ps(0.693359375f));
  __m256 z = _mm256_mul_ps(fx, _mm256_set1_ps(-2.12194440e-4f));
  x = _mm256_sub_ps(x, tmp);
  x = _mm256_sub_ps(x, z);

  z = _mm256_mul_ps(x,x);

  __m256 y = _mm256_set1_ps(1.9875691500e-4f);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, _mm256_set1_ps(1.3981999507e-3f));
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, _mm256_set1_ps(8.3334519073e-3f));
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, _mm256_set1_ps(4.1665795894e-2));
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, _mm256_set1_ps(1.6666665459e-1));
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, _mm256_set1_ps(5.0000001201e-1));
  y = _mm256_mul_ps(y, z);
  y = _mm256_add_ps(y, x);
  y = _mm256_add_ps(y, _mm256_set1_ps(1.0f));

  /* build 2^n */
  imm0 = _mm256_cvttps_epi32(fx);
  // another two AVX2 instructions
  imm0 = _mm256_add_epi32(imm0, _mm256_set1_epi32(0x7f));
  imm0 = _mm256_slli_epi32(imm0, 23);
  __m256 pow2n = _mm256_castsi256_ps(imm0);
  y = _mm256_mul_ps(y, pow2n);
  return y;
}

#elif defined(__aarch__) || defined(__aarch64__) || defined(__arm64__)

/* ********************************************** *
 * Based on the ARM NEON implementation of log_ps *
 * by\ Julien Pommier                             *
 * Website\ http://gruntthepeon.free.fr/ssemath/  *
 * ********************************************** */
float32x4_t vlog_f32(float32x4_t x)
{
  /* force flush to zero on denormal values */
  x = vmaxq_f32(x, vdupq_n_f32(0));
  uint32x4_t invalid_mask = vcleq_f32(x, vdupq_n_f32(0));

  int32x4_t ux = vreinterpretq_s32_f32(x);

  int32x4_t emm0 = vshrq_n_s32(ux, 23);

  /* keep only the fractional part */
  ux = vandq_s32(ux, vdupq_n_s32(~0x7f800000u));
  ux = vorrq_s32(ux, vreinterpretq_s32_f32(vdupq_n_f32(0.5f)));
  x = vreinterpretq_f32_s32(ux);

  emm0 = vsubq_s32(emm0, vdupq_n_s32(0x7f));
  float32x4_t e = vcvtq_f32_s32(emm0);

  e = vaddq_f32(e, vdupq_n_f32(1));

  /* part2:
     if( x < SQRTHF ) {
       e -= 1;
       x = x + x - 1.0;
     } else { x = x - 1.0; }
  */
  uint32x4_t mask = vcltq_f32(x, vdupq_n_f32(0.707106781186547524f));
  float32x4_t tmp = vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(x), mask));
  x = vsubq_f32(x, vdupq_n_f32(1));
  uint32x4_t ones_bin = vreinterpretq_u32_f32(vdupq_n_f32(1));
  e = vsubq_f32(e, vreinterpretq_f32_u32(vandq_u32(ones_bin, mask)));
  x = vaddq_f32(x, tmp);

  float32x4_t z = vmulq_f32(x,x);

  float32x4_t y = vdupq_n_f32(7.0376836292e-2f);
  y = vmulq_f32(y, x);
  y = vaddq_f32(y, vdupq_n_f32(-1.1514610310e-1f));
  y = vmulq_f32(y, x);
  y = vaddq_f32(y, vdupq_n_f32( 1.1676998740e-1f));
  y = vmulq_f32(y, x);
  y = vaddq_f32(y, vdupq_n_f32(-1.2420140846e-1f));
  y = vmulq_f32(y, x);
  y = vaddq_f32(y, vdupq_n_f32( 1.4249322787e-1f));
  y = vmulq_f32(y, x);
  y = vaddq_f32(y, vdupq_n_f32(-1.6668057665e-1f));
  y = vmulq_f32(y, x);
  y = vaddq_f32(y, vdupq_n_f32( 2.0000714765e-1f));
  y = vmulq_f32(y, x);
  y = vaddq_f32(y, vdupq_n_f32(-2.4999993993e-1f));
  y = vmulq_f32(y, x);
  y = vaddq_f32(y, vdupq_n_f32( 3.3333331174e-1f));
  y = vmulq_f32(y, x);

  y = vmulq_f32(y, z);

  tmp = vmulq_f32(e, vdupq_n_f32(-2.12194440e-4f));
  y = vaddq_f32(y, tmp);

  tmp = vmulq_f32(z, vdupq_n_f32(0.5f));
  y = vsubq_f32(y, tmp);

  tmp = vmulq_f32(e, vdupq_n_f32(0.693359375f));
  x = vaddq_f32(x, y);
  x = vaddq_f32(x, tmp);
  uint32x4_t nans = vreinterpretq_u32_f32(x);
  nans = vorrq_u32(nans, invalid_mask); /* Negative inputs will be NAN */
  x = vreinterpretq_f32_u32(nans);

  return x;
}

/* ********************************************** *
 * Based on the ARM NEON implementation of exp_ps *
 * by\ Julien Pommier                             *
 * Website\ http://gruntthepeon.free.fr/ssemath/  *
 * ********************************************** */
float32x4_t vexp_f32(float32x4_t x)
{
  float32x4_t tmp, fx;

  x = vminq_f32(x, vdupq_n_f32( 88.3762626647949f));
  x = vmaxq_f32(x, vdupq_n_f32(-88.3762626647949f));

  /* express exp(x) as exp(g + n*log(2)) */
  tmp = vdupq_n_f32(1.44269504088896341f);
  fx = vmlaq_f32(vdupq_n_f32(0.5f), x, tmp);

  /* perform a floorf */
  tmp = vcvtq_f32_s32(vcvtq_s32_f32(fx));

  /* if greater, substract 1 */
  uint32x4_t mask = vcgtq_f32(tmp, fx);
  uint32x4_t ones_bin = vreinterpretq_u32_f32(vdupq_n_f32(1.0f));
  mask = vandq_u32(mask, ones_bin);

  fx = vsubq_f32(tmp, vreinterpretq_f32_u32(mask));

  tmp = vmulq_f32(fx, vdupq_n_f32(0.693359375f));
  float32x4_t z = vmulq_f32(fx, vdupq_n_f32(-2.12194440e-4f));
  x = vsubq_f32(x, tmp);
  x = vsubq_f32(x, z);

  float32x4_t y = vmulq_f32(x, vdupq_n_f32(1.9875691500e-4f));
  z = vmulq_f32(x, x);
  y = vaddq_f32(y, vdupq_n_f32(1.3981999507e-3f));
  y = vmulq_f32(y, x);
  y = vaddq_f32(y, vdupq_n_f32(8.3334519073e-3f));
  y = vmulq_f32(y, x);
  y = vaddq_f32(y, vdupq_n_f32(4.1665795894e-2f));
  y = vmulq_f32(y, x);
  y = vaddq_f32(y, vdupq_n_f32(1.6666665459e-1f));
  y = vmulq_f32(y, x);
  y = vaddq_f32(y, vdupq_n_f32(5.0000001201e-1f));

  y = vmulq_f32(y, z);
  y = vaddq_f32(y, x);
  y = vaddq_f32(y, vdupq_n_f32(1.0f));

  /* build 2^n */
  int32x4_t mm;
  mm = vcvtq_s32_f32(fx);
  mm = vaddq_s32(mm, vdupq_n_s32(0x7f));
  mm = vshlq_n_s32(mm, 23);
  float32x4_t pow2n = vreinterpretq_f32_s32(mm);

  y = vmulq_f32(y, pow2n);
  return y;
}
#endif

#endif //__COMMON_VMATH_H_
