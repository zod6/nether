EXE = nether
SRC = src
PREFIX = /usr
EXTRADIR = ./build/linux

GAMEDIR = $(PREFIX)/games
STARTUP = $(GAMEDIR)/$(EXE)
BINDIR = $(PREFIX)/share/games/$(EXE)
ICNDIR = $(PREFIX)/share/pixmaps
APPDIR = $(PREFIX)/share/applications

DATA = maps models sound textures
ICON = nether.png
DESKTOP = nether.desktop

OBJS = \
	$(SRC)/3dobject-ase.o	$(SRC)/3dobject.o	$(SRC)/bitmap.o		$(SRC)/bullet.o		\
	$(SRC)/cmc.o		$(SRC)/construction.o	$(SRC)/enemy_ai.o	$(SRC)/glprintf.o	\
	$(SRC)/main.o		$(SRC)/mainmenu.o	$(SRC)/maps.o		$(SRC)/menu.o		\
	$(SRC)/myglutaux.o	$(SRC)/nether.o		$(SRC)/nethercycle.o	$(SRC)/netherdebug.o	\
	$(SRC)/nethersave.o	$(SRC)/particles.o	$(SRC)/piece3dobject.o	$(SRC)/quaternion.o	\
	$(SRC)/radar.o		$(SRC)/robot_ai.o	$(SRC)/robots.o		$(SRC)/shadow3dobject.o	\
	$(SRC)/vector.o		$(SRC)/debug.o		$(SRC)/filehandling.o	$(SRC)/matrix.o

CXX = g++
CFLAGS = -g3 -O3 `sdl-config --cflags` -I/usr/X11R6/include -Wall
LDFLAGS = `sdl-config --libs` -L/usr/X11R6/lib/ -lSDL_mixer -lGL -lGLU -lglut
RM = rm -f
CP = cp -r
MD = mkdir -p
ECHO = echo
CHMOD = chmod
STRIP = strip

all: $(EXE)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

$(EXE): $(OBJS)
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@$(STRIP) $@
	@$(ECHO) " o If there are no errors, the game compiled succesfully"

clean:
	@$(ECHO) " o Cleaning up source directory"
	@$(RM) $(SRC)/*.o $(SRC)/*.bak core $(EXE)

install: all
	@$(ECHO) " o Creating install directory $(BINDIR)"
	@$(MD) "$(BINDIR)"
	@$(ECHO) " o Installing game and data to $(BINDIR)"
	@$(CP) "$(EXE)" $(DATA) "$(BINDIR)"
	@$(ECHO) " o Creating startup script $(STARTUP)"
	@$(MD) "$(GAMEDIR)"
	@$(ECHO) "#!/bin/sh" >"$(STARTUP)"
	@$(ECHO) "cd \"$(BINDIR)\"; ./$(EXE); cd -" >>"$(STARTUP)"
	@$(CHMOD) 755 "$(STARTUP)"
	@$(ECHO) " o Creating application menu entry"
	@$(MD) "$(ICNDIR)"
	@$(CP) "$(EXTRADIR)/$(ICON)" "$(ICNDIR)"
	@$(MD) "$(APPDIR)"
	@$(CP) "$(EXTRADIR)/$(DESKTOP)" "$(APPDIR)"
	@$(ECHO) ""

uninstall:
	@$(ECHO) " o Removing game and data from $(BINDIR)"
	@$(RM) -r "$(BINDIR)"
	@$(ECHO) " o Removing startup script $(STARTUP)"
	@$(RM) "$(STARTUP)"
	@$(ECHO) " o Removing application menu entry"
	@$(RM) "$(ICNDIR)/$(ICON)" "$(APPDIR)/$(DESKTOP)"
	@$(ECHO) ""

