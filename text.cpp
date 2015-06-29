	/*Kernel_x[3][3]=	{{-1, 0, 1}
					 {-2, 0, 2}
					 {-1, 0, 1}
					}
	  Kernel_y[3][3]=	{{ 1, 2, 1}
					 { 0, 0, 0}
					 {-1,-2,-1}
					}
					
	int MaskGaussian[3][3]={{1,2,1},
					    {2,4,2},
					    {1,2,1}};
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char BOOL;
typedef unsigned char BYTE;
#define true 1
#define false 0
#define PI 3.14159265

typedef struct{
	char M,N;
	int width;
	int height;
	BYTE* pixels;
}InputImage;

int main(int argc, char* argv[])
{
	FILE * fp;
	FILE * outfile;
	int i,j;
	const int Low_th=100;
	const int High_th=200;
	int Gray_edge=100;
	int Certain_edge=255;
	int Suppressed_edge=0;
	const int height=256;
	const int width=256;
	

	BYTE* OutImg;
	InputImage InImg;
	int * SobelX, *SobelY;
	double * Slope;
	int Where, vol1, vol2;  
	int mag_temp, direction;
	BOOL real_edge;

	if(argc==1){
		printf("please 'input image file name' and 'output image file name'!\n");
		return 0;
	}
	InImg.pixels = (BYTE*)calloc(height*width,sizeof(char));
	
	fopen_s(&fp,argv[1],"r");
	if(fp==NULL){
		printf("read file error\n");
		return 0;
	}
	
	fread(InImg.pixels,sizeof(BYTE),height*width,fp);
	fclose(fp);
	
	OutImg=(unsigned char*)calloc(height*width,sizeof(unsigned char));
	
	for(i=0;i<height*width;i++){
		OutImg[i]=0;
	}
	/****************************************Smoothing************************************************/
	for(i=1; i<height-1; i++){
		for(j=1; j<width-1; j++){
			Where=i*width+j;
			OutImg[Where]=(InImg.pixels[Where-width-1]*1+InImg.pixels[Where-width]*2+InImg.pixels[Where-width+1]*1+InImg.pixels[Where-1]*2+InImg.pixels[Where]*4+InImg.pixels[Where+1]*2+InImg.pixels[Where+width-1]*1+InImg.pixels[Where+width]*2+InImg.pixels[Where+width+1]*1)/16;
		}
	}

	/****************************************Finding gradient************************************************/
	
	unsigned char* Mag_table=(unsigned char*)calloc(height*width,sizeof(unsigned char));
	SobelX=(int*)calloc(height*width,sizeof(int));
	SobelY=(int*)calloc(height*width,sizeof(int));
	Slope=(double*)calloc(height*width,sizeof(double));
	for(i=0; i<height*width; i++){
		SobelX[i]=0;
		SobelY[i]=0;
		Mag_table[i]=0;
	}

	for(i=1; i<height-1; i++){
		for(j=1; j<width-1; j++){
			Where=i*width+j;
			SobelX[Where]=((OutImg[Where-width-1]*(-1)+OutImg[Where-width+1]*1)+(OutImg[Where-1]*(-2)+OutImg[Where+1]*2)+(OutImg[Where+width-1]*(-1)+OutImg[Where+width-1]*1));
			SobelY[Where]=((OutImg[Where-width-1]*1+OutImg[Where-width]*2+OutImg[Where-width+1]*1)+(OutImg[Where+width-1]*(-1)+OutImg[Where+width]*(-2)+OutImg[Where+width+1]*(-1)));
			Slope[Where]=(double)atan2((double)SobelY[Where],(double)SobelX[Where]);
			Mag_table[Where]=abs(SobelX[Where])+abs(SobelY[Where]);
		}
	}
	for(i=1; i<height-1; i++){
		for(j=1; j<width-1; j++){
			Where = i*width+j;
			Slope[Where]=Slope[Where]*180/PI;
			if(Slope[Where]<0){
				Slope[Where]=-Slope[Where];
			}
			if(Slope[Where]<22.5)
				Slope[Where]=0;
			else if(Slope[Where]<67.5)
				Slope[Where]=1;
			else if(Slope[Where]<112.5)
				Slope[Where]=2;
			else if(Slope[Where]<157.5)
				Slope[Where]=3;
			else
				Slope[Where]=0;
		}
	}										//direction-lize!

	/*	Sobel mask is implemented
		We can use Slope!
		
	/****************************************Non_maximum surppression<unit direction******************************************/
	for(i=1; i<height-1; i++){
		for(j=1; j<width-1; j++){
			Where=i*width+j;
			if(Slope[Where]==0){
				if(OutImg[Where]<OutImg[Where+1]||OutImg[Where]<OutImg[Where-1])
					OutImg[Where]=0;
			}
			else if(Slope[Where]==1){
				if(OutImg[Where]<OutImg[Where-width+1]||OutImg[Where]<OutImg[Where+width-1])
					OutImg[Where]=0;
			}
			else if(Slope[Where]==2){
				if(OutImg[Where]<OutImg[Where+width]||OutImg[Where]<OutImg[Where-width])
					OutImg[Where]=0;
			}
			else if(Slope[Where]==3){
				if(OutImg[Where]<OutImg[Where-width-1]||OutImg[Where]<OutImg[Where+width+1])
					OutImg[Where]=0;
			}
		}
	}
	/****************************************Double Threshold<unit magnitude>******************************************/
	
	for(i=1; i<height-1; i++){
		for(j=1; j<width-1; j++){
			Where=i*width+j;
			if(Mag_table[Where]>High_th)
				OutImg[Where]=Certain_edge;
			else if(Mag_table[Where]>Low_th)
				OutImg[Where]=Gray_edge;
			else
				OutImg[Where]=Suppressed_edge;
			
		}
	}
	
	/****************************************Edge tracking by htsteresis******************************************/
	for(i=1; i<height-1; i++){
		for(j=1; j<width-1; j++){
			Where=i*width+j;
			if(OutImg[Where]==Gray_edge){
				if(OutImg[Where-width-1]==255||OutImg[Where-width]==255||OutImg[Where-width+1]==255||OutImg[Where-1]==255||OutImg[Where+1]==255||OutImg[Where+width-1]==255||OutImg[Where+width]==255||OutImg[Where+width+1]==255)
					OutImg[Where]=Certain_edge;
				else
					OutImg[Where]=Suppressed_edge;
			}
		}
	}


	outfile = fopen(argv[2],"wb");
	fwrite(OutImg,sizeof(char),height*width,outfile);
	fclose(outfile);

	free(Mag_table);
	free(SobelX);
	free(SobelY);
	free(Slope);		
	free(OutImg);

	return 0;
}
