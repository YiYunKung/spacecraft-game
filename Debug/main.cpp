/*
 * ---------------- www.spacesimulator.net --------------
 *   ---- Space simulators and 3d engine tutorials ----
 *
 * Author: Damiano Vitulli
 *
 * This program is released under the BSD licence
 * By using this program you agree to licence terms on spacesimulator.net copyright page
 *
 *
 * Tutorial 5: Vectors and OpenGL lighting
 * 
 * 
 *
 * To compile this project you must include the following libraries:
 * opengl32.lib,glu32.lib,glut32.lib
 * You need also the header file glut.h in your compiler directory.
 *  
 */

/*
 * Spaceships credits:
 * fighter1.3ds - created by: Dario Vitulli 
 */

#ifdef __WIN32__
#include <windows.h>
#endif

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <GL/glut.h>
#include "load_bmp.h"
#include "load_3ds.h"
#include "object.h"
#include "texture.h"

#define TIMESTEP       0.00002
#define MAX_CHAR       128

/**********************************************************
 *
 * VARIABLES DECLARATION
 *
 *********************************************************/

// The width and height of your window, change them as you like
int screen_width=640;
int screen_height=480;

// Background Image
GLuint textureID = LoadBMP("texture1.bmp");
float textureOffset = 0.0f;     // 垂直偏移量

// Absolute rotation values (0-359 degrees) and rotation increments for each frame
double translation_x=0, translation_x_increment=0.0;
double translation_y=0, translation_y_increment=0.0;
double translation_z=0, translation_z_increment=-0.001;
double spaceshipSpeed = 0.01f;
 
// Flag for rendering as lines or filled polygons
int filling=1; //0=OFF 1=ON

//Lights settings
GLfloat light_ambient[]= { 0.1f, 0.1f, 0.1f, 0.1f };
GLfloat light_diffuse[]= { 1.0f, 1.0f, 1.0f, 0.0f };
GLfloat light_specular[]= { 1.0f, 1.0f, 1.0f, 0.0f };
GLfloat light_position[]= { 100.0f, 0.0f, -10.0f, 1.0f };

//Materials settings
GLfloat mat_ambient[]= { 0.1f, 0.1f, 0.1f, 0.0f };
GLfloat mat_diffuse[]= { 1.0f, 1.0f, 1.0f, 0.0f };
GLfloat mat_specular[]= { 0.2f, 0.2f, 0.2f, 0.0f };
GLfloat mat_shininess[]= { 1.0f };

// 存原始值
GLfloat original_light_ambient[4];
GLfloat original_light_diffuse[4];
GLfloat original_light_specular[4];
GLfloat original_light_position[4];

GLfloat original_mat_ambient[4];
GLfloat original_mat_diffuse[4];
GLfloat original_mat_specular[4];
GLfloat original_mat_shininess[1];


struct v4d {
    float x;
    float y;
    float z;
    float w;
};
float t;
v4d p = { 0, 0, 0, 1 };
v4d cameraPosition = { 0, 0, 0, 1 };
v4d obstacle[5];
int numObstacles = 5;   // 紀錄障礙物數量
float ship_width = 5.0f;
float ship_height = 3.0f;
float obstacle_width = 0.5f;
float obstacle_height = 0.5f;

// 倒數計時
int countdown = 30; 
int timerInterval = 1000; 

// 生命值
int score = 3;


// 貝氏曲線
v4d bezier(float t) {

    float c1, c2, c3, c4;
    v4d b;

    //v4d p0 = { -50.0, 0.0, 0.0 }, p1 = { 150.0, 150.0, 0.0 }, p2 = { -150.0, 150.0, 0.0, }, p3 = { 50.0, 0.0, 0.0 };
    v4d p0 = { 0.0, 0.0, -20.0, 1.0 }, p1 = { -50.0, 50.0, -80.0, 1.0 }, p2 = { 100.0, -50.0, -120.0, 1.0 }, p3 = { 0.0, 20.0, -150.0, 1.0 };

    c1 = (1 - t) * (1 - t) * (1 - t);
    c2 = 3 * t * (1 - t) * (1 - t);
    c3 = 3 * t * t * (1 - t);
    c4 = t * t * t;

    b.x = c1 * p0.x + c2 * p1.x + c3 * p2.x + c4 * p3.x;
    b.y = c1 * p0.y + c2 * p1.y + c3 * p2.y + c4 * p3.y;
    b.z = c1 * p0.z + c2 * p1.z + c3 * p2.z + c4 * p3.z;
    b.w = 1.0f;

    return b;
}


// 隨機生成障礙物
void generateObstacles() {

    // 隨機生成障礙物的位置
    for (int i = 0; i < numObstacles; i++) {
        obstacle[i].x = rand() % 40 - 20;    // x軸隨機位置
        obstacle[i].y = rand() % 40 - 20;    // y軸隨機位置
        obstacle[i].z = -(rand() % 30 + 30);    // z軸隨機位置
    }
}

void drawObstacles() {
    for (int i = 0; i < numObstacles; i++) {

        if (obstacle[i].z >= translation_z + 3) {       // 越過飛機
            obstacle[i].x = rand() % 40 - 20;    // x軸隨機位置
            obstacle[i].y = rand() % 40 - 20;    // y軸隨機位置
            obstacle[i].z = -(rand() % 30 + 30);    // z軸隨機位置
        }

        // 紅色材質
        GLfloat mat_ambient[] = { 1.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_diffuse[] = { 1.0f, 0.0f, 0.0f, 1.0f };
        GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat mat_shininess[] = { 200.0f };

        // 綁定材質屬性
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

        glPushMatrix();
            glColor3f(1, 0, 0);
            glLoadIdentity();
            glTranslatef(obstacle[i].x, obstacle[i].y, obstacle[i].z);
            glutSolidSphere(obstacle_width, 20, 20);
        glPopMatrix();

        obstacle[i].z += spaceshipSpeed;
    }
}


bool checkCollision(double ship_x, double ship_y, double ship_z, v4d obstacle[]) {

    // 检查边界框是否相交
    for (int i = 0; i < numObstacles; i++) {
        if ((ship_x - ship_width <= obstacle[i].x && ship_x + ship_width >= obstacle[i].x)
            && (ship_y + ship_height >= obstacle[i].y && ship_y - ship_height <= obstacle[i].y)
            && (ship_z + spaceshipSpeed/2 >= obstacle[i].z && ship_z - spaceshipSpeed/2 <= obstacle[i].z)) {
            return true;
        }
    }
    return false;
}

// 倒數計時
void drawTime() {

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), 0); // 设置正交投影矩阵
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(25.0f, 70.0f);
    std::string countdownStr = "Time: " + std::to_string(countdown);
    for (const char& c : countdownStr) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// 顯示生命值
void drawScore() {

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), 0); // 设置正交投影矩阵
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 设置文字颜色和位置
    glColor3f(1.0f, 1.0f, 1.0f); // 白色
    glRasterPos2f(25.0f, 40.0f); // 左上角位置，可根据需要进行调整

    // 将分数转换为字符串
    std::string scoreStr = "Life: " + std::to_string(score);

    // 逐个绘制字符
    for (char c : scoreStr) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


// 顯示遊戲結束之輸贏
void drawEnd() {

    // 恢复原始材质属性
    /*glMaterialfv(GL_FRONT, GL_AMBIENT, original_mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, original_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, original_mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, original_mat_shininess);
    glLightfv(GL_LIGHT1, GL_AMBIENT, original_light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, original_light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, original_light_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, original_light_position);*/

    // 紅色材質
    GLfloat mat_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_shininess[] = { 200.0f };

    // 綁定材質屬性
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), 0); // 设置正交投影矩阵
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

 

    // 文字結果
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2i(glutGet(GLUT_WINDOW_WIDTH) / 2 - 50, glutGet(GLUT_WINDOW_HEIGHT) / 2);

    std::string resultStr = "You win!";
    
    if (score <= 0) {
        resultStr = "You lose!";
    }

    for (char c : resultStr) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


/**********************************************************
 *
 * SUBROUTINE init(void)
 *
 * Used to initialize OpenGL and to setup our world
 *
 *********************************************************/

void init(void)
{
    glClearColor(0.0, 0.0, 0.0, 0.0); // Clear background color to black

    // Viewport transformation
    glViewport(0,0,screen_width,screen_height); 

    // 在初始化中保存原始材質屬性
    glGetMaterialfv(GL_FRONT, GL_AMBIENT, original_mat_ambient);
    glGetMaterialfv(GL_FRONT, GL_DIFFUSE, original_mat_diffuse);
    glGetMaterialfv(GL_FRONT, GL_SPECULAR, original_mat_specular);
    glGetMaterialfv(GL_FRONT, GL_SHININESS, original_mat_shininess);

    // 在初始化函数中保存原始光照属性
    glGetLightfv(GL_LIGHT1, GL_AMBIENT, original_light_ambient);
    glGetLightfv(GL_LIGHT1, GL_DIFFUSE, original_light_diffuse);
    glGetLightfv(GL_LIGHT1, GL_SPECULAR, original_light_specular);
    glGetLightfv(GL_LIGHT1, GL_POSITION, original_light_position);

    // Projection transformation
    glMatrixMode(GL_PROJECTION); // Specifies which matrix stack is the target for matrix operations 
    glLoadIdentity(); // We initialize the projection matrix as identity
    gluPerspective(45.0f,(GLfloat)screen_width/(GLfloat)screen_height,5.0f,10000.0f); // We define the "viewing volume"
   
	//Lights initialization and activation
    glLightfv (GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv (GL_LIGHT1, GL_SPECULAR, light_specular);
    glLightfv (GL_LIGHT1, GL_POSITION, light_position);    
    glEnable (GL_LIGHT1);
    glEnable (GL_LIGHTING);

    //Materials initialization and activation
	glMaterialfv (GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv (GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv (GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv (GL_FRONT, GL_SHININESS, mat_shininess);    

	//Other initializations
    glShadeModel(GL_SMOOTH); // Type of shading for the polygons
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Texture mapping perspective correction (OpenGL... thank you so much!)
    glEnable(GL_TEXTURE_2D); // Texture mapping ON
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL); // Polygon rasterization mode (polygon filled)
	glEnable(GL_CULL_FACE); // Enable the back face culling
    glEnable(GL_DEPTH_TEST); // Enable the depth test (also called z buffer)

	//Objects loading
	ObjLoad ("fighter1.3ds","skull.bmp");

}



/**********************************************************
 *
 * SUBROUTINE resize(int p_width, int p_height)
 *
 * This routine must be called everytime we resize our window.
 *
 * Input parameters: p_width = width in pixels of our viewport
 *					 p_height = height in pixels of our viewport
 * 
 *********************************************************/

void resize (int p_width, int p_height)
{
	if (screen_width==0 && screen_height==0) exit(0);
    screen_width=p_width; // We obtain the new screen width values and store it
    screen_height=p_height; // Height value

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // We clear both the color and the depth buffer so to draw the next frame
    glViewport(0,0,screen_width,screen_height); // Viewport transformation

    glMatrixMode(GL_PROJECTION); // Projection transformation
    glLoadIdentity(); // We initialize the projection matrix as identity
    gluPerspective(45.0f,(GLfloat)screen_width/(GLfloat)screen_height,5.0f,10000.0f);

    glutPostRedisplay (); // This command redraw the scene (it calls the same routine of glutDisplayFunc)
}



/**********************************************************
 *
 * SUBROUTINE keyboard(unsigned char p_key, int p_x, int p_y)
 *
 * Used to handle the keyboard input (ASCII Characters)
 *
 * Input parameters: p_key = ascii code of the key pressed
 *					 p_x = not used, keeped to maintain compatibility with glut function
 *                   p_y = not used, keeped to maintain compatibility with glut function
 * 
 *********************************************************/

void keyboard(unsigned char p_key, int p_x, int p_y)
{  
    switch (p_key)
    {
        case ' ':
            translation_x_increment=0;
            translation_y_increment=0;
            translation_z_increment=0;
        break;
		case 'r': case 'R':
            if (filling==0)
            {
                glPolygonMode (GL_FRONT_AND_BACK, GL_FILL); // Polygon rasterization mode (polygon filled)
                filling=1;
            }   
            else 
            {
                glPolygonMode (GL_FRONT_AND_BACK, GL_LINE); // Polygon rasterization mode (polygon outlined)
                filling=0;
            }
        break;
		case 27:
			exit(0);
		break;

        case 'w':
            translation_y_increment = 0.0005;  // 向上飛行（增加 Y 座標）
            break;
        case 's':
            translation_y_increment = -0.0005;  // 向下飛行（減少 Y 座標）
            break;
        case 'a':
            translation_x_increment = -0.0005;  // 向左飄移（減少 X 座標）
            break;
        case 'd':
            translation_x_increment = 0.0005;  // 向右飄移（增加 X 座標）
            break;
        case 'z':
            if (spaceshipSpeed <= 0.03)
                spaceshipSpeed += 0.002f;     // 加速行駛
            break;
        case 'x':
            if (spaceshipSpeed >= 0.005)
                spaceshipSpeed -= 0.002f;     // 減速行駛
            break;
    }
}



/**********************************************************
 *
 * Camera
 *
 *********************************************************/

void updateCamera() {
    // 設定鏡頭跟隨的偏移量
    float cameraOffsetX = 0.0f;
    float cameraOffsetY = 10.0f;
    float cameraOffsetZ = 5.0f;

    // 更新視角的位置
    cameraPosition.z = translation_z + cameraOffsetZ;

    // 設定 OpenGL 的視角
    gluLookAt(cameraPosition.x, cameraPosition.y, cameraPosition.z, -translation_x, -translation_y, translation_z, 0.0f, 1.0f, 0.0f);
}

/**********************************************************
 *
 * SUBROUTINE display(void)
 *
 * This is our main rendering subroutine, called each frame
 * 
 *********************************************************/

void display(void)
{
    int j;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // This clear the background color to dark blue
    glMatrixMode(GL_MODELVIEW); // Modeling transformation
    //glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // Initialize the model matrix as identity

    updateCamera();     // 相機跟隨

    drawObstacles();    // 障礙物

    // 背景
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), 0); // 设置正交投影矩阵
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
        glBindTexture(GL_TEXTURE_2D, textureID);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, textureOffset);
        glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, textureOffset);
        glVertex2f(1.0f, -1.0f);
        glTexCoord2f(1.0f, textureOffset + 1.0f);
        glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0.0f, textureOffset + 1.0f);
        glVertex2f(-1.0f, 1.0f);
        glEnd();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    drawTime();
    drawScore();

    //glTranslatef(p.x, p.y, p.z);    // 貝式曲線移動
    glTranslatef(0.0, 0.0, -20);    // We move the object forward (the model matrix is multiplied by the translation matrix)
 
    translation_x += translation_x_increment;
    translation_y += translation_y_increment;
    translation_z += translation_z_increment;

    if (translation_x > 359) translation_x = 0;
    if (translation_y > 359) translation_y = 0;
    if (translation_z > 359) translation_z = 0;

    /*
    glRotatef(rotation_x,1.0,0.0,0.0); // Rotations of the object (the model matrix is multiplied by the rotation matrices)
    glRotatef(rotation_y,0.0,1.0,0.0);
    glRotatef(rotation_z,0.0,0.0,1.0);*/

    glTranslatef(translation_x, translation_y, translation_z);
    
    //Lights initialization and activation
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);

    //Materials initialization and activation
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    //Other initializations
    glShadeModel(GL_SMOOTH); // Type of shading for the polygons
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Texture mapping perspective correction (OpenGL... thank you so much!)
    glEnable(GL_TEXTURE_2D); // Texture mapping ON
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Polygon rasterization mode (polygon filled)
    glEnable(GL_CULL_FACE); // Enable the back face culling
    glEnable(GL_DEPTH_TEST); // Enable the depth test (also called z buffer)


	if (object.id_texture!=-1) 
	{
		glBindTexture(GL_TEXTURE_2D, object.id_texture); // We set the active texture 
	    glEnable(GL_TEXTURE_2D); // Texture mapping ON
	}
	else
	    glDisable(GL_TEXTURE_2D); // Texture mapping OFF

	glBegin(GL_TRIANGLES); // glBegin and glEnd delimit the vertices that define a primitive (in our case triangles)
    for (j=0;j<object.polygons_qty;j++)
	{
		//----------------- FIRST VERTEX -----------------
		//Normal coordinates of the first vertex
		glNormal3f( object.normal[ object.polygon[j].a ].x,
					object.normal[ object.polygon[j].a ].y,
					object.normal[ object.polygon[j].a ].z);
		// Texture coordinates of the first vertex
		glTexCoord2f( object.mapcoord[ object.polygon[j].a ].u,
					  object.mapcoord[ object.polygon[j].a ].v);
		// Coordinates of the first vertex
		glVertex3f( object.vertex[ object.polygon[j].a ].x,
					object.vertex[ object.polygon[j].a ].y,
					object.vertex[ object.polygon[j].a ].z);

		//----------------- SECOND VERTEX -----------------
		//Normal coordinates of the second vertex
		glNormal3f( object.normal[ object.polygon[j].b ].x,
					object.normal[ object.polygon[j].b ].y,
					object.normal[ object.polygon[j].b ].z);
		// Texture coordinates of the second vertex
		glTexCoord2f( object.mapcoord[ object.polygon[j].b ].u,
					  object.mapcoord[ object.polygon[j].b ].v);
		// Coordinates of the second vertex
		glVertex3f( object.vertex[ object.polygon[j].b ].x,
					object.vertex[ object.polygon[j].b ].y,
					object.vertex[ object.polygon[j].b ].z);
        
		//----------------- THIRD VERTEX -----------------
		//Normal coordinates of the third vertex
		glNormal3f( object.normal[ object.polygon[j].c ].x,
					object.normal[ object.polygon[j].c ].y,
					object.normal[ object.polygon[j].c ].z);
		// Texture coordinates of the third vertex
		glTexCoord2f( object.mapcoord[ object.polygon[j].c ].u,
					  object.mapcoord[ object.polygon[j].c ].v);
		// Coordinates of the Third vertex
		glVertex3f( object.vertex[ object.polygon[j].c ].x,
					object.vertex[ object.polygon[j].c ].y,
					object.vertex[ object.polygon[j].c ].z);
	}
	glEnd();

    if (countdown <= 0 || score <= 0) {
        drawEnd();
    }

    glFlush(); // This force the execution of OpenGL commands
    glutSwapBuffers(); // In double buffered mode we invert the positions of the visible buffer and the writing buffer
}

void idle() {

    t += TIMESTEP;

    // bezier curve
    p = bezier(t);

    textureOffset += 0.001f;    // 背景滾動

    if (checkCollision(translation_x, translation_y, translation_z, obstacle)) {
        score--;
    }

    if (countdown <= 0 || score <= 0) {
        return;
    }

    glutPostRedisplay();
}

void timer(int value) {
    countdown--;

    if (countdown >= 0 && score > 0) {
        glutPostRedisplay(); 
        glutTimerFunc(timerInterval, timer, 0);     // 觸發下一個計時器
    }
}


/**********************************************************
 *
 * The main routine
 * 
 *********************************************************/

int main(int argc, char **argv)
{
    // We use the GLUT utility to initialize the window, to handle the input and to interact with the windows system
    glutInit(&argc, argv);    
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screen_width,screen_height);
    glutInitWindowPosition(0,0);
    glutCreateWindow("www.spacesimulator.net - 3d engine tutorials - To exit press ESC");    
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutReshapeFunc (resize);
    glutKeyboardFunc (keyboard);
    //glutSpecialFunc (keyboard_s);
    glutTimerFunc(timerInterval, timer, 0);

    generateObstacles();    // 產生障礙物
    init();
    glutMainLoop();

    return(0);    
}
