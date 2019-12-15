//g++ -g -Wall Game.cpp `pkg-config opencv --cflags --libs`

/*
###Ideas###
Obstaculos

*/
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv/cv.hpp"
#include "defs.h"

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;
using namespace cv;


class elem{
	public:
		int countdown,mode,vel;
		int foiPego,pegou;
		Scalar color;
		Point center;
		float angle;
		
		float pesos[2][2][NUM_VARS];
		int aval[2];

		void switchMode(){
			if(mode){
				countdown=0;
				pegou++;
				mode=0;
			}else{
				countdown=50;
				foiPego++;
				mode=1;
			}
			color=Scalar(50,250-(200*mode),50+(200*mode));
		}

		void move(){
			if(countdown){
				countdown--;
				if(mode){
					return;
				}
			}

			angle+=PI;
			angle=fmod(angle,2*PI);
			angle-=PI;

			center.x+=vel*(cos(angle));
			center.y+=vel*(sin(angle));
			
			aval[mode]++;

			if(center.x>POS_RATIO*WIDTH){
				angle=(PI-angle);
				if(rand()%2){
					angle+=0.05;
				}else{
					angle-=0.05;
				}
				center.x=POS_RATIO*WIDTH-10;
			}else if(center.x<0){
				angle=(PI-angle);
				if(rand()%2){
					angle+=0.05;
				}else{
					angle-=0.05;
				}
				center.x=10;
			}

			if(center.y>POS_RATIO*HEIGHT){
				angle*=-1;
				if(rand()%2){
					angle+=0.05;
				}else{
					angle-=0.05;
				}
				center.y=POS_RATIO*HEIGHT-10;
			}else if(center.y<0){
				angle*=-1;
				if(rand()%2){
					angle+=0.05;
				}else{
					angle-=0.05;
				}
				center.y=10;
			}

		}


		elem(){
			center=Point(rand()%(POS_RATIO*WIDTH),rand()%(POS_RATIO*HEIGHT));

			color=Scalar(50,250,50);

			angle=FLOAT_RAND*(2*PI);
			vel=1+(rand()%4);
			
			
			for(int i=0; i<2; i++){
				for(int j=0; j<2; j++){
					for(int k=0; k<NUM_VARS; k++){
						pesos[i][j][k]=FLOAT_RAND;
						pesos[i][j][k]*=(2*RANGE);
						pesos[i][j][k]-=RANGE;
					}
				}
			}

			countdown=0;
			aval[0]=0;
			aval[1]=0;
			foiPego=0;
			pegou=0;		
			mode=0;
		}

		~elem(){
			/*for (i=0;i<2;i++){
				freePesos(pesosV_lin[i]);
				freePesos(pesosV_ang[i]);
			}
			freePesos(pesosV_shout);*/
		}

};


void angDist(Point center,float angle, Point pt, int mode, float res[2]){
	res[0] = angle;
	res[0]-= (float)atan2((center.y-pt.y),(center.x-pt.x));
	
	res[0]+= PI;

	res[0]+= PI;
	res[0]=fmod(res[0],2*PI);
	res[0]-= PI;

	res[1] = sqrt((center.x-pt.x)*(center.x-pt.x) + (center.y-pt.y)*(center.y-pt.y));
}

void calcMove(vector<elem*> marco, vector<elem*> polo, int atual, int mode){
	float (*pesos)[NUM_VARS];
	float calcs[2];
	float res[2];
	elem *curr;
	float tmp;
	int i,j;

	if(mode){
		curr=marco[atual];
	}else{
		curr=polo[atual];
	}
	pesos=curr->pesos[mode];


	for(i=0; i<2; i++){
		res[i]=0;

		res[i]+=(pesos[i][0]*(curr->center.x-(POS_RATIO*WIDTH/2)))/((float)200*POS_RATIO*WIDTH);
		res[i]+=(pesos[i][1]*(curr->center.y-(POS_RATIO*HEIGHT/2)))/((float)200*POS_RATIO*HEIGHT);
		
		for(j=0; j<NUM_MARCO; j++){
			if(mode==0 || j!=atual){
				angDist(curr->center,curr->angle,marco[j]->center,mode,calcs);
				tmp=(pesos[i][2]*calcs[0])/(pesos[i][3]*(calcs[1]/2));
				if(!isnan(tmp)){
					res[i]+=tmp;
				}
			}
		}
		
		for(j=0; j<NUM_POLO; j++){
			if(mode || j!=atual){
				angDist(curr->center,curr->angle,polo[j]->center,mode,calcs);
				tmp=(pesos[i][4]*calcs[0])/(pesos[i][5]*(calcs[1]/2));
				if(!isnan(tmp)){
					res[i]+=tmp;
				}
			}
		}
	}

	res[0]=fmod(res[0],VEL_COEF);
	res[0]+=VEL_COEF;
	res[0]/=VEL_COEF;
	res[0]*=2*(POS_RATIO+mode);

	res[1]*=3;
	res[1]=fmod(res[1],0.1);

	if(isnan(res[1])){
		res[1]=0;
	}
	
	curr->vel=(int)res[0];
	curr->angle+=res[1];
}

float aval(elem *e, int mode){
	float nota;

	if(mode){
		nota=e->pegou;
		nota*=1000;
		nota/=(1+e->aval[1]);
	}else{
		nota=e->aval[0];
		nota/=(e->foiPego+1);
		nota/=(TIME_ROUND/1000);
	}

	return nota;
}

elem* torneio(vector<elem*> pop, int mode){
		int tmp_1=rand()%NUM_POP;
		int tmp_2=rand()%NUM_POP;
		if((tmp_1!=tmp_2) && (aval(pop[tmp_1],mode) > aval(pop[tmp_2],mode)) && (mode==0 || pop[tmp_1]->pegou)){
			return pop[tmp_1];
		}else{
			return pop[tmp_2];
		}
}

void nextGen(vector<elem*> &pop, elem* best[2]){
	float newPop[2][NUM_POP][2][NUM_VARS];
	elem *tmp;
	int i,j,k;

	for(i=0; i<2; i++){
		for(j=0;j<NUM_POP;j++){
			for(k=0;k<NUM_VARS;k++){
				newPop[i][j][0][k]=0;
				newPop[i][j][1][k]=0;
				for(int count=0;count<NUM_PAIS;count++){
					tmp=torneio(pop,i);
					newPop[i][j][0][k]+=tmp->pesos[i][0][k];
					newPop[i][j][1][k]+=tmp->pesos[i][1][k];			
				}
				newPop[i][j][0][k]/=NUM_PAIS;
				newPop[i][j][1][k]/=NUM_PAIS;
			}
		}
	}
	for(i=0;i<NUM_POP;i++){
		for(j=0; j<2; j++){
			if(pop[i] != best[j]){
				for(k=0;k<NUM_VARS;k++){
					pop[i]->pesos[j][0][k]=newPop[j][i][0][k];
					pop[i]->pesos[j][1][k]=newPop[j][i][1][k];
				}
			}
		}
		pop[i]->aval[0]=0;
		pop[i]->aval[1]=0;
		pop[i]->foiPego=0;
		pop[i]->pegou=0;
	}
}

void mutate(vector<elem*> &pop, float str, elem* best[2]){
	int i,j,k;
	for(i=0; i<NUM_POP; i++){
		for(j=0;j<2;j++){
			if(pop[i]!=best[j]){
				for(k=0;k<NUM_VARS;k++){
					if(rand()%2){
						if(rand()%2){
							pop[i]->pesos[j][0][k] += MUT*str;
						}else{
							pop[i]->pesos[j][0][k] -= MUT*str;
						}
					}
					
					if(rand()%2){
						if(rand()%2){
							pop[i]->pesos[j][1][k] += MUT*str;
						}else{
							pop[i]->pesos[j][1][k] -= MUT*str;
						}
					}

					if(pop[i]->pesos[j][0][k]>RANGE){
						pop[i]->pesos[j][0][k]=RANGE;
					}else if(pop[i]->pesos[j][0][k]<-RANGE){
						pop[i]->pesos[j][0][k]=-RANGE;
					}
					if(pop[i]->pesos[j][1][k]>RANGE){
						pop[i]->pesos[j][1][k]=RANGE;
					}else if(pop[i]->pesos[j][1][k]<-RANGE){
						pop[i]->pesos[j][1][k]=-RANGE;
					}	
				}
			}
		}
	}
}

void predate(vector<elem*> &pop, elem* best[2]){
	int removed,mode,pos,i;
	float min;

	removed=-1;
	for(mode=0; mode<2; mode++){
		min=aval(pop[0],mode);
		pos=0;
		for(i=2;i<NUM_POP;i++){
			if(aval(pop[i],mode) < min && i!=removed && pop[i] != best[mode]){
				min=aval(pop[i],mode);
				pos=i;
			}
		}
		delete pop[pos];
		pop[pos]=new elem();
		pos=removed;
	}
}

int main(){
	Mat base(HEIGHT, WIDTH, CV_8UC3, Scalar(31, 27, 21));
	Mat img(HEIGHT, WIDTH, CV_8UC3, Scalar(31, 27, 21));
	fstream marcoParam[2],poloParam[2];

	srand(time(NULL));
	elem *best[2];
	elem *tmp;
	
	vector<elem*> marco;
	vector<elem*> polo;
	vector<elem*> pop;
	
	long int timer,ger;
	int str,i,j;

	for(i=0;i<NUM_MARCO;i++){
		marco.push_back(new elem());
		pop.push_back(marco[i]);
		marco[i]->switchMode();
	}

	for(i=0;i<NUM_POLO;i++){
		polo.push_back(new elem());
		pop.push_back(polo[i]);
	}

	str=1;
	
	cout.flush();


	
	for(ger=0;ger<=NUM_CYCLES;ger++){
		if((ger%TIME_LAPSE==0*TIME_LAPSE && ger>TIME_DELAY)){
			namedWindow("Field", CV_WINDOW_AUTOSIZE);
		}

		for(timer=0;(timer<TIME_ROUND)&&(ger%TIME_LAPSE!=0*TIME_LAPSE || ger<TIME_DELAY || timer<TIME_ROUND/4);timer++){
			if((ger%TIME_LAPSE==0*TIME_LAPSE && ger>TIME_DELAY)){
				img = base.clone();
				cout << ger << ") " << timer << "\n";
			}

			for(i=0; i<NUM_POLO; i++){
				calcMove(marco,polo,i,0);
			}
			for(i=0; i<NUM_MARCO; i++){
				calcMove(marco,polo,i,1);
			}

			for(i=0;i<NUM_MARCO;i++){
				marco[i]->move();
				if((ger%TIME_LAPSE==0*TIME_LAPSE && ger>TIME_DELAY)){
					circle(img, marco[i]->center, POS_RATIO*3, marco[i]->color, -1, CV_AA, POS_EXP);
				}
			}

			//catch check
			for(i=0;i<NUM_MARCO;i++){
				if(marco[i]->countdown==0){
					for(j=0;j<NUM_POLO;j++){
						if(norm(marco[i]->center-polo[j]->center)<POS_RATIO*4){
							tmp=marco[i];
							marco[i]=polo[j];
							polo[j]=tmp;

							marco[i]->switchMode();
							polo[j]->switchMode();
							break;
						}
					}
				}
			}


			for(i=0;i<NUM_POLO;i++){
				polo[i]->move();
				if((ger%TIME_LAPSE==0*TIME_LAPSE && ger>TIME_DELAY)){
					circle(img, polo[i]->center, POS_RATIO*3, polo[i]->color, -1, CV_AA, POS_EXP);
				}
			}
			if((ger%TIME_LAPSE==0*TIME_LAPSE && ger>TIME_DELAY)){
				imshow("Field", img);
				waitKey(16);
			}

		}
		destroyWindow("Field");

		for(j=0; j<2; j++){
			best[j]=pop[0];
			for(i=0; i<NUM_POP; i++){
				if(aval(pop[i],j)>aval(best[j],j) && (j==0 || best[j]->pegou)){
					best[j]=pop[i];
				}
			}
		}
		cout << "\n" << ger << ") " << "Best: " << aval(best[0],0) << " " << aval(best[1],1);

		if(ger%10==0){
			marcoParam[0].open("MarcoVelParams.txt",std::ios::app);
			marcoParam[1].open("MarcoAngParams.txt",std::ios::app);
			poloParam[0].open("PoloVelParams.txt",std::ios::app);
			poloParam[1].open("PoloAngParams.txt",std::ios::app);
			
			for(i=0;i<2;i++){
				marcoParam[i] << (ger/10);
				poloParam[i] << (ger/10);
				for(j=0;j<NUM_VARS;j++){
					marcoParam[i] << " " << best[1]->pesos[1][i][j];
					poloParam[i] << " " << best[0]->pesos[0][i][j];
				}
				marcoParam[i] << "\n";
				poloParam[i] << "\n";
				marcoParam[i].close();
				poloParam[i].close();
			}
		}
		
		nextGen(pop,best);
		mutate(pop,str,best);
		if(ger%16==0){
			predate(pop,best);
		}


		/*for (int i=0;i<NUM_VARS;i++){
			if(pop[0][i]-prvBest[i]>0.1){
				cout << "Slowing down mutation\n\n";
				mutCount=-1;
				str=1;
				break;
			}
		}
		mutCount++;
		if(mutCount==5){
			cout << "Raising speed to " << str << "\n\n";
			mutCount=0;
			str++;
		}*/

		for(i=0;i<NUM_MARCO;i++){
			marco.pop_back();
		}
		for(i=0;i<NUM_POLO;i++){
			polo.pop_back();
		}

		for(i=0;i<NUM_POP;i++){
			if(i<NUM_MARCO){
				if(pop[i]->mode){}else{
					pop[i]->switchMode();
				}
				marco.push_back(pop[i]);
			}else{
				if(pop[i]->mode){
					pop[i]->switchMode();
				}
				polo.push_back(pop[i]);
			}
			pop[i]->center=Point(rand()%(POS_RATIO*WIDTH),rand()%(POS_RATIO*HEIGHT));
		}
	}

	waitKey(1);
	return 0;
}