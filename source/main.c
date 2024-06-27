// Patrick CEledio

#include <stdlib.h>
#include <time.h>

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
	int x, y, width, height, velocityX, velocityY, prevX, prevY;
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

// Clear the pixels when objects moves
void clearRect(struct rect* cRect){
	for (int i = cRect->prevX; i < cRect->prevX + cRect->width; i++){
		for (int j = cRect->prevY; j < cRect->prevY + cRect->height; j++){
			drawPixel(i, j, 0x0000);
		}
	}
};

// Reset pong ball
void resetBall(struct rect* cRect){
	// Set ball to center of the screen
	cRect->x = (SCREEN_WIDTH/2) - (cRect->width/2);
	cRect->y = (SCREEN_HEIGHT/2) - (cRect->height/2);

	// Randomize initizal direction of the ball
	cRect->velocityX = (rand() % 2 == 0) ? 2 : -2;
	cRect->velocityY = (rand() % 2 == 0) ? 2 : -2;

	// Reset the previous position to the current position
	cRect->prevX = cRect->x;
	cRect->prevY = cRect->y;
};

// Check pongBall collision
void checkCollision(struct rect* pongBall, struct rect* humanPaddle, struct rect* cpuPaddle){
		// If humanPaddle or cpuPaddle is next to ceiling or floor, stop it
		if((humanPaddle->y <= 0 && humanPaddle->velocityY < 0) || ((humanPaddle->y >= SCREEN_HEIGHT - humanPaddle->height) 
			&& humanPaddle->velocityY > 0))
		{
			humanPaddle->velocityY = 0;
		}

		// Check collision between pong ball and left wall
		if (pongBall->x <= 0){
			clearRect(pongBall);
			resetBall(pongBall);

		// Check collision between pong ball and right wall
		}else if (pongBall->x + pongBall->width >= SCREEN_WIDTH){
			clearRect(pongBall);
			resetBall(pongBall);

		}

		// Check collision betwen pong ball and cpu paddle
		if (pongBall->x <= cpuPaddle->x + cpuPaddle->width &&
			pongBall->x + pongBall->width >= cpuPaddle->x &&
			pongBall->y + pongBall->height >= cpuPaddle->y &&
			pongBall->y <= cpuPaddle->y + cpuPaddle->height){
				pongBall->velocityX = -pongBall->velocityX;
			}

		// // Check collision between pong ball and human paddle
		// if (pongBall->x + pongBall->width >= humanPaddle->x &&
		// 	pongBall->x <= humanPaddle->x + humanPaddle->width &&
		// 	pongBall->y + pongBall->height >= ){
		// 		pongBall->velocityX = -pongBall->velocityX;
		// 	}


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
	humanPaddle.prevX = humanPaddle.x;
	humanPaddle.prevY = humanPaddle.y;
	humanPaddle.width = 8;
	humanPaddle.height = 24;
	humanPaddle.velocityX = 0;
	humanPaddle.velocityY = 0;

	// Initialize CPU paddle
	struct rect cpuPaddle;
	cpuPaddle.x = SCREEN_WIDTH - 8 - 1;
	cpuPaddle.y = SCREEN_HEIGHT/2 - 24/2;
	cpuPaddle.prevX = cpuPaddle.x;
	cpuPaddle.prevY = cpuPaddle.y;
	cpuPaddle.width = 8;
	cpuPaddle.height = 24;
	cpuPaddle.velocityX = 0;
	cpuPaddle.velocityY = 0;

	struct rect pongBall;
	pongBall.x = SCREEN_WIDTH/2 - 8/2;
	pongBall.y = SCREEN_HEIGHT/2 - 8/2;
	pongBall.prevX = pongBall.x;
	pongBall.prevY = pongBall.y;
	pongBall.width = 8;
	pongBall.height = 10;
	pongBall.velocityX = 0;
	pongBall.velocityY = 0;

	while (1) {
		// Draw white central line on screen
		for (int j=0; j<SCREEN_HEIGHT; j++){
			drawPixel(SCREEN_WIDTH/2, j, 0x7FFF);
		}

		VBlankIntrWait();

		// Respond to user input
		scanKeys();
		int keys_pressed = keysDown();
		int keys_released = keysUp();

		// humanPaddle vertical movement logic 

		// If a movement key is not pressed or released; velocity to 0
		if ((keys_released & KEY_UP) || (keys_released & KEY_DOWN)){
			// Move up
			humanPaddle.velocityY = 0;
		}

		// If up is pressed; move humanPaddle up
		if ((keys_pressed & KEY_UP) && humanPaddle.y >= 0){
			// Essentially, velocity is -2
			humanPaddle.velocityY = -2;
		}

		// If down is pressed; velocity to +2
		if((keys_pressed & KEY_DOWN) && humanPaddle.y 
			<= SCREEN_HEIGHT - humanPaddle.height){
			// Essentially, velocity is +2
			humanPaddle.velocityY = 2;
		}

		checkCollision(&pongBall, &humanPaddle, &cpuPaddle);

		// Update movement speed of human paddle
		humanPaddle.y += humanPaddle.velocityY;

		// Pong ball movement
		pongBall.velocityX = 2;
		pongBall.x += pongBall.velocityX;

		// Clear pixel footsteps 
		clearRect(&humanPaddle);
		clearRect(&cpuPaddle);
		clearRect(&pongBall);

		// Draw the following objects on screen
		drawRect(&humanPaddle);
		drawRect(&cpuPaddle);
		drawRect(&pongBall);

		// Update position
		humanPaddle.prevX = humanPaddle.x;
		humanPaddle.prevY = humanPaddle.y;
		pongBall.prevX = pongBall.x;


	}
}


