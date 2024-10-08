// Patrick CEledio

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba.h>

// Definitions for rendering graphics
#define MEM_VRAM 		0x06000000
#define SCREEN_WIDTH	240
#define SCREEN_HEIGHT 	160
#define DASH_LENGTH		6
#define GAP_LENGTH		4
#define LINE_X_POSITION (SCREEN_WIDTH / 2)
#define PLAYERPADDLE_COLOR 0x7C00
#define CPUPADDLE_COLOR 0x03E0
#define DIGIT_WIDTH  5
#define DIGIT_HEIGHT 7
#define LETTER_WIDTH  5
#define LETTER_HEIGHT 7

// Need these for essentially writing directly to frame buffer with VRAM 
typedef u16		M3LINE[SCREEN_WIDTH];
#define m3_mem	((M3LINE*)MEM_VRAM)

// Global variables for score
int playerScore = 0;
int cpuScore = 0;

// Gotta nerf the CPU AI
int cpuPaddleSpeed = 4;
int cpuReactionDelay = 0;

// Store how many frames has passed
int frameCounter = 0;
int freezeCounter = 0;

// Checks if the winning message is displayed for CPU or Player
bool isWinningMsgDisplayed = false; 
bool isGameFrozen = false;

// Okay, I need dedicated header and source files now; these are digits for score
// 0 = pixels left blank; 1 = pixels to be drawn
const u8 digits[10][DIGIT_HEIGHT] = {
    // 0
    {
        0b01110,
        0b10001,
        0b10011,
        0b10101,
        0b11001,
        0b10001,
        0b01110
    },
    // 1
    {
        0b00100,
        0b01100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b01110
    },
    // 2
    {
        0b01110,
        0b10001,
        0b00001,
        0b00110,
        0b01000,
        0b10000,
        0b11111
    },
    // 3
    {
        0b01110,
        0b10001,
        0b00001,
        0b00110,
        0b00001,
        0b10001,
        0b01110
    },
    // 4
    {
        0b00010,
        0b00110,
        0b01010,
        0b10010,
        0b11111,
        0b00010,
        0b00010
    },
    // 5
    {
        0b11111,
        0b10000,
        0b11110,
        0b00001,
        0b00001,
        0b10001,
        0b01110
    },
    // 6
    {
        0b00110,
        0b01000,
        0b10000,
        0b11110,
        0b10001,
        0b10001,
        0b01110
    },
    // 7
    {
        0b11111,
        0b00001,
        0b00010,
        0b00100,
        0b01000,
        0b01000,
        0b01000
    },
    // 8
    {
        0b01110,
        0b10001,
        0b10001,
        0b01110,
        0b10001,
        0b10001,
        0b01110
    },
    // 9
    {
        0b01110,
        0b10001,
        0b10001,
        0b01111,
        0b00001,
        0b00010,
        0b11100
    }
};

// Jesus Christ Bitmap

const u8 alphabet[26][LETTER_HEIGHT] = {
    // A
    {
        0b01110,
        0b10001,
        0b10001,
        0b11111,
        0b10001,
        0b10001,
        0b10001
    },
    // B
    {
        0b11110,
        0b10001,
        0b10001,
        0b11110,
        0b10001,
        0b10001,
        0b11110
    },
    // C
    {
        0b01110,
        0b10001,
        0b10000,
        0b10000,
        0b10000,
        0b10001,
        0b01110
    },
    // D
    {
        0b11110,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b11110
    },
    // E
    {
        0b11111,
        0b10000,
        0b10000,
        0b11110,
        0b10000,
        0b10000,
        0b11111
    },
    // F
    {
        0b11111,
        0b10000,
        0b10000,
        0b11110,
        0b10000,
        0b10000,
        0b10000
    },
    // G
    {
        0b01110,
        0b10001,
        0b10000,
        0b10111,
        0b10001,
        0b10001,
        0b01110
    },
    // H
    {
        0b10001,
        0b10001,
        0b10001,
        0b11111,
        0b10001,
        0b10001,
        0b10001
    },
    // I
    {
        0b01110,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b01110
    },
    // J
    {
        0b00111,
        0b00010,
        0b00010,
        0b00010,
        0b00010,
        0b10010,
        0b01100
    },
    // K
    {
        0b10001,
        0b10010,
        0b10100,
        0b11000,
        0b10100,
        0b10010,
        0b10001
    },
    // L
    {
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b11111
    },
    // M
    {
        0b10001,
        0b11011,
        0b10101,
        0b10001,
        0b10001,
        0b10001,
        0b10001
    },
    // N
    {
        0b10001,
        0b11001,
        0b10101,
        0b10011,
        0b10001,
        0b10001,
        0b10001
    },
    // O
    {
        0b01110,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b01110
    },
    // P
    {
        0b11110,
        0b10001,
        0b10001,
        0b11110,
        0b10000,
        0b10000,
        0b10000
    },
    // Q
    {
        0b01110,
        0b10001,
        0b10001,
        0b10001,
        0b10101,
        0b10010,
        0b01101
    },
    // R
    {
        0b11110,
        0b10001,
        0b10001,
        0b11110,
        0b10100,
        0b10010,
        0b10001
    },
    // S
    {
        0b01111,
        0b10000,
        0b10000,
        0b01110,
        0b00001,
        0b00001,
        0b11110
    },
    // T
    {
        0b11111,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00100
    },
    // U
    {
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b01110
    },
    // V
    {
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b01010,
        0b00100
    },
    // W
    {
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10101,
        0b11011,
        0b10001
    },
    // X
    {
        0b10001,
        0b10001,
        0b01010,
        0b00100,
        0b01010,
        0b10001,
        0b10001
    },
    // Y
    {
        0b10001,
        0b10001,
        0b01010,
        0b00100,
        0b00100,
        0b00100,
        0b00100
    },
    // Z
    {
        0b11111,
        0b00001,
        0b00010,
        0b00100,
        0b01000,
        0b10000,
        0b11111
    }
};

// Paddle and ball structure (To basically make rectangles)
struct rect { 
	int x, y, width, height, velocityX, velocityY, prevX, prevY;
};

// Draw pixel function; writing individual pixel values to frame buffer
void inline drawPixel(int x, int y, int color){
	m3_mem[y][x] = color;
};

// Draw ball
void drawBall(struct rect* cRect){
	for (int i = cRect->x; i < cRect->x + cRect->width; i++){
		for (int j = cRect->y; j < cRect->y + cRect->height; j++){
			drawPixel(i, j, 0x7FCF);
		}
	}
};

void drawCpuPaddle(struct rect* cRect){
	for (int i = cRect->x; i < cRect->x + cRect->width; i++){
		for (int j = cRect->y; j < cRect->y + cRect->height; j++){
			drawPixel(i, j, CPUPADDLE_COLOR);
		}
	}
};


void drawPlayerPaddle(struct rect* cRect){
	for (int i = cRect->x; i < cRect->x + cRect->width; i++){
		for (int j = cRect->y; j < cRect->y + cRect->height; j++){
			drawPixel(i, j, PLAYERPADDLE_COLOR);
		}
	}
};

void drawNumber(int num, int x, int y, u16 color){
	// Ensure number is a single digit
	if (num < 0 || num > 9){
		return ;
	} 


	// Loop over digit's bitmap and draw corresponding pixels
	for (int row = 0; row < DIGIT_HEIGHT; row++){
		for (int col = 0; col < DIGIT_WIDTH; col++){
			if (digits[num][row] & (1 << (DIGIT_WIDTH - 1 - col))) {
				drawPixel(x+col, y + row, color);
			}
		}
	}
};

void drawScore(int score, int x, int y, u16 color){
	int tens = score / 10; // Tens digit
	int units = score % 10; // Ones digit

	if (tens > 0){
		drawNumber(tens, x, y, color);
	}

	drawNumber(units, x + DIGIT_WIDTH + 1, y, color);
}

void drawLetter(char letter, int x, int y, u16 color) {
    // Ensure the letter is uppercase A-Z
    if (letter < 'A' || letter > 'Z') {
        return;
    }

    int index = letter - 'A';  // Convert ASCII value to index (0-25)
	// struct rect pixel;

    // Loop over the letter's bitmap and draw the corresponding pixels
    for (int row = 0; row < LETTER_HEIGHT; row++) {
        for (int col = 0; col < LETTER_WIDTH; col++) {
            if (alphabet[index][row] & (1 << (LETTER_WIDTH - 1 - col))) {
				drawPixel(x + col, y + row, color);
            }
        }
    }
}

void drawTitle(const char* title, int x, int y, u16 color) {
    int offset = 0;

    // Loop through each character in the string
    for (int i = 0; title[i] != '\0'; i++) {
        // If the character is a space, skip it with an offset
        if (title[i] == ' ') {
            offset += LETTER_WIDTH + 1;  // Add space between letters
            continue;
        }

        // Draw the letter at the current position
        drawLetter(title[i], x + offset, y, color);

        // Move the x position to the right for the next letter
        offset += LETTER_WIDTH + 1;  // Add space between letters
    }
}

void clearPixelArea(int x, int y){
	// Create an eraser area size
	int width = (DIGIT_WIDTH + 1) * 2; 
	int height = DIGIT_HEIGHT;

	for (int i = x; i < x + width; i++){
		for (int j = y; j < y + height; j++) { 
			drawPixel(i, j, 0x0000); // Clear area with black (0x0000)
		}
	}
}

// Clear the pixels when objects moves
void clearRect(struct rect* cRect){
	for (int i = cRect->prevX; i < cRect->prevX + cRect->width; i++){
		for (int j = cRect->prevY; j < cRect->prevY + cRect->height; j++){
			drawPixel(i, j, 0x0000);
		}
	}
};

// Initializes pong ball the first time its called; acts as a reset afterwards
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
void checkCollision(struct rect* pongBall, struct rect* playerPaddle, struct rect* cpuPaddle){
		// If playerPaddle or cpuPaddle is next to ceiling or floor, stop it
		if((playerPaddle->y <= 0 && playerPaddle->velocityY < 0) || 
		   ((playerPaddle->y >= SCREEN_HEIGHT - playerPaddle->height) && 
			playerPaddle->velocityY > 0))
		{
			playerPaddle->velocityY = 0;
		}

		// Check collision between pong ball and ceiling or floor
		if((pongBall->y <= 0 && pongBall->velocityY < 0)||
			((pongBall->y >= SCREEN_HEIGHT - pongBall->height)&&
			 pongBall->velocityY > 0))
			{
				pongBall->velocityY = -pongBall->velocityY;
		}

		// Check collision between pong ball and left wall (Player misses)
		if (pongBall->x <= 0){
			cpuScore++;
			clearRect(pongBall);
			resetBall(pongBall);
		}

		// Check collision between pong ball and right wall (CPU misses)
		if (pongBall->x + pongBall->width >= SCREEN_WIDTH){
			playerScore++;
			clearRect(pongBall);
			resetBall(pongBall);
		}



		// Check collision betwen pong ball and cpu paddle
		if (pongBall->x + pongBall->width >= cpuPaddle->x &&
			pongBall->x <= cpuPaddle->x + cpuPaddle->width &&
			pongBall->y + pongBall->height >= cpuPaddle->y &&
			pongBall->y <= cpuPaddle->y + cpuPaddle->height){
				pongBall->velocityX = -pongBall->velocityX;
			}

		// // Check collision between pong ball and player paddle
		if (pongBall->x <= playerPaddle->x + playerPaddle->width &&
			pongBall->x + pongBall->width >= playerPaddle->x &&
			pongBall->y + pongBall->height >= playerPaddle->y &&
			pongBall->y <= playerPaddle->y + playerPaddle->height){
				pongBall->velocityX = -pongBall->velocityX;
			}
};

void static inline updateBall(struct rect* pongBall){
	pongBall->prevX = pongBall->x;
	pongBall->prevY = pongBall->y;
	pongBall->x += pongBall->velocityX;
	pongBall->y += pongBall->velocityY;
};

void updateCpuPaddle(struct rect* cpuPaddle, struct rect* pongBall){
	// Update cpuPaddle position
	cpuPaddle->prevY = cpuPaddle->y;

	// Make CPU paddle delayed in reaction
	if (cpuReactionDelay == 0){
		if (pongBall->y < cpuPaddle->y){
			cpuPaddle->y -= cpuPaddleSpeed;
		}
		else if (pongBall->y + pongBall->height > cpuPaddle->y + cpuPaddle->height){
			cpuPaddle->y += cpuPaddleSpeed;
		}
		cpuReactionDelay = 2; 
	} else {
		cpuReactionDelay--;
	}

	// Make the CPU Paddle miss randomly
    if (rand() % 4 == 0) {  // 1 in 4 chance the CPU won't move
        return;
    }
};

void resetPositions(struct rect* playerPaddle, struct rect* cpuPaddle, struct rect* pongBall) {

	// Clear pixel footsteps 
	clearRect(playerPaddle);
	clearRect(cpuPaddle);
	clearRect(pongBall);
	
	// Reset player paddle position
	playerPaddle->x = 1;
	playerPaddle->y = SCREEN_HEIGHT / 2 - playerPaddle->height / 2;
	playerPaddle->prevX = playerPaddle->x;
	playerPaddle->prevY = playerPaddle->y;
	playerPaddle->velocityX = 0;
	playerPaddle->velocityY = 0;

	// Reset CPU paddle position
	cpuPaddle->x = SCREEN_WIDTH - playerPaddle->width - 1;
	cpuPaddle->y = SCREEN_HEIGHT / 2 - cpuPaddle->height / 2;
	cpuPaddle->prevX = cpuPaddle->x;
	cpuPaddle->prevY = cpuPaddle->y;
	cpuPaddle->velocityX = 0;
	cpuPaddle->velocityY = 0;

	// Reset ball position and velocity
	pongBall->x = SCREEN_WIDTH / 2 - pongBall->width / 2;
	pongBall->y = SCREEN_HEIGHT / 2 - pongBall->height / 2;
	pongBall->prevX = pongBall->x;
	pongBall->prevY = pongBall->y;
    
    // Randomize initial direction of the ball
    pongBall->velocityX = (rand() % 2 == 0) ? 2 : -2;
    pongBall->velocityY = (rand() % 2 == 0) ? 2 : -2;
}


void checkWinCondition(struct rect* playerPaddle, struct rect* cpuPaddle, struct rect* pongBall){			
		if (playerScore == 5){
			drawTitle("PLAYER WINS", 10, 80, 0x02FF);
			playerScore = 0;
			cpuScore = 0;        
			isWinningMsgDisplayed = true;
			resetPositions(playerPaddle, cpuPaddle, pongBall);  // Reset positions
			isGameFrozen = true;
			freezeCounter = 0;  // Start freeze period
        	frameCounter = 0;  // Reset counter when title is displayed
			resetPositions(playerPaddle, cpuPaddle, pongBall); 
		}else if (cpuScore == 5){
			drawTitle("CPU WINS", 180, 80, 0x03FF);
			playerScore = 0;
			cpuScore = 0;		
			isWinningMsgDisplayed = true;
			resetPositions(playerPaddle, cpuPaddle, pongBall); 
			isGameFrozen = true;
			freezeCounter = 0;
        	frameCounter = 0;  
			resetPositions(playerPaddle, cpuPaddle, pongBall); 
		}
};

void clearTitle() {
    // Clear the area where the title is displayed (adjust size as needed)
    for (int y = 80; y < 90; y++) {  // Adjust this range to cover the title area
        for (int x = 10; x < 230; x++) {  // Clear from left to right
            drawPixel(x, y, 0x0000);  // Clear with black
        }
    }
}

void initGBA(){
	irqInit();
	irqEnable(IRQ_VBLANK);

	// Set GBA to mode 3; video memory
	// SetMode( MODE_3 | BG2_ON );
	REG_DISPCNT = MODE_3 | BG0_ENABLE | BG1_ENABLE | BG2_ENABLE; // This line probably does what the above line does

};

void inline drawCenterLine(){
	// Draw white central line on screen
	for (int vertical_line = 0; vertical_line < SCREEN_HEIGHT; vertical_line += (DASH_LENGTH + GAP_LENGTH)){
		for (int dashY = vertical_line; dashY < vertical_line + DASH_LENGTH && dashY < SCREEN_HEIGHT; dashY++){
			drawPixel(LINE_X_POSITION, dashY, 0x7FFF);
		}
	}

};

int main(void) {
	// Sets up the necessary configurations for the GBA to run this game
	initGBA();

	// Seed the RNG for resetBall()
	srand(time(NULL));

	// Initialize player paddle obj
	struct rect playerPaddle;
	playerPaddle.x = 1;
	playerPaddle.y = SCREEN_HEIGHT/2 - 24/2;
	playerPaddle.prevX = playerPaddle.x;
	playerPaddle.prevY = playerPaddle.y;
	playerPaddle.width = 8;
	playerPaddle.height = 32;
	playerPaddle.velocityX = 0;
	playerPaddle.velocityY = 0;

	// Initialize CPU paddle obj
	struct rect cpuPaddle;
	cpuPaddle.x = SCREEN_WIDTH - 8 - 1;
	cpuPaddle.y = SCREEN_HEIGHT/2 - 24/2;
	cpuPaddle.prevX = cpuPaddle.x;
	cpuPaddle.prevY = cpuPaddle.y;
	cpuPaddle.width = 8;
	cpuPaddle.height = 32;
	cpuPaddle.velocityX = 0;
	cpuPaddle.velocityY = 0;

	// Initialize pongBall obj
	struct rect pongBall;
	pongBall.x = SCREEN_WIDTH/2 - 8/2;
	pongBall.y = SCREEN_HEIGHT/2 - 8/2;
	pongBall.prevX = pongBall.x;
	pongBall.prevY = pongBall.y;
	pongBall.width = 8;
	pongBall.height = 10;
	pongBall.velocityX = 0;
	pongBall.velocityY = 0;

	// Initialize position and velocity of pongball to default
	resetBall(&pongBall);



	while (1) {
		// Updates game objects before the next frame is drawn
		VBlankIntrWait();

		// Draw line down center of pong field
		drawCenterLine();

		drawTitle("PATRICK CELEDIO", 10, 140, 0x02FF);
		drawTitle("GBA PONG", 180, 140, 0x03FF);

		// Respond to user input
		scanKeys();
		int keys_pressed = keysDown();
		int keys_released = keysUp();

		// playerPaddle vertical movement logic 

		// If a movement key is not pressed or released; velocity to 0
		if ((keys_released & KEY_UP) || (keys_released & KEY_DOWN)){
			// Move up
			playerPaddle.velocityY = 0;
		}

		// If up is pressed; move playerPaddle up
		if ((keys_pressed & KEY_UP) && playerPaddle.y >= 0){
			// Essentially, velocity is -2
			playerPaddle.velocityY = -2;
		}

		// If down is pressed; velocity to +2
		if((keys_pressed & KEY_DOWN) && playerPaddle.y 
			<= SCREEN_HEIGHT - playerPaddle.height){
			// Essentially, velocity is +2
			playerPaddle.velocityY = 2;
		}

		// Update movement speed of player paddle
		playerPaddle.y += playerPaddle.velocityY;

		updateBall(&pongBall);
		updateCpuPaddle(&cpuPaddle, &pongBall);
		checkCollision(&pongBall, &playerPaddle, &cpuPaddle);

		// Clear pixel footsteps 
		clearRect(&playerPaddle);
		clearRect(&cpuPaddle);
		clearRect(&pongBall);

		// Draw the following objects on screen
		drawPlayerPaddle(&playerPaddle);
		drawCpuPaddle(&cpuPaddle);
		drawBall(&pongBall);

		checkWinCondition(&playerPaddle, &cpuPaddle, &pongBall);

		clearPixelArea(20, 10);
		clearPixelArea(200,10);
		drawScore(playerScore, 20, 10, 0x7FFF);
		drawScore(cpuScore, 200, 10, 0x7FFF);

		// Update position
		playerPaddle.prevX = playerPaddle.x;
		playerPaddle.prevY = playerPaddle.y;
		pongBall.prevX = pongBall.x;

		if (isWinningMsgDisplayed) {
			frameCounter++;

			// After 300 frames (~5 seconds at 60 FPS), clear the title
			if (frameCounter >= 150) {
				clearTitle();  // Implement this function to clear the title area
				isWinningMsgDisplayed = false;  // Reset the flag
       		}
    	}
	}// End while() 
}