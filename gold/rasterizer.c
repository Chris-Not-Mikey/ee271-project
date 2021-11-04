#include "rasterizer.h"
#include <stdlib.h>

#ifdef __cplusplus
#include <vector>
#endif

/* Utility Functions */

/*
 *   Function: min
 *  Function Description: Returns the minimum value of two integers and b.
*/
int min(int a, int b)
{
  // START CODE HERE
  // END CODE HERE
  if (a <= b)
  {
    return a;
  }
  else
  {
    return b;
  }
}

/*
 *   Function: max
 *   Function Description: Returns the maximum value of two integers and b.
*/
int max(int a, int b)
{
  // START CODE HERE
  // END CODE HERE
  if (a >= b)
  {
    return a;
  }
  else
  {
    return b;
  }
}

/*
/   Function: floor_ss
/   Function Description: Returns a fixed point value rounded down to the subsample grid.
*/
int floor_ss(int val, int r_shift, int ss_w_lg2)
{
  // START CODE HERE
  // END CODE HERE
}

/*
 *  Function: rastBBox_bbox_fix
 *  Function Description: Determine a bounding box for the triangle.
 *  Note that this is a fixed point function.
*/
BoundingBox get_bounding_box(Triangle triangle, Screen screen, Config config)
{
  BoundingBox bbox;

  // START CODE HERE
  int r_shift = config.r_shift;
  int ss = config.ss;
  double ss_i = config.ss_i;
  int ss_w = config.ss_w;
  int ss_w_lg2 = config.ss_w_lg2;

  // initialize bounding box to first vertex

  ColorVertex3D v0 = triangle.v[0];
  ColorVertex3D v1 = triangle.v[0];
  ColorVertex3D v2 = triangle.v[0];

  int current_min_x = v0.x;
  int current_min_y = v0.y;
  int current_max_x = v0.x;
  int current_max_y = v0.y;

  // iterate over remaining vertices
  for (int i = 1; i < 3; i++)
  {
    current_min_x = min(current_min_x, triangle.v[i].x);
    current_min_y = min(current_min_y, triangle.v[i].y);
    current_max_x = max(current_max_x, triangle.v[i].x);
    current_max_y = max(current_max_y, triangle.v[i].y);
  }

  // round down to subsample grid

  int samples[1024];
  // int length = 1024/ss; ss_i
  int index = 0;
  // Fill in sampled indeces
  for (int j = -1 * ss_i; j < ss_i; j += ss) //Check this
  {
    samples[index] = j;
  }

  int min_x = current_min_x;
  int min_y = current_min_y;
  int max_x = current_max_x;
  int max_y = current_max_y;
  // From sampled indices, find find nearest to current vals
  for (int k = 0; k < 2 * ss_i; k += 1) //check this
  {

    int index = samples[k];
    min_x = (min_x > current_min_x - index) ? current_min_x - index : min_x;
    min_y = (min_y > current_min_y - index) ? current_min_y - index : min_y;
    max_x = (max_x > current_min_y - index) ? current_min_x - index : max_x;
    max_y = (min_x > current_min_y - index) ? current_min_x - index : max_y;
  }

  int index_x = current_min_x - min_x;
  int index_y = current_min_y - min_y;
  int index_max_x = current_max_x - max_x;
  int index_max_y = current_max_y - max_y;

  // clip to screen

  if (index_x < 0)
  {
    index_x = 0;
  }
  if (index_y < 0)
  {
    index_y = 0;
  }

  if (index_max_x >= 1024)
  {
    index_x = 1024;
  }
  if (index_max_y >= 1024)
  {
    index_y = 1024;
  }

  // check if bbox is valid
  Vertex2D lower_left;
  lower_left.x = index_x;
  lower_left.y = index_y;

  Vertex2D upper_right;
  upper_right.x = index_max_x;
  upper_right.y = index_max_y;

  bbox.lower_left = lower_left;
  bbox.upper_right = upper_right;

  bool valid = true;
  if (index_max_x < 0 && index_max_y < 0)
  {
    valid = false;
  }

  if (index_x >= 1024 && index_y > 1024)
  {
    valid = false;
  }

  // END CODE HERE
  return bbox;
}

/*
 *  Function: sample_test
 *  Function Description: Checks if sample lies inside triangle
 *
 *
 */
bool sample_test(Triangle triangle, Sample sample)
{
  bool isHit;

  // START CODE HERE
  // END CODE HERE

  return isHit;
}

int rasterize_triangle(Triangle triangle, ZBuff *z, Screen screen, Config config)
{
  int hit_count = 0;

  //Calculate BBox
  BoundingBox bbox = get_bounding_box(triangle, screen, config);

  //Iterate over samples and test if in triangle
  Sample sample;
  for (sample.x = bbox.lower_left.x; sample.x <= bbox.upper_right.x; sample.x += config.ss_i)
  {
    for (sample.y = bbox.lower_left.y; sample.y <= bbox.upper_right.y; sample.y += config.ss_i)
    {

      Sample jitter = jitter_sample(sample, config.ss_w_lg2);
      jitter.x = jitter.x << 2;
      jitter.y = jitter.y << 2;

      Sample jittered_sample;
      jittered_sample.x = sample.x + jitter.x;
      jittered_sample.y = sample.y + jitter.y;

      bool hit = sample_test(triangle, jittered_sample);

      if (hit)
      {
        hit_count++;
        if (z != NULL)
        {
          Sample hit_location;
          hit_location.x = sample.x >> config.r_shift;
          hit_location.y = sample.y >> config.r_shift;

          Sample subsample;
          subsample.x = (sample.x - (hit_location.x << config.r_shift)) / config.ss_i;
          subsample.y = (sample.y - (hit_location.y << config.r_shift)) / config.ss_i;

          Fragment f;
          f.z = triangle.v[0].z;
          f.R = triangle.v[0].R;
          f.G = triangle.v[0].G;
          f.B = triangle.v[0].B;

          process_fragment(z, hit_location, subsample, f);
        }
      }
    }
  }

  return hit_count;
}

void hash_40to8(uchar *arr40, ushort *val, int shift)
{
  uchar arr32[4];
  uchar arr16[2];
  uchar arr8;

  ushort mask = 0x00ff;
  mask = mask >> shift;

  arr32[0] = arr40[0] ^ arr40[1];
  arr32[1] = arr40[1] ^ arr40[2];
  arr32[2] = arr40[2] ^ arr40[3];
  arr32[3] = arr40[3] ^ arr40[4];

  arr16[0] = arr32[0] ^ arr32[2];
  arr16[1] = arr32[1] ^ arr32[3];

  arr8 = arr16[0] ^ arr16[1];

  mask = arr8 & mask;
  val[0] = mask;
}

Sample jitter_sample(const Sample sample, const int ss_w_lg2)
{
  long x = sample.x >> 4;
  long y = sample.y >> 4;
  uchar arr40_1[5];
  uchar arr40_2[5];

  long *arr40_1_ptr = (long *)arr40_1;
  long *arr40_2_ptr = (long *)arr40_2;

  ushort val_x[1];
  ushort val_y[1];

  *arr40_1_ptr = (y << 20) | x;
  *arr40_2_ptr = (x << 20) | y;

  hash_40to8(arr40_1, val_x, ss_w_lg2);
  hash_40to8(arr40_2, val_y, ss_w_lg2);

  Sample jitter;
  jitter.x = val_x[0];
  jitter.y = val_y[0];

  return jitter;
}

