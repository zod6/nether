#ifdef _WIN32
#include "windows.h"
#endif

#include "string.h"
#include "stdio.h"
#include "math.h"

#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glut.h"
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
#ifdef _WIN32
#include <SDL_opengl.h>
#endif

#include "List.h"
#include "vector.h"
#include "cmc.h"
#include "3dobject.h"
#include "shadow3dobject.h"
#include "piece3dobject.h"
#include "myglutaux.h"
#include "matrix.h"
#include "nether.h"

#include "glprintf.h"


extern int frames_per_sec;
extern int shadows;
extern bool sound;
extern int up_key,down_key,left_key,right_key,fire_key;
extern int SCREEN_X, SCREEN_Y;
extern bool mouse_left_pressed;


void NETHER::construction_draw()
{
	float lightpos2[4]={0,0,1000,1};
	float tmpls[4]={1.0F,1.0F,1.0F,1.0};
	float tmpld[4]={0.6F,0.6F,0.6F,1.0};
	float tmpla[4]={0.1F,0.1F,0.1F,1.0};

	/* Enable Lights, etc.: */ 
//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0,GL_AMBIENT,tmpla);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,tmpld);
	glLightfv(GL_LIGHT0,GL_SPECULAR,tmpls);
	glLightfv(GL_LIGHT0,GL_POSITION,lightpos2);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel( GL_SMOOTH );
	glCullFace( GL_BACK );
	glFrontFace( GL_CCW );
    glEnable( GL_CULL_FACE );
	glDisable( GL_SCISSOR_TEST );  
	glEnable( GL_DEPTH_TEST );
	glDisable(GL_BLEND);
	
	/* Draw the CONSTRUCTION screen: */ 
    glClearColor(0,0,0.0,0);
    glViewport(0,0,SCREEN_X,SCREEN_Y);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
//    gluPerspective( 30.0, float(SCREEN_X)/float(SCREEN_Y), 1.0, 1024.0 );
	glFrustum( -0.26*float(SCREEN_X)/float(SCREEN_Y), 0.26*float(SCREEN_X)/float(SCREEN_Y), -0.26, 0.26, 1, 1024 );

    /* Clear the color and depth buffers. */ 
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	gluLookAt(0,0,64,0,0,0,0,1,0);

	glPushMatrix();
	glColor3f(1.0f,1.0f,0.0);
	glTranslatef(-12,5,0);
	scaledglprintf(0.01f,0.01f,"-- RESOURCES --");
	glTranslatef(0,-1.4,0);
	scaledglprintf(0.01f,0.01f,"-- AVAILABLE --");

	{
		int i,total=0;
		int res[7];

		RobotCost(0,&in_construction,res);
		for(i=0;i<7;i++) res[i]=resources[0][i]-res[i];

		glColor3f(0.5f,1.0f,0.5f);
		glTranslatef(0,-2.4,0);
		scaledglprintf(0.01f,0.01f,"    GENERAL %.2i",res[0]);
		glTranslatef(0,-2.4,0);
		scaledglprintf(0.01f,0.01f,"ELECTRONICS %.2i",res[1]);
		glTranslatef(0,-1.4,0);
		scaledglprintf(0.01f,0.01f,"    NUCLEAR %.2i",res[2]);
		glTranslatef(0,-1.4,0);
		scaledglprintf(0.01f,0.01f,"    PHASERS %.2i",res[3]);
		glTranslatef(0,-1.4,0);
		scaledglprintf(0.01f,0.01f,"   MISSILES %.2i",res[4]);
		glTranslatef(0,-1.4,0);
		scaledglprintf(0.01f,0.01f,"     CANNON %.2i",res[5]);
		glTranslatef(0,-1.4,0);
		scaledglprintf(0.01f,0.01f,"    CHASSIS %.2i",res[6]);

		for(i=0;i<7;i++) total+=res[i];

		glTranslatef(0,-2.4,0);
		scaledglprintf(0.01f,0.01f,"      TOTAL %.2i",total);
	}

	glColor3f(0.3f,0.8f,1.0f);
	glTranslatef(0,-3,0);
	scaledglprintf(0.01f,0.01f,"    EXIT  START");
	glTranslatef(0,-1.4,0);
	scaledglprintf(0.01f,0.01f,"    MENU  ROBOT");
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-10,12,0);
	{
		float angle=float(sin(animation_timer/2)*10);
		float angle2=float(sin(animation_timer/4)*15);
		glRotatef(angle2-10,1,0,0);
		glRotatef(angle,0,1,0);
		animation_timer+=0.02;
	}
	construction_tile[0]->draw(0.5f,0.5f,0.5f);
	glTranslatef(0.0,-1.1,1);
	construction_tile[1]->draw(0.8f,0.8f,0.8f);
	glTranslatef(0.0,2.2,0);
	construction_tile[2]->draw(0.8f,0.8f,0.8f);
	glPopMatrix();

	glPushMatrix();
	glColor3f(1.0f,0.0f,0.0f);
	glTranslatef(12,15,0);
	in_construction.cmc=RobotCMC(&in_construction,0);
	if (RobotCollision(&in_construction,true)) {
		if ((int(animation_timer*4)%2)==0) scaledglprintf(0.01f,0.01f,"ENTRANCE BLOCKED!");
	} /* if */ 

	glColor3f(0.3f,0.8f,1.0f);
	glTranslatef(3,-3,0);
	scaledglprintf(0.01f,0.01f,"ELECTRONICS");
	glTranslatef(0,-3.5,0);
	scaledglprintf(0.01f,0.01f,"NUCLEAR");
	glTranslatef(0,-3.5,0);
	scaledglprintf(0.01f,0.01f,"PHASERS");
	glTranslatef(0,-3.5,0);
	scaledglprintf(0.01f,0.01f,"MISSILES");
	glTranslatef(0,-3.5,0);
	scaledglprintf(0.01f,0.01f,"CANNON");
	glTranslatef(0,-3.5,0);
	scaledglprintf(0.01f,0.01f,"ANTI-GRAV");
	glTranslatef(0,-3.5,0);
	scaledglprintf(0.01f,0.01f,"TRACKS");
	glTranslatef(0,-3.5,0);
	scaledglprintf(0.01f,0.01f,"BIPOD");
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.8f,0.3f,1.0f);
	glTranslatef(15,10.6,0);
	scaledglprintf(0.01f,0.01f,"3");
	glTranslatef(0,-3.5,0);
	scaledglprintf(0.01f,0.01f,"20");
	glTranslatef(0,-3.5,0);
	scaledglprintf(0.01f,0.01f,"4");
	glTranslatef(0,-3.5,0);
	scaledglprintf(0.01f,0.01f,"4");
	glTranslatef(0,-3.5,0);
	scaledglprintf(0.01f,0.01f,"2");
	glTranslatef(0,-3.5,0);
	scaledglprintf(0.01f,0.01f,"10");
	glTranslatef(0,-3.5,0);
	scaledglprintf(0.01f,0.01f,"5");
	glTranslatef(0,-3.5,0);
	scaledglprintf(0.01f,0.01f,"3");
	glPopMatrix();

	{
		glEnable(GL_RESCALE_NORMAL);
		int i;
		for(i=0;i<8;i++) {
			glPushMatrix();
			glTranslatef(6,-(13.5-i*3.5),0);
			glScalef(2.4,2.4,2.4);
			glRotatef(30,1,0,0);
			glRotatef(animation_timer*32,0,1,0);
			glRotatef(-90,1,0,0);
			if (construction[i]) piece_tile[0][i]->draw_notexture(1.0,1.0,1.0);
							else piece_tile[0][i]->draw_notexture(0.5,0.5,0.5);
			glPopMatrix();
		} /* for */ 

		glPushMatrix();
		glTranslatef(0,-8,0);
		glScalef(4.0f,4.0f,4.0f);
		glRotatef(30,1,0,0);
		glRotatef(animation_timer*32,0,1,0);
		glRotatef(-90,1,0,0);
		DrawRobot(&in_construction,0,false);
		glPopMatrix();
		glDisable(GL_RESCALE_NORMAL);
	}

	{
		STATUSBUTTON *b;

		glPushMatrix();
		glColor3f(0.75f,0.0,0.0);

		if((b=getbutton(construction_pointer))!=0){
			glTranslatef(b->x,b->y,-1);
			glutSolidBox(b->sx,b->sy,0.5);
		}
		glPopMatrix();
	}

} /* NETHER::construction_draw */ 


void NETHER::construction_action(int key, int mx, int my)
{
	if (mx>0) {
		List<STATUSBUTTON> l;
		STATUSBUTTON *b;
		int found=0;
		float x, y, z;
		float view_matrix[16];
		float projection_matrix[16];
		float ratio=float(SCREEN_X)/float(SCREEN_Y);
		int viewport[4];
		viewport[0]=viewport[1]=0;
		viewport[2]=SCREEN_X;
		viewport[3]=SCREEN_Y;

		// gluLookAt(0,0,64,0,0,0,0,1,0);
		Vector pos(0,0,64);
		Vector target(0,0,0);
		Vector up(0,1,0);
		set_view_matrix(pos, target, up, view_matrix);

		set_projection( -0.26*ratio, 0.26*ratio, -0.26, 0.26, 1, 1024, projection_matrix);

		// winZ = ((z*(f+n)/(n-f) + 2fn/(n-f))/(-z) + 1)/2 // z is negative // why to keep this winZ = (q.z+1)/2 here and in gluunproject? 
		// z=((-65.5*projection_matrix[10]+projection_matrix[14])/65.5 + 1) / 2; // =0.9856954176f
		if(!unproject(mx, my, 0.9856954176f, view_matrix, projection_matrix, viewport, &x, &y, &z)) return;
//		printf("%d, %d (%f, %f)\n", mx, my, x, y);

        l.Instance(buttons);
        l.Rewind();
        while(l.Iterate(b)) {
            if (x > (b->x - b->sx) && x < (b->x + b->sx) && y > (b->y - b->sy) && y < (b->y + b->sy)){
                construction_pointer=b->ID;
                found=1;
                break;
            }
        }
        if (!found) return;
	}

	if (construction_pointer==10 && key==right_key) construction_pointer=20;
	if (construction_pointer==0 && key==right_key) construction_pointer=10;

	if (construction_pointer==10 && key==left_key) construction_pointer=0;
	if (construction_pointer>=20 && key==left_key) construction_pointer=10;

	if (construction_pointer>=20 && construction_pointer<27 && key==up_key) construction_pointer++;
	if (construction_pointer>20 && construction_pointer<=27 && key==down_key) construction_pointer--;

	if (construction_pointer>=20 && key==fire_key) {
		int i;
		int cost[7];
		bool tmp[8];
		ROBOT r_tmp;
		bool enoughresources=true;

		r_tmp=in_construction;
		for(i=0;i<8;i++) tmp[i]=construction[i];

		if (construction[construction_pointer-20]) {
			if (construction_pointer<=22) in_construction.traction=-1;
			if (construction_pointer>=23) in_construction.pieces[construction_pointer-23]=false;
			construction[construction_pointer-20]=false;
		} else {
			if (construction_pointer<=22) {
				in_construction.traction=construction_pointer-20;
				construction[0]=false;
				construction[1]=false;
				construction[2]=false;
			} /* if */ 
			if (construction_pointer>=23) in_construction.pieces[construction_pointer-23]=true;
			construction[construction_pointer-20]=true;
		} /* if */ 

		RobotCost(0,&in_construction,cost);
		enoughresources=true;
		for(i=0;i<7;i++) {
			if (resources[0][i]<cost[i]) {
				int j;
				/* Not enough resources! */ 
				for(j=0;j<8;j++) {
					in_construction=r_tmp;
					construction[j]=tmp[j];
				} /* for */ 
				enoughresources=false;
			} /* if */ 
		} /* for */ 

		if (enoughresources) {
			if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
		} else {
			if (S_wrong!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_wrong,0),128);
		} /* if */ 
	} /* if */ 

	if (construction_pointer==0 && key==fire_key) {
		game_state=STATE_PLAYING;
		shipp.z=2.0;
		flushbuttons();
		newmenu(GENERAL_MENU);
		animation_timer=0;
	}

	if (construction_pointer==10 && key==fire_key) {
		if (in_construction.valid()) {
			int i;
			int cost[7];

			/* Valid robot, build it: */ 
			ROBOT *r;
			r=new ROBOT();
			*r=in_construction;
			r->angle=0;
			r->program=PROGRAM_FORWARD;
			r->program_parameter=9;
			r->op=ROBOTOP_NONE;
			r->cmc=RobotCMC(r,0);
			r->shipover=false;

			if (!RobotCollision(r,true)) {
				robots[0].Add(r);
				AI_newrobot(r->pos,0);

				RobotCost(0,&in_construction,cost);
				for(i=0;i<7;i++) resources[0][i]-=cost[i];

				game_state=STATE_PLAYING;
				shipp.z=2.0;
				flushbuttons();
				newmenu(GENERAL_MENU);
				animation_timer=0;
				if(mx>0) mouse_left_pressed=false;
				if (S_construction!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_construction,0),128);
			} else {
				/* The factory entrance is blocked: */ 
				delete r;
			} /* if */ 
		} else {
			/* Wrong robot: */ 
			if (S_wrong!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_wrong,0),128);
		} /* if */ 
	} /* if */ 

	redrawmenu=2;
	redrawradar=1;
} /* NETHER::construction_action */ 

void NETHER::new_construction_buttons(){
	// z=-1, thickness=0.5
	newbutton(0,   -14, -14, 3,  2, "EXIT MENU",0, 0,0,0);
	newbutton(10,   -7, -14, 3,  2, "START ROBOT",0, 0,0,0);
	newbutton(20, 15.3, -13, 6,1.7, "BIPOD",0, 0,0,0);
	newbutton(21, 15.3,-9.2, 6,1.7, "TRACKS",0, 0,0,0);
	newbutton(22, 15.3,-5.7, 6,1.7, "ANTI-GRAV",0, 0,0,0);
	newbutton(23, 15.3,-2.2, 6,1.7, "CANNON",0, 0,0,0);
	newbutton(24, 15.3, 1.2, 6,1.7, "MISSILES",0, 0,0,0);
	newbutton(25, 15.3, 4.7, 6,1.7, "PHASERS",0, 0,0,0);
	newbutton(26, 15.3, 8.2, 6,1.7, "NUCLEAR",0, 0,0,0);
	newbutton(27, 15.3,  12, 6,1.7, "ELECTRONICS",0, 0,0,0);
}

