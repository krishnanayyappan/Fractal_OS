
#include "bitmap.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>

int threadcount;

typedef struct
{
    struct bitmap *bm;
/*  double xstart;
    double xstop;
    double ystart;
    double ystop; */
    double xmin; 
    double xmax;
    double ymin;
    double ymax;
    double maxiter;
    int    start_height;
    int    stop_height;
} thread_args;

int iteration_to_color( int i, int max );
int iterations_at_point( double x, double y, int max );
//void compute_image( struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max );
void * compute_image(void *temp );

void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates. (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=500)\n");
	printf("-H <pixels> Height of the image in pixels. (default=500)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}

int main( int argc, char *argv[] )
{
	char c;
	// These are the default configuration values used
	// if no command line arguments are given.

        struct timeval start, stop;

        threadcount = 1;
	const char *outfile = "mandel.bmp";
	double xcenter = 0;
	double ycenter = 0;
	double scale = 4;
	int    image_width = 500;
	int    image_height = 500;
	int    max = 1000;

        int i = 1;
	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:n:h"))!=-1) {
		switch(c) {
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				scale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'n':
				threadcount = atoi(optarg);
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

	// Display the configuration of the image.
//	printf("mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s\n",xcenter,ycenter,scale,max,outfile);
	printf("mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s threads=%d\n",xcenter,ycenter,scale,max,outfile,threadcount);

        int hgt_part = ( image_height / threadcount ) ;

        pthread_t* split = malloc(threadcount * sizeof(pthread_t));
        thread_args* parms = malloc(threadcount * sizeof(thread_args));

	// Create a bitmap of the appropriate size.
	struct bitmap *bm = bitmap_create(image_width,image_height);

	// Fill it with a dark blue, for debugging
	bitmap_reset(bm,MAKE_RGBA(0,0,255,0));

        gettimeofday(&start, NULL);

        for (i=0 ; i < threadcount ; i++)
        {
           parms[i].bm = bm;
           parms[i].xmin = ( xcenter - scale );
           parms[i].xmax = ( xcenter + scale );
           parms[i].ymin = ( ycenter - scale );
           parms[i].ymax = ( ycenter + scale );
           parms[i].maxiter = max ;

           if ( i == 0 )
           {
               parms[0].start_height = 0;
               parms[0].stop_height = hgt_part;
           }
           else
           {
               parms[i].start_height = parms[i-1].stop_height;
               parms[i].stop_height  = ( parms[i-1].stop_height + hgt_part );
           }

           pthread_create(&split[i], NULL, compute_image, (void *) &parms[i]);
        }

        for ( i = 0 ; i < threadcount ; i++)
        {
           printf("\nPrint join    = %d\n",i);
            pthread_join(split[i], NULL);
           printf("\nPrint complete= %d\n",i);
        }

	// Compute the Mandelbrot image
        // compute_image(bm,xcenter-scale,xcenter+scale,ycenter-scale,ycenter+scale,max);

        // Save the image in the stated file.
        if(!bitmap_save((*parms).bm,outfile)) {
            fprintf(stderr,"mandel: couldn't write to %s: %s\n",outfile,strerror(errno));
            return 1;
        }

       gettimeofday(&stop, NULL);
       printf("\nTime taken for multithreading = %lu\n", (( stop.tv_sec - start.tv_sec ) * 1000000 + (stop.tv_usec - start.tv_usec)));

       return 0;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

//void compute_image( struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max )
void *compute_image( void *temp )
{
	int i,j;

        thread_args* parms = (thread_args*) temp;
        int h1 = (*parms).start_height;
        int h2 = (*parms).stop_height;

	int width = bitmap_width((*parms).bm);
	int height = bitmap_height((*parms).bm);

	// For every pixel in the image...

        // for(j=0;j<height;j++) {
	for(j=h1;j<h2;j++) {

		for(i=0;i<width;i++) {

			// Determine the point in x,y space for that pixel.
                        // double x = xmin + i*(xmax-xmin)/width;
                        // double y = ymin + j*(ymax-ymin)/height;
                           double x = (*parms).xmin + i * ( (*parms).xmax - (*parms).xmin ) / width;
                           double y = (*parms).ymin + j * ( (*parms).ymax - (*parms).ymin ) / height;

			// Compute the iterations at that point.
                        // int iters = iterations_at_point(x,y,max);
                        int iters = iterations_at_point(x,y,(*parms).maxiter);

			// Set the pixel in the bitmap.
			bitmap_set((*parms).bm,i,j,iters);
		}
	}
        return (void *) 0;
}

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iteration_to_color(iter,max);
}

/*
Convert a iteration number to an RGBA color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/

int iteration_to_color( int i, int max )
{
	int gray = 255*i/max;
	return MAKE_RGBA(gray,gray,gray,0);
}




