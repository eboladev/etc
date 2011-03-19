/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Wideangle plug-in - adjust for lens distortion
 * Copyright (C) 2001-2005 David Hodson hodsond@acm.org
 * Many thanks for Lars Clausen for the original inspiration,
 *   useful discussion, optimisation and improvements.
 * Framework borrowed from many similar plugins...
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/******************************************

to do:
  speedup?
  jump to correct preset on load (look for matching parameters)
    -- avoided; preset menu is now input only, not displayed
  internationalisation
  use Gimp bg colour for fill (also "correct" antialiasing)

******************************************/

/*
 *
 * version 1.0.10 - fix some compiler warnings (hopefully) on gcc 4.0.1
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <gtk/gtk.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

static void query (void);
static void run(const gchar*, gint, const GimpParam*, gint*, GimpParam**);

static void wideangleSetupContext(GimpDrawable *drawable);
static void wideangleSetupCalc();
static void wideangle();
static void calcWideangleRect(int x, int y, int step, int w, int h, int d, guchar* dst);
static void wideangleDialog();
static void readPresetFile();
static void writePresetFile();

#define PLUGIN_NAME "plug_in_wideangle"

GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

typedef struct
{
  gdouble centre_x;
  gdouble centre_y;
  gdouble square_a;
  gdouble quad_a;
  gdouble scale_a;
  gdouble brighten;
} WideangleValues;

static WideangleValues vals = { 0.0, 0.0, 0.0, 0.0 };

typedef struct
{
  WideangleValues* val;
  char name[101];
} WideanglePreset;

typedef struct {
  GimpDrawable* drawable;
  gint32 id;
  int width;
  int height;
  int depth;
  int do_it;
} WideangleContextValues;

static WideangleContextValues contextVals;

typedef struct {
  double normallise_radius_sq;
  double centre_x;
  double centre_y;
  double mult_sq;
  double mult_qd;
  double rescale;
  double brighten;
} WideangleCalcValues;

static WideangleCalcValues calcVals;

#define MAX_PIXEL_DEPTH 4
#define PREVIEW_ROUGH_AREA (200 * 200)

typedef struct {
  /* preview consists of 25 regions, each width * height (* depth),
   * arranged in a 5 * 5 grid. Buffer can hold one such region.
   * The region at [i,j] in the grid comes from the image starting at
   * [xStart + i * xSpacing, [yStart + j * ySpacing], and stepping
   * by step in both x and y. Note that the preview is calculated
   * directly from the image, not from a sampled version.
   */
  int width;
  int height;
  int depth;
  guchar* buffer;
  /* image access */
  double xStart;
  double xSpacing;
  double yStart;
  double ySpacing;
  double step;
  double maxStep;
  double maxXSpacing;
  double maxYSpacing;
  /* widget */
  GtkWidget* preview;
  /* idle updating */
  int enabled;
  int progress;
} WideanglePreview;

static WideanglePreview* previewCreate();
static void previewStartUpdate(WideanglePreview*);
static gint previewIdleUpdateCB(gpointer);

typedef struct
{
  GtkWidget* saveButton;
  GtkWidget* presetMenu;
  GtkWidget* selectButton;
  GtkWidget* previewBox;
  GtkAdjustment* centre_x;
  GtkAdjustment* centre_y;
  GtkAdjustment* square_a;
  GtkAdjustment* quad_a;
  GtkAdjustment* scale_a;
  GtkAdjustment* bright_a;
  WideanglePreview* preview;
  int do_preview;
  GSList* presets;
} WideangleGuiValues;

static WideangleGuiValues guiVals;


MAIN ()

static void
query (void)
{
  static GimpParamDef args[] =
  {
    { GIMP_PDB_INT32, "run_mode", "Interactive, non-interactive" },
    { GIMP_PDB_IMAGE, "image", "Input image (unused)" },
    { GIMP_PDB_DRAWABLE, "drawable", "Input drawable" },
    { GIMP_PDB_FLOAT, "offset x", "Effect centre offset in X" },
    { GIMP_PDB_FLOAT, "offset y", "Effect centre offset in Y" },
    { GIMP_PDB_FLOAT, "main adjust", "Amount of second-order distortion" },
    { GIMP_PDB_FLOAT, "edge adjust", "Amount of fourth-order distortion" },
    { GIMP_PDB_FLOAT, "rescale", "Rescale overall image size" },
    { GIMP_PDB_FLOAT, "brighten", "Adjust brightness in corners" }
  };
  static gint nargs = sizeof(args) / sizeof(args[0]);

  gimp_install_procedure(PLUGIN_NAME,
    "Corrects wideangle lens distortion",
    "Corrects (or creates) the distortion common to wideangle lenses.",
    "David Hodson <hodsond@acm.org>",
    "Copyright (C) 2001-2005 David Hodson",
    "Version 1.0.10 20 Dec 2005",
    "<Image>/Filters/Distorts/Wideangle...",
    "RGB*, GRAY*", GIMP_PLUGIN, nargs, 0, args, NULL);
}

static void
run (const gchar   *name,
     gint     nparams,
     const GimpParam  *param,
     gint    *nreturn_vals,
     GimpParam **return_vals)
{
  static GimpParam values[1];
  GimpDrawable *drawable;
  GimpPDBStatusType status = GIMP_PDB_SUCCESS;
  GimpRunMode run_mode = (GimpRunMode)param[0].data.d_int32;
  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = status;

  *nreturn_vals = 1;
  *return_vals = values;

  drawable = gimp_drawable_get(param[2].data.d_drawable);
  wideangleSetupContext(drawable);

  switch (run_mode) {
    case GIMP_RUN_INTERACTIVE:
      gimp_get_data(PLUGIN_NAME, &vals);
      wideangleDialog();
      if (!contextVals.do_it) {
	return;
      }
      break;

    case GIMP_RUN_NONINTERACTIVE:
      if (nparams != 9)
	status = GIMP_PDB_CALLING_ERROR;

      if (status == GIMP_PDB_SUCCESS)
	{
	  vals.centre_x = param[3].data.d_float;
	  vals.centre_y = param[4].data.d_float;
	  vals.square_a = param[5].data.d_float;
	  vals.quad_a = param[6].data.d_float;
	  vals.scale_a = param[7].data.d_float;
	  vals.brighten = param[8].data.d_float;
	}

      break;

    case GIMP_RUN_WITH_LAST_VALS:
      gimp_get_data(PLUGIN_NAME, &vals);
      break;
    
    default:
      break;
    }

  wideangleSetupCalc();
  wideangle();

  if (run_mode != GIMP_RUN_NONINTERACTIVE)
    gimp_displays_flush();
  if (run_mode == GIMP_RUN_INTERACTIVE)
    gimp_set_data(PLUGIN_NAME, &vals, sizeof (WideangleValues));

  values[0].data.d_status = status;
  
  gimp_drawable_detach(drawable);
}

static void
getSourceCoords(double i, double j, double* x, double* y, double* mag) {

  double radius_sq;

  double off_x;
  double off_y;

  double radius_mult;

  off_x = i - calcVals.centre_x;
  off_y = j - calcVals.centre_y;
  radius_sq = (off_x * off_x) + (off_y * off_y);

  radius_sq *= calcVals.normallise_radius_sq;

  radius_mult = radius_sq * calcVals.mult_sq + radius_sq * radius_sq * calcVals.mult_qd;
  *mag = radius_mult;
  radius_mult = calcVals.rescale * (1.0 + radius_mult);

  *x = calcVals.centre_x + radius_mult * off_x;
  *y = calcVals.centre_y + radius_mult * off_y;
}

static void
wideangleSetupContext(GimpDrawable* drawable)
{
  contextVals.drawable = drawable;
  contextVals.id = drawable->drawable_id;
  contextVals.width = drawable->width;
  contextVals.height = drawable->height;
  contextVals.depth = drawable->bpp;
  contextVals.do_it = FALSE;
}

static void
wideangleSetupCalc()
{
  calcVals.normallise_radius_sq =
    4.0 / (contextVals.width * contextVals.width + contextVals.height * contextVals.height);
  calcVals.centre_x = contextVals.width * (100.0 + vals.centre_x) / 200.0;
  calcVals.centre_y = contextVals.height * (100.0 + vals.centre_y) / 200.0;
  calcVals.mult_sq = vals.square_a / 200.0;
  calcVals.mult_qd = vals.quad_a / 200.0;
  calcVals.rescale = pow(2.0, - vals.scale_a / 100.0);
  calcVals.brighten = - vals.brighten / 10.0;
}

/*
 * Catmull-Rom cubic interpolation
 *
 * equally spaced points p0, p1, p2, p3
 * interpolate 0 <= u < 1 between p1 and p2
 *
 * (1 u u^2 u^3) (  0.0  1.0  0.0  0.0 ) (p0)
 *               ( -0.5  0.0  0.5  0.0 ) (p1)
 *               (  1.0 -2.5  2.0 -0.5 ) (p2)
 *               ( -0.5  1.5 -1.5  0.5 ) (p3)
 *
 */

void
cubicInterpolate(guchar* src, int rowStride, int srcDepth, guchar* dst, int dstDepth,
  double dx, double dy, double brighten)
{
  float um1, u, up1, up2;
  float vm1, v, vp1, vp2;
  int c;
  float verts[4 * MAX_PIXEL_DEPTH];

  um1 = ((-0.5 * dx + 1.0) * dx - 0.5) * dx;
  u = (1.5 * dx - 2.5) * dx * dx + 1.0;
  up1 = ((-1.5 * dx + 2.0) * dx + 0.5) * dx;
  up2 = (0.5 * dx - 0.5) * dx * dx;

  vm1 = ((-0.5 * dy + 1.0) * dy - 0.5) * dy;
  v = (1.5 * dy - 2.5) * dy * dy + 1.0;
  vp1 = ((-1.5 * dy + 2.0) * dy + 0.5) * dy;
  vp2 = (0.5 * dy - 0.5) * dy * dy;

  /* Note: if dstDepth < srcDepth, we calculate unneeded pixels here */
  /* later - select or create index array */
  for (c = 0; c < 4 * srcDepth; ++c) {
    verts[c] = vm1 * src[c] + v * src[c+rowStride] + vp1 * src[c+rowStride*2] + vp2 * src[c+rowStride*3];
  }
  for (c = 0; c < dstDepth; ++c) {
    float result;
    result = um1 * verts[c] + u * verts[c+srcDepth] + up1 * verts[c+srcDepth*2] + up2 * verts[c+srcDepth*3];
    result *= brighten;
    if (result < 0.0) {
      dst[c] = 0;
    } else if (result > 255.0) {
      dst[c] = 255;
    } else {
      dst[c] = result;
    }
  }
}

/* Solving the eternal problem: random, cubic-interpolated,
 * sub-pixel coordinate access to a tiled image.
 * Assuming that accesses are at least slightly coherent,
 * PixelAccess keeps PixelAccessRegions buffers, each containing a
 * PixelAccessWidth x PixelAccessHeight region of pixels.
 * Buffer[0] is always checked first, so move the last accessed
 * region into that position.
 * When a request arrives which is outside all the regions,
 * get a new region (using GimpPixelRgn - good idea? bad idea?).
 * The new region is placed so that the requested pixel is positioned
 * at [PixelAccessXOffset, PixelAccessYOffset] in the region.
 */

#define PixelAccessRegions 20
#define PixelAccessWidth 40
#define PixelAccessHeight 20
#define PixelAccessXOffset 3
#define PixelAccessYOffset 3

#define GATHERING_STATS 0

typedef struct {
  GimpPixelRgn srcPR;
  guchar* buffer[PixelAccessRegions];
  int width;
  int height;
  int depth;
  int imageWidth;
  int imageHeight;
  int tileMinX[PixelAccessRegions];
  int tileMaxX[PixelAccessRegions];
  int tileMinY[PixelAccessRegions];
  int tileMaxY[PixelAccessRegions];
#if GATHERING_STATS
  int pixelsFound;
  int pixelsFoundInBuffer[PixelAccessRegions];
  int pixelsLoadedFromImage;
#endif
} PixelAccess;

static PixelAccess* pa = 0;

void
pixelAccessDumpRegion(PixelAccess* pa, int n)
{
  printf("Region %d: buffer %p\n", n, pa->buffer[n]);
  printf("  X min %d max %d, Y min %d max %d\n",
    pa->tileMinX[n], pa->tileMaxX[n],
    pa->tileMinY[n], pa->tileMaxY[n]);
}

#if GATHERING_STATS
void
pixelAccessDumpStats(PixelAccess* pa)
{
  printf("Pixels found %d\n", pa->pixelsFound);
  if (pa->pixelsFound) {
    int i;
    for (i = 0; i < PixelAccessRegions; ++i) {
      printf("  In buffer[%d]: %d ratio %f\n", i, pa->pixelsFoundInBuffer[i],
        (float)pa->pixelsFoundInBuffer[i] / (float)pa->pixelsFound );
      pa->pixelsFoundInBuffer[i] = 0;
    }
    printf("  Loaded: %d ratio %f\n", pa->pixelsLoadedFromImage,
      (float)pa->pixelsLoadedFromImage / (float)pa->pixelsFound );
    pa->pixelsLoadedFromImage = 0;
  }
  pa->pixelsFound = 0;
}
#endif

PixelAccess*
pixelAccessNew(GimpDrawable* drawable)
{
  int i;
  PixelAccess* pa = (PixelAccess*)g_malloc(sizeof(PixelAccess));

  pa->width = PixelAccessWidth;
  pa->height = PixelAccessHeight;
  pa->depth = drawable->bpp;
  pa->imageWidth = drawable->width;
  pa->imageHeight = drawable->height;

  gimp_pixel_rgn_init(&pa->srcPR, drawable, 0, 0, drawable->width, drawable->height, FALSE, FALSE);

  for (i = 0; i < PixelAccessRegions; ++i) {
    pa->buffer[i] = (guchar*)g_malloc(pa->height * pa->width * pa->depth);
    /* better: pixelAccessReposition(), pixelAccessSelect()! */
    gimp_pixel_rgn_get_rect(&pa->srcPR, pa->buffer[i], 0, 0, pa->width, pa->height);
    pa->tileMinX[i] = 1;
    pa->tileMaxX[i] = pa->width - 2;
    pa->tileMinY[i] = 1;
    pa->tileMaxY[i] = pa->height - 2;
 #if GATHERING_STATS
   pa->pixelsFoundInBuffer[i] = 0;
#endif
  }
#if GATHERING_STATS
  pa->pixelsFound = 0;
  pa->pixelsLoadedFromImage = 0;
#endif

  return pa;
}

void
pixelAccessDelete(PixelAccess* pa)
{
  int i;
  for (i = 0; i < PixelAccessRegions; ++i) {
    g_free(pa->buffer[i]);
  }
  g_free(pa);
}

inline guchar*
pixelAccessAddress(PixelAccess* pa, int i, int j)
{
  return pa->buffer[0] + pa->depth * (pa->width * (j + 1 - pa->tileMinY[0]) + (i + 1 - pa->tileMinX[0]));
}

/* swap region[n] with region[0] */
void
pixelAccessSelectRegion(PixelAccess* pa, int n)
{
  guchar* temp;
  int a, b, c, d;
  int i;

  temp = pa->buffer[n];
  a = pa->tileMinX[n];
  b = pa->tileMaxX[n];
  c = pa->tileMinY[n];
  d = pa->tileMaxY[n];

  for (i = n; i > 0; --i) {
    pa->buffer[i] = pa->buffer[i-1];
    pa->tileMinX[i] = pa->tileMinX[i-1];
    pa->tileMaxX[i] = pa->tileMaxX[i-1];
    pa->tileMinY[i] = pa->tileMinY[i-1];
    pa->tileMaxY[i] = pa->tileMaxY[i-1];
  }

  pa->buffer[0] = temp;
  pa->tileMinX[0] = a;
  pa->tileMaxX[0] = b;
  pa->tileMinY[0] = c;
  pa->tileMaxY[0] = d;

}

/* buffer[0] is cleared, should start at [i, j], fill rows that overlap image */
void
pixelAccessDoEdge(PixelAccess* pa, int i, int j)
{
  int y;
  int lineStart, lineEnd;
  int rowStart, rowEnd;
  int lineWidth;
  guchar* line;

  lineStart = i;
  if (lineStart < 0) lineStart = 0;
  lineEnd = i + pa->width;
  if (lineEnd > pa->imageWidth) lineEnd = pa->imageWidth;
  lineWidth = lineEnd - lineStart;

  if (lineStart >= lineEnd) {
    return;
  }

  rowStart = j;
  if (rowStart < 0) rowStart = 0;
  rowEnd = j + pa->height;
  if (rowEnd > pa->imageHeight) rowEnd = pa->imageHeight;

  for (y = rowStart; y < rowEnd; ++y) {
    line = pixelAccessAddress(pa, lineStart, y);
#if GATHERING_STATS
    pa->pixelsLoadedFromImage += lineWidth;
#endif
    gimp_pixel_rgn_get_row(&pa->srcPR, line, lineStart, y, lineWidth);
  }
}

/* moves buffer[0] so that [x, y] is inside it */
void
pixelAccessReposition(PixelAccess* pa, int xInt, int yInt)
{
  int newStartX, newStartY;

  /* experiment! */
  /* could look at current position of region, for example... */
  /* this assumes random direction */
  /*
   *  newStartX = xInt - pa->width / 2;
   *  newStartY = yInt - pa->height / 2;
   */
#if 0
  /* what's wrong here? */
  /* creating an array of overlapped tiles should work fine... */
  newStartX = (PixelAccessWidth - 4) * (xInt / (PixelAccessWidth - 4)) - PixelAccessXOffset;
  newStartY = (PixelAccessHeight - 4) * (yInt / (PixelAccessHeight - 4)) - PixelAccessYOffset;
#else
  /* this assumes mostly stepping min->max in x and y */
  newStartX = xInt - PixelAccessXOffset;
  newStartY = yInt - PixelAccessYOffset;
#endif

  pa->tileMinX[0] = newStartX + 1;
  pa->tileMaxX[0] = newStartX + pa->width - 2;
  pa->tileMinY[0] = newStartY + 1;
  pa->tileMaxY[0] = newStartY + pa->height - 2;

  if ( (newStartX < 0) || ((newStartX + pa->width) >= pa->imageWidth) ||
       (newStartY < 0) || ((newStartY + pa->height) >= pa->imageHeight) ) {
    /* some data is off edge of image */

    memset(pa->buffer[0], 0, pa->width * pa->height * pa->depth);

    if ( ((newStartX + pa->width) < 0) || (newStartX >= pa->imageWidth) ||
	 ((newStartY + pa->height) < 0) || (newStartY >= pa->imageHeight) ) {
      /* totally outside, just leave it */

    } else {
      pixelAccessDoEdge(pa, newStartX, newStartY);
    }

  } else {
#if GATHERING_STATS
    pa->pixelsLoadedFromImage += pa->width * pa->height;
#endif
    gimp_pixel_rgn_get_rect(&pa->srcPR, pa->buffer[0], newStartX, newStartY, pa->width, pa->height);
  }
}

void
pixelAccessGetCubic(PixelAccess* pa, double srcX, double srcY, double brighten, guchar* dst, int dstDepth)
{
  int i;

  int xInt, yInt;
  double dx, dy;
  guchar* corner;

  xInt = floor(srcX);
  dx = srcX - xInt;
  yInt = floor(srcY);
  dy = srcY - yInt;

#if GATHERING_STATS
  ++pa->pixelsFound;
#endif

  /* we need 4x4 pixels, xInt-1 to xInt+2 horz, yInt-1 to yInt+2 vert */
  /* they're probably in the last place we looked... */
  if ((xInt >= pa->tileMinX[0]) && (xInt < pa->tileMaxX[0]) &&
      (yInt >= pa->tileMinY[0]) && (yInt < pa->tileMaxY[0]) ) {
#if GATHERING_STATS
    ++pa->pixelsFoundInBuffer[0];
#endif
    corner = pixelAccessAddress(pa, xInt - 1, yInt - 1);
    cubicInterpolate(corner, pa->depth * pa->width, pa->depth, dst, dstDepth, dx, dy, brighten);
    return;
  }

  /* or maybe it was a while back... */
  for (i = 1; i < PixelAccessRegions; ++i) {
    if ((xInt >= pa->tileMinX[i]) && (xInt < pa->tileMaxX[i]) &&
        (yInt >= pa->tileMinY[i]) && (yInt < pa->tileMaxY[i]) ) {
#if GATHERING_STATS
      ++pa->pixelsFoundInBuffer[i];
#endif
      /* check here first next time */
      pixelAccessSelectRegion(pa, i);
      corner = pixelAccessAddress(pa, xInt - 1, yInt - 1);
      cubicInterpolate(corner, pa->depth * pa->width, pa->depth, dst, dstDepth, dx, dy, brighten);
      return;
    }
  }

  /* nope, recycle an old region */
  pixelAccessSelectRegion(pa, PixelAccessRegions - 1);
  pixelAccessReposition(pa, xInt, yInt);

  corner = pixelAccessAddress(pa, xInt - 1, yInt - 1);
  cubicInterpolate(corner, pa->depth * pa->width, pa->depth, dst, dstDepth, dx, dy, brighten);
}

/*
 * start at image (i, j), increment by (step, step)
 * output goes to dst, which is w x h x d in size
 * NB: d <= image.bpp
 */
void
calcWideangleRect(int i, int j, int step, int dstWidth, int dstHeight, int dstDepth, guchar* dst) {

  int dstI, dstJ;
  int iLimit, jLimit;
  double srcX, srcY, mag, brighten;

  iLimit = i + dstWidth * step;
  jLimit = j + dstHeight * step;

  for (dstJ = j; dstJ < jLimit; dstJ += step) {
    for (dstI = i; dstI < iLimit; dstI += step) {

      getSourceCoords(dstI, dstJ, &srcX, &srcY, &mag);
      brighten = 1.0 + mag * calcVals.brighten;
      pixelAccessGetCubic(pa, srcX, srcY, brighten, dst, dstDepth);
      dst += dstDepth;
    }
  }
}

static void
wideangle() {

  GimpPixelRgn destPR;
  gpointer pr;
  guchar* dest;
  gint x1, y1, x2, y2;
  int progress, progress_max;

  if (pa == 0) {
    pa = pixelAccessNew(contextVals.drawable);
  }

#if GATHERING_STATS
  pixelAccessDumpStats(pa);
#endif

  gimp_progress_init("Applying wideangle...");

  gimp_drawable_mask_bounds(contextVals.id, &x1, &y1, &x2, &y2);
  progress_max = ((x2 - x1 - 1) / gimp_tile_width() + 1) * ((y2 - y1 - 1) / gimp_tile_height() + 1);

  dest = g_malloc(gimp_tile_width() * gimp_tile_height() * contextVals.depth);

  gimp_pixel_rgn_init(&destPR, contextVals.drawable, x1, y1, x2-x1, y2-y1, TRUE, TRUE);
  pr = gimp_pixel_rgns_register(1, &destPR);

  progress = 0;
  for ( ; pr; pr = gimp_pixel_rgns_process(pr)) {

    calcWideangleRect(destPR.x, destPR.y, 1, destPR.w, destPR.h, destPR.bpp, destPR.data);
    gimp_progress_update((float)progress / (float)progress_max);
    ++progress;
  }

  g_free(dest);

  gimp_drawable_flush(contextVals.drawable);
  gimp_drawable_merge_shadow(contextVals.id, TRUE);
  gimp_drawable_update(contextVals.id, x1, y1, (x2 - x1), (y2 - y1));

#if GATHERING_STATS
  pixelAccessDumpStats(pa);
#endif
}

static void
previewInit(WideanglePreview* wp, int detail)
{
  if (detail) {
    wp->step = 1;
    wp->xSpacing = (contextVals.width - wp->width) / 4;
    wp->xStart = (contextVals.width - (wp->width + wp->xSpacing * 4)) / 2;
    wp->ySpacing = (contextVals.height - wp->height) / 4;
    wp->yStart = (contextVals.height - (wp->height + wp->ySpacing * 4)) / 2;
  } else {
    wp->step = wp->maxStep;
    wp->xStart = (contextVals.width - wp->width * wp->step * 5) / 2;
    wp->xSpacing = wp->width * wp->step;
    wp->yStart = (contextVals.height - wp->height * wp->step * 5) / 2;
    wp->ySpacing = wp->height * wp->step;
  }
}

static WideanglePreview*
previewCreate()
{
  int scale;

  WideanglePreview* wp = (WideanglePreview*) g_malloc(sizeof(WideanglePreview));

  scale = floor(sqrt(contextVals.width * contextVals.height / PREVIEW_ROUGH_AREA));
  if (scale < 1) scale = 1;
  wp->maxStep = scale;
  wp->width = contextVals.width / (scale * 5);
  wp->height = contextVals.height / (scale * 5);

  if (gimp_drawable_is_rgb(contextVals.id)) {
    wp->depth = 3;
    wp->preview = gtk_preview_new(GTK_PREVIEW_COLOR);
  } else {
    wp->depth = 1;
    wp->preview = gtk_preview_new(GTK_PREVIEW_GRAYSCALE);
  }
  wp->buffer = g_malloc0(wp->width * wp->height * wp->depth);

  gtk_preview_size(GTK_PREVIEW(wp->preview), wp->width * 5, wp->height * 5);
  wp->enabled = 0;
  wp->progress = 0;

  previewInit(wp, 0);

  return wp;
}

static void
previewStartUpdate(WideanglePreview* wp)
{
  if (!wp->enabled) {
    return;
  }

#if GATHERING_STATS
  pixelAccessDumpStats(pa);
#endif

  wideangleSetupCalc();
  if (wp->progress == 0) {
    gtk_idle_add(previewIdleUpdateCB, wp);
  }
  wp->progress = 25;
}

static gint
previewIdleUpdateCB(gpointer arg)
{
  WideanglePreview* wp = (WideanglePreview*)arg;
  int x, y;
  int i;

  if (wp->enabled == 0) {
    wp->progress = 0;
    return 0;
  }

  --wp->progress;
  x = wp->progress / 5;
  y = wp->progress % 5;

  memset(wp->buffer, 255, wp->width * wp->depth);
  gtk_preview_draw_row(GTK_PREVIEW(wp->preview), wp->buffer, x * wp->width, y * wp->height, wp->width);
  gtk_widget_draw(wp->preview, 0);
  gdk_flush();

  calcWideangleRect(wp->xStart + x * wp->xSpacing, wp->yStart + y * wp->ySpacing, wp->step,
    wp->width, wp->height, wp->depth, wp->buffer);

  for (i = 0; i < wp->height; ++i) {
    gtk_preview_draw_row(GTK_PREVIEW(wp->preview), wp->buffer + i * wp->width * wp->depth,
      x * wp->width, y * wp->height + i, wp->width);
  }
  gtk_widget_draw(wp->preview, 0);

  if (wp->progress == 0) {
    return 0;
  }

  return 1;
}


/* UI callback functions */

static void
valueChangedCB(GtkWidget *widget, gpointer data) {

  vals.centre_x = guiVals.centre_x->value;
  vals.centre_y = guiVals.centre_y->value;
  vals.square_a = guiVals.square_a->value;
  vals.quad_a = guiVals.quad_a->value;
  vals.scale_a = guiVals.scale_a->value;
  vals.brighten = guiVals.bright_a->value;

  gtk_widget_set_sensitive(guiVals.saveButton, TRUE);
  previewStartUpdate(guiVals.preview);
}

static void
previewEnableCB(GtkWidget *widget, gpointer data) {
  guiVals.preview->enabled = GTK_TOGGLE_BUTTON(widget)->active;
  if (guiVals.preview->enabled) {
    gtk_widget_show_all(guiVals.previewBox);
    previewStartUpdate(guiVals.preview);
  } else {
    gtk_widget_hide_all(guiVals.previewBox);
  }
}

static void
previewDetailCB(GtkWidget *widget, gpointer data) {
  previewInit(guiVals.preview, GTK_TOGGLE_BUTTON(widget)->active);
  if (guiVals.preview->enabled) {
    previewStartUpdate(guiVals.preview);
  }
}

static void
optionSelectCB(GtkWidget *widget, gpointer data) {
  if (data) {
    WideangleValues* preset = (WideangleValues*)data;
    gtk_adjustment_set_value(guiVals.centre_x, preset->centre_x);
    gtk_adjustment_set_value(guiVals.centre_y, preset->centre_y);
    gtk_adjustment_set_value(guiVals.square_a, preset->square_a);
    gtk_adjustment_set_value(guiVals.quad_a, preset->quad_a);
    gtk_adjustment_set_value(guiVals.scale_a, preset->scale_a);
    gtk_adjustment_set_value(guiVals.bright_a, preset->brighten);
  }
  gtk_widget_set_sensitive(guiVals.saveButton, FALSE);
}

/* utility function to add a preset option */
static void
addPreset(const char* title, WideangleValues* values)
{
  WideanglePreset* preset;
  GtkWidget* item;

  preset = g_new(WideanglePreset, 1);
  preset->val = values;
  strncpy(preset->name, title, 100);
  guiVals.presets = g_slist_append(guiVals.presets, preset);

  item = gtk_menu_item_new_with_label(preset->name);
  gtk_signal_connect(GTK_OBJECT(item), "activate",
    GTK_SIGNAL_FUNC(optionSelectCB), values);
  gtk_widget_show(item);
  gtk_menu_append(GTK_MENU(guiVals.presetMenu), item);

  gtk_widget_set_sensitive(guiVals.selectButton, TRUE);
}

static void
saveDoItCB(GtkWidget *query_box, const gchar *string, gpointer data)
{
  WideangleValues* values;

  if (string == 0) {
    return;
  }
  if (strlen(string) == 0) {
    //  g_free(string);
    return;
  }

  values = (WideangleValues*)g_malloc(sizeof(WideangleValues));
  memcpy(values, &vals, sizeof(WideangleValues));
  addPreset(string, values);
  writePresetFile();
  gtk_widget_set_sensitive(guiVals.saveButton, FALSE);
}

static void
resetCB(GtkWidget *widget, gpointer data) {
  gtk_adjustment_set_value(guiVals.centre_x, 0.0);
  gtk_adjustment_set_value(guiVals.centre_y, 0.0);
  gtk_adjustment_set_value(guiVals.square_a, 0.0);
  gtk_adjustment_set_value(guiVals.quad_a, 0.0);
  gtk_adjustment_set_value(guiVals.scale_a, 0.0);
  gtk_adjustment_set_value(guiVals.bright_a, 0.0);
  gtk_widget_set_sensitive(guiVals.saveButton, FALSE);
}

static void
savePresetCB(GtkWidget *widget, gpointer data) {
  GtkWidget* dialog;

  dialog = gimp_query_string_box("Save Preset",
    0, 0, 0, /* parent, help_func, help_id */
    "Enter name for new preset", "newPreset",
    NULL, NULL, /* object, (deletion)signal */
    &saveDoItCB, 0);
  gtk_widget_show_all(dialog);
}

static gint
selectPresetCB(GtkWidget* widget, GdkEventButton* event)
{
  gtk_menu_popup(GTK_MENU(guiVals.presetMenu), NULL, NULL, NULL, NULL,
    event->button, event->time);
  return TRUE;
}

static void
readPresetFile() {
  gchar* fileName;
  FILE* file;
  gchar presetName[101];
  float x, y, s, q, r, b;

  fileName =
    g_strdup_printf("%s" G_DIR_SEPARATOR_S "wideangle" G_DIR_SEPARATOR_S "presets.txt",
      gimp_directory());
  file = fopen(fileName, "r");
  if (file) {
    while (7 == fscanf(file, "%f %f %f %f %f %f %100[^\n]\n", &x, &y, &s, &q, &r, &b, presetName)) {
      WideangleValues* newVal = (WideangleValues*)g_malloc(sizeof(WideangleValues));
      newVal->centre_x = x;
      newVal->centre_y = y;
      newVal->square_a = s;
      newVal->quad_a = q;
      newVal->scale_a = r;
      newVal->brighten = b;
      addPreset(presetName, newVal);
    }
    fclose(file);
  }
  g_free(fileName);
}

static void
writePresetFile() {
  gchar* dirName;
  gchar* fileName;
  FILE* file;

  /* half-assed attempt to make sure the directory exists */
  dirName = g_strdup_printf("%s" G_DIR_SEPARATOR_S "wideangle", gimp_directory());
  mkdir(dirName, 0777);
  g_free(dirName);

  fileName =
    g_strdup_printf("%s" G_DIR_SEPARATOR_S "wideangle" G_DIR_SEPARATOR_S "presets.txt",
      gimp_directory());
  file = fopen(fileName, "w");
  if (file) {
    GSList* item = guiVals.presets;
    while (item) {
      WideanglePreset* preset = (WideanglePreset*)(item->data);
      fprintf(file, "%f %f %f %f %f %f %s\n",
        preset->val->centre_x, preset->val->centre_y,
        preset->val->square_a, preset->val->quad_a,
        preset->val->scale_a, preset->val->brighten,
        preset->name);
      item = item->next;
    }
    fclose(file);
  }
  g_free(fileName);
}

/* utility fn to create a control group and add it to the control panel */
static GtkAdjustment*
addControl(GtkWidget* vbox, const char* name, double init) {

  GtkWidget* label;
  GtkWidget* hbox;
  GtkWidget* hbox2;
  GtkWidget* slider;
  GtkWidget* spinner;
  GtkAdjustment* adjustment;
  GtkStyle* style;
  gint zeroWidth;

  label = gtk_label_new(name);

  adjustment = GTK_ADJUSTMENT(gtk_adjustment_new(init, -100.0, +100.0, 0.1, 1.0, 0.0));

  slider = gtk_hscale_new(adjustment);
  gtk_scale_set_digits(GTK_SCALE(slider), 3);
  gtk_scale_set_draw_value(GTK_SCALE(slider), FALSE);

  spinner = gtk_spin_button_new(adjustment, 0.0, 1);
  style = gtk_widget_get_style(spinner);
  /* Need sensible width for value display */
  /* Add a small amount to accommodate the arrows */
  /* Of course, this should be done automagically */
  zeroWidth = gdk_string_width(gtk_style_get_font(style), "0");
  gtk_widget_set_usize(GTK_WIDGET(spinner), zeroWidth * 10, 0);

  hbox = gtk_hbox_new(TRUE, 2);
  hbox2 = gtk_hbox_new(TRUE, 2);
  gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox2), spinner, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), hbox2, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), slider, TRUE, TRUE, 0);

  gtk_signal_connect(GTK_OBJECT(adjustment), "value_changed",
		      GTK_SIGNAL_FUNC(valueChangedCB),
		      0);

  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 0);

  return adjustment;
}

static void
wideangleDialog()
{
  GtkWidget* dialog;
  GtkWidget* toggle;
  GtkWidget* controls;
  GtkWidget* frame;
  GtkWidget* vbox;
  GtkWidget* hbox;
  GtkWidget* hbox2;
  GtkWidget* button;

  if (pa == 0) {
    pa = pixelAccessNew(contextVals.drawable);
  }

  gimp_ui_init(PLUGIN_NAME, TRUE);

  guiVals.do_preview = 0;

  dialog = gimp_dialog_new(
    "Wideangle Effect",
    PLUGIN_NAME,
    0, 0, 0, 0,
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_OK, GTK_RESPONSE_OK,
    NULL);

  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 0);

  /* control widgets */
  controls = gtk_vbox_new(FALSE, 0);
  /*  gtk_container_set_border_width(GTK_CONTAINER(controls), 4); */
  gtk_box_pack_start(GTK_BOX(hbox), controls, TRUE, TRUE, 0);

  frame = gtk_frame_new("Parameter Settings");
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
  gtk_container_set_border_width(GTK_CONTAINER(frame), 6);
  gtk_box_pack_start(GTK_BOX(controls), frame, FALSE, FALSE, 0);

  vbox = gtk_vbox_new(FALSE, 2);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
  gtk_container_add(GTK_CONTAINER(frame), vbox);

  guiVals.centre_x = addControl(vbox, "X Shift", vals.centre_x);
  guiVals.centre_y = addControl(vbox, "Y Shift", vals.centre_y);
  guiVals.square_a = addControl(vbox, "Main", vals.square_a);
  guiVals.quad_a = addControl(vbox, "Edge", vals.quad_a);
  guiVals.scale_a = addControl(vbox, "Zoom", vals.scale_a);
  guiVals.bright_a = addControl(vbox, "Brighten", vals.brighten);

  frame = gtk_frame_new("Presets");
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
  gtk_container_set_border_width(GTK_CONTAINER(frame), 6);
  gtk_box_pack_start(GTK_BOX(controls), frame, FALSE, FALSE, 0);

  hbox2 = gtk_hbox_new(TRUE, 2);
  gtk_container_set_border_width(GTK_CONTAINER(hbox2), 4);
  gtk_container_add(GTK_CONTAINER(frame), hbox2);

  button = gtk_button_new_with_label("Reset");
  /* Is this really the way to give the label some air? */
  gtk_misc_set_padding(GTK_MISC(GTK_BIN(button)->child), 6, 3);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                      GTK_SIGNAL_FUNC(resetCB), 0);
  gtk_box_pack_start(GTK_BOX(hbox2), button, FALSE, FALSE, 0);


  guiVals.presets = NULL;
  guiVals.presetMenu = gtk_menu_new();
  gtk_widget_show_all(guiVals.presetMenu);


  guiVals.selectButton = gtk_button_new_with_label("Select");
  gtk_widget_set_sensitive(guiVals.selectButton, FALSE);
  gtk_misc_set_padding(GTK_MISC(GTK_BIN(guiVals.selectButton)->child), 6, 3);
  gtk_signal_connect_object(GTK_OBJECT(guiVals.selectButton), "button_press_event",
    GTK_SIGNAL_FUNC(selectPresetCB), NULL);
  gtk_box_pack_start(GTK_BOX(hbox2), guiVals.selectButton, FALSE, FALSE, 0);


  guiVals.saveButton = gtk_button_new_with_label("Save As");
  gtk_misc_set_padding(GTK_MISC(GTK_BIN(guiVals.saveButton)->child), 6, 3);
  gtk_widget_set_sensitive(guiVals.saveButton, FALSE);
  gtk_signal_connect(GTK_OBJECT(guiVals.saveButton), "clicked",
                      GTK_SIGNAL_FUNC(savePresetCB), 0);
  gtk_box_pack_start(GTK_BOX(hbox2), guiVals.saveButton, FALSE, FALSE, 0);


  hbox2 = gtk_hbox_new(FALSE, 2);
  gtk_container_set_border_width(GTK_CONTAINER(hbox2), 4);
  gtk_box_pack_start(GTK_BOX(controls), hbox2, FALSE, FALSE,0);

  /* would like to put check box on other side */
  toggle = gtk_check_button_new_with_label("Do Preview");
  gtk_container_set_border_width(GTK_CONTAINER(toggle), 4);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), guiVals.do_preview);
  gtk_signal_connect(GTK_OBJECT(toggle), "toggled",
                      GTK_SIGNAL_FUNC(previewEnableCB), 0);
  gtk_box_pack_end(GTK_BOX(hbox2), toggle, FALSE, FALSE, 2);


  /* That's all that's normally visible */
  gtk_widget_show_all(dialog);



  /* preview widgets - hidden by default */
  /* on rhs so that dialog box doesn't jump when they're made visible */
  /* however, dialog OK and Cancel buttons jump instead... */
  /* maybe should put preview on left and adjust (?)window gravity(?) */
  guiVals.previewBox = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), guiVals.previewBox, FALSE, FALSE, 8);

  guiVals.preview = previewCreate();
  gtk_box_pack_start(GTK_BOX(guiVals.previewBox), guiVals.preview->preview, FALSE, FALSE, 8);

  toggle = gtk_check_button_new_with_label("Detail");
  gtk_signal_connect(GTK_OBJECT(toggle), "toggled",
                      GTK_SIGNAL_FUNC(previewDetailCB), 0);
  gtk_box_pack_start(GTK_BOX(guiVals.previewBox), toggle, FALSE, FALSE, 2);

  gtk_widget_hide_all(guiVals.previewBox);


  readPresetFile();

  valueChangedCB(0, 0);

  if (gimp_dialog_run(GIMP_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    contextVals.do_it = TRUE;
  }
  gtk_widget_destroy(dialog);

#if GATHERING_STATS
  pixelAccessDumpStats(pa);
#endif
}
