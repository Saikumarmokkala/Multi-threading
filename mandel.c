/*

Name : SAI KUMAR REDDY MOKKALA
ID   : 1001728207

*/


#include "bitmap.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

struct thread_arguments

{
     struct bitmap *bm;  // Structure for bitmap reference
     double xmin;        //Minimum x co-ordinate
     double xmax;        //Maximum x co-ordinate
     double ymin;        //Minimum y co-ordinate
     double ymax;        //maximum y co-ordinate
     int max;
    // int height_increment;
     int start_height;          // start height of image
     int end_height;            // end height of image

};





// Function converts the iteration number into a color
int iteration_to_color( int i, int max );

/*
 Function return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/
int iterations_at_point( double x, double y, int max );
//void compute_image( struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max );


void*compute_image(void *args); // Generates the bitmap image



// Displays the help menu
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

    struct timeval starttime, endtime; // Declaring the struct for calculating time of function



    gettimeofday(&starttime, NULL);    // Gets the start time


	// These are the default configuration values used
	// if no command line arguments are given.

	const char *outfile = "mandel.bmp";
	double xcenter = 0;
	double ycenter = 0;
	double scale = 4;
	int    image_width = 500;
	int thread_count = 1;                // Default thread count is 1 if it is not given
	int    image_height = 500;
	int    max = 1000;

	// For each command line argument given,
	// override the appropriate configuration value.

	char c;

    // It will switch for differenr command line arguments
	while((c = getopt(argc,argv,"x:y:s:W:n:H:m:o:h"))!=-1)

    {
		switch(c)
		{
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
             case 'n':
                thread_count = atoi(optarg);
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

			case 'h':
				show_help();
				exit(1);
				break;



        }
	}

	// Display the configuration of the image.


	printf("mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s Number of threads=%d\n",xcenter,ycenter,scale,max,outfile,thread_count);



    int height_increment = image_height/thread_count;  // Taking the height for specific image based on the thread count






    pthread_t* thread = malloc(thread_count*sizeof(pthread_t));



    struct thread_arguments * cmd_options = malloc( thread_count*sizeof(struct thread_arguments));

    struct bitmap *bm = bitmap_create(image_width,image_height);



	/*bitmap_reset(bm,MAKE_RGBA(0,0,255,0));



	*/


    int z;

    for( z=0;  z< thread_count; z++)

    {
        /*
        Compute the Mandelbrot image
        compute_image(bm,xcenter-scale,xcenter+scale,ycenter-scale,ycenter+scale,max);

        computing the Mandelbrot image based on the above function given.

        */

        // Bitmap reference
        cmd_options[z].bm = bm;

        // Max iterations
        cmd_options[z].max = max;

        // Scaling max x co-ordinate
        cmd_options[z].xmax = xcenter+scale;

        // Scaling min x co-ordinate
        cmd_options[z].xmin = xcenter-scale;

        // Scaling max y co-ordinate
        cmd_options[z].ymax = ycenter+scale;

         // Scaling min y co-ordinate
        cmd_options[z].ymin = ycenter-scale;


        // if number of threads greater than zero, computing as per height increment
        if (z>0)

        {
            cmd_options[z].start_height = cmd_options[z-1].end_height;
            cmd_options[z].end_height = cmd_options[z-1].end_height + height_increment;

        }

        // if number of threads equal to zero

        else if (z==0)

        {

            cmd_options[z].start_height= 0;
            cmd_options[z].end_height = height_increment;
        }


        // creating the threads through pthread_create function
        pthread_create(&thread[z],NULL,compute_image,(void *)&cmd_options[z]);



	}



     // Joining the threads so that work is parallelized
	 for( z=0;  z< thread_count; z++)

    {
        pthread_join(thread[z],NULL);

    }



	// Save the image in the stated file.
	if(!bitmap_save(cmd_options->bm,outfile))
	{

		fprintf(stderr,"mandel: couldn't write to %s: %s\n",outfile,strerror(errno));
		return 1;
	}


    gettimeofday(&endtime, NULL); //gets the end time


    // Calculates the total time for function
    float time = (float)((endtime.tv_sec*1000000+ endtime.tv_usec)- (starttime.tv_sec*1000000 + starttime.tv_usec))/1000000;

    printf("The execution time for the process is: %f sec\n", time);



	return 0;

}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void* compute_image( void *args )
{
	int i,j;


     struct thread_arguments *o = (struct thread_arguments*) args;



	int startheight = o->start_height;
	int endheight = o-> end_height;



	int width = bitmap_width(o->bm);
	int height = bitmap_height(o->bm);

	// For every pixel in the image..




	for(j=startheight;j<endheight;j++)
    {

       for(i=0;i<width;i++)
       {
			// Determine the point in x,y space for that pixel.
			double x = o->xmin + i*(o->xmax-o->xmin)/width;
			double y = o->ymin + j*(o->ymax-o->ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,o->max);

			// Set the pixel in the bitmap.
			bitmap_set(o->bm,i,j,iters);
		}
	}

	return 0;
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

	while( (x*x + y*y <= 4) && iter < max )

    {

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


