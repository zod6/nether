#ifdef _WIN32
#include "windows.h"
#else
#include <stddef.h>
#include <sys/types.h>
#include <dirent.h>
#endif

#include "string.h"
#include "stdio.h"
#include "math.h"
#include "ctype.h"

#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glut.h"
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"

#include "List.h"
#include "vector.h"
#include "cmc.h"
#include "3dobject.h"
#include "shadow3dobject.h"
#include "piece3dobject.h"
#include "myglutaux.h"

#include "glprintf.h"
#include "debug.h"
#include "filehandling.h"

#include "mainmenu.h"

#ifndef __WINDOWS__
char *strupr(char *in)
 {
    static char buffer[1024];
    char *c;

    for (c = buffer;*in != 0;c++, in++)
     {
	*c = toupper(*in);
     }
    *c = 0;

    return(buffer);
 }
#endif

extern debuglvl dbglvl;
extern int SCREEN_X;
extern int SCREEN_Y;
extern int COLOUR_DEPTH;
extern bool fullscreen;
extern int shadows;
extern int detaillevel;
extern bool sound;
extern int level;
extern int up_key,down_key,left_key,right_key,fire_key,pause_key;
extern float default_zoom;
extern bool show_fps;
extern bool cfg_mouse;
extern bool cfg_copypixels;


extern char mapname[128];
extern bool show_radar;
List<char> mapnames;

extern int mainmenu_status;
extern int mainmenu_substatus;
extern C3DObject *nethertittle;

void save_configuration(void);
void load_configuration(void);


// return 1 when init_video() needed
int mainmenu_cycle(int key, int mx, int my)
{
	if (mx>0) {
		float x_norm = ((float)mx+1) / SCREEN_X; // 0.00 - 1.00
		float y_norm = ((float)my+1) / SCREEN_Y;
		float ratio=float(SCREEN_Y)/float(SCREEN_X);

//		printf("%f, %f (%f: %f (%f), %f (%f))\n", x_norm, y_norm, ratio, x_norm/ratio, (1-x_norm)/ratio, y_norm/ratio, (1-y_norm)/ratio);

		if(mainmenu_status==9) key=SDLK_1; // help
		else if(x_norm < (-0.43*ratio+0.5) || x_norm > (1 - (-0.43*ratio+0.5))) return 0;
		
		float step=0.060818;
		if(mainmenu_status==1){ // main menu
			float start=0.615;
			if(y_norm > (start+0*step) && y_norm < (start+1*step)) key=SDLK_1;
			if(y_norm > (start+1*step) && y_norm < (start+2*step)) key=SDLK_2;
			if(y_norm > (start+2*step) && y_norm < (start+3*step)) key=SDLK_3;
			if(y_norm > (start+3*step) && y_norm < (start+4*step)) key=SDLK_4;
			if(y_norm > (start+4*step) && y_norm < (start+5*step)) key=SDLK_5;
			if(y_norm > (start+5*step) && y_norm < (start+6*step)) key=SDLK_0;
	} else if(mainmenu_status==3){ // options
			float start=0.140;
			if(y_norm > (start+0*step) && y_norm < (start+1*step)) key=SDLK_1;
			if(y_norm > (start+1*step) && y_norm < (start+2*step)) key=SDLK_2;
			if(y_norm > (start+2*step) && y_norm < (start+3*step)) key=SDLK_3;
			if(y_norm > (start+3*step) && y_norm < (start+4*step)) key=SDLK_4;
			if(y_norm > (start+4*step) && y_norm < (start+5*step)) key=SDLK_5;
			if(y_norm > (start+5*step) && y_norm < (start+6*step)) key=SDLK_6;
			if(y_norm > (start+6*step) && y_norm < (start+7*step)) key=SDLK_7;
			if(y_norm > (start+7*step) && y_norm < (start+8*step)) key=SDLK_8;
			if(y_norm > (start+8*step) && y_norm < (start+9*step)) key=SDLK_9;
			if(y_norm > (start+9*step) && y_norm < (start+10*step)) key=SDLK_m;
			if(y_norm > (start+10*step) && y_norm < (start+11*step)) key=SDLK_a;
			if(y_norm > (start+11*step) && y_norm < (start+12*step)) key=SDLK_0;
		}
	}

	if(!key) return 0; // probably mouse and outside a region

	switch(mainmenu_status) {
	case 1:
		if (key==SDLK_0) { // exit game
			mainmenu_status=5;
			mainmenu_substatus=0;
		}
		if (key==fire_key || key==SDLK_1) { // new game
			mainmenu_status=4;
			mainmenu_substatus=0;
		}
		if (key==SDLK_2) { // redef. keyboard
			mainmenu_status=6;
			mainmenu_substatus=0;
		}
		if (key==SDLK_3) { // options. animation first
			mainmenu_status=2;
			mainmenu_substatus=0;
		}
		if (key==SDLK_4) {
			/* Change the MAP: */ 
			if (mapnames.EmptyP()) {
				/* Fill the mapnames list: */ 
#ifdef _WIN32
				WIN32_FIND_DATA finfo;
				HANDLE h;

				h=FindFirstFile("maps/*.map",&finfo);
				if (h!=INVALID_HANDLE_VALUE) {
					if (strcmp(finfo.cFileName,".")!=0 &&
						strcmp(finfo.cFileName,"..")!=0) {
						char *name;
						name=new char[strlen(finfo.cFileName)+1];
						strcpy(name,finfo.cFileName);
						mapnames.Add(name);
					} /* if */ 

					while(FindNextFile(h,&finfo)==TRUE) {

						if (strcmp(finfo.cFileName,".")!=0 &&
							strcmp(finfo.cFileName,"..")!=0) {
							char *name;
							name=new char[strlen(finfo.cFileName)+1];
							strcpy(name,finfo.cFileName);
							mapnames.Add(name);
						} /* if */ 
					} /* while */ 
				} /* if */ 
				
#else
				DIR *dp;
				struct dirent *ep;
				  
				dp = opendir ("./maps/");
				if (dp != NULL)
				 {
				    while ( (ep=readdir(dp)) != NULL)
				     {
					if ((strstr(ep->d_name,".map") + 4) == ep->d_name + strlen(ep->d_name))
					 {
						char *name;
						name=new char[strlen(ep->d_name)+1];
						strcpy(name,ep->d_name);
						mapnames.Add(name);
					 }
				     }
				    (void) closedir (dp);
				 }
#endif
				/* Look for the actualmap: */ 
				mapnames.Rewind();
				while(!mapnames.EndP() && strcmp(mapnames.GetObj(),mapname)!=0) mapnames.Next();

			} /* if */ 

			if (!mapnames.EmptyP()) {
				mapnames.Next();
				if (mapnames.EndP()) mapnames.Rewind();
				strcpy(mapname,mapnames.GetObj());
			} /* if */ 

			save_configuration();
		} /* if */ 
		if (key==SDLK_5) { // help. animation first
			mainmenu_status=8;
			mainmenu_substatus=0;
		}
		break;
	case 3:
		if (key==SDLK_1) { /* change resolution */
			change_resolution();
			return 1;
		}
		if (key==SDLK_2) {
			switch(COLOUR_DEPTH) {
			case 8: COLOUR_DEPTH=16; break;
			case 16: COLOUR_DEPTH=24; break;
			case 24: COLOUR_DEPTH=32; break;
			default:
				COLOUR_DEPTH=8;
			} /* switch */ 
			return 1;
		}
		if (key==SDLK_3) {
			fullscreen = (fullscreen == false);
			return 1;
		}
		if (key==SDLK_4) {
			shadows++;
			if (shadows>=3) shadows=0;
			save_configuration();
		}
		if (key==SDLK_5) {
			detaillevel++;
			if (detaillevel>=5) detaillevel=0;
			save_configuration();
		}
		if (key==SDLK_6) {
			sound = (sound == false);
			save_configuration();
		}
		if (key==SDLK_7) {
			level++;
			if (level>=4) level=0;
			save_configuration();
		}
		if (key==SDLK_8) {
			show_radar = (show_radar == false);
			save_configuration();
		}
		if (key==SDLK_9) {
			show_fps = (show_fps == false);
			save_configuration();
		}
		if (key==SDLK_m) {
			cfg_mouse = (cfg_mouse == false);
			SDL_ShowCursor( (cfg_mouse?SDL_ENABLE:SDL_DISABLE) );
			save_configuration();
		}
		if (key==SDLK_a) {
			cfg_copypixels = (cfg_copypixels==false);
			save_configuration();
		}
		if (key==SDLK_0) { // back
			mainmenu_status=0;
			mainmenu_substatus=0;
		}
		break;
	case 7:
		switch(mainmenu_substatus) {
			case 0:up_key=key;
				   break;
			case 1:down_key=key;
				   break;
			case 2:left_key=key;
				   break;
			case 3:right_key=key;
				   break;
			case 4:fire_key=key;
				   break;
			case 5:pause_key=key;
				   break;
		} /* sritch */ 
		mainmenu_substatus++;
		if (mainmenu_substatus==7) {
			mainmenu_status=0;
			mainmenu_substatus=0;
			save_configuration();
		}
		break;
	case 9: // show help
		// wait for any keypress
		// return to main menu
		mainmenu_status=0;
		mainmenu_substatus=0;
		break;
	}

	return 0;
} /* mainmenu_cycle */ 


void mainmenu_draw()
{
	float lightpos2[4]={0,0,1000,0};
	float tmpls[4]={1.0F,1.0F,1.0F,1.0};
	float tmpld[4]={0.6F,0.6F,0.6F,1.0};
	float tmpla[4]={0.2F,0.2F,0.2F,1.0};
    float ratio;

	if (nethertittle==0) {
		nethertittle=new C3DObject("models/tittle.asc","textures/");
		nethertittle->normalize(7.0);
	} /* if */ 

	/* Enable Lights, etc.: */ 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0,GL_AMBIENT,tmpla);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,tmpld);
	glLightfv(GL_LIGHT0,GL_SPECULAR,tmpls);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel( GL_SMOOTH );
	glCullFace( GL_BACK );
	glFrontFace( GL_CCW );
    glEnable( GL_CULL_FACE );
	glDisable( GL_SCISSOR_TEST );  
	glEnable( GL_DEPTH_TEST );
	
	glLightfv(GL_LIGHT0,GL_POSITION,lightpos2);
    glClearColor(0,0,0,0.0);
    glViewport(0,0,SCREEN_X,SCREEN_Y);
	ratio=(float)SCREEN_X/float(SCREEN_Y);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    gluPerspective( 30.0, ratio, 1.0, 1024.0 );
	gluLookAt(0,0,30,0,0,0,0,1,0);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	switch(mainmenu_status) {
	case 0:
		// zoom in title
		glTranslatef(0,3,40-mainmenu_substatus/1000.0);
		nethertittle->draw(1.0,1.0,1.0);
		break;
	case 1:
		// main menu
		glTranslatef(0,3,0);
		glPushMatrix();
		glRotatef(sin(mainmenu_substatus*0.02)*5.0f,0,1,0);
		nethertittle->draw(1.0,1.0,1.0);
		glPopMatrix();
		glColor3f(0.5,0.5,1.0);
		glTranslatef(-6,-5.5,0);
		scaledglprintf2(0.005,0.005,"1 - START NEW GAME   ");
		glTranslatef(0,-1,0);
		scaledglprintf2(0.005,0.005,"2 - REDEFINE KEYBOARD");
		glTranslatef(0,-1,0);
		scaledglprintf2(0.005,0.005,"3 - OPTIONS          ");
		glTranslatef(0,-1,0);
		scaledglprintf2(0.005,0.005,"4 - MAP: %s",mapname);
		glTranslatef(0,-1,0);
		scaledglprintf2(0.005,0.005,"5 - HELP");
		glTranslatef(0,-1,0);
		scaledglprintf2(0.005,0.005,"0 - EXIT GAME        ");
		break;
	case 2:
	case 4:
	case 5:
	case 6:
	case 8:
		// zoom out title
		glTranslatef(0,3,mainmenu_substatus/1000.0);
		nethertittle->draw(1.0,1.0,1.0);
		break;
	case 3:
		// option menu
		glColor3f(0.5,0.5,1.0);
		glTranslatef(0,5.3,0);
		scaledglprintf(0.005,0.005,"1 - RESOLUTION: %4dx%-4d", SCREEN_X, SCREEN_Y);
		glTranslatef(0,-1,0);
		if (COLOUR_DEPTH== 8) scaledglprintf(0.005,0.005,"2 - COLOR DEPTH:  8bit   ");
		if (COLOUR_DEPTH==16) scaledglprintf(0.005,0.005,"2 - COLOR DEPTH: 16bit   ");
		if (COLOUR_DEPTH==24) scaledglprintf(0.005,0.005,"2 - COLOR DEPTH: 24bit   ");
		if (COLOUR_DEPTH==32) scaledglprintf(0.005,0.005,"2 - COLOR DEPTH: 32bit   ");
		glTranslatef(0,-1,0);
		if (fullscreen) scaledglprintf(0.005,0.005,"3 - FULLSCREEN           ");
		else scaledglprintf(0.005,0.005,"3 - WINDOWED             ");
		glTranslatef(0,-1,0);
		if (shadows==0) scaledglprintf(0.005,0.005,"4 - SHADOWS: OFF         ");
		if (shadows==1) scaledglprintf(0.005,0.005,"4 - SHADOWS: ON - DIAG   ");
		if (shadows==2) scaledglprintf(0.005,0.005,"4 - SHADOWS: ON - VERT   ");
		glTranslatef(0,-1,0);
		if (detaillevel==0) scaledglprintf(0.005,0.005,"5 - DETAIL: LOWEST       ");
		if (detaillevel==1) scaledglprintf(0.005,0.005,"5 - DETAIL: LOW          ");
		if (detaillevel==2) scaledglprintf(0.005,0.005,"5 - DETAIL: MEDIUM       ");
		if (detaillevel==3) scaledglprintf(0.005,0.005,"5 - DETAIL: HIGH         ");
		if (detaillevel==4) scaledglprintf(0.005,0.005,"5 - DETAIL: HIGHEST      ");
		glTranslatef(0,-1,0);
		if (sound) scaledglprintf(0.005,0.005,"6 - SOUND: ON            ");
		else scaledglprintf(0.005,0.005,"6 - SOUND: OFF           ");
		glTranslatef(0,-1,0);
		if (level==0) scaledglprintf(0.005,0.005,"7 - LEVEL: EASY          ");
		if (level==1) scaledglprintf(0.005,0.005,"7 - LEVEL: NORMAL        ");
		if (level==2) scaledglprintf(0.005,0.005,"7 - LEVEL: HARD          ");
		if (level==3) scaledglprintf(0.005,0.005,"7 - LEVEL: IMPOSSIBLE    ");
		glTranslatef(0,-1,0);
		if (show_radar) scaledglprintf(0.005,0.005,"8 - RADAR: ON            ");
		else scaledglprintf(0.005,0.005,"8 - RADAR: OFF           ");
		glTranslatef(0,-1,0);
		if (show_fps) scaledglprintf(0.005,0.005,"9 - SHOW FPS: ON         ");
		else scaledglprintf(0.005,0.005,"9 - SHOW FPS: OFF        ");
		glTranslatef(0,-1,0);
		if (cfg_mouse) scaledglprintf(0.005,0.005,"m - MOUSE: ON            ");
		else scaledglprintf(0.005,0.005,"m - MOUSE: OFF           ");
		glTranslatef(0,-1,0);
		if (cfg_copypixels) scaledglprintf(0.005,0.005,"a - ANTI-FLICKERING: ON  ");
		else scaledglprintf(0.005,0.005,"a - ANTI-FLICKERING: OFF ");
		glTranslatef(0,-1,0);
		scaledglprintf(0.005,0.005,"0 - BACK                 ");
		break;
	case 7:
		{
			char tmp[256];

			glColor3f(0.5,0.5,1.0);
			glTranslatef(0,5,0);
			scaledglprintf(0.005,0.005,"REDEFINE KEYBOARD");
			glTranslatef(0,-2,0);
			if (mainmenu_substatus!=0) glColor3f(0.5,0.5,1.0);
								  else glColor3f(1.0,0.0,0.0);
			sprintf(tmp,"PRESS A KEY FOR UP: %s",strupr(SDL_GetKeyName((SDLKey)up_key)));
			scaledglprintf(0.005,0.005,tmp);
			glTranslatef(0,-1,0);
			if (mainmenu_substatus!=1) glColor3f(0.5,0.5,1.0);
						 	 	  else glColor3f(1.0,0.0,0.0);
			sprintf(tmp,"PRESS A KEY FOR DOWN: %s",strupr(SDL_GetKeyName((SDLKey)down_key)));
			scaledglprintf(0.005,0.005,tmp);
			glTranslatef(0,-1,0);
			if (mainmenu_substatus!=2) glColor3f(0.5,0.5,1.0);
								  else glColor3f(1.0,0.0,0.0);
			sprintf(tmp,"PRESS A KEY FOR LEFT: %s",strupr(SDL_GetKeyName((SDLKey)left_key)));
			scaledglprintf(0.005,0.005,tmp);
			glTranslatef(0,-1,0);
			if (mainmenu_substatus!=3) glColor3f(0.5,0.5,1.0);
								  else glColor3f(1.0,0.0,0.0);
			sprintf(tmp,"PRESS A KEY FOR RIGHT: %s",strupr(SDL_GetKeyName((SDLKey)right_key)));
			scaledglprintf(0.005,0.005,tmp);
			glTranslatef(0,-1,0);
			if (mainmenu_substatus!=4) glColor3f(0.5,0.5,1.0);
								  else glColor3f(1.0,0.0,0.0);
			sprintf(tmp,"PRESS A KEY FOR FIRE: %s",strupr(SDL_GetKeyName((SDLKey)fire_key)));
			scaledglprintf(0.005,0.005,tmp);

			glTranslatef(0,-1,0);
			if (mainmenu_substatus!=5) glColor3f(0.5,0.5,1.0);
								  else glColor3f(1.0,0.0,0.0);
			sprintf(tmp,"PRESS A KEY FOR PAUSE/MENU: %s",strupr(SDL_GetKeyName((SDLKey)pause_key)));
			scaledglprintf(0.005,0.005,tmp);

			glColor3f(0.5,0.5,1.0);
			glTranslatef(0,-2,0);
			scaledglprintf(0.005,0.005,"PG.UP/PG.DOWN CHANGE THE ZOOM");

			if (mainmenu_substatus>5) {
				glColor3f(1,1,1);
				glTranslatef(0,-2,0);
				scaledglprintf(0.005,0.005,"PRESS ANY KEY TO RETURN TO MAIN MENU");
			} /* if */ 
		}
		break;
	case 9:
		glColor3f(0.5,0.5,1.0);
		glTranslatef(-10,7,0);
		scaledglprintf2(0.004,0.004,"Welcome to Nether Earth, the first ever realtime");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"strategy game. You are a warlord in control of");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"an indestructable commandship. With this ship");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"you can land on warbases (H) and build robots to");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"do your bidding. To build these robots you are");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"going to need resources. You get a daily amount");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"of general resources from your warbase, but you");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"can also let your robots capture factories from");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"which you gather specialized resources.");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"To command your robots, land on them like you");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"would on your warbase and either control them");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"directly or give them a task to complete.");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"Unfortunately you are not the only warlord here");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"and your goal is to rid this world of the others");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"Capture all their bases to win the game.");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"Use up, down, left, right and fire to control");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"your commandship. Page-up and -down to zoom in");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"and out. F12 is an emergency quit.");
		glTranslatef(0,-0.7,0);
		scaledglprintf2(0.004,0.004,"");
		glTranslatef(2.5,-1,0);
		scaledglprintf2(0.004,0.004,"Press any key to return to main menu");
		break;
	} /* switch */ 

	SDL_GL_SwapBuffers();
} /* NETHER::draw */ 



void load_configuration(void)
{
	int v;
	FILE *fp;

	fp=f1open("nether.cfg", "r", USERDATA);
	if (fp==0) return;

	if (2!=fscanf(fp,"%i %i",&SCREEN_X,&SCREEN_Y)) return;
	if (1!=fscanf(fp,"%i",&v)) return;
	if (v==0) fullscreen=true;
	else fullscreen=false;

	if (1!=fscanf(fp,"%i",&shadows)) return;
	if (1!=fscanf(fp,"%i",&detaillevel)) return;

	if (6!=fscanf(fp,"%i %i %i %i %i %i",&up_key,&down_key,&left_key,&right_key,&fire_key,&pause_key)) return;
	if (1!=fscanf(fp,"%i",&v)) return;
	if (v==0) sound=true;
	else sound=false;

	if (1!=fscanf(fp,"%i",&level)) return;

	if (1!=fscanf(fp,"%s",mapname)) return; 
	if (1!=fscanf(fp,"%i",(int*)&dbglvl)) return;

	if (1!=fscanf(fp,"%f",&default_zoom)) return;

	if (1!=fscanf(fp,"%i",&v)) return;
	if (v==0) show_fps=true;
	else show_fps=false;

	if (1!=fscanf(fp,"%i",&v)) return;
	if (v==0) cfg_mouse=true;
	else cfg_mouse=false;

	if (1!=fscanf(fp,"%i",&v)) return;
	if (v==0) cfg_copypixels=true;
	else cfg_copypixels=false;

	fclose(fp);
} /* load_configuration */ 


void save_configuration(void)
{
	FILE *fp;

	fp=f1open("nether.cfg", "w", USERDATA);
	if (fp==0) return;

	fprintf(fp,"%i %i\n",SCREEN_X,SCREEN_Y);
	fprintf(fp,"%i %i %i\n",(fullscreen ? 0 : 1),shadows,detaillevel);
	fprintf(fp,"%i %i %i %i %i %i\n",up_key,down_key,left_key,right_key,fire_key,pause_key);
	fprintf(fp,"%i\n",(sound ? 0 : 1));
	fprintf(fp,"%i\n",level);
	fprintf(fp,"%s\n",mapname);
	fprintf(fp,"%i\n",dbglvl);
	fprintf(fp,"%f\n",default_zoom);
	fprintf(fp,"%i\n",(show_fps ? 0 : 1));
	fprintf(fp,"%i\n",(cfg_mouse ? 0 : 1));
	fprintf(fp,"%i\n",(cfg_copypixels ? 0 : 1));

	fclose(fp);
} /* save_configuration */ 

int mainmenu_timers(){
	static int timer;

	if(mainmenu_status==1){
		mainmenu_substatus++; // speed depends on fps
		return 0;
	}

	// zoom in and zoom out timers
	if (mainmenu_status==0 || mainmenu_status==2 || mainmenu_status==4 || mainmenu_status==5 || mainmenu_status==6 || mainmenu_status==8) {

		if(mainmenu_substatus==0){
			timer=SDL_GetTicks();
			mainmenu_substatus++;
		} else {
			if((SDL_GetTicks()-timer)>0) mainmenu_substatus=40*(SDL_GetTicks()-timer);
		}
//		printf("%d\n", mainmenu_substatus);

		if (mainmenu_substatus>=40*1000) { // 1000ms
			mainmenu_substatus=0;
			switch(mainmenu_status){
				case 0: mainmenu_status=1; break;	// pause before main
				case 2: mainmenu_status=3; break;	// pause before options
				case 4: return 1;					// pause before new game
				case 5: return 2;					// pause before quit
				case 6: mainmenu_status=7; break;	// pause before redefine keys
				case 8: mainmenu_status=9; break;	// pause before help
			}
		}
	} // if zoomin/zoomout
	return 0;
}


