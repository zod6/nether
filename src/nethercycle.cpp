#ifdef _WIN32
#include "windows.h"
#endif

#include "string.h"
#include "stdlib.h"

#include "stdio.h"
#include "math.h"

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
#include "matrix.h"
#include "nether.h"

#include "glprintf.h"

extern int detaillevel;
extern int frames_per_sec;
extern bool fullscreen;
extern int shadows;
extern bool sound;
extern int up_key,down_key,left_key,right_key,fire_key,pause_key;
extern bool up_key_pressed,down_key_pressed,left_key_pressed,right_key_pressed,fire_key_pressed,pause_key_pressed,zoomin_key_pressed,zoomout_key_pressed;
extern bool mouse_left_pressed, mouse_right_pressed;
extern int level;
extern int SCREEN_X,SCREEN_Y;
extern bool show_radar;
extern bool cfg_mouse;
extern int time_diff;

extern FILE *debug_fp;

bool NETHER::gamecycle()
{
#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"PLAY CYCLE STARTS\n");
	fflush(debug_fp);
#endif

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Recomputing statistics\n");
	fflush(debug_fp);
#endif

	if (zoomout_key_pressed) zoomout();
	if (zoomin_key_pressed) zoomin();

	if (recomputestatistics) {
		List<BUILDING> l;
		BUILDING *b;

		statistics[0][0]=0;
		statistics[0][1]=0;
		statistics[0][2]=0;
		statistics[0][3]=0;
		statistics[0][4]=0;
		statistics[0][5]=0;
		statistics[0][6]=0;
		statistics[0][7]=0;
		statistics[1][0]=0;
		statistics[1][1]=0;
		statistics[1][2]=0;
		statistics[1][3]=0;
		statistics[1][4]=0;
		statistics[1][5]=0;
		statistics[1][6]=0;
		statistics[1][7]=0;

		l.Instance(buildings);
		l.Rewind();
		while(l.Iterate(b)) {
			if (b->type==B_WARBASE) {
				if (b->owner==1) statistics[0][0]++;
				if (b->owner==2) statistics[1][0]++;
			} /* if */ 
			if (b->type==B_FACTORY_ELECTRONICS) {
				if (b->owner==1) statistics[0][1]++;
				if (b->owner==2) statistics[1][1]++;
			} /* if */ 
			if (b->type==B_FACTORY_NUCLEAR) {
				if (b->owner==1) statistics[0][2]++;
				if (b->owner==2) statistics[1][2]++;
			} /* if */ 
			if (b->type==B_FACTORY_PHASERS) {
				if (b->owner==1) statistics[0][3]++;
				if (b->owner==2) statistics[1][3]++;
			} /* if */ 
			if (b->type==B_FACTORY_MISSILES) {
				if (b->owner==1) statistics[0][4]++;
				if (b->owner==2) statistics[1][4]++;
			} /* if */ 
			if (b->type==B_FACTORY_CANNONS) {
				if (b->owner==1) statistics[0][5]++;
				if (b->owner==2) statistics[1][5]++;
			} /* if */ 
			if (b->type==B_FACTORY_CHASSIS) {
				if (b->owner==1) statistics[0][6]++;
				if (b->owner==2) statistics[1][6]++;
			} /* if */ 
		} /* while */ 
		recomputestatistics=false;
	} /* if */ 

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Starting enemy AI\n");
	fflush(debug_fp);
#endif

	/* ENEMY Artificial Intelligence: */ 
	if (second==0) {
		if (level==0 && (hour&0x01)==0 && minute==0) AI_enemy();
		if (level==1 && minute==0) AI_enemy();
		if (level>=2 && (minute==0 || minute==30)) AI_enemy();
	} /* if */ 

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Starting GAME CYCLE\n");
	fflush(debug_fp);
#endif

	/* GAME Cycle: */ 
	{
		shiplanded=false;

#ifdef _WRITE_REPORT_
		fprintf(debug_fp,"Menu\n");
		fflush(debug_fp);
#endif

		/* Free movement of the ship through the map: */ 
		// allow ship movement by mouse in other menus ?
		if(act_menu==GENERAL_MENU) {
			float x[2],y[2];
			float minz;
			Vector old_shipp;

			old_shipp=shipp;
			x[0]=shipp.x;
			x[1]=shipp.x+1.0;
			y[0]=shipp.y;
			y[1]=shipp.y+1.0;
			minz=MapMaxZ(x,y);

			if (fire_key_pressed || (cfg_mouse && mouse_right_pressed) || animation_timer>0) {
				if(shipp.z<5.0) shipp.z+=0.0025*time_diff;
				if(animation_timer>0) animation_timer-=time_diff;
			} else if (shipp.z>minz){
				shipp.z-=0.00125*time_diff;
				if (shipp.z<minz) shipp.z=minz;
			}

			if(ship_op!=OP_NONE){
				float old_x=shipp.x;
				float speed=0.00625*time_diff;

				if (ship_op==OP_RIGHT && shipp.x<map_w-1) {
					shipp.x+=speed;
					if (ship_timemoving>=1000) shipp.x+=speed;
					if ( (shipp.x+speed/2) > int((old_x+0.5)*2)/float(2) ) shipp.x=int((old_x+0.5)*2)/float(2);
				}
				else if (ship_op==OP_LEFT && shipp.x>0) {
					shipp.x-=speed;
					if (ship_timemoving>=1000) shipp.x-=speed;
					if( (shipp.x-speed/2) < ceil((old_x-0.5)*2)/float(2) ) shipp.x=ceil((old_x-0.5)*2)/float(2);
				}
				if(shipp.x<0) shipp.x=0;
				else if(shipp.x>map_w-1) shipp.x=map_w-1;
			}

			if(ship_op2!=OP_NONE){
				float old_y=shipp.y;
				float speed=0.00625*time_diff;

				if (ship_op2==OP_FORWARD && shipp.y<map_h-1) {
					shipp.y+=speed;
					if (ship_timemoving>=1000) shipp.y+=speed;
					if ( (shipp.y+speed/2) > int((old_y+0.5)*2)/float(2) ) shipp.y=int((old_y+0.5)*2)/float(2);
				}
				else if (ship_op2==OP_BACKWARD && shipp.y>0) {
					shipp.y-=speed;
					if (ship_timemoving>=1000) shipp.y-=speed;
					if( (shipp.y-speed/2) < ceil((old_y-0.5)*2)/float(2) ) shipp.y=ceil((old_y-0.5)*2)/float(2);
				}

				if(shipp.y<0) shipp.y=0;
				else if(shipp.y>map_h-1) shipp.y=map_h-1;
			}

			if (ship_op==OP_NONE && ship_op2==OP_NONE) {
				ship_timemoving=0;
			} else {
				ship_timemoving+=time_diff;
			} /* if */ 

			if (shipp!=old_shipp) {
				if (ShipCollision(ship,shipp.x,shipp.y,shipp.z)) {
					ship_timemoving=0;
					Vector p=shipp;
					shipp.x=old_shipp.x;
					shipp.y=old_shipp.y;
					if (p.z!=old_shipp.z && ShipCollision(ship,shipp.x,shipp.y,shipp.z)) {
						shipp.z=old_shipp.z;
						shiplanded=true;
					} else {
						shipp.z=p.z;
					}
					shipp.x=p.x;
					if (p.x!=old_shipp.x && ShipCollision(ship,shipp.x,shipp.y,shipp.z)) {
						shipp.x=old_shipp.x;
					} else {
						shipp.x=p.x;
					}
					shipp.y=p.y;
					if (p.y!=old_shipp.y && ShipCollision(ship,shipp.x,shipp.y,shipp.z)) {
						shipp.y=old_shipp.y;
					} else {
						shipp.y=p.y;
					}
				} /* if */ 
			} /* if */ 

			// stop after 0.5 steps
			if ((int(shipp.x*8)%4)==0) ship_op= OP_NONE;
			if ((int(shipp.y*8)%4)==0) ship_op2=OP_NONE;

			// must be called every cycle because ship moves
			if(mouse_left_pressed) set_movement();

			if (left_key_pressed)	ship_op= OP_LEFT;
			if (right_key_pressed)	ship_op= OP_RIGHT;
			if (up_key_pressed)		ship_op2=OP_FORWARD;
			if (down_key_pressed)	ship_op2=OP_BACKWARD;
			if (fire_key_pressed || (cfg_mouse && mouse_right_pressed)) animation_timer=0;
		} /* if GENERAL_MENU */
	}


#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Viewport\n");
	fflush(debug_fp);
#endif

		viewp.x=shipp.x+0.5;
		viewp.y=shipp.y+0.5;
		viewp.z=shipp.z-3*zoom;
		if (viewp.z<0) viewp.z=0;
		if (viewp.x<3*zoom) {
			viewp.x=3*zoom;
			if (viewp.x>map_w-3*zoom) viewp.x=map_w/2;
		} else {
			if (viewp.x>map_w-3*zoom) {
				viewp.x=map_w-3*zoom;
				if (viewp.x<3*zoom) viewp.x=map_w/2;
			} /* if */ 
		} /* if */ 
		if (viewp.y<3*zoom) {
			viewp.y=3*zoom;
			if (viewp.y>map_h-3*zoom) viewp.y=map_h/2; 
		} else {
			if (viewp.y>map_h-3*zoom) {
				viewp.y=map_h-3*zoom;
				if (viewp.y<3*zoom) viewp.y=map_h/2; 
			} /* if */ 
		} /* if */ 

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Resources\n");
	fflush(debug_fp);
#endif

		{
			STATUSBUTTON *timeb;

			second+=time_diff/4; // 20ms=5s
			if (second>=60) {
				second=0;
				redrawmenu=2;
				minute+=5;
				if (minute>=60) {
					minute=0;
					hour++;
					if (hour>=24) {
						int i;

						hour=0;
						day++;
						/* Resource actualization: */ 
						resources[0][0]+=statistics[0][0]*5;
						if (level==0) {
							int bonus[5]={0,5,6,6,8};
							if (statistics[1][0]<4) resources[1][0]+=bonus[statistics[1][0]];
											   else resources[1][0]+=statistics[1][0]*2;
						} /* if */ 
						if (level==1) {
							int bonus[5]={0,5,8,9,12};
							if (statistics[1][0]<4) resources[1][0]+=bonus[statistics[1][0]];
											   else resources[1][0]+=statistics[1][0]*3;
						} /* if */ 
						if (level==2) {
							int bonus[5]={0,5,10,12,16};
							if (statistics[1][0]<4) resources[1][0]+=bonus[statistics[1][0]];
											   else resources[1][0]+=statistics[1][0]*4;
						} /* if */ 
						if (level==3) {
							int bonus[5]={0,5,10,15,20};
							if (statistics[1][0]<4) resources[1][0]+=bonus[statistics[1][0]];
											   else resources[1][0]+=statistics[1][0]*5;
						} /* if */ 
						for(i=1;i<7;i++) {
							resources[0][i]+=statistics[0][i]*2;
							resources[1][i]+=statistics[1][i]*2;
						} /* for */ 

					} /* if */ 
				} /* if */ 
				timeb=getbutton(TIME_BUTTON);
				if (timeb!=0) {
					sprintf(timeb->text1,"Day: %i",day);
					sprintf(timeb->text2,"Hour: %.2i:%.2i",hour,minute);
				} /* if */ 
			} /* if */ 
		}

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Ship landing\n");
	fflush(debug_fp);
#endif

		/* Test if the ship has landed over a Factory: */ 
		{
			List<BUILDING> l;
			BUILDING *b;

			l.Instance(buildings);
			l.Rewind();
			while(l.Iterate(b)) {
				if (b->type==B_WARBASE && b->owner==1 && 
					shipp.x==b->pos.x && shipp.y==b->pos.y && shiplanded) {
					game_state=STATE_CONSTRUCTION;
					construction_pointer=20;
					construction[0]=false;
					construction[1]=false;
					construction[2]=false;
					construction[3]=false;
					construction[4]=false;
					construction[5]=false;
					construction[6]=false;
					construction[7]=false;
					in_construction.traction=-1;
					in_construction.pieces[0]=false;
					in_construction.pieces[1]=false;
					in_construction.pieces[2]=false;
					in_construction.pieces[3]=false;
					in_construction.pieces[4]=false;
					in_construction.pos.x=b->pos.x+2.5;
					in_construction.pos.y=b->pos.y+0.5;
					in_construction.pos.z=b->pos.z;
					flushbuttons();
					new_construction_buttons();
				} /* if */ 
			} /* while */ 
		}

		/* Test if the ship has landed over a robot: */ 
		if (act_menu==GENERAL_MENU &&
			(int(shipp.x*8)%4)==0 &&
			(int(shipp.y*8)%4)==0) {
			List<ROBOT> l;
			ROBOT *r;

			l.Instance(robots[0]);
			l.Rewind();
			while(l.Iterate(r)) {
				if (shipp.x==(r->pos.x-0.5) && shipp.y==(r->pos.y-0.5) && shiplanded) {
					/* The ship has landed over a robot: */ 
					r->shipover=true;
					controlled=r;
					if (controlled->op==ROBOTOP_FORWARD) controlled->op=ROBOTOP_NONE;
					if (controlled->program==PROGRAM_FORWARD) controlled->program=PROGRAM_STOPDEFEND;
					killmenu(GENERAL_MENU);
					newmenu(ROBOT_MENU);
					act_button=ROBOT4_BUTTON;
				} /* while */ 
			} /* while */ 
		} /* if */ 

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Robots\n");
	fflush(debug_fp);
#endif

		/* Robot cycles: */ 
		{
			int i;
			List<ROBOT> l,robotstodelete;
			ROBOT *r;
			float x[2],y[2],minz;
			Vector old_pos;
			int old_chassis_state;
			int terrain;
			
			for(i=0;i<2;i++) {

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Player %i robots\n",i);
	fflush(debug_fp);
#endif

				l.Instance(robots[i]);
				l.Rewind();
				while(l.Iterate(r)) {
					/* Robot cycle: */ 
#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"START ROBOT\n");
	fflush(debug_fp);
#endif
					
					/* Animations: */ 
					if (r->electronics_state!=0) {
						r->electronics_state+=time_diff*0.3;
						if (r->electronics_state>=360) r->electronics_state=0;
					} /* if */ 
					
					/* Apply ROBOT operator: */ 
					old_pos=r->pos;
					old_chassis_state=r->chassis_state;
					if (r->traction==2) {
						r->chassis_state++;
					} /* if */ 
					x[0]=r->pos.x-0.5;
					x[1]=r->pos.x+0.5;
					y[0]=r->pos.y-0.5;
					y[1]=r->pos.y+0.5;
					minz=MapMaxZ(x,y);
					terrain=WorseMapTerrain(x,y);

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Avoid robot deadlock\n");
	fflush(debug_fp);
#endif

					/* Avoid that a Robot can walk agains another and they both get stuck: */ 
					if (r->op==ROBOTOP_FORWARD &&
						(int(r->pos.x*256)%128)==0 &&
						(int(r->pos.y*256)%128)==0) {
						switch(r->angle) {
						case 0:
							if (AI_WorseMapTerrain(int((r->pos.x+0.5)/0.5),int((r->pos.y-0.5)/0.5),1,2)>T_HOLE) r->op=ROBOTOP_NONE;
							break;
						case 90:
							if (AI_WorseMapTerrain(int((r->pos.x-0.5)/0.5),int((r->pos.y+0.5)/0.5),2,1)>T_HOLE) r->op=ROBOTOP_NONE;
							break;
						case 180:
							if (AI_WorseMapTerrain(int((r->pos.x-1.0)/0.5),int((r->pos.y-0.5)/0.5),1,2)>T_HOLE) r->op=ROBOTOP_NONE;
							break;
						case 270:
							if (AI_WorseMapTerrain(int((r->pos.x-0.5)/0.5),int((r->pos.y-1.0)/0.5),2,1)>T_HOLE) r->op=ROBOTOP_NONE;
							break;
						} /* switch */ 

						if (r->op==ROBOTOP_NONE && r->program==PROGRAM_FORWARD) r->program=PROGRAM_STOPDEFEND;
					} /* if */ 

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"ROBOT COMMAND: %i\n",r->op);
	fflush(debug_fp);
#endif
					if (r->op==ROBOTOP_FORWARD) {
						float speed=RobotSpeed(r->traction,terrain)*time_diff;

						/* BIPOD ANIMATION: */ 
						if (r->traction==0) {
							r->chassis_state+=int(speed/0.00390625);
							if (r->chassis_state>64) r->chassis_state=-63;
						} /* if */ 

						/* TRACKS PARTICLES: */ 
						if (r->traction==1 && detaillevel>=4) {
							int i;
							PARTICLE *p;
							Vector pos,sp1;
							float red,g,b;

							for(i=0;i<2;i++) {
								pos.x=r->pos.x+float(rand()%10)/100.0;
								pos.y=r->pos.y+float(rand()%10)/100.0;
								pos.z=0;
								red=0.9F+float(rand()%21-10)/100.0;
								g=0.7F+float(rand()%21-10)/100.0;
								b=0.5F+float(rand()%21-10)/100.0;
								switch(r->angle) {
									case 0:
										sp1=Vector(-0.05,float(rand()%9-4)/200.0,0);
										pos.x-=0.25;
										pos.y+=((rand()%2)==0 ? -0.5 : 0.5);
										break;
									case 90:
										sp1=Vector(float(rand()%9-4)/200.0,-0.05,0);
										pos.y-=0.25;
										pos.x+=((rand()%2)==0 ? -0.5 : 0.5);
										break;
									case 180:
										sp1=Vector(0.05,float(rand()%9-4)/200.0,0);
										pos.x+=0.25;
										pos.y+=((rand()%2)==0 ? -0.5 : 0.5);
										break;
									case 270:
										sp1=Vector(float(rand()%9-4)/200.0,0.05,0);
										pos.y+=0.25;
										pos.x+=((rand()%2)==0 ? -0.5 : 0.5);
										break;
								} /* switch */ 									
								p=new PARTICLE(pos,sp1,Vector(0,0,0.05),0,0.3, red,g,b, 1.0,0.0,20+(rand()%10));
								particles.Add(p);
							} /* for */ 
						} /* if */ 

						// move and stop after every 0.5 step
						switch(r->angle) {
							case 0:
								if (r->pos.x<map_w-0.5){
									r->pos.x+=speed;
									if ( (r->pos.x+speed/2) >= int((old_pos.x+0.5)*2)/float(2) ) {
										r->pos.x=int((old_pos.x+0.5)*2)/float(2);
										r->op=ROBOTOP_NONE;
									}
								}
								break;
							case 90:
								if (r->pos.y>0.5){
									r->pos.y+=speed;
									if ( (r->pos.y+speed/2) >= int((old_pos.y+0.5)*2)/float(2) ) {
										r->pos.y=int((old_pos.y+0.5)*2)/float(2);
										r->op=ROBOTOP_NONE;
									}
								}
								break;
							case 180:
								if (r->pos.x>0.5) {
									r->pos.x-=speed;
									if ( (r->pos.x-speed/2) <= ceil((old_pos.x-0.5)*2)/float(2) ) {
										r->pos.x=ceil((old_pos.x-0.5)*2)/float(2);
										r->op=ROBOTOP_NONE;
									}
								}
								break;
							case 270:
								if (r->pos.y<map_h-0.5){
									r->pos.y-=speed;
									if ( (r->pos.y-speed/2) <= ceil((old_pos.y-0.5)*2)/float(2) ) {
										r->pos.y=ceil((old_pos.y-0.5)*2)/float(2);
										r->op=ROBOTOP_NONE;
									}
								}
								break;
						} /* switch */ 
					} else {
						if (r->traction==0) r->chassis_state=0;
					} /* if */ 
					
					// stop after every 90deg
					if (r->op==ROBOTOP_LEFT){
						int speed=RobotRotationSpeed(r->traction,terrain)*time_diff;
						r->angle-=speed;
						if ( (r->angle-speed/2) <= ceil((r->angle+speed-90)/90.0)*90 ) {
							r->angle=ceil((r->angle+speed-90)/90.0)*90;
							r->op=ROBOTOP_NONE;
						}
					}
					if (r->op==ROBOTOP_RIGHT) {
						int speed=RobotRotationSpeed(r->traction,terrain)*time_diff;
						r->angle+=speed;
						if ( (r->angle+int(speed/2)) >= int((r->angle-speed+90)/90)*90 ) {
							r->angle=floor((r->angle-speed+90)/90)*90;
							r->op=ROBOTOP_NONE;
						}
					}
					if (r->angle>=360) r->angle-=360;
					if (r->angle<0) r->angle+=360;

					if (r->op==ROBOTOP_CANNONS && r->firetimer==0) {
						BULLET *b;
						Vector pos;

						pos=r->pos;
						pos.z=r->piecez(0)+0.3f;

						b=new BULLET(0,pos,r->angle,r);
						b->cmc=BulletCMC(b);

						bullets.Add(b);
						if (S_shot!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_shot,0),SFX_volume(r->pos));
					} /* if */ 

					if (r->op==ROBOTOP_MISSILES && r->firetimer==0) {
						BULLET *b;
						Vector pos;

						pos=r->pos;
						pos.z=r->piecez(1)+0.2f;

						b=new BULLET(1,pos,r->angle,r);
						b->cmc=BulletCMC(b);

						bullets.Add(b);
						if (S_shot!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_shot,0),SFX_volume(r->pos));
					} /* if */ 

					if (r->op==ROBOTOP_PHASERS && r->firetimer==0) {
						BULLET *b;
						Vector pos;

						pos=r->pos;
						pos.z=r->piecez(2)+0.3f;

						b=new BULLET(2,pos,r->angle,r);
						b->cmc=BulletCMC(b);

						bullets.Add(b);
						if (S_shot!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_shot,0),SFX_volume(r->pos));
					} /* if */ 

					if (r->op==ROBOTOP_CANNONS ||
						r->op==ROBOTOP_MISSILES ||
						r->op==ROBOTOP_PHASERS) r->firetimer+=time_diff;

					if (r->op==ROBOTOP_NUCLEAR) {
						EXPLOSION *n;

						n=new EXPLOSION(r->pos,2);
						explosions.Add(n);

						/* Robot destroyed: */ 
						if (r==controlled) {
							controlled->shipover=false;
							controlled=0;
							killmenu(act_menu);
							newmenu(GENERAL_MENU);
						} /* if */ 

						/* Find Robots to destroy: */ 
						{
							List<ROBOT> l;
							ROBOT *r;
							int i;
							float distance;

							for(i=0;i<2;i++) {
								l.Instance(robots[i]);
								l.Rewind();
								while(l.Iterate(r)) {
									distance=(r->pos-n->pos).norma();
									if (distance<=2.5f) robotstodelete.Add(r);
								} /* while */ 
							} /* for */ 
						}

						/* Find buildings to destroy: */ 
						{
							List<BUILDING> l,todelete;
							BUILDING *b;
							float distance;

							l.Instance(buildings);
							l.Rewind();
							while(l.Iterate(b)) {
								distance=(b->pos-(n->pos-Vector(0.5,0.5,0.5))).norma();
								if (distance<=2.5f) todelete.Add(b);
							} /* while */ 

							while(!todelete.EmptyP()) {
								b=todelete.Extract();
								buildings.DeleteElement(b);
								AI_removebuilding(b->pos);
								delete b;
							} /* while */ 
						}

						if (S_explosion!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_explosion,0),SFX_volume(r->pos));
						recomputestatistics=true;
					} /* if */ 

					if (r!=0) {
						x[0]=r->pos.x-0.5;
						x[1]=r->pos.x+0.5;
						y[0]=r->pos.y-0.5;
						y[1]=r->pos.y+0.5;
						minz=MapMaxZ(x,y);
						terrain=WorseMapTerrain(x,y);
						r->pos.z=minz;

						if (r->shipover) {
							shipp.x=r->pos.x-0.5;
							shipp.y=r->pos.y-0.5;
							shipp.z=r->pos.z+r->cmc.z[1];
						} /* if */ 

						/* Collision: */ 
						if (RobotCollision(r,false) || !Walkable(r->traction,terrain)) {
							r->pos=old_pos;
							if (r->traction==0) r->chassis_state=old_chassis_state;
							if (r->shipover) {
								shipp.x=r->pos.x-0.5;
								shipp.y=r->pos.y-0.5;
								shipp.z=r->pos.z+r->cmc.z[1];
							} /* if */ 
						} else {
							AI_moverobot(old_pos,r->pos,i);
						} /* if */ 
						
//						if (r->op==ROBOTOP_FORWARD && (r->angle==0 || r->angle==180) && (int(r->pos.x*256)%128)==0) r->op=ROBOTOP_NONE;
//						if (r->op==ROBOTOP_FORWARD && (r->angle==90 || r->angle==270) && (int(r->pos.y*256)%128)==0) r->op=ROBOTOP_NONE;
//						if (r->op==ROBOTOP_LEFT && (r->angle%90)==0) r->op=ROBOTOP_NONE;
//						if (r->op==ROBOTOP_RIGHT && (r->angle%90)==0) r->op=ROBOTOP_NONE;
//
						if (r->op==ROBOTOP_CANNONS && r->firetimer>=1280) {
							r->op=ROBOTOP_NONE;
							r->firetimer=0;
						} /* if */ 
						if (r->op==ROBOTOP_MISSILES && r->firetimer>=1280) {
							r->op=ROBOTOP_NONE;
							r->firetimer=0;
						} /* if */ 
						if (r->op==ROBOTOP_PHASERS && r->firetimer>=1280) {
							r->op=ROBOTOP_NONE;
							r->firetimer=0;
						} /* if */ 

						/* Follow ROBOT program: */ 
						if (r->op==ROBOTOP_NONE && !r->shipover) {
#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Robot program: %i\n",r->program);
	fflush(debug_fp);
#endif
							switch(r->program) {
							case PROGRAM_NONE:
								break;
							case PROGRAM_FORWARD:
								r->op=ROBOTOP_FORWARD;
								r->program_parameter--;
								if (r->program_parameter<=0) r->program=PROGRAM_STOPDEFEND;
								break;
							case PROGRAM_STOPDEFEND:
								r->op=AI_program_stopdefend(&(r->program_goal),r->pos,r->angle,r->traction,r->pieces[4],i+1,r->pieces);
								break;
							case PROGRAM_ADVANCE:
								r->op=AI_program_advance(r->program_parameter,r->pos,r->angle,r->traction,r->pieces[4],i+1,r->pieces);
								if (r->op==ROBOTOP_FORWARD && r->angle==90) r->program_parameter--;
								if (r->op==ROBOTOP_FORWARD && r->angle==270) r->program_parameter++;
								if (r->program_parameter==0) r->program=PROGRAM_STOPDEFEND;
								break;
							case PROGRAM_RETREAT:
								r->op=AI_program_retreat(r->program_parameter,r->pos,r->angle,r->traction,r->pieces[4],i+1,r->pieces);
								if (r->op==ROBOTOP_FORWARD && r->angle==270) r->program_parameter--;
								if (r->op==ROBOTOP_FORWARD && r->angle==90) r->program_parameter++;
								if (r->program_parameter==0) r->program=PROGRAM_STOPDEFEND;
								break;
							case PROGRAM_DESTROY:
								r->op=AI_program_destroy(r->program_parameter,&(r->program_goal),r->pos,r->angle,r->traction,r->pieces[4],i+1,r->pieces);
								// if (r->program_goal.x==-1) r->program=PROGRAM_STOPDEFEND;
								break;
							case PROGRAM_CAPTURE:
								r->op=AI_program_capture(r->program_parameter,&(r->program_goal),r->pos,r->angle,r->traction,r->pieces[4],i+1,r->pieces);
								// if (r->program_goal.x==-1) r->program=PROGRAM_STOPDEFEND;
								break;
							} /* switch */ 
						} /* if */ 

						/* Follow USER's command: */ 
						if (r->op==ROBOTOP_NONE && r->shipover &&
							(act_menu==DIRECTCONTROL_MENU ||
							 act_menu==DIRECTCONTROL2_MENU)) {
#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"User command\n",i);
	fflush(debug_fp);
#endif
							if(mouse_left_pressed) set_movement();

							if (right_key_pressed) {
								if (r->angle==0) {
									r->op=ROBOTOP_FORWARD;
								} else {
									if (r->angle==270) r->op=ROBOTOP_RIGHT;
												  else r->op=ROBOTOP_LEFT;
								} /* if */ 
							} /* if */ 
							if (left_key_pressed) {
								if (r->angle==180) {
									r->op=ROBOTOP_FORWARD;
								} else {
									if (r->angle==90) r->op=ROBOTOP_RIGHT;
												 else r->op=ROBOTOP_LEFT;
								} /* if */ 
							} /* if */ 
							if (up_key_pressed) {
								if (r->angle==90) {
									r->op=ROBOTOP_FORWARD;
								} else {
									if (r->angle==0) r->op=ROBOTOP_RIGHT;
												else r->op=ROBOTOP_LEFT;
								} /* if */ 
							} /* if */ 
							if (down_key_pressed) {
								if (r->angle==270) {
									r->op=ROBOTOP_FORWARD;
								} else {
									if (r->angle==180) r->op=ROBOTOP_RIGHT;
												  else r->op=ROBOTOP_LEFT;
								} /* if */ 
							} /* if */ 

						} /* if */ 
					} /* if */ 
				} /* while */ 

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Deleting robots\n",i);
	fflush(debug_fp);
#endif
				while(!robotstodelete.EmptyP()) {
					r=robotstodelete.Extract();
					AI_killrobot(r->pos);
					robots[0].DeleteElement(r);
					robots[1].DeleteElement(r);
					delete r;
				} /* while */ 

			} /* for */ 
		}

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Buildings\n");
	fflush(debug_fp);
#endif

		/* Buildings: */ 
		{
			List<BUILDING> l;
			BUILDING *b;

			l.Instance(buildings);
			l.Rewind();
			while(l.Iterate(b)) {
				if (b->type==B_FACTORY_ELECTRONICS ||
					b->type==B_FACTORY_NUCLEAR ||
					b->type==B_FACTORY_PHASERS ||
					b->type==B_FACTORY_MISSILES ||
					b->type==B_FACTORY_CANNONS	||
					b->type==B_FACTORY_CHASSIS) {
					int robot=AI_robothere(b->pos+Vector(1,0,0));
					if (robot==0) {
						b->status=0;
					} else {
						if (robot==T_ROBOT) b->status+=time_diff;
						if (robot==T_EROBOT) b->status-=time_diff;

						if (b->status>=35000) { // 35sec
							b->owner=1;
							b->status=0;
							recomputestatistics=true;
						} /* if */ 

						if (b->status<=-35000) {
							b->owner=2;
							b->status=0;
							recomputestatistics=true;
						} /* if */ 
					} /* if */ 
				} /* if */ 

				if (b->type==B_WARBASE) {
					int robot=AI_robothere(b->pos+Vector(2,0,0));
					if (robot==0) {
						b->status=0;
					} else {
						if (robot==T_ROBOT) b->status+=time_diff;
						if (robot==T_EROBOT) b->status-=time_diff;

						if (b->status>=35000) {
							b->owner=1;
							b->status=0;
							recomputestatistics=true;
						} /* if */ 

						if (b->status<=-35000) {
							b->owner=2;
							b->status=0;
							recomputestatistics=true;
						} /* if */ 
					} /* if */ 
				} /* if */ 

			} /* while */ 
		}

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Bullets\n");
	fflush(debug_fp);
#endif

		/* Bullets: */ 
		{
			List<BULLET> l,todelete;
			BULLET *b;
			ROBOT *r;
			int persistence=CANNON_PERSISTENCE;
			
			l.Instance(bullets);
			l.Rewind();
			while(l.Iterate(b)) {
				r=0;

				if (b->angle==0) b->pos.x+=BULLET_SPEED*time_diff/20;
				if (b->angle==90) b->pos.y+=BULLET_SPEED*time_diff/20;
				if (b->angle==180) b->pos.x-=BULLET_SPEED*time_diff/20;
				if (b->angle==270) b->pos.y-=BULLET_SPEED*time_diff/20;
				b->step++;

				if (b->type==1) persistence=MISSILE_PERSISTENCE;
				if (b->type==2) persistence=PHASER_PERSISTENCE;
				if (b->step>=persistence || BulletCollision(b,&r)) {
					todelete.Add(b);
					if (b->step<persistence) {
						EXPLOSION *n;

						n=new EXPLOSION(b->pos,0);
						explosions.Add(n);
					} /* if */ 
				} /* if */ 

				if (r!=0) {
					/* The bullet has collided with a robot: */ 
					if (!r->bullethit(b->type)) {
						/* Robot destroyed: */ 
						EXPLOSION *n;

						n=new EXPLOSION(r->pos,1);
						explosions.Add(n);

						if (S_explosion!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_explosion,0),SFX_volume(r->pos));
						if (r==controlled) {
							controlled->shipover=false;
							controlled=0;
							killmenu(ALL_MENUS);
							newmenu(GENERAL_MENU);
						} /* if */ 
						AI_killrobot(r->pos);
						if (robots[0].MemberRefP(r)) robots[0].DeleteElement(r);
											    else robots[1].DeleteElement(r);
						delete r;
					} /* if */ 
				} /* if */ 

			} /* while */ 

			while(!todelete.EmptyP()) {
				b=todelete.Extract();
				bullets.DeleteElement(b);
				delete b;
			} /* while */ 
		}

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Nuclear explosions\n");
	fflush(debug_fp);
#endif

		/* Nuclear explosions: */ 
		{
			List<EXPLOSION> l,todelete;
			EXPLOSION *n;

			l.Instance(explosions);
			l.Rewind();
			while(l.Iterate(n)) {
				if (n->size==0) n->step+=2*time_diff;
				if (n->size==1) n->step+=time_diff;
				n->step++;
				if (n->step>=2560) todelete.Add(n);
			} /* while */ 

			while(!todelete.EmptyP()) {
				n=todelete.Extract();
				explosions.DeleteElement(n);
				delete n;
			} /* while */ 
		}

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Particles\n");
	fflush(debug_fp);
#endif

		/* Particles: */ 
		{
			List<PARTICLE> l,todelete;
			PARTICLE *p;

			l.Instance(particles);
			l.Rewind();
			while(l.Iterate(p)) {
				if (!CycleParticle(p)) todelete.Add(p);
			} /* while */ 

			while(!todelete.EmptyP()) {
				p=todelete.Extract();
				particles.DeleteElement(p);
				delete p;
			} /* while */ 
		}

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Starting STATUS cycle\n");
	fflush(debug_fp);
#endif

	/* STATUS Cycle: */ 
	{
		List<STATUSBUTTON> l,todelete;
		STATUSBUTTON *b;

		l.Instance(buttons);
		l.Rewind();
		while(l.Iterate(b)) {
			if (b->status!=0) {
				if(b->status<0 && b->status+time_diff>0) b->status=0;
				else b->status+=time_diff;
				redrawmenu=2;
				if (b->status>=320) {
					todelete.Add(b);
				} /* if */ 
			} /* if */ 
		} /* while */ 

		while(!todelete.EmptyP()) {
			b=todelete.Extract();
			buttons.DeleteElement(b);
			delete b;
		} /* while */ 
	}

	if ((statistics[0][0]==0 || statistics[1][0]==0) && game_finished==0) {
		game_finished++;
		game_started=0;
	} /* if */ 
	if (game_finished>0) game_finished+=time_diff;
	if (game_finished>=5200) return false; // 5,2sec

	if (game_started>0) game_started-=time_diff;

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"PLAY CYCLE ENDS\n");
	fflush(debug_fp);
#endif

	return true;
} /* NETHER::cycle */ 


void NETHER::act_menu_action(int key, int mx, int my){

	if(mx>0) {
		List<STATUSBUTTON> l;
		STATUSBUTTON *b;
		float x, y, z;
		int found=0;
		float projection_matrix[16];
		int viewport[4];

		viewport[0]=int(SCREEN_X*25.0F/32.0F);
		viewport[1]=0;
		viewport[2]=SCREEN_X-viewport[0];
		viewport[3]=SCREEN_Y;

		// glOrtho(0, (SCREEN_X-split)/SCREEN_X*640.0, 0, 480, -100, 100);
		set_projection_ortho( 0, viewport[2]*640.0/SCREEN_X, 0, 480, -100, 100, projection_matrix);

		if(!unproject(mx, my, 1.0f, NULL, projection_matrix, viewport, &x, &y, &z)) return;

		l.Instance(buttons);
		l.Rewind();
		while(l.Iterate(b)) {
			if (b->ID!=TIME_BUTTON && b->status>=-320 && x > (b->x - b->sx/2) && x < (b->x + b->sx/2) && y > (b->y - b->sy/2) && y < (b->y + b->sy/2)){
				act_button=b->ID;
				found=1;
				break;
			}
		}
		if (!found) return;
	}

	switch (act_menu) {
		case DIRECTCONTROL_MENU:
			/* Direct control of a robot by the user: */ 
			{
				STATUSBUTTON *b;
				if (key==fire_key) {
					b=getbutton(act_button);
					if (b!=0) {
						b->r=0.5f;
						b->g=0.5f;
						b->b=1.0f;
					}
					if(act_button!=ROBOT1_BUTTON){
						b=getbutton(ROBOT1_BUTTON);
						if (b!=0) {
							b->r=0.0f;
							b->g=0.0f;
							b->b=0.8f;
						}
					}

					redrawmenu=2;
					act_menu=ROBOT_MENU;

					if( ! (mx && act_button!=ROBOT1_BUTTON) ) break; // don't break on mouse click
				} else break;
			}

		case ROBOT_MENU:
			/* Browsing through the ROBOT MENU: */ 
			{
				int i;
				int buttons[4]={ROBOT1_BUTTON,ROBOT2_BUTTON,ROBOT3_BUTTON,ROBOT4_BUTTON};
				STATUSBUTTON *b;

				if (key==up_key) {
					int j=-1;
					for(i=0;i<4 && j==-1;i++) {
						if (act_button==buttons[i]) {
							j=i-1;
							if (j<0) j+=4;
							act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					redrawmenu=2;
				} /* if */ 
				if (key==down_key) {
					int j=-1;
					for(i=0;i<4 && j==-1;i++) {
						if (act_button==buttons[i]) {
							j=i+1;
							if (j>=4) j-=4;
							act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					redrawmenu=2;
				} /* if */ 

				for(i=0;i<4;i++) {
					b=getbutton(buttons[i]);
					if (b!=0) {
						b->r=0.0f;
						b->g=0.0f;
						b->b=0.8f;
					} /* if */ 
				} /* if */ 
				b=getbutton(act_button);
				if (b!=0) {
					b->r=0.5f;
					b->g=0.5f;
					b->b=1.0f;
				} /* if */ 

				if (key==fire_key) {
					switch(act_button) {
					case ROBOT1_BUTTON:
						{
							STATUSBUTTON *b;

							b=getbutton(act_button);
							if (b!=0) {
								b->r=1.0f;
								b->g=0.5f;
								b->b=0.5f;
							} /* if */ 

							act_menu=DIRECTCONTROL_MENU;
							redrawmenu=2;
							if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						}
						break;
					case ROBOT2_BUTTON:
						{
							killmenu(ROBOT_MENU);
							newmenu(ORDERS_MENU);
							act_button=ORDERS1_BUTTON;
							if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						}
						break;
					case ROBOT3_BUTTON:
						{
							killmenu(ROBOT_MENU);
							newmenu(COMBATMODE_MENU);
							act_button=COMBAT6_BUTTON;
							if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						}
						break;
					case ROBOT4_BUTTON:
						/* Back to the general menu: */ 
						controlled->electronics_state+=6;
						controlled->shipover=false;
						controlled=0;
						killmenu(ROBOT_MENU);
						newmenu(GENERAL_MENU);
						if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						animation_timer=1000; // lift ship for 1000ms
						break;
					} /* switch */ 
				} /* if */ 
			}
			break;

		case DIRECTCONTROL2_MENU:
			/* Direct control of a robot by the user: */ 
			{
				STATUSBUTTON *b;
				if (key==fire_key) {
					b=getbutton(act_button);
					if (b!=0) {
						b->r=0.5f;
						b->g=0.5f;
						b->b=1.0f;
					}
					if(act_button!=COMBAT5_BUTTON){
						b=getbutton(COMBAT5_BUTTON);
						if (b!=0) {
							b->r=0.0f;
							b->g=0.0f;
							b->b=0.8f;
						}
					}

					redrawmenu=2;
					act_menu=COMBATMODE_MENU;

					if( ! (mx && act_button!=COMBAT5_BUTTON) ) break; // don't break when mouse click
				} else break;
			}

		case COMBATMODE_MENU:
			/* Browsing through the COMBAT MENU: */ 
			{
				int i;
				int buttons[6]={COMBAT1_BUTTON,COMBAT2_BUTTON,COMBAT3_BUTTON,COMBAT4_BUTTON,COMBAT5_BUTTON,COMBAT6_BUTTON};
				STATUSBUTTON *b;

				if (key==up_key) {
					int j=-1;
					for(i=0;i<6 && j==-1;i++) {
						if (act_button==buttons[i]) {
							j=i-1;
							if (j<0) j+=6;
							act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					redrawmenu=2;
				} /* if */ 
				if (key==down_key) {
					int j=-1;
					for(i=0;i<6 && j==-1;i++) {
						if (act_button==buttons[i]) {
							j=i+1;
							if (j>=6) j-=6;
							act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					redrawmenu=2;
				} /* if */ 

				for(i=0;i<6;i++) {
					b=getbutton(buttons[i]);
					if (b!=0) {
						b->r=0.0f;
						b->g=0.0f;
						b->b=0.8f;
					} /* if */ 
				} /* if */ 
				b=getbutton(act_button);
				if (b!=0) {
					b->r=0.5f;
					b->g=0.5f;
					b->b=1.0f;
				} /* if */ 
				
				if (key==fire_key) {
					switch(act_button) {
					case COMBAT1_BUTTON:
						/* Fire Nuclear: */ 
						if ((controlled->angle==0 || controlled->angle==90 ||
							 controlled->angle==180 || controlled->angle==270) &&
							 controlled->pieces[3] &&
							 controlled->firetimer==0) {
							controlled->op=ROBOTOP_NUCLEAR;
						} /* if */ 						
						break;
					case COMBAT2_BUTTON:
						/* Fire Phasers: */ 
						if ((controlled->angle==0 || controlled->angle==90 ||
							 controlled->angle==180 || controlled->angle==270) &&
							 controlled->pieces[2] &&
							 controlled->op==ROBOTOP_NONE) {
							controlled->op=ROBOTOP_PHASERS;
						} /* if */ 
						break;
					case COMBAT3_BUTTON:
						/* Fire Missiles: */ 
						if ((controlled->angle==0 || controlled->angle==90 ||
							 controlled->angle==180 || controlled->angle==270) &&
							 controlled->pieces[1] &&
							 controlled->op==ROBOTOP_NONE) {
							controlled->op=ROBOTOP_MISSILES;
						} /* if */ 
						break;
					case COMBAT4_BUTTON:
						/* Fire Canons: */ 
						if ((controlled->angle==0 || controlled->angle==90 ||
							 controlled->angle==180 || controlled->angle==270) &&
							 controlled->pieces[0] &&
							 controlled->op==ROBOTOP_NONE) {
							controlled->op=ROBOTOP_CANNONS;
						} /* if */ 
						break;
					case COMBAT5_BUTTON:
						{
							STATUSBUTTON *b;

							b=getbutton(act_button);
							if (b!=0) {
								b->r=1.0f;
								b->g=0.5f;
								b->b=0.5f;
							} /* if */ 

							act_menu=DIRECTCONTROL2_MENU;
							redrawmenu=2;
							if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						}
						break;
					case COMBAT6_BUTTON:
						/* Back to the robot menu: */ 
						killmenu(COMBATMODE_MENU);
						newmenu(ROBOT_MENU);
						act_button=ROBOT3_BUTTON;
						if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						break;
					} /* switch */ 
				} /* if */ 
			}
			break;
		case ORDERS_MENU:
			/* Browsing through the ORDERS MENU: */ 
			{
				int i;
				int buttons[5]={ORDERS1_BUTTON,ORDERS2_BUTTON,ORDERS3_BUTTON,ORDERS4_BUTTON,ORDERS5_BUTTON};
				STATUSBUTTON *b;

				if (key==up_key) {
					int j=-1;
					for(i=0;i<5 && j==-1;i++) {
						if (act_button==buttons[i]) {
							j=i-1;
							if (j<0) j+=5;
							act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					redrawmenu=2;
				} /* if */ 
				if (key==down_key) {
					int j=-1;
					for(i=0;i<5 && j==-1;i++) {
						if (act_button==buttons[i]) {
							j=i+1;
							if (j>=5) j-=5;
							act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					redrawmenu=2;
				} /* if */ 

				for(i=0;i<5;i++) {
					b=getbutton(buttons[i]);
					if (b!=0) {
						b->r=0.0f;
						b->g=0.0f;
						b->b=0.8f;
					} /* if */ 
				} /* if */ 
				b=getbutton(act_button);
				if (b!=0) {
					b->r=0.5f;
					b->g=0.5f;
					b->b=1.0f;
				} /* if */ 
				
				if (key==fire_key) {
					switch(act_button) {
					case ORDERS1_BUTTON:
						/* STOP & DEFEND: */ 
						controlled->program=PROGRAM_STOPDEFEND;
						controlled->program_goal=Vector(-1,-1,-1);

						killmenu(ORDERS_MENU);
						newmenu(ROBOT_MENU);
						act_button=ROBOT4_BUTTON;
						if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						break;
					case ORDERS2_BUTTON:
						/* ADVANCE ?? MILES: */ 
						controlled->program=PROGRAM_ADVANCE;
						controlled->program_parameter=0;
						controlled->program_goal=Vector(-1,-1,-1);

						killmenu(ORDERS_MENU);
						newmenu(SELECTDISTANCE_MENU);
						act_button=ORDERS_BUTTON;
						if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						break;
					case ORDERS3_BUTTON:
						/* RETREAT ?? MILES: */ 
						controlled->program=PROGRAM_RETREAT;
						controlled->program_parameter=0;
						controlled->program_goal=Vector(-1,-1,-1);

						killmenu(ORDERS_MENU);
						newmenu(SELECTDISTANCE_MENU);
						act_button=ORDERS_BUTTON;
						if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						break;
					case ORDERS4_BUTTON:
						/* SEARCH AND DESTROY: */ 
						killmenu(ORDERS_MENU);
						newmenu(TARGETD_MENU);
						act_button=TARGET1_BUTTON;
						if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						break;
					case ORDERS5_BUTTON:
						/* SEARCH AND CAPTURE: */ 
						killmenu(ORDERS_MENU);
						newmenu(TARGETC_MENU);
						act_button=TARGET1_BUTTON;
						if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						break;
					} /* switch */ 
				} /* if */ 
			}
			break;

		case SELECTDISTANCE_MENU:
			{
				if (key==up_key || (act_button==TARGET1_BUTTON && key==fire_key && mouse_left_pressed)) {
					controlled->program_parameter+=10;
					if (controlled->program_parameter>190) controlled->program_parameter=0;
					controlled->program_goal=Vector(-1,-1,-1);

					redrawmenu=2;
				} else if (key==down_key || (act_button==TARGET1_BUTTON && !key && mouse_right_pressed)) {
					controlled->program_parameter-=10;
					if (controlled->program_parameter<0) controlled->program_parameter=190;
					controlled->program_goal=Vector(-1,-1,-1);

					redrawmenu=2;
				} else if (key==fire_key) {
					if (controlled->program_parameter==0) controlled->program=PROGRAM_STOPDEFEND;
					controlled->program_goal=Vector(-1,-1,-1);

					killmenu(SELECTDISTANCE_MENU);
					newmenu(ROBOT_MENU);
					act_button=ROBOT4_BUTTON;
					if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
				} /* if */ 

				if(act_button!=ROBOT4_BUTTON) {
					STATUSBUTTON *b;
					b=getbutton(TARGET1_BUTTON);
					sprintf(b->text1,"%.2i MILES",controlled->program_parameter/2);
				}
			}
			break;

		case TARGETD_MENU:
			/* Browsing through the SELECT TARGET FOR DESTROYING MENU: */ 
			{
				int i;
				int buttons[3]={TARGET1_BUTTON,TARGET2_BUTTON,TARGET3_BUTTON};
				STATUSBUTTON *b;

				if (key==up_key) {
					int j=-1;
					for(i=0;i<3 && j==-1;i++) {
						if (act_button==buttons[i]) {
							j=i-1;
							if (j<0) j+=3;
							act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					redrawmenu=2;
				} /* if */ 
				if (key==down_key) {
					int j=-1;
					for(i=0;i<3 && j==-1;i++) {
						if (act_button==buttons[i]) {
							j=i+1;
							if (j>=3) j-=3;
							act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					redrawmenu=2;
				} /* if */ 

				for(i=0;i<3;i++) {
					b=getbutton(buttons[i]);
					if (b!=0) {
						b->r=0.0f;
						b->g=0.0f;
						b->b=0.8f;
					} /* if */ 
				} /* if */ 
				b=getbutton(act_button);
				if (b!=0) {
					b->r=0.5f;
					b->g=0.5f;
					b->b=1.0f;
				} /* if */ 
				
				if (key==fire_key) {
					switch(act_button) {
					case ORDERS_BUTTON:
						killmenu(TARGETD_MENU);
						newmenu(ROBOT_MENU);
						act_button=ROBOT4_BUTTON;
						break;
					case TARGET1_BUTTON:
						if (controlled->pieces[0] ||
							controlled->pieces[1] ||
							controlled->pieces[2]) {
							killmenu(TARGETD_MENU);
							newmenu(ROBOT_MENU);
							act_button=ROBOT4_BUTTON;
							controlled->program=PROGRAM_DESTROY;
							controlled->program_parameter=P_PARAM_ROBOTS;
							controlled->program_goal=Vector(-1,-1,-1);
							if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						} else {
							/* The robot has no standard WEAPONS!: */ 
							if (S_wrong!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_wrong,0),128);
						} /* if */ 
						break;
					case TARGET2_BUTTON:
						if (controlled->pieces[3]) {
							killmenu(TARGETD_MENU);
							newmenu(ROBOT_MENU);
							act_button=ROBOT4_BUTTON;
							controlled->program=PROGRAM_DESTROY;
							controlled->program_parameter=P_PARAM_EFACTORIES;
							controlled->program_goal=Vector(-1,-1,-1);
							if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						} else {
							/* The robot has no NUCLEAR weapons: */ 
							if (S_wrong!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_wrong,0),128);
						} /* if */ 
						break;
					case TARGET3_BUTTON:
						if (controlled->pieces[3]) {
							killmenu(TARGETD_MENU);
							newmenu(ROBOT_MENU);
							act_button=ROBOT4_BUTTON;
							controlled->program=PROGRAM_DESTROY;
							controlled->program_parameter=P_PARAM_WARBASES;
							controlled->program_goal=Vector(-1,-1,-1);
							if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						} else {
							/* The robot has no NUCLEAR weapons: */ 
							if (S_wrong!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_wrong,0),128);
						} /* if */ 
						break;
					} /* switch */ 
				} /* if */ 
			}
			break;

		case TARGETC_MENU:
			/* Browsing through the SELECT TARGET FOR CAPTURING MENU: */ 
			{
				int i;
				int buttons[3]={TARGET1_BUTTON,TARGET2_BUTTON,TARGET3_BUTTON};
				STATUSBUTTON *b;

				if (key==up_key) {
					int j=-1;
					for(i=0;i<3 && j==-1;i++) {
						if (act_button==buttons[i]) {
							j=i-1;
							if (j<0) j+=3;
							act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					redrawmenu=2;
				} /* if */ 
				if (key==down_key) {
					int j=-1;
					for(i=0;i<3 && j==-1;i++) {
						if (act_button==buttons[i]) {
							j=i+1;
							if (j>=3) j-=3;
							act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					redrawmenu=2;
				} /* if */ 

				for(i=0;i<3;i++) {
					b=getbutton(buttons[i]);
					if (b!=0) {
						b->r=0.0f;
						b->g=0.0f;
						b->b=0.8f;
					} /* if */ 
				} /* if */ 
				b=getbutton(act_button);
				if (b!=0) {
					b->r=0.5f;
					b->g=0.5f;
					b->b=1.0f;
				} /* if */ 

				if (key==fire_key) {
					switch(act_button) {
					case ORDERS_BUTTON:
						killmenu(TARGETC_MENU);
						newmenu(ROBOT_MENU);
						act_button=ROBOT4_BUTTON;
						break;
					case TARGET1_BUTTON:
						killmenu(TARGETC_MENU);
						newmenu(ROBOT_MENU);
						act_button=ROBOT4_BUTTON;
						controlled->program=PROGRAM_CAPTURE;
						controlled->program_parameter=P_PARAM_NFACTORIES;
						controlled->program_goal=Vector(-1,-1,-1);
						if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						break;
					case TARGET2_BUTTON:
						killmenu(TARGETC_MENU);
						newmenu(ROBOT_MENU);
						act_button=ROBOT4_BUTTON;
						controlled->program=PROGRAM_CAPTURE;
						controlled->program_parameter=P_PARAM_EFACTORIES;
						controlled->program_goal=Vector(-1,-1,-1);
						if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						break;
					case TARGET3_BUTTON:
						killmenu(TARGETC_MENU);
						newmenu(ROBOT_MENU);
						act_button=ROBOT4_BUTTON;
						controlled->program=PROGRAM_CAPTURE;
						controlled->program_parameter=P_PARAM_WARBASES;
						controlled->program_goal=Vector(-1,-1,-1);
						if (S_select!=0 && sound) Mix_Volume(Mix_PlayChannel(-1,S_select,0),128);
						break;
					} /* switch */ 
				} /* if */ 
			}
			break;
		} /* switch */ 
}

void NETHER::set_movement(){
	int x,y;
	int mx,my;
	int split, splity;
	float ratio;
	float view_matrix[16];
	float projection_matrix[16];
	int viewport[4];

	split = int((SCREEN_X*25.0F)/32.0F);
    if (show_radar) splity = int((SCREEN_Y*2.0F)/15.0F)+1;
    else splity = 0;
    ratio=float(split)/float(SCREEN_Y-splity);

	up_key_pressed=false;
	down_key_pressed=false;
	left_key_pressed=false;
	right_key_pressed=false;

	SDL_GetMouseState(&mx, &my);

	if(!mx || !my || mx>split || my>(SCREEN_Y-splity)) return; // mouse outside borders

	viewport[0]=0;
	viewport[1]=splity;
	viewport[2]=split;
	viewport[3]=SCREEN_Y-splity;

	// gluLookAt(viewp.x+camera.x*zoom, viewp.y+camera.y*zoom, viewp.z+camera.z*zoom,  viewp.x, viewp.y, viewp.z, 0,0,1);
	Vector pos(viewp.x+camera.x*zoom, viewp.y+camera.y*zoom, viewp.z+camera.z*zoom);
	Vector target(viewp.x, viewp.y, viewp.z);
	Vector up(0,0,1);
	set_view_matrix(pos, target, up, view_matrix);

    // glFrustum( -0.26*ratio, 0.26*ratio, -0.26, 0.26, 1, 1024 );
    set_projection( -0.26*ratio, 0.26*ratio, -0.26, 0.26, 1, 1024, projection_matrix);

	float z;
    // ship top center = 0.5, 0.5, 0.4
	if(!project(shipp.x+0.5, shipp.y+0.5, shipp.z+0.4, view_matrix, projection_matrix, viewport, &x, &y, &z)) return;

	if(act_menu==GENERAL_MENU){ // allow multiple directions
		int deg=int(float((atan2(my-y,mx-x)*180))/3.141592F)+180;

		// allow overlap (diagonal moving)
		if(deg>338 || deg<112) left_key_pressed=true;	// 15pi/8 - 5 pi/8
		if(deg>68  && deg<202) up_key_pressed=true;		// 3 pi/8 - 9 pi/8
		if(deg>158 && deg<292) right_key_pressed=true;	// 7 pi/8 - 13pi/8
		if(deg>248 || deg<22 ) down_key_pressed=true;	// 11pi/8 -   pi/8
	} else {
		// 0,0 is upper left
		if(mx>x && my>y) right_key_pressed=true;
		else if(mx>x && my<y) up_key_pressed=true;
		else if(mx<x && my>y) down_key_pressed=true;
		else if(mx<x && my<y) left_key_pressed=true;
	}
}

