#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include "Filter.h"

using namespace std;

#include "rdtsc.h"

//
// Forward declare the functions
//
Filter *readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input, cs1300bmp *output);

int main(int argc, char **argv) {

	if ( argc < 2) {
		fprintf(stderr, "Usage: %s filter inputfile1 inputfile2 .... \n", argv[0]);
	}

	//
	// Convert to w++ strings to simplify manipulation
	//
	string filtername = argv[1];

	//
	// remove any ".filter" in the filtername
	//
	string filterOutputName = filtername;
	string::size_type loc = filterOutputName.find(".filter");
	if (loc != string::npos) {
		//
		// Remove the ".filter" name, which should occur on all the provided filters
		//
		filterOutputName = filtername.substr(0, loc);
	}

	Filter *filter = readFilter(filtername);

	double sum = 0.0;
	int samples = 0;

	for (int inNum = 2; inNum < argc; inNum++) {
		string inputFilename = argv[inNum];
		string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
		struct cs1300bmp *input = new struct cs1300bmp;
		struct cs1300bmp *output = new struct cs1300bmp;
		int ok = cs1300bmp_readfile( (char *) inputFilename.c_str(), input);

		if ( ok ) {
			double sample = applyFilter(filter, input, output);
			sum += sample;
			samples++;
			cs1300bmp_writefile((char *) outputFilename.c_str(), output);
		}
		delete input;
		delete output;
	}
	fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);

}

struct Filter *
readFilter(string filename) {
	ifstream input(filename.c_str());

	if ( ! input.bad() ) {
		int size = 0;
		input >> size;
		Filter *filter = new Filter(size);
		int div;
		input >> div;
		filter -> setDivisor(div);
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				int value;
				input >> value;
				filter -> set(i, j, value);
			}
		}
		return filter;
	}
	else {
		cerr << "Bad input in readFilter:" << filename << endl;
		exit(-1);
	}
}


double applyFilter(struct Filter *filter, cs1300bmp *input, cs1300bmp *output) {

	long long cycStart, cycStop;
	cycStart = rdtscll();

	int imageHeight = input -> height - 1;
	int imageWidth = input -> width - 1;

	output -> width = imageWidth + 1;
	output -> height = imageHeight + 1;


	int filterSet[3][3];
	#pragma omp parallel for
	for (int i = 0; i < 3; i++) {
		filterSet[i][0] = filter -> get(i, 0);
		filterSet[i][1] = filter -> get(i, 1);
		filterSet[i][2] = filter -> get(i, 2);
	}

	int plane, h, w;

	// Hi-Line
	if (filterSet[0][1] == -2) {
		#pragma omp parallel for
		for (plane = 0; plane < 3; plane++) {
			for (h = 1; h < imageHeight; h++) {

				int row1 = h - 1;
				int row3 = h + 1;

				#pragma omp parallel for
				for (w = 1; w < imageWidth; w++) {

					int temp1 = 0, temp2 = 0, temp3 = 0;
					int col1 = w - 1;
					int col3 = w + 1;

					temp1 += -(input -> color[plane][row1][col1]);
					temp2 += -(input -> color[plane][row1][w] << 1);
					temp3 += -(input -> color[plane][row1][col3]);

					temp1 += input -> color[plane][row3][col1];
					temp2 += input -> color[plane][row3][w] << 1;
					temp3 += input -> color[plane][row3][col3];

					// no divisor
					temp1 = temp1 + temp2 + temp3;

					if (temp1 > 255) {
						output -> color[plane][h][w] = 255;
						continue;
					}
					if (temp1 < 0) {
						output -> color[plane][h][w] = 0;
						continue;
					}
					output -> color[plane][h][w] = temp1;
				}
			}
		}
	}

	// Gauss
	else if (filterSet[1][1] == 8) {
		#pragma omp parallel for
		for (plane = 0; plane < 3; plane++) {
			for (h = 1; h < imageHeight; h++) {

				int row1 = h - 1;
				int row3 = h + 1;

				#pragma omp parallel for
				for (w = 1; w < imageWidth; w++) {

					int temp1 = 0, temp2 = 0, temp3 = 0;
					int col1 = w - 1;
					int col3 = w + 1;

					temp2 += input -> color[plane][row1][w] << 2;

					temp1 += input -> color[plane][h][col1] << 2;
					temp2 += input -> color[plane][h][w] << 3;
					temp3 += input -> color[plane][h][col3] << 2;

					temp2 += input -> color[plane][row3][w] << 2;

					// divisor
					temp1 = ((temp1 + temp2 + temp3) >> 3) / 3;

					if (temp1 > 255) {
						output -> color[plane][h][w] = 255;
						continue;
					}
					if (temp1 < 0) {
						output -> color[plane][h][w] = 0;
						continue;
					}
					output -> color[plane][h][w] = temp1;
				}
			}
		}
	}

	// Emboss
	else if (filterSet[1][2] == -1) {
		#pragma omp parallel for
		for (plane = 0; plane < 3; plane++) {
			for (h = 1; h < imageHeight; h++) {

				int row1 = h - 1;
				int row3 = h + 1;

				#pragma omp parallel for
				for (w = 1; w < imageWidth; w++) {

					int temp1 = 0, temp2 = 0, temp3 = 0;
					int col1 = w - 1;
					int col3 = w + 1;

					temp1 += input -> color[plane][row1][col1];
					temp2 += input -> color[plane][row1][w];
					temp3 += -(input -> color[plane][row3][w]);

					temp1 += input -> color[plane][h][col1];
					temp2 += input -> color[plane][h][w];
					temp3 += -(input -> color[plane][row3][w]);

					temp1 += input -> color[plane][row3][col1];
					temp2 += -(input -> color[plane][row3][w]);
					temp3 += -(input -> color[plane][row3][col3]);

					// no divisor
					temp1 += temp2 + temp3;

					if (temp1 > 255) {
						output -> color[plane][h][w] = 255;
						continue;
					}
					if (temp1 < 0) {
						output -> color[plane][h][w] = 0;
						continue;
					}
					output -> color[plane][h][w] = temp1;
				}
			}
		}
	}

	// Average
	else {
		#pragma omp parallel for
		for (plane = 0; plane < 3; plane++) {
			for (h = 1; h < imageHeight; h++) {

				int row1 = h - 1;
				int row3 = h + 1;

				#pragma omp parallel for
				for (int w = 1; w < imageWidth; w++) {

					int temp1 = 0, temp2 = 0, temp3 = 0;
					int col1 = w - 1;
					int col3 = w + 1;

					temp1 += input -> color[plane][row1][col1];
					temp2 += input -> color[plane][row1][w];
					temp3 += input -> color[plane][row1][col3];

					temp1 += input -> color[plane][h][col1];
					temp2 += input -> color[plane][h][w];
					temp3 += input -> color[plane][h][col3];

					temp1 += input -> color[plane][row3][col1];
					temp2 += input -> color[plane][row3][w];
					temp3 += input -> color[plane][row3][col3];

					// divisor
					temp1 = (temp1 + temp2 + temp3) / 9;

					if (temp1 > 255) {
						output -> color[plane][h][w] = 255;
						continue;
					}
					if (temp1 < 0) {
						output -> color[plane][h][w] = 0;
						continue;
					}
					output -> color[plane][h][w] = temp1;
				}
			}
		}
	}

	// original function, left here for reference
	// for (int col = 1; col < (input -> width) - 1; col = col + 1) {
	// 	for (int row = 1; row < (input -> height) - 1 ; row = row + 1) {
	// 		for (int plane = 0; plane < 3; plane++) {

	// 			output -> color[plane][row][col] = 0;

	// 			for (int j = 0; j < size; j++) {
	// 				for (int i = 0; i < size; i++) {
	// 					output -> color[plane][row][col]
	// 						= output -> color[plane][row][col]
	// 						  + (input -> color[plane][row + i - 1][col + j - 1]
	// 							 * filter -> get(i, j) );
	// 				}
	// 			}

	// 			output -> color[plane][row][col] =
	// 				output -> color[plane][row][col] / divisor;

	// 			if ( output -> color[plane][row][col]  < 0 ) {
	// 				output -> color[plane][row][col] = 0;
	// 			}

	// 			if ( output -> color[plane][row][col]  > 255 ) {
	// 				output -> color[plane][row][col] = 255;
	// 			}
	// 		}
	// 	}
	// }

	// original ending code
	cycStop = rdtscll();
	double diff = cycStop - cycStart;
	double diffPerPixel = diff / (output -> width * output -> height);
	fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
			diff, diff / (output -> width * output -> height));
	return diffPerPixel;
}
