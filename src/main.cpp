#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#include <fcntl.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
#include <GL/glut.h>

#include "List.h"
#include "vector.h"

#include "cmc.h"
#include "3dobject.h"
#include "shadow3dobject.h"
#include "piece3dobject.h"
#include "nether.h"
#include "mainmenu.h"

#include "debug.h"

/*						GLOBAL VARIABLES INITIALIZATION:							*/ 

extern debuglvl dbglvl;

int SCREEN_X=640;
int SCREEN_Y=480;
int native_x=640;
int native_y=480;

int COLOUR_DEPTH=16;
int shadows=1;
int detaillevel=4;
bool sound=true;
int up_key=SDLK_UP,down_key=SDLK_DOWN,left_key=SDLK_LEFT,right_key=SDLK_RIGHT,fire_key=SDLK_SPACE,pause_key=SDLK_ESCAPE;
float default_zoom=1.6;
bool show_fps=false;
bool cfg_mouse=false;
bool cfg_copypixels=true;

bool up_key_pressed=false,down_key_pressed=false,left_key_pressed=false,right_key_pressed=false,fire_key_pressed=false,pause_key_pressed=false,zoomin_key_pressed=false,zoomout_key_pressed=false;
bool mouse_left_pressed=false, mouse_right_pressed=false;
int level=1;
int mainmenu_status=0;
int mainmenu_substatus=0;
bool fullscreen=false;
bool show_radar=true;
char mapname[128]="original.map";
C3DObject *nethertittle=0;
char message[21];
int message_timer=0;

/* DRAWING REGION AROUND THE SHIP: */ 
float MINY=-8,MAXY=8,MINX=-8,MAXX=8;

/* Redrawing constant: */ 
const int REDRAWING_PERIOD=20; // 20ms, 50fps
int time_diff=20; // ellapsed time in ms. ideally same as REDRAWING_PERIOD

/* Frames per second counter: */ 
int frames_per_sec=0;
int frames_per_sec_tmp=0;


/* Surfaces: */ 
SDL_Surface *screen_sfc=0;

NETHER *game=0;

void save_configuration(void);
void load_configuration(void);
bool init_video(void);

/*						AUXILIAR FUNCTION DEFINITION:							*/ 

void pause(unsigned int time)
{
	unsigned int initt=SDL_GetTicks();

	while((SDL_GetTicks()-initt)<time);
} /* pause */ 


int initialization() {
	Uint32          colorkey;
	SDL_Surface     *image;
	const SDL_VideoInfo* info;

    if(SDL_Init(SDL_INIT_AUDIO|SDL_INIT_TIMER)<0) {
        fprintf(stderr,"Audio initialization failed: %s\n",SDL_GetError());
		return 0;
    }

	if(SDL_InitSubSystem(SDL_INIT_VIDEO)<0){
		fprintf(stderr,"Video initialization failed: %s\n",SDL_GetError());
		return 0;
	}

	if(! (info=SDL_GetVideoInfo()) ){
		fprintf(stderr,"Video query failed: %s\n",SDL_GetError());
		return 0;
	} else {
		native_x=info->current_w;
		native_y=info->current_h;
//		bpp=info->vfmt->BitsPerPixel;
	}

	if(init_video()) return 0;

	if( access("nether.bmp", F_OK|R_OK) != -1 ){
		image = SDL_LoadBMP("nether.bmp");
		colorkey = SDL_MapRGB(image->format, 255, 255, 255);
		SDL_SetColorKey(image, SDL_SRCCOLORKEY, colorkey); 
		SDL_WM_SetIcon(image, NULL);
	}

	SDL_WM_SetCaption("Nether Earth REMAKE v0.53", 0);

	SDL_Delay(400);
	if (Mix_OpenAudio(22050, AUDIO_S16, 2, 1024)) {
		return 0;
	}

	return 1;
} /* initialization */ 

void end_game(){
	if(!game) return;
	default_zoom=game->get_zoom();
	save_configuration();
	delete game;
	game=0;
	mainmenu_status=0;
	mainmenu_substatus=0;
}

void finalization()
{
	Mix_CloseAudio();
	SDL_Quit();
} /* finalization */ 


#ifdef _WIN32

#ifdef _DEBUG
void SetStdOutToConsole()
{
	int hConHandle;
	long lStdHandle;
	FILE *fp;

	AllocConsole();

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );
}
#endif

int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nCmdShow)
{
	char *argv[1];
	int argc=1;
	argv[0]=strdup("Nether Earth");
#ifdef _DEBUG
	SetStdOutToConsole();
#endif
#else
int main(int argc, char** argv)
{
#endif

	int time,fps_init_time;
	SDL_Event event;
    bool quit = false;

	dbglvl = WARNING;


	load_configuration();

	glutInit(&argc, argv);
	initialization();
	printf("ven: %s\nrenderer: %s\nversion: %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));
	if (screen_sfc==0) return 0;

	fps_init_time=SDL_GetTicks();

	while (!quit) {
		time=SDL_GetTicks();

		// calculate fps
		frames_per_sec_tmp+=1;
		if ((time-fps_init_time)>=1000) {
			frames_per_sec=frames_per_sec_tmp;
			frames_per_sec_tmp=0;
			fps_init_time=time;
		}

		while( SDL_PollEvent( &event ) ) {
			switch( event.type ) {
				case SDL_VIDEORESIZE:
					SCREEN_X=event.resize.w;
					SCREEN_Y=event.resize.h;
					quit = init_video();
				case SDL_VIDEOEXPOSE:
					if(game) game->set_redraw_all();
					break;
				case SDL_ACTIVEEVENT:
					if( game && ( event.active.state&SDL_APPACTIVE || event.active.state & SDL_APPINPUTFOCUS) && event.active.gain) game->set_redraw_all();
					break;

				case SDL_MOUSEBUTTONDOWN:
					if (!cfg_mouse) break;
					if (game && event.button.button == SDL_BUTTON_WHEELDOWN) game->zoomout();
					if (game && event.button.button == SDL_BUTTON_WHEELUP) game->zoomin();

					if (game && event.button.button == SDL_BUTTON_RIGHT){
					   mouse_right_pressed=true;
					   if(game->is_act_menu()) game->act_menu_action(0, event.button.x, event.button.y); // only for distance selection
					}

					if (event.button.button == SDL_BUTTON_LEFT) {
						if(game){
							switch(game->get_game_state()){
								case STATE_PLAYING:
									if( game->is_act_menu() && event.button.x>int((SCREEN_X*25.0F)/32.0F) ) game->act_menu_action(fire_key, event.button.x, event.button.y);
									else {
										switch(game->get_clicked_button(event.button.x, event.button.y)){
											case MOUSE_ESC: game->toggle_pause(); break;
											case MOUSE_ZOOMIN: game->zoomin(); break;
											case MOUSE_ZOOMOUT: game->zoomout(); break;
											case 0: mouse_left_pressed=true;
										}
									}
									break;
								case STATE_SAVINGGAME:
								case STATE_LOADINGGAME:
								case STATE_PAUSE:
									if(!game->option_action(fire_key, event.button.x, event.button.y)) end_game();
									break;
								case STATE_CONSTRUCTION:
									game->construction_action(fire_key, event.button.x, event.button.y);
									break;
							} // switch
						} else {
							if(mainmenu_cycle(0, event.button.x, event.button.y)) quit = init_video();
						}
					}
					break;

				case SDL_MOUSEBUTTONUP:
					if(!cfg_mouse) break;
					if (event.button.button == SDL_BUTTON_LEFT){
						mouse_left_pressed=false;
						up_key_pressed=false;
						down_key_pressed=false;
						left_key_pressed=false;
						right_key_pressed=false;
					}

                    if (event.button.button == SDL_BUTTON_RIGHT) mouse_right_pressed=false;
					break;

					/* Keyboard event */
				case SDL_KEYDOWN:
					if(event.key.keysym.sym==SDLK_F1){
						change_resolution();
						quit = init_video();
						snprintf(message, 20, "RES: %4dx%-4d", SCREEN_X, SCREEN_Y);
						message_timer=2000;
					} else if(event.key.keysym.sym==SDLK_F2){
						if (COLOUR_DEPTH== 8){ COLOUR_DEPTH=16; strcpy(message, "COLOR DEPTH: 16bit"); }
						else if (COLOUR_DEPTH==16){ COLOUR_DEPTH=24; strcpy(message, "COLOR DEPTH: 24bit"); }
						else if (COLOUR_DEPTH==24){ COLOUR_DEPTH=32; strcpy(message, "COLOR DEPTH: 32bit"); }
						else if (COLOUR_DEPTH==32){ COLOUR_DEPTH=8 ; strcpy(message, "COLOR DEPTH: 8bit"); }
						quit = init_video();
						message_timer=2000;
					} else if(event.key.keysym.sym==SDLK_F3){
						shadows++;
						if (shadows>=3) shadows=0;
						if (shadows==0) strcpy(message, "SHADOWS: OFF");
						if (shadows==1) strcpy(message, "SHADOWS: ON - DIAG");
						if (shadows==2) strcpy(message, "SHADOWS: ON - VERT");
						if(game) game->set_shadows();
						save_configuration();
						message_timer=2000;
					} else if(event.key.keysym.sym==SDLK_F4){
						detaillevel++;
						if (detaillevel>=5) detaillevel=0;
						if (detaillevel==0) strcpy(message, "DETAIL: LOWEST");
						if (detaillevel==1) strcpy(message, "DETAIL: LOW");
						if (detaillevel==2) strcpy(message, "DETAIL: MEDIUM");
						if (detaillevel==3) strcpy(message, "DETAIL: HIGH");
						if (detaillevel==4) strcpy(message, "DETAIL: HIGHEST");
						save_configuration();
						message_timer=2000;
					} else if(event.key.keysym.sym==SDLK_F5){
						if (sound){ sound=false; strcpy(message, "SOUND: OFF"); }
						else { sound=true; strcpy(message, "SOUND: ON"); }
						save_configuration();
						message_timer=2000;
					} else if(event.key.keysym.sym==SDLK_F6){
						if (show_radar){ show_radar=false; strcpy(message, "RADAR: OFF"); }
						else { show_radar=true; strcpy(message, "RADAR: ON"); }
						save_configuration();
						message_timer=2000;
					} else if(event.key.keysym.sym==SDLK_F7){
						if (show_fps){ show_fps=false; strcpy(message, "SHOW FPS: OFF"); }
						else { show_fps=true; strcpy(message, "SHOW FPS: ON"); }
						save_configuration();
						message_timer=2000;
					} else if(event.key.keysym.sym==SDLK_F8){
						if (cfg_mouse) {
							cfg_mouse=false;
							strcpy(message, "MOUSE: OFF");
							SDL_ShowCursor(SDL_DISABLE);
						} else {
							cfg_mouse=true;
							strcpy(message, "MOUSE: ON");
							SDL_ShowCursor(SDL_ENABLE);
						}
						save_configuration();
						message_timer=2000;
                    } else if(event.key.keysym.sym==SDLK_F9){
                        if (cfg_copypixels){ cfg_copypixels=false; strcpy(message, "ANTI-FLICKERING: OFF"); }
                        else { cfg_copypixels=true; strcpy(message, "ANTI-FLICKERING: ON"); }
                        save_configuration();
                        message_timer=2000;
					} else if(event.key.keysym.sym==SDLK_F12){
						quit = true;
					} else if(event.key.keysym.sym==SDLK_RETURN){
						SDLMod modifiers;
						modifiers=SDL_GetModState();
						if ((modifiers&KMOD_ALT)!=0) {
							fullscreen = (fullscreen ? false : true);
							quit = init_video();
						}
					}

#ifdef _DEBUG
					if(game){
						int i=0;
						if(event.key.keysym.sym==SDLK_a){ game->lightpos[0]+=1000; i++; }
						else if(event.key.keysym.sym==SDLK_z){ game->lightpos[0]-=1000; i++; }
						else if(event.key.keysym.sym==SDLK_s){ game->lightpos[1]+=1000; i++; }
						else if(event.key.keysym.sym==SDLK_x){ game->lightpos[1]-=1000; i++; }
						else if(event.key.keysym.sym==SDLK_d){ game->lightpos[2]+=1000; i++; }
						else if(event.key.keysym.sym==SDLK_c){ game->lightpos[2]-=1000; i++; }
						if(i){
							game->lightposv.x=game->lightpos[0];
							game->lightposv.y=game->lightpos[1];
							game->lightposv.z=game->lightpos[2];
							printf("x: %f y: %f, z: %f\n",game->lightposv.x, game->lightposv.y, game->lightposv.z);
						}
					}
#endif

					if(game) {
						if(event.key.keysym.sym==up_key) up_key_pressed=true;
						if(event.key.keysym.sym==down_key) down_key_pressed=true;
						if(event.key.keysym.sym==left_key) left_key_pressed=true;
						if(event.key.keysym.sym==right_key) right_key_pressed=true;
						if(event.key.keysym.sym==fire_key) fire_key_pressed=true;

						switch(game->get_game_state()){
							case STATE_PLAYING:
								if(event.key.keysym.sym==SDLK_PAGEUP || event.key.keysym.sym==SDLK_KP_MINUS) zoomout_key_pressed=true;
								if(event.key.keysym.sym==SDLK_PAGEDOWN || event.key.keysym.sym==SDLK_KP_PLUS) zoomin_key_pressed=true;
								if(game->is_act_menu()) game->act_menu_action(event.key.keysym.sym, 0, 0); // action menu
								if(event.key.keysym.sym==pause_key) game->toggle_pause(); // only if STATE_PLAYING or STATE_PAUSE
								break;
							case STATE_SAVINGGAME:
							case STATE_LOADINGGAME:
							case STATE_PAUSE:
								if(!game->option_action(event.key.keysym.sym, 0, 0)) end_game();
								break;
							case STATE_CONSTRUCTION:
								game->construction_action(event.key.keysym.sym, 0, 0);
						} // switch
					} else {  // mainmenu
						if(mainmenu_cycle(event.key.keysym.sym, 0, 0)) quit = init_video();
					}
					break;

				case SDL_KEYUP:
					if(game && game->get_game_state()==STATE_PLAYING){
						if(event.key.keysym.sym==SDLK_PAGEUP || event.key.keysym.sym==SDLK_KP_MINUS) zoomout_key_pressed=false;
						if(event.key.keysym.sym==SDLK_PAGEDOWN || event.key.keysym.sym==SDLK_KP_PLUS) zoomin_key_pressed=false;
					}
					if(event.key.keysym.sym==up_key) up_key_pressed=false;
					if(event.key.keysym.sym==down_key) down_key_pressed=false;
					if(event.key.keysym.sym==left_key) left_key_pressed=false;
					if(event.key.keysym.sym==right_key) right_key_pressed=false;
					if(event.key.keysym.sym==fire_key) fire_key_pressed=false;
					break;

					/* SDL_QUIT event (window close) */
				case SDL_QUIT:
					quit = true;
					break;
			} /* switch event.type */ 

		} /* while pollevent() */

		if (game) {
			if (game->get_game_state()==STATE_PLAYING) {
				if (!game->gamecycle()) end_game();
			}
		} else { // mainmenu
			int val=mainmenu_timers(); // title zoomin/zoomout
			if (val==1) { // new game
				char tmp[256];
				sprintf(tmp,"maps/%s",mapname);
				game=new NETHER(tmp);
			}
			if (val==2) quit=true;
		}

		if (game!=0) {
			game->gameredraw();
		} else {
			mainmenu_draw();
		}

		time_diff=SDL_GetTicks()-time;
		if(time_diff < REDRAWING_PERIOD){
			SDL_Delay(REDRAWING_PERIOD-time_diff);
			time_diff=REDRAWING_PERIOD;
		}
		if(message_timer>0) message_timer-=time_diff; // hidden options, F4-F11
	} /* while */ 

	end_game();

	finalization();

	return 0;
}

bool init_video(){
	if (game!=0) game->refresh_display_lists();
	if (nethertittle!=0) nethertittle->refresh_display_lists();
	if (game!=0) game->deleteobjects();

//	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE| GLUT_RGBA);
//	glutInitWindowPosition(100,100);
//	glutInitWindowSize(SCREEN_X,SCREEN_Y);

		if(fullscreen==true){
#ifdef __LINUX__
			// X fullscreen is buggy. use native resolution
			SCREEN_X=native_x;
			SCREEN_Y=native_y;
#else
			if(!SDL_VideoModeOK(SCREEN_X, SCREEN_Y, COLOUR_DEPTH, SDL_OPENGL|SDL_FULLSCREEN|SDL_HWSURFACE)){
				SCREEN_X=640;
				SCREEN_Y=480;
			}
#endif
		}
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,1);

		if( ! (screen_sfc = SDL_SetVideoMode(SCREEN_X, SCREEN_Y, COLOUR_DEPTH, SDL_OPENGL|SDL_RESIZABLE|(fullscreen ? SDL_FULLSCREEN : 0))) ) return true;
		if(cfg_mouse) SDL_ShowCursor(SDL_ENABLE);
		else SDL_ShowCursor(SDL_DISABLE);
		if (game!=0) game->loadobjects();

	/* save after successful init, resolution change, etc */
	save_configuration();
	return false;
}

void change_resolution(){
	SDL_Rect** modes = SDL_ListModes(NULL, SDL_OPENGL|SDL_FULLSCREEN|SDL_HWSURFACE );
	if (modes != (SDL_Rect**)0 && modes != (SDL_Rect**)-1 && modes!=NULL && modes[0]!=NULL) {
		int count, i;
		for (count=0; modes[count]; count++); count--;
		// count. reverse order. skip portrait modes
		for (i=count; i>=0; i--){
			if(modes[i]->w == SCREEN_X && modes[i]->h == SCREEN_Y){ // current res. found
				for(i--;i>=0; i--){ // next non-portrait
					if(modes[i]->w > modes[i]->h){
						SCREEN_X=modes[i]->w;
						SCREEN_Y=modes[i]->h;
						break;
					}
				}
				// i=-1; back to lowest (first) res. hopefully non-portrait
				break;
			}
		}
		if(i<0){ // current not found or next is portrait. or maybe windows was resized. use lowest
			SCREEN_X=modes[count]->w;
			SCREEN_Y=modes[count]->h;
		}
	}
}

