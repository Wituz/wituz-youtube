#pragma once

typedef struct {
	int x, y, height;
	Line line;
} Bat;

typedef struct {
	int size, x, y, is_moving, speed_x, speed_y;
	Box box;
} Ball;

typedef struct {
	int score_left, score_right;
} Scoreboard;

Scoreboard scoreboard;

Scoreboard createScoreboard() {
	Scoreboard scoreboard;
	scoreboard.score_left = 0;
	scoreboard.score_right = 0;
	return scoreboard;
}

Ball createBall() {
	Ball ball;
	ball.size = 2;
	ball.x = SCREEN_WIDTH / 2 - ball.size / 2;
	ball.y = SCREEN_HEIGHT / 2 - ball.size / 2;
	ball.box = createBox(createColor(0, 255, 0), ball.x, ball.y, ball.x + ball.size, ball.y + ball.size);
	ball.is_moving = 0;
	ball.speed_x = 0;
	ball.speed_y = 0;
	return ball;
}

Ball accelerateBall(Ball ball) {
	if(ball.speed_x < 0) ball.speed_x -= rand() % 2;
	else ball.speed_x += rand() % 2;
	if(ball.speed_y < 0) ball.speed_y -= rand() % 2;
	else ball.speed_y += rand() % 2;
	return ball;
}

Ball resetBall(Ball ball) {
	ball = createBall();
	return ball;
}

Ball moveBall(Ball ball, Bat batLeft, Bat batRight) {
	if(!ball.is_moving) return ball;
	ball.x += ball.speed_x;
	ball.y += ball.speed_y;
	ball.box = moveBox(ball.box, ball.x, ball.y);

	if(ball.x < batLeft.x && ball.x > 10 && ball.y > batLeft.y && ball.y < batLeft.y + batLeft.height) {
		ball.speed_x = ball.speed_x *= -1;
		ball.x = batLeft.x;
		ball = accelerateBall(ball);
	}

	if(ball.x > batRight.x && ball.x < SCREEN_WIDTH - 10 && ball.y > batRight.y && ball.y < batRight.y + batRight.height) {
		ball.speed_x = ball.speed_x *= -1;
		ball.x = batRight.x;
		ball = accelerateBall(ball);
	}

	if(ball.y > SCREEN_HEIGHT - 20) {
		ball.speed_y = ball.speed_y *= -1;
		ball.y = SCREEN_HEIGHT - 20;
	}

	if(ball.y < 20) {
		ball.speed_y = ball.speed_y *= -1;
		ball.y = 20;
	}

	if(ball.x < -5) {
		ball = resetBall(ball);
		scoreboard.score_right ++;
	}

	if(ball.x > SCREEN_WIDTH + 5) {
		ball = resetBall(ball);
		scoreboard.score_left ++;
	}

	return ball;
}

Ball kickBall(Ball ball) {
	if(ball.is_moving) return ball;
	ball.speed_x = 1 + rand() % 2 - rand() % 4;
	ball.speed_y = 1 + rand() % 2 - rand() % 2;
	ball.is_moving = 1;
	return ball;
}

void drawBall(Ball ball) {
	drawBox(ball.box);
}

Bat createBat(int x) {
	Bat bat;

	bat.x = x;
	bat.y = SCREEN_HEIGHT / 2 - 30;
	bat.height = 60;
	bat.line = createLine(createColor(255, 255, 255), bat.x, bat.y, bat.x, bat.y + bat.height);

	return bat;
}

Bat moveBat(Bat bat, int delta_y) {
	if(delta_y < 0 && bat.y + delta_y <= 22) {
		bat.y = 20;
		return bat;
	}

	if(delta_y > 0 && bat.y + bat.height + delta_y > SCREEN_HEIGHT - 22) {
		bat.y = SCREEN_HEIGHT - 20 - bat.height;
		return bat;
	}

	bat.y += delta_y;
	bat.line = moveLine(bat.line, bat.x, bat.y, bat.x, bat.y + bat.height);
	return bat;
}

void drawBat(Bat bat) {
	drawLine(bat.line);
}
