// Patrick CEledio

#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>

// Definitions for rendering graphics
#define MEM_VRAM 		0x06000000
#define SCREEN_WIDTH	240
#define SCREEN_HEIGHT 	160

// Need these for essentially writing directly to frame buffer with VRAM 
typedef u16		M3LINE[SCREEN_WIDTH];
#define m3_mem	((M3LINE*)MEM_VRAM)

// Paddle and ball structure (To basically make rectangles)
struct rect { 
	int x, y, width, height;
};

// Draw pixel function; writing individual pixel values to frame buffer
void drawPixel(int x, int y, int color){
	m3_mem[y][x] = color;
};

// Draw paddle and ball
void drawRect(struct rect* cRect){
	for (int i = cRect->x; i < cRect->x + cRect->width; i++){
		for (int j = cRect->y; j < cRect->y + cRect->height; j++){
			drawPixel(i, j, 0x7FFF);
		}
	}
};

int main(void) {
	irqInit();
	irqEnable(IRQ_VBLANK);

	// Set GBA to mode 3; video memory
	SetMode( MODE_3 | BG2_ON );

	// Initialize Human paddle
	struct rect humanPaddle;
	humanPaddle.x = 1;
	humanPaddle.y = SCREEN_HEIGHT/2 - 24/2;
	humanPaddle.width = 8;
	humanPaddle.height = 24;

	// Initialize CPU paddle
	struct rect cpuPaddle;
	cpuPaddle.x = SCREEN_WIDTH - 8 - 1;
	cpuPaddle.y = SCREEN_HEIGHT/2 - 24/2;
	cpuPaddle.width = 8;
	cpuPaddle.height = 24;

	struct rect pongBall;
	pongBall.x = SCREEN_WIDTH/2 - 8/2;
	pongBall.y = SCREEN_HEIGHT/2 - 8/2;
	pongBall.width = 8;
	pongBall.height = 10;
	
	// Draw white central line on screen
	for (int j=0; j<SCREEN_HEIGHT; j++){
		drawPixel(SCREEN_WIDTH/2, j, 0x7FFF);
	}

	while (1) {
		VBlankIntrWait();

		// Draw user input
		scanKeys();

		// Draw player paddle on screen
		drawRect(&humanPaddle);
		
		// Draw comptuer paddle on screen
		drawRect(&cpuPaddle);

		// Draw pong ball on screen
		drawRect(&pongBall);
	}
}


