#include <jni.h>
#include <string>
#include <vector>
#include <random>

//Representation if each entity of the game
struct Ball
{
    float x, y, dx, dy, speed, radius;
};

struct Paddle
{
    float x, y, width, height;
};

struct Brick
{
    float x, y;
    bool destroyed;
    int type; //0 = normal, 1 = strong
    int hits;
};


std::vector<Brick> bricks;
Ball ball;
Paddle paddle;
int score, lives;
float ballSpeedIncrement;


void createLevel()
{
    //We create the ball, the paddle and the bricks

    score = 0;
    lives = 3;

    ball = { 0.5f, 0.8f, 0.01f, -0.01f, 0.55f, 0.02f };
    ballSpeedIncrement = 0.00035f;

    paddle = { 0.4f, 0.92f, 0.2f, 0.03f };

    std::random_device rd;  // Obtain a random seed
    std::mt19937 gen(rd()); // Seed the generator
    std::uniform_int_distribution<int> dist(0, 2);

    bricks.clear();
    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 7; ++j)
        {
            float Xpadding = 0.14f;
            float Ypadding = 0.05f;

            float Xoffset = 0.02;
            float Yoffset = 0.15;

            int randomNumber = rand() % 3;
            int type = randomNumber > 0 ? 0 : 1;  // Randomly assign types
            int hits = (type == 1) ? 2 : 1; // Strong bricks take 2 hits

            bricks.push_back({Xoffset + static_cast<float>(j) * Xpadding, Yoffset + static_cast<float>(i) * Ypadding, false,type, hits});
        }
    }
}

bool areAllBricksDestroyed()
{
    for(const auto& brick : bricks)
    {
        if(!brick.destroyed)
        {
            return false;
        }
    }

    return true;
}

bool checkAABBCollision(float x1, float y1, float w1, float h1,
                        float x2, float y2, float w2, float h2)
{
    return (x1 < x2 + w2 && x1 + w1 > x2 &&
            y1 < y2 + h2 && y1 + h1 > y2);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_breakout_Game_nativeInit(JNIEnv* env, jobject obj)
{
    createLevel();
}



extern "C" JNIEXPORT void JNICALL
Java_com_example_breakout_Game_nativeUpdate(JNIEnv* env, jobject obj)
{
    ball.x += ball.dx * ball.speed;
    ball.y += ball.dy * ball.speed;

    //Ball collides the screen sides
    if (ball.x < 0 || ball.x > 1)
    {
        ball.dx = -ball.dx;
    }

    //Ball collides with the top of the screen
    if (ball.y < 0.15f)
    {
        ball.dy = -ball.dy;
    }

    // Check ball collision with paddle
    if (checkAABBCollision(ball.x - ball.radius, ball.y - ball.radius,
                           ball.radius * 2, ball.radius * 2,
                           paddle.x, paddle.y, paddle.width, 0.02f))
    {

        // Ensure the ball only bounces up, not into the paddle
        if (ball.dy > 0)
        {
            ball.dy = -ball.dy;
        }
    }

    //Ball goes below the paddle
    if (ball.y > 1)
    {
        lives--;
        if (lives <= 0) // Trigger game over screen
        {
            jclass gameViewClass = env->GetObjectClass(obj);
            jmethodID gameOverMethod = env->GetMethodID(gameViewClass, "showGameOverScreen", "()V");
            if (gameOverMethod)
            {
                env->CallVoidMethod(obj, gameOverMethod);
            }
        }
        else  //Reset ball position
        {
            ball = { 0.5f, 0.8f, 0.01f, -0.01f, 0.55f, 0.02f };
        }

    }

    //Check ball collision with bricks
    for (auto& brick: bricks)
    {
        if (!brick.destroyed && checkAABBCollision(ball.x - ball.radius, ball.y - ball.radius,
                                                   ball.radius * 2, ball.radius * 2,
                                                   brick.x, brick.y, 0.1f, 0.05f))
        {
            brick.hits--;
            if(brick.hits <= 0)
            {
                brick.destroyed = true;
                score += (brick.type == 0 ? 10 : 20);
            }

            ball.dy = -ball.dy;
        }
    }

    //Create another level when all bricks are destroyed
    if(areAllBricksDestroyed())
    {
        createLevel();
    }

    //Ball speed increases over time
    ball.speed += ballSpeedIncrement;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_breakout_Game_nativeMovePaddle(JNIEnv* env, jobject obj, jfloat pos)
{
    paddle.x = pos;
}

extern "C" JNIEXPORT float JNICALL
Java_com_example_breakout_Game_nativeGetPaddleX(JNIEnv* env, jobject obj)
{
    return paddle.x;
}

extern "C" JNIEXPORT float JNICALL
Java_com_example_breakout_Game_nativeGetPaddleY(JNIEnv* env, jobject obj)
{
    return paddle.y;
}


extern "C" JNIEXPORT float JNICALL
Java_com_example_breakout_Game_nativeGetPaddleWidth(JNIEnv* env, jobject obj)
{
    return paddle.width;
}

extern "C" JNIEXPORT float JNICALL
Java_com_example_breakout_Game_nativeGetPaddleHeight(JNIEnv* env, jobject obj)
{
    return paddle.height;
}

extern "C" JNIEXPORT float JNICALL
Java_com_example_breakout_Game_nativeGetBallX(JNIEnv* env, jobject obj)
{
    return ball.x;
}

extern "C" JNIEXPORT float JNICALL
Java_com_example_breakout_Game_nativeGetBallY(JNIEnv* env, jobject obj)
{
    return ball.y;
}

extern "C" JNIEXPORT float JNICALL
Java_com_example_breakout_Game_nativeGetBallRadius(JNIEnv* env, jobject obj)
{
    return ball.radius;
}

extern "C" JNIEXPORT float JNICALL
Java_com_example_breakout_Game_nativeGetBrickX(JNIEnv* env, jobject obj, jint index )
{
    return bricks[index].x;
}

extern "C" JNIEXPORT float JNICALL
Java_com_example_breakout_Game_nativeGetBrickY(JNIEnv* env, jobject obj, jint index )
{
    return bricks[index].y;
}

extern "C" JNIEXPORT int JNICALL
Java_com_example_breakout_Game_nativeGetBrickType(JNIEnv* env, jobject obj, jint index )
{
    return bricks[index].type;
}

extern "C" JNIEXPORT int JNICALL
Java_com_example_breakout_Game_nativeGetBricksCount(JNIEnv* env, jobject obj)
{
    return bricks.size();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_breakout_Game_nativeIsBrickDestroyed(JNIEnv* env, jobject obj, int index)
{
    return bricks[index].destroyed;
}

extern "C" JNIEXPORT int JNICALL
Java_com_example_breakout_Game_nativeGetScore(JNIEnv* env, jobject obj)
{
    return score;
}

extern "C" JNIEXPORT int JNICALL
Java_com_example_breakout_Game_nativeGetLives(JNIEnv* env, jobject obj)
{
    return lives;
}